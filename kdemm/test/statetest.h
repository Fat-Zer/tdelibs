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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef STATETEST_H
#define STATETEST_H

#include <tqobject.h>

#include <kdemm/player.h>

class KURL;

namespace KDE
{
	namespace Multimedia
	{
		class Factory;
		class Channel;
	}
}

class StateTester : public QObject
{
	Q_OBJECT
	public:
		void run( const KURL & );

	private slots:
		void stateChanged( KDE::Multimedia::Player::State, KDE::Multimedia::Player::State );

	private:
		void testplaying();
		void wrongStateChange();

		KDE::Multimedia::Factory * f;
		KDE::Multimedia::Channel * c;
		KDE::Multimedia::Player * p;
};

#endif // STATETEST_H
// vim: sw=4 ts=4 noet
