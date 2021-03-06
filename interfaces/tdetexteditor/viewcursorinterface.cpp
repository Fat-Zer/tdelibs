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

#include "viewcursorinterface.h"
#include "viewcursordcopinterface.h"
#include "view.h"

namespace KTextEditor
{

class PrivateViewCursorInterface
{
  public:
    PrivateViewCursorInterface() {interface=0;}
    ~PrivateViewCursorInterface() {}
    ViewCursorDCOPInterface *interface;
};

}

using namespace KTextEditor;

unsigned int ViewCursorInterface::globalViewCursorInterfaceNumber = 0;

ViewCursorInterface::ViewCursorInterface()
{
  globalViewCursorInterfaceNumber++;
  myViewCursorInterfaceNumber = globalViewCursorInterfaceNumber++;

  d = new PrivateViewCursorInterface();
  TQString name = "ViewCursorInterface#" + TQString::number(myViewCursorInterfaceNumber);
  d->interface = new ViewCursorDCOPInterface(this, name.latin1());
}

ViewCursorInterface::~ViewCursorInterface()
{
  delete d->interface;
  delete d;
}

unsigned int ViewCursorInterface::viewCursorInterfaceNumber () const
{
  return myViewCursorInterfaceNumber;
}

void ViewCursorInterface::setViewCursorInterfaceDCOPSuffix (const TQCString &suffix)
{
  d->interface->setObjId ("ViewCursorInterface#"+suffix);
}

ViewCursorInterface *KTextEditor::viewCursorInterface (View *view)
{                   
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::ViewCursorInterface*>(view);
}
