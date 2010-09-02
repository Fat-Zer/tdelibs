/*  This file is part of kdepim.
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

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

#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqvbox.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>

#include <connectionmanager.h>
#include <networkstatusindicator.h>

#include "testclientview.h"
#include "testclient2.h"

TestClient::TestClient()
    : KMainWindow( 0, "ktestnetworkstatus" ),
      m_layout( new TQVBox( 0, "layout" ) ),
      m_status( AppDisconnected )
{
    m_view = new TestClientView( this );
    new StatusBarNetworkStatusIndicator( m_view, "statusindicator" );
    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(m_view);

    networkStatusChanged( ConnectionManager::self()->status() );
    appDisconnected();

    connect( ConnectionManager::self(), TQT_SIGNAL( statusChanged( NetworkStatus::Status ) ), TQT_SLOT( networkStatusChanged( NetworkStatus::Status ) ) );
    ConnectionManager::self()->registerConnectSlot( this, TQT_SLOT( doConnect() ) );
    ConnectionManager::self()->registerDisconnectSlot( this, TQT_SLOT( doDisconnect() ) );

    connect( m_view->connectButton, TQT_SIGNAL( clicked() ), TQT_SLOT( connectButtonClicked() ) );
}

TestClient::~TestClient()
{
}

void TestClient::networkStatusChanged( NetworkStatus::Status status )
{
    kdDebug() << k_funcinfo << endl;
    kdDebug() << "Networking is now: " << NetworkStatus::toString( status ) << " (" << status << ")" << endl;
    m_view->netStatusLabel->setText( NetworkStatus::toString( status ) );
    m_view->netStatusLabel->setPaletteBackgroundColor( toQColor( status ) );
}

void TestClient::doConnect()
{
  Q_ASSERT( ConnectionManager::self()->status() == NetworkStatus::Online );
  if ( m_status != AppConnected ) {
    appIsConnected();
  }
}

void TestClient::doDisconnect()
{
  Q_ASSERT( ConnectionManager::self()->status() != NetworkStatus::Online );
  if ( m_status == AppConnected ) {
    appDisconnected();
  }
}

void TestClient::connectButtonClicked()
{
  kdDebug() << k_funcinfo << endl;
  if ( m_status == AppDisconnected ) {
    switch ( ConnectionManager::self()->status() )
    {
      case NetworkStatus::NoNetworks:
      case NetworkStatus::Online:
        appIsConnected();
        break;
      default:
        appWaiting();
        break;
    }
  }
  else if ( m_status == AppConnected || m_status == AppWaitingForConnect ) {
    appDisconnected();
  }
}

void TestClient::appWaiting()
{
  kdDebug() << k_funcinfo << endl;
  //m_status = AppWaitingForConnect;
  m_view->appStatusLabel->setText( "Waiting" );
}

void TestClient::appIsConnected()
{
  kdDebug() << k_funcinfo << endl;
  m_view->connectButton->setEnabled( true );
  m_view->connectButton->setText( "Disconnect" );
  m_view->appStatusLabel->setText( "Connected" );
  m_status = AppConnected;
}

void TestClient::appEstablishing()
{
  kdDebug() << k_funcinfo << endl;
  m_view->netStatusLabel->setText( "Establishing" );
  m_view->connectButton->setEnabled( false );
}

void TestClient::appDisestablishing( )
{
  kdDebug() << k_funcinfo << endl;
  m_view->connectButton->setEnabled( false );
  m_view->appStatusLabel->setText( "Disconnected" );
}

void TestClient::appDisconnected( )
{
  kdDebug() << k_funcinfo << endl;
  m_view->connectButton->setEnabled( true );
  m_view->connectButton->setText( "Start Connect" );
  m_view->appStatusLabel->setText( "Disconnected" );
  m_status = AppDisconnected;
}

TQColor TestClient::toQColor( NetworkStatus::Status st )
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
//main
static const char description[] =
    I18N_NOOP("Test Client for Network Status kded module");

static const char version[] = "v0.1";

static KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
  KAboutData about("KNetworkStatusTestClient", I18N_NOOP("knetworkstatustestclient"), version, description, KAboutData::License_GPL, "(C) 2007 Will Stephenson", 0, 0, "wstephenson@kde.org");
  about.addAuthor( "Will Stephenson", 0, "wstephenson@kde.org" );
  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions(options);
  KApplication app;

  // register ourselves as a dcop client
  app.dcopClient()->registerAs(app.name(), false);

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->count() == 0)
  {
    TestClient *widget = new TestClient;
    widget->show();
  }
  else
  {
    int i = 0;
    for (; i < args->count(); i++)
    {
      TestClient *widget = new TestClient;
      widget->show();
    }
  }
  args->clear();

  return app.exec();
}

#include "testclient2.moc"

