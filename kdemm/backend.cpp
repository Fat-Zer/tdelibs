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

#include "backend.h"
#include "player.h"

#include <tqstringlist.h>

#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>

namespace KDE
{
namespace Multimedia
{

class Backend::Private
{
	public:
		Channel * channel;
};

Backend::Backend( TQObject * parent, const char * name )
	: TQObject( parent, name )
	, d( 0 )
{
}

Backend::~Backend()
{
	delete d;
}

bool Backend::playSoundEvent( const KURL & url )
{
	if( ! d )
	{
		d = new Private;

		TQString ctype = "notifications";
		if( availableChannels( Channel::Output ).contains( ctype ) < 1 )
			ctype = "default";
		d->channel = createChannel( KGlobal::instance()->aboutData()->programName(), ctype, Channel::Output );
	}

	Player * player = createPlayer();
	player->setOutputChannel( d->channel );
	connect( player, TQT_SIGNAL( finished() ), player, TQT_SLOT( deleteLater() ) );

	if( player->load( url ) )
		if( player->play() )
			return true;
	delete player;
	return false;
}

}} // namespaces

#include "backend.moc"

// vim: sw=4 ts=4 noet
