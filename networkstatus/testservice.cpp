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

#include <tqtimer.h>
#include <dcopclient.h>
#include <kapplication.h>
#include "provideriface.h"

#include "testservice.h"
#include "serviceiface_stub.h"

TestService::TestService() : TQObject(), DCOPObject("ProviderIface")
{
	kapp->dcopClient()->registerAs("testservice" );
	m_service = new ServiceIface_stub( "kded", "networkstatus" );
	m_status = NetworktqStatus::Offline;
	NetworktqStatus::Properties nsp;
	nsp.internet = true;
	nsp.name = "test_net";
	nsp.onDemandPolicy = NetworktqStatus::All;
	nsp.service = kapp->dcopClient()->appId();
	nsp.status = m_status;
	m_service->registerNetwork( "test_net", nsp );
}

TestService::~TestService()
{
	delete m_service;
}

int TestService::status( const TQString & network )
{
	Q_UNUSED( network );
	return (int)m_status;
}

int TestService::establish( const TQString & network )
{
	Q_UNUSED( network );
	m_status = NetworktqStatus::Establishing;
	m_service->setNetworktqStatus( "test_net", (int)m_status );
	m_nexttqStatus = NetworktqStatus::Online;
	TQTimer::singleShot( 5000, this, TQT_SLOT( slottqStatusChange() ) );
	return (int)NetworktqStatus::RequestAccepted;
}

int TestService::shutdown( const TQString & network )
{
	Q_UNUSED( network );
	m_status = NetworktqStatus::ShuttingDown;
	m_service->setNetworktqStatus( "test_net", (int)m_status );
	m_nexttqStatus = NetworktqStatus::Offline;
	TQTimer::singleShot( 5000, this, TQT_SLOT( slottqStatusChange() ) );
	return (int)NetworktqStatus::RequestAccepted;
}

void TestService::simulateFailure()
{
	m_status = NetworktqStatus::OfflineFailed;
	m_service->setNetworktqStatus( "test_net", (int)m_status );
}

void TestService::simulateDisconnect()
{
	m_status = NetworktqStatus::OfflineDisconnected;
	m_service->setNetworktqStatus( "test_net", (int)m_status );
}

void TestService::slottqStatusChange()
{
	m_status = m_nexttqStatus;
	m_service->setNetworktqStatus( "test_net", (int)m_status );
}

int main( int argc, char** argv )
{
	KApplication app(argc, argv, "testdcop");
	TestService * test = new TestService;
	Q_UNUSED( test );
	return app.exec();
}

#include "testservice.moc"
