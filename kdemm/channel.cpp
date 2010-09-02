/*  This file is part of the KDE project
    Copyright (C) 2004 Scott Wheeler <wheeler@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "channel.h"

#include <tqstring.h>

namespace KDE
{
namespace Multimedia
{

class Channel::Private
{
	public:
		TQString channelName;
		TQString channelType;
		Direction direction;
};

Channel::Channel( const TQString & channelName, const TQString & type, Direction direction,
		TQObject * parent, const char * name )
	: TQObject( parent, name )
{
	d = new Private;
	d->channelName = channelName;
	d->channelType = type;
	d->direction = direction;
}

Channel::~Channel()
{
	delete d;
}

TQString Channel::channelName() const
{
	return d->channelName;
}

TQString Channel::channelType() const
{
	return d->channelType;
}

Channel::Direction Channel::direction() const
{
	return d->direction;
}

}}
#include "channel.moc"
// vim: sw=4 ts=4 noet