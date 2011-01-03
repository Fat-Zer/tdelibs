/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// $Id$

#include "viewstatusmsginterface.h"       
#include "viewstatusmsgdcopinterface.h" 
#include "view.h"

#include <tqstring.h>

namespace KTextEditor
{

class PrivateViewtqStatusMsgInterface
{
  public:
    PrivateViewtqStatusMsgInterface() {interface=0;}
    ~PrivateViewtqStatusMsgInterface() {}
    ViewtqStatusMsgDCOPInterface  *interface;
};

}

using namespace KTextEditor;

unsigned int ViewtqStatusMsgInterface::globalViewtqStatusMsgInterfaceNumber = 0;

ViewtqStatusMsgInterface::ViewtqStatusMsgInterface()
{
  globalViewtqStatusMsgInterfaceNumber++;
  myViewtqStatusMsgInterfaceNumber = globalViewtqStatusMsgInterfaceNumber++;

  d = new PrivateViewtqStatusMsgInterface();
  ::TQString name = "ViewtqStatusMsgInterface#" + ::TQString::number(myViewtqStatusMsgInterfaceNumber);
  d->interface = new ViewtqStatusMsgDCOPInterface(this, name.latin1());
}

ViewtqStatusMsgInterface::~ViewtqStatusMsgInterface()
{
  delete d->interface;
  delete d;
}

unsigned int ViewtqStatusMsgInterface::viewtqStatusMsgInterfaceNumber () const
{
  return myViewtqStatusMsgInterfaceNumber;
}

void ViewtqStatusMsgInterface::setViewtqStatusMsgInterfaceDCOPSuffix (const TQCString &suffix)
{
  d->interface->setObjId ("ViewtqStatusMsgInterface#"+suffix);
}

ViewtqStatusMsgInterface *KTextEditor::viewtqStatusMsgInterface (View *view)
{           
  if (!view)
    return 0;

  return static_cast<ViewtqStatusMsgInterface*>(view->qt_cast("KTextEditor::ViewtqStatusMsgInterface"));
}
