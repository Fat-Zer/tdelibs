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

#include "volumecontrold.h"

#include <kapplication.h>
#include <dcopclient.h>

extern "C"
{
	KDEDModule * create_volumecontrold( const TQCString &name )
	{
		return new VolumeControlD( name );
	}
}

class VolumeControlD::Private
{
	public:
};

VolumeControlD::VolumeControlD( const TQCString & name )
	: KDEDModule( name )
{
	connect( KApplication::dcopClient(), TQT_SIGNAL( applicationRemoved( const TQCString & ) ),
		TQT_SLOT( applicationRemoved( const TQCString& ) ) );
}

#include "volumecontrold.moc"

// vim: sw=4 ts=4 noet
