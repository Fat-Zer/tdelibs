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

#ifndef SIMPLEPLAYER_H
#define SIMPLEPLAYER_H

#include <kdemm/factory.h>
#include <kdemm/channel.h>
#include <kdemm/player.h>
#include <tqobject.h>

class KURL;

namespace KDE
{
namespace Multimedia
{

class KDE_EXPORT SimplePlayer : public TQObject
{
	Q_OBJECT
	public:
		SimplePlayer( TQObject * parent = 0, const char * name = 0 );
		~SimplePlayer();

		void play( const KURL & url );
		void pause();
		void stop();

		long totalTime() const;
		long currentTime() const;
		void seek( long ms );

		float volume() const;
		void setVolume( float volume );

		bool isPlaying() const;
		bool isPaused() const;

	signals:
		void finished();

	private slots:
		void stateChanged( KDE::Multimedia::Player::State, KDE::Multimedia::Player::State );
		void deleteYourObjects();
		void recreateObjects();

	private:
		class Private;
		Private * d;
};

}} // namespaces

#endif // SIMPLEPLAYER_H
// vim: sw=4 ts=4 noet tw=80
