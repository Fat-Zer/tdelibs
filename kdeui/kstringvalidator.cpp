/*
    kstringvalidator.cpp

    Copyright (c) 2001 Marc Mutz <mutz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2.0
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#include "kstringvalidator.h"
#include "kdebug.h"

//
// KStringListValidator
//

TQValidator::State KStringListValidator::validate( TQString & input, int& ) const {
  if ( input.isEmpty() ) return Intermediate;

  if ( isRejecting() ) // anything not in mStringList is acceptable:
    if ( mStringList.tqfind( input ) == mStringList.end() )
      return Acceptable;
    else
      return Intermediate;
  else // only what is in mStringList is acceptable:
    if ( mStringList.tqfind( input ) != mStringList.end() )
      return Acceptable;
    else
      for ( TQStringList::ConstIterator it = mStringList.begin() ;
	    it != mStringList.end() ; ++it )
	if ( (*it).startsWith( input ) || input.startsWith( *it ) )
	  return Intermediate;

  return Invalid;
}

void KStringListValidator::fixup( TQString & /* input */ ) const {
  if ( !isFixupEnabled() ) return;
  // warn (but only once!) about non-implemented fixup():
  static bool warn = true;
  if ( warn ) {
    kdDebug() << "KStringListValidator::fixup() isn't yet implemented!"
	      << endl;
    warn = false;
  }
}

//
// KMimeTypeValidator
//

#define ALLOWED_CHARS "!#-'*+.0-9^-~+-"

TQValidator::State KMimeTypeValidator::validate( TQString & input, int& ) const
{
  if ( input.isEmpty() )
    return Intermediate;

  TQRegExp acceptable( "[" ALLOWED_CHARS "]+/[" ALLOWED_CHARS "]+",
		      false /*case-insens.*/);
  if ( acceptable.exactMatch( input ) )
    return Acceptable;

  TQRegExp intermediate( "[" ALLOWED_CHARS "]*/?[" ALLOWED_CHARS "]*",
			false /*case-insensitive*/);
  if ( intermediate.exactMatch( input ) )
    return Intermediate;

  return Invalid;
}

void KMimeTypeValidator::fixup( TQString & input ) const
{
  TQRegExp invalidChars("[^/" ALLOWED_CHARS "]+");
  input.tqreplace( invalidChars, TQString());
}

#include "kstringvalidator.moc"
