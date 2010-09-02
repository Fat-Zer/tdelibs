#include "connectionmanager_p.h"

ConnectionManagerPrivate::ConnectionManagerPrivate(TQObject * parent, const char * name ) : TQObject( parent, name ), service( 0 ), connectPolicy( ConnectionManager::Managed ),
  disconnectPolicy( ConnectionManager::Managed ), connectReceiver( 0 ), connectSlot( 0 ),
  disconnectReceiver( 0 ), disconnectSlot( 0 )
{
}

ConnectionManagerPrivate::~ConnectionManagerPrivate()
{
}

#include "connectionmanager_p.moc"
