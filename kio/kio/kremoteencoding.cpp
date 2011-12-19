/* This file is part of the KDE libraries
   Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>

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

#include <config.h>

#include <kdebug.h>
#include <kstringhandler.h>
#include "kremoteencoding.h"

KRemoteEncoding::KRemoteEncoding(const char *name)
  : codec(0L), d(0L)
{
  setEncoding(name);
}

KRemoteEncoding::~KRemoteEncoding()
{
  // delete d;		// not necessary yet
}

TQString KRemoteEncoding::decode(const TQCString& name) const
{
#ifdef CHECK_UTF8
  if (codec->mibEnum() == 106 && !KStringHandler::isUtf8(name))
    return TQString::fromLatin1(name);
#endif

  TQString result = codec->toUnicode(name);
  if (codec->fromUnicode(result) != name)
    // fallback in case of decoding failure
    return TQString::fromLatin1(name);

  return result;
}

TQCString KRemoteEncoding::encode(const TQString& name) const
{
  TQCString result = codec->fromUnicode(name);
  if (codec->toUnicode(result) != name)
    return name.latin1();

  return result;
}

TQCString KRemoteEncoding::encode(const KURL& url) const
{
  return encode(url.path());
}

TQCString KRemoteEncoding::directory(const KURL& url, bool ignore_trailing_slash) const
{
  TQString dir = url.directory(true, ignore_trailing_slash);

  return encode(dir);
}

TQCString KRemoteEncoding::fileName(const KURL& url) const
{
  return encode(url.fileName());
}

void KRemoteEncoding::setEncoding(const char *name)
{
  // don't delete codecs

  if (name)
    codec = TQTextCodec::codecForName(name);
  else
    codec = TQTextCodec::codecForMib( 106 ); // fallback to UTF-8

  if (codec == 0L)
    codec = TQTextCodec::codecForMib(1);

  kdDebug() << k_funcinfo << "setting encoding " << codec->name()
	    << " for name=" << name << endl;
}

void KRemoteEncoding::virtual_hook(int, void*)
{
}
