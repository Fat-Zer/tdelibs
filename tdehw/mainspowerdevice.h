/* This file is part of the TDE libraries
   Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>
             (C) 2013 Golubev Alexander <fatzer2@gmail.com>

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

#ifndef _TDEMAINSPOWERDEVICE_H
#define _TDEMAINSPOWERDEVICE_H

#include "genericdevice.h"

namespace TDEHW {

class TDEHW_EXPORT MainsPowerDevice : public GenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		MainsPowerDevice(GenericDeviceType::GenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~MainsPowerDevice();

		/**
		* @return TRUE if power supply is online via mains power, FALSE if not
		*/
		bool online();

	protected:
		/**
		* @param TRUE if power supply is online via mains power, FALSE if not
		* @internal
		*/
		void internalSetOnline(bool vt);

	private:
		bool m_online;

	friend class HardwareDevices;
};

} //namespace TDEHW

#endif // _TDEMAINSPOWERDEVICE_H
