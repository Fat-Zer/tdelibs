/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2001-2005 Michael Brade <brade@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdirlister.h"

#include <tqregexp.h>
#include <tqptrlist.h>
#include <tqtimer.h>
#include <tqeventloop.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdeio/job.h>
#include <tdemessagebox.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kstaticdeleter.h>
#include <kprotocolinfo.h>

#include "kdirlister_p.h"

#include <assert.h>
#include <unistd.h>

KDirListerCache* KDirListerCache::s_pSelf = 0;
static KStaticDeleter<KDirListerCache> sd_KDirListerCache;

// Enable this to get printDebug() called often, to see the contents of the cache
//#define DEBUG_CACHE

// Make really sure it doesn't get activated in the final build
#ifdef NDEBUG
#undef DEBUG_CACHE
#endif

KDirListerCache::KDirListerCache( int maxCount )
  : itemsCached( maxCount )
{
  kdDebug(7004) << "+KDirListerCache" << endl;

  itemsInUse.setAutoDelete( false );
  itemsCached.setAutoDelete( true );
  urlsCurrentlyListed.setAutoDelete( true );
  urlsCurrentlyHeld.setAutoDelete( true );
  pendingUpdates.setAutoDelete( true );

  connect( kdirwatch, TQT_SIGNAL( dirty( const KURL& ) ),
           this, TQT_SLOT( slotFileDirty( const KURL& ) ) );
  connect( kdirwatch, TQT_SIGNAL( created( const TQString& ) ),
           this, TQT_SLOT( slotFileCreated( const TQString& ) ) );
  connect( kdirwatch, TQT_SIGNAL( deleted( const TQString& ) ),
           this, TQT_SLOT( slotFileDeleted( const TQString& ) ) );
}

KDirListerCache::~KDirListerCache()
{
  kdDebug(7004) << "-KDirListerCache" << endl;

  itemsInUse.setAutoDelete( true );
  itemsInUse.clear();
  itemsCached.clear();
  urlsCurrentlyListed.clear();
  urlsCurrentlyHeld.clear();

  if ( KDirWatch::exists() )
    kdirwatch->disconnect( this );
}

// setting _reload to true will emit the old files and
// call updateDirectory
bool KDirListerCache::listDir( KDirLister *lister, const KURL& _u,
                               bool _keep, bool _reload )
{
  // HACK
  // The media:/ tdeioslave has massive problems related to not properly updating its root directory
  // Therefore, force a reload every time the media:/ tdeioslave root is accessed!
  // FIXME
  // For anyone wanting to tackle this problem, it was traced into the KDirListerCache::updateDirectory TDEIO::listDir TDEIO job
  // Specifically, slotUpdateResult is never called for the root directory *iff* the user descends into an unmounted media device
  // Strangely, slotUpdateResult *is* called if the user instead right-clicks on the unmounted media device and selects Mount from the context menu
  if ((_u.protocol() == "media") && (_u.path() == "/")) {
    _reload = true;
  }

  // like this we don't have to worry about trailing slashes any further
  KURL _url = _u;
  _url.cleanPath(); // kill consecutive slashes
  _url.adjustPath(-1);
  TQString urlStr = _url.url();
  TQString urlReferenceStr = _url.internalReferenceURL();

  if ( !lister->validURL( _url ) ) {
    return false;
  }

#ifdef DEBUG_CACHE
  printDebug();
#endif
  kdDebug(7004) << k_funcinfo << lister << " url=" << _url
                << " keep=" << _keep << " reload=" << _reload << endl;

  if ( !_keep )
  {
    // stop any running jobs for lister
    stop( lister );

    // clear our internal list for lister
    forgetDirs( lister );

    lister->d->rootFileItem = 0;
  }
  else if ( lister->d->lstDirs.find( _url ) != lister->d->lstDirs.end() )
  {
    // stop the job listing _url for this lister
    stop( lister, _url );

    // clear _url for lister
    forgetDirs( lister, _url, true );

    if ( lister->d->url == _url )
      lister->d->rootFileItem = 0;
  }

  lister->d->lstDirs.append( _url );

  if ( lister->d->url.isEmpty() || !_keep ) // set toplevel URL only if not set yet
    lister->d->url = _url;

  DirItem *itemU = itemsInUse[urlStr + ":" + urlReferenceStr];
  DirItem *itemC;

  if ( !urlsCurrentlyListed[urlStr + ":" + urlReferenceStr] )
  {
    // if there is an update running for _url already we get into
    // the following case - it will just be restarted by updateDirectory().

    if ( itemU )
    {
      kdDebug(7004) << "listDir: Entry already in use: " << _url << endl;

      bool oldState = lister->d->complete;
      lister->d->complete = false;

      emit lister->started( _url );

      if ( !lister->d->rootFileItem && lister->d->url == _url ) {
        lister->d->rootFileItem = itemU->rootItem;
      }

      lister->addNewItems( *(itemU->lstItems) );
      lister->emitItems();

      // _url is already in use, so there is already an entry in urlsCurrentlyHeld
      assert( urlsCurrentlyHeld[urlStr + ":" + urlReferenceStr] );
      urlsCurrentlyHeld[urlStr + ":" + urlReferenceStr]->append( lister );

      lister->d->complete = oldState;

      lister->emitCompleted( _url );
      if ( lister->d->complete ) {
        emit lister->completed();
      }

      if ( _reload || !itemU->complete ) {
        updateDirectory( _url );
      }
    }
    else if ( !_reload && (itemC = itemsCached.take( urlStr )) )
    {
      kdDebug(7004) << "listDir: Entry in cache: " << _url << endl;

      itemC->decAutoUpdate();
      itemsInUse.insert( urlStr + ":" + urlReferenceStr, itemC );
      itemU = itemC;

      bool oldState = lister->d->complete;
      lister->d->complete = false;

      emit lister->started( _url );

      if ( !lister->d->rootFileItem && lister->d->url == _url ) {
        lister->d->rootFileItem = itemC->rootItem;
      }

      lister->addNewItems( *(itemC->lstItems) );
      lister->emitItems();

      Q_ASSERT( !urlsCurrentlyHeld[urlStr + ":" + urlReferenceStr] );
      TQPtrList<KDirLister> *list = new TQPtrList<KDirLister>;
      list->append( lister );
      urlsCurrentlyHeld.insert( urlStr + ":" + urlReferenceStr, list );

      lister->d->complete = oldState;

      lister->emitCompleted( _url );
      if ( lister->d->complete ) {
        emit lister->completed();
      }

      if ( !itemC->complete ) {
        updateDirectory( _url );
      }
    }
    else  // dir not in cache or _reload is true
    {
      kdDebug(7004) << "listDir: Entry not in cache or reloaded: " << _url << endl;

      TQPtrList<KDirLister> *list = new TQPtrList<KDirLister>;
      list->append( lister );
      urlsCurrentlyListed.insert( urlStr + ":" + urlReferenceStr, list );

      itemsCached.remove( urlStr );
      itemU = new DirItem( _url );
      itemsInUse.insert( urlStr + ":" + urlReferenceStr, itemU );

//        // we have a limit of MAX_JOBS_PER_LISTER concurrently running jobs
//        if ( lister->numJobs() >= MAX_JOBS_PER_LISTER )
//        {
//          lstPendingUpdates.append( _url );
//        }
//        else
//        {

      if ( lister->d->url == _url ) {
        lister->d->rootFileItem = 0;
      }

      TDEIO::ListJob* job = TDEIO::listDir( _url, false /* no default GUI */ );
      jobs.insert( job, TQValueList<TDEIO::UDSEntry>() );

      lister->jobStarted( job );
      lister->connectJob( job );

      if ( lister->d->window ) {
        job->setWindow( lister->d->window );
      }

      connect( job, TQT_SIGNAL( entries( TDEIO::Job *, const TDEIO::UDSEntryList & ) ),
               this, TQT_SLOT( slotEntries( TDEIO::Job *, const TDEIO::UDSEntryList & ) ) );
      connect( job, TQT_SIGNAL( result( TDEIO::Job * ) ),
               this, TQT_SLOT( slotResult( TDEIO::Job * ) ) );
      connect( job, TQT_SIGNAL( redirection( TDEIO::Job *, const KURL & ) ),
               this, TQT_SLOT( slotRedirection( TDEIO::Job *, const KURL & ) ) );

      emit lister->started( _url );

//        }
    }
  }
  else
  {
    kdDebug(7004) << "listDir: Entry currently being listed: " << _url << endl;

    emit lister->started( _url );

    urlsCurrentlyListed[urlStr + ":" + urlReferenceStr]->append( lister );

    TDEIO::ListJob *job = jobForUrl( urlStr + ":" + urlReferenceStr );
    Q_ASSERT( job );

    lister->jobStarted( job );
    lister->connectJob( job );

    Q_ASSERT( itemU );

    if ( !lister->d->rootFileItem && lister->d->url == _url ) {
      lister->d->rootFileItem = itemU->rootItem;
    }

    lister->addNewItems( *(itemU->lstItems) );
    lister->emitItems();
  }

  // automatic updating of directories
  if ( lister->d->autoUpdate ) {
    itemU->incAutoUpdate();
  }

  return true;
}

bool KDirListerCache::validURL( const KDirLister *lister, const KURL& url ) const
{
  if ( !url.isValid() )
  {
    if ( lister->d->autoErrorHandling )
    {
      TQString tmp = i18n("Malformed URL\n%1").arg( url.prettyURL() );
      KMessageBox::error( lister->d->errorParent, tmp );
    }
    return false;
  }

  if ( !KProtocolInfo::supportsListing( url ) )
  {
    if ( lister->d->autoErrorHandling )
    {
      // TODO: this message should be changed during next string unfreeze!
      TQString tmp = i18n("Malformed URL\n%1").arg( url.prettyURL() );
      KMessageBox::error( lister->d->errorParent, tmp );
    }
    return false;
  }

  return true;
}

void KDirListerCache::stop( KDirLister *lister )
{
#ifdef DEBUG_CACHE
  printDebug();
#endif
  kdDebug(7004) << k_funcinfo << "lister: " << lister << endl;
  bool stopped = false;

  TQDictIterator< TQPtrList<KDirLister> > it( urlsCurrentlyListed );
  TQPtrList<KDirLister> *listers;
  while ( (listers = it.current()) )
  {
    if ( listers->findRef( lister ) > -1 )
    {
      // lister is listing url
      TQString url = it.currentKey();

      //kdDebug(7004) << k_funcinfo << " found lister in list - for " << url << endl;
      bool ret = listers->removeRef( lister );
      Q_ASSERT( ret );

      TDEIO::ListJob *job = jobForUrl( url );
      if ( job ) {
        lister->jobDone( job );
      }

      // move lister to urlsCurrentlyHeld
      TQPtrList<KDirLister> *holders = urlsCurrentlyHeld[url];
      if ( !holders )
      {
        holders = new TQPtrList<KDirLister>;
        urlsCurrentlyHeld.insert( url, holders );
      }

      holders->append( lister );

      emit lister->canceled( KURL( url ) );

      //kdDebug(7004) << k_funcinfo << "remaining list: " << listers->count() << " listers" << endl;

      if ( listers->isEmpty() )
      {
        // kill the job since it isn't used any more
        if ( job )
          killJob( job );

        urlsCurrentlyListed.remove( url );
      }

      stopped = true;
    }
    else
      ++it;
  }

  if ( stopped )
  {
    emit lister->canceled();
    lister->d->complete = true;
  }

  // this is wrong if there is still an update running!
  //Q_ASSERT( lister->d->complete );
}

void KDirListerCache::stop( KDirLister *lister, const KURL& _u )
{
  TQString urlStr( _u.url(-1) );
  TQString urlReferenceStr = _u.internalReferenceURL();
  KURL _url( urlStr );

  // TODO: consider to stop all the "child jobs" of _url as well
  kdDebug(7004) << k_funcinfo << lister << " url=" << _url << endl;

  TQPtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr + ":" + urlReferenceStr];
  if ( !listers || !listers->removeRef( lister ) )
    return;

  // move lister to urlsCurrentlyHeld
  TQPtrList<KDirLister> *holders = urlsCurrentlyHeld[urlStr + ":" + urlReferenceStr];
  if ( !holders )
  {
    holders = new TQPtrList<KDirLister>;
    urlsCurrentlyHeld.insert( urlStr + ":" + urlReferenceStr, holders );
  }

  holders->append( lister );


  TDEIO::ListJob *job = jobForUrl( urlStr + ":" + urlReferenceStr );
  if ( job )
    lister->jobDone( job );

  emit lister->canceled( _url );

  if ( listers->isEmpty() )
  {
    // kill the job since it isn't used any more
    if ( job )
      killJob( job );

    urlsCurrentlyListed.remove( urlStr + ":" + urlReferenceStr );
  }

  if ( lister->numJobs() == 0 )
  {
    lister->d->complete = true;

    // we killed the last job for lister
    emit lister->canceled();
  }
}

void KDirListerCache::setAutoUpdate( KDirLister *lister, bool enable )
{
  // IMPORTANT: this method does not check for the current autoUpdate state!

  for ( KURL::List::Iterator it = lister->d->lstDirs.begin();
        it != lister->d->lstDirs.end(); ++it )
  {
    if ( enable ) {
      itemsInUse[(*it).url() + ":" + (*it).internalReferenceURL()]->incAutoUpdate();
    }
    else {
      itemsInUse[(*it).url() + ":" + (*it).internalReferenceURL()]->decAutoUpdate();
    }
  }
}

void KDirListerCache::forgetDirs( KDirLister *lister )
{
  kdDebug(7004) << k_funcinfo << lister << endl;

  emit lister->clear();

  // forgetDirs() will modify lstDirs, make a copy first
  KURL::List lstDirsCopy = lister->d->lstDirs;
  for ( KURL::List::Iterator it = lstDirsCopy.begin();
        it != lstDirsCopy.end(); ++it )
  {
    forgetDirs( lister, *it, false );
  }
}

void KDirListerCache::forgetDirs( KDirLister *lister, const KURL& _url, bool notify )
{
  kdDebug(7004) << k_funcinfo << lister << " _url: " << _url << endl;

  KURL url( _url );
  url.adjustPath( -1 );
  TQString urlStr = url.url();
  TQString urlReferenceStr = url.internalReferenceURL();
  TQPtrList<KDirLister> *holders = urlsCurrentlyHeld[urlStr + ":" + urlReferenceStr];
  //Q_ASSERT( holders );
  if ( holders )
  {
    holders->removeRef( lister );
  }

  // remove the dir from lister->d->lstDirs so that it doesn't contain things
  // that itemsInUse doesn't. When emitting the canceled signals lstDirs must
  // not contain anything that itemsInUse does not contain. (otherwise it 
  // might crash in findByName()).
  lister->d->lstDirs.remove( lister->d->lstDirs.find( url ) );

  DirItem *item = itemsInUse[urlStr + ":" + urlReferenceStr];

  if ( holders && holders->isEmpty() )
  {
    urlsCurrentlyHeld.remove( urlStr + ":" + urlReferenceStr ); // this deletes the (empty) holders list
    if ( !urlsCurrentlyListed[urlStr + ":" + urlReferenceStr] )
    {
      // item not in use anymore -> move into cache if complete
      itemsInUse.remove( urlStr + ":" + urlReferenceStr );

      // this job is a running update
      TDEIO::ListJob *job = jobForUrl( urlStr + ":" + urlReferenceStr );
      if ( job )
      {
        lister->jobDone( job );
        killJob( job );
        kdDebug(7004) << k_funcinfo << "Killing update job for " << urlStr << endl;

        emit lister->canceled( url );
        if ( lister->numJobs() == 0 )
        {
          lister->d->complete = true;
          emit lister->canceled();
        }
      }

      if ( notify )
        emit lister->clear( url );

      if ( item && item->complete )
      {
        kdDebug(7004) << k_funcinfo << lister << " item moved into cache: " << url << endl;
        itemsCached.insert( urlStr, item ); // TODO: may return false!!

        // Should we forget the dir for good, or keep a watch on it?
        // Generally keep a watch, except when it would prevent
        // unmounting a removable device (#37780)
        const bool isLocal = item->url.isLocalFile();
        const bool isManuallyMounted = isLocal && TDEIO::manually_mounted( item->url.path() );
        bool containsManuallyMounted = false;
        if ( !isManuallyMounted && item->lstItems && isLocal ) 
        {
          // Look for a manually-mounted directory inside
          // If there's one, we can't keep a watch either, FAM would prevent unmounting the CDROM
          // I hope this isn't too slow (manually_mounted caches the last device so most
          // of the time this is just a stat per subdir)
          KFileItemListIterator kit( *item->lstItems );
          for ( ; kit.current() && !containsManuallyMounted; ++kit )
            if ( (*kit)->isDir() && TDEIO::manually_mounted( (*kit)->url().path() ) )
              containsManuallyMounted = true;
        }

        if ( isManuallyMounted || containsManuallyMounted ) 
        {
          kdDebug(7004) << "Not adding a watch on " << item->url << " because it " <<
            ( isManuallyMounted ? "is manually mounted" : "contains a manually mounted subdir" ) << endl;
          item->complete = false; // set to "dirty"
        }
        else
          item->incAutoUpdate(); // keep watch
      }
      else
      {
        delete item;
        item = 0;
      }
    }
  }

  if ( item && lister->d->autoUpdate )
    item->decAutoUpdate();
}

void KDirListerCache::updateDirectory( const KURL& _dir )
{
  kdDebug(7004) << k_funcinfo << _dir << endl;

  TQString urlStr = _dir.url(-1);
  TQString urlReferenceStr = _dir.internalReferenceURL();
  if ( !checkUpdate( _dir, -1 ) ) {
    return;
  }

  // A job can be running to
  //   - only list a new directory: the listers are in urlsCurrentlyListed
  //   - only update a directory: the listers are in urlsCurrentlyHeld
  //   - update a currently running listing: the listers are in urlsCurrentlyListed
  //     and urlsCurrentlyHeld

  TQPtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr + ":" + urlReferenceStr];
  TQPtrList<KDirLister> *holders = urlsCurrentlyHeld[urlStr + ":" + urlReferenceStr];

  // restart the job for _dir if it is running already
  bool killed = false;
  TQWidget *window = 0;
  TDEIO::ListJob *job = jobForUrl( urlStr + ":" + urlReferenceStr );
  if ( job )
  {
     window = job->window();

     killJob( job );
     killed = true;

     if ( listers ) {
        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
           kdl->jobDone( job );
        }
     }

     if ( holders ) {
        for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() ) {
           kdl->jobDone( job );
        }
     }
  }
  kdDebug(7004) << k_funcinfo << "Killed = " << killed << endl;

  // we don't need to emit canceled signals since we only replaced the job,
  // the listing is continuing.

  Q_ASSERT( !listers || (listers && killed) );

  job = TDEIO::listDir( _dir, false /* no default GUI */ );
  jobs.insert( job, TQValueList<TDEIO::UDSEntry>() );

  connect( job, TQT_SIGNAL(entries( TDEIO::Job *, const TDEIO::UDSEntryList & )),
           this, TQT_SLOT(slotUpdateEntries( TDEIO::Job *, const TDEIO::UDSEntryList & )) );
  connect( job, TQT_SIGNAL(result( TDEIO::Job * )),
           this, TQT_SLOT(slotUpdateResult( TDEIO::Job * )) );

  kdDebug(7004) << k_funcinfo << "update started in " << _dir << endl;

  if ( listers ) {
     for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
        kdl->jobStarted( job );
     }
  }

  if ( holders )
  {
     if ( !killed )
     {
        bool first = true;
        for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
        {
           kdl->jobStarted( job );
           if ( first && kdl->d->window )
           {
              first = false;
              job->setWindow( kdl->d->window );
           }
           emit kdl->started( _dir );
        }
     }
     else
     {
        job->setWindow( window );

        for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() ) {
           kdl->jobStarted( job );
        }
     }
  }
}

bool KDirListerCache::checkUpdate( const KURL& _dir, int truncationMode )
{
  if ( !itemsInUse[_dir.url(truncationMode) + ":" + _dir.internalReferenceURL()] )
  {
    DirItem *item = itemsCached[_dir.url(truncationMode)];
    if ( item && item->complete )
    {
      item->complete = false;
      item->decAutoUpdate();
      // Hmm, this debug output might include login/password from the _dir URL.
      //kdDebug(7004) << k_funcinfo << "directory " << _dir << " not in use, marked dirty." << endl;
    }
    //else
      //kdDebug(7004) << k_funcinfo << "aborted, directory " << _dir << " not in cache." << endl;

    return false;
  }
  else
    return true;
}

KFileItemList *KDirListerCache::itemsForDir( const KURL &_dir ) const
{
  TQString urlStr = _dir.url(-1);
  TQString urlReferenceStr = _dir.internalReferenceURL();
  DirItem *item = itemsInUse[ urlStr + ":" + urlReferenceStr ];
  if ( !item ) {
    item = itemsCached[ urlStr ];
  }
  return item ? item->lstItems : 0;
}

KFileItem *KDirListerCache::findByName( const KDirLister *lister, const TQString& _name ) const
{
  Q_ASSERT( lister );

  for ( KURL::List::Iterator it = lister->d->lstDirs.begin();
        it != lister->d->lstDirs.end(); ++it )
  {
    KFileItemListIterator kit( *itemsInUse[(*it).url() + ":" + (*it).internalReferenceURL()]->lstItems );
    for ( ; kit.current(); ++kit )
      if ( (*kit)->name() == _name )
        return (*kit);
  }

  return 0L;
}

KFileItem *KDirListerCache::findByURL( const KDirLister *lister, const KURL& _u ) const
{
  KURL _url = _u;
  _url.adjustPath(-1);

  KURL parentDir( _url );
  parentDir.setPath( parentDir.directory() );

  // If lister is set, check that it contains this dir
  if ( lister && !lister->d->lstDirs.contains( parentDir ) )
      return 0L;

  KFileItemList *itemList = itemsForDir( parentDir );
  if ( itemList )
  {
    KFileItemListIterator kit( *itemList );
    for ( ; kit.current(); ++kit )
      if ( (*kit)->url() == _url )
        return (*kit);
  }
  return 0L;
}

void KDirListerCache::FilesAdded( const KURL &dir )
{
  kdDebug(7004) << k_funcinfo << dir << endl;
  updateDirectory( dir );
}

void KDirListerCache::FilesRemoved( const KURL::List &fileList )
{
  kdDebug(7004) << k_funcinfo << endl;
  KURL::List::ConstIterator it = fileList.begin();
  for ( ; it != fileList.end() ; ++it )
  {
    // emit the deleteItem signal if this file was shown in any view
    KFileItem *fileitem = 0L;
    KURL parentDir( *it );
    parentDir.setPath( parentDir.directory() );
    KFileItemList *lstItems = itemsForDir( parentDir );
    if ( lstItems )
    {
      KFileItem *fit = lstItems->first();
      for ( ; fit; fit = lstItems->next() )
        if ( fit->url() == *it ) {
          fileitem = fit;
          lstItems->take(); // remove fileitem from list
          break;
        }
    }

    // Tell the views about it before deleting the KFileItems. They might need the subdirs'
    // file items (see the dirtree).
    if ( fileitem )
    {
      TQPtrList<KDirLister> *listers = urlsCurrentlyHeld[parentDir.url() + ":" + parentDir.internalReferenceURL()];
      if ( listers ) {
        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
          kdl->emitDeleteItem( fileitem );
        }
      }
    }

    // If we found a fileitem, we can test if it's a dir. If not, we'll go to deleteDir just in case.
    if ( !fileitem || fileitem->isDir() )
    {
      // in case of a dir, check if we have any known children, there's much to do in that case
      // (stopping jobs, removing dirs from cache etc.)
      deleteDir( *it );
    }

    // now remove the item itself
    delete fileitem;
  }
}

void KDirListerCache::FilesChanged( const KURL::List &fileList )
{
  KURL::List dirsToUpdate;
  kdDebug(7004) << k_funcinfo << "only half implemented" << endl;
  KURL::List::ConstIterator it = fileList.begin();
  for ( ; it != fileList.end() ; ++it )
  {
    if ( ( *it ).isLocalFile() )
    {
      kdDebug(7004) << "KDirListerCache::FilesChanged " << *it << endl;
      KFileItem *fileitem = findByURL( 0, *it );
      if ( fileitem )
      {
          // we need to refresh the item, because e.g. the permissions can have changed.
          aboutToRefreshItem( fileitem );
          fileitem->refresh();
          emitRefreshItem( fileitem );
      }
      else {
          kdDebug(7004) << "item not found" << endl;
      }
    } else {
      // For remote files, refresh() won't be able to figure out the new information.
      // Let's update the dir.
      KURL dir( *it );
      dir.setPath( dir.directory( true ) );
      if ( dirsToUpdate.find( dir ) == dirsToUpdate.end() ) {
        dirsToUpdate.prepend( dir );
      }
    }
  }

  KURL::List::ConstIterator itdir = dirsToUpdate.begin();
  for ( ; itdir != dirsToUpdate.end() ; ++itdir ) {
    updateDirectory( *itdir );
  }
  // ## TODO problems with current jobs listing/updating that dir
  // ( see kde-2.2.2's kdirlister )
}

void KDirListerCache::FileRenamed( const KURL &src, const KURL &dst )
{
  kdDebug(7004) << k_funcinfo << src.prettyURL() << " -> " << dst.prettyURL() << endl;
#ifdef DEBUG_CACHE
  printDebug();
#endif

  // Somehow this should only be called if src is a dir. But how could we know if it is?
  // (Note that looking into itemsInUse isn't good enough. One could rename a subdir in a view.)
  renameDir( src, dst );

  // Now update the KFileItem representing that file or dir (not exclusive with the above!)
  KURL oldurl( src );
  oldurl.adjustPath( -1 );
  KFileItem *fileitem = findByURL( 0, oldurl );
  if ( fileitem )
  {
    if ( !fileitem->isLocalFile() && !fileitem->localPath().isEmpty() ) // it uses UDS_LOCAL_PATH? ouch, needs an update then
        FilesChanged( src );
    else
    {
        aboutToRefreshItem( fileitem );
        fileitem->setURL( dst );
        fileitem->refreshMimeType();
        emitRefreshItem( fileitem );
    }
  }
#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::aboutToRefreshItem( KFileItem *fileitem )
{
  // Look whether this item was shown in any view, i.e. held by any dirlister
  KURL parentDir( fileitem->url() );
  parentDir.setPath( parentDir.directory() );
  TQString parentDirURL = parentDir.url();
  TQString parentDirReferenceURL = parentDir.internalReferenceURL();
  TQPtrList<KDirLister> *listers = urlsCurrentlyHeld[parentDirURL + ":" + parentDirReferenceURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
      kdl->aboutToRefreshItem( fileitem );

  // Also look in urlsCurrentlyListed, in case the user manages to rename during a listing
  listers = urlsCurrentlyListed[parentDirURL + ":" + parentDirReferenceURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
      kdl->aboutToRefreshItem( fileitem );
}

void KDirListerCache::emitRefreshItem( KFileItem *fileitem )
{
  // Look whether this item was shown in any view, i.e. held by any dirlister
  KURL parentDir( fileitem->url() );
  parentDir.setPath( parentDir.directory() );
  TQString parentDirURL = parentDir.url();
  TQString parentDirReferenceURL = parentDir.internalReferenceURL();
  TQPtrList<KDirLister> *listers = urlsCurrentlyHeld[parentDirURL + ":" + parentDirReferenceURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->addRefreshItem( fileitem );
      kdl->emitItems();
    }

  // Also look in urlsCurrentlyListed, in case the user manages to rename during a listing
  listers = urlsCurrentlyListed[parentDirURL + ":" + parentDirReferenceURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->addRefreshItem( fileitem );
      kdl->emitItems();
    }
}

KDirListerCache* KDirListerCache::self()
{
  if ( !s_pSelf )
    s_pSelf = sd_KDirListerCache.setObject( s_pSelf, new KDirListerCache );

  return s_pSelf;
}

bool KDirListerCache::exists()
{
  return s_pSelf != 0;
}
 

// private slots

// _file can also be a directory being currently held!
void KDirListerCache::slotFileDirty( const KURL& _url )
{
  kdDebug(7004) << k_funcinfo << _url << endl;

  if ( !pendingUpdates[_url.path()] )
  {
    KURL dir;
    dir.setPath( _url.path() );
    dir.setInternalReferenceURL(_url.internalReferenceURL());
    if ( checkUpdate( dir, -1 ) ) {
      updateDirectory( _url );
    }

    // the parent directory of _url.path()
    dir.setPath( dir.directory() );
    dir.setInternalReferenceURL(_url.internalReferenceURL());
    if ( checkUpdate( dir ) )
    {
      // Nice hack to save memory: use the qt object name to store the filename
      TQTimer *timer = new TQTimer( this, _url.path().utf8() );
      connect( timer, TQT_SIGNAL(timeout()), this, TQT_SLOT(slotFileDirtyDelayed()) );
      pendingUpdates.insert( _url.path(), timer );
      timer->start( 500, true );
    }
  }
}

// delayed updating of files, FAM is flooding us with events
void KDirListerCache::slotFileDirtyDelayed()
{
  TQString file = TQString::fromUtf8( TQT_TQOBJECT_CONST(sender())->name() );

  kdDebug(7004) << k_funcinfo << file << endl;

  // TODO: do it better: don't always create/delete the TQTimer but reuse it.
  // Delete the timer after the parent directory is removed from the cache.
  pendingUpdates.remove( file );

  KURL u;
  u.setPath( file );
  KFileItem *item = findByURL( 0, u ); // search all items
  if ( item )
  {
    // we need to refresh the item, because e.g. the permissions can have changed.
    aboutToRefreshItem( item );
    item->refresh();
    emitRefreshItem( item );
  }
}

void KDirListerCache::slotFileCreated( const TQString& _file )
{
  kdDebug(7004) << k_funcinfo << _file << endl;
  // XXX: how to avoid a complete rescan here?
  KURL u;
  u.setPath( _file );
  u.setPath( u.directory() );
  FilesAdded( u );
}

void KDirListerCache::slotFileDeleted( const TQString& _file )
{
  kdDebug(7004) << k_funcinfo << _file << endl;
  KURL u;
  u.setPath( _file );
  FilesRemoved( u );
}

void KDirListerCache::slotEntries( TDEIO::Job *job, const TDEIO::UDSEntryList &entries )
{
  KURL url = joburl( static_cast<TDEIO::ListJob *>(job) );
  url.adjustPath(-1);
  TQString urlStr = url.url();
  TQString urlReferenceStr = url.internalReferenceURL();

  kdDebug(7004) << k_funcinfo << "new entries for " << url << endl;

  DirItem *dir = itemsInUse[urlStr + ":" + urlReferenceStr];
  Q_ASSERT( dir );

  TQPtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr + ":" + urlReferenceStr];
  Q_ASSERT( listers );
  Q_ASSERT( !listers->isEmpty() );

  // check if anyone wants the mimetypes immediately
  bool delayedMimeTypes = true;
  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
    delayedMimeTypes = delayedMimeTypes && kdl->d->delayedMimeTypes;
  }

  // avoid creating these QStrings again and again
  static const TQString& dot = TDEGlobal::staticQString(".");
  static const TQString& dotdot = TDEGlobal::staticQString("..");

  TDEIO::UDSEntryListConstIterator it = entries.begin();
  TDEIO::UDSEntryListConstIterator end = entries.end();

  for ( ; it != end; ++it )
  {
    TQString name;

    // find out about the name
    TDEIO::UDSEntry::ConstIterator entit = (*it).begin();
    for( ; entit != (*it).end(); ++entit ) {
      if ( (*entit).m_uds == TDEIO::UDS_NAME ) {
        name = (*entit).m_str;
        break;
      }
    }

    Q_ASSERT( !name.isEmpty() );
    if ( name.isEmpty() ) {
      continue;
    }

    if ( name == dot )
    {
      Q_ASSERT( !dir->rootItem );
      dir->rootItem = new KFileItem( *it, url, delayedMimeTypes, true  );

      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
        if ( !kdl->d->rootFileItem && kdl->d->url == url ) {
          kdl->d->rootFileItem = dir->rootItem;
        }
      }
    }
    else if ( name != dotdot )
    {
      KFileItem* item = new KFileItem( *it, url, delayedMimeTypes, true );
      Q_ASSERT( item );

      //kdDebug(7004)<< "Adding item: " << item->url() << endl;
      dir->lstItems->append( item );

      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
        kdl->addNewItem( item );
      }
    }
  }

  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
    kdl->emitItems();
  }
}

void KDirListerCache::slotResult( TDEIO::Job *j )
{
  Q_ASSERT( j );
  TDEIO::ListJob *job = static_cast<TDEIO::ListJob *>( j );
  jobs.remove( job );

  KURL jobUrl = joburl( job );
  jobUrl.adjustPath(-1);  // need remove trailing slashes again, in case of redirections
  TQString jobUrlStr = jobUrl.url();
  TQString jobReferenceUrlStr = jobUrl.internalReferenceURL();

  kdDebug(7004) << k_funcinfo << "finished listing " << jobUrl << endl;
#ifdef DEBUG_CACHE
  printDebug();
#endif

  TQPtrList<KDirLister> *listers = urlsCurrentlyListed.take( jobUrlStr + ":" + jobReferenceUrlStr );
  Q_ASSERT( listers );

  // move the directory to the held directories, do it before emitting
  // the signals to make sure it exists in KDirListerCache in case someone
  // calls listDir during the signal emission
  Q_ASSERT( !urlsCurrentlyHeld[jobUrlStr + ":" + jobReferenceUrlStr] );
  urlsCurrentlyHeld.insert( jobUrlStr + ":" + jobReferenceUrlStr, listers );

  KDirLister *kdl;

  if ( job->error() )
  {
    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->jobDone( job );
      kdl->handleError( job );
      emit kdl->canceled( jobUrl );
      if ( kdl->numJobs() == 0 )
      {
        kdl->d->complete = true;
        emit kdl->canceled();
      }
    }
  }
  else
  {
    DirItem *dir = itemsInUse[jobUrlStr + ":" + jobReferenceUrlStr];
    Q_ASSERT( dir );
    dir->complete = true;

    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->jobDone( job );
      kdl->emitCompleted( jobUrl );
      if ( kdl->numJobs() == 0 )
      {
        kdl->d->complete = true;
        emit kdl->completed();
      }
    }
  }

  // TODO: hmm, if there was an error and job is a parent of one or more
  // of the pending urls we should cancel it/them as well
  processPendingUpdates();

#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::slotRedirection( TDEIO::Job *j, const KURL& url )
{
  Q_ASSERT( j );
  TDEIO::ListJob *job = static_cast<TDEIO::ListJob *>( j );

  KURL oldUrl = job->url();  // here we really need the old url!
  KURL newUrl = url;

  // strip trailing slashes
  oldUrl.adjustPath(-1);
  newUrl.adjustPath(-1);

  if ( oldUrl == newUrl )
  {
    kdDebug(7004) << k_funcinfo << "New redirection url same as old, giving up." << endl;
    return;
  }

  kdDebug(7004) << k_funcinfo << oldUrl.prettyURL() << " -> " << newUrl.prettyURL() << endl;

#ifdef DEBUG_CACHE
  printDebug();
#endif

  // I don't think there can be dirItems that are childs of oldUrl.
  // Am I wrong here? And even if so, we don't need to delete them, right?
  // DF: redirection happens before listDir emits any item. Makes little sense otherwise.

  // oldUrl cannot be in itemsCached because only completed items are moved there
  DirItem *dir = itemsInUse.take( oldUrl.url() + ":" + oldUrl.internalReferenceURL() );
  Q_ASSERT( dir );

  TQPtrList<KDirLister> *listers = urlsCurrentlyListed.take( oldUrl.url() + ":" + oldUrl.internalReferenceURL() );
  Q_ASSERT( listers );
  Q_ASSERT( !listers->isEmpty() );

  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
  {
    // TODO: put in own method?
    if ( kdl->d->url.equals( oldUrl, true ) )
    {
      kdl->d->rootFileItem = 0;
      kdl->d->url = newUrl;
    }

    *kdl->d->lstDirs.find( oldUrl ) = newUrl;

    if ( kdl->d->lstDirs.count() == 1 )
    {
      emit kdl->clear();
      emit kdl->redirection( newUrl );
      emit kdl->redirection( oldUrl, newUrl );
    }
    else
    {
      emit kdl->clear( oldUrl );
      emit kdl->redirection( oldUrl, newUrl );
    }
  }

  // when a lister was stopped before the job emits the redirection signal, the old url will
  // also be in urlsCurrentlyHeld
  TQPtrList<KDirLister> *holders = urlsCurrentlyHeld.take( oldUrl.url() + ":" + oldUrl.internalReferenceURL() );
  if ( holders )
  {
    Q_ASSERT( !holders->isEmpty() );

    for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
    {
      kdl->jobStarted( job );
      
      // do it like when starting a new list-job that will redirect later
      emit kdl->started( oldUrl );

      // TODO: maybe don't emit started if there's an update running for newUrl already?

      if ( kdl->d->url.equals( oldUrl, true ) )
      {
        kdl->d->rootFileItem = 0;
        kdl->d->url = newUrl;
      }

      *kdl->d->lstDirs.find( oldUrl ) = newUrl;

      if ( kdl->d->lstDirs.count() == 1 )
      {
        emit kdl->clear();
        emit kdl->redirection( newUrl );
        emit kdl->redirection( oldUrl, newUrl );
      }
      else
      {
        emit kdl->clear( oldUrl );
        emit kdl->redirection( oldUrl, newUrl );
      }
    }
  }

  DirItem *newDir = itemsInUse[newUrl.url() + ":" + newUrl.internalReferenceURL()];
  if ( newDir )
  {
    kdDebug(7004) << "slotRedirection: " << newUrl.url() << " already in use" << endl;
    
    // only in this case there can newUrl already be in urlsCurrentlyListed or urlsCurrentlyHeld
    delete dir;

    // get the job if one's running for newUrl already (can be a list-job or an update-job), but
    // do not return this 'job', which would happen because of the use of redirectionURL()
    TDEIO::ListJob *oldJob = jobForUrl( newUrl.url() + ":" + newUrl.internalReferenceURL(), job );

    // listers of newUrl with oldJob: forget about the oldJob and use the already running one
    // which will be converted to an updateJob
    TQPtrList<KDirLister> *curListers = urlsCurrentlyListed[newUrl.url() + ":" + newUrl.internalReferenceURL()];
    if ( curListers )
    {
      kdDebug(7004) << "slotRedirection: and it is currently listed" << endl;

      Q_ASSERT( oldJob );  // ?!

      for ( KDirLister *kdl = curListers->first(); kdl; kdl = curListers->next() )  // listers of newUrl
      {
        kdl->jobDone( oldJob );

        kdl->jobStarted( job );
        kdl->connectJob( job );
      }

      // append listers of oldUrl with newJob to listers of newUrl with oldJob
      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        curListers->append( kdl );
    }
    else
      urlsCurrentlyListed.insert( newUrl.url() + ":" + newUrl.internalReferenceURL(), listers );

    if ( oldJob )         // kill the old job, be it a list-job or an update-job
      killJob( oldJob );

    // holders of newUrl: use the already running job which will be converted to an updateJob
    TQPtrList<KDirLister> *curHolders = urlsCurrentlyHeld[newUrl.url() + ":" + newUrl.internalReferenceURL()];
    if ( curHolders )
    {
      kdDebug(7004) << "slotRedirection: and it is currently held." << endl;

      for ( KDirLister *kdl = curHolders->first(); kdl; kdl = curHolders->next() )  // holders of newUrl
      {
        kdl->jobStarted( job );
        emit kdl->started( newUrl );
      }

      // append holders of oldUrl to holders of newUrl
      if ( holders )
        for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
          curHolders->append( kdl );
    }
    else if ( holders )
      urlsCurrentlyHeld.insert( newUrl.url() + ":" + newUrl.internalReferenceURL(), holders );

    
    // emit old items: listers, holders. NOT: newUrlListers/newUrlHolders, they already have them listed
    // TODO: make this a separate method?
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      if ( !kdl->d->rootFileItem && kdl->d->url == newUrl )
        kdl->d->rootFileItem = newDir->rootItem;

      kdl->addNewItems( *(newDir->lstItems) );
      kdl->emitItems();
    }

    if ( holders )
    {
      for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
      {
        if ( !kdl->d->rootFileItem && kdl->d->url == newUrl )
          kdl->d->rootFileItem = newDir->rootItem;

        kdl->addNewItems( *(newDir->lstItems) );
        kdl->emitItems();
      }
    }
  }
  else if ( (newDir = itemsCached.take( newUrl.url() )) )
  {
    kdDebug(7004) << "slotRedirection: " << newUrl.url() << " is unused, but already in the cache." << endl;

    delete dir;
    itemsInUse.insert( newUrl.url() + ":" + newUrl.internalReferenceURL(), newDir );
    urlsCurrentlyListed.insert( newUrl.url() + ":" + newUrl.internalReferenceURL(), listers );
    if ( holders )
      urlsCurrentlyHeld.insert( newUrl.url() + ":" + newUrl.internalReferenceURL(), holders );

    // emit old items: listers, holders
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      if ( !kdl->d->rootFileItem && kdl->d->url == newUrl )
        kdl->d->rootFileItem = newDir->rootItem;

      kdl->addNewItems( *(newDir->lstItems) );
      kdl->emitItems();
    }

    if ( holders )
    {
      for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
      {
        if ( !kdl->d->rootFileItem && kdl->d->url == newUrl )
          kdl->d->rootFileItem = newDir->rootItem;

        kdl->addNewItems( *(newDir->lstItems) );
        kdl->emitItems();
      }
    }
  }
  else
  {
    kdDebug(7004) << "slotRedirection: " << newUrl.url() << " has not been listed yet." << endl;

    delete dir->rootItem;
    dir->rootItem = 0;
    dir->lstItems->clear();
    dir->redirect( newUrl );
    itemsInUse.insert( newUrl.url() + ":" + newUrl.internalReferenceURL(), dir );
    urlsCurrentlyListed.insert( newUrl.url() + ":" + newUrl.internalReferenceURL(), listers );

    if ( holders )
      urlsCurrentlyHeld.insert( newUrl.url() + ":" + newUrl.internalReferenceURL(), holders );
    else
    {
#ifdef DEBUG_CACHE
      printDebug();
#endif
      return; // only in this case the job doesn't need to be converted, 
    }
  }

  // make the job an update job
  job->disconnect( this );
    
  connect( job, TQT_SIGNAL(entries( TDEIO::Job *, const TDEIO::UDSEntryList & )),
           this, TQT_SLOT(slotUpdateEntries( TDEIO::Job *, const TDEIO::UDSEntryList & )) );
  connect( job, TQT_SIGNAL(result( TDEIO::Job * )),
           this, TQT_SLOT(slotUpdateResult( TDEIO::Job * )) );

  // FIXME: autoUpdate-Counts!!

#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::renameDir( const KURL &oldUrl, const KURL &newUrl )
{
  kdDebug(7004) << k_funcinfo << oldUrl.prettyURL() << " -> " << newUrl.prettyURL() << endl;
  TQString oldUrlStr = oldUrl.url(-1);
  TQString newUrlStr = newUrl.url(-1);

  // Not enough. Also need to look at any child dir, even sub-sub-sub-dir.
  //DirItem *dir = itemsInUse.take( oldUrlStr );
  //emitRedirections( oldUrl, url );

  // Look at all dirs being listed/shown
  TQDictIterator<DirItem> itu( itemsInUse );
  bool goNext;
  while ( itu.current() )
  {
    goNext = true;
    DirItem *dir = itu.current();
    TQString oldDirURLIndep = itu.currentKey();
    oldDirURLIndep.truncate(oldDirURLIndep.length() - (dir->url.internalReferenceURL().length()+strlen(":")));
    KURL oldDirUrl ( oldDirURLIndep );
    //kdDebug(7004) << "itemInUse: " << oldDirUrl.prettyURL() << endl;
    // Check if this dir is oldUrl, or a subfolder of it
    if ( oldUrl.isParentOf( oldDirUrl ) )
    {
      // TODO should use KURL::cleanpath like isParentOf does
      TQString relPath = oldDirUrl.path().mid( oldUrl.path().length() );

      KURL newDirUrl( newUrl ); // take new base
      if ( !relPath.isEmpty() ) {
        newDirUrl.addPath( relPath ); // add unchanged relative path
      }
      //kdDebug(7004) << "KDirListerCache::renameDir new url=" << newDirUrl.prettyURL() << endl;

      // Update URL in dir item and in itemsInUse
      dir->redirect( newDirUrl );
      itemsInUse.remove( itu.currentKey() ); // implies ++itu
      itemsInUse.insert( newDirUrl.url(-1), dir );
      goNext = false; // because of the implied ++itu above
      if ( dir->lstItems )
      {
        // Rename all items under that dir
        KFileItemListIterator kit( *dir->lstItems );
        for ( ; kit.current(); ++kit )
        {
          KURL oldItemUrl = (*kit)->url();
          TQString oldItemUrlStr( oldItemUrl.url(-1) );
          KURL newItemUrl( oldItemUrl );
          newItemUrl.setPath( newDirUrl.path() );
          newItemUrl.addPath( oldItemUrl.fileName() );
          kdDebug(7004) << "KDirListerCache::renameDir renaming " << oldItemUrlStr << " to " << newItemUrl.url() << endl;
          (*kit)->setURL( newItemUrl );
        }
      }
      emitRedirections( oldDirUrl, newDirUrl );
    }
    if ( goNext )
      ++itu;
  }

  // Is oldUrl a directory in the cache?
  // Remove any child of oldUrl from the cache - even if the renamed dir itself isn't in it!
  removeDirFromCache( oldUrl );
  // TODO rename, instead.
}

void KDirListerCache::emitRedirections( const KURL &oldUrl, const KURL &url )
{
  kdDebug(7004) << k_funcinfo << oldUrl.prettyURL() << " -> " << url.prettyURL() << endl;
  TQString oldUrlStr = oldUrl.url(-1);
  TQString urlStr = url.url(-1);
  TQString oldReferenceUrlStr = oldUrl.internalReferenceURL();
  TQString urlReferenceStr = url.internalReferenceURL();

  TDEIO::ListJob *job = jobForUrl( oldUrlStr + ":" + oldReferenceUrlStr );
  if ( job )
    killJob( job );

  // Check if we were listing this dir. Need to abort and restart with new name in that case.
  TQPtrList<KDirLister> *listers = urlsCurrentlyListed.take( oldUrlStr + ":" + oldReferenceUrlStr );
  if ( listers )
  {
    // Tell the world that the job listing the old url is dead.
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      if ( job )
        kdl->jobDone( job );

      emit kdl->canceled( oldUrl );
    }

    urlsCurrentlyListed.insert( urlStr + ":" + urlReferenceStr, listers );
  }

  // Check if we are currently displaying this directory (odds opposite wrt above)
  // Update urlsCurrentlyHeld dict with new URL
  TQPtrList<KDirLister> *holders = urlsCurrentlyHeld.take( oldUrlStr + ":" + oldReferenceUrlStr );
  if ( holders )
  {
    if ( job )
      for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
        kdl->jobDone( job );

    urlsCurrentlyHeld.insert( urlStr + ":" + urlReferenceStr, holders );
  }

  if ( listers )
  {
    updateDirectory( url );

    // Tell the world about the new url
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
      emit kdl->started( url );
  }

  if ( holders )
  {
    // And notify the dirlisters of the redirection
    for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
    {
      *kdl->d->lstDirs.find( oldUrl ) = url;

      if ( kdl->d->lstDirs.count() == 1 )
        emit kdl->redirection( url );

      emit kdl->redirection( oldUrl, url );
    }
  }
}

void KDirListerCache::removeDirFromCache( const KURL& dir )
{
  kdDebug(7004) << "KDirListerCache::removeDirFromCache " << dir.prettyURL() << endl;
  TQCacheIterator<DirItem> itc( itemsCached );
  while ( itc.current() )
  {
    if ( dir.isParentOf( KURL( itc.currentKey() ) ) )
      itemsCached.remove( itc.currentKey() );
    else
      ++itc;
  }
}

void KDirListerCache::slotUpdateEntries( TDEIO::Job* job, const TDEIO::UDSEntryList& list )
{
  jobs[static_cast<TDEIO::ListJob*>(job)] += list;
}

void KDirListerCache::slotUpdateResult( TDEIO::Job * j )
{
  Q_ASSERT( j );
  TDEIO::ListJob *job = static_cast<TDEIO::ListJob *>( j );

  KURL jobUrl = joburl( job );
  jobUrl.adjustPath(-1);  // need remove trailing slashes again, in case of redirections
  TQString jobUrlStr = jobUrl.url();
  TQString jobReferenceUrlStr = jobUrl.internalReferenceURL();

  kdDebug(7004) << k_funcinfo << "finished update " << jobUrl << endl;

  KDirLister *kdl;

  TQPtrList<KDirLister> *listers = urlsCurrentlyHeld[jobUrlStr + ":" + jobReferenceUrlStr];
  TQPtrList<KDirLister> *tmpLst = urlsCurrentlyListed.take( jobUrlStr + ":" + jobReferenceUrlStr );

  if ( tmpLst )
  {
    if ( listers )
      for ( kdl = tmpLst->first(); kdl; kdl = tmpLst->next() )
      {
        Q_ASSERT( listers->containsRef( kdl ) == 0 );
        listers->append( kdl );
      }
    else
    {
      listers = tmpLst;
      urlsCurrentlyHeld.insert( jobUrlStr + ":" + jobReferenceUrlStr, listers );
    }
  }

  // once we are updating dirs that are only in the cache this will fail!
  Q_ASSERT( listers );

  if ( job->error() )
  {
    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->jobDone( job );

      //don't bother the user
      //kdl->handleError( job );

      emit kdl->canceled( jobUrl );
      if ( kdl->numJobs() == 0 )
      {
        kdl->d->complete = true;
        emit kdl->canceled();
      }
    }

    jobs.remove( job );

    // TODO: if job is a parent of one or more
    // of the pending urls we should cancel them
    processPendingUpdates();
    return;
  }

  DirItem *dir = itemsInUse[jobUrlStr + ":" + jobReferenceUrlStr];
  dir->complete = true;


  // check if anyone wants the mimetypes immediately
  bool delayedMimeTypes = true;
  for ( kdl = listers->first(); kdl; kdl = listers->next() ) {
    delayedMimeTypes = delayedMimeTypes && kdl->d->delayedMimeTypes;
  }

  // should be enough to get reasonable speed in most cases
  TQDict<KFileItem> fileItems( 9973 );

  KFileItemListIterator kit ( *(dir->lstItems) );

  // Unmark all items in url
  for ( ; kit.current(); ++kit )
  {
    (*kit)->unmark();
    if (!((*kit)->listerURL().isEmpty())) {
      fileItems.insert( (*kit)->listerURL().url(), *kit );
    }
    else {
      fileItems.insert( (*kit)->url().url(), *kit );
    }
  }

  static const TQString& dot = TDEGlobal::staticQString(".");
  static const TQString& dotdot = TDEGlobal::staticQString("..");

  KFileItem *item = 0, *tmp;

  TQValueList<TDEIO::UDSEntry> buf = jobs[job];
  TQValueListIterator<TDEIO::UDSEntry> it = buf.begin();
  for ( ; it != buf.end(); ++it )
  {
    // Form the complete url
    if ( !item ) {
      item = new KFileItem( *it, jobUrl, delayedMimeTypes, true );
    }
    else {
      item->setUDSEntry( *it, jobUrl, delayedMimeTypes, true );
    }

    // Find out about the name
    TQString name = item->name();
    Q_ASSERT( !name.isEmpty() );

    // we duplicate the check for dotdot here, to avoid iterating over
    // all items again and checking in matchesFilter() that way.
    if ( name.isEmpty() || name == dotdot ) {
      continue;
    }

    if ( name == dot )
    {
      // if the update was started before finishing the original listing
      // there is no root item yet
      if ( !dir->rootItem )
      {
        dir->rootItem = item;
        item = 0;

        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
          if ( !kdl->d->rootFileItem && kdl->d->url == jobUrl ) {
            kdl->d->rootFileItem = dir->rootItem;
          }
        }
      }

      continue;
    }

    // Find this item
    if ( (tmp = fileItems[item->url().url()]) )
    {
      tmp->mark();

      // check if something changed for this file
      if ( !tmp->cmp( *item ) )
      {
        for ( kdl = listers->first(); kdl; kdl = listers->next() ) {
          kdl->aboutToRefreshItem( tmp );
        }

        //kdDebug(7004) << "slotUpdateResult: file changed: " << tmp->name() << endl;
        tmp->assign( *item );

        for ( kdl = listers->first(); kdl; kdl = listers->next() ) {
          kdl->addRefreshItem( tmp );
        }
      }
    }
    else // this is a new file
    {
      //kdDebug(7004) << "slotUpdateResult: new file: " << name << endl;

      item->mark();
      dir->lstItems->append( item );

      for ( kdl = listers->first(); kdl; kdl = listers->next() ) {
        kdl->addNewItem( item );
      }

      // item used, we need a new one for the next iteration
      item = 0;
    }
  }

  if ( item ) {
    delete item;
  }

  jobs.remove( job );

  deleteUnmarkedItems( listers, dir->lstItems );

  for ( kdl = listers->first(); kdl; kdl = listers->next() )
  {
    kdl->emitItems();

    kdl->jobDone( job );

    kdl->emitCompleted( jobUrl );
    if ( kdl->numJobs() == 0 )
    {
      kdl->d->complete = true;
      emit kdl->completed();
    }
  }

  // TODO: hmm, if there was an error and job is a parent of one or more
  // of the pending urls we should cancel it/them as well
  processPendingUpdates();
}

// private

TDEIO::ListJob *KDirListerCache::jobForUrl( const TQString& url, TDEIO::ListJob *not_job )
{
  TDEIO::ListJob *job;
  TQMap< TDEIO::ListJob *, TQValueList<TDEIO::UDSEntry> >::Iterator it = jobs.begin();
  while ( it != jobs.end() )
  {
    job = it.key();
    if ( joburl( job ).url(-1) == url && job != not_job )
       return job;
    ++it;
  }
  return 0;
}

const KURL& KDirListerCache::joburl( TDEIO::ListJob *job )
{
  if ( job->redirectionURL().isValid() ) {
     return job->redirectionURL();
  }
  else {
     return job->url();
  }
}

void KDirListerCache::killJob( TDEIO::ListJob *job )
{
  jobs.remove( job );
  job->disconnect( this );
  job->kill();
}

void KDirListerCache::deleteUnmarkedItems( TQPtrList<KDirLister> *listers, KFileItemList *lstItems )
{
  // Find all unmarked items and delete them
  KFileItem* item;
  lstItems->first();
  while ( (item = lstItems->current()) )
    if ( !item->isMarked() )
    {
      //kdDebug() << k_funcinfo << item->name() << endl;
      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() ) {
        kdl->emitDeleteItem( item );
      }

      if ( item->isDir() ) {
        deleteDir( item->url() );
      }

      // finally actually delete the item
      lstItems->take();
      delete item;
    }
    else
      lstItems->next();
}

void KDirListerCache::deleteDir( const KURL& dirUrl )
{
  //kdDebug() << k_funcinfo << dirUrl.prettyURL() << endl;
  // unregister and remove the childs of the deleted item.
  // Idea: tell all the KDirListers that they should forget the dir
  //       and then remove it from the cache.

  TQDictIterator<DirItem> itu( itemsInUse );
  while ( itu.current() )
  {
    TQString deletedUrlIndep = itu.currentKey();
    deletedUrlIndep.truncate(deletedUrlIndep.length() - ((*itu)->url.internalReferenceURL().length()+strlen(":")));
    KURL deletedUrl( deletedUrlIndep );
    if ( dirUrl.isParentOf( deletedUrl ) )
    {
      // stop all jobs for deletedUrl

      TQPtrList<KDirLister> *kdls = urlsCurrentlyListed[deletedUrl.url() + ":" + deletedUrl.internalReferenceURL()];
      if ( kdls )  // yeah, I lack good names
      {
        // we need a copy because stop modifies the list
        kdls = new TQPtrList<KDirLister>( *kdls );
        for ( KDirLister *kdl = kdls->first(); kdl; kdl = kdls->next() )
          stop( kdl, deletedUrl );

        delete kdls;
      }

      // tell listers holding deletedUrl to forget about it
      // this will stop running updates for deletedUrl as well

      kdls = urlsCurrentlyHeld[deletedUrl.url() + ":" + deletedUrl.internalReferenceURL()];
      if ( kdls )
      {
        // we need a copy because forgetDirs modifies the list
        kdls = new TQPtrList<KDirLister>( *kdls );

        for ( KDirLister *kdl = kdls->first(); kdl; kdl = kdls->next() )
        {
          // lister's root is the deleted item
          if ( kdl->d->url == deletedUrl )
          {
            // tell the view first. It might need the subdirs' items (which forgetDirs will delete)
            if ( kdl->d->rootFileItem )
              emit kdl->deleteItem( kdl->d->rootFileItem );
            forgetDirs( kdl );
            kdl->d->rootFileItem = 0;
          }
          else
          {
            bool treeview = kdl->d->lstDirs.count() > 1;
            if ( !treeview )
              emit kdl->clear();

            forgetDirs( kdl, deletedUrl, treeview );
          }
        }

        delete kdls;
      }

      // delete the entry for deletedUrl - should not be needed, it's in
      // items cached now

      DirItem *dir = itemsInUse.take( deletedUrl.url() + ":" + deletedUrl.internalReferenceURL() );
      Q_ASSERT( !dir );
      if ( !dir ) // take didn't find it - move on
          ++itu;
    }
    else
      ++itu;
  }

  // remove the children from the cache
  removeDirFromCache( dirUrl );
}

void KDirListerCache::processPendingUpdates()
{
  // TODO
}

#ifndef NDEBUG
void KDirListerCache::printDebug()
{
  kdDebug(7004) << "Items in use: " << endl;
  TQDictIterator<DirItem> itu( itemsInUse );
  for ( ; itu.current() ; ++itu ) {
      kdDebug(7004) << "   " << itu.currentKey() << "  URL: " << itu.current()->url
                    << " rootItem: " << ( itu.current()->rootItem ? itu.current()->rootItem->url() : KURL() )
                    << " autoUpdates refcount: " << itu.current()->autoUpdates
                    << " complete: " << itu.current()->complete
                  << ( itu.current()->lstItems ? TQString(" with %1 items.").arg(itu.current()->lstItems->count()) : TQString(" lstItems=NULL") ) << endl;
  }

  kdDebug(7004) << "urlsCurrentlyHeld: " << endl;
  TQDictIterator< TQPtrList<KDirLister> > it( urlsCurrentlyHeld );
  for ( ; it.current() ; ++it )
  {
    TQString list;
    for ( TQPtrListIterator<KDirLister> listit( *it.current() ); listit.current(); ++listit )
      list += " 0x" + TQString::number( (long)listit.current(), 16 );
    kdDebug(7004) << "   " << it.currentKey() << "  " << it.current()->count() << " listers: " << list << endl;
  }

  kdDebug(7004) << "urlsCurrentlyListed: " << endl;
  TQDictIterator< TQPtrList<KDirLister> > it2( urlsCurrentlyListed );
  for ( ; it2.current() ; ++it2 )
  {
    TQString list;
    for ( TQPtrListIterator<KDirLister> listit( *it2.current() ); listit.current(); ++listit )
      list += " 0x" + TQString::number( (long)listit.current(), 16 );
    kdDebug(7004) << "   " << it2.currentKey() << "  " << it2.current()->count() << " listers: " << list << endl;
  }

  TQMap< TDEIO::ListJob *, TQValueList<TDEIO::UDSEntry> >::Iterator jit = jobs.begin();
  kdDebug(7004) << "Jobs: " << endl;
  for ( ; jit != jobs.end() ; ++jit )
    kdDebug(7004) << "   " << jit.key() << " listing " << joburl( jit.key() ).prettyURL() << ": " << (*jit).count() << " entries." << endl;

  kdDebug(7004) << "Items in cache: " << endl;
  TQCacheIterator<DirItem> itc( itemsCached );
  for ( ; itc.current() ; ++itc )
    kdDebug(7004) << "   " << itc.currentKey() << "  rootItem: "
                  << ( itc.current()->rootItem ? itc.current()->rootItem->url().prettyURL() : TQString("NULL") )
                  << ( itc.current()->lstItems ? TQString(" with %1 items.").arg(itc.current()->lstItems->count()) : TQString(" lstItems=NULL") ) << endl;
}
#endif

/*********************** -- The new KDirLister -- ************************/


KDirLister::KDirLister( bool _delayedMimeTypes )
{
  kdDebug(7003) << "+KDirLister" << endl;

  d = new KDirListerPrivate;

  d->complete = true;
  d->delayedMimeTypes = _delayedMimeTypes;

  setAutoUpdate( true );
  setDirOnlyMode( false );
  setShowingDotFiles( false );

  setAutoErrorHandlingEnabled( true, 0 );
}

KDirLister::~KDirLister()
{
  kdDebug(7003) << "-KDirLister" << endl;

  if ( KDirListerCache::exists() )
  {
    // Stop all running jobs
    stop();
    s_pCache->forgetDirs( this );
  }

  delete d;
}

bool KDirLister::openURL( const KURL& _url, bool _keep, bool _reload )
{
  kdDebug(7003) << k_funcinfo << _url.prettyURL()
                << " keep=" << _keep << " reload=" << _reload << endl;

  // emit the current changes made to avoid an inconsistent treeview
  if ( d->changes != NONE && _keep ) {
    emitChanges();
  }

  d->changes = NONE;

  // If a local path is available, monitor that instead of the given remote URL...
  if (!_url.isLocalFile()) {
      TDEIO::LocalURLJob* localURLJob = TDEIO::localURL(_url);
      if (localURLJob) {
          d->openURL_url[localURLJob] = _url;
          d->openURL_keep[localURLJob] = _keep;
          d->openURL_reload[localURLJob] = _reload;
          connect(localURLJob, TQT_SIGNAL(localURL(TDEIO::Job*, const KURL&, bool)), this, TQT_SLOT(slotOpenURLGotLocalURL(TDEIO::Job*, const KURL&, bool)));
          connect(localURLJob, TQT_SIGNAL(destroyed()), this, TQT_SLOT(slotLocalURLKIODestroyed()));
      }
      return true;
  }
  else {
      return s_pCache->listDir( this, _url, _keep, _reload );
  }
}

void KDirLister::slotOpenURLGotLocalURL(TDEIO::Job *job, const KURL& url, bool isLocal) {
  KURL realURL = d->openURL_url[job];
  if (isLocal) {
      realURL = url;
      realURL.setInternalReferenceURL(d->openURL_url[job].url());
      d->m_referenceURLMap[d->openURL_url[job].url()] = url.path();
  }
  s_pCache->listDir( this, realURL, d->openURL_keep[job], d->openURL_reload[job] );
  d->openURL_url.remove(job);
  d->openURL_keep.remove(job);
  d->openURL_reload.remove(job);
}

void KDirLister::slotLocalURLKIODestroyed() {
  TDEIO::LocalURLJob* terminatedJob = const_cast<TDEIO::LocalURLJob*>(static_cast<const TDEIO::LocalURLJob*>(sender()));

  if (d->openURL_url.contains(terminatedJob)) {
      s_pCache->listDir( this, d->openURL_url[terminatedJob], d->openURL_keep[terminatedJob], d->openURL_reload[terminatedJob] );
      d->openURL_url.remove(terminatedJob);
      d->openURL_keep.remove(terminatedJob);
      d->openURL_reload.remove(terminatedJob);
  }
}

void KDirLister::stop()
{
  kdDebug(7003) << k_funcinfo << endl;
  s_pCache->stop( this );
  d->m_referenceURLMap.clear();
}

void KDirLister::stop( const KURL& _url )
{
  kdDebug(7003) << k_funcinfo << _url.prettyURL() << endl;
  s_pCache->stop( this, _url );
  d->m_referenceURLMap.remove(_url.url());
}

bool KDirLister::autoUpdate() const
{
  return d->autoUpdate;
}

void KDirLister::setAutoUpdate( bool _enable )
{
  if ( d->autoUpdate == _enable )
    return;

  d->autoUpdate = _enable;
  s_pCache->setAutoUpdate( this, _enable );
}

bool KDirLister::showingDotFiles() const
{
  return d->isShowingDotFiles;
}

void KDirLister::setShowingDotFiles( bool _showDotFiles )
{
  if ( d->isShowingDotFiles == _showDotFiles )
    return;

  d->isShowingDotFiles = _showDotFiles;
  d->changes ^= DOT_FILES;
}

bool KDirLister::dirOnlyMode() const
{
  return d->dirOnlyMode;
}

void KDirLister::setDirOnlyMode( bool _dirsOnly )
{
  if ( d->dirOnlyMode == _dirsOnly )
    return;

  d->dirOnlyMode = _dirsOnly;
  d->changes ^= DIR_ONLY_MODE;
}

bool KDirLister::autoErrorHandlingEnabled() const
{
  return d->autoErrorHandling;
}

void KDirLister::setAutoErrorHandlingEnabled( bool enable, TQWidget* parent )
{
  d->autoErrorHandling = enable;
  d->errorParent = parent;
}

const KURL& KDirLister::url() const
{
  return d->url;
}

const KURL::List& KDirLister::directories() const
{
  return d->lstDirs;
}

void KDirLister::emitChanges()
{
  if ( d->changes == NONE )
    return;

  static const TQString& dot = TDEGlobal::staticQString(".");
  static const TQString& dotdot = TDEGlobal::staticQString("..");

  for ( KURL::List::Iterator it = d->lstDirs.begin();
        it != d->lstDirs.end(); ++it )
  {
    KFileItemListIterator kit( *s_pCache->itemsForDir( *it ) );
    for ( ; kit.current(); ++kit )
    {
      if ( (*kit)->text() == dot || (*kit)->text() == dotdot )
        continue;

      bool oldMime = true, newMime = true;

      if ( d->changes & MIME_FILTER )
      {
        oldMime = doMimeFilter( (*kit)->mimetype(), d->oldMimeFilter )
                && doMimeExcludeFilter( (*kit)->mimetype(), d->oldMimeExcludeFilter );
        newMime = doMimeFilter( (*kit)->mimetype(), d->mimeFilter )
                && doMimeExcludeFilter( (*kit)->mimetype(), d->mimeExcludeFilter );

        if ( oldMime && !newMime )
        {
          emit deleteItem( *kit );
          continue;
        }
      }

      if ( d->changes & DIR_ONLY_MODE )
      {
        // the lister switched to dirOnlyMode
        if ( d->dirOnlyMode )
        {
          if ( !(*kit)->isDir() )
            emit deleteItem( *kit );
        }
        else if ( !(*kit)->isDir() ) {
          addNewItem( *kit );
        }

        continue;
      }

      if ( (*kit)->isHidden() )
      {
        if ( d->changes & DOT_FILES )
        {
          // the lister switched to dot files mode
          if ( d->isShowingDotFiles ) {
            addNewItem( *kit );
          }
          else {
            emit deleteItem( *kit );
          }

          continue;
        }
      }
      else if ( d->changes & NAME_FILTER )
      {
        bool oldName = (*kit)->isDir() ||
                       d->oldFilters.isEmpty() ||
                       doNameFilter( (*kit)->text(), d->oldFilters );

        bool newName = (*kit)->isDir() ||
                       d->lstFilters.isEmpty() ||
                       doNameFilter( (*kit)->text(), d->lstFilters );

        if ( oldName && !newName )
        {
          emit deleteItem( *kit );
          continue;
        }
        else if ( !oldName && newName ) {
          addNewItem( *kit );
        }
      }

      if ( (d->changes & MIME_FILTER) && !oldMime && newMime ) {
        addNewItem( *kit );
      }
    }

    emitItems();
  }

  d->changes = NONE;
}

void KDirLister::updateDirectory( const KURL& _u )
{
  s_pCache->updateDirectory( _u );
}

bool KDirLister::isFinished() const
{
  return d->complete;
}

KFileItem *KDirLister::rootItem() const
{
  return d->rootFileItem;
}

KFileItem *KDirLister::findByURL( const KURL& _url ) const
{
  return s_pCache->findByURL( this, _url );
}

KFileItem *KDirLister::findByName( const TQString& _name ) const
{
  return s_pCache->findByName( this, _name );
}

#ifndef KDE_NO_COMPAT
KFileItem *KDirLister::find( const KURL& _url ) const
{
  return findByURL( _url );
}
#endif


// ================ public filter methods ================ //

void KDirLister::setNameFilter( const TQString& nameFilter )
{
  if ( !(d->changes & NAME_FILTER) )
  {
    d->oldFilters = d->lstFilters;
    d->lstFilters.setAutoDelete( false );
  }

  d->lstFilters.clear();
  d->lstFilters.setAutoDelete( true );

  d->nameFilter = nameFilter;

  // Split on white space
  TQStringList list = TQStringList::split( ' ', nameFilter );
  for ( TQStringList::Iterator it = list.begin(); it != list.end(); ++it )
    d->lstFilters.append( new TQRegExp(*it, false, true ) );

  d->changes |= NAME_FILTER;
}

const TQString& KDirLister::nameFilter() const
{
  return d->nameFilter;
}

void KDirLister::setMimeFilter( const TQStringList& mimeFilter )
{
  if ( !(d->changes & MIME_FILTER) )
    d->oldMimeFilter = d->mimeFilter;

  if ( mimeFilter.find("all/allfiles") != mimeFilter.end() || 
       mimeFilter.find("all/all") != mimeFilter.end() )
    d->mimeFilter.clear();
  else
    d->mimeFilter = mimeFilter;

  d->changes |= MIME_FILTER;
}

void KDirLister::setMimeExcludeFilter( const TQStringList& mimeExcludeFilter )
{
  if ( !(d->changes & MIME_FILTER) )
    d->oldMimeExcludeFilter = d->mimeExcludeFilter;

  d->mimeExcludeFilter = mimeExcludeFilter;
  d->changes |= MIME_FILTER;
}


void KDirLister::clearMimeFilter()
{
  if ( !(d->changes & MIME_FILTER) )
  {
    d->oldMimeFilter = d->mimeFilter;
    d->oldMimeExcludeFilter = d->mimeExcludeFilter;
  }
  d->mimeFilter.clear();
  d->mimeExcludeFilter.clear();
  d->changes |= MIME_FILTER;
}

const TQStringList& KDirLister::mimeFilters() const
{
  return d->mimeFilter;
}

bool KDirLister::matchesFilter( const TQString& name ) const
{
  return doNameFilter( name, d->lstFilters );
}

bool KDirLister::matchesMimeFilter( const TQString& mime ) const
{
  return doMimeFilter( mime, d->mimeFilter ) && doMimeExcludeFilter(mime,d->mimeExcludeFilter);
}

// ================ protected methods ================ //

bool KDirLister::matchesFilter( const KFileItem *item ) const
{
  Q_ASSERT( item );
  static const TQString& dotdot = TDEGlobal::staticQString("..");

  if ( item->text() == dotdot )
    return false;

  if ( !d->isShowingDotFiles && item->isHidden() )
    return false;

  if ( item->isDir() || d->lstFilters.isEmpty() )
    return true;

  return matchesFilter( item->text() );
}

bool KDirLister::matchesMimeFilter( const KFileItem *item ) const
{
  Q_ASSERT( item );
  // Don't lose time determining the mimetype if there is no filter
  if ( d->mimeFilter.isEmpty() && d->mimeExcludeFilter.isEmpty() )
      return true;
  return matchesMimeFilter( item->mimetype() );
}

bool KDirLister::doNameFilter( const TQString& name, const TQPtrList<TQRegExp>& filters ) const
{
  for ( TQPtrListIterator<TQRegExp> it( filters ); it.current(); ++it )
    if ( it.current()->exactMatch( name ) )
      return true;

  return false;
}

bool KDirLister::doMimeFilter( const TQString& mime, const TQStringList& filters ) const
{
  if ( filters.isEmpty() )
    return true;

  KMimeType::Ptr mimeptr = KMimeType::mimeType(mime);
  //kdDebug(7004) << "doMimeFilter: investigating: "<<mimeptr->name()<<endl;
  TQStringList::ConstIterator it = filters.begin();
  for ( ; it != filters.end(); ++it )
    if ( mimeptr->is(*it) )
      return true;
    //else   kdDebug(7004) << "doMimeFilter: compared without result to  "<<*it<<endl;


  return false;
}

bool KDirLister::doMimeExcludeFilter( const TQString& mime, const TQStringList& filters ) const
{
  if ( filters.isEmpty() )
    return true;

  TQStringList::ConstIterator it = filters.begin();
  for ( ; it != filters.end(); ++it )
    if ( (*it) == mime )
      return false;

  return true;
}


bool KDirLister::validURL( const KURL& _url ) const
{
  return s_pCache->validURL( this, _url );
}

void KDirLister::handleError( TDEIO::Job *job )
{
  if ( d->autoErrorHandling ) {
    job->showErrorDialog( d->errorParent );
  }
}


// ================= private methods ================= //

void KDirLister::addNewItem( const KFileItem *item )
{
  if ( ( d->dirOnlyMode && !item->isDir() ) || !matchesFilter( item ) ) {
    return; // No reason to continue... bailing out here prevents a mimetype scan.
  }

  if ((item->url().internalReferenceURL() != "")
      && (d->m_referenceURLMap.contains(item->url().internalReferenceURL()))) {
    // Likely a media:/ tdeioslave URL or similar
    // Rewrite the URL to ensure that the user remains within the media:/ tree!
    TQString itemPath = item->url().path();
    if (itemPath.startsWith(d->m_referenceURLMap[item->url().internalReferenceURL()])) {
      itemPath = itemPath.remove(0, d->m_referenceURLMap[item->url().internalReferenceURL()].length());
      TQString newPath = item->url().internalReferenceURL();
      if (!newPath.endsWith("/")) newPath = newPath + "/";
      while (itemPath.startsWith("/")) itemPath = itemPath.remove(0,1);
      while (itemPath.endsWith("/")) itemPath.truncate(itemPath.length()-1);
      newPath = newPath + itemPath;
      const_cast<KFileItem*>(item)->setListerURL(item->url());
      const_cast<KFileItem*>(item)->setURL(newPath);
    }
  }

  if ( matchesMimeFilter( item ) )
  {
    if ( !d->lstNewItems ) {
      d->lstNewItems = new KFileItemList;
    }

    d->lstNewItems->append( item );            // items not filtered
  }
  else
  {
    if ( !d->lstMimeFilteredItems ) {
      d->lstMimeFilteredItems = new KFileItemList;
    }

    d->lstMimeFilteredItems->append( item );   // only filtered by mime
  }
}

void KDirLister::addNewItems( const KFileItemList& items )
{
  // TODO: make this faster - test if we have a filter at all first
  // DF: was this profiled? The matchesFoo() functions should be fast, w/o filters...
  // Of course if there is no filter and we can do a range-insertion instead of a loop, that might be good.
  // But that's for Qt4, not possible with TQPtrList.
  for ( KFileItemListIterator kit( items ); kit.current(); ++kit ) {
    addNewItem( *kit );
  }
}

void KDirLister::aboutToRefreshItem( const KFileItem *item )
{
  // The code here follows the logic in addNewItem
  if ( ( d->dirOnlyMode && !item->isDir() ) || !matchesFilter( item ) )
    d->refreshItemWasFiltered = true;
  else if ( !matchesMimeFilter( item ) )
    d->refreshItemWasFiltered = true;
  else
    d->refreshItemWasFiltered = false;
}

void KDirLister::addRefreshItem( const KFileItem *item )
{
  bool isExcluded = (d->dirOnlyMode && !item->isDir()) || !matchesFilter( item );

  if ((item->url().internalReferenceURL() != "")
      && (d->m_referenceURLMap.contains(item->url().internalReferenceURL()))) {
    // Likely a media:/ tdeioslave URL or similar
    // Rewrite the URL to ensure that the user remains within the media:/ tree!
    TQString itemPath = item->url().path();
    if (itemPath.startsWith(d->m_referenceURLMap[item->url().internalReferenceURL()])) {
      itemPath = itemPath.remove(0, d->m_referenceURLMap[item->url().internalReferenceURL()].length());
      TQString newPath = item->url().internalReferenceURL();
      if (!newPath.endsWith("/")) newPath = newPath + "/";
      while (itemPath.startsWith("/")) itemPath = itemPath.remove(0,1);
      while (itemPath.endsWith("/")) itemPath.truncate(itemPath.length()-1);
      newPath = newPath + itemPath;
      const_cast<KFileItem*>(item)->setListerURL(item->url());
      const_cast<KFileItem*>(item)->setURL(newPath);
    }
  }

  if ( !isExcluded && matchesMimeFilter( item ) )
  {
    if ( d->refreshItemWasFiltered )
    {
      if ( !d->lstNewItems ) {
        d->lstNewItems = new KFileItemList;
      }

      d->lstNewItems->append( item );
    }
    else
    {
      if ( !d->lstRefreshItems ) {
        d->lstRefreshItems = new KFileItemList;
      }

      d->lstRefreshItems->append( item );
    }
  }
  else if ( !d->refreshItemWasFiltered )
  {
    if ( !d->lstRemoveItems ) {
      d->lstRemoveItems = new KFileItemList;
    }

    // notify the user that the mimetype of a file changed that doesn't match
    // a filter or does match an exclude filter
    d->lstRemoveItems->append( item );
  }
}

void KDirLister::emitItems()
{
  KFileItemList *tmpNew = d->lstNewItems;
  d->lstNewItems = 0;

  KFileItemList *tmpMime = d->lstMimeFilteredItems;
  d->lstMimeFilteredItems = 0;

  KFileItemList *tmpRefresh = d->lstRefreshItems;
  d->lstRefreshItems = 0;

  KFileItemList *tmpRemove = d->lstRemoveItems;
  d->lstRemoveItems = 0;

  if ( tmpNew )
  {
    // For historical reasons, items with different protocols and/or prefixes must be emitted separately
    TQString protocol;
    TQString prefix;
    TQString prevProtocol;
    TQString prevPrefix;
    KFileItemList emitList;
    for ( KFileItemListIterator kit( *tmpNew ); kit.current(); ++kit )
    {
      KFileItem *item = *kit;
      protocol = item->url().protocol();
      prefix = TQStringList::split("/", item->url().path())[0];
      if ((protocol != prevProtocol) || (prefix != prevPrefix)) {
        if (emitList.count() > 0) {
          emit newItems( emitList );
          emitList.clear();
        }
      }
      emitList.append(item);
      prevProtocol = protocol;
      prevPrefix = prefix;
    }

    if (emitList.count() > 0) {
      emit newItems( emitList );
    }
    delete tmpNew;
  }

  if ( tmpMime )
  {
    emit itemsFilteredByMime( *tmpMime );
    delete tmpMime;
  }

  if ( tmpRefresh )
  {
    emit refreshItems( *tmpRefresh );
    delete tmpRefresh;
  }

  if ( tmpRemove )
  {
    for ( KFileItem *tmp = tmpRemove->first(); tmp; tmp = tmpRemove->next() ) {
      emit deleteItem( tmp );
    }
    delete tmpRemove;
  }
}

void KDirLister::emitDeleteItem( KFileItem *item )
{
  if ( ( d->dirOnlyMode && !item->isDir() ) || !matchesFilter( item ) ) {
    return; // No reason to continue... bailing out here prevents a mimetype scan.
  }

  if ( matchesMimeFilter( item ) ) {
    emit deleteItem( item );
  }
}


// ================ private slots ================ //

void KDirLister::slotInfoMessage( TDEIO::Job *, const TQString& message )
{
  emit infoMessage( message );
}

void KDirLister::slotPercent( TDEIO::Job *job, unsigned long pcnt )
{
  d->jobData[static_cast<TDEIO::ListJob *>(job)].percent = pcnt;

  int result = 0;

  TDEIO::filesize_t size = 0;

  TQMap< TDEIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).percent * (*dataIt).totalSize;
    size += (*dataIt).totalSize;
    ++dataIt;
  }

  if ( size != 0 )
    result /= size;
  else
    result = 100;
  emit percent( result );
}

void KDirLister::slotTotalSize( TDEIO::Job *job, TDEIO::filesize_t size )
{
  d->jobData[static_cast<TDEIO::ListJob *>(job)].totalSize = size;

  TDEIO::filesize_t result = 0;
  TQMap< TDEIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).totalSize;
    ++dataIt;
  }

  emit totalSize( result );
}

void KDirLister::slotProcessedSize( TDEIO::Job *job, TDEIO::filesize_t size )
{
  d->jobData[static_cast<TDEIO::ListJob *>(job)].processedSize = size;

  TDEIO::filesize_t result = 0;
  TQMap< TDEIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).processedSize;
    ++dataIt;
  }

  emit processedSize( result );
}

void KDirLister::slotSpeed( TDEIO::Job *job, unsigned long spd )
{
  d->jobData[static_cast<TDEIO::ListJob *>(job)].speed = spd;

  int result = 0;
  TQMap< TDEIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).speed;
    ++dataIt;
  }

  emit speed( result );
}

uint KDirLister::numJobs()
{
  return d->jobData.count();
}

void KDirLister::jobDone( TDEIO::ListJob *job )
{
  d->jobData.remove( job );
}

void KDirLister::jobStarted( TDEIO::ListJob *job )
{
  KDirListerPrivate::JobData jobData;
  jobData.speed = 0;
  jobData.percent = 0;
  jobData.processedSize = 0;
  jobData.totalSize = 0;

  d->jobData.insert( job, jobData );
  d->complete = false;
}

void KDirLister::connectJob( TDEIO::ListJob *job )
{
  connect( job, TQT_SIGNAL(infoMessage( TDEIO::Job *, const TQString& )),
           this, TQT_SLOT(slotInfoMessage( TDEIO::Job *, const TQString& )) );
  connect( job, TQT_SIGNAL(percent( TDEIO::Job *, unsigned long )),
           this, TQT_SLOT(slotPercent( TDEIO::Job *, unsigned long )) );
  connect( job, TQT_SIGNAL(totalSize( TDEIO::Job *, TDEIO::filesize_t )),
           this, TQT_SLOT(slotTotalSize( TDEIO::Job *, TDEIO::filesize_t )) );
  connect( job, TQT_SIGNAL(processedSize( TDEIO::Job *, TDEIO::filesize_t )),
           this, TQT_SLOT(slotProcessedSize( TDEIO::Job *, TDEIO::filesize_t )) );
  connect( job, TQT_SIGNAL(speed( TDEIO::Job *, unsigned long )),
           this, TQT_SLOT(slotSpeed( TDEIO::Job *, unsigned long )) );
}

void KDirLister::emitCompleted( const KURL& _url )
{
  KURL emitURL = _url;

  if ((_url.internalReferenceURL() != "")
      && (d->m_referenceURLMap.contains(_url.internalReferenceURL()))) {
    // Likely a media:/ tdeioslave URL or similar
    // Rewrite the URL to ensure that the user remains within the media:/ tree!
    TQString itemPath = _url.path();
    if (itemPath.startsWith(d->m_referenceURLMap[_url.internalReferenceURL()])) {
      itemPath = itemPath.remove(0, d->m_referenceURLMap[_url.internalReferenceURL()].length());
      TQString newPath = _url.internalReferenceURL();
      if (!newPath.endsWith("/")) newPath = newPath + "/";
      while (itemPath.startsWith("/")) itemPath = itemPath.remove(0,1);
      while (itemPath.endsWith("/")) itemPath.truncate(itemPath.length()-1);
      newPath = newPath + itemPath;
      emitURL = newPath;
    }
  }

  emit completed( emitURL );
}

void KDirLister::setMainWindow( TQWidget *window )
{
  d->window = window;
}

TQWidget *KDirLister::mainWindow()
{
  return d->window;
}

KFileItemList KDirLister::items( WhichItems which ) const
{
    return itemsForDir( url(), which );
}

KFileItemList KDirLister::itemsForDir( const KURL& dir, WhichItems which ) const
{
    KFileItemList result;
    KFileItemList *allItems = s_pCache->itemsForDir( dir );
    if ( !allItems ) {
        return result;
    }

    if ( which == AllItems ) {
        result = *allItems; // shallow copy
    }
    else // only items passing the filters
    {
        for ( KFileItemListIterator kit( *allItems ); kit.current(); ++kit )
        {
            KFileItem *item = *kit;
            bool isExcluded = (d->dirOnlyMode && !item->isDir()) || !matchesFilter( item );
            if ( !isExcluded && matchesMimeFilter( item ) ) {
                result.append( item );
            }
        }
    }

    return result;
}

// to keep BC changes

void KDirLister::virtual_hook( int, void * )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdirlister.moc"
#include "kdirlister_p.moc"
