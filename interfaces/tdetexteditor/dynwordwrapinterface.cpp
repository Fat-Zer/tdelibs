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

#include "dynwordwrapinterface.h"
#include "view.h"

namespace KTextEditor
{

class PrivateDynWordWrapInterface
{
  public:
    PrivateDynWordWrapInterface() {}
    ~PrivateDynWordWrapInterface() {}
};

}

using namespace KTextEditor;

unsigned int DynWordWrapInterface::globalDynWordWrapInterfaceNumber = 0;

DynWordWrapInterface::DynWordWrapInterface()
{
  globalDynWordWrapInterfaceNumber++;
  myDynWordWrapInterfaceNumber = globalDynWordWrapInterfaceNumber++;

  d = new PrivateDynWordWrapInterface();
}

DynWordWrapInterface::~DynWordWrapInterface()
{
  delete d;
}

unsigned int DynWordWrapInterface::dynWordWrapInterfaceNumber () const
{
  return myDynWordWrapInterfaceNumber;
}

void DynWordWrapInterface::setDynWordWrapInterfaceDCOPSuffix (const TQCString &/*suffix*/)
{
  //d->interface->setObjId ("DynWordWrapInterface#"+suffix);
}

DynWordWrapInterface *KTextEditor::dynWordWrapInterface (View *view)
{           
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::DynWordWrapInterface*>(view);
}
