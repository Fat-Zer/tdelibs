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

#include "sessionconfiginterface.h"

#include "document.h"
#include "view.h"
#include "plugin.h"

namespace KTextEditor
{

class PrivateSessionConfigInterface
{
  public:
    PrivateSessionConfigInterface() {}
    ~PrivateSessionConfigInterface() {}
};

}

using namespace KTextEditor;

unsigned int SessionConfigInterface::globalSessionConfigInterfaceNumber = 0;

SessionConfigInterface::SessionConfigInterface()
{
  globalSessionConfigInterfaceNumber++;
  mySessionConfigInterfaceNumber = globalSessionConfigInterfaceNumber++;

  d = new PrivateSessionConfigInterface();
}

SessionConfigInterface::~SessionConfigInterface()
{
  delete d;
}

unsigned int SessionConfigInterface::configInterfaceNumber () const
{
  return mySessionConfigInterfaceNumber;
}

void SessionConfigInterface::setSessionConfigInterfaceDCOPSuffix (const TQCString &/*suffix*/)
{
  //d->interface->setObjId ("SessionConfigInterface#"+suffix);
}

SessionConfigInterface *KTextEditor::sessionConfigInterface (Document *doc)
{                       
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::SessionConfigInterface*>(doc);
}      

SessionConfigInterface *KTextEditor::sessionConfigInterface (View *view)
{                       
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::SessionConfigInterface*>(view);
}

SessionConfigInterface *KTextEditor::sessionConfigInterface (Plugin *plugin)
{                       
  if (!plugin)
    return 0;

  return dynamic_cast<KTextEditor::SessionConfigInterface*>(plugin);
}
