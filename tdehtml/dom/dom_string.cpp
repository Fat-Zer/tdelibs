/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "dom/dom_string.h"
#include "xml/dom_stringimpl.h"


using namespace DOM;


DOMString::DOMString(const TQChar *str, uint len)
{
    impl = new DOMStringImpl( str, len );
    impl->ref();
}

DOMString::DOMString(const TQString &str)
{
    if (str.isNull()) {
	impl = 0;
	return;
    }

    impl = new DOMStringImpl( str.unicode(), str.length() );
    impl->ref();
}

DOMString::DOMString(const char *str)
{
    if (!str) {
	impl = 0;
	return;
    }

    impl = new DOMStringImpl( str );
    impl->ref();
}

DOMString::DOMString(DOMStringImpl *i)
{
    impl = i;
    if(impl) impl->ref();
}

DOMString::DOMString(const DOMString &other)
{
    impl = other.impl;
    if(impl) impl->ref();
}

DOMString::~DOMString()
{
    if(impl) impl->deref();
}

DOMString &DOMString::operator =(const DOMString &other)
{
    if ( impl != other.impl ) {
        if(impl) impl->deref();
        impl = other.impl;
        if(impl) impl->ref();
    }
    return *this;
}

DOMString &DOMString::operator += (const DOMString &str)
{
    if(!impl)
    {
	// ### FIXME!!!
	impl = str.impl;
	if (impl)
	     impl->ref();
	return *this;
    }
    if(str.impl)
    {
	DOMStringImpl *i = impl->copy();
	impl->deref();
	impl = i;
	impl->ref();
	impl->append(str.impl);
    }
    return *this;
}

DOMString DOMString::operator + (const DOMString &str)
{
    if(!impl) return str.copy();
    if(str.impl)
    {
	DOMString s = copy();
	s += str;
	return s;
    }

    return copy();
}

void DOMString::insert(DOMString str, uint pos)
{
    if(!impl)
    {
	impl = str.impl->copy();
	impl->ref();
    }
    else
	impl->insert(str.impl, pos);
}


const TQChar &DOMString::operator [](unsigned int i) const
{
    static const TQChar nullChar = 0;

    if(!impl || i >= impl->l ) return nullChar;

    return *(impl->s+i);
}

int DOMString::find(const TQChar c, int start) const
{
    unsigned int l = start;
    if(!impl || l >= impl->l ) return -1;
    while( l < impl->l )
    {
	if( *(impl->s+l) == c ) return l;
	l++;
    }
    return -1;
}

uint DOMString::length() const
{
    if(!impl) return 0;
    return impl->l;
}

void DOMString::truncate( unsigned int len )
{
    if(impl) impl->truncate(len);
}

void DOMString::remove(unsigned int pos, int len)
{
  if(impl) impl->remove(pos, len);
}

DOMString DOMString::split(unsigned int pos)
{
  if(!impl) return DOMString();
  return impl->split(pos);
}

DOMString DOMString::lower() const
{
  if(!impl) return DOMString();
  return impl->lower();
}

DOMString DOMString::upper() const
{
  if(!impl) return DOMString();
  return impl->upper();
}

bool DOMString::percentage(int &_percentage) const
{
    if(!impl || !impl->l) return false;

    if ( *(impl->s+impl->l-1) != TQChar('%'))
       return false;

    _percentage = TQConstString(impl->s, impl->l-1).string().toInt();
    return true;
}

TQChar *DOMString::unicode() const
{
    if(!impl) return 0;
    return impl->unicode();
}

TQString DOMString::string() const
{
    if(!impl) return TQString::null;

    return impl->string();
}

int DOMString::toInt() const
{
    if(!impl) return 0;

    return impl->toInt();
}

DOMString DOMString::copy() const
{
    if(!impl) return DOMString();
    return impl->copy();
}

// ------------------------------------------------------------------------

bool DOM::strcasecmp( const DOMString &as, const DOMString &bs )
{
    if ( as.length() != bs.length() ) return true;

    const TQChar *a = as.unicode();
    const TQChar *b = bs.unicode();
    if ( a == b )  return false;
    if ( !( a && b ) )  return true;
    int l = as.length();
    while ( l-- ) {
        if ( *a != *b && a->lower() != b->lower() ) return true;
	a++,b++;
    }
    return false;
}

bool DOM::strcasecmp( const DOMString &as, const char* bs )
{
    const TQChar *a = as.unicode();
    int l = as.length();
    if ( !bs ) return ( l != 0 );
    while ( l-- ) {
        if ( a->latin1() != *bs ) {
            char cc = ( ( *bs >= 'A' ) && ( *bs <= 'Z' ) ) ? ( ( *bs ) + 'a' - 'A' ) : ( *bs );
            if ( a->lower().latin1() != cc ) return true;
        }
        a++, bs++;
    }
    return ( *bs != '\0' );
}

bool DOMString::isEmpty() const
{
    return (!impl || impl->l == 0);
}

//-----------------------------------------------------------------------------

bool DOM::operator==( const DOMString &a, const DOMString &b )
{
    unsigned int l = a.length();

    if( l != b.length() ) return false;

    if(!memcmp(a.unicode(), b.unicode(), l*sizeof(TQChar)))
	return true;
    return false;
}

bool DOM::operator==( const DOMString &a, const TQString &b )
{
    unsigned int l = a.length();

    if( l != b.length() ) return false;

    if(!memcmp(a.unicode(), b.unicode(), l*sizeof(TQChar)))
	return true;
    return false;
}

bool DOM::operator==( const DOMString &a, const char *b )
{
    DOMStringImpl* aimpl = a.impl;
    if ( !b ) return !aimpl;

    if ( aimpl ) {
        int alen = aimpl->l;
        const TQChar *aptr = aimpl->s;
        while ( alen-- ) {
            unsigned char c = *b++;
            if ( !c || ( *aptr++ ).unicode() != c )
                return false;
        }
    }

    return !*b;
}
