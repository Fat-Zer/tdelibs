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
#include <tqtimer.h>
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
	Disk,
	StorageController,
	Mouse,
	Keyboard,
	HID,
	Network,
	Printer,
	Scanner,
	Sound,
	IEEE1394,
	Camera,
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

namespace TDEDiskDeviceType {
enum TDEDiskDeviceType {
	Floppy =	0x00000002,
	CDROM =		0x00000004,
	CDRW =		0x00000008,
	DVDROM =	0x00000010,
	DVDRAM =	0x00000020,
	DVDRW =		0x00000040,
	BDROM =		0x00000080,
	BDRW =		0x00000100,
	Zip =		0x00000200,
	Jaz =		0x00000400,
	Camera =	0x00000800,
	LUKS =		0x00001000,
	OtherCrypted =	0x00002000,
	CDAudio =	0x00004000,
	CDVideo =	0x00008000,
	DVDVideo =	0x00010000,
	BDVideo =	0x00020000,
	Flash =		0x00040000,
	USB =		0x00080000,
	Tape =		0x00100000,
	HDD =		0x00200000,
	Optical =	0x00400000,
	RAM =		0x00800000,
	Loop =		0x01000000,
	CompactFlash =	0x02000000,
	MemoryStick =	0x04000000,
	SmartMedia =	0x08000000,
	SDMMC =		0x10000000,
	Other =		0x80000000
};

inline TDEDiskDeviceType operator|(TDEDiskDeviceType a, TDEDiskDeviceType b)
{
	return static_cast<TDEDiskDeviceType>(static_cast<int>(a) | static_cast<int>(b));
}

inline TDEDiskDeviceType operator&(TDEDiskDeviceType a, TDEDiskDeviceType b)
{
	return static_cast<TDEDiskDeviceType>(static_cast<int>(a) & static_cast<int>(b));
}

inline TDEDiskDeviceType operator~(TDEDiskDeviceType a)
{
	return static_cast<TDEDiskDeviceType>(~static_cast<int>(a));
}
};

namespace TDEDiskDeviceStatus {
enum TDEDiskDeviceStatus {
	Mountable =	0x00000001,
	Removable =	0x00000002,
	Inserted =	0x00000004,
	Blank =		0x00000008,
	Other =		0x80000000
};

inline TDEDiskDeviceStatus operator|(TDEDiskDeviceStatus a, TDEDiskDeviceStatus b)
{
	return static_cast<TDEDiskDeviceStatus>(static_cast<int>(a) | static_cast<int>(b));
}

inline TDEDiskDeviceStatus operator&(TDEDiskDeviceStatus a, TDEDiskDeviceStatus b)
{
	return static_cast<TDEDiskDeviceStatus>(static_cast<int>(a) & static_cast<int>(b));
}

inline TDEDiskDeviceStatus operator~(TDEDiskDeviceStatus a)
{
	return static_cast<TDEDiskDeviceStatus>(~static_cast<int>(a));
}
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
		* @return a TQString with the vendor name, if any
		*/
		TQString &vendorName();

		/**
		* @param a TQString with the vendor name, if any
		*/
		void setVendorName(TQString vn);

		/**
		* @return a TQString with the vendor model, if any
		*/
		TQString &vendorModel();

		/**
		* @param a TQString with the vendor model, if any
		*/
		void setVendorModel(TQString vm);

		/**
		* @return a TQString with the device bus name, if any
		*/
		TQString &deviceBus();

		/**
		* @param a TQString with the device bus name, if any
		*/
		void setDeviceBus(TQString db);

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
		TQString m_vendorName;
		TQString m_vendorModel;
		TQString m_deviceBus;
};

class TDECORE_EXPORT TDEStorageDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEStorageDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEStorageDevice();

		/**
		* @return a TQString with the disk or partition label, if any
		*/
		TQString &diskLabel();

		/**
		* @param a TQString with the disk or partition label, if any
		*/
		void setDiskLabel(TQString dn);

		/**
		* @return a TQString with the disk UUID, if any
		*/
		TQString &diskUUID();

		/**
		* @param a TQString with the disk UUID, if any
		*/
		void setDiskUUID(TQString id);

		/**
		* @return an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		*/
		TDEDiskDeviceType::TDEDiskDeviceType diskType();

		/**
		* @param an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		*/
		void setDiskType(TDEDiskDeviceType::TDEDiskDeviceType tf);

		/**
		* @return an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		*/
		TDEDiskDeviceStatus::TDEDiskDeviceStatus diskStatus();

		/**
		* @param an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		*/
		void setDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus st);

		/**
		* @return true if media inserted, false if no media available
		*/
		bool mediaInserted();

		/**
		* @param a bool with the media status
		*/
		void setMediaInserted(bool inserted);

		/**
		* @return a TQString with the filesystem name, if any
		*/
		TQString &fileSystemName();

		/**
		* @param a TQString with the filesystem name, if any
		*/
		void setFileSystemName(TQString fn);

		/**
		* @return a TQString with the filesystem usage string, if any
		*/
		TQString &fileSystemUsage();

		/**
		* @param a TQString with the filesystem usage string, if any
		*/
		void setFileSystemUsage(TQString fu);

		/**
		* @return a TQString with the mount path, if mounted
		*/
		TQString mountPath();

	private:
		TDEDiskDeviceType::TDEDiskDeviceType m_diskType;
		TDEDiskDeviceStatus::TDEDiskDeviceStatus m_diskStatus;
		TQString m_diskName;
		TQString m_diskUUID;
		TQString m_fileSystemName;
		TQString m_fileSystemUsage;
		bool m_mediaInserted;
		TQString m_mountPath;
};

typedef TQPtrList<TDEGenericDevice> TDEGenericHardwareList;

class TDECORE_EXPORT TDEHardwareDevices : TQObject
{
	Q_OBJECT

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

	signals:
		void hardwareAdded(TDEGenericDevice);
		void hardwareRemoved(TDEGenericDevice);
		void mountTableModified();

	private slots:
		void checkForHotPluggedHardware();
		void checkForModifiedMounts();

	private:
		TDEGenericDevice *classifyUnknownDevice(udev_device* dev);

		struct udev *m_udevStruct;
		struct udev_monitor *m_udevMonitorStruct;
		TDEGenericHardwareList m_deviceList;

		TQTimer* m_devScanTimer;
		TQTimer* m_mountScanTimer;
};

#endif