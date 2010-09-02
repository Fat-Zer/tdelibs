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

#include "statetest.h"

#include <kdemm/factory.h>
#include <kdemm/channel.h>
#include <kdemm/player.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kurl.h>
#include <cstdlib>

using namespace KDE::Multimedia;

kdbgstream& operator<<( kdbgstream & stream, const Player::State state )
{
	switch( state )
	{
		case Player::NoMedia:
			stream << "NoMedia";
			break;
		case Player::Loading:
			stream << "Loading";
			break;
		case Player::Stopped:
			stream << "Stopped";
			break;
		case Player::Playing:
			stream << "Playing";
			break;
		case Player::Buffering:
			stream << "Buffering";
			break;
		case Player::Paused:
			stream << "Paused";
			break;
	}

	return stream;
}

void StateTester::run( const KURL & url )
{
	/*
	check for correct states:

	- after construction:
	  NoMedia

	- load()
	  NoMedia -> NoMedia, (only? for remote files: Loading -> NoMedia, Stopped ), Stopped

	- play()
	  Stopped, Paused -> Playing (except: Stopped -> Stopped)

	- when playing:
	  Playing -> Buffering -> Playing

	- pause()
	  Playing -> Paused

	- stop()
	  Playing, Paused -> Stopped
	*/

	f = Factory::self();
	kdDebug() << "using backend: " << f->backendName() <<
		"\n Comment: " << f->backendComment() <<
		"\n Version: " << f->backendVersion() << endl;

	c = f->createChannel( "teststates" );
	p = f->createPlayer();
	p->setOutputChannel( c );
	connect( p, TQT_SIGNAL( stateChanged( KDE::Multimedia::Player::State, KDE::Multimedia::Player::State ) ),
			TQT_SLOT( stateChanged( KDE::Multimedia::Player::State, KDE::Multimedia::Player::State ) ) );
	connect( p, TQT_SIGNAL( finished() ), kapp, TQT_SLOT( quit() ) );

	if( p->state() != Player::NoMedia )
		kdDebug() << p->state() << " should be NoMedia" << endl;

	kdDebug() << "loading " << url << endl;

	if( ! p->load( url ) )
		kdDebug() << "load failed" << endl;
	if( p->state() == Player::Loading )
		kdDebug() << "wait until Player finished Loading" << endl;
	else if( p->state() == Player::Stopped )
		testplaying();
	else if( p->state() == Player::NoMedia )
	{
		kdDebug() << "could not load media. exiting." << endl;
		exit( 0 );
	}
}

void StateTester::stateChanged( Player::State newstate, Player::State oldstate )
{
	kdDebug() << "stateChanged( new = " << newstate << ", old = " << oldstate << " )" << endl;
	switch( oldstate )
	{
		case Player::NoMedia:
			switch( newstate )
			{
				case Player::NoMedia:
				case Player::Loading:
				case Player::Stopped:
					return;
				default:
					break;
			}
		case Player::Loading:
			switch( newstate )
			{
				case Player::NoMedia:
					return;
				case Player::Stopped:
					testplaying();
					return;
				default:
					break;
			}
		case Player::Stopped:
			switch( newstate )
			{
				case Player::Playing:
				case Player::Stopped:
					return;
				default:
					break;
			}
		case Player::Playing:
			switch( newstate )
			{
				case Player::Buffering:
					//testbuffering();
				case Player::Paused:
				case Player::Stopped:
					return;
				default:
					break;
			}
		case Player::Buffering:
			switch( newstate )
			{
				case Player::Playing:
				case Player::Stopped:
				case Player::Paused:
					return;
				default:
					break;
			}
		case Player::Paused:
			switch( newstate )
			{
				case Player::Playing:
				case Player::Stopped:
				case Player::Buffering:
					return;
				default:
					break;
			}
	}

	wrongStateChange();
}

void StateTester::testplaying()
{
	if( ! p->play() )
		kdDebug() << "play failed" << endl;
	if( p->state() == Player::Stopped )
	{
		kdDebug() << "could not play media. exiting." << endl;
		exit( 0 );
	}
	else if( p->state() == Player::Playing )
	{
		if( ! p->pause() )
		{
			kdDebug() << "pause failed" << endl;
			if( p->state() != Player::Playing )
				wrongStateChange();
		}
		else
		{
			if( p->state() != Player::Paused )
				wrongStateChange();
			if( ! p->play() )
			{
				kdDebug() << "play failed" << endl;
				if( p->state() != Player::Paused )
					wrongStateChange();
				kdError() << "what now? play shouldn't fail here" << endl;
				exit( 1 );
			}
			if( p->state() != Player::Playing )
				wrongStateChange();
		}
		// it's playing now
		if( ! p->stop() )
		{
			kdDebug() << "stop failed" << endl;
			if( p->state() != Player::Playing )
				wrongStateChange();
		}
		else
		{
			if( p->state() != Player::Stopped )
				wrongStateChange();
			if( ! p->play() )
			{
				kdDebug() << "play failed" << endl;
				if( p->state() != Player::Stopped )
					wrongStateChange();
				kdError() << "play shouldn't fail after it worked before. exiting." << endl;
				exit( 1 );
			}
		}
		// it's playing again
		if( ! p->pause() )
		{
			kdDebug() << "pause failed. exiting." << endl;
			exit( 1 );
		}
		if( p->state() != Player::Paused )
			wrongStateChange();
		if( ! p->stop() )
		{
			kdDebug() << "stop failed" << endl;
			if( p->state() != Player::Paused )
				wrongStateChange();
			exit( 1 );
		}
		if( p->state() != Player::Stopped )
			wrongStateChange();
		// do further checking, calling load again
		kdDebug() << "success! playing the last 1/5 of the file now and quit on the finished signal" << endl;
		p->play();
		p->seek( p->totalTime() * 4 / 5 );
	}
}

void StateTester::wrongStateChange()
{
	kdError() << "wrong state change in backend!" << endl;
	exit( 1 );
}

static const KCmdLineOptions options[] =
{
	  { "+url", I18N_NOOP( "media file to play" ), 0 },
	  KCmdLineLastOption // End of options.
};

int main( int argc, char ** argv )
{
	KAboutData about( "kdemmtest", "KDE Multimedia Test",
			"0.1", "Testprogram",
			KAboutData::License_LGPL, 0 );
	about.addAuthor( "Matthias Kretz", 0, "kretz@kde.org" );
	KCmdLineArgs::init( argc, argv, &about );
	KCmdLineArgs::addCmdLineOptions( options );
	KApplication app; // we need it for KTrader

	StateTester tester;
	if( KCmdLineArgs::parsedArgs()->count() > 0 )
		tester.run( KCmdLineArgs::parsedArgs()->url( 0 ) );
	else
	{
		KCmdLineArgs::usage();
		exit( 2 );
	}

	return app.exec();
}

#include "statetest.moc"

// vim: sw=4 ts=4 noet
