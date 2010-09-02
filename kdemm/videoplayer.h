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

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "player.h"

namespace KDE
{
namespace Multimedia
{

class VideoPlayer : public Player
{
	Q_OBJECT
	public:
		/**
		 * Create a widget that will show the video.
		 *
		 * \param parent The parent to use for the video widget
		 * \param name The TQObject name for the widget
		 *
		 * \returns Returns the video widget.
		 */
		virtual TQWidget * createVideoWidget( TQWidget * parent, const char * name ) = 0;

	protected:
		/**
		 * You can not instantiate players yourself, use the Factory to
		 * create them.
		 */
		VideoPlayer( TQObject * parent, const char * name );
};

}} //namespaces
#endif // VIDEOPLAYER_H
// vim: sw=4 ts=4 tw=80 noet
