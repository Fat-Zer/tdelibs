//----------------------------------------------------------------------------
//    filename             : tdemdimainfrm.cpp
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                           01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2003 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------


/*
 * ATTENTION: please do you part to try to make this file legible.  It's
 * extremely hard to read already.  Especially follow the indenting rules.
 */
#include "config.h"

#include <assert.h>

#include <tqcursor.h>
#include <tqclipboard.h>
#include <tqobjectlist.h>
#include <tqpopupmenu.h>
#include <tqmenubar.h>

#include <tdemenubar.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeversion.h>
#include <tqtabwidget.h>
#include <tdelocale.h>
#include <tdestdaccel.h>

#include <kiconloader.h>
#include <tdemdidockcontainer.h>


#include <tqtoolbutton.h>
#include <tqdockarea.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqtextstream.h>
#include <tqstring.h>
#include <tqmap.h>
#include <tqvaluelist.h>

#include "tdemdimainfrm.h"
#include "tdemditaskbar.h"
#include "tdemdichildfrm.h"
#include "tdemdichildarea.h"
#include "tdemdichildview.h"
#include "tdemdidockcontainer.h"
#include "tdemditoolviewaccessor_p.h"
#include "tdemdifocuslist.h"
#include "tdemdidocumentviewtabwidget.h"
#include "tdemdiguiclient.h"

#include "win_undockbutton.xpm"
#include "win_minbutton.xpm"
#include "win_restorebutton.xpm"
#include "win_closebutton.xpm"
#include "kde_undockbutton.xpm"
#include "kde_minbutton.xpm"
#include "kde_restorebutton.xpm"
#include "kde_closebutton.xpm"
#include "kde2_undockbutton.xpm"
#include "kde2_minbutton.xpm"
#include "kde2_restorebutton.xpm"
#include "kde2_closebutton.xpm"
#include "kde2laptop_undockbutton.xpm"
#include "kde2laptop_minbutton.xpm"
#include "kde2laptop_restorebutton.xpm"
#include "kde2laptop_closebutton.xpm"
#include "kde2laptop_closebutton_menu.xpm"

#ifdef Q_WS_X11
#ifndef NO_KDE
#include <X11/X.h> // schroder
#include <X11/Xlib.h> // schroder
#endif

#ifdef KeyRelease 
/* I hate the defines in the X11 header files. Get rid of one of them */
#undef KeyRelease
#endif

#ifdef KeyPress 
/* I hate the defines in the X11 header files. Get rid of one of them */
#undef KeyPress
#endif
#endif // Q_WS_X11 && ! K_WS_QTONLY

using namespace KParts;

KMdi::FrameDecor KMdiMainFrm::m_frameDecoration = KMdi::KDELook;

class KMdiMainFrmPrivate
{
public:
	KMdiMainFrmPrivate() : focusList( 0 )
	{
		for ( int i = 0;i < 4;i++ )
		{
			activeDockPriority[ i ] = 0;
			m_styleIDEAlMode = 0;
			m_toolviewStyle = 0;
		}
	}
	~KMdiMainFrmPrivate()
	{}
	KMdiDockContainer* activeDockPriority[ 4 ];
	KMdiFocusList *focusList;
	int m_styleIDEAlMode;
	int m_toolviewStyle;
	TDEAction *closeWindowAction;
};

//============ constructor ============//
KMdiMainFrm::KMdiMainFrm( TQWidget* parentWidget, const char* name, KMdi::MdiMode mdiMode, WFlags flags )
		: KParts::DockMainWindow( parentWidget, name, flags )
		, m_mdiMode( KMdi::UndefinedMode )
		, m_pMdi( 0L )
		, m_pTaskBar( 0L )
		, m_pDocumentViews( 0L )
		, m_pCurrentWindow( 0L )
		, m_pWindowPopup( 0L )
		, m_pTaskBarPopup( 0L )
		, m_pWindowMenu( 0L )
		, m_pDockMenu( 0L )
		, m_pMdiModeMenu( 0L )
		, m_pPlacingMenu( 0L )
		, m_pMainMenuBar( 0L )
		, m_pUndockButtonPixmap( 0L )
		, m_pMinButtonPixmap( 0L )
		, m_pRestoreButtonPixmap( 0L )
		, m_pCloseButtonPixmap( 0L )
		, m_pUndock( 0L )
		, m_pMinimize( 0L )
		, m_pRestore( 0L )
		, m_pClose( 0L )
		, m_bMaximizedChildFrmMode( false )
		, m_oldMainFrmHeight( 0 )
		, m_oldMainFrmMinHeight( 0 )
		, m_oldMainFrmMaxHeight( 0 )
		, m_bSDIApplication( false )
		, m_pDockbaseAreaOfDocumentViews( 0L )
		, m_pTempDockSession( 0L )
		, m_bClearingOfWindowMenuBlocked( false )
		, m_pDragEndTimer( 0L )
		, m_bSwitching( false )
		, m_leftContainer( 0 )
		, m_rightContainer( 0 )
		, m_topContainer( 0 )
		, m_bottomContainer( 0 )
		, d( new KMdiMainFrmPrivate() )
		, m_mdiGUIClient( 0 )
		, m_managedDockPositionMode( false )
		, m_documentTabWidget( 0 )
{
	kdDebug(760) << k_funcinfo << endl;
	// Create the local lists of windows
	m_pDocumentViews = new TQPtrList<KMdiChildView>;
	m_pDocumentViews->setAutoDelete( false );
	m_pToolViews = new TQMap<TQWidget*, KMdiToolViewAccessor*>;

	// This seems to be needed (re-check it after Qt2.0 comed out)
	setFocusPolicy( TQ_ClickFocus );

	// create the central widget
	createMdiManager();

	// cover KMdi's childarea by a dockwidget
	m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", TQPixmap(), 0L, "mdi_area_cover" );
	m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );
	m_pDockbaseAreaOfDocumentViews->setEnableDocking( KDockWidget::DockNone );
	m_pDockbaseAreaOfDocumentViews->setDockSite( KDockWidget::DockCorner );
	m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
	// set this dock to main view
	setView( m_pDockbaseAreaOfDocumentViews );
	setMainDockWidget( m_pDockbaseAreaOfDocumentViews );

	// Apply options for the MDI manager
	applyOptions();

	m_pTaskBarPopup = new TQPopupMenu( this, "taskbar_popup_menu" );
	m_pWindowPopup = new TQPopupMenu( this, "window_popup_menu" );

	m_pWindowMenu = new TQPopupMenu( this, "window_menu" );
	m_pWindowMenu->setCheckable( true );
	TQObject::connect( m_pWindowMenu, TQT_SIGNAL( aboutToShow() ), this, TQT_SLOT( fillWindowMenu() ) );

	m_pDockMenu = new TQPopupMenu( this, "dock_menu" );
	m_pDockMenu->setCheckable( true );

	m_pMdiModeMenu = new TQPopupMenu( this, "mdimode_menu" );
	m_pMdiModeMenu->setCheckable( true );

	m_pPlacingMenu = new TQPopupMenu( this, "placing_menu" );

	d->closeWindowAction = new TDEAction(i18n("&Close"), TDEStdAccel::close(),
	                                   TQT_TQOBJECT(this), TQT_SLOT(closeActiveView()), actionCollection(), "window_close");

	// the MDI view taskbar
	createTaskBar();

	// this is only a hack, but prevents us from crash because the buttons are otherwise
	// not created before we switch the modes where we need them !!!
	setMenuForSDIModeSysButtons( menuBar() );

	switch ( mdiMode )
	{
	case KMdi::IDEAlMode:
		kdDebug(760) << k_funcinfo << "Switching to IDEAl mode" << endl;
		switchToIDEAlMode();
		break;
	case KMdi::TabPageMode:
		kdDebug(760) << k_funcinfo << "Switching to tab page mode" << endl;
		switchToTabPageMode();
		break;
	case KMdi::ToplevelMode:
		kdDebug(760) << k_funcinfo << "Switching to top level mode" << endl;
		switchToToplevelMode();
		break;
	default:
		m_mdiMode = KMdi::ChildframeMode;
		kdDebug(760) << k_funcinfo << "Switching to child frame mode" << endl;
		break;
	}

	// drag end timer
	m_pDragEndTimer = new TQTimer();
	connect( m_pDragEndTimer, TQT_SIGNAL( timeout() ), this, TQT_SLOT( dragEndTimeOut() ) );
	connect( guiFactory(), TQT_SIGNAL( clientAdded( KXMLGUIClient* ) ),
	         this, TQT_SLOT( verifyToplevelHeight() ) );
	connect( guiFactory(), TQT_SIGNAL( clientRemoved( KXMLGUIClient* ) ),
	         this, TQT_SLOT( verifyToplevelHeight() ) );
}

void KMdiMainFrm::verifyToplevelHeight()
{
	if ( m_mdiMode != KMdi::ToplevelMode )
		return;
	
	//kdDebug(760) << k_funcinfo << endl;
	int topDockHeight = topDock() ? topDock()->height() : 0;
	int menuBarHeight = hasMenuBar() ? menuBar()->height() : 0;
	setFixedHeight( topDockHeight + menuBarHeight );
	resize( width(), height() );
}

void KMdiMainFrm::setStandardMDIMenuEnabled( bool showModeMenu )
{
	m_mdiGUIClient = new KMDIPrivate::KMDIGUIClient( this, showModeMenu );
	connect( m_mdiGUIClient, TQT_SIGNAL( toggleTop() ), this, TQT_SIGNAL( toggleTop() ) );
	connect( m_mdiGUIClient, TQT_SIGNAL( toggleLeft() ), this, TQT_SIGNAL( toggleLeft() ) );
	connect( m_mdiGUIClient, TQT_SIGNAL( toggleRight() ), this, TQT_SIGNAL( toggleRight() ) );
	connect( m_mdiGUIClient, TQT_SIGNAL( toggleBottom() ), this, TQT_SIGNAL( toggleBottom() ) );

	if ( m_mdiMode == KMdi::IDEAlMode )
	{
		if ( m_topContainer )
			connect( this, TQT_SIGNAL( toggleTop() ), m_topContainer->getWidget(), TQT_SLOT( toggle() ) );
		if ( m_leftContainer )
			connect( this, TQT_SIGNAL( toggleLeft() ), m_leftContainer->getWidget(), TQT_SLOT( toggle() ) );
		if ( m_rightContainer )
			connect( this, TQT_SIGNAL( toggleRight() ), m_rightContainer->getWidget(), TQT_SLOT( toggle() ) );
		if ( m_bottomContainer )
			connect( this, TQT_SIGNAL( toggleBottom() ), m_bottomContainer->getWidget(), TQT_SLOT( toggle() ) );
	}

	emit mdiModeHasBeenChangedTo( m_mdiMode );
}

//============ ~KMdiMainFrm ============//
KMdiMainFrm::~KMdiMainFrm()
{
	//save the children first to a list, as removing invalidates our iterator
	TQValueList<KMdiChildView*> children;
	for ( KMdiChildView * w = m_pDocumentViews->first();w;w = m_pDocumentViews->next() )
		children.append( w );

	// safely close the windows so properties are saved...
	TQValueListIterator<KMdiChildView*> childIt;
	for ( childIt = children.begin(); childIt != children.end(); ++childIt )
	{
		closeWindow( *childIt, false ); // without re-layout taskbar!
	}

	emit lastChildViewClosed();
	delete m_pDocumentViews;
	delete m_pToolViews;
	m_pToolViews = 0;
	delete m_pDragEndTimer;

	delete m_pUndockButtonPixmap;
	delete m_pMinButtonPixmap;
	delete m_pRestoreButtonPixmap;
	delete m_pCloseButtonPixmap;

	//deletes added for Release-Version-Pop-Up-WinMenu-And-Go-Out-Problem
	delete m_pDockMenu;
	delete m_pMdiModeMenu;
	delete m_pPlacingMenu;
	delete m_pTaskBarPopup;
	delete m_pWindowPopup;
	delete m_pWindowMenu;
	delete m_mdiGUIClient;
	delete m_pTempDockSession;
	m_mdiGUIClient = 0;
	delete d;
	d = 0;
}

//============ applyOptions ============//
//FIXME something wrong with this function. dunno what though
void KMdiMainFrm::applyOptions()
{
	TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
	{
		TQWidget* childFrame = 0L;
		if ( ( *it )->mdiParent() )
		{
			kdDebug(760) << k_funcinfo << "using child view's mdi parent for resize hack" << endl;
			childFrame = ( *it )->mdiParent();
		}
		else
		{
			kdDebug(760) << k_funcinfo << "using child view for resize hack" << endl;
			childFrame = ( *it );
		}
		
		int w = childFrame->width();
		int h = childFrame->height();
		childFrame->resize( w + 1, h + 1 );
		childFrame->resize( w - 1, h - 1 );
	}
}

//============ createMdiManager ============//
void KMdiMainFrm::createMdiManager()
{
	kdDebug(760) << k_funcinfo << "creating MDI manager" << endl;
	m_pMdi = new KMdiChildArea( this );
	setCentralWidget( m_pMdi );
	TQObject::connect( m_pMdi, TQT_SIGNAL( nowMaximized( bool ) ),
	                  this, TQT_SLOT( setEnableMaximizedChildFrmMode( bool ) ) );
	TQObject::connect( m_pMdi, TQT_SIGNAL( noMaximizedChildFrmLeft( KMdiChildFrm* ) ),
	                  this, TQT_SLOT( switchOffMaximizeModeForMenu( KMdiChildFrm* ) ) );
	TQObject::connect( m_pMdi, TQT_SIGNAL( sysButtonConnectionsMustChange( KMdiChildFrm*, KMdiChildFrm* ) ),
	                  this, TQT_SLOT( updateSysButtonConnections( KMdiChildFrm*, KMdiChildFrm* ) ) );
	TQObject::connect( m_pMdi, TQT_SIGNAL( popupWindowMenu( TQPoint ) ),
	                  this, TQT_SLOT( popupWindowMenu( TQPoint ) ) );
	TQObject::connect( m_pMdi, TQT_SIGNAL( lastChildFrmClosed() ),
	                  this, TQT_SIGNAL( lastChildFrmClosed() ) );
}

//============ createTaskBar ==============//
void KMdiMainFrm::createTaskBar()
{
	m_pTaskBar = new KMdiTaskBar( this, TQMainWindow::DockBottom );
	m_pTaskBar->installEventFilter( this );
}

void KMdiMainFrm::slot_toggleTaskBar()
{
	if ( !m_pTaskBar )
		return;
	m_pTaskBar->switchOn( !m_pTaskBar->isSwitchedOn() );
}

void KMdiMainFrm::resizeEvent( TQResizeEvent *e )
{
	if ( ( m_mdiMode == KMdi::ToplevelMode ) && !parentWidget() )
	{
		if ( e->oldSize().height() != e->size().height() )
			return ;
	}
	KParts::DockMainWindow::resizeEvent( e );
	if ( !m_mdiGUIClient )
		return ;
	setSysButtonsAtMenuPosition();
}

//================ setMinimumSize ===============//

void KMdiMainFrm::setMinimumSize( int minw, int minh )
{
	if ( ( m_mdiMode == KMdi::ToplevelMode ) && !parentWidget() )
		return ;
	DockMainWindow::setMinimumSize( minw, minh );
}

//================ wrapper ===============//

KMdiChildView* KMdiMainFrm::createWrapper( TQWidget *view, const TQString& name, const TQString& shortName )
{
	Q_ASSERT( view ); // if this assert fails, then some part didn't return a widget. Fix the part ;)

	KMdiChildView* pMDICover = new KMdiChildView( name /*caption*/, 0L /*parent*/,
	                                              name.latin1() );
	TQBoxLayout* pLayout = new TQHBoxLayout( pMDICover, 0, -1, "layout" );
	view->reparent( pMDICover, TQPoint( 0, 0 ) );
	pLayout->addWidget( view );
	//  pMDICover->setName(name);
	pMDICover->setTabCaption( shortName );
	pMDICover->setCaption( name );

	const TQPixmap* wndIcon = view->icon();
	if ( wndIcon )
		pMDICover->setIcon( *wndIcon );

	pMDICover->trackIconAndCaptionChanges( view );
	return pMDICover;
}

//================ addWindow ===============//

void KMdiMainFrm::addWindow( KMdiChildView* pWnd, int flags )
{
	addWindow( pWnd, flags, -1 );
}

void KMdiMainFrm::addWindow( KMdiChildView* pWnd, int flags, int index )
{
	if ( windowExists( pWnd, AnyView ) ) //already added
		return;

	if ( flags & KMdi::ToolWindow )
	{
		addToolWindow( pWnd );
		// some kind of cascading
		pWnd->move( m_pMdi->mapToGlobal( m_pMdi->getCascadePoint() ) );

		return ;
	}

	d->closeWindowAction->setEnabled(true);

	// common connections used when under MDI control
	TQObject::connect( pWnd, TQT_SIGNAL( clickedInWindowMenu( int ) ), this, TQT_SLOT( windowMenuItemActivated( int ) ) );
	TQObject::connect( pWnd, TQT_SIGNAL( focusInEventOccurs( KMdiChildView* ) ), this, TQT_SLOT( activateView( KMdiChildView* ) ) );
	TQObject::connect( pWnd, TQT_SIGNAL( childWindowCloseRequest( KMdiChildView* ) ), this, TQT_SLOT( childWindowCloseRequest( KMdiChildView* ) ) );
	TQObject::connect( pWnd, TQT_SIGNAL( attachWindow( KMdiChildView*, bool ) ), this, TQT_SLOT( attachWindow( KMdiChildView*, bool ) ) );
	TQObject::connect( pWnd, TQT_SIGNAL( detachWindow( KMdiChildView*, bool ) ), this, TQT_SLOT( detachWindow( KMdiChildView*, bool ) ) );
	TQObject::connect( pWnd, TQT_SIGNAL( clickedInDockMenu( int ) ), this, TQT_SLOT( dockMenuItemActivated( int ) ) );
	TQObject::connect( pWnd, TQT_SIGNAL( activated( KMdiChildView* ) ), this, TQT_SIGNAL( viewActivated( KMdiChildView* ) ) );
	TQObject::connect( pWnd, TQT_SIGNAL( deactivated( KMdiChildView* ) ), this, TQT_SIGNAL( viewDeactivated( KMdiChildView* ) ) );
	
	if ( index == -1 )
		m_pDocumentViews->append( pWnd );
	else
		m_pDocumentViews->insert( index, pWnd );
	
	if ( m_pTaskBar )
	{
		KMdiTaskBarButton* but = m_pTaskBar->addWinButton( pWnd );
		TQObject::connect( pWnd, TQT_SIGNAL( tabCaptionChanged( const TQString& ) ), but, TQT_SLOT( setNewText( const TQString& ) ) );
	}

	// embed the view depending on the current MDI mode
	if ( m_mdiMode == KMdi::TabPageMode || m_mdiMode == KMdi::IDEAlMode )
	{
		TQPixmap pixmap;
		if ( pWnd->icon() )
			pixmap = *( pWnd->icon() );
		
		m_documentTabWidget->insertTab( pWnd, pixmap, pWnd->tabCaption(), index );

		connect( pWnd, TQT_SIGNAL( iconUpdated( TQWidget*, TQPixmap ) ), m_documentTabWidget, TQT_SLOT( updateIconInView( TQWidget*, TQPixmap ) ) );
		connect( pWnd, TQT_SIGNAL( captionUpdated( TQWidget*, const TQString& ) ), m_documentTabWidget, TQT_SLOT( updateCaptionInView( TQWidget*, const TQString& ) ) );
	}
	else
	{
		if ( ( flags & KMdi::Detach ) || ( m_mdiMode == KMdi::ToplevelMode ) )
		{
			detachWindow( pWnd, !( flags & KMdi::Hide ) );
			emit childViewIsDetachedNow( pWnd ); // fake it because detach won't call it in this case of addWindow-to-MDI
		}
		else
			attachWindow( pWnd, !( flags & KMdi::Hide ), flags & KMdi::UseKMdiSizeHint );

		if ( ( m_bMaximizedChildFrmMode && ( !m_bSDIApplication && ( flags & KMdi::Detach ) )
		       && m_mdiMode != KMdi::ToplevelMode ) || ( flags & KMdi::Maximize ) )
		{
			if ( !pWnd->isMaximized() )
				pWnd->maximize();
		}
		
		if ( !m_bSDIApplication || ( flags & KMdi::Detach ) )
		{
			if ( flags & KMdi::Minimize )
				pWnd->minimize();
			
			if ( !( flags & KMdi::Hide ) )
			{
				if ( pWnd->isAttached() )
					pWnd->mdiParent()->show();
				else
					pWnd->show();
			}
		}
	}
}

//============ addWindow ============//
void KMdiMainFrm::addWindow( KMdiChildView* pWnd, TQRect rectNormal, int flags )
{
	addWindow( pWnd, flags );
	if ( m_bMaximizedChildFrmMode && pWnd->isAttached() )
		pWnd->setRestoreGeometry( rectNormal );
	else
		pWnd->setGeometry( rectNormal );
}

//============ addWindow ============//
void KMdiMainFrm::addWindow( KMdiChildView* pWnd, TQPoint pos, int flags )
{
	addWindow( pWnd, flags );
	if ( m_bMaximizedChildFrmMode && pWnd->isAttached() )
		pWnd->setRestoreGeometry( TQRect( pos, pWnd->restoreGeometry().size() ) );
	else
		pWnd->move( pos );
}



KMdiToolViewAccessor *KMdiMainFrm::createToolWindow()
{
	return new KMdiToolViewAccessor( this );
}


void KMdiMainFrm::deleteToolWindow( TQWidget* pWnd )
{
	if ( m_pToolViews->contains( pWnd ) )
		deleteToolWindow( ( *m_pToolViews ) [ pWnd ] );
}

void KMdiMainFrm::deleteToolWindow( KMdiToolViewAccessor *accessor )
{
	delete accessor;
}

//============ addWindow ============//
KMdiToolViewAccessor *KMdiMainFrm::addToolWindow( TQWidget* pWnd, KDockWidget::DockPosition pos, TQWidget* pTargetWnd,
                                                  int percent, const TQString& tabToolTip, const TQString& tabCaption )
{
	TQWidget* tvta = pWnd;
	KDockWidget* pDW = dockManager->getDockWidgetFromName( pWnd->name() );
	if ( pDW )
	{
		// probably readDockConfig already created the widgetContainer, use that
		pDW->setWidget( pWnd );

		if ( pWnd->icon() )
			pDW->setPixmap( *pWnd->icon() );

		pDW->setTabPageLabel( ( tabCaption == 0 ) ? pWnd->caption() : tabCaption );
		pDW->setToolTipString( tabToolTip );
		dockManager->removeFromAutoCreateList( pDW );
		pWnd = pDW;
	}

	TQRect r = pWnd->geometry();

	KMdiToolViewAccessor *mtva = new KMdiToolViewAccessor( this, pWnd, tabToolTip, ( tabCaption == 0 ) ? pWnd->caption() : tabCaption );
	m_pToolViews->insert( tvta, mtva );

	if ( pos == KDockWidget::DockNone )
	{
		mtva->d->widgetContainer->setEnableDocking( KDockWidget::DockNone );
		mtva->d->widgetContainer->reparent( this, (WFlags)(WType_TopLevel | WType_Dialog), r.topLeft(), true ); //pToolView->isVisible());
	}
	else //add and dock the toolview as a dockwidget view
		mtva->place( pos, pTargetWnd, percent );


	return mtva;
}

//============ attachWindow ============//
void KMdiMainFrm::attachWindow( KMdiChildView *pWnd, bool bShow, bool bAutomaticResize )
{
	pWnd->installEventFilter( this );

	// decide whether window shall be cascaded
	bool bCascade = false;
	TQApplication::sendPostedEvents();
	TQRect frameGeo = pWnd->frameGeometry();
	TQPoint topLeftScreen = pWnd->mapToGlobal( TQPoint( 0, 0 ) );
	TQPoint topLeftMdiChildArea = m_pMdi->mapFromGlobal( topLeftScreen );
	TQRect childAreaGeo = m_pMdi->geometry();
	if ( topLeftMdiChildArea.x() < 0 || topLeftMdiChildArea.y() < 0 ||
	     ( topLeftMdiChildArea.x() + frameGeo.width() > childAreaGeo.width() ) ||
	     ( topLeftMdiChildArea.y() + frameGeo.height() > childAreaGeo.height() ) )
	{
		bCascade = true;
	}

	// create frame and insert child view
	KMdiChildFrm *lpC = new KMdiChildFrm( m_pMdi );
	pWnd->hide();
	if ( !bCascade )
		lpC->move( topLeftMdiChildArea );

	lpC->setClient( pWnd, bAutomaticResize );
	lpC->setFocus();
	pWnd->youAreAttached( lpC );
	if ( ( m_mdiMode == KMdi::ToplevelMode ) && !parentWidget() )
	{
		setMinimumHeight( m_oldMainFrmMinHeight );
		setMaximumHeight( m_oldMainFrmMaxHeight );
		resize( width(), m_oldMainFrmHeight );
		m_oldMainFrmHeight = 0;
		switchToChildframeMode();
	}

	m_pMdi->manageChild( lpC, false, bCascade );
	if ( m_pMdi->topChild() && m_pMdi->topChild() ->isMaximized() )
	{
		TQRect r = lpC->geometry();
		lpC->setGeometry( -lpC->m_pClient->x(), -lpC->m_pClient->y(),
		                  m_pMdi->width() + KMDI_CHILDFRM_DOUBLE_BORDER,
		                  m_pMdi->height() + lpC->captionHeight() + KMDI_CHILDFRM_SEPARATOR + KMDI_CHILDFRM_DOUBLE_BORDER );
		lpC->setRestoreGeometry( r );
	}

	if ( bShow )
	{
		lpC->show();
	}

#undef FocusIn
	TQFocusEvent fe( TQEvent::FocusIn );
	TQApplication::sendEvent( pWnd, &fe );

	m_pCurrentWindow = pWnd;  // required for checking the active item
}

//============= detachWindow ==============//
void KMdiMainFrm::detachWindow( KMdiChildView *pWnd, bool bShow )
{
	if ( pWnd->isAttached() )
	{
		pWnd->removeEventFilter( this );
		pWnd->youAreDetached();
		// this is only if it was attached and you want to detach it
		if ( pWnd->parent() )
		{
			KMdiChildFrm * lpC = pWnd->mdiParent();
			if ( lpC )
			{
				if ( lpC->icon() )
				{
					TQPixmap pixm( *( lpC->icon() ) );
					pWnd->setIcon( pixm );
				}
				TQString capt( lpC->caption() );
				if ( !bShow )
					lpC->hide();
				lpC->unsetClient( m_undockPositioningOffset );
				m_pMdi->destroyChildButNotItsView( lpC, false ); //Do not focus the new top child , we loose focus...
				pWnd->setCaption( capt );
			}
		}
	}
	else
	{
		if ( pWnd->size().isEmpty() || ( pWnd->size() == TQSize( 1, 1 ) ) )
		{
			if ( m_pCurrentWindow )
			{
				pWnd->setGeometry( TQRect( m_pMdi->getCascadePoint( m_pDocumentViews->count() - 1 ), m_pCurrentWindow->size() ) );
			}
			else
			{
				pWnd->setGeometry( TQRect( m_pMdi->getCascadePoint( m_pDocumentViews->count() - 1 ), defaultChildFrmSize() ) );
			}
		}
#ifdef Q_WS_X11
		if ( mdiMode() == KMdi::ToplevelMode )
		{
			XSetTransientForHint( tqt_xdisplay(), pWnd->winId(), topLevelWidget() ->winId() );
		}
#endif

		return ;
	}

#ifdef Q_WS_X11
	if ( mdiMode() == KMdi::ToplevelMode )
	{
		XSetTransientForHint( tqt_xdisplay(), pWnd->winId(), topLevelWidget() ->winId() );
	}
#endif

	// this will show it...
	if ( bShow )
	{
		activateView( pWnd );
	}

	emit childViewIsDetachedNow( pWnd );
}

//============== removeWindowFromMdi ==============//
void KMdiMainFrm::removeWindowFromMdi( KMdiChildView *pWnd )
{
	Q_UNUSED( pWnd );
	//Closes a child window. sends no close event : simply deletes it
	//FIXME something wrong with this, but nobody knows whatcart
#if 0
	if ( !( m_pWinList->removeRef( pWnd ) ) )
		return ;
	if ( m_pWinList->count() == 0 )
		m_pCurrentWindow = 0L;

	TQObject::disconnect( pWnd, TQT_SIGNAL( attachWindow( KMdiChildView*, bool ) ), this, TQT_SLOT( attachWindow( KMdiChildView*, bool ) ) );
	TQObject::disconnect( pWnd, TQT_SIGNAL( detachWindow( KMdiChildView*, bool ) ), this, TQT_SLOT( detachWindow( KMdiChildView*, bool ) ) );
	TQObject::disconnect( pWnd, TQT_SIGNAL( focusInEventOccurs( KMdiChildView* ) ), this, TQT_SLOT( activateView( KMdiChildView* ) ) );
	TQObject::disconnect( pWnd, TQT_SIGNAL( childWindowCloseRequest( KMdiChildView* ) ), this, TQT_SLOT( childWindowCloseRequest( KMdiChildView* ) ) );
	TQObject::disconnect( pWnd, TQT_SIGNAL( clickedInWindowMenu( int ) ), this, TQT_SLOT( windowMenuItemActivated( int ) ) );
	TQObject::disconnect( pWnd, TQT_SIGNAL( clickedInDockMenu( int ) ), this, TQT_SLOT( dockMenuItemActivated( int ) ) );

	if ( m_pTaskBar )
	{
		KMdiTaskBarButton * but = m_pTaskBar->getButton( pWnd );
		if ( but != 0L )
		{
			TQObject::disconnect( pWnd, TQT_SIGNAL( tabCaptionChanged( const TQString& ) ), but, TQT_SLOT( setNewText( const TQString& ) ) );
		}
		m_pTaskBar->removeWinButton( pWnd );
	}

	if ( m_mdiMode == KMdi::TabPageMode )
	{
		if ( m_pWinList->count() == 0 )
		{
			if ( !m_pDockbaseAreaOfDocumentViews )
			{
				m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", TQPixmap(), 0L, "mdi_area_cover" );
				m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );

				m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
				setMainDockWidget( m_pDockbaseAreaOfDocumentViews );
			}
			m_pDockbaseOfTabPage->setDockSite( KDockWidget::DockFullSite );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( KDockWidget::DockCenter );
			m_pDockbaseAreaOfDocumentViews->manualDock( m_pDockbaseOfTabPage, KDockWidget::DockCenter );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( KDockWidget::DockNone );
			m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
			m_pClose->hide();
		}
		KDockWidget* pDockW = ( KDockWidget* ) pWnd->parentWidget();
		pWnd->reparent( 0L, TQPoint( 0, 0 ) );
		pDockW->setWidget( 0L );
		if ( pDockW == m_pDockbaseOfTabPage )
		{
			TQTabWidget * pTab = ( TQTabWidget* ) pDockW->parentWidget() ->parentWidget();
			int cnt = pTab->count();
			m_pDockbaseOfTabPage = ( KDockWidget* ) pTab->page( cnt - 2 );
			if ( pDockW == m_pDockbaseOfTabPage )
			{
				m_pDockbaseOfTabPage = ( KDockWidget* ) pTab->page( cnt - 1 ); // different to the one deleted next
			}
		}
		delete pDockW;
		if ( m_pWinList->count() == 1 )
		{
			m_pWinList->last() ->activate(); // all other views are activated by tab switch
		}
	}
	else if ( pWnd->isAttached() )
	{
		pWnd->mdiParent() ->hide();
		m_pMdi->destroyChildButNotItsView( pWnd->mdiParent() );
	}
	else
	{
		// is not attached
		if ( m_pMdi->getVisibleChildCount() > 0 )
		{
			setActiveWindow();
			m_pCurrentWindow = 0L;
			KMdiChildView* pView = m_pMdi->topChild() ->m_pClient;
			if ( pView )
			{
				pView->activate();
			}
		}
		else if ( m_pWinList->count() > 0 )
		{
			//crash?         m_pWinList->last()->activate();
			//crash?         m_pWinList->last()->setFocus();
		}
	}

	if ( pWnd->isToolView() )
		pWnd->m_bToolView = false;

	if ( !m_pCurrentWindow )
		emit lastChildViewClosed();
#endif
}

//============== closeWindow ==============//
void KMdiMainFrm::closeWindow( KMdiChildView *pWnd, bool layoutTaskBar )
{
	if ( !pWnd )
		return ;
	//Closes a child window. sends no close event : simply deletes it
	m_pDocumentViews->removeRef( pWnd );
	if ( m_pDocumentViews->count() == 0 )
		m_pCurrentWindow = 0L;

	if ( m_pTaskBar )
	{
		m_pTaskBar->removeWinButton( pWnd, layoutTaskBar );
	}

	if ( ( m_mdiMode == KMdi::TabPageMode ) || ( m_mdiMode == KMdi::IDEAlMode ) )
	{
		if ( !m_documentTabWidget )
			return ; //oops
		if ( m_pDocumentViews->count() == 0 )
			m_pClose->hide();
		pWnd->reparent( 0L, TQPoint( 0, 0 ) );
		kdDebug(760) << "-------- 1" << endl;
		if ( m_pDocumentViews->count() == 1 )
		{
			m_pDocumentViews->last() ->activate(); // all other views are activated by tab switch
		}
	}
	if ( ( m_mdiMode == KMdi::TabPageMode ) || ( m_mdiMode == KMdi::IDEAlMode ) )
	{
		if ( m_pDocumentViews->count() == 0 )
		{
			if ( !m_pDockbaseAreaOfDocumentViews )
			{
				m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", TQPixmap(), 0L, "mdi_area_cover" );
				m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );
				m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
				setMainDockWidget( m_pDockbaseAreaOfDocumentViews );
			}
#if 0
			m_pDockbaseOfTabPage->setDockSite( KDockWidget::DockFullSite );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( KDockWidget::DockCenter );
			m_pDockbaseAreaOfDocumentViews->manualDock( m_pDockbaseOfTabPage, KDockWidget::DockCenter );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( KDockWidget::DockNone );
			m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
#endif

			m_pClose->hide();
		}
#if 0
		KDockWidget* pDockW = ( KDockWidget* ) pWnd->parentWidget();
		pWnd->reparent( 0L, TQPoint( 0, 0 ) );
		pDockW->setWidget( 0L );
		if ( pDockW == m_pDockbaseOfTabPage )
		{
			TQTabWidget * pTab = ( TQTabWidget* ) pDockW->parentWidget() ->parentWidget();
			int cnt = pTab->count();
			m_pDockbaseOfTabPage = ( KDockWidget* ) pTab->page( cnt - 2 );
			if ( pDockW == m_pDockbaseOfTabPage )
			{
				m_pDockbaseOfTabPage = ( KDockWidget* ) pTab->page( cnt - 1 ); // different to the one deleted next
			}
		}
		delete pDockW;
#endif

		delete pWnd;
		if ( m_pDocumentViews->count() == 1 )
		{
			m_pDocumentViews->last() ->activate(); // all other views are activated by tab switch
		}
	}
	else if ( pWnd->isAttached() )
	{
		m_pMdi->destroyChild( pWnd->mdiParent() );
	}
	else
	{
		delete pWnd;
		// is not attached
		if ( m_pMdi->getVisibleChildCount() > 0 )
		{
			setActiveWindow();
			m_pCurrentWindow = 0L;
			KMdiChildView* pView = m_pMdi->topChild() ->m_pClient;
			if ( pView )
			{
				pView->activate();
			}
		}
		else if ( m_pDocumentViews->count() > 0 )
		{
			if ( m_pDocumentViews->current() )
			{
				m_pDocumentViews->current() ->activate();
				m_pDocumentViews->current() ->setFocus();
			}
			else
			{
				m_pDocumentViews->last() ->activate();
				m_pDocumentViews->last() ->setFocus();
			}
		}
	}

	if ( !m_pCurrentWindow )
	{
		d->closeWindowAction->setEnabled(false);
		emit lastChildViewClosed();
	}
}

//================== findWindow =================//
KMdiChildView* KMdiMainFrm::findWindow( const TQString& caption )
{
	TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
	{
		if ( ( *it )->caption() == caption )
			return ( *it );
	}
	return 0L;
}

//================== activeWindow ===================//
KMdiChildView* KMdiMainFrm::activeWindow()
{
	return m_pCurrentWindow;
}

//================== windowExists ? =================//
bool KMdiMainFrm::windowExists( KMdiChildView *pWnd, ExistsAs as )
{
	if ( ( as == ToolView ) || ( as == AnyView ) )
	{
		if ( m_pToolViews->contains( pWnd ) )
			return true;
		if ( as == ToolView )
			return false;
	}
	
	if ( m_pDocumentViews->findRef( pWnd ) != -1  )
		return true;

	return false;
}

TQPopupMenu * KMdiMainFrm::windowPopup( KMdiChildView * pWnd, bool bIncludeTaskbarPopup )
{
	m_pWindowPopup->clear();
	if ( bIncludeTaskbarPopup )
	{
		m_pWindowPopup->insertItem( i18n( "Window" ), taskBarPopup( pWnd, false ) );
		m_pWindowPopup->insertSeparator();
	}
	return m_pWindowPopup;
}

//================ taskBarPopup =================//
TQPopupMenu* KMdiMainFrm::taskBarPopup( KMdiChildView *pWnd, bool /*bIncludeWindowPopup*/ )
{
	//returns the g_pTaskBarPopup filled according to the KMdiChildView state
	m_pTaskBarPopup->clear();
	if ( pWnd->isAttached() )
	{
		m_pTaskBarPopup->insertItem( i18n( "Undock" ), pWnd, TQT_SLOT( detach() ) );
		m_pTaskBarPopup->insertSeparator();
		if ( pWnd->isMinimized() || pWnd->isMaximized() )
			m_pTaskBarPopup->insertItem( i18n( "Restore" ), pWnd, TQT_SLOT( restore() ) );
		if ( !pWnd->isMaximized() )
			m_pTaskBarPopup->insertItem( i18n( "Maximize" ), pWnd, TQT_SLOT( maximize() ) );
		if ( !pWnd->isMinimized() )
			m_pTaskBarPopup->insertItem( i18n( "Minimize" ), pWnd, TQT_SLOT( minimize() ) );
	}
	else
		m_pTaskBarPopup->insertItem( i18n( "Dock" ), pWnd, TQT_SLOT( attach() ) );
	m_pTaskBarPopup->insertSeparator();
	m_pTaskBarPopup->insertItem( i18n( "Close" ), pWnd, TQT_SLOT( close() ) );
	// the window has a view...get the window popup
	m_pTaskBarPopup->insertSeparator();
	m_pTaskBarPopup->insertItem( i18n( "Operations" ), windowPopup( pWnd, false ) );  //alvoid recursion
	return m_pTaskBarPopup;
}

void KMdiMainFrm::slotDocCurrentChanged( TQWidget* pWidget )
{
	KMdiChildView * pWnd = static_cast<KMdiChildView*>( pWidget );
	pWnd->m_bMainframesActivateViewIsPending = true;

	bool bActivateNecessary = true;
	if ( m_pCurrentWindow != pWnd )
		m_pCurrentWindow = pWnd;

	if ( m_pTaskBar )
		m_pTaskBar->setActiveButton( pWnd );

	if ( m_documentTabWidget && ( m_mdiMode == KMdi::TabPageMode || m_mdiMode == KMdi::IDEAlMode ) )
	{
		m_documentTabWidget->showPage( pWnd );
		pWnd->activate();
	}
	else
	{
		if ( pWnd->isAttached() )
		{
			if ( bActivateNecessary && ( m_pMdi->topChild() == pWnd->mdiParent() ) )
				pWnd->activate();
			
			pWnd->mdiParent()->raiseAndActivate();
		}
		if ( !pWnd->isAttached() )
		{
			if ( bActivateNecessary )
				pWnd->activate();
			
			m_pMdi->setTopChild( 0L ); // lose focus in the mainframe window
			if ( !pWnd->isActiveWindow() )
				pWnd->setActiveWindow();
			
			pWnd->raise();
		}
	}
	if ( !switching() )
		activeWindow()->updateTimeStamp();
	emit collapseOverlapContainers();
	pWnd->m_bMainframesActivateViewIsPending = false;
}


void KMdiMainFrm::activateView( KMdiChildView* pWnd )
{
	pWnd->m_bMainframesActivateViewIsPending = true;

	bool bActivateNecessary = true;
	if ( m_pCurrentWindow != pWnd )
		m_pCurrentWindow = pWnd;
	else
	{
		bActivateNecessary = false;
		// if this method is called as answer to view->activate(),
		// interrupt it because it's not necessary
		pWnd->m_bInterruptActivation = true;
	}

	if ( m_pTaskBar )
		m_pTaskBar->setActiveButton( pWnd );

	if ( m_documentTabWidget && m_mdiMode == KMdi::TabPageMode || m_mdiMode == KMdi::IDEAlMode )
	{
		m_documentTabWidget->showPage( pWnd );
		pWnd->activate();
	}
	else
	{
		if ( pWnd->isAttached() )
		{
			if ( bActivateNecessary && ( m_pMdi->topChild() == pWnd->mdiParent() ) )
				pWnd->activate();
			
			pWnd->mdiParent() ->raiseAndActivate();
		}
		if ( !pWnd->isAttached() )
		{
			if ( bActivateNecessary )
				pWnd->activate();
			
			m_pMdi->setTopChild( 0L ); // lose focus in the mainframe window
			if ( !pWnd->isActiveWindow() )
				pWnd->setActiveWindow();
			
			pWnd->raise();
		}
	}

	emit collapseOverlapContainers();

	pWnd->m_bMainframesActivateViewIsPending = false;
}

void KMdiMainFrm::taskbarButtonRightClicked( KMdiChildView *pWnd )
{
	activateView( pWnd ); // set focus
	//TQApplication::sendPostedEvents();
	taskBarPopup( pWnd, true ) ->popup( TQCursor::pos() );
}

void KMdiMainFrm::childWindowCloseRequest( KMdiChildView *pWnd )
{
	KMdiViewCloseEvent * ce = new KMdiViewCloseEvent( pWnd );
	TQApplication::postEvent( this, ce );
}

bool KMdiMainFrm::event( TQEvent* e )
{
	if ( e->type() == TQEvent::User )
	{
		KMdiChildView * pWnd = ( KMdiChildView* ) ( ( KMdiViewCloseEvent* ) e )->data();
		if ( pWnd != 0L )
			closeWindow( pWnd );
		return true;
		// A little hack: If MDI child views are moved implicietly by moving
		// the main widget they should know this too. Unfortunately there seems to
		// be no way to catch the move start / move stop situations for the main
		// widget in a clean way. (There is no MouseButtonPress/Release or
		// something like that.) Therefore we do the following: When we get the
		// "first" move event we start a timer and interprete it as "drag begin".
		// If we get the next move event and the timer is running we restart the
		// timer and don't do anything else. If the timer elapses (this meens we
		// haven't had any move event for a while) we interprete this as "drag
		// end". If the moving didn't stop actually, we will later get another
		// "drag begin", so we get a drag end too much, but this would be the same
		// as if the user would stop moving for a little while.
		// Actually we seem to be lucky that the timer does not elapse while we
		// are moving -> so we have no obsolete drag end / begin
	}
	else if ( isVisible() && e->type() == TQEvent::Move )
	{
		if ( m_pDragEndTimer->isActive() )
		{
			// this is not the first move -> stop old timer
			m_pDragEndTimer->stop();
		}
		else
		{
			// this is the first move -> send the drag begin to all concerned views
			TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
			for ( ; ( *it ); ++it )
			{
				KMdiChildFrmDragBeginEvent dragBeginEvent( 0L );
				TQApplication::sendEvent( ( *it ), &dragBeginEvent );
			}
		}
		m_pDragEndTimer->start( 200, true ); // single shot after 200 ms
	}

	return DockMainWindow::event( e );
}

bool KMdiMainFrm::eventFilter( TQObject * /*obj*/, TQEvent *e )
{
	if ( e->type() == TQEvent::Resize && m_mdiMode == KMdi::ToplevelMode )
	{
		verifyToplevelHeight();
		return false; //let the rest of the resize magic do its work
	}
	
	if ( e->type() == TQEvent::FocusIn )
	{
		TQFocusEvent * pFE = ( TQFocusEvent* ) e;
		if ( pFE->reason() == TQFocusEvent::ActiveWindow )
		{
			if ( m_pCurrentWindow && !m_pCurrentWindow->isHidden() &&
			     !m_pCurrentWindow->isAttached() && m_pMdi->topChild() )
			{
				return true;   // eat the event
			}
		}
		if ( m_pMdi )
		{
			static bool focusTCIsPending = false;
			if ( !focusTCIsPending && m_mdiMode == KMdi::ChildframeMode )
			{
				focusTCIsPending = true;
				m_pMdi->focusTopChild();
				focusTCIsPending = false;
			}
		}
	}
	else if ( e->type() == TQEvent::KeyRelease )
	{
		if ( switching() )
		{
			TDEAction * a = actionCollection() ->action( "view_last_window" ) ;
			if ( a )
			{
				const TDEShortcut cut( a->shortcut() );
				const KKeySequence& seq = cut.seq( 0 );
				const KKey& key = seq.key( 0 );
				int modFlags = key.modFlags();
				int state = ( ( TQKeyEvent * ) e ) ->state();
				KKey key2( ( TQKeyEvent * ) e );

				/** these are quite some assumptions:
				 *   The key combination uses exactly one modifier key
				 *   The WIN button in KDE is the meta button in Qt
				 **/
				if ( state != ( ( TQKeyEvent * ) e ) ->stateAfter() &&
				        ( ( modFlags & KKey::CTRL ) > 0 ) == ( ( state & TQt::ControlButton ) > 0 ) &&
				        ( ( modFlags & KKey::ALT ) > 0 ) == ( ( state & TQt::AltButton ) > 0 ) &&
				        ( ( modFlags & KKey::WIN ) > 0 ) == ( ( state & TQt::MetaButton ) > 0 ) )
				{
					activeWindow() ->updateTimeStamp();
					setSwitching( false );
				}
				return true;
			}
			else
			{
				kdDebug(760) << "TDEAction( \"view_last_window\") not found." << endl;
			}
		}
	}
	return false;  // standard event processing
}

/**
 * close all views
 */
void KMdiMainFrm::closeAllViews()
{
	//save the children first to a list, as removing invalidates our iterator
	TQValueList<KMdiChildView*> children;
	for ( KMdiChildView * w = m_pDocumentViews->first();w;w = m_pDocumentViews->next() )
	{
		children.append( w );
	}
	TQValueListIterator<KMdiChildView *> childIt;
	for ( childIt = children.begin(); childIt != children.end(); ++childIt )
	{
		( *childIt )->close();
	}
}


/**
 * iconify all views
 */
void KMdiMainFrm::iconifyAllViews()
{
	kdDebug(760) << k_funcinfo << "minimizing all the views" << endl;
	TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
		( *it )->minimize();
}

/**
 * closes the view of the active (topchild) window
 */
void KMdiMainFrm::closeActiveView()
{
	kdDebug(760) << k_funcinfo << "closing the active view" << endl;
	if ( m_pCurrentWindow )
		m_pCurrentWindow->close();
}

/** find the root dockwidgets and store their geometry */
void KMdiMainFrm::findRootDockWidgets( TQPtrList<KDockWidget>* rootDockWidgetList, TQValueList<TQRect>* positionList )
{
	//nothing is valid
	if ( !rootDockWidgetList && !positionList )
		return ;

	// since we set some windows to toplevel, we must consider the window manager's window frame
	const int frameBorderWidth = 7;  // @todo: Can we / do we need to ask the window manager?
	const int windowTitleHeight = 10; // @todo:    -"-

	TQObjectList* pObjList = queryList( "KDockWidget" );
	if ( pObjList->isEmpty() )
		pObjList = queryList( "KDockWidget_Compat::KDockWidget" );

	TQObjectListIt it( *pObjList );
	// for all dockwidgets (which are children of this mainwindow)
	while ( ( *it ) )
	{
		KDockWidget* dockWindow = 0L; /* pDockW */
		KDockWidget* rootDockWindow = 0L; /* pRootDockWindow */
		KDockWidget* undockCandidate = 0L; /* pUndockCandidate */
		TQWidget* pW = TQT_TQWIDGET( ( *it ) );
		
		// find the oldest ancestor of the current dockwidget that can be undocked
		while ( !pW->isTopLevel() )
		{
			if ( ::tqqt_cast<KDockWidget*>( pW ) ||  pW->inherits( "KDockWidget_Compat::KDockWidget" ) )
			{
				undockCandidate = static_cast<KDockWidget*>( pW );
				if ( undockCandidate->enableDocking() != KDockWidget::DockNone )
					rootDockWindow = undockCandidate;
			}
			pW = pW->parentWidget();
		}
		
		if ( rootDockWindow )
		{
			// if that oldest ancestor is not already in the list, append it
			bool found = false;
			if ( !rootDockWidgetList->isEmpty() )
			{
				TQPtrListIterator<KDockWidget> it2( *rootDockWidgetList );
				for ( ; it2.current() && !found; ++it2 )
				{
					dockWindow = it2.current();
					if ( dockWindow == rootDockWindow )
						found = true;
				}
			}
			
			if ( !found || rootDockWidgetList->isEmpty() )
			{
					rootDockWidgetList->append( dockWindow );
					kdDebug(760) << k_funcinfo << "Appending " << rootDockWindow << " to our list of " <<
						"root dock windows" << endl;
					TQPoint p = rootDockWindow->mapToGlobal( rootDockWindow->pos() ) - rootDockWindow->pos();
					TQRect r( p.x(), p.y() + m_undockPositioningOffset.y(),
					         rootDockWindow->width() - windowTitleHeight - frameBorderWidth * 2,
					         rootDockWindow->height() - windowTitleHeight - frameBorderWidth * 2 );
					positionList->append( r );
			}
		}
		++it;
	}
	delete pObjList;
}

/**
 * undocks all view windows (unix-like)
 */
void KMdiMainFrm::switchToToplevelMode()
{
	if ( m_mdiMode == KMdi::ToplevelMode )
	{
		emit mdiModeHasBeenChangedTo( KMdi::ToplevelMode );
		return ;
	}

	KMdi::MdiMode oldMdiMode = m_mdiMode;

	const int frameBorderWidth = 7;  // @todo: Can we / do we need to ask the window manager?
	setUndockPositioningOffset( TQPoint( 0, ( m_pTaskBar ? m_pTaskBar->height() : 0 ) + frameBorderWidth ) );

	// 1.) select the dockwidgets to be undocked and store their geometry
	TQPtrList<KDockWidget> rootDockWidgetList;
	TQValueList<TQRect> positionList;

	// 2.) undock the MDI views of KMDI
	switch( oldMdiMode )
	{
	case KMdi::ChildframeMode:
		finishChildframeMode();
		break;
	case KMdi::TabPageMode:
		finishTabPageMode();
		break;
	case KMdi::IDEAlMode:
		finishIDEAlMode();
		findRootDockWidgets( &rootDockWidgetList, &positionList );
		break;
	default:
		break; //do nothing
	}
	
	// 3.) undock all these found oldest ancestors (being KDockWidgets)
	TQPtrListIterator<KDockWidget> kdwit( rootDockWidgetList );
	for ( ; ( *kdwit ); ++kdwit )
		( *kdwit )->undock();

	// 4.) recreate the MDI childframe area and hide it
	if ( oldMdiMode == KMdi::TabPageMode || oldMdiMode == KMdi::IDEAlMode )
	{
		if ( !m_pDockbaseAreaOfDocumentViews )
		{
			m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", TQPixmap(), 0L, "mdi_area_cover" );
			m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( KDockWidget::DockNone );
			m_pDockbaseAreaOfDocumentViews->setDockSite( KDockWidget::DockCorner );
			m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
		}
		// set this dock to main view
		setView( m_pDockbaseAreaOfDocumentViews );
		setMainDockWidget( m_pDockbaseAreaOfDocumentViews );
	}
	//	TQApplication::sendPostedEvents(); //why do we need to empty the event queue?
	if ( !parentWidget() )
	{
		//if we don't have a parent widget ( which i expect we wouldn't )
		//make sure we take into account the size of the docks provided by
		//QMainWindow
		int topDockHeight = topDock() ? topDock()->height() : 0;
		int bottomDockHeight = bottomDock() ? bottomDock()->height() : 0;
		int menuBarHeight = hasMenuBar() ? menuBar()->height() : 0;
		if ( m_pDocumentViews->count() != 0 )
			setFixedHeight( height() - m_pDockbaseAreaOfDocumentViews->height() );
		else
		{
			kdDebug(760) << k_funcinfo << "height is: " << height() << endl;
			kdDebug(760) << k_funcinfo << "top dock height: " << topDockHeight << endl;
			kdDebug(760) << k_funcinfo << "bottom dock height: " << bottomDockHeight << endl;
			kdDebug(760) << k_funcinfo << "menu bar height: " << menuBarHeight << endl;
			kdDebug(760) << k_funcinfo << "dock base area height: " << m_pDockbaseAreaOfDocumentViews->height() << endl;
			setFixedHeight( topDockHeight + menuBarHeight );
		}
	} 
   
	//FIXME although i don't know what to fix
	// 5. show the child views again
	TQPtrListIterator<KMdiChildView> tdemdicvit( *m_pDocumentViews );
	for ( tdemdicvit.toFirst(); ( *tdemdicvit ); ++tdemdicvit )
	{
#ifdef Q_WS_X11
		XSetTransientForHint( tqt_xdisplay(), ( *tdemdicvit )->winId(), winId() );
#endif
		( *tdemdicvit )->show();
	}

	// 6.) reset all memorized positions of the undocked ones and show them again
	TQValueList<TQRect>::Iterator qvlqrit;
	TQValueList<TQRect>::Iterator qvlEnd = positionList.end();
	for ( tdemdicvit.toFirst(), qvlqrit = positionList.begin() ; ( *tdemdicvit ) && qvlqrit != qvlEnd; ++tdemdicvit, ++qvlqrit )
	{
		( *tdemdicvit )->setGeometry( ( *qvlqrit ) );
		( *tdemdicvit )->show();
	}

	m_pDockbaseAreaOfDocumentViews->setDockSite( KDockWidget::DockNone );
	m_mdiMode = KMdi::ToplevelMode;
	
	kdDebug(760) << k_funcinfo << "Switch to toplevel mode completed" << endl;
	emit mdiModeHasBeenChangedTo( KMdi::ToplevelMode );

}

void KMdiMainFrm::finishToplevelMode()
{
	m_pDockbaseAreaOfDocumentViews->setDockSite( KDockWidget::DockCorner );
}

/**
 * docks all view windows (Windows-like)
 */
void KMdiMainFrm::switchToChildframeMode()
{
	if ( m_mdiMode == KMdi::ChildframeMode )
	{
		emit mdiModeHasBeenChangedTo( KMdi::ChildframeMode );
		return ;
	}

	TQPtrList<KDockWidget> rootDockWidgetList;
	TQValueList<TQRect> positionList;
	
	if ( m_mdiMode == KMdi::TabPageMode )
	{
		kdDebug(760) << k_funcinfo << "finishing tab page mode" << endl;
		// select the dockwidgets to be undocked and store their geometry
		findRootDockWidgets( &rootDockWidgetList, &positionList );
		kdDebug(760) << k_funcinfo << "Found " << rootDockWidgetList.count() << " widgets to undock" << endl;
		
		// undock all these found oldest ancestors (being KDockWidgets)
		TQPtrListIterator<KDockWidget> it( rootDockWidgetList );
		for ( ; ( *it ) ; ++it )
			( *it )->undock();

		finishTabPageMode();
	}
	else if ( m_mdiMode == KMdi::ToplevelMode )
	{
		finishToplevelMode();
	}
	else if ( m_mdiMode == KMdi::IDEAlMode )
	{
		kdDebug(760) << k_funcinfo << "finishing ideal mode" << endl;
		finishIDEAlMode( false );

		// select the dockwidgets to be undocked and store their geometry
		findRootDockWidgets( &rootDockWidgetList, &positionList );
		kdDebug(760) << k_funcinfo << "Found " << rootDockWidgetList.count() << " widgets to undock" << endl;
		
		// undock all these found oldest ancestors (being KDockWidgets)
		TQPtrListIterator<KDockWidget> it( rootDockWidgetList );
		for ( ; ( *it ) ; ++it )
			( *it )->undock();

		m_mdiMode = KMdi::TabPageMode;
		finishTabPageMode();
		m_mdiMode = KMdi::IDEAlMode;
	}

	if ( !m_pDockbaseAreaOfDocumentViews )
	{
		// cover KMdi's childarea by a dockwidget
		m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", TQPixmap(), 0L, "mdi_area_cover" );
		m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );
		m_pDockbaseAreaOfDocumentViews->setEnableDocking( KDockWidget::DockNone );
		m_pDockbaseAreaOfDocumentViews->setDockSite( KDockWidget::DockCorner );
		m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
		kdDebug(760) << k_funcinfo << "childarea is now covered by a dockwidget" << endl;
	}
	
	if ( m_pDockbaseAreaOfDocumentViews->isTopLevel() )
	{
		// set this dock to main view
		setView( m_pDockbaseAreaOfDocumentViews );
		setMainDockWidget( m_pDockbaseAreaOfDocumentViews );
		m_pDockbaseAreaOfDocumentViews->setEnableDocking( KDockWidget::DockNone );
		m_pDockbaseAreaOfDocumentViews->setDockSite( KDockWidget::DockCorner );
		kdDebug(760) << k_funcinfo << "Dock base area has been set to the main view" << endl;
	}
	m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi ); //JW
	m_pDockbaseAreaOfDocumentViews->show();
	
	if ( ( m_mdiMode == KMdi::TabPageMode ) || ( m_mdiMode == KMdi::IDEAlMode ) )
	{
		kdDebug(760) << k_funcinfo << "trying to dock back the undock toolviews" << endl;
		TQPtrListIterator<KDockWidget> it( rootDockWidgetList );
		for ( ; ( *it ); ++it )
			( *it )->dockBack();
	}

	if ( m_mdiMode == KMdi::ToplevelMode && m_pTempDockSession )
	{
		// restore the old dock scenario which we memorized at the time we switched to toplevel mode
		kdDebug(760) << k_funcinfo << "Restoring old dock scenario memorized from toplevel mode" << endl;
		TQDomElement oldDockState = m_pTempDockSession->namedItem( "cur_dock_state" ).toElement();
		readDockConfig( oldDockState );
	}

	KMdi::MdiMode oldMdiMode = m_mdiMode;
	m_mdiMode = KMdi::ChildframeMode;

	//FIXME although i don't know what to fix.
	TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
	{
		KMdiChildView* pView = ( *it );
		if ( !pView->isToolView() && pView->isAttached() )
			attachWindow( pView, true );
	}
	for ( it.toFirst(); ( *it ); ++it )
	{
		KMdiChildView* pView = ( *it );
		if ( !pView->isToolView() )
			pView->show();
	}
	if ( ( oldMdiMode == KMdi::ToplevelMode ) && !parentWidget() )
	{
		setMinimumHeight( m_oldMainFrmMinHeight );
		setMaximumHeight( m_oldMainFrmMaxHeight );
		resize( width(), m_oldMainFrmHeight );
		m_oldMainFrmHeight = 0;
		kdDebug(760) << k_funcinfo << "left top level mode completely" << endl;
		emit leftTopLevelMode();
	}
	emit mdiModeHasBeenChangedTo( KMdi::ChildframeMode );
}

void KMdiMainFrm::finishChildframeMode()
{
	// save the old dock scenario of the dockwidget-like tool views to a DOM tree
	kdDebug(760) << k_funcinfo << "saving the current dock scenario" << endl;
	delete m_pTempDockSession;
	m_pTempDockSession = new TQDomDocument( "docksession" );
	TQDomElement curDockState = m_pTempDockSession->createElement( "cur_dock_state" );
	m_pTempDockSession->appendChild( curDockState );
	writeDockConfig( curDockState );

	// detach all non-tool-views to toplevel
	kdDebug(760) << k_funcinfo << "detaching all document views and moving them to toplevel" << endl;
	TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
	{
		KMdiChildView* pView = ( *it );
		if ( pView->isToolView() )
			continue;
		if ( pView->isAttached() )
		{
			if ( pView->isMaximized() )
				pView->mdiParent()->setGeometry( 0, 0, m_pMdi->width(), m_pMdi->height() );
			detachWindow( pView, false );
		}
	}
}

/**
 * Docks all view windows (Windows-like)
 */
void KMdiMainFrm::switchToTabPageMode()
{
	if ( m_mdiMode == KMdi::TabPageMode )
	{
		emit mdiModeHasBeenChangedTo( KMdi::TabPageMode );
		return ;  // nothing need to be done
	}

	switch( m_mdiMode )
	{
	case KMdi::ChildframeMode:
		finishChildframeMode();
		break;
	case KMdi::ToplevelMode:
		finishToplevelMode();
		break;
	case KMdi::IDEAlMode:
		finishIDEAlMode( false );
		emit mdiModeHasBeenChangedTo( KMdi::TabPageMode );
		m_mdiMode = KMdi::TabPageMode;
		return;
		break;
	default:
		break;
	}
	
	setupTabbedDocumentViewSpace();
	m_mdiMode = KMdi::TabPageMode;
	if ( m_pCurrentWindow )
		m_pCurrentWindow->setFocus();

	m_pTaskBar->switchOn( false );

	if ( m_pClose )
	{
		TQObject::connect( m_pClose, TQT_SIGNAL( clicked() ), this, TQT_SLOT( closeViewButtonPressed() ) );
		if ( m_pDocumentViews->count() > 0 )
			m_pClose->show();
	}
	else
		kdDebug(760) << "close button nonexistant. strange things might happen" << endl;
	
	kdDebug(760) << "Switch to tab page mode complete" << endl;
	emit mdiModeHasBeenChangedTo( KMdi::TabPageMode );
}

void KMdiMainFrm::finishTabPageMode()
{
	// if tabified, release all views from their docking covers
	if ( m_mdiMode == KMdi::TabPageMode )
	{
		m_pClose->hide();
		TQObject::disconnect( m_pClose, TQT_SIGNAL( clicked() ), this, TQT_SLOT( closeViewButtonPressed() ) );

		TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
		for ( ; it.current(); ++it )
		{
			KMdiChildView* pView = it.current();
			if ( pView->isToolView() )
				continue;
			kdDebug(760) << "KMdiMainFrm::finishTabPageMode: in loop" << endl;
			TQSize mins = pView->minimumSize();
			TQSize maxs = pView->maximumSize();
			TQSize sz = pView->size();
			TQWidget* pParent = pView->parentWidget();
			TQPoint p( pParent->mapToGlobal( pParent->pos() ) - pParent->pos() + m_undockPositioningOffset );
			m_documentTabWidget->removePage( pView );
			pView->reparent( 0, 0, p );
			//         pView->reparent(0,0,p);
			pView->resize( sz );
			pView->setMinimumSize( mins.width(), mins.height() );
			pView->setMaximumSize( maxs.width(), maxs.height() );
			//         ((KDockWidget*)pParent)->undock(); // this destroys the dockwiget cover, too
			//         pParent->close();
			//         delete pParent;
			//         if (centralWidget() == pParent) {
			//            setCentralWidget(0L); // avoid dangling pointer
			//         }
		}
		delete m_documentTabWidget;
		m_documentTabWidget = 0;
		m_pTaskBar->switchOn( true );
	}
}



void KMdiMainFrm::setupTabbedDocumentViewSpace()
{
	// resize to childframe mode size of the mainwindow if we were in toplevel mode
	if ( ( m_mdiMode == KMdi::ToplevelMode ) && !parentWidget() )
	{
		setMinimumHeight( m_oldMainFrmMinHeight );
		setMaximumHeight( m_oldMainFrmMaxHeight );
		resize( width(), m_oldMainFrmHeight );
		m_oldMainFrmHeight = 0;
		//tqDebug("TopLevelMode off");
		emit leftTopLevelMode();
		TQApplication::sendPostedEvents();

		// restore the old dock szenario which we memorized at the time we switched to toplevel mode
		if ( m_pTempDockSession )
		{
			TQDomElement oldDockState = m_pTempDockSession->namedItem( "cur_dock_state" ).toElement();
			readDockConfig( oldDockState );
		}
	}

#if 0
	if ( m_pDockbaseOfTabPage != m_pDockbaseAreaOfDocumentViews )
	{
		delete m_pDockbaseOfTabPage;
		m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
	}
#endif
	delete m_documentTabWidget;
	m_documentTabWidget = new KMdiDocumentViewTabWidget( m_pDockbaseAreaOfDocumentViews );
	connect( m_documentTabWidget, TQT_SIGNAL( currentChanged( TQWidget* ) ), this, TQT_SLOT( slotDocCurrentChanged( TQWidget* ) ) );
	m_pDockbaseAreaOfDocumentViews->setWidget( m_documentTabWidget );
	m_documentTabWidget->show();
	TQPtrListIterator<KMdiChildView> it4( *m_pDocumentViews );
	for ( ; it4.current(); ++it4 )
	{
		KMdiChildView* pView = it4.current();
		m_documentTabWidget->addTab( pView, pView->icon() ? *( pView->icon() ) : TQPixmap(), pView->tabCaption() );
		/*
		   connect(pView,TQT_SIGNAL(iconOrCaptionUdpated(TQWidget*,TQPixmap,const TQString&)),
		   m_documentTabWidget,TQT_SLOT(updateView(TQWidget*,TQPixmap,const TQString&)));
		 */
		connect( pView, TQT_SIGNAL( iconUpdated( TQWidget*, TQPixmap ) ), m_documentTabWidget, TQT_SLOT( updateIconInView( TQWidget*, TQPixmap ) ) );
		connect( pView, TQT_SIGNAL( captionUpdated( TQWidget*, const TQString& ) ), m_documentTabWidget, TQT_SLOT( updateCaptionInView( TQWidget*, const TQString& ) ) );

	}
}


void KMdiMainFrm::setIDEAlModeStyle( int flags )
{
	d->m_styleIDEAlMode = flags; // see KMultiTabBar for the first 3 bits
	if ( m_leftContainer )
	{
		KMdiDockContainer * tmpL = ::tqqt_cast<KMdiDockContainer*>( m_leftContainer->getWidget() );
		if ( tmpL )
			tmpL->setStyle( flags );
	}

	if ( m_rightContainer )
	{
		KMdiDockContainer * tmpR = ::tqqt_cast<KMdiDockContainer*>( m_rightContainer->getWidget() );
		if ( tmpR )
			tmpR->setStyle( flags );
	}

	if ( m_topContainer )
	{
		KMdiDockContainer * tmpT = ::tqqt_cast<KMdiDockContainer*>( m_topContainer->getWidget() );
		if ( tmpT )
			tmpT->setStyle( flags );
	}

	if ( m_bottomContainer )
	{
		KMdiDockContainer * tmpB = ::tqqt_cast<KMdiDockContainer*>( m_bottomContainer->getWidget() );
		if ( tmpB )
			tmpB->setStyle( flags );
	}
}

void KMdiMainFrm::setToolviewStyle( int flag )
{
	if ( m_mdiMode == KMdi::IDEAlMode )
	{
		setIDEAlModeStyle( flag );
	}
	d->m_toolviewStyle = flag;
	bool toolviewExists = false;
	TQMap<TQWidget*, KMdiToolViewAccessor*>::Iterator it;
	for ( it = m_pToolViews->begin(); it != m_pToolViews->end(); ++it )
	{
		KDockWidget *dockWidget = tqt_dynamic_cast<KDockWidget*>( it.data()->wrapperWidget() );
		if ( dockWidget )
		{
			switch ( flag )
			{
			case KMdi::IconOnly:
				dockWidget->setTabPageLabel( TQString::null );
				dockWidget->setPixmap( *( it.data()->wrappedWidget()->icon() ) );
				break;
			case KMdi::TextOnly:
				dockWidget->setPixmap(); //FIXME: Does not hide the icon in the IDEAl mode.
				dockWidget->setTabPageLabel( it.data()->wrappedWidget()->caption() );
				break;
			case KMdi::TextAndIcon:
				dockWidget->setPixmap( *( it.data()->wrappedWidget()->icon() ) );
				dockWidget->setTabPageLabel( it.data()->wrappedWidget()->caption() );
			default:
				break;
			}
			toolviewExists = true;
		}
	}
	
	if ( toolviewExists )
	{
		//workaround for the above FIXME to make switching to TextOnly mode work in IDEAl as well. Be sure that this version of switch* is called.
		if ( m_mdiMode == KMdi::IDEAlMode && flag == KMdi::TextOnly )
		{
			KMdiMainFrm::switchToTabPageMode();
			KMdiMainFrm::switchToIDEAlMode();
		}
		else
		{
			writeDockConfig();
			readDockConfig();
		}
	}
}

/**
 * Docks all view windows (Windows-like)
 */
void KMdiMainFrm::switchToIDEAlMode()
{
	kdDebug(760) << k_funcinfo << "switching to IDEAl mode" << endl;
	
	if ( m_mdiMode == KMdi::IDEAlMode )
	{
		emit mdiModeHasBeenChangedTo( KMdi::IDEAlMode );
		return ;  // nothing need to be done
	}
	
	switch( m_mdiMode )
	{
	case KMdi::ChildframeMode:
		finishChildframeMode();
		break;
	case KMdi::ToplevelMode:
		finishToplevelMode();
		break;
	case KMdi::TabPageMode:
		m_mdiMode = KMdi::IDEAlMode;
		setupToolViewsForIDEALMode();
		emit mdiModeHasBeenChangedTo( KMdi::IDEAlMode );
		return;
		break;
	default:
		break;
	}

	setupTabbedDocumentViewSpace();
	m_mdiMode = KMdi::IDEAlMode;
	setupToolViewsForIDEALMode();

	if ( m_pCurrentWindow )
		m_pCurrentWindow->setFocus();

	m_pTaskBar->switchOn( false );

	if ( m_pClose )
	{
		TQObject::connect( m_pClose, TQT_SIGNAL( clicked() ), this, TQT_SLOT( closeViewButtonPressed() ) );
		if ( m_pDocumentViews->count() > 0 )
			m_pClose->show();
	}
	else
		kdWarning(760) << k_funcinfo << "close button pointer does not exist!" << endl;

	kdDebug(760) << k_funcinfo << "switch to IDEAl mode complete" << endl;

	emit mdiModeHasBeenChangedTo( KMdi::IDEAlMode );
}


void KMdiMainFrm::dockToolViewsIntoContainers( TQPtrList<KDockWidget>& widgetsToReparent, KDockWidget *container )
{
	TQPtrListIterator<KDockWidget> it( widgetsToReparent );
	for ( ; ( *it ); ++it )
	{
		( *it )->manualDock( container, KDockWidget::DockCenter, 20 );
		( *it )->loseFormerBrotherDockWidget();
	}
}

void KMdiMainFrm::findToolViewsDockedToMain( TQPtrList<KDockWidget>* list, KDockWidget::DockPosition dprtmw )
{
	KDockWidget* mainDock = getMainDockWidget();
	if ( mainDock->parentDockTabGroup() )
	{
		mainDock = tqt_dynamic_cast<KDockWidget*>( mainDock->parentDockTabGroup()->parent() );
		// FIXME: will likely crash below due to unchecked cast
	}

	if ( !mainDock )
	{
		kdDebug(760) << k_funcinfo << "mainDock invalid. No main dock widget found." << endl;
		return;
	}
	
	KDockWidget* widget = mainDock->findNearestDockWidget( dprtmw );
	if ( widget && widget->parentDockTabGroup() )
	{
		widget = static_cast<KDockWidget*>( TQT_TQWIDGET(widget->parentDockTabGroup() ->parent()) );

		if ( widget )
		{
			KDockTabGroup* tg = tqt_dynamic_cast<KDockTabGroup*>( widget->getWidget() );
			if ( tg )
			{
				kdDebug(760) << k_funcinfo << "KDockTabGroup found" << endl;
				for ( int i = 0;i < tg->count();i++ )
					list->append( static_cast<KDockWidget*>( tg->page( i ) ) );
			}
			else
				list->append( widget );
		}
		else
			kdDebug(760) << k_funcinfo << "no widget found" << endl;
	}
	else
		kdDebug(760) << "No main dock widget found" << endl;
}


void KMdiMainFrm::setupToolViewsForIDEALMode()
{
	m_leftContainer = createDockWidget( "KMdiDock::leftDock", SmallIcon( "application-vnd.tde.misc" ), 0L, "Left Dock" );
	m_rightContainer = createDockWidget( "KMdiDock::rightDock", SmallIcon( "application-vnd.tde.misc" ), 0L, "Right Dock" );
	m_topContainer = createDockWidget( "KMdiDock::topDock", SmallIcon( "application-vnd.tde.misc" ), 0L, "Top Dock" );
	m_bottomContainer = createDockWidget( "KMdiDock::bottomDock", SmallIcon( "application-vnd.tde.misc" ), 0L, "Bottom Dock" );

	KDockWidget *mainDock = getMainDockWidget();
	KDockWidget *w = mainDock;
	if ( mainDock->parentDockTabGroup() )
		w = static_cast<KDockWidget*>( TQT_TQWIDGET(mainDock->parentDockTabGroup()->parent()) );

	TQPtrList<KDockWidget> leftReparentWidgets;
	TQPtrList<KDockWidget> rightReparentWidgets;
	TQPtrList<KDockWidget> bottomReparentWidgets;
	TQPtrList<KDockWidget> topReparentWidgets;

	if ( mainDock->parentDockTabGroup() )
		mainDock = static_cast<KDockWidget*>( TQT_TQWIDGET(mainDock->parentDockTabGroup() ->parent()) );

	findToolViewsDockedToMain( &leftReparentWidgets, KDockWidget::DockLeft );
	findToolViewsDockedToMain( &rightReparentWidgets, KDockWidget::DockRight );
	findToolViewsDockedToMain( &bottomReparentWidgets, KDockWidget::DockBottom );
	findToolViewsDockedToMain( &topReparentWidgets, KDockWidget::DockTop );

	mainDock->setEnableDocking( KDockWidget::DockNone ); //::DockCorner);
	mainDock->setDockSite( KDockWidget::DockCorner );


	KMdiDockContainer *tmpDC;
	m_leftContainer->setWidget( tmpDC = new KMdiDockContainer( m_leftContainer, this, KDockWidget::DockLeft, d->m_styleIDEAlMode ) );
	m_leftContainer->setEnableDocking( KDockWidget::DockLeft );
	m_leftContainer->manualDock( mainDock, KDockWidget::DockLeft, 20 );
	tmpDC->init();
	if ( m_mdiGUIClient )
		connect ( this, TQT_SIGNAL( toggleLeft() ), tmpDC, TQT_SLOT( toggle() ) );
	connect( this, TQT_SIGNAL( collapseOverlapContainers() ), tmpDC, TQT_SLOT( collapseOverlapped() ) );
	connect( tmpDC, TQT_SIGNAL( activated( KMdiDockContainer* ) ), this, TQT_SLOT( setActiveToolDock( KMdiDockContainer* ) ) );
	connect( tmpDC, TQT_SIGNAL( deactivated( KMdiDockContainer* ) ), this, TQT_SLOT( removeFromActiveDockList( KMdiDockContainer* ) ) );

	m_rightContainer->setWidget( tmpDC = new KMdiDockContainer( m_rightContainer, this, KDockWidget::DockRight, d->m_styleIDEAlMode ) );
	m_rightContainer->setEnableDocking( KDockWidget::DockRight );
	m_rightContainer->manualDock( mainDock, KDockWidget::DockRight, 80 );
	tmpDC->init();
	if ( m_mdiGUIClient )
		connect ( this, TQT_SIGNAL( toggleRight() ), tmpDC, TQT_SLOT( toggle() ) );
	connect( this, TQT_SIGNAL( collapseOverlapContainers() ), tmpDC, TQT_SLOT( collapseOverlapped() ) );
	connect( tmpDC, TQT_SIGNAL( activated( KMdiDockContainer* ) ), this, TQT_SLOT( setActiveToolDock( KMdiDockContainer* ) ) );
	connect( tmpDC, TQT_SIGNAL( deactivated( KMdiDockContainer* ) ), this, TQT_SLOT( removeFromActiveDockList( KMdiDockContainer* ) ) );

	m_topContainer->setWidget( tmpDC = new KMdiDockContainer( m_topContainer, this, KDockWidget::DockTop, d->m_styleIDEAlMode ) );
	m_topContainer->setEnableDocking( KDockWidget::DockTop );
	m_topContainer->manualDock( mainDock, KDockWidget::DockTop, 20 );
	tmpDC->init();
	if ( m_mdiGUIClient )
		connect ( this, TQT_SIGNAL( toggleTop() ), tmpDC, TQT_SLOT( toggle() ) );
	connect( this, TQT_SIGNAL( collapseOverlapContainers() ), tmpDC, TQT_SLOT( collapseOverlapped() ) );
	connect( tmpDC, TQT_SIGNAL( activated( KMdiDockContainer* ) ), this, TQT_SLOT( setActiveToolDock( KMdiDockContainer* ) ) );
	connect( tmpDC, TQT_SIGNAL( deactivated( KMdiDockContainer* ) ), this, TQT_SLOT( removeFromActiveDockList( KMdiDockContainer* ) ) );

	m_bottomContainer->setWidget( tmpDC = new KMdiDockContainer( m_bottomContainer, this, KDockWidget::DockBottom, d->m_styleIDEAlMode ) );
	m_bottomContainer->setEnableDocking( KDockWidget::DockBottom );
	m_bottomContainer->manualDock( mainDock, KDockWidget::DockBottom, 80 );
	tmpDC->init();
	if ( m_mdiGUIClient )
		connect ( this, TQT_SIGNAL( toggleBottom() ), tmpDC, TQT_SLOT( toggle() ) );
	connect( this, TQT_SIGNAL( collapseOverlapContainers() ), tmpDC, TQT_SLOT( collapseOverlapped() ) );
	connect( tmpDC, TQT_SIGNAL( activated( KMdiDockContainer* ) ), this, TQT_SLOT( setActiveToolDock( KMdiDockContainer* ) ) );
	connect( tmpDC, TQT_SIGNAL( deactivated( KMdiDockContainer* ) ), this, TQT_SLOT( removeFromActiveDockList( KMdiDockContainer* ) ) );

	m_leftContainer->setDockSite( KDockWidget::DockCenter );
	m_rightContainer->setDockSite( KDockWidget::DockCenter );
	m_topContainer->setDockSite( KDockWidget::DockCenter );
	m_bottomContainer->setDockSite( KDockWidget::DockCenter );

	dockToolViewsIntoContainers( leftReparentWidgets, m_leftContainer );
	dockToolViewsIntoContainers( rightReparentWidgets, m_rightContainer );
	dockToolViewsIntoContainers( bottomReparentWidgets, m_bottomContainer );
	dockToolViewsIntoContainers( topReparentWidgets, m_topContainer );


	dockManager->setSpecialLeftDockContainer( m_leftContainer );
	dockManager->setSpecialRightDockContainer( m_rightContainer );
	dockManager->setSpecialTopDockContainer( m_topContainer );
	dockManager->setSpecialBottomDockContainer( m_bottomContainer );


	( ( KMdiDockContainer* ) ( m_leftContainer->getWidget() ) ) ->hideIfNeeded();
	( ( KMdiDockContainer* ) ( m_rightContainer->getWidget() ) ) ->hideIfNeeded();
	( ( KMdiDockContainer* ) ( m_topContainer->getWidget() ) ) ->hideIfNeeded();
	( ( KMdiDockContainer* ) ( m_bottomContainer->getWidget() ) ) ->hideIfNeeded();

}



void KMdiMainFrm::finishIDEAlMode( bool full )
{
	// if tabified, release all views from their docking covers
	if ( m_mdiMode == KMdi::IDEAlMode )
	{
		assert( m_pClose );
		m_pClose->hide();
		TQObject::disconnect( m_pClose, TQT_SIGNAL( clicked() ), this, TQT_SLOT( closeViewButtonPressed() ) );


		TQStringList leftNames;
		leftNames = prepareIdealToTabs( m_leftContainer );
		int leftWidth = m_leftContainer->width();

		TQStringList rightNames;
		rightNames = prepareIdealToTabs( m_rightContainer );
		int rightWidth = m_rightContainer->width();

		TQStringList topNames;
		topNames = prepareIdealToTabs( m_topContainer );
		int topHeight = m_topContainer->height();

		TQStringList bottomNames;
		bottomNames = prepareIdealToTabs( m_bottomContainer );
		int bottomHeight = m_bottomContainer->height();


		kdDebug(760) << "leftNames" << leftNames << endl;
		kdDebug(760) << "rightNames" << rightNames << endl;
		kdDebug(760) << "topNames" << topNames << endl;
		kdDebug(760) << "bottomNames" << bottomNames << endl;

		delete m_leftContainer;
		m_leftContainer = 0;
		delete m_rightContainer;
		m_rightContainer = 0;
		delete m_bottomContainer;
		m_bottomContainer = 0;
		delete m_topContainer;
		m_topContainer = 0;


		idealToolViewsToStandardTabs( bottomNames, KDockWidget::DockBottom, bottomHeight );
		idealToolViewsToStandardTabs( leftNames, KDockWidget::DockLeft, leftWidth );
		idealToolViewsToStandardTabs( rightNames, KDockWidget::DockRight, rightWidth );
		idealToolViewsToStandardTabs( topNames, KDockWidget::DockTop, topHeight );

		TQApplication::sendPostedEvents();

		if ( !full )
			return ;

		TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
		for ( ; it.current(); ++it )
		{
			KMdiChildView* pView = it.current();
			if ( pView->isToolView() )
				continue;
			TQSize mins = pView->minimumSize();
			TQSize maxs = pView->maximumSize();
			TQSize sz = pView->size();
			TQWidget* pParent = pView->parentWidget();
			TQPoint p( pParent->mapToGlobal( pParent->pos() ) - pParent->pos() + m_undockPositioningOffset );
			pView->reparent( 0, 0, p );
			pView->reparent( 0, 0, p );
			pView->resize( sz );
			pView->setMinimumSize( mins.width(), mins.height() );
			pView->setMaximumSize( maxs.width(), maxs.height() );
			KDockWidget* pDockW = 0L;
			// find the oldest ancestor of the current dockwidget that can be undocked
			do
			{
				if ( pParent->inherits( "KDockWidget" ) || pParent->inherits( "KDockWidget_Compat::KDockWidget" ) )
				{
					pDockW = ( KDockWidget* ) pParent;
					pDockW->undock(); // this destroys the dockwiget cover, too
					if ( pParent != m_pDockbaseAreaOfDocumentViews )
					{
						pParent->close();
						delete pParent;
					}
				}
				else
				{
					pParent = pParent->parentWidget();
				}
			}
			while ( pParent && !pDockW );
			if ( centralWidget() == pParent )
			{
				setCentralWidget( 0L ); // avoid dangling pointer
			}
		}
		m_pTaskBar->switchOn( true );

	}

}

TQStringList KMdiMainFrm::prepareIdealToTabs( KDockWidget* container )
{
	KDockContainer * pDW = tqt_dynamic_cast<KDockContainer*>( container->getWidget() );
	TQStringList widgetNames = ( ( KMdiDockContainer* ) pDW ) ->containedWidgets();
	for ( TQStringList::iterator it = widgetNames.begin();it != widgetNames.end();++it )
	{
		KDockWidget* dw = ( KDockWidget* ) manager() ->getDockWidgetFromName( *it );
		dw->undock();
		dw->setLatestKDockContainer( 0 );
		dw->loseFormerBrotherDockWidget();
	}
	return widgetNames;
}

void KMdiMainFrm::idealToolViewsToStandardTabs( TQStringList widgetNames, KDockWidget::DockPosition pos, int size )
{
	Q_UNUSED( size )

	KDockWidget * mainDock = getMainDockWidget();
	if ( mainDock->parentDockTabGroup() )
	{
		mainDock = static_cast<KDockWidget*>( TQT_TQWIDGET(mainDock->parentDockTabGroup() ->parent()) );
	}

	if ( widgetNames.count() > 0 )
	{
		TQStringList::iterator it = widgetNames.begin();
		KDockWidget *dwpd = manager() ->getDockWidgetFromName( *it );
		if ( !dwpd )
		{
			kdDebug(760) << "Fatal error in finishIDEAlMode" << endl;
			return ;
		}
		dwpd->manualDock( mainDock, pos, 20 );
		++it;
		for ( ;it != widgetNames.end();++it )
		{
			KDockWidget *tmpdw = manager() ->getDockWidgetFromName( *it );
			if ( !tmpdw )
			{
				kdDebug(760) << "Fatal error in finishIDEAlMode" << endl;
				return ;
			}
			tmpdw->manualDock( dwpd, KDockWidget::DockCenter, 20 );
		}

#if 0
		TQWidget *wid = dwpd->parentDockTabGroup();
		if ( !wid )
			wid = dwpd;
		wid->setGeometry( 0, 0, 20, 20 );
		/*  wid->resize(
		    ((pos==KDockWidget::DockLeft) || (pos==KDockWidget::DockRight))?size:wid->width(),
		    ((pos==KDockWidget::DockLeft) || (pos==KDockWidget::DockRight))?wid->height():size);
		 */
#endif

	}

}


/**
 * redirect the signal for insertion of buttons to an own slot
 * that means: If the menubar (where the buttons should be inserted) is given,
 *             QextMDI can insert them automatically.
 *             Otherwise only signals can be emitted to tell the outside that
 *             someone must do this job itself.
 */
void KMdiMainFrm::setMenuForSDIModeSysButtons( KMenuBar* pMenuBar )
{
	if ( m_bSDIApplication )   // there are no buttons in the menubar in this mode (although the view is always maximized)
		return ;

	m_pMainMenuBar = pMenuBar;
	if ( m_pMainMenuBar == 0L )
		return ;  // use setMenuForSDIModeSysButtons( 0L) for unsetting the external main menu!

	if ( !m_pUndock )
		m_pUndock = new TQToolButton( pMenuBar );
	if ( !m_pRestore )
		m_pRestore = new TQToolButton( pMenuBar );
	if ( !m_pMinimize )
		m_pMinimize = new TQToolButton( pMenuBar );
	if ( !m_pClose )
		m_pClose = new TQToolButton( pMenuBar );
	m_pUndock->setAutoRaise( false );
	m_pMinimize->setAutoRaise( false );
	m_pRestore->setAutoRaise( false );
	m_pClose->setAutoRaise( false );

	setSysButtonsAtMenuPosition();

	delete m_pUndockButtonPixmap;
	delete m_pMinButtonPixmap;
	delete m_pRestoreButtonPixmap;
	delete m_pCloseButtonPixmap;
	// create the decoration pixmaps
	if ( frameDecorOfAttachedViews() == KMdi::Win95Look )
	{
		m_pUndockButtonPixmap = new TQPixmap( win_undockbutton );
		m_pMinButtonPixmap = new TQPixmap( win_minbutton );
		m_pRestoreButtonPixmap = new TQPixmap( win_restorebutton );
		m_pCloseButtonPixmap = new TQPixmap( win_closebutton );
	}
	else if ( frameDecorOfAttachedViews() == KMdi::KDE1Look )
	{
		m_pUndockButtonPixmap = new TQPixmap( kde_undockbutton );
		m_pMinButtonPixmap = new TQPixmap( kde_minbutton );
		m_pRestoreButtonPixmap = new TQPixmap( kde_restorebutton );
		m_pCloseButtonPixmap = new TQPixmap( kde_closebutton );
		m_pUndock->setAutoRaise( true );
		m_pMinimize->setAutoRaise( true );
		m_pRestore->setAutoRaise( true );
		m_pClose->setAutoRaise( true );
	}
	else if ( frameDecorOfAttachedViews() == KMdi::KDELook )
	{
		m_pUndockButtonPixmap = new TQPixmap( kde2_undockbutton );
		m_pMinButtonPixmap = new TQPixmap( kde2_minbutton );
		m_pRestoreButtonPixmap = new TQPixmap( kde2_restorebutton );
		m_pCloseButtonPixmap = new TQPixmap( kde2_closebutton );
	}
	else
	{   // kde2laptop look
		m_pUndockButtonPixmap = new TQPixmap( kde2laptop_undockbutton );
		m_pMinButtonPixmap = new TQPixmap( kde2laptop_minbutton );
		m_pRestoreButtonPixmap = new TQPixmap( kde2laptop_restorebutton );
		m_pCloseButtonPixmap = new TQPixmap( kde2laptop_closebutton );
	}

	m_pUndock->hide();
	m_pMinimize->hide();
	m_pRestore->hide();
	m_pClose->hide();

	m_pUndock->setPixmap( *m_pUndockButtonPixmap );
	m_pMinimize->setPixmap( *m_pMinButtonPixmap );
	m_pRestore->setPixmap( *m_pRestoreButtonPixmap );
	m_pClose->setPixmap( *m_pCloseButtonPixmap );
}

void KMdiMainFrm::setSysButtonsAtMenuPosition()
{
	if ( m_pMainMenuBar == 0L )
		return ;
	if ( m_pMainMenuBar->parentWidget() == 0L )
		return ;

	int menuW = m_pMainMenuBar->parentWidget() ->width();
	int h;
	int y;
	if ( frameDecorOfAttachedViews() == KMdi::Win95Look )
		h = 16;
	else if ( frameDecorOfAttachedViews() == KMdi::KDE1Look )
		h = 20;
	else if ( frameDecorOfAttachedViews() == KMdi::KDELook )
		h = 16;
	else
		h = 14;
	y = m_pMainMenuBar->height() / 2 - h / 2;

	if ( frameDecorOfAttachedViews() == KMdi::KDELaptopLook )
	{
		int w = 27;
		m_pUndock->setGeometry( ( menuW - ( w * 3 ) - 5 ), y, w, h );
		m_pMinimize->setGeometry( ( menuW - ( w * 2 ) - 5 ), y, w, h );
		m_pRestore->setGeometry( ( menuW - w - 5 ), y, w, h );
	}
	else
	{
		m_pUndock->setGeometry( ( menuW - ( h * 4 ) - 5 ), y, h, h );
		m_pMinimize->setGeometry( ( menuW - ( h * 3 ) - 5 ), y, h, h );
		m_pRestore->setGeometry( ( menuW - ( h * 2 ) - 5 ), y, h, h );
		m_pClose->setGeometry( ( menuW - h - 5 ), y, h, h );
	}
}

/** Activates the next open view */
void KMdiMainFrm::activateNextWin()
{
	KMdiIterator<KMdiChildView*>* it = createIterator();
	KMdiChildView* aWin = activeWindow();
	for ( it->first(); !it->isDone(); it->next() )
	{
		if ( it->currentItem() == aWin )
		{
			it->next();
			if ( !it->currentItem() )
			{
				it->first();
			}
			if ( it->currentItem() )
			{
				activateView( it->currentItem() );
			}
			break;
		}
	}
	delete it;
}

/** Activates the previous open view */
void KMdiMainFrm::activatePrevWin()
{
	KMdiIterator<KMdiChildView*>* it = createIterator();
	KMdiChildView* aWin = activeWindow();
	for ( it->first(); !it->isDone(); it->next() )
	{
		if ( it->currentItem() == aWin )
		{
			it->prev();
			if ( !it->currentItem() )
			{
				it->last();
			}
			if ( it->currentItem() )
			{
				activateView( it->currentItem() );
			}
			break;
		}
	}
	delete it;
}

/** Activates the view we accessed the most time ago */
void KMdiMainFrm::activateFirstWin()
{
	m_bSwitching= true; // flag that we are currently switching between windows
	KMdiIterator<KMdiChildView*>* it = createIterator();
	TQMap<TQDateTime, KMdiChildView*> m;
	for ( it->first(); !it->isDone(); it->next() )
	{
		m.insert( it->currentItem() ->getTimeStamp(), it->currentItem() );
	}

	if ( !activeWindow() )
		return ;

	TQDateTime current = activeWindow() ->getTimeStamp();
	TQMap<TQDateTime, KMdiChildView*>::iterator pos( m.find( current ) );
	TQMap<TQDateTime, KMdiChildView*>::iterator newPos = pos;
	if ( pos != m.end() )
	{
		++newPos;
	}
	if ( newPos != m.end() )
	{ // look ahead
		++pos;
	}
	else
	{
		pos = m.begin();
	}
	activateView( pos.data() );
	delete it;
}

/** Activates the previously accessed view before this one was activated */
void KMdiMainFrm::activateLastWin()
{
	m_bSwitching= true; // flag that we are currently switching between windows
	KMdiIterator<KMdiChildView*>* it = createIterator();
	TQMap<TQDateTime, KMdiChildView*> m;
	for ( it->first(); !it->isDone(); it->next() )
	{
		m.insert( it->currentItem() ->getTimeStamp(), it->currentItem() );
	}

	if ( !activeWindow() )
		return ;

	TQDateTime current = activeWindow() ->getTimeStamp();
	TQMap<TQDateTime, KMdiChildView*>::iterator pos( m.find( current ) );
	if ( pos != m.begin() )
	{
		--pos;
	}
	else
	{
		pos = m.end();
		--pos;
	}
	activateView( pos.data() );
	delete it;
}

/** Activates the view with a certain index (TabPage mode only) */
void KMdiMainFrm::activateView( int index )
{
	KMdiChildView * pView = m_pDocumentViews->first();
	for ( int i = 0; pView && ( i < index ); i++ )
	{
		pView = m_pDocumentViews->next();
	}
	if ( pView )
	{
		pView->activate();
	}
}

/** turns the system buttons for maximize mode (SDI mode) on, and connects them with the current child frame */
void KMdiMainFrm::setEnableMaximizedChildFrmMode( bool enableMaxChildFrameMode )
{
	if ( enableMaxChildFrameMode )
	{
		kdDebug(760) << k_funcinfo << "Turning on maximized child frame mode" << endl;
		m_bMaximizedChildFrmMode = true;
		
		KMdiChildFrm* pCurrentChild = m_pMdi->topChild();
		
		//If we have no child or there is no menubar, we do nothing
		if ( !pCurrentChild || !m_pMainMenuBar )
			return ;

		TQObject::connect( m_pUndock, TQT_SIGNAL( clicked() ), pCurrentChild, TQT_SLOT( undockPressed() ) );
		TQObject::connect( m_pMinimize, TQT_SIGNAL( clicked() ), pCurrentChild, TQT_SLOT( minimizePressed() ) );
		TQObject::connect( m_pRestore, TQT_SIGNAL( clicked() ), pCurrentChild, TQT_SLOT( maximizePressed() ) );
		m_pMinimize->show();
		m_pUndock->show();
		m_pRestore->show();

		if ( frameDecorOfAttachedViews() == KMdi::KDELaptopLook )
		{
			m_pMainMenuBar->insertItem( TQPixmap( kde2laptop_closebutton_menu ), m_pMdi->topChild(), TQT_SLOT( closePressed() ), 0, -1, 0 );
		}
		else
		{
			m_pMainMenuBar->insertItem( *pCurrentChild->icon(), pCurrentChild->systemMenu(), -1, 0 );
			if ( m_pClose )
			{
				TQObject::connect( m_pClose, TQT_SIGNAL( clicked() ), pCurrentChild, TQT_SLOT( closePressed() ) );
				m_pClose->show();
			}
			else
				kdDebug(760) << k_funcinfo << "no close button. things won't behave correctly" << endl;
		}
	}
	else
	{
		if ( !m_bMaximizedChildFrmMode )
			return ;  // already set, nothing to do

		kdDebug(760) << k_funcinfo << "Turning off maximized child frame mode" << endl;
		m_bMaximizedChildFrmMode = false;

		KMdiChildFrm* pFrmChild = m_pMdi->topChild();
		if ( pFrmChild && pFrmChild->m_pClient && pFrmChild->state() == KMdiChildFrm::Maximized )
		{
			pFrmChild->m_pClient->restore();
			switchOffMaximizeModeForMenu( pFrmChild );
		}
	}
}

/** turns the system buttons for maximize mode (SDI mode) off, and disconnects them */
void KMdiMainFrm::switchOffMaximizeModeForMenu( KMdiChildFrm* oldChild )
{
	// if there is no menubar given, those system buttons aren't possible
	if ( !m_pMainMenuBar )
		return ;
	
	m_pMainMenuBar->removeItem( m_pMainMenuBar->idAt( 0 ) );
	
	if ( oldChild )
	{
		Q_ASSERT( m_pClose );
		TQObject::disconnect( m_pUndock, TQT_SIGNAL( clicked() ), oldChild, TQT_SLOT( undockPressed() ) );
		TQObject::disconnect( m_pMinimize, TQT_SIGNAL( clicked() ), oldChild, TQT_SLOT( minimizePressed() ) );
		TQObject::disconnect( m_pRestore, TQT_SIGNAL( clicked() ), oldChild, TQT_SLOT( maximizePressed() ) );
		TQObject::disconnect( m_pClose, TQT_SIGNAL( clicked() ), oldChild, TQT_SLOT( closePressed() ) );
	}
	m_pUndock->hide();
	m_pMinimize->hide();
	m_pRestore->hide();
	m_pClose->hide();
}

/** reconnects the system buttons form maximize mode (SDI mode) with the new child frame */
void KMdiMainFrm::updateSysButtonConnections( KMdiChildFrm* oldChild, KMdiChildFrm* newChild )
{
	//tqDebug("updateSysButtonConnections");
	// if there is no menubar given, those system buttons aren't possible
	if ( !m_pMainMenuBar )
		return ;

	if ( newChild )
	{
		if ( frameDecorOfAttachedViews() == KMdi::KDELaptopLook )
			m_pMainMenuBar->insertItem( TQPixmap( kde2laptop_closebutton_menu ), newChild, TQT_SLOT( closePressed() ), 0, -1, 0 );
		else
			m_pMainMenuBar->insertItem( *newChild->icon(), newChild->systemMenu(), -1, 0 );
	}
	
	if ( oldChild )
	{
		m_pMainMenuBar->removeItem( m_pMainMenuBar->idAt( 1 ) );
		Q_ASSERT( m_pClose );
		TQObject::disconnect( m_pUndock, TQT_SIGNAL( clicked() ), oldChild, TQT_SLOT( undockPressed() ) );
		TQObject::disconnect( m_pMinimize, TQT_SIGNAL( clicked() ), oldChild, TQT_SLOT( minimizePressed() ) );
		TQObject::disconnect( m_pRestore, TQT_SIGNAL( clicked() ), oldChild, TQT_SLOT( maximizePressed() ) );
		TQObject::disconnect( m_pClose, TQT_SIGNAL( clicked() ), oldChild, TQT_SLOT( closePressed() ) );
	}
	if ( newChild )
	{
		Q_ASSERT( m_pClose );
		TQObject::connect( m_pUndock, TQT_SIGNAL( clicked() ), newChild, TQT_SLOT( undockPressed() ) );
		TQObject::connect( m_pMinimize, TQT_SIGNAL( clicked() ), newChild, TQT_SLOT( minimizePressed() ) );
		TQObject::connect( m_pRestore, TQT_SIGNAL( clicked() ), newChild, TQT_SLOT( maximizePressed() ) );
		TQObject::connect( m_pClose, TQT_SIGNAL( clicked() ), newChild, TQT_SLOT( closePressed() ) );
	}
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
bool KMdiMainFrm::isViewTaskBarOn()
{
	if ( m_pTaskBar )
		return m_pTaskBar->isSwitchedOn();
	else
		return false;
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
void KMdiMainFrm::showViewTaskBar()
{
	if ( m_pTaskBar )
		m_pTaskBar->switchOn( true );
}

/** Hides the view taskbar. This should be connected with your "View" menu. */
void KMdiMainFrm::hideViewTaskBar()
{
	if ( m_pTaskBar )
		m_pTaskBar->switchOn( false );
}

//=============== fillWindowMenu ===============//
void KMdiMainFrm::fillWindowMenu()
{
	bool tabPageMode = false;
	if ( m_mdiMode == KMdi::TabPageMode )
		tabPageMode = true;
	
	bool IDEAlMode = false;
	if ( m_mdiMode == KMdi::IDEAlMode )
		IDEAlMode = true;

	bool noViewOpened = false;
	if ( m_pDocumentViews->isEmpty() )
		noViewOpened = true;

	// construct the menu and its submenus
	if ( !m_bClearingOfWindowMenuBlocked )
		m_pWindowMenu->clear();

	d->closeWindowAction->plug(m_pWindowMenu);

	int closeAllId = m_pWindowMenu->insertItem( i18n( "Close &All" ), this, TQT_SLOT( closeAllViews() ) );
	if ( noViewOpened )
	{
		d->closeWindowAction->setEnabled(false);
		m_pWindowMenu->setItemEnabled( closeAllId, false );
	}
	
	if ( !tabPageMode && !IDEAlMode )
	{
		int iconifyId = m_pWindowMenu->insertItem( i18n( "&Minimize All" ), this, TQT_SLOT( iconifyAllViews() ) );
		if ( noViewOpened )
			m_pWindowMenu->setItemEnabled( iconifyId, false );
	}
	
	m_pWindowMenu->insertSeparator();
	m_pWindowMenu->insertItem( i18n( "&MDI Mode" ), m_pMdiModeMenu );
	m_pMdiModeMenu->clear();
	m_pMdiModeMenu->insertItem( i18n( "&Toplevel Mode" ), this, TQT_SLOT( switchToToplevelMode() ) );
	m_pMdiModeMenu->insertItem( i18n( "C&hildframe Mode" ), this, TQT_SLOT( switchToChildframeMode() ) );
	m_pMdiModeMenu->insertItem( i18n( "Ta&b Page Mode" ), this, TQT_SLOT( switchToTabPageMode() ) );
	m_pMdiModeMenu->insertItem( i18n( "I&DEAl Mode" ), this, TQT_SLOT( switchToIDEAlMode() ) );
	switch ( m_mdiMode )
	{
	case KMdi::ToplevelMode:
		m_pMdiModeMenu->setItemChecked( m_pMdiModeMenu->idAt( 0 ), true );
		break;
	case KMdi::ChildframeMode:
		m_pMdiModeMenu->setItemChecked( m_pMdiModeMenu->idAt( 1 ), true );
		break;
	case KMdi::TabPageMode:
		m_pMdiModeMenu->setItemChecked( m_pMdiModeMenu->idAt( 2 ), true );
		break;
	case KMdi::IDEAlMode:
		m_pMdiModeMenu->setItemChecked( m_pMdiModeMenu->idAt( 3 ), true );
		break;
	default:
		break;
	}
	
	m_pWindowMenu->insertSeparator();
	if ( !tabPageMode && !IDEAlMode )
	{
		int placMenuId = m_pWindowMenu->insertItem( i18n( "&Tile" ), m_pPlacingMenu );
		m_pPlacingMenu->clear();
		m_pPlacingMenu->insertItem( i18n( "Ca&scade Windows" ), m_pMdi, TQT_SLOT( cascadeWindows() ) );
		m_pPlacingMenu->insertItem( i18n( "Cascade &Maximized" ), m_pMdi, TQT_SLOT( cascadeMaximized() ) );
		m_pPlacingMenu->insertItem( i18n( "Expand &Vertically" ), m_pMdi, TQT_SLOT( expandVertical() ) );
		m_pPlacingMenu->insertItem( i18n( "Expand &Horizontally" ), m_pMdi, TQT_SLOT( expandHorizontal() ) );
		m_pPlacingMenu->insertItem( i18n( "Tile &Non-Overlapped" ), m_pMdi, TQT_SLOT( tileAnodine() ) );
		m_pPlacingMenu->insertItem( i18n( "Tile Overla&pped" ), m_pMdi, TQT_SLOT( tilePragma() ) );
		m_pPlacingMenu->insertItem( i18n( "Tile V&ertically" ), m_pMdi, TQT_SLOT( tileVertically() ) );
		if ( m_mdiMode == KMdi::ToplevelMode )
		{
			m_pWindowMenu->setItemEnabled( placMenuId, false );
		}
		m_pWindowMenu->insertSeparator();
		int dockUndockId = m_pWindowMenu->insertItem( i18n( "&Dock/Undock" ), m_pDockMenu );
		m_pDockMenu->clear();
		m_pWindowMenu->insertSeparator();
		if ( noViewOpened )
		{
			m_pWindowMenu->setItemEnabled( placMenuId, false );
			m_pWindowMenu->setItemEnabled( dockUndockId, false );
		}
	}
	int entryCount = m_pWindowMenu->count();

	// for all child frame windows: give an ID to every window and connect them in the end with windowMenuItemActivated()
	int i = 100;
	KMdiChildView* pView = 0L;
	TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
	TQValueList<TQDateTime> timeStamps;
	for ( ; it.current(); ++it )
	{
		pView = it.current();
		TQDateTime timeStamp( pView->getTimeStamp() );

		if ( pView->isToolView() )
		{
			continue;
		}

		TQString item;
		// set titles of minimized windows in brackets
		if ( pView->isMinimized() )
		{
			item += "(";
			item += pView->caption();
			item += ")";
		}
		else
		{
			item += " ";
			item += pView->caption();
		}

		// insert the window entry sorted by access time
		unsigned int indx;
		unsigned int windowItemCount = m_pWindowMenu->count() - entryCount;
		bool inserted = false;
		TQString tmpString;
		TQValueList<TQDateTime>::iterator timeStampIterator = timeStamps.begin();
		for ( indx = 0; indx <= windowItemCount; indx++, ++timeStampIterator )
		{
			bool putHere = false;
			if ( ( *timeStampIterator ) < timeStamp )
			{
				putHere = true;
				timeStamps.insert( timeStampIterator, timeStamp );
			}
			if ( putHere )
			{
				m_pWindowMenu->insertItem( item, pView, TQT_SLOT( slot_clickedInWindowMenu() ), 0, -1, indx + entryCount );
				if ( pView == m_pCurrentWindow )
				{
					m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx + entryCount ), true );
				}
				pView->setWindowMenuID( i );
				if ( !tabPageMode )
				{
					m_pDockMenu->insertItem( item, pView, TQT_SLOT( slot_clickedInDockMenu() ), 0, -1, indx );
					if ( pView->isAttached() )
					{
						m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx ), true );
					}
				}
				inserted = true;
				break;
				indx = windowItemCount + 1;  // break the loop
			}
		}
		if ( !inserted )
		{  // append it
			m_pWindowMenu->insertItem( item, pView, TQT_SLOT( slot_clickedInWindowMenu() ), 0, -1, windowItemCount + entryCount );
			if ( pView == m_pCurrentWindow )
			{
				m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( windowItemCount + entryCount ), true );
			}
			pView->setWindowMenuID( i );
			if ( !tabPageMode )
			{
				m_pDockMenu->insertItem( item, pView, TQT_SLOT( slot_clickedInDockMenu() ), 0, -1, windowItemCount );
				if ( pView->isAttached() )
				{
					m_pDockMenu->setItemChecked( m_pDockMenu->idAt( windowItemCount ), true );
				}
			}
		}
		i++;
	}
}

//================ windowMenuItemActivated ===============//

void KMdiMainFrm::windowMenuItemActivated( int id )
{
	if ( id < 100 )
		return ;
	id -= 100;
	KMdiChildView *pView = m_pDocumentViews->at( id );
	if ( !pView )
		return ;
	if ( pView->isMinimized() )
		pView->minimize();
	if ( m_mdiMode != KMdi::TabPageMode )
	{
		KMdiChildFrm * pTopChild = m_pMdi->topChild();
		if ( pTopChild )
		{
			if ( ( pView == pTopChild->m_pClient ) && pView->isAttached() )
			{
				return ;
			}
		}
	}
	activateView( pView );
}

//================ dockMenuItemActivated ===============//

void KMdiMainFrm::dockMenuItemActivated( int id )
{
	if ( id < 100 )
		return ;
	id -= 100;
	KMdiChildView *pView = m_pDocumentViews->at( id );
	if ( !pView )
		return ;
	if ( pView->isMinimized() )
		pView->minimize();
	if ( pView->isAttached() )
	{
		detachWindow( pView, true );
	}
	else
	{   // is detached
		attachWindow( pView, true );
	}
}

//================ popupWindowMenu ===============//

void KMdiMainFrm::popupWindowMenu( TQPoint p )
{
	if ( !isFakingSDIApplication() )
	{
		m_pWindowMenu->popup( p );
	}
}

//================ dragEndTimeOut ===============//
void KMdiMainFrm::dragEndTimeOut()
{
	// send drag end to all concerned views.
	KMdiChildView * pView;
	for ( m_pDocumentViews->first(); ( pView = m_pDocumentViews->current() ) != 0L; m_pDocumentViews->next() )
	{
		KMdiChildFrmDragEndEvent dragEndEvent( 0L );
		TQApplication::sendEvent( pView, &dragEndEvent );
	}
}

//================ setFrameDecorOfAttachedViews ===============//

void KMdiMainFrm::setFrameDecorOfAttachedViews( int frameDecor )
{
	switch ( frameDecor )
	{
	case 0:
		m_frameDecoration = KMdi::Win95Look;
		break;
	case 1:
		m_frameDecoration = KMdi::KDE1Look;
		break;
	case 2:
		m_frameDecoration = KMdi::KDELook;
		break;
	case 3:
		m_frameDecoration = KMdi::KDELaptopLook;
		break;
	default:
		tqDebug( "unknown MDI decoration" );
		break;
	}
	setMenuForSDIModeSysButtons( m_pMainMenuBar );
	TQPtrListIterator<KMdiChildView> it( *m_pDocumentViews );
	for ( ; it.current(); ++it )
	{
		KMdiChildView* pView = it.current();
		if ( pView->isToolView() )
			continue;
		if ( pView->isAttached() )
			pView->mdiParent() ->redecorateButtons();
	}
}

void KMdiMainFrm::fakeSDIApplication()
{
	m_bSDIApplication = true;
	if ( m_pTaskBar )
		m_pTaskBar->close();
	m_pTaskBar = 0L;
}

void KMdiMainFrm::closeViewButtonPressed()
{
	KMdiChildView * pView = activeWindow();
	if ( pView )
	{
		pView->close();
	}
}

void KMdiMainFrm::setManagedDockPositionModeEnabled( bool enabled )
{
	m_managedDockPositionMode = enabled;
}

void KMdiMainFrm::setActiveToolDock( KMdiDockContainer* td )
{
	if ( td == d->activeDockPriority[ 0 ] )
		return ;
	if ( d->activeDockPriority[ 0 ] == 0 )
	{
		d->activeDockPriority[ 0 ] = td;
		//        d->focusList=new KMdiFocusList(this);
		//        if (m_pMdi)  d->focusList->addWidgetTree(m_pMdi);
		//        if (m_documentTabWidget) d->focusList->addWidgetTree(m_documentTabWidget);
		return ;
	}
	for ( int dst = 3, src = 2;src >= 0;dst--, src-- )
	{
		if ( d->activeDockPriority[ src ] == td )
			src--;
		if ( src < 0 )
			break;
		d->activeDockPriority[ dst ] = d->activeDockPriority[ src ];
	}
	d->activeDockPriority[ 0 ] = td;
}

void KMdiMainFrm::removeFromActiveDockList( KMdiDockContainer* td )
{
	for ( int i = 0;i < 4;i++ )
	{
		if ( d->activeDockPriority[ i ] == td )
		{
			for ( ;i < 3;i++ )
				d->activeDockPriority[ i ] = d->activeDockPriority[ i + 1 ];
			d->activeDockPriority[ 3 ] = 0;
			break;
		}
	}
	/*
	      if (d->activeDockPriority[0]==0) {
	      if (d->focusList) d->focusList->restore();
	      delete d->focusList;
	      d->focusList=0;
	      }
	 */
}

void KMdiMainFrm::prevToolViewInDock()
{
	KMdiDockContainer * td = d->activeDockPriority[ 0 ];
	if ( !td )
		return ;
	td->prevToolView();
}

void KMdiMainFrm::nextToolViewInDock()
{
	KMdiDockContainer * td = d->activeDockPriority[ 0 ];
	if ( !td )
		return ;
	td->nextToolView();
}

KMdi::TabWidgetVisibility KMdiMainFrm::tabWidgetVisibility()
{
	if ( m_documentTabWidget )
		return m_documentTabWidget->tabWidgetVisibility();

	return KMdi::NeverShowTabs;
}

void KMdiMainFrm::setTabWidgetVisibility( KMdi::TabWidgetVisibility visibility )
{
	if ( m_documentTabWidget )
		m_documentTabWidget->setTabWidgetVisibility( visibility );
}

KTabWidget * KMdiMainFrm::tabWidget() const
{
	return m_documentTabWidget;
}

#include "tdemdimainfrm.moc"

// vim: ts=2 sw=2 et
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
