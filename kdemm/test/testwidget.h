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

#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <tqwidget.h>

#include <kdemm/player.h>

class TQSlider;
class TQLabel;
class TQString;

namespace KDE
{
	namespace Multimedia
	{
		class Channel;
	}
}

using namespace KDE::Multimedia;

class TestWidget : public QWidget
{
	Q_OBJECT
	public:
		TestWidget();
	private slots:
		void v1changed( int );
		void v2changed( int );
		void tick( long );
		void stateChanged( KDE::Multimedia::Player::State );
		void seek( int );
		void length( long );
		void loadFile( const TQString & );

	private:
		TQSlider *m_v1slider, *m_v2slider, *m_seekslider;
		TQLabel *m_statelabel, *m_volumelabel1, *m_volumelabel2, *m_totaltime, *m_currenttime, *m_remainingtime;
		Channel * c;
		Player * po;
		bool m_ticking;
};

#endif // TESTWIDGET_H
// vim: sw=4 ts=4 noet
