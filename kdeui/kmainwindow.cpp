 /* This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-2000 Matthias Ettrich (ettrich@kde.org)
     (C) 1999 Chris Schlaeger (cs@kde.org)
     (C) 2002 Joseph Wenninger (jowenn@kde.org)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License version 2 as published by the Free Software Foundation.

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

#include "kmainwindow.h"
#include "kmainwindowiface.h"
#include "ktoolbarhandler.h"
#include "kwhatsthismanager_p.h"
#include <tqsessionmanager.h>
#include <tqobjectlist.h>
#include <tqstyle.h>
#include <tqlayout.h>
#include <tqwidgetlist.h>
#include <tqtimer.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kwin.h>
#include <kedittoolbar.h>
#include <kmainwindow.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#if defined Q_WS_X11
#include <netwm.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

class KMainWindowPrivate {
public:
    bool showHelpMenu:1;

    bool autoSaveSettings:1;
    bool settingsDirty:1;
    bool autoSaveWindowSize:1;
    bool care_about_tqgeometry:1;
    bool shuttingDown:1;
    TQString autoSaveGroup;
    KAccel * kaccel;
    KMainWindowInterface *m_interface;
    KDEPrivate::ToolBarHandler *toolBarHandler;
    TQTimer* settingsTimer;
    KToggleAction *showStatusBarAction;
    TQRect defaultWindowSize;
    TQPtrList<TQDockWindow> hiddenDockWindows;
};

TQPtrList<KMainWindow>* KMainWindow::memberList = 0L;
static bool no_query_exit = false;
static KMWSessionManaged* ksm = 0;
static KStaticDeleter<KMWSessionManaged> ksmd;

class KMWSessionManaged : public KSessionManaged
{
public:
    KMWSessionManaged()
    {
    }
    ~KMWSessionManaged()
    {
    }
    bool saveState( QSessionManager& )
    {
        KConfig* config = KApplication::kApplication()->sessionConfig();
        if ( KMainWindow::memberList->first() ){
            // According to Jochen Wilhelmy <digisnap@cs.tu-berlin.de>, this
            // hook is useful for better document orientation
            KMainWindow::memberList->first()->saveGlobalProperties(config);
        }

        TQPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
        int n = 0;
        for (it.toFirst(); it.current(); ++it){
            n++;
            it.current()->savePropertiesInternal(config, n);
        }
        config->setGroup(TQString::tqfromLatin1("Number"));
        config->writeEntry(TQString::tqfromLatin1("NumberOfWindows"), n );
        return true;
    }

    bool commitData( QSessionManager& sm )
    {
        // not really a fast method but the only compatible one
        if ( sm.allowsInteraction() ) {
            bool canceled = false;
            TQPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
            ::no_query_exit = true;
            for (it.toFirst(); it.current() && !canceled;){
                KMainWindow *window = *it;
                ++it; // Update now, the current window might get deleted
                if ( !window->testWState( Qt::WState_ForceHide ) ) {
                    TQCloseEvent e;
                    TQApplication::sendEvent( window, &e );
                    canceled = !e.isAccepted();
		    /* Don't even think_about deleting widgets with
		     Qt::WDestructiveClose flag set at this point. We
		     are faking a close event, but we are *not*_
		     closing the window. The purpose of the faked
		     close event is to prepare the application so it
		     can safely be quit without the user losing data
		     (possibly showing a message box "do you want to
		     save this or that?"). It is possible that the
		     session manager quits the application later
		     (emitting TQApplication::aboutToQuit() when this
		     happens), but it is also possible that the user
		     cancels the shutdown, so the application will
		     continue to run.
		     */
                }
            }
            ::no_query_exit = false;
            if (canceled)
               return false;

            KMainWindow* last = 0;
            for (it.toFirst(); it.current() && !canceled; ++it){
                KMainWindow *window = *it;
                if ( !window->testWState( Qt::WState_ForceHide ) ) {
                    last = window;
                }
            }
            if ( last )
                return last->queryExit();
            // else
            return true;
        }

        // the user wants it, the user gets it
        return true;
    }
};

static bool being_first = true;

KMainWindow::KMainWindow( TQWidget* parent, const char *name, WFlags f )
    : TQMainWindow( parent, name, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    initKMainWindow(name, 0);
}

KMainWindow::KMainWindow( int cflags, TQWidget* parent, const char *name, WFlags f )
    : TQMainWindow( parent, name, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    initKMainWindow(name, cflags);
}

void KMainWindow::initKMainWindow(const char *name, int cflags)
{
    KWhatsThisManager::init ();
    setDockMenuEnabled( false );
    mHelpMenu = 0;
    kapp->setTopWidget( this );
    actionCollection()->setWidget( this );
    connect(kapp, TQT_SIGNAL(shutDown()), this, TQT_SLOT(shuttingDown()));
    if( !memberList )
        memberList = new TQPtrList<KMainWindow>;

    if ( !ksm )
        ksm = ksmd.setObject(ksm, new KMWSessionManaged());
    // set a unique object name. Required by session management.
    TQCString objname;
    TQCString s;
    int unusedNumber;
    if ( !name )
        { // no name given
        objname = kapp->instanceName() + "-mainwindow#";
        s = objname + '1'; // start adding number immediately
        unusedNumber = 1;
        }
    else if( name[0] != '\0' && name[ strlen( name ) - 1 ] == '#' )
        { // trailing # - always add a number
        objname = name;
        s = objname + '1'; // start adding number immediately
        unusedNumber = 1;
        }
    else
        {
        objname = name;
        s = objname;
        unusedNumber = 0; // add numbers only when needed
        }
    for(;;) {
        TQWidgetList* list = kapp->tqtopLevelWidgets();
        TQWidgetListIt it( *list );
        bool found = false;
        for( TQWidget* w = it.current();
             w != NULL;
             ++it, w = it.current())
            if( w != this && w->name() == s )
                {
                found = true;
                break;
                }
        delete list;
        if( !found )
            break;
        s.setNum( ++unusedNumber );
        s = objname + s;
    }
    setName( s );

    memberList->append( this );

    d = new KMainWindowPrivate;
    d->showHelpMenu = true;
    d->settingsDirty = false;
    d->autoSaveSettings = false;
    d->autoSaveWindowSize = true; // for compatibility
    d->kaccel = actionCollection()->kaccel();
    d->toolBarHandler = 0;
    d->settingsTimer = 0;
    d->showStatusBarAction = NULL;
    d->shuttingDown = false;
    if ((d->care_about_tqgeometry = being_first)) {
        being_first = false;
        if ( kapp->geometryArgument().isNull() ) // if there is no tqgeometry, it doesn't mater
            d->care_about_tqgeometry = false;
        else
            parseGeometry(false);
    }

    setCaption( kapp->caption() );
    if ( cflags & NoDCOPObject)
        d->m_interface = 0;
    else
        d->m_interface = new KMainWindowInterface(this);

    if (!kapp->authorize("movable_toolbars"))
        setDockWindowsMovable(false);
}

KAction *KMainWindow::toolBarMenuAction()
{
    if ( !d->toolBarHandler )
	return 0;

    return d->toolBarHandler->toolBarMenuAction();
}


void KMainWindow::setupToolbarMenuActions()
{
    if ( d->toolBarHandler )
        d->toolBarHandler->setupActions();
}

void KMainWindow::parseGeometry(bool parsewidth)
{
    assert ( !kapp->geometryArgument().isNull() );
    assert ( d->care_about_tqgeometry );

#if defined Q_WS_X11
    int x, y;
    int w, h;
    int m = XParseGeometry( kapp->geometryArgument().latin1(), &x, &y, (unsigned int*)&w, (unsigned int*)&h);
    if (parsewidth) {
        TQSize minSize = tqminimumSize();
        TQSize maxSize = tqmaximumSize();
        if ( !(m & WidthValue) )
            w = width();
        if ( !(m & HeightValue) )
            h = height();
         w = QMIN(w,maxSize.width());
         h = QMIN(h,maxSize.height());
         w = QMAX(w,minSize.width());
         h = QMAX(h,minSize.height());
         resize(w, h);
    } else {
        if ( parsewidth && !(m & XValue) )
            x = tqgeometry().x();
        if ( parsewidth && !(m & YValue) )
            y = tqgeometry().y();
        if ( (m & XNegative) )
            x = KApplication::desktop()->width()  + x - w;
        if ( (m & YNegative) )
            y = KApplication::desktop()->height() + y - h;
        move(x, y);
    }
#endif
}

KMainWindow::~KMainWindow()
{
    delete d->settingsTimer;
    TQMenuBar* mb = internalMenuBar();
    delete mb;
    delete d->m_interface;
    delete d;
    memberList->remove( this );
}

KPopupMenu* KMainWindow::helpMenu( const TQString &aboutAppText, bool showWhatsThis )
{
    if( !mHelpMenu ) {
        if ( aboutAppText.isEmpty() )
            mHelpMenu = new KHelpMenu( this, instance()->aboutData(), showWhatsThis);
        else
            mHelpMenu = new KHelpMenu( this, aboutAppText, showWhatsThis );

        if ( !mHelpMenu )
            return 0;
        connect( mHelpMenu, TQT_SIGNAL( showAboutApplication() ),
                 this, TQT_SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

KPopupMenu* KMainWindow::customHelpMenu( bool showWhatsThis )
{
    if( !mHelpMenu ) {
        mHelpMenu = new KHelpMenu( this, TQString::null, showWhatsThis );
        connect( mHelpMenu, TQT_SIGNAL( showAboutApplication() ),
                 this, TQT_SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

bool KMainWindow::canBeRestored( int number )
{
    if ( !kapp->isRestored() )
        return false;
    KConfig *config = kapp->sessionConfig();
    if ( !config )
        return false;
    config->setGroup( TQString::tqfromLatin1("Number") );
    int n = config->readNumEntry( TQString::tqfromLatin1("NumberOfWindows") , 1 );
    return number >= 1 && number <= n;
}

const TQString KMainWindow::classNameOfToplevel( int number )
{
    if ( !kapp->isRestored() )
        return TQString::null;
    KConfig *config = kapp->sessionConfig();
    if ( !config )
        return TQString::null;
    TQString s;
    s.setNum( number );
    s.prepend( TQString::tqfromLatin1("WindowProperties") );
    config->setGroup( s );
    if ( !config->hasKey( TQString::tqfromLatin1("ClassName") ) )
        return TQString::null;
    else
        return config->readEntry( TQString::tqfromLatin1("ClassName") );
}

void KMainWindow::show()
{
    TQMainWindow::show();

    for ( TQPtrListIterator<TQDockWindow> it( d->hiddenDockWindows ); it.current(); ++it )
	it.current()->show();

    d->hiddenDockWindows.clear();
}

void KMainWindow::hide()
{
    if ( isVisible() ) {

        d->hiddenDockWindows.clear();

        TQObjectList *list = queryList( "TQDockWindow" );
        for( TQObjectListIt it( *list ); it.current(); ++it ) {
            TQDockWindow *dw = (TQDockWindow*)it.current();
            if ( dw->isTopLevel() && dw->isVisible() ) {
                d->hiddenDockWindows.append( dw );
                dw->hide();
            }
        }
        delete list;
    }

    TQWidget::hide();
}

bool KMainWindow::restore( int number, bool show )
{
    if ( !canBeRestored( number ) )
        return false;
    KConfig *config = kapp->sessionConfig();
    if ( readPropertiesInternal( config, number ) ){
        if ( show )
            KMainWindow::show();
        return false;
    }
    return false;
}

KXMLGUIFactory *KMainWindow::guiFactory()
{
    if ( !factory_ )
        factory_ = new KXMLGUIFactory( this, this, "guifactory" );
    return factory_;
}

int KMainWindow::configureToolbars()
{
    saveMainWindowSettings(KGlobal::config());
    KEditToolbar dlg(actionCollection(), xmlFile(), true, this);
    connect(&dlg, TQT_SIGNAL(newToolbarConfig()), TQT_SLOT(saveNewToolbarConfig()));
    return dlg.exec();
}

void KMainWindow::saveNewToolbarConfig()
{
    createGUI(xmlFile());
    applyMainWindowSettings( KGlobal::config() );
}

void KMainWindow::setupGUI( int options, const TQString & xmlfile ) {
    setupGUI(TQSize(), options, xmlfile);
}

void KMainWindow::setupGUI( TQSize defaultSize, int options, const TQString & xmlfile ) {
    if( options & Keys ){
        KStdAction::keyBindings(guiFactory(),
                    TQT_SLOT(configureShortcuts()), actionCollection());
    }

    if( (options & StatusBar) && internalStatusBar() ){
        createStandardStatusBarAction();
    }

    if( options & ToolBar ){
        setStandardToolBarMenuEnabled( true );
        KStdAction::configureToolbars(this,
                      TQT_SLOT(configureToolbars() ), actionCollection());
    }

    if( options & Create ){
        createGUI(xmlfile,false);
    }

    if( options & Save ){
        // setupGUI() is typically called in the constructor before show(),
        // so the default window size will be incorrect unless the application
        // hard coded the size which they should try not to do (i.e. use
        // size hints).
        if(initialGeometrySet())
        {
          // Do nothing...
        }
        else if(defaultSize.isValid())
        {
          resize(defaultSize);
        }
        else if(!isShown())
        {
          adjustSize();
        }
        setAutoSaveSettings();
    }

}

void KMainWindow::createGUI( const TQString &xmlfile, bool _conserveMemory )
{
    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient( this );

    // make sure to have an empty GUI
    TQMenuBar* mb = internalMenuBar();
    if ( mb )
        mb->clear();

    (void)toolBarIterator(); // make sure toolbarList is most-up-to-date
    toolbarList.setAutoDelete( true );
    toolbarList.clear();
    toolbarList.setAutoDelete( false );

    // don't build a help menu unless the user ask for it
    if (d->showHelpMenu) {
        // we always want a help menu
        if (!helpMenu2)
            helpMenu2 = new KHelpMenu(this, instance()->aboutData(), true,
                                      actionCollection());
    }

    // we always want to load in our global standards file
    setXMLFile( locate( "config", "ui/ui_standards.rc", instance() ) );

    // now, merge in our local xml file.  if this is null, then that
    // means that we will be only using the global file
    if ( !xmlfile.isNull() ) {
        setXMLFile( xmlfile, true );
    } else {
        TQString auto_file(instance()->instanceName() + "ui.rc");
        setXMLFile( auto_file, true );
    }

    // make sure we don't have any state saved already
    setXMLGUIBuildDocument( TQDomDocument() );

    // do the actual GUI building
    guiFactory()->addClient( this );

    // try and get back *some* of our memory
    if ( _conserveMemory )
    {
      // before freeing the memory allocated by the DOM document we also
      // free all memory allocated internally in the KXMLGUIFactory for
      // the menubar and the toolbars . This however implies that we
      // have to take care of deleting those widgets ourselves. For
      // destruction this is no problem, but when rebuilding we have
      // to take care of that (and we want to rebuild the GUI when
      // using stuff like the toolbar editor ).
      // In addition we have to take care of not removing containers
      // like popupmenus, defined in the XML document.
      // this code should probably go into a separate method in KMainWindow.
      // there's just one problem: I'm bad in tqfinding names ;-) , so
      // I skipped this ;-)

      TQDomDocument doc = domDocument();

      for( TQDomNode n = doc.documentElement().firstChild();
           !n.isNull(); n = n.nextSibling())
      {
          TQDomElement e = n.toElement();

          if ( e.tagName().lower() == "toolbar" )
              factory_->resetContainer( e.attribute( "name" ) );
          else if ( e.tagName().lower() == "menubar" )
              factory_->resetContainer( e.tagName(), true );
      }

      conserveMemory();
    }

    setUpdatesEnabled( true );
    updateGeometry();
}

void KMainWindow::setHelpMenuEnabled(bool showHelpMenu)
{
    d->showHelpMenu = showHelpMenu;
}

bool KMainWindow::isHelpMenuEnabled()
{
    return d->showHelpMenu;
}

void KMainWindow::setCaption( const TQString &caption )
{
    setPlainCaption( kapp->makeStdCaption(caption) );
}

void KMainWindow::setCaption( const TQString &caption, bool modified )
{
    setPlainCaption( kapp->makeStdCaption(caption, true, modified) );
}

void KMainWindow::setPlainCaption( const TQString &caption )
{
    TQMainWindow::setCaption( caption );
#if defined Q_WS_X11
    NETWinInfo info( qt_xdisplay(), winId(), qt_xrootwin(), 0 );
    info.setName( caption.utf8().data() );
#endif
}

void KMainWindow::appHelpActivated( void )
{
    if( !mHelpMenu ) {
        mHelpMenu = new KHelpMenu( this );
        if ( !mHelpMenu )
            return;
    }
    mHelpMenu->appHelpActivated();
}

void KMainWindow::slotStateChanged(const TQString &newstate)
{
  stateChanged(newstate, KXMLGUIClient::StateNoReverse);
}

/*
 * Get rid of this for KDE 4.0
 */
void KMainWindow::slotStateChanged(const TQString &newstate,
                                   KXMLGUIClient::ReverseStateChange reverse)
{
  stateChanged(newstate, reverse);
}

/*
 * Enable this for KDE 4.0
 */
// void KMainWindow::slotStateChanged(const TQString &newstate,
//                                    bool reverse)
// {
//   stateChanged(newstate,
//                reverse ? KXMLGUIClient::StateReverse : KXMLGUIClient::StateNoReverse);
// }

void KMainWindow::closeEvent ( TQCloseEvent *e )
{
    // Save settings if auto-save is enabled, and settings have changed
    if (d->settingsDirty && d->autoSaveSettings)
        saveAutoSaveSettings();

    if (queryClose()) {
        e->accept();

        int not_withdrawn = 0;
        TQPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
        for (it.toFirst(); it.current(); ++it){
            if ( !it.current()->isHidden() && it.current()->isTopLevel() && it.current() != this )
                not_withdrawn++;
        }

        if ( !no_query_exit && not_withdrawn <= 0 ) { // last window close accepted?
            if ( queryExit() && !kapp->sessionSaving() && !d->shuttingDown ) { // Yes, Quit app?
                // don't call queryExit() twice
                disconnect(kapp, TQT_SIGNAL(shutDown()), this, TQT_SLOT(shuttingDown()));
                d->shuttingDown = true;
                kapp->deref();             // ...and quit application.
            }  else {
                // cancel closing, it's stupid to end up with no windows at all....
                e->ignore();
            }
        }
    }
}

bool KMainWindow::queryExit()
{
    return true;
}

bool KMainWindow::queryClose()
{
    return true;
}

void KMainWindow::saveGlobalProperties( KConfig*  )
{
}

void KMainWindow::readGlobalProperties( KConfig*  )
{
}

#if defined(KDE_COMPAT)
void KMainWindow::updateRects()
{
}
#endif

void KMainWindow::showAboutApplication()
{
}

void KMainWindow::savePropertiesInternal( KConfig *config, int number )
{
    bool oldASWS = d->autoSaveWindowSize;
    d->autoSaveWindowSize = true; // make saveMainWindowSettings save the window size

    TQString s;
    s.setNum(number);
    s.prepend(TQString::tqfromLatin1("WindowProperties"));
    config->setGroup(s);

    // store objectName, className, Width and Height  for later restoring
    // (Only useful for session management)
    config->writeEntry(TQString::tqfromLatin1("ObjectName"), name());
    config->writeEntry(TQString::tqfromLatin1("ClassName"), className());

    saveMainWindowSettings(config); // Menubar, statusbar and Toolbar settings.

    s.setNum(number);
    config->setGroup(s);
    saveProperties(config);

    d->autoSaveWindowSize = oldASWS;
}

void KMainWindow::saveMainWindowSettings(KConfig *config, const TQString &configGroup)
{
    kdDebug(200) << "KMainWindow::saveMainWindowSettings " << configGroup << endl;
    TQString oldGroup;

    if (!configGroup.isEmpty())
    {
       oldGroup = config->group();
       config->setGroup(configGroup);
    }

    // Called by session management - or if we want to save the window size anyway
    if ( d->autoSaveWindowSize )
        saveWindowSize( config );

    TQStatusBar* sb = internalStatusBar();
    if (sb) {
       if(!config->hasDefault("StatusBar") && !sb->isHidden() )
           config->revertToDefault("StatusBar");
       else
           config->writeEntry("StatusBar", sb->isHidden() ? "Disabled" : "Enabled");
    }

    TQMenuBar* mb = internalMenuBar();
    if (mb) {
       TQString MenuBar = TQString::tqfromLatin1("MenuBar");
       if(!config->hasDefault("MenuBar") && !mb->isHidden() )
           config->revertToDefault("MenuBar");
       else
           config->writeEntry("MenuBar", mb->isHidden() ? "Disabled" : "Enabled");
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    KToolBar *toolbar = 0;
    TQPtrListIterator<KToolBar> it( toolBarIterator() );
    while ( ( toolbar = it.current() ) ) {
        ++it;
        TQString group;
        if (!configGroup.isEmpty())
        {
           // Give a number to the toolbar, but prefer a name if there is one,
           // because there's no real guarantee on the ordering of toolbars
           group = (!::qstrcmp(toolbar->name(), "unnamed") ? TQString::number(n) : TQString(" ")+toolbar->name());
           group.prepend(" Toolbar");
           group.prepend(configGroup);
        }
        toolbar->saveSettings(config, group);
        n++;
    }
    if (!configGroup.isEmpty())
       config->setGroup(oldGroup);
}

void KMainWindow::setStandardToolBarMenuEnabled( bool enable )
{
    if ( enable ) {
        if ( d->toolBarHandler )
            return;

    d->toolBarHandler = new KDEPrivate::ToolBarHandler( this );

    if ( factory() )
        factory()->addClient( d->toolBarHandler );
    } else {
        if ( !d->toolBarHandler )
            return;

        if ( factory() )
            factory()->removeClient( d->toolBarHandler );

        delete d->toolBarHandler;
        d->toolBarHandler = 0;
    }
}

bool KMainWindow::isStandardToolBarMenuEnabled() const
{
    return ( d->toolBarHandler );
}

void KMainWindow::createStandardStatusBarAction(){
  if(!d->showStatusBarAction){
    d->showStatusBarAction = KStdAction::showtqStatusbar(this, TQT_SLOT(setSettingsDirty()), actionCollection());
    KStatusBar *sb = statusBar(); // Creates statusbar if it doesn't exist already.
    connect(d->showStatusBarAction, TQT_SIGNAL(toggled(bool)), sb, TQT_SLOT(setShown(bool)));
    d->showStatusBarAction->setChecked(sb->isHidden());
  }
}

bool KMainWindow::readPropertiesInternal( KConfig *config, int number )
{
    if ( number == 1 )
        readGlobalProperties( config );

    // in order they are in toolbar list
    TQString s;
    s.setNum(number);
    s.prepend(TQString::tqfromLatin1("WindowProperties"));

    config->setGroup(s);

    // restore the object name (window role)
    if ( config->hasKey(TQString::tqfromLatin1("ObjectName" )) )
        setName( config->readEntry(TQString::tqfromLatin1("ObjectName")).latin1()); // latin1 is right here

    applyMainWindowSettings(config); // Menubar, statusbar and toolbar settings.

    s.setNum(number);
    config->setGroup(s);
    readProperties(config);
    return true;
}

void KMainWindow::applyMainWindowSettings(KConfig *config, const TQString &configGroup)
{
    return applyMainWindowSettings(config,configGroup,false);
}

void KMainWindow::applyMainWindowSettings(KConfig *config, const TQString &configGroup,bool force)
{
    kdDebug(200) << "KMainWindow::applyMainWindowSettings" << endl;

    KConfigGroupSaver saver( config, configGroup.isEmpty() ? config->group() : configGroup );

    restoreWindowSize(config);

    TQStatusBar* sb = internalStatusBar();
    if (sb) {
        TQString entry = config->readEntry("StatusBar", "Enabled");
        if ( entry == "Disabled" )
           sb->hide();
        else
           sb->show();
        if(d->showStatusBarAction)
            d->showStatusBarAction->setChecked(!sb->isHidden());
    }

    TQMenuBar* mb = internalMenuBar();
    if (mb) {
        TQString entry = config->readEntry ("MenuBar", "Enabled");
        if ( entry == "Disabled" )
           mb->hide();
        else
           mb->show();
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    KToolBar *toolbar;
    TQPtrListIterator<KToolBar> it( toolBarIterator() ); // must use own iterator

    for ( ; it.current(); ++it) {
        toolbar= it.current();
        TQString group;
        if (!configGroup.isEmpty())
        {
           // Give a number to the toolbar, but prefer a name if there is one,
           // because there's no real guarantee on the ordering of toolbars
           group = (!::qstrcmp(toolbar->name(), "unnamed") ? TQString::number(n) : TQString(" ")+toolbar->name());
           group.prepend(" Toolbar");
           group.prepend(configGroup);
        }
        toolbar->applySettings(config, group, force);
        n++;
    }

    finalizeGUI( true );
}

void KMainWindow::finalizeGUI( bool force )
{
    //kdDebug(200) << "KMainWindow::finalizeGUI force=" << force << endl;
    // The whole reason for this is that moveToolBar relies on the indexes
    // of the other toolbars, so in theory it should be called only once per
    // toolbar, but in increasing order of indexes.
    // Since we can't do that immediately, we move them, and _then_
    // we call positionYourself again for each of them, but this time
    // the toolbariterator should give them in the proper order.
    // Both the XMLGUI and applySettings call this, hence "force" for the latter.
    TQPtrListIterator<KToolBar> it( toolBarIterator() );
    for ( ; it.current() ; ++it ) {
        it.current()->positionYourself( force );
    }

    d->settingsDirty = false;
}

void KMainWindow::saveWindowSize( KConfig * config ) const
{
  int scnum = TQApplication::desktop()->screenNumber(tqparentWidget());
  TQRect desk = TQApplication::desktop()->screenGeometry(scnum);
  int w, h;
#if defined Q_WS_X11
  // save maximalization as desktop size + 1 in that direction
  KWin::WindowInfo info = KWin::windowInfo( winId(), NET::WMState );
  w = info.state() & NET::MaxHoriz ? desk.width() + 1 : width();
  h = info.state() & NET::MaxVert ? desk.height() + 1 : height();
#else
  if (isMaximized()) {
    w = desk.width() + 1;
    h = desk.height() + 1;
  }
  //TODO: add "Maximized" property instead "+1" hack
#endif
  TQRect size( desk.width(), w, desk.height(), h );
  bool defaultSize = (size == d->defaultWindowSize);
  TQString widthString = TQString::tqfromLatin1("Width %1").arg(desk.width());
  TQString heightString = TQString::tqfromLatin1("Height %1").arg(desk.height());
  if (!config->hasDefault(widthString) && defaultSize)
     config->revertToDefault(widthString);
  else
     config->writeEntry(widthString, w );

  if (!config->hasDefault(heightString) && defaultSize)
     config->revertToDefault(heightString);
  else
     config->writeEntry(heightString, h );
}

void KMainWindow::restoreWindowSize( KConfig * config )
{
    if (d->care_about_tqgeometry) {
        parseGeometry(true);
    } else {
        // restore the size
        int scnum = TQApplication::desktop()->screenNumber(tqparentWidget());
        TQRect desk = TQApplication::desktop()->screenGeometry(scnum);
        if ( d->defaultWindowSize.isNull() ) // only once
          d->defaultWindowSize = TQRect(desk.width(), width(), desk.height(), height()); // store default values
        TQSize size( config->readNumEntry( TQString::tqfromLatin1("Width %1").arg(desk.width()), 0 ),
                    config->readNumEntry( TQString::tqfromLatin1("Height %1").arg(desk.height()), 0 ) );
        if (size.isEmpty()) {
            // try the KDE 2.0 way
            size = TQSize( config->readNumEntry( TQString::tqfromLatin1("Width"), 0 ),
                          config->readNumEntry( TQString::tqfromLatin1("Height"), 0 ) );
            if (!size.isEmpty()) {
                // make sure the other resolutions don't get old settings
                config->writeEntry( TQString::tqfromLatin1("Width"), 0 );
                config->writeEntry( TQString::tqfromLatin1("Height"), 0 );
            }
        }
        if ( !size.isEmpty() ) {
#ifdef Q_WS_X11
            int state = ( size.width() > desk.width() ? NET::MaxHoriz : 0 )
                        | ( size.height() > desk.height() ? NET::MaxVert : 0 );
            if(( state & NET::Max ) == NET::Max )
                ; // no resize
            else if(( state & NET::MaxHoriz ) == NET::MaxHoriz )
                resize( width(), size.height());
            else if(( state & NET::MaxVert ) == NET::MaxVert )
                resize( size.width(), height());
            else
                resize( size );
            // TQWidget::showMaximized() is both insufficient and broken
            KWin::setState( winId(), state );
#else
            if (size.width() > desk.width() || size.height() > desk.height())
              setWindowState( WindowMaximized );
            else
              resize( size );
#endif
        }
    }
}

bool KMainWindow::initialGeometrySet() const
{
    return d->care_about_tqgeometry;
}

void KMainWindow::ignoreInitialGeometry()
{
    d->care_about_tqgeometry = false;
}

void KMainWindow::setSettingsDirty()
{
    //kdDebug(200) << "KMainWindow::setSettingsDirty" << endl;
    d->settingsDirty = true;
    if ( d->autoSaveSettings )
    {
        // Use a timer to save "immediately" user-wise, but not too immediately
        // (to compress calls and save only once, in case of multiple changes)
        if ( !d->settingsTimer )
        {
           d->settingsTimer = new TQTimer( this );
           connect( d->settingsTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( saveAutoSaveSettings() ) );
        }
        d->settingsTimer->start( 500, true );
    }
}

bool KMainWindow::settingsDirty() const
{
    return d->settingsDirty;
}

TQString KMainWindow::settingsGroup() const
{
    return d->autoSaveGroup;
}

void KMainWindow::setAutoSaveSettings( const TQString & groupName, bool saveWindowSize )
{
    d->autoSaveSettings = true;
    d->autoSaveGroup = groupName;
    d->autoSaveWindowSize = saveWindowSize;
    // Get notified when the user moves a toolbar around
    disconnect( this, TQT_SIGNAL( dockWindowPositionChanged( TQDockWindow * ) ),
                this, TQT_SLOT( setSettingsDirty() ) );
    connect( this, TQT_SIGNAL( dockWindowPositionChanged( TQDockWindow * ) ),
             this, TQT_SLOT( setSettingsDirty() ) );

    // Now read the previously saved settings
    applyMainWindowSettings( KGlobal::config(), groupName );
}

void KMainWindow::resetAutoSaveSettings()
{
    d->autoSaveSettings = false;
    if ( d->settingsTimer )
        d->settingsTimer->stop();
}

bool KMainWindow::autoSaveSettings() const
{
    return d->autoSaveSettings;
}

TQString KMainWindow::autoSaveGroup() const
{
    return d->autoSaveGroup;
}

void KMainWindow::saveAutoSaveSettings()
{
    Q_ASSERT( d->autoSaveSettings );
    //kdDebug(200) << "KMainWindow::saveAutoSaveSettings -> saving settings" << endl;
    saveMainWindowSettings( KGlobal::config(), d->autoSaveGroup );
    KGlobal::config()->sync();
    d->settingsDirty = false;
    if ( d->settingsTimer )
        d->settingsTimer->stop();
}

void KMainWindow::resizeEvent( TQResizeEvent * )
{
    if ( d->autoSaveWindowSize )
        setSettingsDirty();
}

bool KMainWindow::hasMenuBar()
{
    return (internalMenuBar());
}

KMenuBar *KMainWindow::menuBar()
{
    KMenuBar * mb = internalMenuBar();
    if ( !mb ) {
        mb = new KMenuBar( this );
        // trigger a re-tqlayout and trigger a call to the private
        // setMenuBar method.
        TQMainWindow::menuBar();
    }
    return mb;
}

KStatusBar *KMainWindow::statusBar()
{
    KStatusBar * sb = internalStatusBar();
    if ( !sb ) {
        sb = new KStatusBar( this );
        // trigger a re-tqlayout and trigger a call to the private
        // setStatusBar method.
        TQMainWindow::statusBar();
    }
    return sb;
}

void KMainWindow::shuttingDown()
{
    // Needed for Qt <= 3.0.3 at least to prevent reentrancy
    // when queryExit() shows a dialog. Check before removing!
    static bool reentrancy_protection = false;
    if (!reentrancy_protection)
    {
       reentrancy_protection = true;
       // call the virtual queryExit
       queryExit();
       reentrancy_protection = false;
    }

}

KMenuBar *KMainWindow::internalMenuBar()
{
    TQObjectList *l = queryList( "KMenuBar", 0, false, false );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    KMenuBar *m = (KMenuBar*)l->first();
    delete l;
    return m;
}

KStatusBar *KMainWindow::internalStatusBar()
{
    TQObjectList *l = queryList( "KStatusBar", 0, false, false );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    KStatusBar *s = (KStatusBar*)l->first();
    delete l;
    return s;
}

void KMainWindow::childEvent( TQChildEvent* e)
{
    TQMainWindow::childEvent( e );
}

KToolBar *KMainWindow::toolBar( const char * name )
{
    if (!name)
       name = "mainToolBar";
    KToolBar *tb = (KToolBar*)child( name, "KToolBar" );
    if ( tb )
        return tb;
    bool honor_mode = (!strcmp(name, "mainToolBar"));

    if ( builderClient() )
        return new KToolBar(this, name, honor_mode); // XMLGUI constructor
    else
        return new KToolBar(this, DockTop, false, name, honor_mode ); // non-XMLGUI
}

TQPtrListIterator<KToolBar> KMainWindow::toolBarIterator()
{
    toolbarList.clear();
    TQPtrList<TQToolBar> lst;
    for ( int i = (int)TQMainWindow::DockUnmanaged; i <= (int)DockMinimized; ++i ) {
        lst = toolBars( (ToolBarDock)i );
        for ( TQToolBar *tb = lst.first(); tb; tb = lst.next() ) {
            if ( !tb->inherits( "KToolBar" ) )
                continue;
            toolbarList.append( (KToolBar*)tb );
        }
    }
    return TQPtrListIterator<KToolBar>( toolbarList );
}

KAccel * KMainWindow::accel()
{
    if ( !d->kaccel )
        d->kaccel = new KAccel( this, "kmw-kaccel" );
    return d->kaccel;
}

void KMainWindow::paintEvent( TQPaintEvent * pe )
{
    TQMainWindow::paintEvent(pe); //Upcall to handle SH_MainWindow_SpaceBelowMenuBar rendering
}

TQSize KMainWindow::sizeForCentralWidgetSize(TQSize size)
{
    KToolBar *tb = (KToolBar*)child( "mainToolBar", "KToolBar" );
    if (tb && !tb->isHidden()) {
        switch( tb->barPos() )
        {
          case KToolBar::Top:
          case KToolBar::Bottom:
            size += TQSize(0, tb->tqsizeHint().height());
            break;

          case KToolBar::Left:
          case KToolBar::Right:
            size += TQSize(toolBar()->tqsizeHint().width(), 0);
            break;

          case KToolBar::Flat:
            size += TQSize(0, 3+kapp->style().tqpixelMetric( TQStyle::PM_DockWindowHandleExtent ));
            break;

          default:
            break;
        }
    }
    KMenuBar *mb = internalMenuBar();
    if (mb && !mb->isHidden()) {
        size += TQSize(0,mb->heightForWidth(size.width()));
        if (style().tqstyleHint(TQStyle::SH_MainWindow_SpaceBelowMenuBar, this))
           size += TQSize( 0, dockWindowsMovable() ? 1 : 2);
    }
    TQStatusBar *sb = internalStatusBar();
    if( sb && !sb->isHidden() )
       size += TQSize(0, sb->tqsizeHint().height());

    return size;
}

#if KDE_IS_VERSION( 3, 9, 0 )
#ifdef __GNUC__
#warning Remove, should be in Qt
#endif
#endif
void KMainWindow::setIcon( const TQPixmap& p )
{
    TQMainWindow::setIcon( p );
#ifdef Q_WS_X11 
    // Qt3 doesn't support _NET_WM_ICON, but KApplication::setTopWidget(), which
    // is used by KMainWindow, sets it
    KWin::setIcons( winId(), p, TQPixmap());
#endif
}

TQPtrList<KMainWindow>* KMainWindow::getMemberList() { return memberList; }

// why do we support old gcc versions? using KXMLGUIBuilder::finalizeGUI;
// DF: because they compile KDE much faster :)
void KMainWindow::finalizeGUI( KXMLGUIClient *client )
{ KXMLGUIBuilder::finalizeGUI( client ); }

void KMainWindow::virtual_hook( int id, void* data )
{ KXMLGUIBuilder::virtual_hook( id, data );
  KXMLGUIClient::virtual_hook( id, data ); }



#include "kmainwindow.moc"

