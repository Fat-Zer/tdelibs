/*
    This file is part of kdepim.

    Copyright (c) 2005 Will Stephenson <lists@stevello.free-online.co.uk>

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

#include "networkstatus.h"

#include <tqdict.h>
#include <tqtimer.h>
#include <tqvaluelist.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

#include "clientifaceimpl.h"
#include "serviceifaceimpl.h"
#include "network.h"

#include <kdeversion.h>
#include <kdemacros.h>

#if KDE_IS_VERSION( 3,3,90 )
/* life is great */
#else
/* workaround typo that breaks compilation with newer gcc */
#undef KDE_EXPORT
#define KDE_EXPORT
#undef KDE_NO_EXPORT
#define KDE_NO_EXPORT
#endif

extern "C" {
	KDE_EXPORT KDEDModule* create_networkstatus( const TQCString& obj )
	{
		return new NetworktqStatusModule( obj );
	}
}

// INTERNALLY USED STRUCTS AND TYPEDEFS

//typedef TQDict< Network > NetworkList;
typedef TQValueList< Network * > NetworkList;

class NetworktqStatusModule::Private
{
public:
	NetworkList networks;
/*	ClientIface * clientIface;
	ServiceIface * serviceIface;*/
};

// CTORS/DTORS

NetworktqStatusModule::NetworktqStatusModule( const TQCString & obj ) : KDEDModule( obj )
{
	d = new Private;
/*	d->clientIface = new ClientIfaceImpl( this );
	d->serviceIface = new ServiceIfaceImpl( this );*/
	connect( kapp->dcopClient(), TQT_SIGNAL( applicationRemoved( const TQCString& ) ) , this, TQT_SLOT( unregisteredFromDCOP( const TQCString& ) ) );
	connect( kapp->dcopClient(), TQT_SIGNAL( applicationRegistered( const TQCString& ) ) , this, TQT_SLOT( registeredToDCOP( const TQCString& ) ) );
}

NetworktqStatusModule::~NetworktqStatusModule()
{
/*	delete d->clientIface;
	delete d->serviceIface;*/
	delete d;
}

// CLIENT INTERFACE

TQStringList NetworktqStatusModule::networks()
{
	kdDebug() << k_funcinfo << " tqcontains " << d->networks.count() << " networks" << endl;
	TQStringList networks;
	NetworkList::iterator end = d->networks.end();
	NetworkList::iterator it = d->networks.begin();
	for ( ; it != end; ++it )
		networks.append( (*it)->name() );
	return networks;
}

int NetworktqStatusModule::status( const TQString & host )
{
	if ( host == "127.0.0.1" || host == "localhost" )
		return NetworktqStatus::Online;
	Network * p = networkForHost( host );
	if ( !p )
	{
		//kdDebug() << k_funcinfo << " no networks have status for host '" << host << "'" << endl;
		return (int)NetworktqStatus::NoNetworks;
	}
	else
	{	
		kdDebug() << k_funcinfo << " got status for host '" << host << "' : " << (int)(p->status()) << endl;
		return (int)(p->status());
	}
}

int NetworktqStatusModule::request( const TQString & host, bool userInitiated )
{
	// identify most suitable network for host
	Network * p = networkForHost( host );
	if ( !p )
		return NetworktqStatus::Unavailable;
	
	NetworktqStatus::EnumtqStatus status = p->status();
	TQCString appId = kapp->dcopClient()->senderId();
	if ( status == NetworktqStatus::Online )
	{
		p->registerUsage( appId, host );
		return NetworktqStatus::Connected;
	}
	// if online
	//   register usage
	//   return Available
	else if ( status == NetworktqStatus::Establishing )
	{
		p->registerUsage( appId, host );
		return NetworktqStatus::RequestAccepted;
	}
	// if establishing
	//   register usage
	//   return Accepted
	else if ( status == NetworktqStatus::Offline || status == NetworktqStatus::ShuttingDown )
	{
		// TODO: check on demand policy
		
		p->registerUsage( appId, host );
		return NetworktqStatus::RequestAccepted;
	}
	// if offline or ShuttingDown
	//   check ODP::
	//   always or Permanent: register, return accepted
	//   user: check userInitiated, register, return Accepted or UserRefused
	//   never: return UserRefused
	else if ( status == NetworktqStatus::OfflineFailed )
	{
		// TODO: check user's preference for dealing with failed networks
		p->registerUsage( appId, host );
		return NetworktqStatus::RequestAccepted;
	}
	// if OfflineFailed
	//   check user's preference
	else if ( status == NetworktqStatus::OfflineDisconnected )
	{
		return NetworktqStatus::Unavailable;
	}
	else
		return NetworktqStatus::Unavailable;
	// if OfflineDisconnected or NoNetworks
	//   return Unavailable
}

void NetworktqStatusModule::relinquish( const TQString & host )
{
	TQCString appId = kapp->dcopClient()->senderId();
	// tqfind network currently used by app for host...
	NetworkList::iterator end = d->networks.end();
	NetworkList::iterator it = d->networks.begin();
	for ( ; it != end; ++it )
	{
		Network * net = *it;
		NetworkUsageList usage = net->usage();
		NetworkUsageList::iterator end2 = usage.end();
		for ( NetworkUsageList::iterator usageIt = usage.begin(); usageIt != end2; ++usageIt )
		{
			if ( (*usageIt).appId == appId && (*usageIt).host == host )
			{
				// remove host usage record
				usage.remove( usageIt );
				// if requested shutdown flagged for network
				//  check if all hosts have relinquished
				//   call confirmShutDown on Service
				//checkShutdownOk();
			}
		}
	}
}

bool NetworktqStatusModule::reportFailure( const TQString & host )
{
	// tqfind network for host
	// check IP record.  remove IP usage record.  if other IP exists, return true.
	Q_UNUSED( host );
	kdDebug() << k_funcinfo << "NOT IMPLEMENTED" << endl;
	return false;
}

// PROTECTED UTILITY FUNCTIONS
/*
 * Determine the network to use for the supplied host
 */
Network * NetworktqStatusModule::networkForHost( const TQString & host ) const
{
	// return a null pointer if no networks are registered
	if ( d->networks.isEmpty() )
		return 0;
	
	NetworkList::const_iterator it = d->networks.begin();
	Network * bestNetwork = *(it++);
	NetworkList::const_iterator end = d->networks.end();
 	for ( ; it != end; ++it )
	{
		if ( (*it)->reachabilityFor( host ) > bestNetwork->reachabilityFor( host ) )
		{
			bestNetwork = (*it);
		}
	}
	return bestNetwork;
}


void NetworktqStatusModule::registeredToDCOP( const TQCString & appId )
{
}

void NetworktqStatusModule::unregisteredFromDCOP( const TQCString & appId )
{
	// unregister any networks owned by a service that has just unregistered
	NetworkList::iterator it = d->networks.begin();
	NetworkList::iterator end = d->networks.end();
	for ( ; it != end; ++it )
	{
		if ( (*it)->service() == appId)
		{
			kdDebug() << k_funcinfo << "removing '" << (*it)->name() << "', registered by " << appId << endl;
			d->networks.remove( it );
			break;
		}
	}
}

// SERVICE INTERFACE //
void NetworktqStatusModule::setNetworktqStatus( const TQString & networkName, int st )
{
	kdDebug() << k_funcinfo << endl;
	NetworktqStatus::EnumtqStatus status = (NetworktqStatus::EnumtqStatus)st;
	Network * net = 0;
	NetworkList::iterator it = d->networks.begin();
	NetworkList::iterator end = d->networks.end();
	for ( ; it != end; ++it )
	{
		if ( (*it)->name() == networkName )
		{
			net = (*it);
			break;
		}
	}
	if ( net )
	{
		if ( net->status() == status )
			return;

		// update the status of the network
		net->setStatus( status );

		// notify for each host in use on that network
		NetworkUsageList usage = net->usage();
		NetworkUsageList::iterator end = usage.end();
		TQStringList notified;
		for ( NetworkUsageList::iterator it = usage.begin(); it != end; ++it )
		{
			// only notify once per host
			if ( !notified.tqcontains( (*it).host ) )
			{
				kdDebug() << "notifying statusChange of " << networkName << " to " << (int)status << 
						" because " << (*it).appId << " is using " << (*it).host << endl;
				/*d->clientIface->*/statusChange( (*it).host, (int)status );
				notified.append( (*it).host );
			}
		}

		// if we are now anything but Establishing or Online, reset the usage records for that network
		if ( !( net->status() == NetworktqStatus::Establishing || net->status() == NetworktqStatus::Establishing ) )
			net->removeAllUsage();
	}
	else
		kdDebug() << k_funcinfo << "No network found by this name" << endl;
}

void NetworktqStatusModule::registerNetwork( const TQString & networkName, const NetworktqStatus::Properties properties )
{
	kdDebug() << k_funcinfo << "registering '" << networkName << "', with status " << properties.status << endl;
	// TODO: check for re-registration, checking appid matches
	
	d->networks.append( new Network( networkName, properties ) );
}

void NetworktqStatusModule::unregisterNetwork( const TQString & networkName )
{
	// TODO: check appid
	//d->networks.remove( networkName );
}

void NetworktqStatusModule::requestShutdown( const TQString & networkName )
{
	Q_UNUSED( networkName );
	kdDebug() << k_funcinfo << "NOT IMPLEMENTED" << endl;
}

#include "networkstatus.moc"
