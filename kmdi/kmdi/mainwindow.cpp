/* This file is part of the KDE libraries
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

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

   Based on:

   //----------------------------------------------------------------------------
   //    Project              : KDE MDI extension
   //
   //    begin                : 07/1999       by Szymon Stefanek as part of kvirc
   //                                         (an IRC application)
   //    changes              : 09/1999       by Falk Brettschneider to create an
   //                           - 06/2000     stand-alone Qt extension set of
   //                                         classes and a Qt-based library
   //                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
   //                           2000-2003     maintained by the KDevelop project
   //    patches              : -/2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
   //                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
   //
   //    copyright            : (C) 1999-2003 by Falk Brettschneider
   //                                         and
   //                                         Szymon Stefanek (stefanek@tin.it)
   //    email                :  falkbr@kdevelop.org (Falk Brettschneider)
   //----------------------------------------------------------------------------
*/

#include "config.h"

#include "tabwidget.h"

#include <assert.h>

#include <tqcursor.h>
#include <tqclipboard.h>
#include <tqobjectlist.h>
#include <tqpopupmenu.h>
#include <tqmenubar.h>

#include <kmenubar.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <tqtabwidget.h>
#include <klocale.h>

#include <kiconloader.h>
#include "dockcontainer.h"
#include "mainwindow.moc"


#include <tqtoolbutton.h>
#include <tqlayout.h>
#include <tqstring.h>
#include <tqmap.h>
#include <tqvaluelist.h>

#include "mainwindow.h"
#include "dockcontainer.h"
#include "toolviewaccessor_p.h"

#include "guiclient.h"

namespace KMDIPrivate
{
  class MainWindowPrivate {
    public:
      MainWindowPrivate() {
        for (int i=0;i<4;i++) {
          activeDockPriority[i]=0;
          m_styleIDEAlMode = 0;
          m_toolviewStyle = 0;
        }
      }
      ~MainWindowPrivate() {}
      KMDI::DockContainer* activeDockPriority[4];
      int m_styleIDEAlMode;
      int m_toolviewStyle;
  };
}

namespace KMDI
{

//============ constructor ============//
MainWindow::MainWindow(TQWidget* tqparentWidget, const char* name)
 : KParts::DockMainWindow( tqparentWidget, name)
 , m_toolViews (new TQMap<TQWidget*,KMDI::ToolViewAccessor*>)
 , d(new KMDIPrivate::MainWindowPrivate())
{
  // setup main dock stuff
  setupMainDock ();

  // setup the sidebar framework
  setupToolViews ();

  // setup all menu entries
  setupGUIClient ();
}

//============ ~MainWindow ============//
MainWindow::~MainWindow()
{
  delete m_toolViews;
  m_toolViews=0;

  delete d;
  d=0;
}

void MainWindow::setupMainDock ()
{
  // cover KMdi's childarea by a dockwidget
  KDockWidget *dw = createDockWidget( "mdiAreaCover", TQPixmap(), 0L, "mdi_area_cover");
  dw->setDockWindowTransient(this,true);
  dw->setEnableDocking(KDockWidget::DockNone);
  dw->setDockSite(KDockWidget::DockCorner);

  // setup tab widget
  m_tabWidget = new KMDI::TabWidget (dw);
  dw->setWidget(m_tabWidget);

  // set this dock to main view
  setView(dw);
  setMainDockWidget(dw);

  // connect signals to collapse sidebars
  connect(m_tabWidget,TQT_SIGNAL(focusInEvent()),this,TQT_SIGNAL(collapseOverlapContainers()));
}

void MainWindow::setupToolViews ()
{
  m_leftContainer = createDockWidget("KMdiDock::leftDock",SmallIcon("misc"),0L,"Left Dock");
  m_rightContainer = createDockWidget("KMdiDock::rightDock",SmallIcon("misc"),0L,"Right Dock");
  m_topContainer = createDockWidget("KMdiDock::topDock",SmallIcon("misc"),0L,"Top Dock");
  m_bottomContainer = createDockWidget("KMdiDock::bottomDock",SmallIcon("misc"),0L,"Bottom Dock");

  KDockWidget *mainDock=getMainDockWidget();
  KDockWidget *w=mainDock;

  if (mainDock->parentDockTabGroup()) {
    w=static_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
  }

  TQPtrList<KDockWidget> leftRetqparentWidgets;
  TQPtrList<KDockWidget> rightRetqparentWidgets;
  TQPtrList<KDockWidget> bottomRetqparentWidgets;
  TQPtrList<KDockWidget> topRetqparentWidgets;

  if (mainDock->parentDockTabGroup()) {
    mainDock=static_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
  }

  findToolViewsDockedToMain(&leftRetqparentWidgets,KDockWidget::DockLeft);
  findToolViewsDockedToMain(&rightRetqparentWidgets,KDockWidget::DockRight);
  findToolViewsDockedToMain(&bottomRetqparentWidgets,KDockWidget::DockBottom);
  findToolViewsDockedToMain(&topRetqparentWidgets,KDockWidget::DockTop);

  mainDock->setEnableDocking(KDockWidget::DockNone); //::DockCorner);
  mainDock->setDockSite(KDockWidget::DockCorner);


  DockContainer *tmpDC;
  m_leftContainer->setWidget(tmpDC=new DockContainer(m_leftContainer, this, KDockWidget::DockLeft, d->m_styleIDEAlMode));
  m_leftContainer->setEnableDocking(KDockWidget::DockLeft);
  m_leftContainer->manualDock(mainDock, KDockWidget::DockLeft,20);
  tmpDC->init();

  connect (this,TQT_SIGNAL(toggleLeft()),tmpDC,TQT_SLOT(toggle()));
  connect(this,TQT_SIGNAL(collapseOverlapContainers()),tmpDC,TQT_SLOT(collapseOverlapped()));
  connect(tmpDC,TQT_SIGNAL(activated(DockContainer*)),this,TQT_SLOT(setActiveToolDock(DockContainer*)));
  connect(tmpDC,TQT_SIGNAL(deactivated(DockContainer*)),this,TQT_SLOT(removeFromActiveDockList(DockContainer*)));

  m_rightContainer->setWidget(tmpDC=new DockContainer(m_rightContainer, this, KDockWidget::DockRight, d->m_styleIDEAlMode));
  m_rightContainer->setEnableDocking(KDockWidget::DockRight);
  m_rightContainer->manualDock(mainDock, KDockWidget::DockRight,80);
  tmpDC->init();

  connect (this,TQT_SIGNAL(toggleRight()),tmpDC,TQT_SLOT(toggle()));
  connect(this,TQT_SIGNAL(collapseOverlapContainers()),tmpDC,TQT_SLOT(collapseOverlapped()));
  connect(tmpDC,TQT_SIGNAL(activated(DockContainer*)),this,TQT_SLOT(setActiveToolDock(DockContainer*)));
  connect(tmpDC,TQT_SIGNAL(deactivated(DockContainer*)),this,TQT_SLOT(removeFromActiveDockList(DockContainer*)));

  m_topContainer->setWidget(tmpDC=new DockContainer(m_topContainer, this, KDockWidget::DockTop, d->m_styleIDEAlMode));
  m_topContainer->setEnableDocking(KDockWidget::DockTop);
  m_topContainer->manualDock(mainDock, KDockWidget::DockTop,20);
  tmpDC->init();

  connect (this,TQT_SIGNAL(toggleTop()),tmpDC,TQT_SLOT(toggle()));
  connect(this,TQT_SIGNAL(collapseOverlapContainers()),tmpDC,TQT_SLOT(collapseOverlapped()));
  connect(tmpDC,TQT_SIGNAL(activated(DockContainer*)),this,TQT_SLOT(setActiveToolDock(DockContainer*)));
  connect(tmpDC,TQT_SIGNAL(deactivated(DockContainer*)),this,TQT_SLOT(removeFromActiveDockList(DockContainer*)));

  m_bottomContainer->setWidget(tmpDC=new DockContainer(m_bottomContainer, this, KDockWidget::DockBottom, d->m_styleIDEAlMode));
  m_bottomContainer->setEnableDocking(KDockWidget::DockBottom);
  m_bottomContainer->manualDock(mainDock, KDockWidget::DockBottom,80);
  tmpDC->init();

  connect (this,TQT_SIGNAL(toggleBottom()),tmpDC,TQT_SLOT(toggle()));
  connect(this,TQT_SIGNAL(collapseOverlapContainers()),tmpDC,TQT_SLOT(collapseOverlapped()));
  connect(tmpDC,TQT_SIGNAL(activated(DockContainer*)),this,TQT_SLOT(setActiveToolDock(DockContainer*)));
  connect(tmpDC,TQT_SIGNAL(deactivated(DockContainer*)),this,TQT_SLOT(removeFromActiveDockList(DockContainer*)));

  m_leftContainer->setDockSite( KDockWidget::DockCenter );
  m_rightContainer->setDockSite( KDockWidget::DockCenter );
  m_topContainer->setDockSite( KDockWidget::DockCenter );
  m_bottomContainer->setDockSite( KDockWidget::DockCenter );

  dockToolViewsIntoContainers(leftRetqparentWidgets,m_leftContainer);
  dockToolViewsIntoContainers(rightRetqparentWidgets,m_rightContainer);
  dockToolViewsIntoContainers(bottomRetqparentWidgets,m_bottomContainer);
  dockToolViewsIntoContainers(topRetqparentWidgets,m_topContainer);

  dockManager->setSpecialLeftDockContainer(m_leftContainer);
  dockManager->setSpecialRightDockContainer(m_rightContainer);
  dockManager->setSpecialTopDockContainer(m_topContainer);
  dockManager->setSpecialBottomDockContainer(m_bottomContainer);

  ((DockContainer*) (m_leftContainer->getWidget()))->hideIfNeeded();
  ((DockContainer*) (m_rightContainer->getWidget()))->hideIfNeeded();
  ((DockContainer*) (m_topContainer->getWidget()))->hideIfNeeded();
  ((DockContainer*) (m_bottomContainer->getWidget()))->hideIfNeeded();
}

void MainWindow::setupGUIClient ()
{
  m_guiClient = new KMDIPrivate::GUIClient (this);

  connect(m_guiClient,TQT_SIGNAL(toggleTop()),this,TQT_SIGNAL(toggleTop()));
  connect(m_guiClient,TQT_SIGNAL(toggleLeft()),this,TQT_SIGNAL(toggleLeft()));
  connect(m_guiClient,TQT_SIGNAL(toggleRight()),this,TQT_SIGNAL(toggleRight()));
  connect(m_guiClient,TQT_SIGNAL(toggleBottom()),this,TQT_SIGNAL(toggleBottom()));
}

TabWidget *MainWindow::tabWidget ()
{
  return m_tabWidget;
}

ToolViewAccessor *MainWindow::createToolWindow()
{
  return new KMDI::ToolViewAccessor(this);
}

KMDI::ToolViewAccessor *MainWindow::addToolWindow( TQWidget* pWnd, KDockWidget::DockPosition pos, TQWidget* pTargetWnd, int percent, const TQString& tabToolTip, const TQString& tabCaption)
{
  TQWidget *tvta=pWnd;
  KDockWidget* pDW = dockManager->getDockWidgetFromName(pWnd->name());
  if (pDW) {
    // probably readDockConfig already created the widgetContainer, use that
    pDW->setWidget(pWnd);

    if (pWnd->icon()) {
      pDW->setPixmap(*pWnd->icon());
    }
    pDW->setTabPageLabel((tabCaption==0)?pWnd->caption():tabCaption);
    pDW->setToolTipString(tabToolTip);
    dockManager->removeFromAutoCreateList(pDW);
    pWnd=pDW;
  }

  TQRect r=pWnd->geometry();

  KMDI::ToolViewAccessor *mtva=new KMDI::ToolViewAccessor(this,pWnd,tabToolTip,(tabCaption==0)?pWnd->caption():tabCaption);
  m_toolViews->insert(tvta,mtva);

  if (pos == KDockWidget::DockNone) {
    mtva->d->widgetContainer->setEnableDocking(KDockWidget::DockNone);
    mtva->d->widgetContainer->reparent(this, (WFlags)(WType_TopLevel | WType_Dialog), r.topLeft(), isVisible());
  } else {   // add (and dock) the toolview as DockWidget view
    //const TQPixmap& wndIcon = pWnd->icon() ? *(pWnd->icon()) : TQPixmap();

    //KDockWidget *pCover=mtva->d->widgetContainer;

    mtva->place(pos, pTargetWnd,percent);
  }

  return mtva;
}

void MainWindow::deleteToolWindow( KMDI::ToolViewAccessor *accessor)
{
  delete accessor;
}

void MainWindow::deleteToolWindow( TQWidget* pWnd)
{
  if (!pWnd)
    return;

  if (m_toolViews->tqcontains(pWnd)) {
    deleteToolWindow((*m_toolViews)[pWnd]);
  }
}

void MainWindow::setToolViewStyle(int flag)
{
  d->m_styleIDEAlMode = flag; // see KMultiTabBar for the first 3 bits

  DockContainer *tmpL=(DockContainer*) (m_leftContainer->getWidget()->tqqt_cast("KMDI::DockContainer"));
  if (tmpL) tmpL->setStyle(flag);

  DockContainer *tmpR=(DockContainer*) (m_rightContainer->getWidget()->tqqt_cast("KMDI::DockContainer"));
  if (tmpR) tmpR->setStyle(flag);

  DockContainer *tmpT=(DockContainer*) (m_topContainer->getWidget()->tqqt_cast("KMDI::DockContainer"));
  if (tmpT) tmpT->setStyle(flag);

  DockContainer *tmpB=(DockContainer*) (m_bottomContainer->getWidget()->tqqt_cast("KMDI::DockContainer"));
  if (tmpB) tmpB->setStyle(flag);

  d->m_toolviewStyle = flag;
  bool toolviewExists = false;
  TQMap<TQWidget*,KMDI::ToolViewAccessor*>::Iterator it;
  for (it = m_toolViews->begin(); it != m_toolViews->end(); ++it) {
    KDockWidget *dockWidget = dynamic_cast<KDockWidget*>(it.data()->wrapperWidget());
    if (dockWidget) {
      if (flag == KMDI::IconOnly)
      {
        dockWidget->setTabPageLabel(" ");
        dockWidget->setPixmap(*(it.data()->wrappedWidget()->icon()));
      } else
      if (flag == KMDI::TextOnly)
      {
        dockWidget->setPixmap(); //FIXME: Does not hide the icon in the IDEAl mode.
        dockWidget->setTabPageLabel(it.data()->wrappedWidget()->caption());
      } else
      if (flag == KMDI::TextAndIcon)
      {
        dockWidget->setPixmap(*(it.data()->wrappedWidget()->icon()));
        dockWidget->setTabPageLabel(it.data()->wrappedWidget()->caption());
      }
      toolviewExists = true;
    }
  }
  if (toolviewExists)
  {
    //workaround for the above FIXME to make switching to TextOnly mode work in IDEAl as well. Be sure that this version of switch* is called.
    if (flag == KMDI::TextOnly)
    {
   /*   MainWindow::switchToTabPageMode();
      MainWindow::switchToIDEAlMode();*/ //TODO
    } else
    {
      writeDockConfig();
      readDockConfig();
    }
  }
}

void MainWindow::dockToolViewsIntoContainers(TQPtrList<KDockWidget>& widgetsToReparent,KDockWidget *container) {
  for ( KDockWidget *dw = widgetsToReparent.first(); dw;
      dw=widgetsToReparent.next()){
    dw->manualDock(container,KDockWidget::DockCenter,20);
    dw->loseFormerBrotherDockWidget();
  }
}

void MainWindow::findToolViewsDockedToMain(TQPtrList<KDockWidget>* list,KDockWidget::DockPosition dprtmw) {
  KDockWidget *mainDock=getMainDockWidget();
  if (mainDock->parentDockTabGroup()) {
    mainDock=dynamic_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
    // FIXME: will likely crash below due to unchecked cast
  }

  KDockWidget* widget=mainDock->findNearestDockWidget(dprtmw);
  if (widget) {
    if (widget->parentDockTabGroup()) {
      widget=static_cast<KDockWidget*>(widget->parentDockTabGroup()->parent());
    }

    if (widget) {
      KDockTabGroup *tg=dynamic_cast<KDockTabGroup*>(widget->
          getWidget());
      if (tg) {
        kdDebug(760)<<"KDockTabGroup found"<<endl;
        for (int i=0;i<tg->count();i++)
          list->append((KDockWidget*)static_cast<KDockWidget*>(
                tg->page(i)));
      } else
        list->append((KDockWidget*)widget);
    } else
      kdDebug(760)<<"setupToolViewsForIDEALMode: no  widget found"<<endl;
  } else
    kdDebug(760)<<"No main dock widget found"<<endl;
}

void MainWindow::setActiveToolDock(DockContainer* td) {
  if (td==d->activeDockPriority[0]) return;
  if (d->activeDockPriority[0]==0) {
    d->activeDockPriority[0]=td;
    //        d->focusList=new KMdiFocusList(this);
    //        if (m_pMdi)  d->focusList->addWidgetTree(m_pMdi);
    //        if (m_documentTabWidget) d->focusList->addWidgetTree(m_documentTabWidget);
    return;
  }
  for (int dst=3,src=2;src>=0;dst--,src--) {
    if (d->activeDockPriority[src]==td) src--;
    if (src<0) break;
    d->activeDockPriority[dst]=d->activeDockPriority[src];
  }
  d->activeDockPriority[0]=td;
}

void MainWindow::removeFromActiveDockList(DockContainer* td) {
  for (int i=0;i<4;i++) {
    if (d->activeDockPriority[i]==td) {
      for (;i<3;i++)
        d->activeDockPriority[i]=d->activeDockPriority[i+1];
      d->activeDockPriority[3]=0;
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

void MainWindow::prevToolViewInDock() {
  DockContainer* td=d->activeDockPriority[0];
  if (!td) return;
  td->prevToolView();
}

void MainWindow::nextToolViewInDock() {
  DockContainer* td=d->activeDockPriority[0];
  if (!td) return;
  td->nextToolView();
}

}

// kate: space-indent on; indent-width 2; replace-tabs on;
