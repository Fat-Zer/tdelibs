/* This file is part of the KDE project
   Copyright (C) 2002 Anders Lund <anders@alweb.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
   
   $Id$
*/

#include "selectioninterfaceext.h"
#include "selectionextdcopinterface.h"
#include "document.h"
#include "view.h"
#include <dcopclient.h>

using namespace KTextEditor;

//BEGIN KTextEditor::SelectionInterfaceExt
class KTextEditor::PrivateSelectionInterfaceExt {
  public:
    PrivateSelectionInterfaceExt() : interface( 0 ) {}
    ~PrivateSelectionInterfaceExt() {}
    
    SelectionExtDCOPInterface *interface;
};

unsigned int SelectionInterfaceExt::globalSelectionInterfaceExtNumber = 0;

SelectionInterfaceExt::SelectionInterfaceExt()
  : d ( new PrivateSelectionInterfaceExt )
{
  globalSelectionInterfaceExtNumber++;
  mySelectionInterfaceExtNumber = globalSelectionInterfaceExtNumber;
  TQString name = "SelectionInterfaceExt#" + TQString::number(mySelectionInterfaceExtNumber);
  d->interface = new SelectionExtDCOPInterface(this, name.latin1());
}

SelectionInterfaceExt::~SelectionInterfaceExt()
{
  delete d->interface;
  delete d;
}

unsigned int SelectionInterfaceExt::selectionInterfaceExtNumber () const
{
  return mySelectionInterfaceExtNumber;
}

void SelectionInterfaceExt::setSelectionInterfaceExtDCOPSuffix (const TQCString &suffix)
{
  d->interface->setObjId ("SelectionInterfaceExt#"+suffix);
}

SelectionInterfaceExt *KTextEditor::selectionInterfaceExt (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::SelectionInterfaceExt*>(doc);
}

SelectionInterfaceExt *KTextEditor::selectionInterfaceExt (View *view)
{
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::SelectionInterfaceExt*>(view);
}

//END KTextEditor::SelectionInterfaceExt

//BEGIN KTextEditor::SelectionExtDCOPInterface
SelectionExtDCOPInterface::SelectionExtDCOPInterface(
                SelectionInterfaceExt *parent, const char* name )
  : DCOPObject( name ),
    m_parent( parent )
{
}

SelectionExtDCOPInterface::~SelectionExtDCOPInterface()
{
}

int SelectionExtDCOPInterface::selStartLine()
{
  return m_parent->selStartLine();
}

int SelectionExtDCOPInterface::selStartCol()
{
  return m_parent->selStartCol();
}

int SelectionExtDCOPInterface::selEndLine()
{
  return m_parent->selEndLine();
}

int SelectionExtDCOPInterface::selEndCol()
{
  return m_parent->selEndCol();
}
//END KTextEditor::SelectionExtDCOPInterface
