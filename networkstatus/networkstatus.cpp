/*  This file is part of kdepim
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of TQt, and distribute the resulting executable,
    without including the source code for TQt in the source distribution.
*/

#include "networkstatus.h"

#include <tqmap.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

#include "network.h"
#include <kdemacros.h>

extern "C" {
    KDE_EXPORT KDEDModule* create_networkstatus( const TQCString& obj )
    {
        return new NetworkStatusModule( obj );
    }
}

// INTERNALLY USED STRUCTS AND TYPEDEFS

typedef TQMap< TQString, Network * > NetworkMap;

class NetworkStatusModule::Private
{
public:
    NetworkMap networks;
    NetworkStatus::Status status;
};

// CTORS/DTORS

NetworkStatusModule::NetworkStatusModule( const TQCString & obj ) : KDEDModule( obj ), d( new Private )
{
    d->status = NetworkStatus::NoNetworks;
    connect( kapp->dcopClient(), TQT_SIGNAL( applicationRemoved( const TQCString& ) ) , this, TQT_SLOT( unregisteredFromDCOP( const TQCString& ) ) );
    //	connect( kapp->dcopClient(), TQT_SIGNAL( applicationRegistered( const TQCString& ) ) , this, TQT_SLOT( registeredToDCOP( const TQCString& ) ) );
}

NetworkStatusModule::~NetworkStatusModule()
{
    NetworkMap::ConstIterator it;
    const NetworkMap::ConstIterator end = d->networks.end();

    for ( it = d->networks.begin(); it != end; ++it ) {
        delete ( *it );
    }

    delete d;
}

// CLIENT INTERFACE

TQStringList NetworkStatusModule::networks()
{
    kdDebug() << k_funcinfo << " contains " << d->networks.count() << " networks" << endl;
    return d->networks.keys();
}

int NetworkStatusModule::status()
{
    kdDebug() << k_funcinfo << " status: " << (int)d->status << endl;
    return (int)d->status;
}

//protected:

void NetworkStatusModule::updateStatus()
{
    NetworkStatus::Status bestStatus = NetworkStatus::NoNetworks;
    const NetworkStatus::Status oldStatus = d->status;

    NetworkMap::ConstIterator it;
    const NetworkMap::ConstIterator end = d->networks.end();
    for ( it = d->networks.begin(); it != end; ++it ) {
        if ( ( *it )->status() > bestStatus )
            bestStatus = ( *it )->status();
    }
    d->status = bestStatus;

    if ( oldStatus != d->status ) {
        statusChange( (int)d->status );
    }
}

void NetworkStatusModule::unregisteredFromDCOP( const TQCString & appId )
{
    // unregister and delete any networks owned by a service that has just unregistered
    NetworkMap::Iterator it = d->networks.begin();
    const NetworkMap::Iterator end = d->networks.end();
	while (it != d->networks.end())
    {
        if ( ( *it )->service() == TQString( appId ) )
        {
            NetworkMap::Iterator toRemove = it++;
			delete *toRemove;
            d->networks.remove( toRemove );
            updateStatus();
			continue;
        }
		++it;
    }
}

// SERVICE INTERFACE //
void NetworkStatusModule::setNetworkStatus( const TQString & networkName, int st )
{
    kdDebug() << k_funcinfo << networkName << ", " << st << endl;
    NetworkStatus::Status changedStatus = (NetworkStatus::Status)st;
    Network * net = 0;
    NetworkMap::Iterator it = d->networks.find( networkName );
    if ( it != d->networks.end() ) {
        net = (*it);
        net->setStatus( changedStatus );
        updateStatus();
    }
    else
        kdDebug() << "  No network named '" << networkName << "' found." << endl;
}

void NetworkStatusModule::registerNetwork( const NetworkStatus::Properties properties )
{
    kdDebug() << k_funcinfo << properties.name << ", with status " << properties.status << endl;

    d->networks.insert( properties.name, new Network( properties ) );
    updateStatus();
}

void NetworkStatusModule::unregisterNetwork( const TQString & networkName )
{
    kdDebug() << k_funcinfo << networkName << endl;

    NetworkMap::Iterator it = d->networks.find( networkName );
    if ( it != d->networks.end() ) {
        delete *it;
        d->networks.remove( it );
    }
    updateStatus();
}

#include "networkstatus.moc"
// vim: set noet sw=4 ts=4:
