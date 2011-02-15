/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <tqstrlist.h>

#include <VCardRToken.h>
#include <VCardNValue.h>
#include <VCardValue.h>
#include <VCardDefines.h>

using namespace VCARD;

NValue::NValue()
	:	Value()
{
	vDebug("ctor");
}

NValue::NValue(const NValue & x)
	:	Value(x),
		family_	(x.family_),
		given_	(x.given_),
		middle_	(x.middle_),
		prefix_	(x.prefix_),
		suffix_	(x.suffix_)
{
}

NValue::NValue(const TQCString & s)
	:	Value(s)
{
	vDebug("ctor");
}

	NValue &
NValue::operator = (NValue & x)
{
	if (*this == x) return *this;
	
	family_	= x.family_;
	given_	= x.given_;
	middle_	= x.middle_;
	prefix_	= x.prefix_;
	suffix_	= x.suffix_;

	Value::operator = (x);
	return *this;
}

	NValue &
NValue::operator = (const TQCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
NValue::operator == (NValue & x)
{
	x.parse();

	return (
		family_	== x.family_	&&
		given_	== x.given_	&&
		middle_	== x.middle_	&&
		prefix_	== x.prefix_	&&
		suffix_ == x.suffix_);
}

NValue::~NValue()
{
}

	NValue *
NValue::clone()
{
	return new NValue( *this );
}

	void
NValue::_parse()
{
	TQStrList l;
	RTokenise(strRep_, ";", l);
	
	for (unsigned int i = 0; i < l.count(); i++) {

		switch (i) {
			case 0: family_	= l.tqat(0);	break;
			case 1: given_	= l.tqat(1);	break;
			case 2: middle_	= l.tqat(2);	break;
			case 3: prefix_	= l.tqat(3);	break;
			case 4: suffix_	= l.tqat(4);	break;
			default:			break;
		}
	}
}

	void
NValue::_assemble()
{
	strRep_ =		family_;
	strRep_ += ";" +	given_;
	strRep_ += ";" +	middle_;
	strRep_ += ";" +	prefix_;
	strRep_ += ";" +	suffix_;
}

