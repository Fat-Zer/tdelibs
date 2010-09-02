/*  This file is part of kdepim.

    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

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

#ifndef _TEST_NETWORKSTATUS_SERVICE_H
#define _TEST_NETWORKSTATUS_SERVICE_H

#include <kmainwindow.h>

#include "networkstatuscommon.h"

class NetworkStatusIface_stub;
class TestServiceView;

class TestService : public KMainWindow {
Q_OBJECT
public:
	TestService();
	virtual ~TestService();
	int status( const TQString & network );
	int establish( const TQString & network );
	int shutdown( const TQString & network );
	void simulateFailure();
	void simulateDisconnect();
protected slots:
	void changeComboActivated( int index );
    void registeredToDCOP( const TQCString& appId );

	void changeButtonClicked();

	void slotStatusChange();
private:
    void registerService();
    static TQColor toQColor( NetworkStatus::Status );
	NetworkStatusIface_stub * m_service;
	NetworkStatus::Status m_status;
	NetworkStatus::Status m_nextStatus;
    TestServiceView * m_view;
};

#endif
