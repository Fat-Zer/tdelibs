
#include <tqguardedptr.h>

#include <kapplication.h>
#include <kaction.h>

#include <assert.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kactiontest" );

    KActionCollection coll( static_cast<TQObject *>( 0 ) );

    TQGuardedPtr<KAction> action1 = new KRadioAction("test",0, &coll);
    TQGuardedPtr<KAction> action2 = new KRadioAction("test",0, &coll);
    TQGuardedPtr<KAction> action3 = new KRadioAction("test",0, &coll);
    TQGuardedPtr<KAction> action4 = new KRadioAction("test",0, &coll);
    TQGuardedPtr<KAction> action5 = new KRadioAction("test",0, &coll);
    TQGuardedPtr<KAction> action6 = new KRadioAction("test",0, &coll);
    TQGuardedPtr<KAction> action7 = new KRadioAction("test",0, &coll);
   
    coll.clear();
    assert( coll.isEmpty() );

    assert( action1.isNull() );
    assert( action2.isNull() );
    assert( action3.isNull() );
    assert( action4.isNull() );
    assert( action5.isNull() );
    assert( action6.isNull() );
    assert( action7.isNull() );
    
    return 0;
}

/* vim: et sw=4 ts=4
 */
