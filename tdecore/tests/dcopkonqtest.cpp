#include <tdeapplication.h>
#include <kdebug.h>
#include <dcopclient.h>

int main( int argc, char** argv )
{
    TDEApplication app( argc, argv, "KIDLTestClient", false );

    kapp->dcopClient()->attach();
    // kapp->dcopClient()->registerAs( "kidlclienttest" );

    TQByteArray data;

    kdDebug() << "sending reparseConfiguration to object KonquerorIface in konqueror" << endl;
       TQByteArray snd;
       TQByteArray rcv;
       TQCString _type_;
       kapp->dcopClient()->call( "konqueror", "KonquerorIface", "reparseConfiguration()", snd, _type_, rcv );
       kdDebug() << _type_ << endl;
       if( _type_ != "void" ) kdDebug() << "void expected, " << _type_.data() << " returned" << endl;

/*
debug("sending configure to object KonquerorIface in konqueror");
    if (kapp->dcopClient()->send( "konqueror", "KonquerorIface", "configure()", data ))
      debug("ok");
    else
      debug("ko");

/// BROADCAST
debug("sending databaseChanged to object tdesycoca in all apps");
    if (kapp->dcopClient()->send( "*", "tdesycoca", "databaseChanged()", data ))
      debug("ok");
    else
      debug("ko");
*/
}
