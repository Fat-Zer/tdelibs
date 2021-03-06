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

#include "picture.h"

using namespace TDEABC;

Picture::Picture()
  : mIntern( false )
{
}

Picture::Picture( const TQString &url )
  : mUrl( url ), mIntern( false )
{
}

Picture::Picture( const TQImage &data )
  : mData( data ), mIntern( true )
{
}

Picture::~Picture()
{
}

bool Picture::operator==( const Picture &p ) const
{
  if ( mIntern != p.mIntern ) return false;

  if ( mIntern ) {
    if ( mData != p.mData )
      return false;
  } else {
    if ( mUrl != p.mUrl )
      return false;
  }

  return true;
}

bool Picture::operator!=( const Picture &p ) const
{
  return !( p == *this );
}

void Picture::setUrl( const TQString &url )
{
  mUrl = url;
  mIntern = false;
}

void Picture::setData( const TQImage &data )
{
  mData = data;
  mIntern = true;
}

void Picture::setType( const TQString &type )
{
  mType = type;
}

bool Picture::isIntern() const
{
  return mIntern;
}

TQString Picture::url() const
{
  return mUrl;
}

TQImage Picture::data() const
{
  return mData;
}

TQString Picture::type() const
{
  return mType;
}

TQString Picture::asString() const
{
  if ( mIntern )
    return "intern picture";
  else
    return mUrl;
}

TQDataStream &TDEABC::operator<<( TQDataStream &s, const Picture &picture )
{
  return s << picture.mIntern << picture.mUrl << picture.mType;
//  return s << picture.mIntern << picture.mUrl << picture.mType << picture.mData;
}

TQDataStream &TDEABC::operator>>( TQDataStream &s, Picture &picture )
{
  s >> picture.mIntern >> picture.mUrl >> picture.mType;
//  s >> picture.mIntern >> picture.mUrl >> picture.mType >> picture.mData;
  return s;
}
