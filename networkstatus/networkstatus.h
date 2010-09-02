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

#ifndef KDED_NETWORKSTATUS_H
#define KDED_NETWORKSTATUS_H

#include <kdedmodule.h>

#include "networkstatuscommon.h"
#include "network.h"

class NetworkStatusModule : virtual public KDEDModule
{
Q_OBJECT
K_DCOP
public:
    NetworkStatusModule( const TQCString& obj );
    ~NetworkStatusModule();
k_dcop:
    // Client interface
    TQStringList networks();
    int status();
    // Service interface
    void setNetworkStatus( const TQString & networkName, int status );
    void registerNetwork( NetworkStatus::Properties properties );
    void unregisterNetwork( const TQString & networkName );
k_dcop_signals:
    /**
     * A status change occurred affecting the overall connectivity
     * @param status The new status
     */
    void statusChange( int status );
protected slots:
    //void registeredToDCOP( const TQCString& appId );
    void unregisteredFromDCOP( const TQCString& appId );

protected:
    // recalculate cached status
    void updateStatus();

private:
    class Private;
    Private *d;
};

#endif
// vim: sw=4 ts=4
