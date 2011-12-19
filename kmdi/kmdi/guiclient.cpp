/* This file is part of the KDE libraries
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
   based on ktoolbarhandler.cpp: Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

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

#include "guiclient.h"
#include "guiclient.moc"

#include <tqpopupmenu.h>
#include <kapplication.h>
#include <kconfig.h>
#include <ktoolbar.h>
#include <klocale.h>
#include <kaction.h>
#include <tqstring.h>
#include <kdebug.h>
#include <kdockwidget.h>

#include "mainwindow.h"
#include "toolviewaccessor.h"
#include "toolviewaccessor_p.h"

static const char *actionListName = "show_kmdi_document_tool_view_actions";

static const char *guiDescription = ""
        "<!DOCTYPE kpartgui><kpartgui name=\"KMDIViewActions\">"
        "<MenuBar>"
        "    <Menu name=\"window\">"
        "        <ActionList name=\"%1\" />"
        "    </Menu>"
        "</MenuBar>"
        "</kpartgui>";

static const char *resourceFileName = "kmdiviewactions.rc";

namespace KMDIPrivate
{

GUIClient::GUIClient (KMDI::MainWindow* mdiMainFrm,const char* name)
 : TQObject ( mdiMainFrm,name )
 , KXMLGUIClient ( mdiMainFrm )
{
  m_mdiMainFrm=mdiMainFrm;

  connect( mdiMainFrm->guiFactory(), TQT_SIGNAL( clientAdded( KXMLGUIClient * ) ),
           this, TQT_SLOT( clientAdded( KXMLGUIClient * ) ) );

  /* re-use an existing resource file if it exists. can happen if the user launches the
   * toolbar editor */
  /*
  setXMLFile( resourceFileName );
  */

  if ( domDocument().documentElement().isNull() )
  {
    TQString completeDescription = TQString::fromLatin1( guiDescription ).arg( actionListName );

    setXML( completeDescription, false /*merge*/ );
  }

  if (actionCollection()->kaccel()==0)
    actionCollection()->setWidget(mdiMainFrm);

  m_toolMenu=new KActionMenu(i18n("Tool &Views"),actionCollection(),"kmdi_toolview_menu");

  m_gotoToolDockMenu=new KActionMenu(i18n("Tool &Docks"),actionCollection(),"kmdi_tooldock_menu");
  m_gotoToolDockMenu->insert(new KAction(i18n("Switch Top Dock"),ALT+CTRL+SHIFT+Key_T,this,TQT_SIGNAL(toggleTop()),
  actionCollection(),"kmdi_activate_top"));
  m_gotoToolDockMenu->insert(new KAction(i18n("Switch Left Dock"),ALT+CTRL+SHIFT+Key_L,this,TQT_SIGNAL(toggleLeft()),
  actionCollection(),"kmdi_activate_left"));
  m_gotoToolDockMenu->insert(new KAction(i18n("Switch Right Dock"),ALT+CTRL+SHIFT+Key_R,this,TQT_SIGNAL(toggleRight()),
  actionCollection(),"kmdi_activate_right"));
  m_gotoToolDockMenu->insert(new KAction(i18n("Switch Bottom Dock"),ALT+CTRL+SHIFT+Key_B,this,TQT_SIGNAL(toggleBottom()),
  actionCollection(),"kmdi_activate_bottom"));
  m_gotoToolDockMenu->insert(new KActionSeparator(actionCollection(),"kmdi_goto_menu_separator"));
  m_gotoToolDockMenu->insert(new KAction(i18n("Previous Tool View"),ALT+CTRL+Key_Left,TQT_TQOBJECT(m_mdiMainFrm),TQT_SLOT(prevToolViewInDock()),
  actionCollection(),"kmdi_prev_toolview"));
  m_gotoToolDockMenu->insert(new KAction(i18n("Next Tool View"),ALT+CTRL+Key_Right,TQT_TQOBJECT(m_mdiMainFrm),TQT_SLOT(nextToolViewInDock()),
  actionCollection(),"kmdi_next_toolview"));

  actionCollection()->readShortcutSettings( "Shortcuts", kapp->config() );
}

GUIClient::~GUIClient()
{
//     actionCollection()->writeShortcutSettings( "KMDI Shortcuts", kapp->config() );
  for (uint i=0;i<m_toolViewActions.count();i++)
    disconnect(m_toolViewActions.tqat(i),0,this,0);

  m_toolViewActions.setAutoDelete( false );
  m_toolViewActions.clear();
  m_documentViewActions.setAutoDelete( false );
  m_documentViewActions.clear();
}

void GUIClient::setupActions()
{
  if ( !factory() || !m_mdiMainFrm )
    return;

  unplugActionList( actionListName );

  TQPtrList<KAction> addList;
  if (m_toolViewActions.count()<3)
  {
    for (uint i=0;i<m_toolViewActions.count();i++)
      addList.append(m_toolViewActions.tqat(i));
  }
  else
    addList.append(m_toolMenu);

  addList.append(m_gotoToolDockMenu);

  kdDebug(760)<<"GUIClient::setupActions: plugActionList"<<endl;

  plugActionList( actionListName, addList );
}

void GUIClient::addToolView(KMDI::ToolViewAccessor* mtva)
{
  kdDebug(760)<<"*****void GUIClient::addToolView(KMDI::ToolViewAccessor* mtva)*****"<<endl;

  TQString aname = TQString("kmdi_toolview_") + mtva->wrappedWidget()->name();

  // try to read the action shortcut
  KShortcut sc;
  KConfig *cfg = kapp->config();
  TQString _grp = cfg->group();
  cfg->setGroup("Shortcuts");
  sc = KShortcut( cfg->readEntry( aname, "" ) );
  cfg->setGroup( _grp );

  KAction *a=new ToggleToolViewAction(i18n("Show %1").arg(mtva->wrappedWidget()->caption()),
    /*TQString::null*/sc,tqt_dynamic_cast<KDockWidget*>(mtva->wrapperWidget()),
    m_mdiMainFrm,actionCollection(), aname.latin1() );

  ((ToggleToolViewAction*)a)->setCheckedState(TQString(i18n("Hide %1").arg(mtva->wrappedWidget()->caption())));

  connect(a,TQT_SIGNAL(destroyed(TQObject*)),this,TQT_SLOT(actionDeleted(TQObject*)));

  m_toolViewActions.append(a);
  m_toolMenu->insert(a);
  mtva->d->action=a;

  setupActions();
}

void GUIClient::actionDeleted(TQObject* a)
{
  m_toolViewActions.remove(static_cast<KAction*>(a));
  setupActions();
}


void GUIClient::clientAdded( KXMLGUIClient *client )
{
  if ( client == this )
    setupActions();
}

ToggleToolViewAction::ToggleToolViewAction ( const TQString& text, const KShortcut& cut,KDockWidget *dw, KMDI::MainWindow *mdiMainFrm,
                                             TQObject* parent, const char* name )
 : KToggleAction(text,cut,parent,name)
 , m_dw(dw)
 , m_mdiMainFrm(mdiMainFrm)
{
  if (m_dw)
  {
    connect(this,TQT_SIGNAL(toggled(bool)),this,TQT_SLOT(slotToggled(bool)));
    connect(m_dw->dockManager(),TQT_SIGNAL(change()),this,TQT_SLOT(anDWChanged()));
    connect(m_dw,TQT_SIGNAL(destroyed()),this,TQT_SLOT(slotWidgetDestroyed()));

    setChecked(m_dw->mayBeHide());
  }
}

ToggleToolViewAction::~ToggleToolViewAction()
{
  unplugAll();
}

void ToggleToolViewAction::anDWChanged()
{
  if (isChecked() && m_dw->mayBeShow())
    setChecked(false);
  else if ((!isChecked()) && m_dw->mayBeHide())
    setChecked(true);
  else if (isChecked() && (m_dw->parentDockTabGroup() &&
          ((::tqqt_cast<KDockWidget*>(m_dw->parentDockTabGroup()->
                  parent()))->mayBeShow())))
    setChecked(false);
}


void ToggleToolViewAction::slotToggled(bool t)
{
  if ((!t) && m_dw->mayBeHide() )
    m_dw->undock();
  else
    if ( t && m_dw->mayBeShow() )
      m_mdiMainFrm->makeDockVisible(m_dw);
}

void ToggleToolViewAction::slotWidgetDestroyed()
{
  disconnect(m_dw->dockManager(),TQT_SIGNAL(change()),this,TQT_SLOT(anDWChanged()));
  disconnect(this,TQT_SIGNAL(toggled(bool)),0,0);

  unplugAll();
  deleteLater();
}

}

// kate: space-indent on; indent-width 2; replace-tabs on;
