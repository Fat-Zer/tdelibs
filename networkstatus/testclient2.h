/*  This file is part of kdepim.

    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of TQt, and distribute the resulting executable,
    without including the source code for TQt in the source distribution.
*/

#ifndef KTESTNETWORKSTATUS_H
#define KTESTNETWORKSTATUS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dcopclient.h>
#include <kapplication.h>
#include <kmainwindow.h>
#include <networkstatuscommon.h>

#include "networkstatusiface_stub.h"

class TestClientView;

/**
 * Test client that uses a ConnectionManager to change its state
 *
 * @short Main window class
 * @author Will Stephenson <wstephenson@kde.org>
 * @version 0.1
 */
class TestClient : public KMainWindow
{
Q_OBJECT
public:
    enum AppStatus{ AppDisconnected, AppWaitingForConnect, AppConnected };
    /**
     * Default Constructor
     */
    TestClient();

    /**
     * Default Destructor
     */
    virtual ~TestClient();

private slots:
    void networkStatusChanged( NetworkStatus::Status status );
    void connectButtonClicked();
    void doConnect();
    void doDisconnect();
private:
    void appWaiting();
    void appEstablishing();
    void appIsConnected();
    void appDisestablishing();
    void appDisconnected();
    static TQColor toQColor( NetworkStatus::Status );
private:
    TQHBox * m_layout;
    NetworkStatusIface_stub *m_service;
    TestClientView *m_view;
    AppStatus m_status; // this represents the app's status not the network's status
};

#endif // KTESTNETWORKSTATUS_H

