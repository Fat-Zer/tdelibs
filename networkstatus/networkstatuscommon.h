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

#ifndef NETWORKSTATUS_COMMON_H
#define NETWORKSTATUS_COMMON_H

#include <tqstringlist.h>

namespace NetworkStatus
{
    enum Status { NoNetworks = 1, Unreachable, OfflineDisconnected,  OfflineFailed, ShuttingDown, Offline, Establishing, Online };
    enum RequestResult { RequestAccepted = 1, Connected, UserRefused, Unavailable };
    enum UnusedDemandPolicy { All, User, None, Permanent };

    // BINARY COMPATIBILITY ALERT BEGIN !!!!
    struct Properties
    {
        TQString name;
        Status status;
        UnusedDemandPolicy unused1;
        TQCString           service;
        bool               unused3;
        TQStringList        unused4;
    };
    // BINARY COMPATIBILITY ALERT END !!!!

    TQString toString( Status st );
}

TQDataStream & operator>> ( TQDataStream & s, NetworkStatus::Properties &p );
TQDataStream & operator<< ( TQDataStream & s, const NetworkStatus::Properties p );

#endif
