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

// $Id$

#define INCLUDE_MENUITEM_DEF
#include <tqmenudata.h>

#include "config.h"

#include "kcheckaccelerators.h"
#include "kaccelmanager.h"
#include <tqpopupmenu.h>
#include <tqapplication.h>
#include <tqdialog.h>
#include <tqlayout.h>
#include <tqtextview.h>
#include <tqobjectlist.h>
#include <tqmenubar.h>
#include <tqtabbar.h>
#include <tqpushbutton.h>
#include <tqmetaobject.h>
#include <tqcheckbox.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kshortcut.h>
#include <klocale.h>

/*

 HOWTO:

 This class allows translators (and application developers) to check for accelerator
 conflicts in menu and widgets. Put the following in your kdeglobals (or the config
 file for the application you're testing):

 [Development]
 CheckAccelerators=F12
 AutoCheckAccelerators=false
 AlwaysShowCheckAccelerators=false

 The checking can be either manual or automatic. To perform manual check, press
 the keyboard shortcut set to 'CheckAccelerators' (here F12). If automatic checking
 is enabled by setting 'AutoCheckAccelerators' to true, check will be performed every
 time the GUI changes. It's possible that in certain cases the check will be
 done also when no visible changes in the GUI happen or the check won't be done
 even if the GUI changed (in the latter case, use manual check ). Automatic
 checks can be anytime disabled by the checkbox in the dialog presenting
 the results of the check. If you set 'AlwaysShowCheckAccelerators' to true,
 the dialog will be shown even if the automatic check didn't find any conflicts,
 and all submenus will be shown, even those without conflicts.

 The dialog first lists the name of the window, then all results for all menus
 (if the window has a menubar) and then result for all controls in the active
 window (if there are any checkboxes etc.). For every submenu and all controls
 there are shown all conflicts grouped by accelerator, and a list of all used
 accelerators.
*/

KCheckAccelerators::KCheckAccelerators( TQObject* parent )
    : TQObject( parent, "kapp_accel_filter" ), key(0), block( false ), drklash(0)
{
    parent->installEventFilter( this );
    TDEConfigGroupSaver saver( TDEGlobal::config(), "Development" );
    TQString sKey = TDEGlobal::config()->readEntry( "CheckAccelerators" ).stripWhiteSpace();
    if( !sKey.isEmpty() ) {
      KShortcut cuts( sKey );
      if( cuts.count() > 0 )
        key = int(cuts.seq(0).qt());
    }
    alwaysShow = TDEGlobal::config()->readBoolEntry( "AlwaysShowCheckAccelerators", false );
    autoCheck = TDEGlobal::config()->readBoolEntry( "AutoCheckAccelerators", true );
    connect( &autoCheckTimer, TQT_SIGNAL( timeout()), TQT_SLOT( autoCheckSlot()));
}

bool KCheckAccelerators::eventFilter( TQObject * , TQEvent * e)
{
    if ( block )
        return false;

    switch ( e->type() ) { // just simplify debuggin
    case TQEvent::Accel:
        if ( key && (TQT_TQKEYEVENT(e)->key() == key) ) {
    	    block = true;
	    checkAccelerators( false );
	    block = false;
	    TQT_TQKEYEVENT(e)->accept();
	    return true;
	}
        break;
    case TQEvent::ChildInserted:
    case TQEvent::ChildRemoved:
    case TQEvent::Resize:
    case TQEvent::LayoutHint:
    case TQEvent::WindowActivate:
    case TQEvent::WindowDeactivate:
        if( autoCheck )
            autoCheckTimer.start( 20, true ); // 20 ms
        break;
    case TQEvent::Timer:
    case TQEvent::MouseMove:
    case TQEvent::Paint:
        return false;
    default:
        // kdDebug(125) << "KCheckAccelerators::eventFilter " << e->type() << " " << autoCheck << endl;
        break;
    }
    return false;
}

void KCheckAccelerators::autoCheckSlot()
{
    if( block )
    {
        autoCheckTimer.start( 20, true );
        return;
    }
    block = true;
    checkAccelerators( !alwaysShow );
    block = false;
}

void KCheckAccelerators::createDialog(TQWidget *actWin, bool automatic)
{
    if ( drklash )
        return;

    drklash = new TQDialog( actWin, "kapp_accel_check_dlg", false, (WFlags)TQt::WDestructiveClose);
    drklash->setCaption( i18n( "Dr. Klash' Accelerator Diagnosis" ));
    drklash->resize( 500, 460 );
    TQVBoxLayout* layout = new TQVBoxLayout( drklash, 11, 6 );
    layout->setAutoAdd( true );
    drklash_view = new TQTextView( drklash );
    TQCheckBox* disableAutoCheck = NULL;
    if( automatic )  {
        disableAutoCheck = new TQCheckBox( i18n( "&Disable automatic checking" ), drklash );
        connect(disableAutoCheck, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotDisableCheck(bool)));
    }
    TQPushButton* btnClose = new TQPushButton( i18n( "&Close" ), drklash );
    btnClose->setDefault( true );
    connect( btnClose, TQT_SIGNAL( clicked() ), drklash, TQT_SLOT( close() ) );
    if (disableAutoCheck)
        disableAutoCheck->setFocus();
    else
        drklash_view->setFocus();
}

void KCheckAccelerators::slotDisableCheck(bool on)
{
    autoCheck = !on;
    if (!on)
        autoCheckSlot();
}

void KCheckAccelerators::checkAccelerators( bool automatic )
{
    TQWidget* actWin = TQT_TQWIDGET(tqApp->activeWindow());
    if ( !actWin )
        return;

    KAcceleratorManager::manage(actWin);
    TQString a, c, r;
    KAcceleratorManager::last_manage(a, c,  r);

    if (automatic) // for now we only show dialogs on F12 checks
        return;

    if (c.isEmpty() && r.isEmpty() && (automatic || a.isEmpty()))
        return;

    TQString s;

    if ( ! c.isEmpty() )  {
        s += i18n("<h2>Accelerators changed</h2>");
        s += "<table border><tr><th><b>Old Text</b></th><th><b>New Text</b></th></tr>"
             + c + "</table>";
    }

    if ( ! r.isEmpty() )  {
        s += i18n("<h2>Accelerators removed</h2>");
        s += "<table border><tr><th><b>Old Text</b></th></tr>" + r + "</table>";
    }

    if ( ! a.isEmpty() )  {
        s += i18n("<h2>Accelerators added (just for your info)</h2>");
        s += "<table border><tr><th><b>New Text</b></th></tr>" + a + "</table>";
    }

    createDialog(actWin, automatic);
    drklash_view->setText(s);
    drklash->show();
    drklash->raise();

    // dlg will be destroyed before returning
}

#include "kcheckaccelerators.moc"
