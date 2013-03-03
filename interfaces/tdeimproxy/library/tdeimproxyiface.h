/*    
	tdeimproxyiface.cpp
	
	IM service library for KDE
	
	DCOP interface to allow us to receive DCOP signals

	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
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

#ifndef KIMPROXYIFACE_H
#define KIMPROXYIFACE_H

#include <dcopobject.h>

class KIMProxyIface : virtual public DCOPObject
{
	K_DCOP
	k_dcop:
		virtual void contactPresenceChanged( TQString uid, TQCString appId, int presence ) = 0;
};

#endif