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

#ifndef VOLUMECONTROLD_H
#define VOLUMECONTROLD_H

#include <kded/kdedmodule.h>
#include <tqstringlist.h>

class VolumeControlD : public KDEDModule
{
	Q_OBJECT
	K_DCOP
	public:
		VolumeControlD(const TQCString &name);
		virtual ~VolumeControlD();

	k_dcop:
		// think of some better API here...
		TQStringList availableChannels() const;
		TQStringList openConnectionsToChannel( const TQString & channel ) const;
		float volume( const TQString & connection, const TQString & channel ) const;
		void setVolume( const TQString & connection, const TQString & channel, float volume );

	private slots:
		/**
		 * When an app with a registered Channel goes down we want to notice...
		 */
		void applicationRemoved( const TQCString & );

	private:
		class Private;
		Private * d;
};

// vim: sw=4 ts=4 tw=80 noet
#endif // VOLUMECONTROLD_H
