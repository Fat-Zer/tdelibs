#include <tdeapplication.h>
#include <dcopclient.h>

#include "KIDLTest_stub.h"

int main( int argc, char** argv )
{
    TDEApplication app( argc, argv, "KIDLTestClient", false /* No GUI */ );

    kapp->dcopClient()->attach();
    // kapp->dcopClient()->registerAs( "kidlclienttest" );

    KIDLTest_stub* t = new KIDLTest_stub( "kidlservertest", "Hello" );

    TQString ret = t->hello("Torben");
    tqDebug("Server says: %s", ret.latin1() );
}
