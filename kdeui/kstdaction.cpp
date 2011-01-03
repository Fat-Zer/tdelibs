/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>

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

#include "kstdaction.h"

#include <tqtoolbutton.h>
#include <tqwhatsthis.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kmainwindow.h>
#include "kstdaction_p.h"

namespace KStdAction
{

TQStringList stdNames()
{
    return internal_stdNames();
}

KAction* create( StdAction id, const char *name, const TQObject *recvr, const char *slot, KActionCollection* parent )
{
	KAction* pAction = 0;
	const KStdActionInfo* pInfo = infoPtr( id );
	kdDebug(125) << "KStdAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << ", " << name << " )" << endl; // ellis
	if( pInfo ) {
		TQString sLabel, iconName = pInfo->psIconName;
		switch( id ) {
		 case Back: sLabel = i18n("go back", "&Back");
			if (TQApplication::reverseLayout() )
			    iconName = "forward";
			break;

		 case Forward: sLabel = i18n("go forward", "&Forward");
			if (TQApplication::reverseLayout() )
			    iconName = "back";
			break;

		 case Home: sLabel = i18n("beginning (of line)", "&Home"); break;
		 case Help: sLabel = i18n("show help", "&Help"); break;
		 case AboutApp: iconName = kapp->miniIconName();
		 case Preferences:
		 case HelpContents:
			{
			const KAboutData *aboutData = KGlobal::instance()->aboutData();
			/* TODO KDE4
			const KAboutData *aboutData;
			if ( parent )
			    aboutData = parent->instance()->aboutData();
			else
			    aboutData = KGlobal::instance()->aboutData();
			*/
			TQString appName = (aboutData) ? aboutData->programName() : TQString::tqfromLatin1(tqApp->name());
			sLabel = i18n(pInfo->psLabel).arg(appName);
			}
			break;
		 default: sLabel = i18n(pInfo->psLabel);
		}

		if (TQApplication::reverseLayout()){
			if (id == Prior) iconName = "forward";
			if (id == Next ) iconName = "back";
			if (id == FirstPage) iconName = "finish";
			if (id == LastPage) iconName = "start";
		}

		KShortcut cut = KStdAccel::shortcut(pInfo->idAccel);
		switch( id ) {
		 case OpenRecent:
			pAction = new KRecentFilesAction( sLabel, pInfo->psIconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			break;
		 case ShowMenubar:
		 case ShowToolbar:
		 case ShowtqStatusbar:
		 {
			KToggleAction *ret;
			ret = new KToggleAction( sLabel, pInfo->psIconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			ret->setChecked( true );
			pAction = ret;
			break;
		 }
		 case FullScreen:
		 {
			KToggleFullScreenAction *ret;
			ret = new KToggleFullScreenAction( cut, recvr, slot,
					parent, NULL, (name) ? name : pInfo->psName );
			ret->setChecked( false );
			pAction = ret;
			break;
		 }
         case PasteText:
         {
            KPasteTextAction *ret;
            ret = new KPasteTextAction(sLabel, iconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
            pAction = ret;
            break;
         }
		 default:
			pAction = new KAction( sLabel, iconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			break;
		}
	}
	return pAction;
}

const char* name( StdAction id )
{
	const KStdActionInfo* pInfo = infoPtr( id );
	return (pInfo) ? pInfo->psName : 0;
}

KAction *openNew( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( New, name, recvr, slot, parent ); }
KAction *open( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Open, name, recvr, slot, parent ); }
KRecentFilesAction *openRecent( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return (KRecentFilesAction*) KStdAction::create( OpenRecent, name, recvr, slot, parent ); }
KAction *save( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Save, name, recvr, slot, parent ); }
KAction *saveAs( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( SaveAs, name, recvr, slot, parent ); }
KAction *revert( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Revert, name, recvr, slot, parent ); }
KAction *print( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Print, name, recvr, slot, parent ); }
KAction *printPreview( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( PrintPreview, name, recvr, slot, parent ); }
KAction *close( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Close, name, recvr, slot, parent ); }
KAction *mail( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Mail, name, recvr, slot, parent ); }
KAction *quit( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Quit, name, recvr, slot, parent ); }
KAction *undo( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Undo, name, recvr, slot, parent ); }
KAction *redo( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Redo, name, recvr, slot, parent ); }
KAction *cut( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Cut, name, recvr, slot, parent ); }
KAction *copy( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Copy, name, recvr, slot, parent ); }
KAction *paste( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Paste, name, recvr, slot, parent ); }
KAction *pasteText( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( PasteText, name, recvr, slot, parent ); }
KAction *clear( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Clear, name, recvr, slot, parent ); }
KAction *selectAll( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( SelectAll, name, recvr, slot, parent ); }
KAction *deselect( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Deselect, name, recvr, slot, parent ); }
KAction *tqfind( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Find, name, recvr, slot, parent ); }
KAction *tqfindNext( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FindNext, name, recvr, slot, parent ); }
KAction *tqfindPrev( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FindPrev, name, recvr, slot, parent ); }
KAction *tqreplace( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Replace, name, recvr, slot, parent ); }
KAction *actualSize( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ActualSize, name, recvr, slot, parent ); }
KAction *fitToPage( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToPage, name, recvr, slot, parent ); }
KAction *fitToWidth( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToWidth, name, recvr, slot, parent ); }
KAction *fitToHeight( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToHeight, name, recvr, slot, parent ); }
KAction *zoomIn( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ZoomIn, name, recvr, slot, parent ); }
KAction *zoomOut( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ZoomOut, name, recvr, slot, parent ); }
KAction *zoom( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Zoom, name, recvr, slot, parent ); }
KAction *redisplay( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Redisplay, name, recvr, slot, parent ); }
KAction *up( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Up, name, recvr, slot, parent ); }
KAction *back( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Back, name, recvr, slot, parent ); }
KAction *forward( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Forward, name, recvr, slot, parent ); }
KAction *home( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Home, name, recvr, slot, parent ); }
KAction *prior( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Prior, name, recvr, slot, parent ); }
KAction *next( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Next, name, recvr, slot, parent ); }
KAction *goTo( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Goto, name, recvr, slot, parent ); }
KAction *gotoPage( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( GotoPage, name, recvr, slot, parent ); }
KAction *gotoLine( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( GotoLine, name, recvr, slot, parent ); }
KAction *firstPage( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FirstPage, name, recvr, slot, parent ); }
KAction *lastPage( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( LastPage, name, recvr, slot, parent ); }
KAction *addBookmark( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( AddBookmark, name, recvr, slot, parent ); }
KAction *editBookmarks( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( EditBookmarks, name, recvr, slot, parent ); }
KAction *spelling( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Spelling, name, recvr, slot, parent ); }

KToggleAction *showMenubar( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *_name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show &Menubar"), "showmenu", KStdAccel::shortcut(KStdAccel::ShowMenubar), recvr, slot,
                            parent, _name ? _name : name(ShowMenubar));
    ret->setWhatsThis( i18n( "Show Menubar<p>"
                             "Shows the menubar again after it has been hidden" ) );
    KGuiItem guiItem( i18n("Hide &Menubar"), 0 /*same icon*/, TQString::null,
                      i18n( "Hide Menubar<p>"
                            "Hide the menubar. You can usually get it back using the right mouse button inside the window itself." ) );
    ret->setCheckedState( guiItem );
    ret->setChecked(true);
    return ret;
}

// obsolete
KToggleAction *showToolbar( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *_name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show &Toolbar"), 0, recvr, slot, parent,
                            _name ? _name : name(ShowToolbar));
    ret->setChecked(true);
    return ret;

}

// obsolete
KToggleToolBarAction *showToolbar( const char* toolBarName, KActionCollection* parent, const char *_name )
{
    KToggleToolBarAction *ret;
    ret = new KToggleToolBarAction(toolBarName, i18n("Show &Toolbar"), parent,
                            _name ? _name : name(ShowToolbar));
    return ret;
}

KToggleAction *showtqStatusbar( const TQObject *recvr, const char *slot,
                                         KActionCollection* parent, const char *_name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show St&atusbar"), 0, recvr, slot, parent,
                            _name ? _name : name(ShowtqStatusbar));
    ret->setWhatsThis( i18n( "Show tqStatusbar<p>"
                             "Shows the statusbar, which is the bar at the bottom of the window used for status information." ) );
    KGuiItem guiItem( i18n("Hide St&atusbar"), TQString::null, TQString::null,
                      i18n( "Hide tqStatusbar<p>"
                            "Hides the statusbar, which is the bar at the bottom of the window used for status information." ) );
    ret->setCheckedState( guiItem );

    ret->setChecked(true);
    return ret;
}

KToggleFullScreenAction *fullScreen( const TQObject *recvr, const char *slot, KActionCollection* parent,
    TQWidget* window, const char *name )
{
    KToggleFullScreenAction *ret;
    ret = static_cast< KToggleFullScreenAction* >( KStdAction::create( FullScreen, name, recvr, slot, parent ));
    ret->setWindow( window );
    return ret;
}

KAction *saveOptions( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( SaveOptions, name, recvr, slot, parent ); }
KAction *keyBindings( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( KeyBindings, name, recvr, slot, parent ); }
KAction *preferences( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Preferences, name, recvr, slot, parent ); }
KAction *configureToolbars( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ConfigureToolbars, name, recvr, slot, parent ); }
KAction *configureNotifications( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ConfigureNotifications, name, recvr, slot, parent ); }
KAction *help( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Help, name, recvr, slot, parent ); }
KAction *helpContents( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( HelpContents, name, recvr, slot, parent ); }
KAction *whatsThis( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( WhatsThis, name, recvr, slot, parent ); }
KAction *tipOfDay( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( TipofDay, name, recvr, slot, parent ); }
KAction *reportBug( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ReportBug, name, recvr, slot, parent ); }
KAction *switchApplicationLanguage( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( SwitchApplicationLanguage, name, recvr, slot, parent ); }
KAction *aboutApp( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( AboutApp, name, recvr, slot, parent ); }
KAction *aboutKDE( const TQObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( AboutKDE, name, recvr, slot, parent ); }

}
