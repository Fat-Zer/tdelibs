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

#ifndef KDEMM_CHANNEL_H
#define KDEMM_CHANNEL_H

#include <kdemm/mixeriface.h>

class TQString;

namespace KDE
{
namespace Multimedia
{
	/**
	 * \short Abstraction of different inputs or outputs
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class KDE_EXPORT Channel : public TQObject, virtual public MixerIface
	{
		Q_OBJECT
		public:
			virtual ~Channel();

			enum Direction
			{
				Input,
				Output
			};

			/**
			 * The user visible name of the channel
			 */
			virtual TQString channelName() const;

			/**
			 * The (internal) channel type.
			 */
			virtual TQString channelType() const;

			/**
			 * Whether it is an Input or Output channel
			 */
			virtual Direction direction() const;

			/**
			 * If the channel has a volume control this function returns \p true
			 */
			virtual bool hasVolumeControl() const = 0;
			
			/**
			 * The current volume of the channel
			 */
			virtual float volume() const = 0;

			/**
			 * Set the volume for the channel
			 *
			 * \returns Returns \p true if the volume has been successfully set
			 */
			virtual bool setVolume( float volume ) = 0;

		protected:
			/**
			 * You can not instantiate channels yourself, use the Factory to
			 * create them.
			 */
			Channel( const TQString & channelName, const TQString & type, Direction direction,
					TQObject * parent = 0, const char * name = 0 );

		private:
			class Private;
			Private * d;
	};
}} // namespaces
// vim: sw=4 ts=4 noet tw=80

#endif // KDEMM_CHANNEL_H
