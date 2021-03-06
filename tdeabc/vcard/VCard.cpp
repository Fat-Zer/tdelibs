/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1998 Rik Hemsley rik@kde.org

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

#include <tqcstring.h>
#include <tqstrlist.h>

#include <VCardEntity.h>
#include <VCardVCard.h>
#include <VCardContentLine.h>
#include <VCardRToken.h>

#include <VCardDefines.h>

using namespace VCARD;

VCard::VCard()
	:	Entity()
{
    contentLineList_.setAutoDelete( true );
}

VCard::VCard(const VCard & x)
	:	Entity(x),
		group_(x.group_),
		contentLineList_(x.contentLineList_)
{
}

VCard::VCard(const TQCString & s)
	:	Entity(s)
{
}

	VCard &
VCard::operator = (VCard & x)
{
	if (*this == x) return *this;

	group_				= x.group();
	contentLineList_	= x.contentLineList_;

	Entity::operator = (x);
	return *this;
}

	VCard &
VCard::operator = (const TQCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
VCard::operator == (VCard & x)
{
	x.parse();
	return false;
}

VCard::~VCard()
{
}

	void
VCard::_parse()
{
	vDebug("parse() called");
	TQStrList l;

	RTokenise(strRep_, "\r\n", l);

	if (l.count() < 3) { // Invalid VCARD !
		vDebug("Invalid vcard");
		return;
	}

	// Get the first line
	TQCString beginLine = TQCString(l.at(0)).stripWhiteSpace();

	vDebug("Begin line == \"" + beginLine + "\"");

	// Remove extra blank lines
	while (TQCString(l.last()).isEmpty())
		l.remove(l.last());

	// Now we know this is the last line
	TQCString endLine = l.last();

	// Trash the first and last lines as we have seen them.
	l.remove(0u);
	l.remove(l.last());

	///////////////////////////////////////////////////////////////
	// FIRST LINE

	int split = beginLine.find(':');

	if (split == -1) { // invalid, no BEGIN
		vDebug("No split");
		return;
	}

	TQCString firstPart(beginLine.left(split));
	TQCString valuePart(beginLine.mid(split + 1));

	split = firstPart.find('.');

	if (split != -1) {
		group_		= firstPart.left(split);
		firstPart	= firstPart.right(firstPart.length() - split - 1);
	}

	if (tqstrnicmp(firstPart, "BEGIN", 5) != 0) { // No BEGIN !
		vDebug("No BEGIN");
		return;
	}

	if (tqstrnicmp(valuePart, "VCARD", 5) != 0) { // Not a vcard !
		vDebug("No VCARD");
		return;
	}

	///////////////////////////////////////////////////////////////
	// CONTENT LINES
	//
	vDebug("Content lines");

	// Handle folded lines.

	TQStrList refolded;

	TQStrListIterator it(l);

	TQCString cur;

	for (; it.current(); ++it) {

		cur = it.current();

		++it;

		while (
			it.current()		&&
			it.current()[0] == ' '	&&
			strlen(it.current()) != 1)
		{
			cur += it.current() + 1;
			++it;
		}

		--it;

		refolded.append(cur);
	}

	TQStrListIterator it2(refolded);

	for (; it2.current(); ++it2) {

		vDebug("New contentline using \"" + TQCString(it2.current()) + "\"");
		ContentLine * cl = new ContentLine(it2.current());

		cl->parse();

		contentLineList_.append(cl);
	}

	///////////////////////////////////////////////////////////////
	// LAST LINE

	split = endLine.find(':');

	if (split == -1) // invalid, no END
		return;

	firstPart = endLine.left(split);
	valuePart = endLine.right(firstPart.length() - split - 1);

	split = firstPart.find('.');

	if (split != -1) {
		group_		= firstPart.left(split);
		firstPart	= firstPart.right(firstPart.length() - split - 1);
	}

	if (tqstricmp(firstPart, "END") != 0) // No END !
		return;

	if (tqstricmp(valuePart, "VCARD") != 0) // Not a vcard !
		return;
}

	void
VCard::_assemble()
{
	vDebug("Assembling vcard");
	strRep_ = "BEGIN:VCARD\r\n";
	strRep_ += "VERSION:3.0\r\n";

	TQPtrListIterator<ContentLine> it(contentLineList_);

	for (; it.current(); ++it)
		strRep_ += it.current()->asString() + "\r\n";

	strRep_ += "END:VCARD\r\n";
}

	bool
VCard::has(EntityType t)
{
	parse();
	return contentLine(t) == 0 ? false : true;
}

	bool
VCard::has(const TQCString & s)
{
	parse();
	return contentLine(s) == 0 ? false : true;
}

	void
VCard::add(const ContentLine & cl)
{
	parse();
	ContentLine * c = new ContentLine(cl);
	contentLineList_.append(c);
}

	void
VCard::add(const TQCString & s)
{
	parse();
	ContentLine * c = new ContentLine(s);
	contentLineList_.append(c);
}

	ContentLine *
VCard::contentLine(EntityType t)
{
	parse();
	TQPtrListIterator<ContentLine> it(contentLineList_);

	for (; it.current(); ++it)
		if (it.current()->entityType() == t)
			return it.current();

	return 0;
}

	ContentLine *
VCard::contentLine(const TQCString & s)
{
	parse();
	TQPtrListIterator<ContentLine> it(contentLineList_);

	for (; it.current(); ++it)
		if (it.current()->entityType() == EntityNameToEntityType(s))
			return it.current();

	return 0;
}

