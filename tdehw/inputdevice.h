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

#ifndef _TDEINPUTDEVICE_H
#define _TDEINPUTDEVICE_H

#include "genericdevice.h"

namespace TDEHW {

namespace InputDeviceType {
enum InputDeviceType {
	Unknown,
	ACPILidSwitch,
	ACPISleepButton,
	ACPIPowerButton,
	Other = 0x80000000
};
};

class TDEHW_EXPORT InputDevice : public GenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		InputDevice(GenericDeviceType::GenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~InputDevice();

		/**
		* @return a InputDeviceType::InputDeviceType with the input device type, if known
		*/
		InputDeviceType::InputDeviceType inputType();

	protected:
		/**
		* @param it a InputDeviceType::InputDeviceType with the input device type, if known
		* @internal
		*/
		void internalSetInputType(InputDeviceType::InputDeviceType it);

	private:
		InputDeviceType::InputDeviceType m_inputType;

	friend class HardwareDevices;
};

} //namespace TDEHW

#endif // _TDEINPUTDEVICE_H
