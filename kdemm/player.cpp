/*  This file is part of the KDE project
    Copyright (C) 2004 Matthias Kretz <kretz@kde.org>

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

#include "player.h"

namespace KDE {
namespace Multimedia {
Player::Player( TQObject * parent, const char * name )
	: TQObject( parent, name )
	, m_state( Player::NoMedia )
{
}

Player::~Player()
{
}

Player::State Player::state() const
{
	return m_state;
}

void Player::setState( Player::State newstate )
{
	if( m_state != newstate )
	{
		Player::State oldstate = m_state;
		m_state = newstate;
		emit stateChanged( newstate, oldstate );
	}
}

}} // namespaces

#include "player.moc"

// vim: sw=4 ts=4 noet
