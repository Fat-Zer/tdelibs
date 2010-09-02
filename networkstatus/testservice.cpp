/*  This file is part of kdepim.
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of TQt, and distribute the resulting executable,
    without including the source code for TQt in the source distribution.
*/

#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqtimer.h>

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>

#include "testservice.h"
#include "testserviceview.h"
#include "networkstatusiface_stub.h"

TestService::TestService() : KMainWindow( 0, "testservice" ),
    m_service( new NetworkStatusIface_stub( "kded", "networkstatus" ) ),
    m_status ( NetworkStatus::Offline ),
    m_nextStatus( NetworkStatus::OfflineDisconnected ),
    m_view( new TestServiceView( this ) )
{
    setCentralWidget( m_view );
    kapp->dcopClient()->registerAs("testservice" );

    connect( m_view->changeCombo, TQT_SIGNAL( activated( int ) ), TQT_SLOT( changeComboActivated( int ) ) );
    connect( m_view->changeButton, TQT_SIGNAL( clicked() ), TQT_SLOT( changeButtonClicked() ) );

    connect( kapp->dcopClient(), TQT_SIGNAL( applicationRegistered( const TQCString& ) ), this, TQT_SLOT( registeredToDCOP( const TQCString& ) ) );
    kapp->dcopClient()->setNotifications( true );

    m_view->statusLabel->setText( NetworkStatus::toString( m_status ) );
    m_view->statusLabel->setPaletteBackgroundColor( toQColor( m_status ) );
    setCaption( NetworkStatus::toString( m_status ) );

    registerService();
}

TestService::~TestService()
{
    delete m_service;
    delete m_view;
}

void TestService::registerService()
{
    NetworkStatus::Properties nsp;
    nsp.name = "test_net";
    nsp.service = kapp->dcopClient()->appId();
    nsp.status = m_status;
    m_service->registerNetwork( nsp );
}

void TestService::registeredToDCOP( const TQCString & appId )
{
    if ( appId == "kded" )
        registerService();
}

int TestService::status( const TQString & network )
{
    Q_UNUSED( network );
    return (int)m_status;
}

void TestService::changeComboActivated( int index )
{
  switch ( index ) {
    case 0 /*NetworkStatus::OfflineDisconnected*/:
      m_nextStatus = NetworkStatus::OfflineDisconnected;
      break;
    case 1 /*NetworkStatus::OfflineFailed*/:
      m_nextStatus = NetworkStatus::OfflineFailed;
      break;
    case 2 /*NetworkStatus::ShuttingDown*/:
      m_nextStatus = NetworkStatus::ShuttingDown;
      break;
    case 3 /*NetworkStatus::Offline*/:
      m_nextStatus = NetworkStatus::Offline;
      break;
    case 4 /*NetworkStatus::Establishing*/:
      m_nextStatus = NetworkStatus::Establishing;
      break;
    case 5 /*NetworkStatus::Online*/:
      m_nextStatus = NetworkStatus::Online;
      break;
    default:
      kdDebug() << "Unrecognised status!" << endl;
      Q_ASSERT( false );
  }
  m_view->changeButton->setEnabled( true );
}

void TestService::changeButtonClicked()
{
  m_view->changeButton->setEnabled( false );
  m_status = m_nextStatus;
  m_service->setNetworkStatus( "test_net", ( int )m_status );
  m_view->statusLabel->setText( NetworkStatus::toString( m_status ) );
  m_view->statusLabel->setPaletteBackgroundColor( toQColor( m_status ) );
  setCaption( NetworkStatus::toString( m_status ) );
}

int TestService::establish( const TQString & network )
{
	Q_UNUSED( network );
	m_status = NetworkStatus::Establishing;
	m_service->setNetworkStatus( "test_net", (int)m_status );
	m_nextStatus = NetworkStatus::Online;
	TQTimer::singleShot( 5000, this, TQT_SLOT( slotStatusChange() ) );
	return (int)NetworkStatus::RequestAccepted;
}

int TestService::shutdown( const TQString & network )
{
	Q_UNUSED( network );
	m_status = NetworkStatus::ShuttingDown;
	m_service->setNetworkStatus( "test_net", (int)m_status );
	m_nextStatus = NetworkStatus::Offline;
	TQTimer::singleShot( 5000, this, TQT_SLOT( slotStatusChange() ) );
	return (int)NetworkStatus::RequestAccepted;
}

void TestService::simulateFailure()
{
	m_status = NetworkStatus::OfflineFailed;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

void TestService::simulateDisconnect()
{
	m_status = NetworkStatus::OfflineDisconnected;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

void TestService::slotStatusChange()
{
	m_status = m_nextStatus;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

TQColor TestService::toQColor( NetworkStatus::Status st )
{
    TQColor col;
    switch ( st ) {
      case NetworkStatus::NoNetworks:
        col = Qt::darkGray;
        break;
      case NetworkStatus::Unreachable:
        col = Qt::darkMagenta;
        break;
      case NetworkStatus::OfflineDisconnected:
        col = Qt::blue;
        break;
      case NetworkStatus::OfflineFailed:
        col = Qt::darkRed;
        break;
      case NetworkStatus::ShuttingDown:
        col = Qt::darkYellow;
        break;
      case NetworkStatus::Offline:
        col = Qt::blue;
        break;
      case NetworkStatus::Establishing:
        col = Qt::yellow;
        break;
      case NetworkStatus::Online:
        col = Qt::green;
        break;
    }
    return col;
}

static const char description[] =
    I18N_NOOP("Test Service for Network Status kded module");

static const char version[] = "v0.1";

static KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main( int argc, char** argv )
{
    KAboutData about("KNetworkStatusTestService", I18N_NOOP("knetworkstatustestservice"), version, description, KAboutData::License_GPL, "(C) 2007 Will Stephenson", 0, 0, "wstephenson@kde.org");
    about.addAuthor( "Will Stephenson", 0, "wstephenson@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    TestService * test = new TestService;
    test->show();
    return app.exec();
}

#include "testservice.moc"
