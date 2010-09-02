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

#include <kdebug.h>

#include "network.h"

Network::Network( NetworkStatus::Properties properties )
	: m_name( properties.name ), m_status( properties.status ), m_service( properties.service )
{
}

void Network::setStatus( NetworkStatus::Status status )
{
	m_status = status;
}

NetworkStatus::Status Network::status()
{
	return m_status;
}

void Network::setName( const TQString& name )
{
	m_name = name;
}

TQString Network::name()
{
	return m_name;
}

TQString Network::service()
{
	return m_service;
}

void Network::setService( const TQString& service )
{
	m_service = service;
}

// vim: sw=4 ts=4
