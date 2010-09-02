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

#ifndef BACKEND_H
#define BACKEND_H

#include <tqobject.h>
#include <kdemacros.h>
#include <kdemm/channel.h>
#include <kurl.h>

namespace KDE
{
namespace Multimedia
{
	class Player;
	class VideoPlayer;
	/**
	 * \brief Base class for all KDE Multimedia Backends
	 *
	 * This class provides the interface for Multimedia Backends. Use it to get
	 * a pointer to a new Player or VideoWidget. There are some reserved
	 * functions for later extension of the class.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class Backend : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * default TQObject constructor
			 */
			Backend( TQObject * parent = 0, const char * name = 0 );
			virtual ~Backend();

			/**
			 * @return A new instance to a Player from the Backend.
			 */
			virtual Player * createPlayer() = 0;

			/**
			 * Create a new VideoPlayer object. The Backend does not have to
			 * implement this function.
			 *
			 * @return A new instance to a VideoPlayer from the Backend. Or 0 if
			 * the Backend does not support the VideoPlayer interface.
			 */
			virtual VideoPlayer * createVideoPlayer() { return 0; }

			virtual bool playSoundEvent(const KURL & url);

			virtual Channel * createChannel( const TQString & title, const TQString & channeltype,
					Channel::Direction direction ) = 0;

			virtual TQStringList availableChannels( Channel::Direction direction ) const = 0;

			virtual TQStringList playableMimeTypes() const = 0;

		private:
			RESERVE_VIRTUAL_10

			class Private;
			Private * d;
	};
}} // namespaces

// vim: sw=4 ts=4 noet tw=80
#endif // BACKEND_H
