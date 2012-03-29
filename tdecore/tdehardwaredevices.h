/* This file is part of the TDE libraries
   Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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
#ifndef _TDEHARDWAREDEVICES_H
#define _TDEHARDWAREDEVICES_H

// TDE includes
#include <tqstring.h>
#include <tqptrlist.h>
#include "tdelibs_export.h"

// udev includes
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

/**
 * Hardware Device Access and Monitoring Library
 *
 * @author Timothy Pearson
 */

namespace TDEGenericDeviceType {
enum TDEGenericDeviceType {
	CPU,
	GPU,
	RAM,
	Mainboard,
	FixedDisk,
	RemovableDisk,
	StorageController,
	HID,
	Network,
	Printer,
	Scanner,
	TextIO,
	Peripheral,
	Battery,
	Power,
	ThermalSensor,
	ThermalControl,
	OtherACPI,
	OtherUSB,
	OtherPeripheral,
	OtherSensor,
	Other
};
};

class TDECORE_EXPORT TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEGenericDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEGenericDevice();

		/**
		* @return a TDEGenericDeviceType::TDEGenericDeviceType specifying the device type
		*/
		TDEGenericDeviceType::TDEGenericDeviceType type();

		/**
		* @return a TQString with the device name, if any
		*/
		TQString &name();

		/**
		* @param a TQString with the device name, if any
		*/
		void setName(TQString dn);

		/**
		* @return a TQString with the system path, if any
		*
		* This method is non-portable, so be careful!
		*/
		TQString &systemPath();

		/**
		* @param a TQString with the system path, if any
		*
		* This method is non-portable, so be careful!
		*/
		void setSystemPath(TQString sp);

		/**
		* @return a TQString with the system device node, if any
		*
		* This method is non-portable, so be careful!
		*/
		TQString &deviceNode();

		/**
		* @param a TQString with the system device node, if any
		*
		* This method is non-portable, so be careful!
		*/
		void setDeviceNode(TQString sn);

	private:
		TDEGenericDeviceType::TDEGenericDeviceType m_deviceType;
		TQString m_deviceName;
		TQString m_systemPath;
		TQString m_deviceNode;
};

typedef TQPtrList<TDEGenericDevice> TDEGenericHardwareList;

class TDECORE_EXPORT TDEHardwareDevices
{
	public:	
		/**
		*  Constructor.
		*/
		TDEHardwareDevices();
		
		/**
		* Destructor.
		*/
		~TDEHardwareDevices();

		/**
		*  Query all hardware capabilities on all devices
		*  This does not normally need to be called by an application, as
		*  device detection is handled internally and automatically
		*  
		*  A call to this method immediately invalidates any TDEGenericHardwareList
		*  structures returned by listAllPhysicalDevices()
		*  
		*  @return TRUE if successful
		*/
		bool queryHardwareInformation();

		/**
		*  List all hardware capabilities on all devices
		*  @return TQPtrList<TDEGenericDevice> containing all known hardware devices
		*/
		TQPtrList<TDEGenericDevice> &listAllPhysicalDevices();

	private:
		struct udev *m_udevStruct;
		TDEGenericHardwareList m_deviceList;
};

#endif