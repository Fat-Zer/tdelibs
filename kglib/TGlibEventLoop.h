/*

Copyright (C) 2011 Serghei Amelian <serghei.amelian@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/

#ifndef _TGLIBEVENTLOOP_H_
#define _TGLIBEVENTLOOP_H_

#include <tqeventloop.h>


#ifdef USE_QT3
	#define GLIB_EVENT_LOOP TGlibEventLoop glibEventLoop;
#else
	#define GLIB_EVENT_LOOP
#endif


class TGlibEventLoop : public TQEventLoop {
public:
	TGlibEventLoop(TQObject *parent = 0, const char *name = 0);
	~TGlibEventLoop();

	virtual bool processEvents(ProcessEventsFlags flags);
};


#endif
