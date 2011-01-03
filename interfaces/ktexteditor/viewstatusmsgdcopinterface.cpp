#include "viewstatusmsgdcopinterface.h"
#include "viewstatusmsginterface.h"

#include <tqstring.h>

#include <dcopclient.h>
using namespace KTextEditor;

ViewtqStatusMsgDCOPInterface::ViewtqStatusMsgDCOPInterface( ViewtqStatusMsgInterface *Parent, const char *name)
	: DCOPObject(name)
{
	m_parent = Parent;
}

ViewtqStatusMsgDCOPInterface::~ViewtqStatusMsgDCOPInterface()
{

}

uint ViewtqStatusMsgDCOPInterface::viewtqStatusMsgInterfaceNumber ()
{
	return m_parent->viewtqStatusMsgInterfaceNumber ();
}

void ViewtqStatusMsgDCOPInterface::viewtqStatusMsg (TQString msg)
{
	m_parent->viewtqStatusMsg(msg);
}

