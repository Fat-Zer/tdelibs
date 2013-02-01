
#include <tqguardedptr.h>

#include <kapplication.h>
#include <kaction.h>

#include <assert.h>

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "kactiontest" );

    TDEActionCollection coll( static_cast<TQObject *>( 0 ) );

    TQGuardedPtr<TDEAction> action1 = new TDERadioAction("test",0, &coll);
    TQGuardedPtr<TDEAction> action2 = new TDERadioAction("test",0, &coll);
    TQGuardedPtr<TDEAction> action3 = new TDERadioAction("test",0, &coll);
    TQGuardedPtr<TDEAction> action4 = new TDERadioAction("test",0, &coll);
    TQGuardedPtr<TDEAction> action5 = new TDERadioAction("test",0, &coll);
    TQGuardedPtr<TDEAction> action6 = new TDERadioAction("test",0, &coll);
    TQGuardedPtr<TDEAction> action7 = new TDERadioAction("test",0, &coll);
   
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
