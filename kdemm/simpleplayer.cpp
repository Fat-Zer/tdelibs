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

#include "simpleplayer.h"
#include <kurl.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>

namespace KDE
{
namespace Multimedia
{

class SimplePlayer::Private
{
	public:
		Channel * channel;
		Player * player;

		KURL url;
		Player::State state;
		float channelvolume;
		long time;
		TQString title;
		TQString type;
};

SimplePlayer::SimplePlayer( TQObject * parent, const char * name )
	: TQObject( parent, name )
	, d( new Private )
{
	connect( Factory::self(), TQT_SIGNAL( deleteYourObjects() ), TQT_SLOT( deleteYourObjects() ) );
	connect( Factory::self(), TQT_SIGNAL( recreateObjects() ), TQT_SLOT( recreateObjects() ) );
	d->channel = Factory::self()->createChannel( KGlobal::instance()->aboutData()->programName() );
	d->player = Factory::self()->createPlayer();
	if ( d->player ) {
  		d->player->setOutputChannel( d->channel );
		connect( d->player, TQT_SIGNAL( stateChanged( KDE::Multimedia::Player::State, KDE::Multimedia::Player::State ) ),
			TQT_SLOT( stateChanged( KDE::Multimedia::Player::State, KDE::Multimedia::Player::State ) ) );
		connect( d->player, TQT_SIGNAL( finished() ), TQT_SIGNAL( finished() ) );
	};
}

SimplePlayer::~SimplePlayer()
{
	delete d->player;
	delete d->channel;
}

void SimplePlayer::play( const KURL & url )
{
	if( ! d->player )
		return;
	if( isPaused() && url == d->url )
	{
		d->player->play();
		return;
	}
	if( ! d->player->load( url ) )
		return;
	d->url = url;
	if( d->player->state() == Player::Stopped )
		d->player->play();
}

void SimplePlayer::pause()
{
	if( ! d->player )
		return;
	d->player->pause();
}

void SimplePlayer::stop()
{
	if( ! d->player )
		return;
	d->player->stop();
}

long SimplePlayer::totalTime() const
{
	if( ! d->player )
		return 0;
	return d->player->totalTime();
}

long SimplePlayer::currentTime() const
{
	if( ! d->player )
		return 0;
	return d->player->currentTime();
}

void SimplePlayer::seek( long ms )
{
	if( ! d->player )
		return;
	d->player->seek( ms );
}

float SimplePlayer::volume() const
{
	if( ! d->player )
		return 0;
	return d->channel->volume();
}

void SimplePlayer::setVolume( float v )
{
	if( ! d->player )
		return;
	d->channel->setVolume( v );
}

bool SimplePlayer::isPlaying() const
{
	if( ! d->player )
		return false;
	return ( d->player->state() == Player::Playing );
}

bool SimplePlayer::isPaused() const
{
	if( ! d->player )
		return false;
	return ( d->player->state() == Player::Paused );
}

void SimplePlayer::stateChanged( Player::State ns, Player::State os )
{
	if( ! d->player )
		return;
	if( os == Player::Loading && ns == Player::Stopped )
		d->player->play();
}

void SimplePlayer::deleteYourObjects()
{
	d->state = d->player->state();
	d->channelvolume = d->channel->volume();
	d->time = d->player->currentTime();
	d->title = d->channel->channelName();
	d->type = d->channel->channelType();

	if( d->player )
		d->player->stop();

	delete d->player;
	delete d->channel;
	d->player = 0;
	d->channel = 0;
}

void SimplePlayer::recreateObjects()
{
	d->channel = Factory::self()->createChannel( d->title, d->type );
	d->channel->setVolume( d->channelvolume );

	d->player = Factory::self()->createPlayer();
	if( ! d->player )
		return;

	d->player->setOutputChannel( d->channel );

	if( d->state != Player::NoMedia )
		if( ! d->player->load( d->url ) )
			return;
	if( d->state == Player::Playing || d->state == Player::Paused || d->state == Player::Buffering )
	{
		d->player->play();
		d->player->seek( d->time );
		if( d->state == Player::Paused )
			d->player->pause();
	}
}

}} // namespaces

#include "simpleplayer.moc"

// vim: sw=4 ts=4 noet
