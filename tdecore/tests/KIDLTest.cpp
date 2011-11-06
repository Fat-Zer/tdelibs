#include "KIDLTest.h"

#include <kapplication.h>
#include <dcopclient.h>

KIDLTest::KIDLTest( const TQCString& id )
    : DCOPObject( id )
{
}

TQString KIDLTest::hello( const TQString& name )
{
    qDebug("Du heter %s", name.latin1() );
    qDebug("Ha det %s", name.latin1() );
	
    return TQString("Jeg heter KIDLTest");
}

int main( int argc, char** argv )
{
    KApplication app( argc, argv, "kidlservertest", false /* No GUI */ );

    app.dcopClient()->attach();
    app.dcopClient()->registerAs( "kidlservertest" );

    qDebug("Server process started...");

    (void) new KIDLTest( "Hello" );

    qDebug("Server listening ...");

    return app.exec();
}
