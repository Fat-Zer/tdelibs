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

#include "testwidget.h"

#include <kdemm/factory.h>
#include <kdemm/player.h>
#include <kdemm/channel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <tqslider.h>
#include <tqlayout.h>
#include <tqapplication.h>
#include <tqpushbutton.h>
#include <tqlabel.h>
#include <klineedit.h>
#include <kurlcompletion.h>

using namespace KDE::Multimedia;

TestWidget::TestWidget()
	: TQWidget( 0, 0 )
	, m_ticking( false )
{
	c = Factory::self()->createChannel( i18n( "KDE Multimedia testprogram" ) );
	if( ! c )
		kdFatal() << "couldn't create outputdevice" << endl;
	po = Factory::self()->createPlayer();
	if( ! po )
		kdFatal() << "couldn't create Player" << endl;

	if( ! po->setOutputChannel( c ) )
		kdFatal() << "PlayObject::setOutputChannel failed" << endl;

	if( ! po->state() == Player::NoMedia )
		kdFatal() << "PlayObject in wrong state 1" << endl;

	( new TQHBoxLayout( this ) )->setAutoAdd( true );

	m_v1slider = new TQSlider( this );
	m_v1slider->setRange( 0, 150 );
	m_v1slider->setValue( ( int )( 100 * po->volume() ) );
	connect( m_v1slider, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( v1changed( int ) ) );

	m_v2slider = new TQSlider( this );
	m_v2slider->setRange( 0, 150 );
	m_v2slider->setValue( ( int )( 100 * c->volume() ) );
	connect( m_v2slider, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( v2changed( int ) ) );

	TQFrame * frame = new TQFrame( this );
	( new TQVBoxLayout( frame ) )->setAutoAdd( true );

	m_seekslider = new TQSlider( frame );
	m_seekslider->setOrientation( Qt::Horizontal );
	connect( m_seekslider, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( seek( int ) ) );
	po->setTickInterval( 100 );
	connect( po, TQT_SIGNAL( tick( long ) ), TQT_SLOT( tick( long ) ) );
	connect( po, TQT_SIGNAL( length( long ) ), TQT_SLOT( length( long ) ) );

	m_statelabel = new TQLabel( frame );
	connect( po, TQT_SIGNAL( stateChanged( KDE::Multimedia::Player::State, KDE::Multimedia::Player::State ) ),
			TQT_SLOT( stateChanged( KDE::Multimedia::Player::State ) ) );
	stateChanged( po->state() );

	TQPushButton * pause = new TQPushButton( frame );
	pause->setText( "pause" );
	connect( pause, TQT_SIGNAL( clicked() ), po, TQT_SLOT( pause() ) );

	TQPushButton * play = new TQPushButton( frame );
	play->setText( "play" );
	connect( play, TQT_SIGNAL( clicked() ), po, TQT_SLOT( play() ) );

	TQPushButton * stop = new TQPushButton( frame );
	stop->setText( "stop" );
	connect( stop, TQT_SIGNAL( clicked() ), po, TQT_SLOT( stop() ) );

	KLineEdit * file = new KLineEdit( frame );
	file->setCompletionObject( new KURLCompletion( KURLCompletion::FileCompletion ) );
	connect( file, TQT_SIGNAL( returnPressed( const TQString & ) ), TQT_SLOT( loadFile( const TQString & ) ) );

	TQFrame * frame2 = new TQFrame( this );
	( new TQVBoxLayout( frame2 ) )->setAutoAdd( true );

	m_volumelabel1 = new TQLabel( frame2 );
	m_volumelabel1->setText( TQString::number( po->volume() ) );
	
	m_volumelabel2 = new TQLabel( frame2 );
	m_volumelabel2->setText( TQString::number( c->volume() ) );

	m_totaltime = new TQLabel( frame2 );
	m_totaltime->setText( TQString::number( po->totalTime() ) );
	
	m_currenttime = new TQLabel( frame2 );
	m_currenttime->setText( TQString::number( po->currentTime() ) );
	
	m_remainingtime = new TQLabel( frame2 );
	m_remainingtime->setText( TQString::number( po->remainingTime() ) );
	
	show();

	if( ! po->load( KURL( "/home/mkretz/Musik/qt23.mp3" ) ) )
		kdFatal() << "PlayObject::load failed" << endl;

	connect( po, TQT_SIGNAL( finished() ), qApp, TQT_SLOT( quit() ) );

}

void TestWidget::v1changed( int v )
{
	if( ! po->setVolume( ( ( float )v ) / 100 ) )
		kdDebug() << "setVolume1 failed" << endl;
	m_volumelabel1->setText( TQString::number( po->volume() ) );
}

void TestWidget::v2changed( int v )
{
	if( ! c->setVolume( ( ( float )v ) / 100 ) )
		kdDebug() << "setVolume2 failed" << endl;
	m_volumelabel2->setText( TQString::number( c->volume() ) );
}

void TestWidget::tick( long t )
{
	m_ticking = true;
	m_seekslider->setValue( t );
	m_currenttime->setText( TQString::number( po->currentTime() ) );
	m_remainingtime->setText( TQString::number( po->remainingTime() ) );
	m_ticking = false;
}

void TestWidget::stateChanged( Player::State newstate )
{
	switch( newstate )
	{
		case Player::NoMedia:
			m_statelabel->setText( "NoMedia" );
			break;
		case Player::Loading:
			m_statelabel->setText( "Loading" );
			break;
		case Player::Stopped:
			m_statelabel->setText( "Stopped" );
			break;
		case Player::Paused:
			m_statelabel->setText( "Paused" );
			break;
		case Player::Buffering:
			m_statelabel->setText( "Buffering" );
			break;
		case Player::Playing:
			m_statelabel->setText( "Playing" );
			break;
	}
}

void TestWidget::seek( int ms )
{
	if( ! m_ticking )
		po->seek( ms );
}

void TestWidget::length( long ms )
{
	m_seekslider->setRange( 0, ms );
	m_totaltime->setText( TQString::number( po->totalTime() ) );
	m_currenttime->setText( TQString::number( po->currentTime() ) );
	m_remainingtime->setText( TQString::number( po->remainingTime() ) );
}

void TestWidget::loadFile( const TQString & file )
{
	po->load( KURL( file ) );
}

#include "testwidget.moc"
// vim: sw=4 ts=4 noet
