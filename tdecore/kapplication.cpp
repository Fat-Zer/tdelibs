/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team

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

#include "config.h"

#ifdef HAVE_XCOMPOSITE
#define COMPOSITE
#endif

// #ifdef QTRANSLATOR_H
// #error qtranslator.h was already included
// #endif // QTRANSLATOR_H
// 
// #ifdef TQTRANSLATOR_H
// #error tqtranslator.h was already included
// #endif // TQTRANSLATOR_H

#undef QT_NO_TRANSLATION
#undef TQT_NO_TRANSLATION
#include <tqtranslator.h>
#include "kapplication.h"
#define QT_NO_TRANSLATION
#define TQT_NO_TRANSLATION
#include <tqdir.h>
#include <tqptrcollection.h>
#include <tqwidgetlist.h>
#include <tqstrlist.h>
#include <tqfile.h>
#include <tqmessagebox.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <tqlineedit.h>
#include <tqtextedit.h>
#include <tqsessionmanager.h>
#include <tqptrlist.h>
#include <tqtimer.h>
#include <tqstylesheet.h>
#include <tqpixmapcache.h>
#include <tqtooltip.h>
#include <tqstylefactory.h>
#include <tqmetaobject.h>
#include <tqimage.h>
#ifndef QT_NO_SQL
#include <tqsqlpropertymap.h>
#endif

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstyle.h>
#include <kiconloader.h>
#include <kclipboard.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobalsettings.h>
#include <kcrash.h>
#include <kdatastream.h>
#include <klibloader.h>
#include <kmimesourcefactory.h>
#include <kstdaccel.h>
#include <kaccel.h>
#include "kcheckaccelerators.h"
#include <tqptrdict.h>
#include <kmacroexpander.h>
#include <kshell.h>
#include <kprotocolinfo.h>
#include <kkeynative.h>
#include <kmdcodec.h>
#include <kglobalaccel.h>

#if defined Q_WS_X11
#include <kstartupinfo.h>
#endif

#include <dcopclient.h>
#include <dcopref.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>
#include <grp.h>
#include <sys/types.h>

#ifndef Q_WS_WIN
#include "twin.h"
#endif

#include <fcntl.h>
#include <stdlib.h> // getenv(), srand(), rand()
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#if defined Q_WS_X11
//#ifndef Q_WS_QWS //FIXME(E): NetWM should talk to QWS...
#include <netwm.h>
#endif

#include "kprocctrl.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#ifdef COMPOSITE
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>
#include <dlfcn.h>
#endif
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <fixx11h.h>
#endif

#include <pwd.h>

#ifndef Q_WS_WIN
#include <KDE-ICE/ICElib.h>
#else
typedef void* IceIOErrorHandler;
#include <windows.h>
//KDE4: remove
#define Button1Mask (1<<8)
#define Button2Mask (1<<9)
#define Button3Mask (1<<10)
#endif

#ifdef Q_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(Q_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#endif

#if defined Q_WS_X11
#include <kipc.h>
#endif

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#include <tqimage.h>
#endif

#include "kappdcopiface.h"

// exported for kdm kfrontend
KDE_EXPORT bool kde_have_kipc = true; // magic hook to disable kipc in kdm
bool kde_kiosk_exception = false; // flag to disable kiosk restrictions
bool kde_kiosk_admin = false;

KApplication* KApplication::KApp = 0L;
bool KApplication::loadedByKdeinit = false;
DCOPClient *KApplication::s_DCOPClient = 0L;
bool KApplication::s_dcopClientNeedsPostInit = false;

#ifdef Q_WS_X11
static Atom atom_DesktopWindow;
static Atom atom_NetSupported;
static Atom kde_xdnd_drop;
#endif

#ifdef Q_WS_X11
static int composite_event, composite_error, composite_opcode;
static bool x11_composite_error_generated;
static int x11_error(Display *dpy, XErrorEvent *ev) {
	if (ev->request_code == composite_opcode && ev->minor_code == X_CompositeRedirectSubwindows)
	{
		x11_composite_error_generated = true;
		return 0;
	}
}
#endif

// duplicated from patched Qt, so that there won't be unresolved symbols if Qt gets
// replaced by unpatched one
TDECORE_EXPORT bool qt_qclipboard_bailout_hack = false;

template class TQPtrList<KSessionManaged>;

#ifdef Q_WS_X11
extern "C" {
static int kde_xio_errhandler( Display * dpy )
{
  return kapp->xioErrhandler( dpy );
}

static int kde_x_errhandler( Display *dpy, XErrorEvent *err )
{
  return kapp->xErrhandler( dpy, err );
}

}

extern "C" {
static void kde_ice_ioerrorhandler( IceConn conn )
{
    if(kapp)
        kapp->iceIOErrorHandler( conn );
    // else ignore the error for now
}
}
#endif

#ifdef Q_WS_WIN
void KApplication_init_windows(bool GUIenabled);

class QAssistantClient;
#endif

/*
  Private data to make keeping binary compatibility easier
 */
class KApplicationPrivate
{
public:
  KApplicationPrivate()
    :   actionRestrictions( false ),
	refCount( 1 ),
	oldIceIOErrorHandler( 0 ),
	checkAccelerators( 0 ),
	overrideStyle( TQString::null ),
	startup_id( "0" ),
	app_started_timer( NULL ),
	m_KAppDCOPInterface( 0L ),
	session_save( false )
#ifdef Q_WS_X11
	,oldXErrorHandler( NULL )
	,oldXIOErrorHandler( NULL )
#elif defined Q_WS_WIN
	,qassistantclient( 0 )
#endif
  {
  }

  ~KApplicationPrivate()
  {
#ifdef Q_WS_WIN
     delete qassistantclient;
#endif
  }


  bool actionRestrictions : 1;
  bool guiEnabled : 1;
  /**
   * This counter indicates when to exit the application.
   * It starts at 1, is decremented in KMainWindow when the last window is closed, but
   * is incremented by operations that should outlive the last window closed
   * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
   */
  int refCount;
  IceIOErrorHandler oldIceIOErrorHandler;
  KCheckAccelerators* checkAccelerators;
  TQString overrideStyle;
  TQString geometry_arg;
  TQCString startup_id;
  TQTimer* app_started_timer;
  KAppDCOPInterface *m_KAppDCOPInterface;
  bool session_save;
#ifdef Q_WS_X11
  int (*oldXErrorHandler)(Display*,XErrorEvent*);
  int (*oldXIOErrorHandler)(Display*);
#elif defined Q_WS_WIN
  QAssistantClient* qassistantclient;
#endif

  class URLActionRule
  {
  public:
#define checkExactMatch(s, b) \
        if (s.isEmpty()) b = true; \
        else if (s[s.length()-1] == '!') \
        { b = false; s.truncate(s.length()-1); } \
        else b = true;
#define checkStartWildCard(s, b) \
        if (s.isEmpty()) b = true; \
        else if (s[0] == '*') \
        { b = true; s = s.mid(1); } \
        else b = false;
#define checkEqual(s, b) \
        b = (s == "=");

     URLActionRule(const TQString &act,
                   const TQString &bProt, const TQString &bHost, const TQString &bPath,
                   const TQString &dProt, const TQString &dHost, const TQString &dPath,
                   bool perm)
                   : action(act),
                     baseProt(bProt), baseHost(bHost), basePath(bPath),
                     destProt(dProt), destHost(dHost), destPath(dPath),
                     permission(perm)
                   {
                      checkExactMatch(baseProt, baseProtWildCard);
                      checkStartWildCard(baseHost, baseHostWildCard);
                      checkExactMatch(basePath, basePathWildCard);
                      checkExactMatch(destProt, destProtWildCard);
                      checkStartWildCard(destHost, destHostWildCard);
                      checkExactMatch(destPath, destPathWildCard);
                      checkEqual(destProt, destProtEqual);
                      checkEqual(destHost, destHostEqual);
                   }

     bool baseMatch(const KURL &url, const TQString &protClass)
     {
        if (baseProtWildCard)
        {
           if ( !baseProt.isEmpty() && !url.protocol().startsWith(baseProt) &&
                (protClass.isEmpty() || (protClass != baseProt)) )
              return false;
        }
        else
        {
           if ( (url.protocol() != baseProt) &&
                (protClass.isEmpty() || (protClass != baseProt)) )
              return false;
        }
        if (baseHostWildCard)
        {
           if (!baseHost.isEmpty() && !url.host().endsWith(baseHost))
              return false;
        }
        else
        {
           if (url.host() != baseHost)
              return false;
        }
        if (basePathWildCard)
        {
           if (!basePath.isEmpty() && !url.path().startsWith(basePath))
              return false;
        }
        else
        {
           if (url.path() != basePath)
              return false;
        }
        return true;
     }

     bool destMatch(const KURL &url, const TQString &protClass, const KURL &base, const TQString &baseClass)
     {
        if (destProtEqual)
        {
           if ( (url.protocol() != base.protocol()) &&
                (protClass.isEmpty() || baseClass.isEmpty() || protClass != baseClass) )
              return false;
        }
        else if (destProtWildCard)
        {
           if ( !destProt.isEmpty() && !url.protocol().startsWith(destProt) &&
                (protClass.isEmpty() || (protClass != destProt)) )
              return false;
        }
        else
        {
           if ( (url.protocol() != destProt) &&
                (protClass.isEmpty() || (protClass != destProt)) )
              return false;
        }
        if (destHostWildCard)
        {
           if (!destHost.isEmpty() && !url.host().endsWith(destHost))
              return false;
        }
        else if (destHostEqual)
        {
           if (url.host() != base.host())
              return false;
        }
        else
        {
           if (url.host() != destHost)
              return false;
        }
        if (destPathWildCard)
        {
           if (!destPath.isEmpty() && !url.path().startsWith(destPath))
              return false;
        }
        else
        {
           if (url.path() != destPath)
              return false;
        }
        return true;
     }

     TQString action;
     TQString baseProt;
     TQString baseHost;
     TQString basePath;
     TQString destProt;
     TQString destHost;
     TQString destPath;
     bool baseProtWildCard : 1;
     bool baseHostWildCard : 1;
     bool basePathWildCard : 1;
     bool destProtWildCard : 1;
     bool destHostWildCard : 1;
     bool destPathWildCard : 1;
     bool destProtEqual    : 1;
     bool destHostEqual    : 1;
     bool permission;
  };
  TQPtrList<URLActionRule> urlActionRestrictions;

    TQString sessionKey;
    TQString pSessionConfigFile;
};


static TQPtrList<TQWidget>*x11Filter = 0;
static bool autoDcopRegistration = true;

void KApplication::installX11EventFilter( TQWidget* filter )
{
    if ( !filter )
        return;
    if (!x11Filter)
        x11Filter = new TQPtrList<TQWidget>;
    connect ( filter, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( x11FilterDestroyed() ) );
    x11Filter->append( filter );
}

void KApplication::x11FilterDestroyed()
{
    removeX11EventFilter( static_cast< const TQWidget* >( sender()));
}

void KApplication::removeX11EventFilter( const TQWidget* filter )
{
    if ( !x11Filter || !filter )
        return;
    x11Filter->removeRef( filter );
    if ( x11Filter->isEmpty() ) {
        delete x11Filter;
        x11Filter = 0;
    }
}

// FIXME: remove this when we've get a better method of
// customizing accelerator handling -- hopefully in Qt.
// For now, this is set whenever an accelerator is overridden
// in KAccelEventHandler so that the AccelOverride isn't sent twice. -- ellis, 19/10/02
extern bool kde_g_bKillAccelOverride;

bool KApplication::notify(TQObject *receiver, TQEvent *event)
{
    TQEvent::Type t = event->type();
    if (kde_g_bKillAccelOverride)
    {
       kde_g_bKillAccelOverride = false;
       // Indicate that the accelerator has been overridden.
       if (t == TQEvent::AccelOverride)
       {
          TQT_TQKEYEVENT(event)->accept();
          return true;
       }
       else
          kdWarning(125) << "kde_g_bKillAccelOverride set, but received an event other than AccelOverride." << endl;
    }

    if ((t == TQEvent::AccelOverride) || (t == TQEvent::KeyPress))
    {
       static const KShortcut& _selectAll = KStdAccel::selectAll();
       TQLineEdit *edit = ::tqqt_cast<TQLineEdit *>(receiver);
       if (edit)
       {
          // We have a keypress for a lineedit...
          TQKeyEvent *kevent = TQT_TQKEYEVENT(event);
          KKey key(kevent);
          if (_selectAll.contains(key))
          {
             if (t == TQEvent::KeyPress)
             {
                edit->selectAll();
                return true;
             }
             else
             {
                kevent->accept();
             }
          }
          // Ctrl-U deletes from start of line.
          if (key == KKey(Qt::CTRL + Qt::Key_U))
          {
             if (t == TQEvent::KeyPress)
             {
                if (!edit->isReadOnly())
                {
                   TQString t(edit->text());
                   t = t.mid(edit->cursorPosition());
                   edit->validateAndSet(t, 0, 0, 0);
                }
                return true;
             }
             else
             {
                kevent->accept();
             }

          }
       }
       TQTextEdit *medit = ::tqqt_cast<TQTextEdit *>(receiver);
       if (medit)
       {
          // We have a keypress for a multilineedit...
          TQKeyEvent *kevent = TQT_TQKEYEVENT(event);
          if (_selectAll.contains(KKey(kevent)))
          {
             if (t == TQEvent::KeyPress)
             {
                medit->selectAll();
                return true;
             }
             else
             {
                kevent->accept();
             }
          }
       }
    }
    if( t == TQEvent::Show && receiver->isWidgetType())
    {
        TQWidget* w = TQT_TQWIDGET( receiver );
#if defined Q_WS_X11
        if( w->isTopLevel() && !startupId().isEmpty() && !TQT_TQSHOWEVENT(event)->spontaneous()) // TODO better done using window group leader?
            KStartupInfo::setWindowStartupId( w->winId(), startupId());
#endif
        if( w->isTopLevel() && !w->testWFlags( WX11BypassWM ) && !w->isPopup() && !event->spontaneous())
        {
            if( d->app_started_timer == NULL )
            {
                d->app_started_timer = new TQTimer( this, "app_started_timer" );
                connect( d->app_started_timer, TQT_SIGNAL( timeout()), TQT_SLOT( checkAppStartedSlot()));
            }
            if( !d->app_started_timer->isActive())
                d->app_started_timer->start( 0, true );
        }
        if( w->isTopLevel() && ( w->icon() == NULL || w->icon()->isNull()))
        {
            // icon() cannot be null pixmap, it'll be the "unknown" icon - so check if there is this application icon
            static TQPixmap* ic = NULL;
            if( ic == NULL )
                ic = new TQPixmap( KGlobal::iconLoader()->loadIcon( iconName(),
                    KIcon::NoGroup, 0, KIcon::DefaultState, NULL, true ));
            if( !ic->isNull())
            {
                w->setIcon( *ic );
#if defined Q_WS_X11
                KWin::setIcons( w->winId(), *ic, miniIcon());
#endif
            }
        }
    }
    return TQApplication::notify(receiver, event);
}

void KApplication::checkAppStartedSlot()
{
#if defined Q_WS_X11
    KStartupInfo::handleAutoAppStartedSending();
#endif
}

// the help class for session management communication
static TQPtrList<KSessionManaged>* sessionClients()
{
    static TQPtrList<KSessionManaged>* session_clients = 0L;
    if ( !session_clients )
        session_clients = new TQPtrList<KSessionManaged>;
    return session_clients;
}

/*
  Auxiliary function to calculate a a session config name used for the
  instance specific config object.
  Syntax:  "session/<appname>_<sessionId>"
 */
TQString KApplication::sessionConfigName() const
{
    TQString sessKey = sessionKey();
    if ( sessKey.isEmpty() && !d->sessionKey.isEmpty() )
        sessKey = d->sessionKey;
    return TQString("session/%1_%2_%3").arg(name()).arg(sessionId()).arg(sessKey);
}

#ifdef Q_WS_X11
static SmcConn mySmcConnection = 0;
static SmcConn tmpSmcConnection = 0;
#else
// FIXME(E): Implement for Qt Embedded
// Possibly "steal" XFree86's libSM?
#endif
static TQTime* smModificationTime = 0;

KApplication::KApplication( int& argc, char** argv, const TQCString& rAppName,
                            bool allowStyles, bool GUIenabled ) :
  TQApplication( argc, argv, GUIenabled ), KInstance(rAppName),
#ifdef Q_WS_X11
  display(0L),
  argb_visual(false),
#endif
  d (new KApplicationPrivate())
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;
    Q_ASSERT (!rAppName.isEmpty());
    setName(rAppName);

    installSigpipeHandler();
    KCmdLineArgs::initIgnore(argc, argv, rAppName.data());
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

KApplication::KApplication( bool allowStyles, bool GUIenabled ) :
//  TQApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(), TRUE ),	// Qt4 requires that there always be a GUI
  TQApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(), GUIenabled ),	// We need to be able to run command line apps
  KInstance( KCmdLineArgs::about),
#ifdef Q_WS_X11
  display(0L),
  argb_visual(false),
#endif
  d (new KApplicationPrivate)
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;
    setName( instanceName() );

    installSigpipeHandler();
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

#ifdef Q_WS_X11
KApplication::KApplication( Display *dpy, bool allowStyles ) :
  TQApplication( dpy, *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                getX11RGBAVisual(dpy), getX11RGBAColormap(dpy) ),
  KInstance( KCmdLineArgs::about), display(0L), d (new KApplicationPrivate)
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
    read_app_startup_id();
    useStyles = allowStyles;
    setName( instanceName() );
    installSigpipeHandler();
    parseCommandLine( );
    init( true );
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

KApplication::KApplication( Display *dpy, bool disable_argb, Qt::HANDLE visual, Qt::HANDLE colormap, bool allowStyles ) :
  TQApplication( dpy, *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                disable_argb?visual:getX11RGBAVisual(dpy), disable_argb?colormap:getX11RGBAColormap(dpy) ),
  KInstance( KCmdLineArgs::about), display(0L), d (new KApplicationPrivate)
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
    read_app_startup_id();
    useStyles = allowStyles;
    if (disable_argb) argb_visual = false;
    setName( instanceName() );
    installSigpipeHandler();
    parseCommandLine( );
    init( true );
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

KApplication::KApplication( Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap,
		            bool allowStyles ) :
  TQApplication( dpy, *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                visual?visual:getX11RGBAVisual(dpy), colormap?colormap:getX11RGBAColormap(dpy) ),
  KInstance( KCmdLineArgs::about), display(0L), d (new KApplicationPrivate)
{
    if ((visual) && (colormap))
        getX11RGBAInformation(dpy);
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
    read_app_startup_id();
    useStyles = allowStyles;
    setName( instanceName() );
    installSigpipeHandler();
    parseCommandLine( );
    init( true );
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

KApplication::KApplication( Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap,
		            bool allowStyles, KInstance * _instance ) :
  TQApplication( dpy, *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                visual?visual:getX11RGBAVisual(dpy), colormap?colormap:getX11RGBAColormap(dpy) ),
  KInstance( _instance ), display(0L), d (new KApplicationPrivate)
{
    if ((visual) && (colormap))
        getX11RGBAInformation(dpy);
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
    read_app_startup_id();
    useStyles = allowStyles;
    setName( instanceName() );
    installSigpipeHandler();
    parseCommandLine( );
    init( true );
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}
#endif

KApplication::KApplication( bool allowStyles, bool GUIenabled, KInstance* _instance ) :
  TQApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                GUIenabled ),
  KInstance( _instance ),
#ifdef Q_WS_X11
  display(0L),
#endif
  argb_visual(false),
  d (new KApplicationPrivate)
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;
    setName( instanceName() );

    installSigpipeHandler();
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

#ifdef Q_WS_X11
KApplication::KApplication(Display *display, int& argc, char** argv, const TQCString& rAppName,
                           bool allowStyles, bool GUIenabled ) :
  TQApplication( display ), KInstance(rAppName),
  display(0L),
  argb_visual(false),
  d (new KApplicationPrivate())
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;

    Q_ASSERT (!rAppName.isEmpty());
    setName(rAppName);

    installSigpipeHandler();
    KCmdLineArgs::initIgnore(argc, argv, rAppName.data());
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}
#endif

int KApplication::xioErrhandler( Display* dpy )
{
    if(kapp)
    {
        emit shutDown();
#ifdef Q_WS_X11
        d->oldXIOErrorHandler( dpy );
#else
        Q_UNUSED(dpy);
#endif
    }
    exit( 1 );
    return 0;
}

int KApplication::xErrhandler( Display* dpy, void* err_ )
{ // no idea how to make forward decl. for XErrorEvent
#ifdef Q_WS_X11
    XErrorEvent* err = static_cast< XErrorEvent* >( err_ );
    if(kapp)
    {
        // add KDE specific stuff here
        d->oldXErrorHandler( dpy, err );
    }
#endif
    return 0;
}

void KApplication::iceIOErrorHandler( _IceConn *conn )
{
    emit shutDown();

#ifdef Q_WS_X11
    if ( d->oldIceIOErrorHandler != NULL )
      (*d->oldIceIOErrorHandler)( conn );
#endif
    exit( 1 );
}

class KDETranslator : public TQTranslator
{
public:
  KDETranslator(TQObject *parent) : TQTranslator(parent, "kdetranslator") {}
  virtual TQTranslatorMessage findMessage(const char* context,
					 const char *sourceText,
					 const char* message) const
  {
    TQTranslatorMessage res;
    res.setTranslation(KGlobal::locale()->translateQt(context, sourceText, message));
    return res;
  }
};

void KApplication::init(bool GUIenabled)
{
  d->guiEnabled = GUIenabled;
  if ((getuid() != geteuid()) ||
      (getgid() != getegid()) )
  {
     // man permissions are not exploitable and better than 
     // world writable directories
     struct group *man = getgrnam("man");
     if ( !man || man->gr_gid != getegid() ){
       fprintf(stderr, "The KDE libraries are not designed to run with suid privileges.\n");
       ::exit(127);
     }
  }

  KProcessController::ref();

  (void) KClipboardSynchronizer::self();

  TQApplication::setDesktopSettingsAware( false );

  KApp = this;


#ifdef Q_WS_X11 //FIXME(E)
  // create all required atoms in _one_ roundtrip to the X server
  if ( GUIenabled ) {
      const int max = 20;
      Atom* atoms[max];
      char* names[max];
      Atom atoms_return[max];
      int n = 0;

      atoms[n] = &kipcCommAtom;
      names[n++] = (char *) "KIPC_COMM_ATOM";

      atoms[n] = &atom_DesktopWindow;
      names[n++] = (char *) "KDE_DESKTOP_WINDOW";

      atoms[n] = &atom_NetSupported;
      names[n++] = (char *) "_NET_SUPPORTED";

      atoms[n] = &kde_xdnd_drop;
      names[n++] = (char *) "XdndDrop";

      XInternAtoms( qt_xdisplay(), names, n, false, atoms_return );

      for (int i = 0; i < n; i++ )
	  *atoms[i] = atoms_return[i];
  }
#endif

  dcopAutoRegistration();
  dcopClientPostInit();

  smw = 0;

  // Initial KIPC event mask.
#if defined Q_WS_X11
  kipcEventMask = (1 << KIPC::StyleChanged) | (1 << KIPC::PaletteChanged) |
                  (1 << KIPC::FontChanged) | (1 << KIPC::BackgroundChanged) |
                  (1 << KIPC::ToolbarStyleChanged) | (1 << KIPC::SettingsChanged) |
                  (1 << KIPC::ClipboardConfigChanged) | (1 << KIPC::BlockShortcuts);
#endif

  // Trigger creation of locale.
  (void) KGlobal::locale();

  KConfig* config = KGlobal::config();
  d->actionRestrictions = config->hasGroup("KDE Action Restrictions" ) && !kde_kiosk_exception;
  // For brain-dead configurations where the user's local config file is not writable.
  // * We use kdialog to warn the user, so we better not generate warnings from
  //   kdialog itself.
  // * Don't warn if we run with a read-only $HOME
  TQCString readOnly = getenv("KDE_HOME_READONLY");
  if (readOnly.isEmpty() && (tqstrcmp(name(), "kdialog") != 0))
  {
    KConfigGroupSaver saver(config, "KDE Action Restrictions");
    if (config->readBoolEntry("warn_unwritable_config",true))
       config->checkConfigFilesWritable(true);
  }

  if (GUIenabled)
  {
#ifdef Q_WS_X11
    // this is important since we fork() to launch the help (Matthias)
    fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, FD_CLOEXEC);
    // set up the fancy (=robust and error ignoring ) KDE xio error handlers (Matthias)
    d->oldXErrorHandler = XSetErrorHandler( kde_x_errhandler );
    d->oldXIOErrorHandler = XSetIOErrorHandler( kde_xio_errhandler );
#endif

    connect( this, TQT_SIGNAL( aboutToQuit() ), this, TQT_SIGNAL( shutDown() ) );

#ifdef Q_WS_X11 //FIXME(E)
    display = desktop()->x11Display();
#endif

    {
        TQStringList plugins = KGlobal::dirs()->resourceDirs( "qtplugins" );
        TQStringList::Iterator it = plugins.begin();
        while (it != plugins.end()) {
            addLibraryPath( *it );
            ++it;
        }

    }
    kdisplaySetStyle();
    kdisplaySetFont();
//    kdisplaySetPalette(); done by kdisplaySetStyle
    propagateSettings(SETTINGS_QT);

    // Set default mime-source factory
    // XXX: This is a hack. Make our factory the default factory, but add the
    // previous default factory to the list of factories. Why? When the default
    // factory can't resolve something, it iterates in the list of factories.
    // But it TQWhatsThis only uses the default factory. So if there was already
    // a default factory (which happens when using an image library using uic),
    // we prefer KDE's factory and so we put that old default factory in the
    // list and use KDE as the default. This may speed up things as well.
    TQMimeSourceFactory* oldDefaultFactory = TQMimeSourceFactory::takeDefaultFactory();
    TQMimeSourceFactory::setDefaultFactory( mimeSourceFactory() );
    if ( oldDefaultFactory ) {
        TQMimeSourceFactory::addFactory( oldDefaultFactory );
    }

    d->checkAccelerators = new KCheckAccelerators( TQT_TQOBJECT(this) );
  }

#ifdef Q_WS_MACX
  if (GUIenabled) {
      TQPixmap pixmap = KGlobal::iconLoader()->loadIcon( KCmdLineArgs::appName(),
              KIcon::NoGroup, KIcon::SizeLarge, KIcon::DefaultState, 0L, false );
      if (!pixmap.isNull()) {
          TQImage i = pixmap.convertToImage().convertDepth(32).smoothScale(40, 40);
          for(int y = 0; y < i.height(); y++) {
              uchar *l = i.scanLine(y);
              for(int x = 0; x < i.width(); x+=4)
                  *(l+x) = 255;
          }
          CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
          CGDataProviderRef dp = CGDataProviderCreateWithData(NULL,
                  i.bits(), i.numBytes(), NULL);
          CGImageRef ir = CGImageCreate(i.width(), i.height(), 8, 32, i.bytesPerLine(),
                  cs, kCGImageAlphaNoneSkipFirst, dp,
                  0, 0, kCGRenderingIntentDefault);
          //cleanup
          SetApplicationDockTileImage(ir);
          CGImageRelease(ir);
          CGColorSpaceRelease(cs);
          CGDataProviderRelease(dp);
      }
  }
#endif


  // save and restore the RTL setting, as installTranslator calls qt_detectRTLLanguage,
  // which makes it impossible to use the -reverse cmdline switch with KDE apps
  bool rtl = reverseLayout();
  installTranslator(new KDETranslator(TQT_TQOBJECT(this)));
  setReverseLayout( rtl );
  if (i18n( "_: Dear Translator! Translate this string to the string 'LTR' in "
	 "left-to-right languages (as english) or to 'RTL' in right-to-left "
	 "languages (such as Hebrew and Arabic) to get proper widget layout." ) == "RTL")
	setReverseLayout( !rtl );

  // install appdata resource type
  KGlobal::dirs()->addResourceType("appdata", KStandardDirs::kde_default("data")
                                   + TQString::fromLatin1(name()) + '/');
  pSessionConfig = 0L;
  bSessionManagement = true;

#ifdef Q_WS_X11
  // register a communication window for desktop changes (Matthias)
  if (GUIenabled && kde_have_kipc )
  {
    smw = new TQWidget(0,0);
    long data = 1;
    XChangeProperty(qt_xdisplay(), smw->winId(),
		    atom_DesktopWindow, atom_DesktopWindow,
		    32, PropModeReplace, (unsigned char *)&data, 1);
  }
  d->oldIceIOErrorHandler = IceSetIOErrorHandler( kde_ice_ioerrorhandler );
#elif defined(Q_WS_WIN)
  KApplication_init_windows(GUIenabled);
#else
  // FIXME(E): Implement for Qt Embedded
#endif
}

static int my_system (const char *command) {
   int pid, status;

   pid = fork();
   if (pid == -1)
      return -1;
   if (pid == 0) {
      const char* shell = "/bin/sh";
      execl(shell, shell, "-c", command, (void *)0);
      ::_exit(127);
   }
   do {
      if (waitpid(pid, &status, 0) == -1) {
         if (errno != EINTR)
            return -1;
       } else
            return status;
   } while(1);
}


DCOPClient *KApplication::dcopClient()
{
  if (s_DCOPClient)
    return s_DCOPClient;

  s_DCOPClient = new DCOPClient();
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
  if (args && args->isSet("dcopserver"))
  {
    s_DCOPClient->setServerAddress( args->getOption("dcopserver"));
  }
  if( kapp ) {
    connect(s_DCOPClient, TQT_SIGNAL(attachFailed(const TQString &)),
            kapp, TQT_SLOT(dcopFailure(const TQString &)));
    connect(s_DCOPClient, TQT_SIGNAL(blockUserInput(bool) ),
            kapp, TQT_SLOT(dcopBlockUserInput(bool)) );
  }
  else
    s_dcopClientNeedsPostInit = true;

  DCOPClient::setMainClient( s_DCOPClient );
  return s_DCOPClient;
}

void KApplication::dcopClientPostInit()
{
  if( s_dcopClientNeedsPostInit )
    {
    s_dcopClientNeedsPostInit = false;
    connect(s_DCOPClient, TQT_SIGNAL(blockUserInput(bool) ),
            TQT_SLOT(dcopBlockUserInput(bool)) );
    s_DCOPClient->bindToApp(); // Make sure we get events from the DCOPClient.
    }
}

void KApplication::dcopAutoRegistration()
{
  if (autoDcopRegistration)
     {
     ( void ) dcopClient();
     if( dcopClient()->appId().isEmpty())
         dcopClient()->registerAs(name());
     }
}

void KApplication::disableAutoDcopRegistration()
{
  autoDcopRegistration = false;
}

KConfig* KApplication::sessionConfig()
{
    if (pSessionConfig)
        return pSessionConfig;

    // create an instance specific config object
    pSessionConfig = new KConfig( sessionConfigName(), false, false);
    return pSessionConfig;
}

void KApplication::ref()
{
    d->refCount++;
    //kdDebug() << "KApplication::ref() : refCount = " << d->refCount << endl;
}

void KApplication::deref()
{
    d->refCount--;
    //kdDebug() << "KApplication::deref() : refCount = " << d->refCount << endl;
    if ( d->refCount <= 0 )
        quit();
}

KSessionManaged::KSessionManaged()
{
    sessionClients()->remove( this );
    sessionClients()->append( this );
}

KSessionManaged::~KSessionManaged()
{
    sessionClients()->remove( this );
}

bool KSessionManaged::saveState(TQSessionManager&)
{
    return true;
}

bool KSessionManaged::commitData(TQSessionManager&)
{
    return true;
}


void KApplication::disableSessionManagement() {
  bSessionManagement = false;
}

void KApplication::enableSessionManagement() {
  bSessionManagement = true;
#ifdef Q_WS_X11
  // Session management support in Qt/KDE is awfully broken.
  // If konqueror disables session management right after its startup,
  // and enables it later (preloading stuff), it won't be properly
  // saved on session shutdown.
  // I'm not actually sure why it doesn't work, but saveState()
  // doesn't seem to be called on session shutdown, possibly
  // because disabling session management after konqueror startup
  // disabled it somehow. Forcing saveState() here for this application
  // seems to fix it.
  if( mySmcConnection ) {
        SmcRequestSaveYourself( mySmcConnection, SmSaveLocal, False,
				SmInteractStyleAny,
				False, False );

	// flush the request
	IceFlush(SmcGetIceConnection(mySmcConnection));
  }
#endif
}


bool KApplication::requestShutDown(
    ShutdownConfirm confirm, ShutdownType sdtype, ShutdownMode sdmode )
{
#ifdef Q_WS_X11
    TQApplication::syncX();
    /*  use ksmserver's dcop interface if necessary  */
    if ( confirm == ShutdownConfirmYes ||
         sdtype != ShutdownTypeDefault ||
         sdmode != ShutdownModeDefault )
    {
        TQByteArray data;
        TQDataStream arg(data, IO_WriteOnly);
        arg << (int)confirm << (int)sdtype << (int)sdmode;
	return dcopClient()->send( "ksmserver", "ksmserver",
                                   "logout(int,int,int)", data );
    }

    if ( mySmcConnection ) {
        // we already have a connection to the session manager, use it.
        SmcRequestSaveYourself( mySmcConnection, SmSaveBoth, True,
				SmInteractStyleAny,
				confirm == ShutdownConfirmNo, True );

	// flush the request
	IceFlush(SmcGetIceConnection(mySmcConnection));
        return true;
    }

    // open a temporary connection, if possible

    propagateSessionManager();
    TQCString smEnv = ::getenv("SESSION_MANAGER");
    if (smEnv.isEmpty())
        return false;

    if (! tmpSmcConnection) {
	char cerror[256];
	char* myId = 0;
	char* prevId = 0;
	SmcCallbacks cb;
	tmpSmcConnection = SmcOpenConnection( 0, 0, 1, 0,
					      0, &cb,
					      prevId,
					      &myId,
					      255,
					      cerror );
	::free( myId ); // it was allocated by C
	if (!tmpSmcConnection )
	    return false;
    }

    SmcRequestSaveYourself( tmpSmcConnection, SmSaveBoth, True,
			    SmInteractStyleAny, False, True );

    // flush the request
    IceFlush(SmcGetIceConnection(tmpSmcConnection));
    return true;
#else
    // FIXME(E): Implement for Qt Embedded
    return false;
#endif
}

void KApplication::propagateSessionManager()
{
#ifdef Q_WS_X11
    TQCString fName = TQFile::encodeName(locateLocal("socket", "KSMserver"));
    TQCString display = ::getenv(DISPLAY);
    // strip the screen number from the display
    display.replace(TQRegExp("\\.[0-9]+$"), "");
    int i;
    while( (i = display.find(':')) >= 0)
       display[i] = '_';

    fName += "_"+display;
    TQCString smEnv = ::getenv("SESSION_MANAGER");
    bool check = smEnv.isEmpty();
    if ( !check && smModificationTime ) {
         TQFileInfo info( fName );
         TQTime current = TQT_TQTIME_OBJECT(info.lastModified().time());
         check = current > *smModificationTime;
    }
    if ( check ) {
        delete smModificationTime;
        TQFile f( fName );
        if ( !f.open( IO_ReadOnly ) )
            return;
        TQFileInfo info ( f );
        smModificationTime = new TQTime( TQT_TQTIME_OBJECT(info.lastModified().time()) );
        TQTextStream t(&f);
        t.setEncoding( TQTextStream::Latin1 );
        TQString s = t.readLine();
        f.close();
        ::setenv( "SESSION_MANAGER", s.latin1(), true  );
    }
#endif
}

void KApplication::commitData( TQSessionManager& sm )
{
    d->session_save = true;
    bool canceled = false;
    for (KSessionManaged* it = sessionClients()->first();
         it && !canceled;
         it = sessionClients()->next() ) {
        canceled = !it->commitData( sm );
    }
    if ( canceled )
        sm.cancel();

    if ( sm.allowsInteraction() ) {
        TQWidgetList done;
        TQWidgetList *list = TQApplication::topLevelWidgets();
        bool canceled = false;
        TQWidget* w = list->first();
        while ( !canceled && w ) {
            if ( !w->testWState( WState_ForceHide ) && !w->inherits("KMainWindow") ) {
                TQCloseEvent e;
                sendEvent( w, &e );
                canceled = !e.isAccepted();
                if ( !canceled )
                    done.append( w );
                delete list; // one never knows...
                list = TQApplication::topLevelWidgets();
                w = list->first();
            } else {
                w = list->next();
            }
            while ( w && done.containsRef( w ) )
                w = list->next();
        }
        delete list;
    }


    if ( !bSessionManagement )
        sm.setRestartHint( TQSessionManager::RestartNever );
    else
	sm.setRestartHint( TQSessionManager::RestartIfRunning );
    d->session_save = false;
}

static void checkRestartVersion( TQSessionManager& sm )
{
    Display* dpy = qt_xdisplay();
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char* data;
    if( XGetWindowProperty( dpy, RootWindow( dpy, 0 ), XInternAtom( dpy, "TDE_FULL_SESSION", False ),
        0, 1, False, AnyPropertyType, &type, &format, &nitems, &after, &data ) == Success ) {
        if( data != NULL )
            XFree( data );
        if( type == XA_STRING && format == 8 ) { // session set, check if TDE_SESSION_VERSION is not set (meaning KDE3)
            if( XGetWindowProperty( dpy, RootWindow( dpy, 0 ), XInternAtom( dpy, "TDE_SESSION_VERSION", False ),
                0, 1, False, AnyPropertyType, &type, &format, &nitems, &after, &data ) == Success ) {
                if( data != NULL )
                    XFree( data ); // KDE4 or newer
                if( type == None )
                    return; // we run in our native session, no need to wrap
            } else {
                return; // we run in our native session, no need to wrap
            }
        }
    }
    TQString wrapper = KStandardDirs::findExe( "trinity" );
    TQStringList restartCommand = sm.restartCommand();
    restartCommand.prepend( wrapper );
    sm.setRestartCommand( restartCommand );
}

void KApplication::saveState( TQSessionManager& sm )
{
    d->session_save = true;
#ifdef Q_WS_X11
    static bool firstTime = true;
    mySmcConnection = (SmcConn) sm.handle();

    if ( !bSessionManagement ) {
        sm.setRestartHint( TQSessionManager::RestartNever );
	d->session_save = false;
        return;
    }
    else
	sm.setRestartHint( TQSessionManager::RestartIfRunning );

    if ( firstTime ) {
        firstTime = false;
	d->session_save = false;
        return; // no need to save the state.
    }

    // remove former session config if still existing, we want a new
    // and fresh one. Note that we do not delete the config file here,
    // this is done by the session manager when it executes the
    // discard commands. In fact it would be harmful to remove the
    // file here, as the session might be stored under a different
    // name, meaning the user still might need it eventually.
    if ( pSessionConfig ) {
        delete pSessionConfig;
        pSessionConfig = 0;
    }

    // tell the session manager about our new lifecycle
    TQStringList restartCommand = sm.restartCommand();

    TQCString multiHead = getenv("TDE_MULTIHEAD");
    if (multiHead.lower() == "true") {
        // if multihead is enabled, we save our -display argument so that
        // we are restored onto the correct head... one problem with this
        // is that the display is hard coded, which means we cannot restore
        // to a different display (ie. if we are in a university lab and try,
        // try to restore a multihead session, our apps could be started on
        // someone else's display instead of our own)
        TQCString displayname = getenv(DISPLAY);
        if (! displayname.isNull()) {
            // only store the command if we actually have a DISPLAY
            // environment variable
            restartCommand.append("-display");
            restartCommand.append(displayname);
        }
        sm.setRestartCommand( restartCommand );
    }

    checkRestartVersion( sm );

    // finally: do session management
    emit saveYourself(); // for compatibility
    bool canceled = false;
    for (KSessionManaged* it = sessionClients()->first();
         it && !canceled;
         it = sessionClients()->next() ) {
        canceled = !it->saveState( sm );
    }

    // if we created a new session config object, register a proper discard command
    if ( pSessionConfig ) {
        pSessionConfig->sync();
        TQStringList discard;
        discard  << "rm" << locateLocal("config", sessionConfigName());
        sm.setDiscardCommand( discard );
    } else {
	sm.setDiscardCommand( TQStringList("") );
    }

    if ( canceled )
        sm.cancel();
#else
    // FIXME(E): Implement for Qt Embedded
#endif
    d->session_save = false;
}

bool KApplication::sessionSaving() const
{
    return d->session_save;
}

void KApplication::startKdeinit()
{
#ifndef Q_WS_WIN //TODO
  KInstance inst( "starttdeinitlock" );
  KLockFile lock( locateLocal( "tmp", "starttdeinitlock", &inst ));
  if( lock.lock( KLockFile::LockNoBlock ) != KLockFile::LockOK ) {
     lock.lock();
     DCOPClient cl;
     if( cl.attach())
         return; // whoever held the lock has already started dcopserver
  }
  // Try to launch tdeinit.
  TQString srv = KStandardDirs::findExe(TQString::fromLatin1("tdeinit"));
  if (srv.isEmpty())
     srv = KStandardDirs::findExe(TQString::fromLatin1("tdeinit"), KGlobal::dirs()->kfsstnd_defaultbindir());
  if (srv.isEmpty())
     return;
  if (kapp && (Tty != kapp->type()))
    setOverrideCursor( tqwaitCursor );
  my_system(TQFile::encodeName(srv)+" --suicide"+" --new-startup");
  if (kapp && (Tty != kapp->type()))
    restoreOverrideCursor();
#endif
}

void KApplication::dcopFailure(const TQString &msg)
{
  static int failureCount = 0;
  failureCount++;
  if (failureCount == 1)
  {
     startKdeinit();
     return;
  }
  if (failureCount == 2)
  {
#ifdef Q_WS_WIN
     KGlobal::config()->setGroup("General");
     if (KGlobal::config()->readBoolEntry("ignoreDCOPFailures", false))
         return;
#endif
     TQString msgStr(i18n("There was an error setting up inter-process "
                      "communications for KDE. The message returned "
                      "by the system was:\n\n"));
     msgStr += msg;
     msgStr += i18n("\n\nPlease check that the \"dcopserver\" program is running!");

     if (Tty != kapp->type())
     {
       TQMessageBox::critical
         (
           kapp->mainWidget(),
           i18n("DCOP communications error (%1)").arg(kapp->caption()),
           msgStr,
           i18n("&OK")
         );
     }
     else
     {
       fprintf(stderr, "%s\n", msgStr.local8Bit().data());
     }

     return;
  }
}

static const KCmdLineOptions qt_options[] =
{
  //FIXME: Check if other options are specific to Qt/X11
#ifdef Q_WS_X11
   { "display <displayname>", I18N_NOOP("Use the X-server display 'displayname'"), 0},
#else
   { "display <displayname>", I18N_NOOP("Use the QWS display 'displayname'"), 0},
#endif
   { "session <sessionId>", I18N_NOOP("Restore the application for the given 'sessionId'"), 0},
   { "cmap", I18N_NOOP("Causes the application to install a private color\nmap on an 8-bit display"), 0},
   { "ncols <count>", I18N_NOOP("Limits the number of colors allocated in the color\ncube on an 8-bit display, if the application is\nusing the TQApplication::ManyColor color\nspecification"), 0},
   { "nograb", I18N_NOOP("tells Qt to never grab the mouse or the keyboard"), 0},
   { "dograb", I18N_NOOP("running under a debugger can cause an implicit\n-nograb, use -dograb to override"), 0},
   { "sync", I18N_NOOP("switches to synchronous mode for debugging"), 0},
   { "fn", 0, 0},
   { "font <fontname>", I18N_NOOP("defines the application font"), 0},
   { "bg", 0, 0},
   { "background <color>", I18N_NOOP("sets the default background color and an\napplication palette (light and dark shades are\ncalculated)"), 0},
   { "fg", 0, 0},
   { "foreground <color>", I18N_NOOP("sets the default foreground color"), 0},
   { "btn", 0, 0},
   { "button <color>", I18N_NOOP("sets the default button color"), 0},
   { "name <name>", I18N_NOOP("sets the application name"), 0},
   { "title <title>", I18N_NOOP("sets the application title (caption)"), 0},
#ifdef Q_WS_X11
   { "visual TrueColor", I18N_NOOP("forces the application to use a TrueColor visual on\nan 8-bit display"), 0},
   { "inputstyle <inputstyle>", I18N_NOOP("sets XIM (X Input Method) input style. Possible\nvalues are onthespot, overthespot, offthespot and\nroot"), 0 },
   { "im <XIM server>", I18N_NOOP("set XIM server"),0},
   { "noxim", I18N_NOOP("disable XIM"), 0 },
#endif
#ifdef Q_WS_QWS
   { "qws", I18N_NOOP("forces the application to run as QWS Server"), 0},
#endif
   { "reverse", I18N_NOOP("mirrors the whole layout of widgets"), 0},
   KCmdLineLastOption
};

static const KCmdLineOptions kde_options[] =
{
   { "caption <caption>",       I18N_NOOP("Use 'caption' as name in the titlebar"), 0},
   { "icon <icon>",             I18N_NOOP("Use 'icon' as the application icon"), 0},
   { "miniicon <icon>",         I18N_NOOP("Use 'icon' as the icon in the titlebar"), 0},
   { "config <filename>",       I18N_NOOP("Use alternative configuration file"), 0},
   { "dcopserver <server>",     I18N_NOOP("Use the DCOP Server specified by 'server'"), 0},
   { "nocrashhandler",          I18N_NOOP("Disable crash handler, to get core dumps"), 0},
   { "waitforwm",          I18N_NOOP("Waits for a WM_NET compatible windowmanager"), 0},
   { "style <style>", I18N_NOOP("sets the application GUI style"), 0},
   { "geometry <geometry>", I18N_NOOP("sets the client geometry of the main widget - see man X for the argument format"), 0},
   { "smkey <sessionKey>", 0, 0}, // this option is obsolete and exists only to allow smooth upgrades from sessions
                                  // saved under Qt 3.0.x -- Qt 3.1.x includes the session key now automatically in
				  // the session id (Simon)
   KCmdLineLastOption
};

void
KApplication::addCmdLineOptions()
{
   KCmdLineArgs::addCmdLineOptions(qt_options, "Qt", "qt");
   KCmdLineArgs::addCmdLineOptions(kde_options, "KDE", "kde");
}

void KApplication::parseCommandLine( )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");

    if ( !args ) return;

    if (args->isSet("config"))
    {
        TQString config = TQString::fromLocal8Bit(args->getOption("config"));
        setConfigName(config);
    }

    if (args->isSet("style"))
    {

        TQStringList plugins = KGlobal::dirs()->resourceDirs( "qtplugins" );
        TQStringList::Iterator itp = plugins.begin();
        while (itp != plugins.end()) {
            addLibraryPath( *itp );
            ++itp;
        }

       TQStringList styles = TQStyleFactory::keys();
       TQString reqStyle(args->getOption("style").lower());

    TQStringList list = libraryPaths();
    TQStringList::Iterator it = list.begin();
    while( it != list.end() ) {
        ++it;
    }

	   for (TQStringList::ConstIterator it = styles.begin(); it != styles.end(); ++it) {
		   if ((*it).lower() == reqStyle)
		   {
			   d->overrideStyle = *it;
			   break;
		   }
	   }

       if (d->overrideStyle.isEmpty())
          fprintf(stderr, "%s", TQString(i18n("The style %1 was not found\n").arg(reqStyle)).local8Bit().data());
    }

    if (args->isSet("caption"))
    {
       aCaption = TQString::fromLocal8Bit(args->getOption("caption"));
    }

    if (args->isSet("miniicon"))
    {
       const char *tmp = args->getOption("miniicon");
       if (!aIconPixmap.pm.miniIcon) {
         aIconPixmap.pm.miniIcon = new TQPixmap;
       }
       *aIconPixmap.pm.miniIcon = SmallIcon(tmp);
       aMiniIconName = tmp;
    }

    if (args->isSet("icon"))
    {
       const char *tmp = args->getOption("icon");
       if (!aIconPixmap.pm.icon) {
          aIconPixmap.pm.icon = new TQPixmap;
       }
       *aIconPixmap.pm.icon = DesktopIcon( tmp );
       aIconName = tmp;
       if (!aIconPixmap.pm.miniIcon) {
         aIconPixmap.pm.miniIcon = new TQPixmap;
       }
       if (aIconPixmap.pm.miniIcon->isNull())
       {
          *aIconPixmap.pm.miniIcon = SmallIcon( tmp );
          aMiniIconName = tmp;
       }
    }

    bool nocrashhandler = (getenv("KDE_DEBUG") != NULL);
    if (!nocrashhandler && args->isSet("crashhandler"))
    {
        // set default crash handler / set emergency save function to nothing
        KCrash::setCrashHandler(KCrash::defaultCrashHandler);
        KCrash::setEmergencySaveFunction(NULL);

        KCrash::setApplicationName(TQString(args->appName()));
    }

#ifdef Q_WS_X11
    if ( args->isSet( "waitforwm" ) ) {
        Atom type;
        (void) desktop(); // trigger desktop creation, we need PropertyNotify events for the root window
        int format;
        unsigned long length, after;
        unsigned char *data;
        while ( XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), atom_NetSupported,
				    0, 1, false, AnyPropertyType, &type, &format,
                                    &length, &after, &data ) != Success || !length ) {
            if ( data )
                XFree( data );
            XEvent event;
            XWindowEvent( qt_xdisplay(), qt_xrootwin(), PropertyChangeMask, &event );
        }
        if ( data )
            XFree( data );
    }
#else
    // FIXME(E): Implement for Qt Embedded
#endif

    if (args->isSet("geometry"))
    {
        d->geometry_arg = args->getOption("geometry");
    }

    if (args->isSet("smkey"))
    {
        d->sessionKey = args->getOption("smkey");
    }

}

TQString KApplication::geometryArgument() const
{
    return d->geometry_arg;
}

TQPixmap KApplication::icon() const
{
  if( !aIconPixmap.pm.icon) {
      aIconPixmap.pm.icon = new TQPixmap;
  }
  if( aIconPixmap.pm.icon->isNull()) {
      *aIconPixmap.pm.icon = DesktopIcon( instanceName() );
  }
  return *aIconPixmap.pm.icon;
}

TQString KApplication::iconName() const
{
  return aIconName.isNull() ? (TQString)instanceName() : aIconName;
}

TQPixmap KApplication::miniIcon() const
{
  if (!aIconPixmap.pm.miniIcon) {
      aIconPixmap.pm.miniIcon = new TQPixmap;
  }
  if (aIconPixmap.pm.miniIcon->isNull()) {
      *aIconPixmap.pm.miniIcon = SmallIcon( instanceName() );
  }
  return *aIconPixmap.pm.miniIcon;
}

TQString KApplication::miniIconName() const
{
  return aMiniIconName.isNull() ? (TQString)instanceName() : aMiniIconName;
}

extern void kDebugCleanup();

KApplication::~KApplication()
{
  delete aIconPixmap.pm.miniIcon;
  aIconPixmap.pm.miniIcon = 0L;
  delete aIconPixmap.pm.icon;
  aIconPixmap.pm.icon = 0L;
  delete d->m_KAppDCOPInterface;

  // First call the static deleters and then call KLibLoader::cleanup()
  // The static deleters may delete libraries for which they need KLibLoader.
  // KLibLoader will take care of the remaining ones.
  KGlobal::deleteStaticDeleters();
  KLibLoader::cleanUp();

  delete smw;

  // close down IPC
  delete s_DCOPClient;
  s_DCOPClient = 0L;

  KProcessController::deref();

#ifdef Q_WS_X11
  if ( d->oldXErrorHandler != NULL )
      XSetErrorHandler( d->oldXErrorHandler );
  if ( d->oldXIOErrorHandler != NULL )
      XSetIOErrorHandler( d->oldXIOErrorHandler );
  if ( d->oldIceIOErrorHandler != NULL )
      IceSetIOErrorHandler( d->oldIceIOErrorHandler );
#endif

  delete d;
  KApp = 0;

#ifdef Q_WS_X11
  mySmcConnection = 0;
  delete smModificationTime;
  smModificationTime = 0;

  // close the temporary smc connection
  if (tmpSmcConnection) {
      SmcCloseConnection( tmpSmcConnection, 0, 0 );
      tmpSmcConnection = 0;
  }
#else
  // FIXME(E): Implement for Qt Embedded
#endif
}


#ifdef Q_WS_X11
class KAppX11HackWidget: public TQWidget
{
public:
    bool publicx11Event( XEvent * e) { return x11Event( e ); }
};
#endif

#if defined(Q_WS_X11) && defined(COMPOSITE)
bool KApplication::isCompositionManagerAvailable() {
	bool have_manager = false;
	const char *home;
	struct passwd *p;
	p = getpwuid(getuid());
	if (p)
		home = p->pw_dir;
	else
		home = getenv("HOME");

	char *filename;
	const char *configfile = "/.kompmgr.available";
	int n = strlen(home)+strlen(configfile)+1;
	filename = (char*)malloc(n*sizeof(char));
	memset(filename,0,n);
	strcat(filename, home);
	strcat(filename, configfile);

        // Now that we did all that by way of introduction...read the file!
	FILE *pFile;
	char buffer[255];
	pFile = fopen(filename, "r");
	int kompmgrpid = 0;
	if (pFile) {
		have_manager = true;
		fclose(pFile);
	}

	free(filename);
	filename = NULL;

	return have_manager;
}

bool KApplication::detectCompositionManagerAvailable(bool force_available, bool available) {
	bool compositing_manager_available;
	if (force_available) {
		compositing_manager_available = available;
	}
	else {
		// See if compositing has been enabled
		KCmdLineArgs *qtargs = KCmdLineArgs::parsedArgs("qt");
		char *displayname = 0;
		if ( qtargs->isSet("display"))
			displayname = qtargs->getOption( "display" ).data();

		Display *dpy = XOpenDisplay( displayname );
	
		x11_composite_error_generated = false;
		compositing_manager_available = false;
		XSetErrorHandler(x11_error);
		if (!XQueryExtension (dpy, COMPOSITE_NAME, &composite_opcode, &composite_event, &composite_error)) {
			XSetErrorHandler(NULL);
			compositing_manager_available = false;
		}
		else {
			Window root_window = XDefaultRootWindow(dpy);
			XCompositeRedirectSubwindows(dpy, root_window, CompositeRedirectManual);
			XSync(dpy, false);
			if (x11_composite_error_generated == true) {
				compositing_manager_available = true;
			}
			else {
				XCompositeUnredirectSubwindows(dpy, root_window, CompositeRedirectManual);
				compositing_manager_available = false;
			}
			XSetErrorHandler(NULL);
			XCloseDisplay(dpy);
		}
	}
	
	const char *home;
	struct passwd *p;
	p = getpwuid(getuid());
	if (p)
		home = p->pw_dir;
	else
		home = getenv("HOME");

	char *filename;
	const char *configfile = "/.kompmgr.available";
	int n = strlen(home)+strlen(configfile)+1;
	filename = (char*)malloc(n*sizeof(char));
	memset(filename,0,n);
	strcat(filename, home);
	strcat(filename, configfile);

	/* now that we did all that by way of introduction...create or remove the file! */
	if (compositing_manager_available) {
		FILE *pFile;
		char buffer[255];
		sprintf(buffer, "available");
		pFile = fopen(filename, "w");
		if (pFile) {
			fwrite(buffer,1,strlen(buffer), pFile);
			fclose(pFile);
		}
	}
	else {
		unlink(filename);
	}

	free(filename);
	filename = NULL;

	return compositing_manager_available;
}

Display* KApplication::openX11RGBADisplay() {
	KCmdLineArgs *qtargs = KCmdLineArgs::parsedArgs("qt");
	char *display = 0;
	if ( qtargs->isSet("display"))
		display = qtargs->getOption( "display" ).data();

	Display *dpy = XOpenDisplay( display );
	if ( !dpy ) {
		kdError() << "cannot connect to X server " << display << endl;
		exit( 1 );
	}

	return dpy;
}

Qt::HANDLE KApplication::getX11RGBAVisual(Display *dpy) {
	getX11RGBAInformation(dpy);
	if (KApplication::isCompositionManagerAvailable() == true) {
		return argb_x11_visual;
	}
	else {
		return NULL;
	}
}

Qt::HANDLE KApplication::getX11RGBAColormap(Display *dpy) {
	getX11RGBAInformation(dpy);
	if (KApplication::isCompositionManagerAvailable() == true) {
		return argb_x11_colormap;
	}
	else {
		return NULL;
	}
}

bool KApplication::isX11CompositionAvailable() {
	return (argb_visual & isCompositionManagerAvailable());
}

void KApplication::getX11RGBAInformation(Display *dpy) {
	if ( !dpy ) {
		argb_visual = false;
		return;
	}

	int screen = DefaultScreen( dpy );
	Colormap colormap = 0;
	Visual *visual = 0;
	int event_base, error_base;
	
	if ( XRenderQueryExtension( dpy, &event_base, &error_base ) ) {
		int nvi;
		XVisualInfo templ;
		templ.screen  = screen;
		templ.depth   = 32;
		templ.c_class = TrueColor;
		XVisualInfo *xvi = XGetVisualInfo( dpy, VisualScreenMask | VisualDepthMask
				| VisualClassMask, &templ, &nvi );
		
		for ( int i = 0; i < nvi; i++ ) {
			XRenderPictFormat *format = XRenderFindVisualFormat( dpy, xvi[i].visual );
			if ( format->type == PictTypeDirect && format->direct.alphaMask ) {
				visual = xvi[i].visual;
				colormap = XCreateColormap( dpy, RootWindow( dpy, screen ), visual, AllocNone );
				kdDebug() << "found visual with alpha support" << endl;
				argb_visual = true;
				break;
			}
		}
	}
	
	if( argb_visual ) {
		argb_x11_visual = Qt::HANDLE( visual );
		argb_x11_colormap = Qt::HANDLE( colormap );
		argb_visual = true;
		return;
	}
	argb_visual = false;
	return;
}
#else
void KApplication::getX11RGBAInformation(Display *dpy) {
}

bool KApplication::isCompositionManagerAvailable() {
	return false;
}

bool KApplication::detectCompositionManagerAvailable(bool force_available) {
	const char *home;
	struct passwd *p;
	p = getpwuid(getuid());
	if (p)
		home = p->pw_dir;
	else
		home = getenv("HOME");

	char *filename;
	const char *configfile = "/.kompmgr.available";
	int n = strlen(home)+strlen(configfile)+1;
	filename = (char*)malloc(n*sizeof(char));
	memset(filename,0,n);
	strcat(filename, home);
	strcat(filename, configfile);

	/* now that we did all that by way of introduction...remove the file! */
	unlink(filename);

	free(filename);
	filename = NULL;

	return false;
}

Display* KApplication::openX11RGBADisplay() {
	return 0;
}

Qt::HANDLE KApplication::getX11RGBAVisual(char *display) {
	return 0;
}

Qt::HANDLE KApplication::getX11RGBAColormap(char *display) {
	return 0;
}

bool KApplication::isX11CompositionAvailable() {
	return false;
}

KApplication KApplication::KARGBApplication( bool allowStyles ) {
	return KApplication::KApplication(allowStyles, true);
}
#endif

static bool kapp_block_user_input = false;

void KApplication::dcopBlockUserInput( bool b )
{
    kapp_block_user_input = b;
}

#ifdef Q_WS_X11
bool KApplication::x11EventFilter( XEvent *_event )
{
    switch ( _event->type ) {
        case ClientMessage:
        {
#if KDE_IS_VERSION( 3, 90, 90 )
#warning This should be already in Qt, check.
#endif
        // Workaround for focus stealing prevention not working when dragging e.g. text from KWrite
        // to KDesktop -> the dialog asking for filename doesn't get activated. This is because
        // Qt-3.2.x doesn't have concept of qt_x_user_time at all, and Qt-3.3.0b1 passes the timestamp
        // in the XdndDrop message in incorrect field (and doesn't update qt_x_user_time either).
        // Patch already sent, future Qt version should have this fixed.
            if( _event->xclient.message_type == kde_xdnd_drop )
                { // if the message is XdndDrop
                if( _event->xclient.data.l[ 1 ] == 1 << 24     // and it's broken the way it's in Qt-3.2.x
                    && _event->xclient.data.l[ 2 ] == 0
                    && _event->xclient.data.l[ 4 ] == 0
                    && _event->xclient.data.l[ 3 ] != 0 )
                    {
                    if( GET_QT_X_USER_TIME() == 0
                        || NET::timestampCompare( _event->xclient.data.l[ 3 ], GET_QT_X_USER_TIME() ) > 0 )
                        { // and the timestamp looks reasonable
                          SET_QT_X_USER_TIME(_event->xclient.data.l[ 3 ]); // update our qt_x_user_time from it
                        }
                    }
                else // normal DND, only needed until Qt updates qt_x_user_time from XdndDrop
                    {
                    if( GET_QT_X_USER_TIME() == 0
                        || NET::timestampCompare( _event->xclient.data.l[ 2 ], GET_QT_X_USER_TIME() ) > 0 )
                        { // the timestamp looks reasonable
                          SET_QT_X_USER_TIME(_event->xclient.data.l[ 2 ]); // update our qt_x_user_time from it
                        }
                    }
                }
        }
	default: break;
    }

    if ( kapp_block_user_input ) {
        switch ( _event->type  ) {
        case ButtonPress:
        case ButtonRelease:
        case XKeyPress:
        case XKeyRelease:
        case MotionNotify:
        case EnterNotify:
        case LeaveNotify:
            return true;
        default:
            break;
        }
    }

    if (x11Filter) {
        for (TQWidget *w=x11Filter->first(); w; w=x11Filter->next()) {
            if (((KAppX11HackWidget*) w)->publicx11Event(_event))
                return true;
        }
    }

    if ((_event->type == ClientMessage) &&
            (_event->xclient.message_type == kipcCommAtom))
    {
        XClientMessageEvent *cme = (XClientMessageEvent *) _event;

        int id = cme->data.l[0];
        int arg = cme->data.l[1];
        if ((id < 32) && (kipcEventMask & (1 << id)))
        {
            switch (id)
            {
            case KIPC::StyleChanged:
                KGlobal::config()->reparseConfiguration();
                kdisplaySetStyle();
                break;

            case KIPC::ToolbarStyleChanged:
                KGlobal::config()->reparseConfiguration();
                if (useStyles)
                    emit toolbarAppearanceChanged(arg);
                break;

            case KIPC::PaletteChanged:
                KGlobal::config()->reparseConfiguration();
                kdisplaySetPalette();
                break;

            case KIPC::FontChanged:
                KGlobal::config()->reparseConfiguration();
                KGlobalSettings::rereadFontSettings();
                kdisplaySetFont();
                break;

            case KIPC::BackgroundChanged:
                emit backgroundChanged(arg);
                break;

            case KIPC::SettingsChanged:
                KGlobal::config()->reparseConfiguration();
                if (arg == SETTINGS_PATHS)
                    KGlobalSettings::rereadPathSettings();
                else if (arg == SETTINGS_MOUSE)
                    KGlobalSettings::rereadMouseSettings();
                propagateSettings((SettingsCategory)arg);
                break;

            case KIPC::IconChanged:
                TQPixmapCache::clear();
                KGlobal::config()->reparseConfiguration();
                KGlobal::instance()->newIconLoader();
                emit updateIconLoaders();
                emit iconChanged(arg);
                break;

            case KIPC::ClipboardConfigChanged:
                KClipboardSynchronizer::newConfiguration(arg);
                break;
                
            case KIPC::BlockShortcuts:
                KGlobalAccel::blockShortcuts(arg);
                emit kipcMessage(id, arg); // some apps may do additional things
                break;
            }
        }
        else if (id >= 32)
        {
            emit kipcMessage(id, arg);
        }
        return true;
    }
    return false;
}
#endif // Q_WS_X11

void KApplication::updateUserTimestamp( unsigned long time )
{
#if defined Q_WS_X11
    if( time == 0 )
    { // get current X timestamp
        Window w = XCreateSimpleWindow( qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0, 0, 0 );
        XSelectInput( qt_xdisplay(), w, PropertyChangeMask );
        unsigned char data[ 1 ];
        XChangeProperty( qt_xdisplay(), w, XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
        XEvent ev;
        XWindowEvent( qt_xdisplay(), w, PropertyChangeMask, &ev );
        time = ev.xproperty.time;
        XDestroyWindow( qt_xdisplay(), w );
    }
    if( GET_QT_X_USER_TIME() == 0
        || NET::timestampCompare( time, GET_QT_X_USER_TIME() ) > 0 ) // check time > qt_x_user_time
        SET_QT_X_USER_TIME(time);
#endif
}

unsigned long KApplication::userTimestamp() const
{
#if defined Q_WS_X11
    return GET_QT_X_USER_TIME();
#else
    return 0;
#endif
}

void KApplication::updateRemoteUserTimestamp( const TQCString& dcopId, unsigned long time )
{
#if defined Q_WS_X11
    if( time == 0 )
        time = GET_QT_X_USER_TIME();
    DCOPRef( dcopId, "MainApplication-Interface" ).call( "updateUserTimestamp", time );
#endif
}

void KApplication::invokeEditSlot( const char *slot )
{
  TQObject *object = TQT_TQOBJECT(focusWidget());
  if( !object )
    return;

  TQMetaObject *meta = object->metaObject();

  int idx = meta->findSlot( slot + 1, true );
  if( idx < 0 )
    return;

  object->qt_invoke( idx, 0 );
}

void KApplication::addKipcEventMask(int id)
{
    if (id >= 32)
    {
        kdDebug(101) << "Cannot use KIPC event mask for message IDs >= 32\n";
        return;
    }
    kipcEventMask |= (1 << id);
}

void KApplication::removeKipcEventMask(int id)
{
    if (id >= 32)
    {
        kdDebug(101) << "Cannot use KIPC event mask for message IDs >= 32\n";
        return;
    }
    kipcEventMask &= ~(1 << id);
}

void KApplication::enableStyles()
{
    if (!useStyles)
    {
        useStyles = true;
        applyGUIStyle();
    }
}

void KApplication::disableStyles()
{
    useStyles = false;
}

void KApplication::applyGUIStyle()
{
    if ( !useStyles ) return;

    KConfigGroup pConfig (KGlobal::config(), "General");
    TQString defaultStyle = KStyle::defaultStyle();
    TQString styleStr = pConfig.readEntry("widgetStyle", defaultStyle);

    if (d->overrideStyle.isEmpty()) {
      // ### add check whether we already use the correct style to return then
      // (workaround for Qt misbehavior to avoid double style initialization)

      TQStyle* sp = TQStyleFactory::create( styleStr );

      // If there is no default style available, try falling back any available style
      if ( !sp && styleStr != defaultStyle)
          sp = TQStyleFactory::create( defaultStyle );
      if ( !sp )
          sp = TQStyleFactory::create( *(TQStyleFactory::keys().begin()) );
      setStyle(sp);
    }
    else
        setStyle(d->overrideStyle);
    // Reread palette from config file.
    kdisplaySetPalette();
}

TQString KApplication::caption() const
{
  // Caption set from command line ?
  if( !aCaption.isNull() )
        return aCaption;
  else
      // We have some about data ?
      if ( KGlobal::instance()->aboutData() )
        return KGlobal::instance()->aboutData()->programName();
      else
        // Last resort : application name
        return name();
}


//
// 1999-09-20: Espen Sand
// An attempt to simplify consistent captions.
//
TQString KApplication::makeStdCaption( const TQString &userCaption,
                                      bool withAppName, bool modified ) const
{
  TQString s = userCaption.isEmpty() ? caption() : userCaption;

  // If the document is modified, add '[modified]'.
  if (modified)
      s += TQString::fromUtf8(" [") + i18n("modified") + TQString::fromUtf8("]");

  if ( !userCaption.isEmpty() ) {
      // Add the application name if:
      // User asked for it, it's not a duplication  and the app name (caption()) is not empty
      if ( withAppName && !caption().isNull() && !userCaption.endsWith(caption())  )
	  s += TQString::fromUtf8(" - ") + caption();
  }

  return s;
}

TQPalette KApplication::createApplicationPalette()
{
    KConfig *config = KGlobal::config();
    KConfigGroupSaver saver( config, "General" );
    return createApplicationPalette( config, KGlobalSettings::contrast() );
}

TQPalette KApplication::createApplicationPalette( KConfig *config, int contrast_ )
{
    TQColor trinity4Background( 239, 239, 239 );
    TQColor trinity4Blue( 103,141,178 );

    TQColor trinity4Button;
    if ( TQPixmap::defaultDepth() > 8 )
      trinity4Button.setRgb( 221, 223, 228 );
    else
      trinity4Button.setRgb( 220, 220, 220 );

    TQColor trinity4Link( 0, 0, 238 );
    TQColor trinity4VisitedLink( 82, 24, 139 );

    TQColor background = config->readColorEntry( "background", &trinity4Background );
    TQColor foreground = config->readColorEntry( "foreground", tqblackptr );
    TQColor button = config->readColorEntry( "buttonBackground", &trinity4Button );
    TQColor buttonText = config->readColorEntry( "buttonForeground", tqblackptr );
    TQColor highlight = config->readColorEntry( "selectBackground", &trinity4Blue );
    TQColor highlightedText = config->readColorEntry( "selectForeground", tqwhiteptr );
    TQColor base = config->readColorEntry( "windowBackground", tqwhiteptr );
    TQColor baseText = config->readColorEntry( "windowForeground", tqblackptr );
    TQColor link = config->readColorEntry( "linkColor", &trinity4Link );
    TQColor visitedLink = config->readColorEntry( "visitedLinkColor", &trinity4VisitedLink );

    int highlightVal, lowlightVal;
    highlightVal = 100 + (2*contrast_+4)*16/10;
    lowlightVal = 100 + (2*contrast_+4)*10;

    TQColor disfg = foreground;

    int h, s, v;
    disfg.hsv( &h, &s, &v );
    if (v > 128)
	// dark bg, light fg - need a darker disabled fg
	disfg = disfg.dark(lowlightVal);
    else if (disfg != Qt::black)
	// light bg, dark fg - need a lighter disabled fg - but only if !black
	disfg = disfg.light(highlightVal);
    else
	// black fg - use darkgray disabled fg
	disfg = Qt::darkGray;


    TQColorGroup disabledgrp(disfg, background,
                            background.light(highlightVal),
                            background.dark(lowlightVal),
                            background.dark(120),
                            background.dark(120), base);

    TQColorGroup colgrp(foreground, background, background.light(highlightVal),
                       background.dark(lowlightVal),
                       background.dark(120),
                       baseText, base);

    int inlowlightVal = lowlightVal-25;
    if(inlowlightVal < 120)
        inlowlightVal = 120;

    colgrp.setColor(TQColorGroup::Highlight, highlight);
    colgrp.setColor(TQColorGroup::HighlightedText, highlightedText);
    colgrp.setColor(TQColorGroup::Button, button);
    colgrp.setColor(TQColorGroup::ButtonText, buttonText);
    colgrp.setColor(TQColorGroup::Midlight, background.light(110));
    colgrp.setColor(TQColorGroup::Link, link);
    colgrp.setColor(TQColorGroup::LinkVisited, visitedLink);

    disabledgrp.setColor(TQColorGroup::Button, button);

    TQColor disbtntext = buttonText;
    disbtntext.hsv( &h, &s, &v );
    if (v > 128)
	// dark button, light buttonText - need a darker disabled buttonText
	disbtntext = disbtntext.dark(lowlightVal);
    else if (disbtntext != Qt::black)
	// light buttonText, dark button - need a lighter disabled buttonText - but only if !black
	disbtntext = disbtntext.light(highlightVal);
    else
	// black button - use darkgray disabled buttonText
	disbtntext = Qt::darkGray;

    disabledgrp.setColor(TQColorGroup::ButtonText, disbtntext);
    disabledgrp.setColor(TQColorGroup::Midlight, background.light(110));
    disabledgrp.setColor(TQColorGroup::Highlight, highlight.dark(120));
    disabledgrp.setColor(TQColorGroup::Link, link);
    disabledgrp.setColor(TQColorGroup::LinkVisited, visitedLink);

    return TQPalette(colgrp, disabledgrp, colgrp);
}


void KApplication::kdisplaySetPalette()
{
#ifdef Q_WS_MACX
    //Can I have this on other platforms, please!? --Sam
    {
        KConfig *config = KGlobal::config();
        KConfigGroupSaver saver( config, "General" );
        bool do_not_set_palette = FALSE;
        if(config->readBoolEntry("nopaletteChange", &do_not_set_palette))
            return;
    }
#endif
    TQApplication::setPalette( createApplicationPalette(), true);
    emit kdisplayPaletteChanged();
    emit appearanceChanged();
}


void KApplication::kdisplaySetFont()
{
    TQApplication::setFont(KGlobalSettings::generalFont(), true);
    TQApplication::setFont(KGlobalSettings::menuFont(), true, TQMENUBAR_OBJECT_NAME_STRING);
    TQApplication::setFont(KGlobalSettings::menuFont(), true, TQPOPUPMENU_OBJECT_NAME_STRING);
    TQApplication::setFont(KGlobalSettings::menuFont(), true, "KPopupTitle");

    // "patch" standard TQStyleSheet to follow our fonts
    TQStyleSheet* sheet = TQStyleSheet::defaultSheet();
    sheet->item ("pre")->setFontFamily (KGlobalSettings::fixedFont().family());
    sheet->item ("code")->setFontFamily (KGlobalSettings::fixedFont().family());
    sheet->item ("tt")->setFontFamily (KGlobalSettings::fixedFont().family());

    emit kdisplayFontChanged();
    emit appearanceChanged();
}


void KApplication::kdisplaySetStyle()
{
    if (useStyles)
    {
        applyGUIStyle();
        emit kdisplayStyleChanged();
        emit appearanceChanged();
    }
}


void KApplication::propagateSettings(SettingsCategory arg)
{
    KConfigBase* config = KGlobal::config();
    KConfigGroupSaver saver( config, "KDE" );

#ifdef QT_HAVE_MAX_IMAGE_SIZE
    TQSize maxImageSize(4096, 4096);
    maxImageSize = config->readSizeEntry("MaxImageSize", &maxImageSize);
    TQImage::setMaxImageSize(maxImageSize);
#endif

    int num = config->readNumEntry("CursorBlinkRate", TQApplication::cursorFlashTime());
    if ((num != 0) && (num < 200))
        num = 200;
    if (num > 2000)
        num = 2000;
    TQApplication::setCursorFlashTime(num);
    num = config->readNumEntry("DoubleClickInterval", TQApplication::doubleClickInterval());
    TQApplication::setDoubleClickInterval(num);
    num = config->readNumEntry("StartDragTime", TQApplication::startDragTime());
    TQApplication::setStartDragTime(num);
    num = config->readNumEntry("StartDragDist", TQApplication::startDragDistance());
    TQApplication::setStartDragDistance(num);
    num = config->readNumEntry("WheelScrollLines", TQApplication::wheelScrollLines());
    TQApplication::setWheelScrollLines(num);

    bool b = config->readBoolEntry("EffectAnimateMenu", false);
    TQApplication::setEffectEnabled( Qt::UI_AnimateMenu, b);
    b = config->readBoolEntry("EffectFadeMenu", false);
    TQApplication::setEffectEnabled( Qt::UI_FadeMenu, b);
    b = config->readBoolEntry("EffectAnimateCombo", false);
    TQApplication::setEffectEnabled( Qt::UI_AnimateCombo, b);
    b = config->readBoolEntry("EffectAnimateTooltip", false);
    TQApplication::setEffectEnabled( Qt::UI_AnimateTooltip, b);
    b = config->readBoolEntry("EffectFadeTooltip", false);
    TQApplication::setEffectEnabled( Qt::UI_FadeTooltip, b);
    b = !config->readBoolEntry("EffectNoTooltip", false);
    TQToolTip::setGloballyEnabled( b );

    emit settingsChanged(arg);
}

void KApplication::installKDEPropertyMap()
{
#ifndef QT_NO_SQL
    static bool installed = false;
    if (installed) return;
    installed = true;
    /**
     * If you are adding a widget that was missing please
     * make sure to also add it to KConfigDialogManager's retrieveSettings()
     * function.
     * Thanks.
     */
    // TQSqlPropertyMap takes ownership of the new default map.
    TQSqlPropertyMap *kdeMap = new TQSqlPropertyMap;
    kdeMap->insert( "KColorButton", "color" );
    kdeMap->insert( "KComboBox", "currentItem" );
    kdeMap->insert( "KDatePicker", "date" );
    kdeMap->insert( "KDateWidget", "date" );
    kdeMap->insert( "KDateTimeWidget", "dateTime" );
    kdeMap->insert( "KEditListBox", "items" );
    kdeMap->insert( "KFontCombo", "family" );
    kdeMap->insert( "KFontRequester", "font" );
    kdeMap->insert( "KFontChooser", "font" );
    kdeMap->insert( "KHistoryCombo", "currentItem" );
    kdeMap->insert( "KListBox", "currentItem" );
    kdeMap->insert( "KLineEdit", "text" );
    kdeMap->insert( "KRestrictedLine", "text" );
    kdeMap->insert( "KSqueezedTextLabel", "text" );
    kdeMap->insert( "KTextBrowser", "source" );
    kdeMap->insert( "KTextEdit", "text" );
    kdeMap->insert( "KURLRequester", "url" );
    kdeMap->insert( "KPasswordEdit", "password" );
    kdeMap->insert( "KIntNumInput", "value" );
    kdeMap->insert( "KIntSpinBox", "value" );
    kdeMap->insert( "KDoubleNumInput", "value" );
    // Temp til fixed in QT then enable ifdef with the correct version num
    kdeMap->insert( TQGROUPBOX_OBJECT_NAME_STRING, "checked" );
    kdeMap->insert( TQTABWIDGET_OBJECT_NAME_STRING, "currentPage" );
    TQSqlPropertyMap::installDefaultMap( kdeMap );
#endif
}

void KApplication::invokeHelp( const TQString& anchor,
                               const TQString& _appname) const
{
    return invokeHelp( anchor, _appname, "" );
}

#ifndef Q_WS_WIN
// for win32 we're using simple help tools like Qt Assistant,
// see kapplication_win.cpp
void KApplication::invokeHelp( const TQString& anchor,
                               const TQString& _appname,
                               const TQCString& startup_id ) const
{
   TQString url;
   TQString appname;
   if (_appname.isEmpty())
     appname = name();
   else
     appname = _appname;

   if (!anchor.isEmpty())
     url = TQString("help:/%1?anchor=%2").arg(appname).arg(anchor);
   else
     url = TQString("help:/%1/index.html").arg(appname);

   TQString error;
   if ( !dcopClient()->isApplicationRegistered("khelpcenter") )
   {
       if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, startup_id, false))
       {
           if (Tty != kapp->type())
               TQMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Help Center"),
               i18n("Could not launch the KDE Help Center:\n\n%1").arg(error), i18n("&OK"));
           else
               kdWarning() << "Could not launch help:\n" << error << endl;
	   return;
       }
   }
   else
       DCOPRef( "khelpcenter", "KHelpCenterIface" ).send( "openUrl", url, startup_id );
}
#endif

void KApplication::invokeHTMLHelp( const TQString& _filename, const TQString& topic ) const
{
   kdWarning() << "invoking HTML help is deprecated! use docbook and invokeHelp!\n";

   TQString filename;

   if( _filename.isEmpty() )
     filename = TQString(name()) + "/index.html";
   else
     filename = _filename;

   TQString url;
   if (!topic.isEmpty())
     url = TQString("help:/%1#%2").arg(filename).arg(topic);
   else
     url = TQString("help:/%1").arg(filename);

   TQString error;
   if ( !dcopClient()->isApplicationRegistered("khelpcenter") )
   {
       if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, "", false))
       {
           if (Tty != kapp->type())
               TQMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Help Center"),
               i18n("Could not launch the KDE Help Center:\n\n%1").arg(error), i18n("&OK"));
           else
               kdWarning() << "Could not launch help:\n" << error << endl;
           return;
       }
   }
   else
       DCOPRef( "khelpcenter", "KHelpCenterIface" ).send( "openUrl", url );
}


void KApplication::invokeMailer(const TQString &address, const TQString &subject)
{
    return invokeMailer(address,subject,"");
}

void KApplication::invokeMailer(const TQString &address, const TQString &subject, const TQCString& startup_id)
{
   invokeMailer(address, TQString::null, TQString::null, subject, TQString::null, TQString::null,
       TQStringList(), startup_id );
}

void KApplication::invokeMailer(const KURL &mailtoURL)
{
    return invokeMailer( mailtoURL, "" );
}

void KApplication::invokeMailer(const KURL &mailtoURL, const TQCString& startup_id )
{
    return invokeMailer( mailtoURL, startup_id, false);
}

void KApplication::invokeMailer(const KURL &mailtoURL, const TQCString& startup_id, bool allowAttachments )
{
   TQString address = KURL::decode_string(mailtoURL.path()), subject, cc, bcc, body;
   TQStringList queries = TQStringList::split('&', mailtoURL.query().mid(1));
   TQStringList attachURLs;
   for (TQStringList::Iterator it = queries.begin(); it != queries.end(); ++it)
   {
     TQString q = (*it).lower();
     if (q.startsWith("subject="))
       subject = KURL::decode_string((*it).mid(8));
     else
     if (q.startsWith("cc="))
       cc = cc.isEmpty()? KURL::decode_string((*it).mid(3)): cc + ',' + KURL::decode_string((*it).mid(3));
     else
     if (q.startsWith("bcc="))
       bcc = bcc.isEmpty()? KURL::decode_string((*it).mid(4)): bcc + ',' + KURL::decode_string((*it).mid(4));
     else
     if (q.startsWith("body="))
       body = KURL::decode_string((*it).mid(5));
     else
     if (allowAttachments && q.startsWith("attach="))
       attachURLs.push_back(KURL::decode_string((*it).mid(7)));
     else
     if (allowAttachments && q.startsWith("attachment="))
       attachURLs.push_back(KURL::decode_string((*it).mid(11)));
     else
     if (q.startsWith("to="))
       address = address.isEmpty()? KURL::decode_string((*it).mid(3)): address + ',' + KURL::decode_string((*it).mid(3));
   }

   invokeMailer( address, cc, bcc, subject, body, TQString::null, attachURLs, startup_id );
}

void KApplication::invokeMailer(const TQString &to, const TQString &cc, const TQString &bcc,
                                const TQString &subject, const TQString &body,
                                const TQString & messageFile, const TQStringList &attachURLs)
{
    return invokeMailer(to,cc,bcc,subject,body,messageFile,attachURLs,"");
}

#ifndef Q_WS_WIN
// on win32, for invoking browser we're using win32 API
// see kapplication_win.cpp

static TQStringList splitEmailAddressList( const TQString & aStr )
{
  // This is a copy of KPIM::splitEmailAddrList().
  // Features:
  // - always ignores quoted characters
  // - ignores everything (including parentheses and commas)
  //   inside quoted strings
  // - supports nested comments
  // - ignores everything (including double quotes and commas)
  //   inside comments

  TQStringList list;

  if (aStr.isEmpty())
    return list;

  TQString addr;
  uint addrstart = 0;
  int commentlevel = 0;
  bool insidequote = false;

  for (uint index=0; index<aStr.length(); index++) {
    // the following conversion to latin1 is o.k. because
    // we can safely ignore all non-latin1 characters
    switch (aStr[index].latin1()) {
    case '"' : // start or end of quoted string
      if (commentlevel == 0)
        insidequote = !insidequote;
      break;
    case '(' : // start of comment
      if (!insidequote)
        commentlevel++;
      break;
    case ')' : // end of comment
      if (!insidequote) {
        if (commentlevel > 0)
          commentlevel--;
        else {
          //kdDebug() << "Error in address splitting: Unmatched ')'"
          //          << endl;
          return list;
        }
      }
      break;
    case '\\' : // quoted character
      index++; // ignore the quoted character
      break;
    case ',' :
      if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, index-addrstart);
        if (!addr.isEmpty())
          list += addr.simplifyWhiteSpace();
        addrstart = index+1;
      }
      break;
    }
  }
  // append the last address to the list
  if (!insidequote && (commentlevel == 0)) {
    addr = aStr.mid(addrstart, aStr.length()-addrstart);
    if (!addr.isEmpty())
      list += addr.simplifyWhiteSpace();
  }
  //else
  //  kdDebug() << "Error in address splitting: "
  //            << "Unexpected end of address list"
  //            << endl;

  return list;
}

void KApplication::invokeMailer(const TQString &_to, const TQString &_cc, const TQString &_bcc,
                                const TQString &subject, const TQString &body,
                                const TQString & /*messageFile TODO*/, const TQStringList &attachURLs,
                                const TQCString& startup_id )
{
   KConfig config("emaildefaults");

   config.setGroup("Defaults");
   TQString group = config.readEntry("Profile","Default");

   config.setGroup( TQString("PROFILE_%1").arg(group) );
   TQString command = config.readPathEntry("EmailClient");

   TQString to, cc, bcc;
   if (command.isEmpty() || command == TQString::fromLatin1("kmail")
       || command.endsWith("/kmail"))
   {
     command = TQString::fromLatin1("kmail --composer -s %s -c %c -b %b --body %B --attach %A -- %t");
     if ( !_to.isEmpty() )
     {
       // put the whole address lists into RFC2047 encoded blobs; technically
       // this isn't correct, but KMail understands it nonetheless
       to = TQString( "=?utf8?b?%1?=" )
            .arg( TQString(KCodecs::base64Encode( _to.utf8(), false )) );
     }
     if ( !_cc.isEmpty() )
       cc = TQString( "=?utf8?b?%1?=" )
            .arg( TQString(KCodecs::base64Encode( _cc.utf8(), false )) );
     if ( !_bcc.isEmpty() )
       bcc = TQString( "=?utf8?b?%1?=" )
             .arg( TQString(KCodecs::base64Encode( _bcc.utf8(), false )) );
   } else {
     to = _to;
     cc = _cc;
     bcc = _bcc;
     if( !command.contains( '%' ))
         command += " %u";
   }

   if (config.readBoolEntry("TerminalClient", false))
   {
     KConfigGroup confGroup( KGlobal::config(), "General" );
     TQString preferredTerminal = confGroup.readPathEntry("TerminalApplication", "konsole");
     command = preferredTerminal + " -e " + command;
   }

   TQStringList cmdTokens = KShell::splitArgs(command);
   TQString cmd = cmdTokens[0];
   cmdTokens.remove(cmdTokens.begin());

   KURL url;
   TQStringList qry;
   if (!to.isEmpty())
   {
     TQStringList tos = splitEmailAddressList( to );
     url.setPath( tos.first() );
     tos.remove( tos.begin() );
     for (TQStringList::ConstIterator it = tos.begin(); it != tos.end(); ++it)
       qry.append( "to=" + KURL::encode_string( *it ) );
   }
   const TQStringList ccs = splitEmailAddressList( cc );
   for (TQStringList::ConstIterator it = ccs.begin(); it != ccs.end(); ++it)
      qry.append( "cc=" + KURL::encode_string( *it ) );
   const TQStringList bccs = splitEmailAddressList( bcc );
   for (TQStringList::ConstIterator it = bccs.begin(); it != bccs.end(); ++it)
      qry.append( "bcc=" + KURL::encode_string( *it ) );
   for (TQStringList::ConstIterator it = attachURLs.begin(); it != attachURLs.end(); ++it)
      qry.append( "attach=" + KURL::encode_string( *it ) );
   if (!subject.isEmpty())
      qry.append( "subject=" + KURL::encode_string( subject ) );
   if (!body.isEmpty())
      qry.append( "body=" + KURL::encode_string( body ) );
   url.setQuery( qry.join( "&" ) );
   if ( ! (to.isEmpty() && qry.isEmpty()) )
      url.setProtocol("mailto");

   TQMap<TQChar, TQString> keyMap;
   keyMap.insert('t', to);
   keyMap.insert('s', subject);
   keyMap.insert('c', cc);
   keyMap.insert('b', bcc);
   keyMap.insert('B', body);
   keyMap.insert('u', url.url());

   TQString attachlist = attachURLs.join(",");
   attachlist.prepend('\'');
   attachlist.append('\'');
   keyMap.insert('A', attachlist);

   for (TQStringList::Iterator it = cmdTokens.begin(); it != cmdTokens.end(); )
   {
     if (*it == "%A")
     {
         if (it == cmdTokens.begin()) // better safe than sorry ...
             continue;
         TQStringList::ConstIterator urlit = attachURLs.begin();
         TQStringList::ConstIterator urlend = attachURLs.end();
         if ( urlit != urlend )
         {
             TQStringList::Iterator previt = it;
             --previt;
             *it = *urlit;
             ++it;
             while ( ++urlit != urlend )
             {
                 cmdTokens.insert( it, *previt );
                 cmdTokens.insert( it, *urlit );
             }
         } else {
             --it;
             it = cmdTokens.remove( cmdTokens.remove( it ) );
         }
     } else {
         *it = KMacroExpander::expandMacros(*it, keyMap);
         ++it;
     }
   }

   TQString error;
   // TODO this should check if cmd has a .desktop file, and use data from it, together
   // with sending more ASN data
   if (tdeinitExec(cmd, cmdTokens, &error, NULL, startup_id ))
     if (Tty != kapp->type())
       TQMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Mail Client"),
             i18n("Could not launch the mail client:\n\n%1").arg(error), i18n("&OK"));
     else
       kdWarning() << "Could not launch mail client:\n" << error << endl;
}
#endif

void KApplication::invokeBrowser( const TQString &url )
{
    return invokeBrowser( url, "" );
}

#ifndef Q_WS_WIN
// on win32, for invoking browser we're using win32 API
// see kapplication_win.cpp
void KApplication::invokeBrowser( const TQString &url, const TQCString& startup_id )
{
   TQString error;

   if (startServiceByDesktopName("kfmclient", url, &error, 0, 0, startup_id, false))
   {
      if (Tty != kapp->type())
          TQMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Browser"),
               i18n("Could not launch the browser:\n\n%1").arg(error), i18n("&OK"));
      else
          kdWarning() << "Could not launch browser:\n" << error << endl;
      return;
   }
}
#endif

void KApplication::cut()
{
  invokeEditSlot( TQT_SLOT( cut() ) );
}

void KApplication::copy()
{
  invokeEditSlot( TQT_SLOT( copy() ) );
}

void KApplication::paste()
{
  invokeEditSlot( TQT_SLOT( paste() ) );
}

void KApplication::clear()
{
  invokeEditSlot( TQT_SLOT( clear() ) );
}

void KApplication::selectAll()
{
  invokeEditSlot( TQT_SLOT( selectAll() ) );
}

void KApplication::broadcastKeyCode(unsigned int keyCode)
{
  emit coreFakeKeyPress(keyCode);
}

TQCString
KApplication::launcher()
{
   return "klauncher";
}

static int
startServiceInternal( const TQCString &function,
              const TQString& _name, const TQStringList &URLs,
              TQString *error, TQCString *dcopService, int *pid, const TQCString& startup_id, bool noWait )
{
   struct serviceResult
   {
      int result;
      TQCString dcopName;
      TQString error;
      pid_t pid;
   };

   // Register app as able to send DCOP messages
   DCOPClient *dcopClient;
   if (kapp)
      dcopClient = kapp->dcopClient();
   else
      dcopClient = new DCOPClient;

   if (!dcopClient->isAttached())
   {
      if (!dcopClient->attach())
      {
         if (error)
            *error = i18n("Could not register with DCOP.\n");
         if (!kapp)
            delete dcopClient;

         return -1;
      }
   }
   TQByteArray params;
   TQDataStream stream(params, IO_WriteOnly);
   stream << _name << URLs;
   TQCString replyType;
   TQByteArray replyData;
   TQCString _launcher = KApplication::launcher();
   TQValueList<TQCString> envs;
#ifdef Q_WS_X11
   if (qt_xdisplay()) {
       TQCString dpystring(XDisplayString(qt_xdisplay()));
       envs.append( TQCString("DISPLAY=") + dpystring );
   } else if( getenv( "DISPLAY" )) {
       TQCString dpystring( getenv( "DISPLAY" ));
       envs.append( TQCString("DISPLAY=") + dpystring );
   }
#endif
   stream << envs;
#if defined Q_WS_X11
   // make sure there is id, so that user timestamp exists
   stream << ( startup_id.isEmpty() ? KStartupInfo::createNewStartupId() : startup_id );
#endif
   if( function.left( 12 ) != "tdeinit_exec" )
       stream << noWait;

   if (!dcopClient->call(_launcher, _launcher,
        function, params, replyType, replyData))
   {
        if (error)
           *error = i18n("KLauncher could not be reached via DCOP.\n");
        if (!kapp)
           delete dcopClient;
        return -1;
   }
   if (!kapp)
      delete dcopClient;

   if (noWait)
      return 0;

   TQDataStream stream2(replyData, IO_ReadOnly);
   serviceResult result;
   stream2 >> result.result >> result.dcopName >> result.error >> result.pid;
   if (dcopService)
      *dcopService = result.dcopName;
   if (error)
      *error = result.error;
   if (pid)
      *pid = result.pid;
   return result.result;
}

int
KApplication::startServiceByName( const TQString& _name, const TQString &URL,
                  TQString *error, TQCString *dcopService, int *pid, const TQCString& startup_id, bool noWait )
{
   TQStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByName( const TQString& _name, const TQStringList &URLs,
                  TQString *error, TQCString *dcopService, int *pid, const TQCString& startup_id, bool noWait )
{
   return startServiceInternal(
                      "start_service_by_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByDesktopPath( const TQString& _name, const TQString &URL,
                  TQString *error, TQCString *dcopService, int *pid, const TQCString& startup_id, bool noWait )
{
   TQStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_desktop_path(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByDesktopPath( const TQString& _name, const TQStringList &URLs,
                  TQString *error, TQCString *dcopService, int *pid, const TQCString& startup_id, bool noWait )
{
   return startServiceInternal(
                      "start_service_by_desktop_path(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByDesktopName( const TQString& _name, const TQString &URL,
                  TQString *error, TQCString *dcopService, int *pid, const TQCString& startup_id, bool noWait )
{
   TQStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_desktop_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByDesktopName( const TQString& _name, const TQStringList &URLs,
                  TQString *error, TQCString *dcopService, int *pid, const TQCString& startup_id, bool noWait )
{
   return startServiceInternal(
                      "start_service_by_desktop_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::tdeinitExec( const TQString& name, const TQStringList &args,
                           TQString *error, int *pid )
{
    return tdeinitExec( name, args, error, pid, "" );
}

int
KApplication::tdeinitExec( const TQString& name, const TQStringList &args,
                           TQString *error, int *pid, const TQCString& startup_id )
{
   return startServiceInternal("tdeinit_exec(TQString,TQStringList,TQValueList<TQCString>,TQCString)",
        name, args, error, 0, pid, startup_id, false);
}

int
KApplication::tdeinitExecWait( const TQString& name, const TQStringList &args,
                           TQString *error, int *pid )
{
    return tdeinitExecWait( name, args, error, pid, "" );
}

int
KApplication::tdeinitExecWait( const TQString& name, const TQStringList &args,
                           TQString *error, int *pid, const TQCString& startup_id )
{
   return startServiceInternal("tdeinit_exec_wait(TQString,TQStringList,TQValueList<TQCString>,TQCString)",
        name, args, error, 0, pid, startup_id, false);
}

TQString KApplication::tempSaveName( const TQString& pFilename ) const
{
  TQString aFilename;

  if( TQDir::isRelativePath(pFilename) )
    {
      kdWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
      aFilename = TQFileInfo( TQDir( "." ), pFilename ).absFilePath();
    }
  else
    aFilename = pFilename;

  TQDir aAutosaveDir( TQDir::homeDirPath() + "/autosave/" );
  if( !aAutosaveDir.exists() )
    {
      if( !aAutosaveDir.mkdir( aAutosaveDir.absPath() ) )
        {
          // Last chance: use temp dir
          aAutosaveDir.setPath( KGlobal::dirs()->saveLocation("tmp") );
        }
    }

  aFilename.replace( "/", "\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absPath() );

  return aFilename;
}


TQString KApplication::checkRecoverFile( const TQString& pFilename,
        bool& bRecover ) const
{
  TQString aFilename;

  if( TQDir::isRelativePath(pFilename) )
    {
      kdWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
      aFilename = TQFileInfo( TQDir( "." ), pFilename ).absFilePath();
    }
  else
    aFilename = pFilename;

  TQDir aAutosaveDir( TQDir::homeDirPath() + "/autosave/" );
  if( !aAutosaveDir.exists() )
    {
      if( !aAutosaveDir.mkdir( aAutosaveDir.absPath() ) )
        {
          // Last chance: use temp dir
          aAutosaveDir.setPath( KGlobal::dirs()->saveLocation("tmp") );
        }
    }

  aFilename.replace( "/", "\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absPath() );

  if( TQFile( aFilename ).exists() )
    {
      bRecover = true;
      return aFilename;
    }
  else
    {
      bRecover = false;
      return pFilename;
    }
}


bool checkAccess(const TQString& pathname, int mode)
{
  int accessOK = access( TQFile::encodeName(pathname), mode );
  if ( accessOK == 0 )
    return true;  // OK, I can really access the file

  // else
  // if we want to write the file would be created. Check, if the
  // user may write to the directory to create the file.
  if ( (mode & W_OK) == 0 )
    return false;   // Check for write access is not part of mode => bail out


  if (!access( TQFile::encodeName(pathname), F_OK)) // if it already exists
      return false;

  //strip the filename (everything until '/' from the end
  TQString dirName(pathname);
  int pos = dirName.findRev('/');
  if ( pos == -1 )
    return false;   // No path in argument. This is evil, we won't allow this
  else if ( pos == 0 ) // don't turn e.g. /root into an empty string
      pos = 1;

  dirName.truncate(pos); // strip everything starting from the last '/'

  accessOK = access( TQFile::encodeName(dirName), W_OK );
  // -?- Can I write to the accessed diretory
  if ( accessOK == 0 )
    return true;  // Yes
  else
    return false; // No
}

void KApplication::setTopWidget( TQWidget *topWidget )
{
  if( !topWidget )
      return;

    // set the specified caption
    if ( !topWidget->inherits("KMainWindow") ) { // KMainWindow does this already for us
        topWidget->setCaption( caption() );
    }

    // set the specified icons
    topWidget->setIcon( icon() ); //standard X11
#if defined Q_WS_X11
//#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
    KWin::setIcons(topWidget->winId(), icon(), miniIcon() ); // NET_WM hints for KWin

    // set the app startup notification window property
    KStartupInfo::setWindowStartupId( topWidget->winId(), startupId());
#endif
}

TQCString KApplication::startupId() const
{
    return d->startup_id;
}

void KApplication::setStartupId( const TQCString& startup_id )
{
    if( startup_id == d->startup_id )
        return;
#if defined Q_WS_X11
    KStartupInfo::handleAutoAppStartedSending(); // finish old startup notification if needed
#endif
    if( startup_id.isEmpty())
        d->startup_id = "0";
    else
        {
        d->startup_id = startup_id;
#if defined Q_WS_X11
        KStartupInfoId id;
        id.initId( startup_id );
        long timestamp = id.timestamp();
        if( timestamp != 0 )
            updateUserTimestamp( timestamp );
#endif
        }
}

// read the startup notification env variable, save it and unset it in order
// not to propagate it to processes started from this app
void KApplication::read_app_startup_id()
{
#if defined Q_WS_X11
    KStartupInfoId id = KStartupInfo::currentStartupIdEnv();
    KStartupInfo::resetStartupEnv();
    d->startup_id = id.id();
#endif
}

int KApplication::random()
{
   static bool init = false;
   if (!init)
   {
      unsigned int seed;
      init = true;
      int fd = open("/dev/urandom", O_RDONLY);
      if (fd < 0 || ::read(fd, &seed, sizeof(seed)) != sizeof(seed))
      {
            // No /dev/urandom... try something else.
            srand(getpid());
            seed = rand()+time(0);
      }
      if (fd >= 0) close(fd);
      srand(seed);
   }
   return rand();
}

TQString KApplication::randomString(int length)
{
   if (length <=0 ) return TQString::null;

   TQString str; str.setLength( length );
   int i = 0;
   while (length--)
   {
      int r=random() % 62;
      r+=48;
      if (r>57) r+=7;
      if (r>90) r+=6;
      str[i++] =  char(r);
      // so what if I work backwards?
   }
   return str;
}

bool KApplication::authorize(const TQString &genericAction)
{
   if (!d->actionRestrictions)
      return true;

   KConfig *config = KGlobal::config();
   KConfigGroupSaver saver( config, "KDE Action Restrictions" );
   return config->readBoolEntry(genericAction, true);
}

bool KApplication::authorizeKAction(const char *action)
{
   if (!d->actionRestrictions || !action)
      return true;

   static const TQString &action_prefix = KGlobal::staticQString( "action/" );

   return authorize(action_prefix + action);
}

bool KApplication::authorizeControlModule(const TQString &menuId)
{
   if (menuId.isEmpty() || kde_kiosk_exception)
      return true;
   KConfig *config = KGlobal::config();
   KConfigGroupSaver saver( config, "TDE Control Module Restrictions" );
   return config->readBoolEntry(menuId, true);
}

TQStringList KApplication::authorizeControlModules(const TQStringList &menuIds)
{
   KConfig *config = KGlobal::config();
   KConfigGroupSaver saver( config, "TDE Control Module Restrictions" );
   TQStringList result;
   for(TQStringList::ConstIterator it = menuIds.begin();
       it != menuIds.end(); ++it)
   {
      if (config->readBoolEntry(*it, true))
         result.append(*it);
   }
   return result;
}

void KApplication::initUrlActionRestrictions()
{
  d->urlActionRestrictions.setAutoDelete(true);
  d->urlActionRestrictions.clear();
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("open", TQString::null, TQString::null, TQString::null, TQString::null, TQString::null, TQString::null, true));
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("list", TQString::null, TQString::null, TQString::null, TQString::null, TQString::null, TQString::null, true));
// TEST:
//  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
//  ("list", TQString::null, TQString::null, TQString::null, TQString::null, TQString::null, TQString::null, false));
//  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
//  ("list", TQString::null, TQString::null, TQString::null, "file", TQString::null, TQDir::homeDirPath(), true));
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("link", TQString::null, TQString::null, TQString::null, ":internet", TQString::null, TQString::null, true));
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", TQString::null, TQString::null, TQString::null, ":internet", TQString::null, TQString::null, true));

  // We allow redirections to file: but not from internet protocols, redirecting to file:
  // is very popular among io-slaves and we don't want to break them
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", TQString::null, TQString::null, TQString::null, "file", TQString::null, TQString::null, true));
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", ":internet", TQString::null, TQString::null, "file", TQString::null, TQString::null, false));

  // local protocols may redirect everywhere
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", ":local", TQString::null, TQString::null, TQString::null, TQString::null, TQString::null, true));

  // Anyone may redirect to about:
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", TQString::null, TQString::null, TQString::null, "about", TQString::null, TQString::null, true));

  // Anyone may redirect to itself, cq. within it's own group
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", TQString::null, TQString::null, TQString::null, "=", TQString::null, TQString::null, true));

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver( config, "KDE URL Restrictions" );
  int count = config->readNumEntry("rule_count");
  TQString keyFormat = TQString("rule_%1");
  for(int i = 1; i <= count; i++)
  {
    TQString key = keyFormat.arg(i);
    TQStringList rule = config->readListEntry(key);
    if (rule.count() != 8)
      continue;
    TQString action = rule[0];
    TQString refProt = rule[1];
    TQString refHost = rule[2];
    TQString refPath = rule[3];
    TQString urlProt = rule[4];
    TQString urlHost = rule[5];
    TQString urlPath = rule[6];
    TQString strEnabled = rule[7].lower();

    bool bEnabled = (strEnabled == "true");

    if (refPath.startsWith("$HOME"))
       refPath.replace(0, 5, TQDir::homeDirPath());
    else if (refPath.startsWith("~"))
       refPath.replace(0, 1, TQDir::homeDirPath());
    if (urlPath.startsWith("$HOME"))
       urlPath.replace(0, 5, TQDir::homeDirPath());
    else if (urlPath.startsWith("~"))
       urlPath.replace(0, 1, TQDir::homeDirPath());

    if (refPath.startsWith("$TMP"))
       refPath.replace(0, 4, KGlobal::dirs()->saveLocation("tmp"));
    if (urlPath.startsWith("$TMP"))
       urlPath.replace(0, 4, KGlobal::dirs()->saveLocation("tmp"));

    d->urlActionRestrictions.append(new KApplicationPrivate::URLActionRule
    	( action, refProt, refHost, refPath, urlProt, urlHost, urlPath, bEnabled));
  }
}

void KApplication::allowURLAction(const TQString &action, const KURL &_baseURL, const KURL &_destURL)
{
  if (authorizeURLAction(action, _baseURL, _destURL))
     return;

  d->urlActionRestrictions.append(new KApplicationPrivate::URLActionRule
        ( action, _baseURL.protocol(), _baseURL.host(), _baseURL.path(-1),
                  _destURL.protocol(), _destURL.host(), _destURL.path(-1), true));
}

bool KApplication::authorizeURLAction(const TQString &action, const KURL &_baseURL, const KURL &_destURL)
{
  if (_destURL.isEmpty())
     return true;

  bool result = false;
  if (d->urlActionRestrictions.isEmpty())
     initUrlActionRestrictions();

  KURL baseURL(_baseURL);
  baseURL.setPath(TQDir::cleanDirPath(baseURL.path()));
  TQString baseClass = KProtocolInfo::protocolClass(baseURL.protocol());
  KURL destURL(_destURL);
  destURL.setPath(TQDir::cleanDirPath(destURL.path()));
  TQString destClass = KProtocolInfo::protocolClass(destURL.protocol());

  for(KApplicationPrivate::URLActionRule *rule = d->urlActionRestrictions.first();
      rule; rule = d->urlActionRestrictions.next())
  {
     if ((result != rule->permission) && // No need to check if it doesn't make a difference
         (action == rule->action) &&
         rule->baseMatch(baseURL, baseClass) &&
         rule->destMatch(destURL, destClass, baseURL, baseClass))
     {
        result = rule->permission;
     }
  }
  return result;
}


uint KApplication::keyboardModifiers()
{
#ifdef Q_WS_X11
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint keybstate;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &keybstate );
    return keybstate & 0x00ff;
#elif defined W_WS_MACX
    return GetCurrentEventKeyModifiers() & 0x00ff;
#else
    //TODO for win32
    return 0;
#endif
}

uint KApplication::mouseState()
{
    uint mousestate;
#ifdef Q_WS_X11
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &mousestate );
#elif defined(Q_WS_WIN)
    const bool mousebtn_swapped = GetSystemMetrics(SM_SWAPBUTTON);
    if (GetAsyncKeyState(VK_LBUTTON))
        mousestate |= (mousebtn_swapped ? Button3Mask : Button1Mask);
    if (GetAsyncKeyState(VK_MBUTTON))
        mousestate |= Button2Mask;
    if (GetAsyncKeyState(VK_RBUTTON))
        mousestate |= (mousebtn_swapped ? Button1Mask : Button3Mask);
#elif defined(Q_WS_MACX)
    mousestate = GetCurrentEventButtonState();
#else
    //TODO: other platforms
#endif
    return mousestate & 0xff00;
}

TQ_ButtonState KApplication::keyboardMouseState()
{
    int ret = 0;
#ifdef Q_WS_X11
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint state;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &state );
    // transform the same way like Qt's qt_x11_translateButtonState()
    if( state & Button1Mask )
        ret |= TQ_LeftButton;
    if( state & Button2Mask )
        ret |= TQ_MidButton;
    if( state & Button3Mask )
        ret |= TQ_RightButton;
    if( state & ShiftMask )
        ret |= TQ_ShiftButton;
    if( state & ControlMask )
        ret |= TQ_ControlButton;
    if( state & KKeyNative::modX( KKey::ALT ))
        ret |= TQ_AltButton;
    if( state & KKeyNative::modX( KKey::WIN ))
        ret |= TQ_MetaButton;
#elif defined(Q_WS_WIN)
    const bool mousebtn_swapped = GetSystemMetrics(SM_SWAPBUTTON);
    if (GetAsyncKeyState(VK_LBUTTON))
        ret |= (mousebtn_swapped ? RightButton : LeftButton);
    if (GetAsyncKeyState(VK_MBUTTON))
        ret |= TQ_MidButton;
    if (GetAsyncKeyState(VK_RBUTTON))
        ret |= (mousebtn_swapped ? TQ_LeftButton : TQ_RightButton);
    if (GetAsyncKeyState(VK_SHIFT))
        ret |= TQ_ShiftButton;
    if (GetAsyncKeyState(VK_CONTROL))
        ret |= TQ_ControlButton;
    if (GetAsyncKeyState(VK_MENU))
        ret |= TQ_AltButton;
    if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN))
        ret |= TQ_MetaButton;
#else
    //TODO: other platforms
#endif
    return static_cast< ButtonState >( ret );
}

void KApplication::installSigpipeHandler()
{
#ifdef Q_OS_UNIX
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigemptyset( &act.sa_mask );
    act.sa_flags = 0;
    sigaction( SIGPIPE, &act, 0 );
#endif
}

void KApplication::sigpipeHandler(int)
{
    int saved_errno = errno;
    // Using kdDebug from a signal handler is not a good idea.
#ifndef NDEBUG
    char msg[1000];
    sprintf(msg, "*** SIGPIPE *** (ignored, pid = %ld)\n", (long) getpid());
    write(2, msg, strlen(msg));
#endif

    // Do nothing.
    errno = saved_errno;
}

bool KApplication::guiEnabled()
{
    return kapp && kapp->d->guiEnabled;
}

void KApplication::virtual_hook( int id, void* data )
{ KInstance::virtual_hook( id, data ); }

void KSessionManaged::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kapplication.moc"
