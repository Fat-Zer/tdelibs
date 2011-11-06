/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#include "kglobalaccel.h"
#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#elif defined(Q_WS_WIN)
#include "kglobalaccel_win.h"
#elif defined(Q_WS_MACX)
#include "kglobalaccel_mac.h"
#else
#include "kglobalaccel_emb.h"
#endif

#include <tqstring.h>
#include "kaccelbase.h"
#include <kdebug.h>
#include <kshortcut.h>
#include <klocale.h>

//----------------------------------------------------

KGlobalAccel::KGlobalAccel( TQObject* pParent, const char* psName )
: TQObject( pParent, psName )
{
	kdDebug(125) << "KGlobalAccel(): this = " << this << endl;
	d = new KGlobalAccelPrivate();
}

KGlobalAccel::~KGlobalAccel()
{
	kdDebug(125) << "~KGlobalAccel(): this = " << this << endl;
	delete d;
}

/*
void KGlobalAccel::clear()
	{ d->clearActions(); }
*/
KAccelActions& KGlobalAccel::actions()
	{ return d->KAccelBase::actions(); }

const KAccelActions& KGlobalAccel::actions() const
	{ return d->KAccelBase::actions(); }

bool KGlobalAccel::isEnabled()
	{ return ((KAccelBase*)d)->isEnabled(); }

void KGlobalAccel::setEnabled( bool bEnabled )
	{ d->setEnabled( bEnabled ); }

void KGlobalAccel::suspend( bool s )
	{ d->suspend( s ); }

void KGlobalAccel::blockShortcuts( bool block )
        { KGlobalAccelPrivate::blockShortcuts( block ); }

void KGlobalAccel::disableBlocking( bool disable )
        { d->disableBlocking( disable ); }

KAccelAction* KGlobalAccel::insert( const TQString& sAction, const TQString& sDesc, const TQString& sHelp,
		const KShortcut& cutDef3, const KShortcut& cutDef4,
		const TQObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( sAction, sDesc, sHelp,
		cutDef3, cutDef4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

KAccelAction* KGlobalAccel::insert( const TQString& sName, const TQString& sDesc )
	{ return d->insert( sName, sDesc ); }
bool KGlobalAccel::updateConnections()
	{ return d->updateConnections(); }

bool KGlobalAccel::remove( const TQString& sAction )
        { return d->remove( sAction ); }

const KShortcut& KGlobalAccel::shortcut( const TQString& sAction ) const
{
	const KAccelAction* pAction = d->KAccelBase::actions().actionPtr( sAction );
	return (pAction) ? pAction->shortcut() : KShortcut::null();
}

bool KGlobalAccel::setShortcut( const TQString& sAction, const KShortcut& cut )
	{ return d->setShortcut( sAction, cut ); }
bool KGlobalAccel::setSlot( const TQString& sAction, const TQObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }
TQString KGlobalAccel::label( const TQString& sAction ) const
{
	const KAccelAction* pAction = d->KAccelBase::actions().actionPtr( sAction );
	return (pAction) ? pAction->label() : TQString();
}
bool KGlobalAccel::setActionEnabled( const TQString& sAction, bool bEnable )
{
        return d->setActionEnabled( sAction, bEnable );
}

const TQString& KGlobalAccel::configGroup() const
	{ return d->configGroup(); }
// for kdemultimedia/kmix
void KGlobalAccel::setConfigGroup( const TQString& s )
	{ d->setConfigGroup( s ); }

bool KGlobalAccel::readSettings( KConfigBase* pConfig )
	{ d->readSettings( pConfig ); return true; }
bool KGlobalAccel::writeSettings( KConfigBase* pConfig ) const
	{ d->writeSettings( pConfig ); return true; }
bool KGlobalAccel::writeSettings( KConfigBase* pConfig, bool bGlobal ) const
{
	d->setConfigGlobal( bGlobal );
	d->writeSettings( pConfig );
	return true;
}

bool KGlobalAccel::useFourModifierKeys()
	{ return KAccelAction::useFourModifierKeys(); }

void KGlobalAccel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kglobalaccel.moc"
