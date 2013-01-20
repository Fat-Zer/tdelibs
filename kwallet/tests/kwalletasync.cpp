#include <tqtextstream.h>
#include <tqtimer.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <kwallet.h>

#include "kwallettest.h"

static TQTextStream _out( stdout, IO_WriteOnly );

void openWallet()
{
	_out << "About to ask for wallet async" << endl;

        // we have no wallet: ask for one.
	KWallet::Wallet *wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), 0, KWallet::Wallet::Asynchronous );

	WalletReceiver r;
	r.connect( wallet, TQT_SIGNAL( walletOpened(bool) ), TQT_SLOT( walletOpened(bool) ) );

	_out << "About to start 30 second event loop" << endl;

	TQTimer::singleShot( 30000, tqApp, TQT_SLOT( quit() ) );
	int ret = tqApp->exec();

	if ( ret == 0 )
		_out << "Timed out!" << endl;
	else
		_out << "Success!" << endl;
}

void WalletReceiver::walletOpened( bool got )
{
	_out << "Got async wallet: " << got << endl;
	tqApp->exit( 1 );
}

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kwalletasync", "kwalletasync", "version" );
	TDECmdLineArgs::init( argc, argv, &aboutData );
	TDEApplication app( "kwalletasync" );

	// register with DCOP
	_out << "DCOP registration returned " << app.dcopClient()->registerAs(app.name()) << endl;

	openWallet();

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

