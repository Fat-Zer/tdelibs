
#include "browserinterface.h"

#include <tqmetaobject.h>

#include <config.h>
#include <private/qucomextra_p.h>

using namespace KParts;

BrowserInterface::BrowserInterface( TQObject *parent, const char *name )
    : TQObject( parent, name )
{
}

BrowserInterface::~BrowserInterface()
{
}

void BrowserInterface::callMethod( const char *name, const TQVariant &argument )
{
    int slot = tqmetaObject()->tqfindSlot( name );

    if ( slot == -1 )
        return;

    QUObject o[ 2 ];
    TQStringList strLst;
    uint i;

    switch ( argument.type() )
    {
        case TQVariant::Invalid:
            break;
        case TQVariant::String:
            static_QUType_QString.set( o + 1, argument.toString() );
            break;
        case TQVariant::StringList:
	    strLst = argument.toStringList();
            static_QUType_ptr.set( o + 1, &strLst );
            break;
        case TQVariant::Int:
            static_QUType_int.set( o + 1, argument.toInt() );
            break;
        case TQVariant::UInt:
	    i = argument.toUInt();
	    static_QUType_ptr.set( o + 1, &i );
            break;
        case TQVariant::Bool:
	    static_QUType_bool.set( o + 1, argument.toBool() );
            break;
        default: return;
    }
  
    qt_invoke( slot, o );
}

#include "browserinterface.moc"
