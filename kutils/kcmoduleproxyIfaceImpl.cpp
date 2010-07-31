/*
 * Copyright (C) 2004 Frans Englich <frans.englich@telia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <tqcstring.h>
#include <tqdatastream.h>

#include <dcopclient.h>

#include <kapplication.h>
#include <kcmoduleproxy.h>
#include <kdebug.h>

#include "kcmoduleproxyIfaceImpl.h"


#include <tqmessagebox.h>

KCModuleProxyIfaceImpl::KCModuleProxyIfaceImpl( const TQCString& name, 
		KCModuleProxy* const client )
	: DCOPObject( name ), TQObject( 0, name ),
		p( const_cast<KCModuleProxy *>( client ))
{ 
	connect( p, TQT_SIGNAL( changed(bool)), 
			TQT_SLOT( changedRelay(bool)));
	connect( p, TQT_SIGNAL( quickHelpChanged()), 
			TQT_SLOT( quickHelpRelay()));
}

void KCModuleProxyIfaceImpl::save()
{
	kdDebug(711) << k_funcinfo << endl;
	p->save();
}

void KCModuleProxyIfaceImpl::load()
{
	kdDebug(711) << k_funcinfo << endl;
	p->load();
}

void KCModuleProxyIfaceImpl::defaults()
{
	kdDebug(711) << k_funcinfo << endl;
	p->defaults();
}

TQString KCModuleProxyIfaceImpl::applicationName()
{
	return kapp->caption();
}

TQString KCModuleProxyIfaceImpl::quickHelp()
{
	return p->quickHelp();
}

bool KCModuleProxyIfaceImpl::changed()
{
	return p->changed();
}

void KCModuleProxyIfaceImpl::changedRelay( bool c )
{
	TQByteArray data;
	TQDataStream stream(data, IO_WriteOnly);
	stream << c;
	emitDCOPSignal( "changed(bool)", data );
}

void KCModuleProxyIfaceImpl::quickHelpRelay()
{
	TQByteArray data;
	emitDCOPSignal( "quickHelpChanged()", data );
}

/***************************************************************/




/***************************************************************/
KCModuleProxyRootCommunicatorImpl::KCModuleProxyRootCommunicatorImpl
		( const TQCString& name, KCModuleProxy* const client )
	: DCOPObject( name ), TQObject( 0, name ), 
		p( const_cast<KCModuleProxy *>( client ))
{ 
	/*
	 * Connect kcmshell's KCModuleProxy's change signal 
	 * to us, such that we act as a proxy for 
	 * KCModuleProxy's API.
	 */

	/* Note, we don't use KCModuleProxy::d->dcopClient */
	kapp->dcopClient()->connectDCOPSignal( 0, p->dcopName(), 
			"changed(bool)", objId(), "changed(bool)", false );

	kapp->dcopClient()->connectDCOPSignal( 0, p->dcopName(), 
			"quickHelpChanged()", objId(), "quickHelpChanged()", false );
}

/* Reimplementations of DCOP members */
void KCModuleProxyRootCommunicatorImpl::changed( bool c )
{
	kdDebug(711) << k_funcinfo << endl;
	p->moduleChanged( c );
}

void KCModuleProxyRootCommunicatorImpl::quickHelpChanged()
{
	kdDebug(711) << k_funcinfo << endl;
	p->emitQuickHelpChanged();
}

#include "kcmoduleproxyIfaceImpl.moc"
