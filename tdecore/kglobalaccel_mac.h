#ifndef _KGLOBALACCEL_MAC_H
#define _KGLOBALACCEL_MAC_H

#include <tqwidget.h>

#include "kshortcut.h"
#include "kaccelbase.h"

class TDEGlobalAccelPrivate: public KAccelBase
{
public:
    TDEGlobalAccelPrivate()
        : KAccelBase(KAccelBase::NATIVE_KEYS)
    {}

    // reimplemented pure virtuals
    void setEnabled( bool bEnabled )
    { Q_UNUSED(bEnabled); }
    bool emitSignal( Signal signal )
    { Q_UNUSED(signal); return false; }
    bool connectKey( KAccelAction& action, const KKeyServer::Key& key)
    { Q_UNUSED(action); Q_UNUSED(key); return false; }
    bool connectKey( const KKeyServer::Key& key)
    { Q_UNUSED(key); return false; }
    bool disconnectKey( KAccelAction&, const KKeyServer::Key& key)
    { Q_UNUSED(key); return false; }
    bool disconnectKey( const KKeyServer::Key& )
    { return false; }
};

#endif // _KGLOBALACCEL_EMB_H
