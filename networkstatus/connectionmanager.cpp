/*  This file is part of kdepim.
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

#include <kapplication.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include "connectionmanager.h"
#include "connectionmanager_p.h"

// Connection manager itself
ConnectionManager::ConnectionManager( TQObject * parent, const char * name ) : DCOPObject( "ConnectionManager" ), TQObject( parent, name ), d( new ConnectionManagerPrivate( this ) )
{
    d->service = new NetworkStatusIface_stub( kapp->dcopClient(), "kded", "networkstatus" );

    connectDCOPSignal( "kded", "networkstatus", "statusChange(int)", "slotStatusChanged(int)", false );

    initialise();
}

ConnectionManager::~ConnectionManager()
{
    delete d;
}

ConnectionManager *ConnectionManager::s_self = 0L;

ConnectionManager *ConnectionManager::self()
{
    static KStaticDeleter<ConnectionManager> deleter;
    if(!s_self)
        deleter.setObject( s_self, new ConnectionManager( 0, "connection_manager" ) );
    return s_self;	
}

void ConnectionManager::initialise()
{
    // determine initial state and set the state object accordingly.
    d->status = ( NetworkStatus::Status )d->service->status();
}

NetworkStatus::Status ConnectionManager::status()
{
    return d->status;
}

void ConnectionManager::slotStatusChanged( int status )
{
    d->status = ( NetworkStatus::Status )status;
    switch ( status ) {
      case NetworkStatus::NoNetworks:
        break;
      case NetworkStatus::Unreachable:
        break;
      case NetworkStatus::OfflineDisconnected:
      case NetworkStatus::OfflineFailed:
      case NetworkStatus::ShuttingDown:
      case NetworkStatus::Offline:
      case NetworkStatus::Establishing:
        if ( d->disconnectPolicy == Managed ) {
          emit d->disconnected();
        } else if ( d->disconnectPolicy == OnNextChange ) {
          setDisconnectPolicy( Manual );
          emit d->disconnected();
        }
        break;
      case NetworkStatus::Online:
        if ( d->disconnectPolicy == Managed ) {
          emit d->connected();
        } else if ( d->disconnectPolicy == OnNextChange ) {
          setConnectPolicy( Manual );
          emit d->connected();
        }
        break;
      default:
        kdDebug() << k_funcinfo <<  "Unrecognised status code!" << endl;
    }
    emit statusChanged( d->status );
}

ConnectionManager::ConnectionPolicy ConnectionManager::connectPolicy() const
{
    return d->connectPolicy;
}

void ConnectionManager::setConnectPolicy( ConnectionManager::ConnectionPolicy policy )
{
    d->connectPolicy = policy;
}

ConnectionManager::ConnectionPolicy ConnectionManager::disconnectPolicy() const
{
    return d->disconnectPolicy;
}

void ConnectionManager::setDisconnectPolicy( ConnectionManager::ConnectionPolicy policy )
{
    d->disconnectPolicy = policy;
}

void ConnectionManager::setManualConnectionPolicies()
{
    d->connectPolicy = ConnectionManager::Manual;
    d->disconnectPolicy = ConnectionManager::Manual;
}

void ConnectionManager::setManagedConnectionPolicies()
{
    d->connectPolicy = ConnectionManager::Managed;
    d->disconnectPolicy = ConnectionManager::Managed;
}

void ConnectionManager::registerConnectSlot( TQObject * receiver, const char * member )
{
    d->connectReceiver = receiver;
    d->connectSlot = member;
    connect( d, TQT_SIGNAL( connected() ), receiver, member );
}

void ConnectionManager::forgetConnectSlot()
{
    disconnect( d, TQT_SIGNAL( connected() ), d->connectReceiver, d->connectSlot );
    d->connectReceiver = 0;
    d->connectSlot = 0;
}

bool ConnectionManager::isConnectSlotRegistered() const
{
    return ( d->connectSlot != 0 );
}

void ConnectionManager::registerDisconnectSlot( TQObject * receiver, const char * member )
{
    d->disconnectReceiver = receiver;
    d->disconnectSlot = member;
    connect( d, TQT_SIGNAL( disconnected() ), receiver, member );
}

void ConnectionManager::forgetDisconnectSlot()
{
    disconnect( d, TQT_SIGNAL( disconnected() ), d->disconnectReceiver, d->disconnectSlot );
    d->disconnectReceiver = 0;
    d->disconnectSlot = 0;
}

bool ConnectionManager::isDisconnectSlotRegistered() const
{
    return ( d->disconnectSlot != 0 );
}

#include "connectionmanager.moc"

