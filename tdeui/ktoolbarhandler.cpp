/* This file is part of the KDE libraries
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

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

#include "ktoolbarhandler.h"

#include <tqpopupmenu.h>
#include <kapplication.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kaction.h>
#include <assert.h>

namespace
{
    const char *actionListName = "show_menu_and_toolbar_actionlist";

    const char *guiDescription = ""
        "<!DOCTYPE kpartgui><kpartgui name=\"StandardToolBarMenuHandler\">"
        "<MenuBar>"
        "    <Menu name=\"settings\">"
        "        <ActionList name=\"%1\" />"
        "    </Menu>"
        "</MenuBar>"
        "</kpartgui>";

    const char *resourceFileName = "barhandler.rc";

    class BarActionBuilder
    {
    public:
        BarActionBuilder( KActionCollection *actionCollection, KMainWindow *mainWindow, TQPtrList<KToolBar> &oldToolBarList )
            : m_actionCollection( actionCollection ), m_mainWindow( mainWindow ), m_needsRebuild( false )
        {
            TQPtrList<TQDockWindow> dockWindows = m_mainWindow->dockWindows();
            TQPtrListIterator<TQDockWindow> dockWindowIt( dockWindows );
            for ( ; dockWindowIt.current(); ++dockWindowIt ) {

                KToolBar *toolBar = tqt_dynamic_cast<KToolBar *>( dockWindowIt.current() );
                if ( !toolBar )
                    continue;

                if ( oldToolBarList.findRef( toolBar ) == -1 )
                    m_needsRebuild = true;

                m_toolBars.append( toolBar );
            }

            if ( !m_needsRebuild )
                m_needsRebuild = ( oldToolBarList.count() != m_toolBars.count() );
        }

        bool needsRebuild() const { return m_needsRebuild; }

        TQPtrList<KAction> create()
        {
            if ( !m_needsRebuild )
                return TQPtrList<KAction>();

            TQPtrListIterator<KToolBar> toolBarIt( m_toolBars );
            for ( ; toolBarIt.current(); ++toolBarIt )
                handleToolBar( toolBarIt.current() );

            TQPtrList<KAction> actions;

            if ( m_toolBarActions.count() == 0 )
                return actions;

            if ( m_toolBarActions.count() == 1 ) {
                KToggleToolBarAction* action = static_cast<KToggleToolBarAction *>( m_toolBarActions.getFirst() );
                action->setText( i18n( "Show Toolbar" ) );
                action->setCheckedState( i18n( "Hide Toolbar" ) );
                return m_toolBarActions;
            }

            KActionMenu *menuAction = new KActionMenu( i18n( "Toolbars" ), m_actionCollection, "toolbars_submenu_action" );

            TQPtrListIterator<KAction> actionIt( m_toolBarActions );
            for ( ; actionIt.current(); ++actionIt )
                menuAction->insert( actionIt.current() );

            actions.append( menuAction );
            return actions;
        }

        const TQPtrList<KToolBar> &toolBars() const { return m_toolBars; }

    private:
        void handleToolBar( KToolBar *toolBar )
        {
            KToggleToolBarAction *action = new KToggleToolBarAction(
                toolBar,
                toolBar->label(),
                m_actionCollection,
                toolBar->name() );
            // ## tooltips, whatsthis?
            m_toolBarActions.append( action );
        }

        KActionCollection *m_actionCollection;
        KMainWindow *m_mainWindow;

        TQPtrList<KToolBar> m_toolBars;
        TQPtrList<KAction> m_toolBarActions;

        bool m_needsRebuild : 1;
    };
}

using namespace KDEPrivate;

ToolBarHandler::ToolBarHandler( KMainWindow *mainWindow, const char *name )
    : TQObject( mainWindow, name ), KXMLGUIClient( mainWindow )
{
    init( mainWindow );
}

ToolBarHandler::ToolBarHandler( KMainWindow *mainWindow, TQObject *parent, const char *name )
    : TQObject( parent, name ), KXMLGUIClient( mainWindow )
{
    init( mainWindow );
}

ToolBarHandler::~ToolBarHandler()
{
    m_actions.setAutoDelete( true );
    m_actions.clear();
}

KAction *ToolBarHandler::toolBarMenuAction()
{
    assert( m_actions.count() == 1 );
    return m_actions.getFirst();
}

void ToolBarHandler::setupActions()
{
    if ( !factory() || !m_mainWindow )
        return;

    BarActionBuilder builder( actionCollection(), m_mainWindow, m_toolBars );

    if ( !builder.needsRebuild() )
        return;

    unplugActionList( actionListName );

    m_actions.setAutoDelete( true );
    m_actions.clear();
    m_actions.setAutoDelete( false );

    m_actions = builder.create();

    /*
    for (  TQPtrListIterator<KToolBar> toolBarIt( m_toolBars );
           toolBarIt.current(); ++toolBarIt )
        toolBarIt.current()->disconnect( this );
        */

    m_toolBars = builder.toolBars();

    /*
    for (  TQPtrListIterator<KToolBar> toolBarIt( m_toolBars );
           toolBarIt.current(); ++toolBarIt )
        connect( toolBarIt.current(), TQT_SIGNAL( destroyed() ),
                 this, TQT_SLOT( setupActions() ) );
                 */

    if (kapp && kapp->authorizeKAction("options_show_toolbar"))
	plugActionList( actionListName, m_actions );

    connectToActionContainers();
}

void ToolBarHandler::clientAdded( KXMLGUIClient *client )
{
    if ( client == this )
        setupActions();
}

void ToolBarHandler::init( KMainWindow *mainWindow )
{
    d = 0;
    m_mainWindow = mainWindow;

    connect( m_mainWindow->guiFactory(), TQT_SIGNAL( clientAdded( KXMLGUIClient * ) ),
             this, TQT_SLOT( clientAdded( KXMLGUIClient * ) ) );

    /* re-use an existing resource file if it exists. can happen if the user launches the
     * toolbar editor */
    /*
    setXMLFile( resourceFileName );
    */

    if ( domDocument().documentElement().isNull() ) {

        TQString completeDescription = TQString::fromLatin1( guiDescription )
            .arg( actionListName );

        setXML( completeDescription, false /*merge*/ );
    }
}

void ToolBarHandler::connectToActionContainers()
{
    TQPtrListIterator<KAction> actionIt( m_actions );
    for ( ; actionIt.current(); ++actionIt )
        connectToActionContainer( actionIt.current() );
}

void ToolBarHandler::connectToActionContainer( KAction *action )
{
    uint containerCount = action->containerCount();
    for ( uint i = 0; i < containerCount; ++i )
        connectToActionContainer( action->container( i ) );
}

void ToolBarHandler::connectToActionContainer( TQWidget *container )
{
    TQPopupMenu *popupMenu = tqt_dynamic_cast<TQPopupMenu *>( container );
    if ( !popupMenu )
        return;

    connect( popupMenu, TQT_SIGNAL( aboutToShow() ),
             this, TQT_SLOT( setupActions() ) );
}

#include "ktoolbarhandler.moc"

/* vim: et sw=4 ts=4
 */
