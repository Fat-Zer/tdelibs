 /* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
#include "browserextension.h"

#include <tqapplication.h>
#include <tqclipboard.h>
#include <tqtimer.h>
#include <tqobjectlist.h>
#include <tqmetaobject.h>
#include <tqregexp.h>
#include <tqstrlist.h>
#include <tqstylesheet.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>
#include <kurifilter.h>
#include <assert.h>

using namespace KParts;

const char *OpenURLEvent::s_strOpenURLEvent = "KParts/BrowserExtension/OpenURLevent";

class OpenURLEvent::OpenURLEventPrivate
{
public:
  OpenURLEventPrivate()
  {
  }
  ~OpenURLEventPrivate()
  {
  }
};

OpenURLEvent::OpenURLEvent( ReadOnlyPart *part, const KURL &url, const URLArgs &args )
: Event( s_strOpenURLEvent ), m_part( part ), m_url( url ), m_args( args )
{
//  d = new OpenURLEventPrivate();
}

OpenURLEvent::~OpenURLEvent()
{
//  delete d;
}

namespace KParts
{

struct URLArgsPrivate
{
    URLArgsPrivate() {
      doPost = false;
      redirectedRequest = false;
      lockHistory = false;
      newTab = false;
      forcesNewWindow = false;
    }
    TQString contentType; // for POST
    TQMap<TQString, TQString> metaData;
    bool doPost;
    bool redirectedRequest;
    bool lockHistory;
    bool newTab;
    bool forcesNewWindow;
};

}

URLArgs::URLArgs()
{
  reload = false;
  xOffset = 0;
  yOffset = 0;
  trustedSource = false;
  d = 0L; // Let's build it on demand for now
}


URLArgs::URLArgs( bool _reload, int _xOffset, int _yOffset, const TQString &_serviceType )
{
  reload = _reload;
  xOffset = _xOffset;
  yOffset = _yOffset;
  serviceType = _serviceType;
  d = 0L; // Let's build it on demand for now
}

URLArgs::URLArgs( const URLArgs &args )
{
  d = 0L;
  (*this) = args;
}

URLArgs &URLArgs::operator=(const URLArgs &args)
{
  if (this == &args) return *this;

  delete d; d= 0;

  reload = args.reload;
  xOffset = args.xOffset;
  yOffset = args.yOffset;
  serviceType = args.serviceType;
  postData = args.postData;
  frameName = args.frameName;
  docState = args.docState;
  trustedSource = args.trustedSource;

  if ( args.d )
     d = new URLArgsPrivate( * args.d );

  return *this;
}

URLArgs::~URLArgs()
{
  delete d;
  d = 0;
}

void URLArgs::setContentType( const TQString & contentType )
{
  if (!d)
    d = new URLArgsPrivate;
  d->contentType = contentType;
}

void URLArgs::setRedirectedRequest( bool redirected )
{
  if (!d)
     d = new URLArgsPrivate;
  d->redirectedRequest = redirected;
}

bool URLArgs::redirectedRequest () const
{
  return d ? d->redirectedRequest : false;
}

TQString URLArgs::contentType() const
{
  return d ? d->contentType : TQString::null;
}

TQMap<TQString, TQString> &URLArgs::metaData()
{
  if (!d)
     d = new URLArgsPrivate;
  return d->metaData;
}

void URLArgs::setDoPost( bool enable )
{
    if ( !d )
        d = new URLArgsPrivate;
    d->doPost = enable;
}

bool URLArgs::doPost() const
{
    return d ? d->doPost : false;
}

void URLArgs::setLockHistory( bool lock )
{
  if (!d)
     d = new URLArgsPrivate;
  d->lockHistory = lock;
}

bool URLArgs::lockHistory() const
{
    return d ? d->lockHistory : false;
}

void URLArgs::setNewTab( bool newTab )
{
  if (!d)
     d = new URLArgsPrivate;
  d->newTab = newTab;
}

bool URLArgs::newTab() const
{
    return d ? d->newTab : false;
}

void URLArgs::setForcesNewWindow( bool forcesNewWindow )
{
  if (!d)
     d = new URLArgsPrivate;
  d->forcesNewWindow = forcesNewWindow;
}

bool URLArgs::forcesNewWindow() const
{
    return d ? d->forcesNewWindow : false;
}

namespace KParts
{

struct WindowArgsPrivate
{
};

}

WindowArgs::WindowArgs()
{
    x = y = width = height = -1;
    fullscreen = false;
    menuBarVisible = true;
    toolBarsVisible = true;
    statusBarVisible = true;
    scrollBarsVisible = true;
    resizable = true;
    lowerWindow = false;
    d = 0;
}

WindowArgs::WindowArgs( const WindowArgs &args )
{
    d = 0;
    (*this) = args;
}

WindowArgs::~WindowArgs()
{
    delete d;
}

WindowArgs &WindowArgs::operator=( const WindowArgs &args )
{
    if ( this == &args ) return *this;

    delete d; d = 0;

    x = args.x;
    y = args.y;
    width = args.width;
    height = args.height;
    fullscreen = args.fullscreen;
    menuBarVisible = args.menuBarVisible;
    toolBarsVisible = args.toolBarsVisible;
    statusBarVisible = args.statusBarVisible;
    scrollBarsVisible = args.scrollBarsVisible;
    resizable = args.resizable;
    lowerWindow = args.lowerWindow;

    /*
    if ( args.d )
    {
      [ ... ]
    }
    */

    return *this;
}

WindowArgs::WindowArgs( const TQRect &_tqgeometry, bool _fullscreen, bool _menuBarVisible,
                        bool _toolBarsVisible, bool _statusBarVisible, bool _resizable )
{
    d = 0;
    x = _tqgeometry.x();
    y = _tqgeometry.y();
    width = _tqgeometry.width();
    height = _tqgeometry.height();
    fullscreen = _fullscreen;
    menuBarVisible = _menuBarVisible;
    toolBarsVisible = _toolBarsVisible;
    statusBarVisible = _statusBarVisible;
    resizable = _resizable;
    lowerWindow = false;
}

WindowArgs::WindowArgs( int _x, int _y, int _width, int _height, bool _fullscreen,
                        bool _menuBarVisible, bool _toolBarsVisible,
                        bool _statusBarVisible, bool _resizable )
{
    d = 0;
    x = _x;
    y = _y;
    width = _width;
    height = _height;
    fullscreen = _fullscreen;
    menuBarVisible = _menuBarVisible;
    toolBarsVisible = _toolBarsVisible;
    statusBarVisible = _statusBarVisible;
    resizable = _resizable;
    lowerWindow = false;
}

namespace KParts
{

// Internal class, use to store the status of the actions
class KBitArray
{
public:
    int val;
    KBitArray() { val = 0; }
    bool operator [](int index) { return (val & (1 << index)) ? true : false; }
    void setBit(int index, bool value) {
        if (value) val = val | (1 << index);
        else val = val & ~(1 << index);
    }
};

class BrowserExtensionPrivate
{
public:
  BrowserExtensionPrivate()
  {
      m_browserInterface = 0;
  }
  ~BrowserExtensionPrivate()
  {
  }

  struct DelayedRequest {
    KURL m_delayedURL;
    KParts::URLArgs m_delayedArgs;
  };
  TQValueList<DelayedRequest> m_requests;
  bool m_urlDropHandlingEnabled;
  KBitArray m_actiontqStatus;
  TQMap<int, TQString> m_actionText;
  BrowserInterface *m_browserInterface;
};

}

BrowserExtension::ActionSlotMap * BrowserExtension::s_actionSlotMap = 0L;
static KStaticDeleter<BrowserExtension::ActionSlotMap> actionSlotMapsd;
BrowserExtension::ActionNumberMap * BrowserExtension::s_actionNumberMap = 0L;
static KStaticDeleter<BrowserExtension::ActionNumberMap> actionNumberMapsd;

BrowserExtension::BrowserExtension( KParts::ReadOnlyPart *parent,
                                    const char *name )
: TQObject( parent, name), m_part( parent )
{
  //kdDebug() << "BrowserExtension::BrowserExtension() " << this << endl;
  d = new BrowserExtensionPrivate;
  d->m_urlDropHandlingEnabled = false;

  if ( !s_actionSlotMap )
      // Create the action-slot map
      createActionSlotMap();

  // Set the initial status of the actions depending on whether
  // they're supported or not
  ActionSlotMap::ConstIterator it = s_actionSlotMap->begin();
  ActionSlotMap::ConstIterator itEnd = s_actionSlotMap->end();
  TQStrList slotNames = tqmetaObject()->slotNames();
  for ( int i=0 ; it != itEnd ; ++it, ++i )
  {
      // Does the extension have a slot with the name of this action ?
      d->m_actiontqStatus.setBit( i, slotNames.tqcontains( it.key()+"()" ) );
  }

  connect( m_part, TQT_SIGNAL( completed() ),
           this, TQT_SLOT( slotCompleted() ) );
  connect( this, TQT_SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
           this, TQT_SLOT( slotOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );
  connect( this, TQT_SIGNAL( enableAction( const char *, bool ) ),
           this, TQT_SLOT( slotEnableAction( const char *, bool ) ) );
  connect( this, TQT_SIGNAL( setActionText( const char *, const TQString& ) ),
           this, TQT_SLOT( slotSetActionText( const char *, const TQString& ) ) );
}

BrowserExtension::~BrowserExtension()
{
  //kdDebug() << "BrowserExtension::~BrowserExtension() " << this << endl;
  delete d;
}

void BrowserExtension::setURLArgs( const URLArgs &args )
{
  m_args = args;
}

URLArgs BrowserExtension::urlArgs() const
{
  return m_args;
}

int BrowserExtension::xOffset()
{
  return 0;
}

int BrowserExtension::yOffset()
{
  return 0;
}

void BrowserExtension::saveState( TQDataStream &stream )
{
  stream << m_part->url() << (TQ_INT32)xOffset() << (TQ_INT32)yOffset();
}

void BrowserExtension::restoreState( TQDataStream &stream )
{
  KURL u;
  TQ_INT32 xOfs, yOfs;
  stream >> u >> xOfs >> yOfs;

  URLArgs args( urlArgs() );
  args.xOffset = xOfs;
  args.yOffset = yOfs;

  setURLArgs( args );

  m_part->openURL( u );
}

bool BrowserExtension::isURLDropHandlingEnabled() const
{
    return d->m_urlDropHandlingEnabled;
}

void BrowserExtension::setURLDropHandlingEnabled( bool enable )
{
    d->m_urlDropHandlingEnabled = enable;
}

void BrowserExtension::slotCompleted()
{
  //empty the argument stuff, to avoid bogus/invalid values when opening a new url
  setURLArgs( URLArgs() );
}

void BrowserExtension::pasteRequest()
{
    TQCString plain( "plain" );
    TQString url = TQApplication::clipboard()->text(plain, QClipboard::Selection).stripWhiteSpace();
    // Remove linefeeds and any whitespace surrounding it.
    url.remove(TQRegExp("[\\ ]*\\n+[\\ ]*"));

    // Check if it's a URL
    TQStringList filters = KURIFilter::self()->pluginNames();
    filters.remove( "kuriikwsfilter" );
    filters.remove( "localdomainurifilter" );
    KURIFilterData filterData;
    filterData.setData( url );
    filterData.setCheckForExecutables( false );
    if ( KURIFilter::self()->filterURI( filterData, filters ) )
    {
        switch ( filterData.uriType() )
	{
	    case KURIFilterData::LOCAL_FILE:
	    case KURIFilterData::LOCAL_DIR:
	    case KURIFilterData::NET_PROTOCOL:
	        slotOpenURLRequest( filterData.uri(), KParts::URLArgs() );
		break;
	    case KURIFilterData::ERROR:
		KMessageBox::sorry( m_part->widget(), filterData.errorMsg() );
		break;
	    default:
		break;
	}
    }
    else if ( KURIFilter::self()->filterURI( filterData, "kuriikwsfilter" ) && url.length() < 250 )
    {
        if ( KMessageBox::questionYesNo( m_part->widget(),
		    i18n( "<qt>Do you want to search the Internet for <b>%1</b>?" ).arg( TQStyleSheet::escape(url) ),
		    i18n( "Internet Search" ), KGuiItem( i18n( "&Search" ), "tqfind"),
		    KStdGuiItem::cancel(), "MiddleClickSearch" ) == KMessageBox::Yes)
          slotOpenURLRequest( filterData.uri(), KParts::URLArgs() );
    }
}

void BrowserExtension::slotOpenURLRequest( const KURL &url, const KParts::URLArgs &args )
{
    //kdDebug() << this << " BrowserExtension::slotOpenURLRequest(): url=" << url.url() << endl;
    BrowserExtensionPrivate::DelayedRequest req;
    req.m_delayedURL = url;
    req.m_delayedArgs = args;
    d->m_requests.append( req );
    TQTimer::singleShot( 0, this, TQT_SLOT( slotEmitOpenURLRequestDelayed() ) );
}

void BrowserExtension::slotEmitOpenURLRequestDelayed()
{
    if (d->m_requests.isEmpty()) return;
    BrowserExtensionPrivate::DelayedRequest req = d->m_requests.front();
    d->m_requests.pop_front();
    emit openURLRequestDelayed( req.m_delayedURL, req.m_delayedArgs );
    // tricky: do not do anything here! (no access to member variables, etc.)
}

void BrowserExtension::setBrowserInterface( BrowserInterface *impl )
{
    d->m_browserInterface = impl;
}

BrowserInterface *BrowserExtension::browserInterface() const
{
    return d->m_browserInterface;
}

void BrowserExtension::slotEnableAction( const char * name, bool enabled )
{
    //kdDebug() << "BrowserExtension::slotEnableAction " << name << " " << enabled << endl;
    ActionNumberMap::ConstIterator it = s_actionNumberMap->tqfind( name );
    if ( it != s_actionNumberMap->end() )
    {
        d->m_actiontqStatus.setBit( it.data(), enabled );
        //kdDebug() << "BrowserExtension::slotEnableAction setting bit " << it.data() << " to " << enabled << endl;
    }
    else
        kdWarning() << "BrowserExtension::slotEnableAction unknown action " << name << endl;
}

bool BrowserExtension::isActionEnabled( const char * name ) const
{
    int actionNumber = (*s_actionNumberMap)[ name ];
    return d->m_actiontqStatus[ actionNumber ];
}

void BrowserExtension::slotSetActionText( const char * name, const TQString& text )
{
    kdDebug() << "BrowserExtension::slotSetActionText " << name << " " << text << endl;
    ActionNumberMap::ConstIterator it = s_actionNumberMap->tqfind( name );
    if ( it != s_actionNumberMap->end() )
    {
        d->m_actionText[ it.data() ] = text;
    }
    else
        kdWarning() << "BrowserExtension::slotSetActionText unknown action " << name << endl;
}

TQString BrowserExtension::actionText( const char * name ) const
{
    int actionNumber = (*s_actionNumberMap)[ name ];
    TQMap<int, TQString>::ConstIterator it = d->m_actionText.tqfind( actionNumber );
    if ( it != d->m_actionText.end() )
        return *it;
    return TQString::null;
}

// for compatibility
BrowserExtension::ActionSlotMap BrowserExtension::actionSlotMap()
{
    return *actionSlotMapPtr();
}

BrowserExtension::ActionSlotMap * BrowserExtension::actionSlotMapPtr()
{
    if (!s_actionSlotMap)
        createActionSlotMap();
    return s_actionSlotMap;
}

void BrowserExtension::createActionSlotMap()
{
    assert(!s_actionSlotMap);
    s_actionSlotMap = actionSlotMapsd.setObject( s_actionSlotMap, new ActionSlotMap );

    s_actionSlotMap->insert( "cut", TQT_SLOT( cut() ) );
    s_actionSlotMap->insert( "copy", TQT_SLOT( copy() ) );
    s_actionSlotMap->insert( "paste", TQT_SLOT( paste() ) );
    s_actionSlotMap->insert( "rename", TQT_SLOT( rename() ) );
    s_actionSlotMap->insert( "trash", TQT_SLOT( trash() ) );
    s_actionSlotMap->insert( "del", TQT_SLOT( del() ) );
    s_actionSlotMap->insert( "properties", TQT_SLOT( properties() ) );
    s_actionSlotMap->insert( "editMimeType", TQT_SLOT( editMimeType() ) );
    s_actionSlotMap->insert( "print", TQT_SLOT( print() ) );
    // Tricky. Those aren't actions in fact, but simply methods that a browserextension
    // can have or not. No need to return them here.
    //s_actionSlotMap->insert( "reparseConfiguration", TQT_SLOT( reparseConfiguration() ) );
    //s_actionSlotMap->insert( "refreshMimeTypes", TQT_SLOT( refreshMimeTypes() ) );
    // nothing for setSaveViewPropertiesLocally either

    // Create the action-number map
    assert(!s_actionNumberMap);
    s_actionNumberMap = actionNumberMapsd.setObject( s_actionNumberMap, new ActionNumberMap );
    ActionSlotMap::ConstIterator it = s_actionSlotMap->begin();
    ActionSlotMap::ConstIterator itEnd = s_actionSlotMap->end();
    for ( int i=0 ; it != itEnd ; ++it, ++i )
    {
        //kdDebug(1202) << " action " << it.key() << " number " << i << endl;
        s_actionNumberMap->insert( it.key(), i );
    }
}

BrowserExtension *BrowserExtension::childObject( TQObject *obj )
{
    if ( !obj || !obj->children() )
        return 0L;

    // we try to do it on our own, in hope that we are faster than
    // queryList, which looks kind of big :-)
    const TQObjectList *children = obj->children();
    TQObjectListIt it( *children );
    for (; it.current(); ++it )
        if ( it.current()->inherits( "KParts::BrowserExtension" ) )
            return static_cast<KParts::BrowserExtension *>( it.current() );

    return 0L;
}

namespace KParts
{

class BrowserHostExtension::BrowserHostExtensionPrivate
{
public:
  BrowserHostExtensionPrivate()
  {
  }
  ~BrowserHostExtensionPrivate()
  {
  }

  KParts::ReadOnlyPart *m_part;
};

}

BrowserHostExtension::BrowserHostExtension( KParts::ReadOnlyPart *parent, const char *name )
 : TQObject( parent, name )
{
  d = new BrowserHostExtensionPrivate;
  d->m_part = parent;
}

BrowserHostExtension::~BrowserHostExtension()
{
  delete d;
}

TQStringList BrowserHostExtension::frameNames() const
{
  return TQStringList();
}

const TQPtrList<KParts::ReadOnlyPart> BrowserHostExtension::frames() const
{
  return TQPtrList<KParts::ReadOnlyPart>();
}

bool BrowserHostExtension::openURLInFrame( const KURL &, const KParts::URLArgs & )
{
  return false;
}

BrowserHostExtension *BrowserHostExtension::childObject( TQObject *obj )
{
    if ( !obj || !obj->children() )
        return 0L;

    // we try to do it on our own, in hope that we are faster than
    // queryList, which looks kind of big :-)
    const TQObjectList *children = obj->children();
    TQObjectListIt it( *children );
    for (; it.current(); ++it )
        if ( it.current()->inherits( "KParts::BrowserHostExtension" ) )
            return static_cast<KParts::BrowserHostExtension *>( it.current() );

    return 0L;
}

void BrowserExtension::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

BrowserHostExtension *
BrowserHostExtension::tqfindFrameParent(KParts::ReadOnlyPart *callingPart, const TQString &frame)
{
    FindFrameParentParams param;
    param.parent = 0;
    param.callingPart = callingPart;
    param.frame = frame;
    virtual_hook(VIRTUAL_FIND_FRAME_PARENT, &param);
    return param.parent;
}

void BrowserHostExtension::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

LiveConnectExtension::LiveConnectExtension( KParts::ReadOnlyPart *parent, const char *name ) : TQObject( parent, name) {}

bool LiveConnectExtension::get( const unsigned long, const TQString &, Type &, unsigned long &, TQString & ) {
    return false;
}

bool LiveConnectExtension::put( const unsigned long, const TQString &, const TQString & ) {
      return false;
}

bool LiveConnectExtension::call( const unsigned long, const TQString &, const TQStringList &, Type &, unsigned long &, TQString & ) {
      return false;
}

void LiveConnectExtension::unregister( const unsigned long ) {}

LiveConnectExtension *LiveConnectExtension::childObject( TQObject *obj )
{
    if ( !obj || !obj->children() )
        return 0L;

    // we try to do it on our own, in hope that we are faster than
    // queryList, which looks kind of big :-)
    const TQObjectList *children = obj->children();
    TQObjectListIt it( *children );
    for (; it.current(); ++it )
        if ( it.current()->inherits( "KParts::LiveConnectExtension" ) )
            return static_cast<KParts::LiveConnectExtension *>( it.current() );

    return 0L;
}

#include "browserextension.moc"
