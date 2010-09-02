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

#ifndef NETWORKSTATUS_NETWORK_H
#define NETWORKSTATUS_NETWORK_H

#include "networkstatuscommon.h"

class Network
{
public:
	Network( const TQString name );
	Network( NetworkStatus::Properties properties );
	/**
	 * Update the status of this network
	 */
	void setStatus( NetworkStatus::Status status );
	/**
	 * The connection status of this network
	 */
	NetworkStatus::Status status();
	/**
	 * The name of this network
	 */
	TQString name();
	void setName( const TQString& name );
	/**
	 * Returns the service owning this network
	 */
	TQString service();
	void setService( const TQString& service );

private:
	Network( const Network & );
	TQString m_name;
	NetworkStatus::Status m_status;
	TQString m_service;
};

#endif
// vim: sw=4 ts=4
