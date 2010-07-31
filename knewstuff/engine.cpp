/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqcstring.h>
#include <tqdom.h>
#include <tqfileinfo.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "knewstuff.h"
#include "downloaddialog.h"
#include "uploaddialog.h"
#include "providerdialog.h"

#include "engine.h"
#include "engine.moc"

using namespace KNS;

struct Engine::Private
{
    bool mIgnoreInstallResult;
    KNewStuff *mNewStuff;
};

Engine::Engine( KNewStuff *newStuff, const TQString &type,
                TQWidget *parentWidget ) :
  mParentWidget( parentWidget ), mDownloadDialog( 0 ),
  mUploadDialog( 0 ), mProviderDialog( 0 ), mUploadProvider( 0 ),
  d(new Private), mType( type )
{
  d->mNewStuff = newStuff;
  d->mIgnoreInstallResult = false;
  mProviderLoader = new ProviderLoader( mParentWidget );
}

Engine::Engine( KNewStuff *newStuff, const TQString &type,
                const TQString &providerList, TQWidget *parentWidget ) :
                mParentWidget( parentWidget ),
		mDownloadDialog( 0 ), mUploadDialog( 0 ),
		mProviderDialog( 0 ), mUploadProvider( 0 ),
                mProviderList( providerList ), d(new Private),
		mType( type )
{
  d->mNewStuff = newStuff;
  d->mIgnoreInstallResult = false;
  mProviderLoader = new ProviderLoader( mParentWidget );
}

Engine::~Engine()
{
  delete d;
  delete mProviderLoader;

  delete mUploadDialog;
  delete mDownloadDialog;
}

void Engine::download()
{
  kdDebug() << "Engine::download()" << endl;

  connect( mProviderLoader,
           TQT_SIGNAL( providersLoaded( Provider::List * ) ),
           TQT_SLOT( getMetaInformation( Provider::List * ) ) );
  mProviderLoader->load( mType, mProviderList );
}

void Engine::getMetaInformation( Provider::List *providers )
{
  mProviderLoader->disconnect();

  mNewStuffJobData.clear();

  if ( !mDownloadDialog ) {
    mDownloadDialog = new DownloadDialog( this, mParentWidget );
    mDownloadDialog->show();
  }
  mDownloadDialog->clear();

  Provider *p;
  for ( p = providers->first(); p; p = providers->next() ) {
    if ( p->downloadUrl().isEmpty() ) continue;

    KIO::TransferJob *job = KIO::get( p->downloadUrl(), false, false );
    connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
             TQT_SLOT( slotNewStuffJobResult( KIO::Job * ) ) );
    connect( job, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
             TQT_SLOT( slotNewStuffJobData( KIO::Job *, const TQByteArray & ) ) );

    mNewStuffJobData.insert( job, "" );
    mProviderJobs[ job ] = p;
  }
}

void Engine::slotNewStuffJobData( KIO::Job *job, const TQByteArray &data )
{
  if ( data.isEmpty() ) return;

  kdDebug() << "Engine:slotNewStuffJobData()" << endl;

  TQCString str( data, data.size() + 1 );

  mNewStuffJobData[ job ].append( TQString::fromUtf8( str ) );
}

void Engine::slotNewStuffJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error downloading new stuff descriptions." << endl;
    job->showErrorDialog( mParentWidget );
  } else {
    TQString knewstuffDoc = mNewStuffJobData[ job ];

    kdDebug() << "---START---" << endl << knewstuffDoc << "---END---" << endl;

    mDownloadDialog->addProvider( mProviderJobs[ job ] );

    TQDomDocument doc;
    if ( !doc.setContent( knewstuffDoc ) ) {
      kdDebug() << "Error parsing knewstuff.xml." << endl;
      return;
    } else {
      TQDomElement knewstuff = doc.documentElement();

      if ( knewstuff.isNull() ) {
        kdDebug() << "No document in knewstuffproviders.xml." << endl;
      } else {
        TQDomNode p;
        for ( p = knewstuff.firstChild(); !p.isNull(); p = p.nextSibling() ) {
          TQDomElement stuff = p.toElement();
          if ( stuff.tagName() != "stuff" ) continue;
          if ( stuff.attribute("type", mType) != mType ) continue;

          Entry *entry = new Entry( stuff );

          mDownloadDialog->show();

          mDownloadDialog->addEntry( entry );

          kdDebug() << "KNEWSTUFF: " << entry->name() << endl;

          kdDebug() << "  SUMMARY: " << entry->summary() << endl;
          kdDebug() << "  VERSION: " << entry->version() << endl;
          kdDebug() << "  RELEASEDATE: " << entry->releaseDate().toString() << endl;
          kdDebug() << "  RATING: " << entry->rating() << endl;

          kdDebug() << "  LANGS: " << entry->langs().join(", ") << endl;
        }
      }
    }
  }

  mNewStuffJobData.remove( job );
  mProviderJobs.remove( job );

  if ( mNewStuffJobData.count() == 0 ) {
    mDownloadDialog->show();
    mDownloadDialog->raise();
  }
}

void Engine::download( Entry *entry )
{
  kdDebug() << "Engine::download(entry)" << endl;

  KURL source = entry->payload();
  mDownloadDestination = d->mNewStuff->downloadDestination( entry );

  if ( mDownloadDestination.isEmpty() ) {
    kdDebug() << "Empty downloadDestination. Cancelling download." << endl;
    return;
  }

  KURL destination = KURL( mDownloadDestination );

  kdDebug() << "  SOURCE: " << source.url() << endl;
  kdDebug() << "  DESTINATION: " << destination.url() << endl;

  KIO::FileCopyJob *job = KIO::file_copy( source, destination, -1, true );
  connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
           TQT_SLOT( slotDownloadJobResult( KIO::Job * ) ) );
}

void Engine::slotDownloadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error downloading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    return;
  }

  if ( d->mNewStuff->install( mDownloadDestination ) ) {
    if ( !d->mIgnoreInstallResult ) {
      KMessageBox::information( mParentWidget,
                                i18n("Successfully installed hot new stuff.") );
    }
  } else 
    if ( !d->mIgnoreInstallResult ){
      KMessageBox::error( mParentWidget,
                          i18n("Failed to install hot new stuff.") );
  }
}

void Engine::upload(const TQString &fileName, const TQString &previewName )
{
  mUploadFile = fileName;
  mPreviewFile = previewName;

  connect( mProviderLoader,
           TQT_SIGNAL( providersLoaded( Provider::List * ) ),
           TQT_SLOT( selectUploadProvider( Provider::List * ) ) );
  mProviderLoader->load( mType );
}

void Engine::selectUploadProvider( Provider::List *providers )
{
  kdDebug() << "Engine:selectUploadProvider()" << endl;

  mProviderLoader->disconnect();

  if ( !mProviderDialog ) {
    mProviderDialog = new ProviderDialog( this, mParentWidget );
  }

  mProviderDialog->clear();

  mProviderDialog->show();
  mProviderDialog->raise();

  for( Provider *p = providers->first(); p; p = providers->next() ) {
    mProviderDialog->addProvider( p );
  }
}

void Engine::requestMetaInformation( Provider *provider )
{
  mUploadProvider = provider;

  if ( !mUploadDialog ) {
    mUploadDialog = new UploadDialog( this, mParentWidget );
  }
  mUploadDialog->setPreviewFile( mPreviewFile );
  mUploadDialog->setPayloadFile( mUploadFile );
  mUploadDialog->show();
  mUploadDialog->raise();
}

void Engine::upload( Entry *entry )
{
  if ( mUploadFile.isNull()) {
    mUploadFile = entry->fullName();
    mUploadFile = locateLocal( "data", TQString(kapp->instanceName()) + "/upload/" + mUploadFile );

    if ( !d->mNewStuff->createUploadFile( mUploadFile ) ) {
      KMessageBox::error( mParentWidget, i18n("Unable to create file to upload.") );
      emit uploadFinished( false );
      return;
    }
  }

  TQString lang = entry->langs().first();
  TQFileInfo fi( mUploadFile );
  entry->setPayload( KURL::fromPathOrURL( fi.fileName() ), lang );

  if ( !createMetaFile( entry ) ) {
    emit uploadFinished( false );
    return;
  } 

  TQString text = i18n("The files to be uploaded have been created at:\n");
  text.append( i18n("Data file: %1\n").arg( mUploadFile) );
  if (!mPreviewFile.isEmpty()) {
    text.append( i18n("Preview image: %1\n").arg( mPreviewFile) );
  }
  text.append( i18n("Content information: %1\n").arg( mUploadMetaFile) );
  text.append( i18n("Those files can now be uploaded.\n") );
  text.append( i18n("Beware that any people might have access to them at any time.") );

  TQString caption = i18n("Upload Files");

  if ( mUploadProvider->noUpload() ) {
    KURL noUploadUrl = mUploadProvider->noUploadUrl();
    if ( noUploadUrl.isEmpty() ) {
      text.append( i18n("Please upload the files manually.") );
      KMessageBox::information( mParentWidget, text, caption );
    } else {
      int result = KMessageBox::questionYesNo( mParentWidget, text, caption,
                                               i18n("Upload Info"),
                                               KStdGuiItem::close() );
      if ( result == KMessageBox::Yes ) {
        kapp->invokeBrowser( noUploadUrl.url() );
      }
    }
  } else {
    int result = KMessageBox::questionYesNo( mParentWidget, text, caption,
                                             i18n("&Upload"), KStdGuiItem::cancel() );
    if ( result == KMessageBox::Yes ) {
      KURL destination = mUploadProvider->uploadUrl();
      destination.setFileName( fi.fileName() );

      KIO::FileCopyJob *job = KIO::file_copy( KURL::fromPathOrURL( mUploadFile ), destination );
      connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
               TQT_SLOT( slotUploadPayloadJobResult( KIO::Job * ) ) );
    } else {
      emit uploadFinished( false );
    }
  }
}

bool Engine::createMetaFile( Entry *entry )
{
  TQDomDocument doc("knewstuff");
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  TQDomElement de = doc.createElement("knewstuff");
  doc.appendChild( de );

  entry->setType(type());
  de.appendChild( entry->createDomElement( doc, de ) );

  kdDebug() << "--DOM START--" << endl << doc.toString()
            << "--DOM_END--" << endl;

  if ( mUploadMetaFile.isNull() ) {
    mUploadMetaFile = entry->fullName() + ".meta";
    mUploadMetaFile = locateLocal( "data", TQString(kapp->instanceName()) + "/upload/" + mUploadMetaFile );
  }

  TQFile f( mUploadMetaFile );
  if ( !f.open( IO_WriteOnly ) ) {
    mUploadMetaFile = TQString::null;
    return false;
  }

  TQTextStream ts( &f );
  ts.setEncoding( TQTextStream::UnicodeUTF8 );
  ts << doc.toString();

  f.close();

  return true;
}

void Engine::slotUploadPayloadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error uploading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    emit uploadFinished( false );
    return;
  }

  if (mPreviewFile.isEmpty()) {
    slotUploadPreviewJobResult(job);
    return;
  }

  TQFileInfo fi( mPreviewFile );

  KURL previewDestination = mUploadProvider->uploadUrl();
  previewDestination.setFileName( fi.fileName() );

  KIO::FileCopyJob *newJob = KIO::file_copy( KURL::fromPathOrURL( mPreviewFile ), previewDestination );
  connect( newJob, TQT_SIGNAL( result( KIO::Job * ) ),
           TQT_SLOT( slotUploadPreviewJobResult( KIO::Job * ) ) );
}

void Engine::slotUploadPreviewJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error uploading new stuff preview." << endl;
    job->showErrorDialog( mParentWidget );
    emit uploadFinished( true );
    return;
  }

  TQFileInfo fi( mUploadMetaFile );

  KURL metaDestination = mUploadProvider->uploadUrl();
  metaDestination.setFileName( fi.fileName() );

  KIO::FileCopyJob *newJob = KIO::file_copy( KURL::fromPathOrURL( mUploadMetaFile ), metaDestination );
  connect( newJob, TQT_SIGNAL( result( KIO::Job * ) ),
           TQT_SLOT( slotUploadMetaJobResult( KIO::Job * ) ) );
}

void Engine::slotUploadMetaJobResult( KIO::Job *job )
{
  mUploadMetaFile = TQString::null;
  if ( job->error() ) {
    kdDebug() << "Error uploading new stuff metadata." << endl;
    job->showErrorDialog( mParentWidget );
    emit uploadFinished( false );
    return;
  }

  KMessageBox::information( mParentWidget,
                            i18n("Successfully uploaded new stuff.") );
  emit uploadFinished( true );
}

void Engine::ignoreInstallResult(bool ignore)
{
  d->mIgnoreInstallResult = ignore;
}
