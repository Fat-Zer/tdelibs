/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
*/

#ifndef KABC_PLUGIN_H
#define KABC_PLUGIN_H

#include <tqstring.h>

#include <tdelibs_export.h>

namespace TDEABC {

class KABC_EXPORT Plugin
{
public:
  Plugin();
  virtual ~Plugin();

  virtual void setType( const TQString& type );
  virtual TQString type() const;

  virtual void setNameLabel( const TQString& label );
  virtual TQString nameLabel() const;

  virtual void setDescriptionLabel( const TQString& label );
  virtual TQString descriptionLabel() const;

private:
  TQString mType;
  TQString mNameLabel;
  TQString mDescriptionLabel;
};

}
#endif
