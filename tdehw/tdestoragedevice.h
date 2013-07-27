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

#ifndef _TDESTORAGEDEVICE_H
#define _TDESTORAGEDEVICE_H

#include "tdegenericdevice.h"

// Keep readDiskDeviceSubtypeFromString() in tdehardwaredevices.cpp in sync with this enum
namespace TDEDiskDeviceType {
enum TDEDiskDeviceType {
	Null =		0x00000000,
	MediaDevice =	0x00000001,
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
	UnlockedCrypt =	0x20000000,
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
	Null =			0x00000000,
	Mountable =		0x00000001,
	Removable =		0x00000002,
	Inserted =		0x00000004,
	Blank =			0x00000008,
	UsedByDevice =		0x00000010,
	UsesDevice =		0x00000020,
	ContainsFilesystem =	0x00000040,
	Hotpluggable =		0x00000080,
	Other =			0x80000000
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

class TDEHW_EXPORT TDEStorageDevice : public TDEGenericDevice
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
		TQString diskLabel();

		/**
		* @return a TQString with the disk UUID, if any
		*/
		TQString diskUUID();

		/**
		* @return an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		*/
		TDEDiskDeviceType::TDEDiskDeviceType diskType();

		/**
		* @return an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		*/
		TDEDiskDeviceStatus::TDEDiskDeviceStatus diskStatus();

		/**
		* @return true if media inserted, false if no media available
		*/
		bool mediaInserted();

		/**
		* @return a TQString with the filesystem name, if any
		*/
		TQString fileSystemName();

		/**
		* @return a TQString with the filesystem usage string, if any
		*/
		TQString fileSystemUsage();

		/**
		* @return a TQStringList containing system paths to all devices with a lock on this device, if any
		*/
		TQStringList holdingDevices();

		/**
		* @return a TQStringList containing system paths to all devices locked by this device, if any
		*/
		TQStringList slaveDevices();

		/**
		* Mounts the device if not encrypted
		*
		* @param a TQString containing a requested mount name under /media, if desired
		* @param a TQString containing any mount options for pmount, if desired
		* @param a pointer to a TQString which will be populated with any error messages from pmount, if desired
		* @param a pointer to an integer which will be populated with the return code from pmount, if desired
		*
		* @return a TQString with the mount path, if successful
		*/
		TQString mountDevice(TQString mediaName=TQString::null, TQString mountOptions=TQString::null, TQString* errRet=0, int* retcode=0);

		/**
		* Mounts the encrypted device if the correct passphrase is given
		*
		* @param a TQString containing the passphrase
		* @param a TQString containing a requested mount name under /media, if desired
		* @param a TQString containing any mount options for pmount, if desired
		* @param a pointer to a TQString which will be populated with any error messages from pmount, if desired
		* @param a pointer to an integer which will be populated with the return code from pmount, if desired
		*
		* @return a TQString with the mount path, if successful
		*/
		TQString mountEncryptedDevice(TQString passphrase, TQString mediaName=TQString::null, TQString mountOptions=TQString::null, TQString* errRet=0, int* retcode=0);

		/**
		* Unmounts the device
		*
		* @param a pointer to a TQString which will be populated with any error messages from pmount, if desired
		* @param a pointer to an integer which will be populated with the return code from pmount, if desired
		*
		* @return TRUE if unmount was successful
		*/
		bool unmountDevice(TQString* errRet, int* retcode=0);

		/**
		* @return a TQString with the mount path, if mounted
		*/
		TQString mountPath();

		/**
		* @return an unsigned long with the device size in bytes
		*/
		unsigned long long deviceSize();

		/**
		* @return a TQString with the device size in human readable form
		*/
		TQString deviceFriendlySize();

		/**
		* Get an icon for this device
		* @param size a TDEIcon::StdSizes structure specifying the desired icon size
		* @return a TQPixmap containing the icon for the specified type
		*
		* This method overrides TDEGenericDevice::icon(TDEIcon::StdSizes size)
		*/
		TQPixmap icon(TDEIcon::StdSizes size);

		/**
		* @return a TQString with a friendly name
		*
		* This method overrides TDEGenericDevice::friendlyName()
		*/
		TQString friendlyName();

		/**
		*  @return a TQString containing the friendly type name
		*
		* This method overrides TDEGenericDevice::friendlyDeviceType()
		*/
		TQString friendlyDeviceType();

		/**
		* @param an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		*/
		bool isDiskOfType(TDEDiskDeviceType::TDEDiskDeviceType tf);

		/**
		* @param an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		*/
		bool checkDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus sf);

		/**
		* @param TRUE to engage media lock, FALSE to disable it
		* @return TRUE on success, FALSE on failure
		*
		* This method currently works on CD-ROM drives and similar devices
		*/
		bool lockDriveMedia(bool lock);

		/**
		* @return TRUE on success, FALSE on failure
		*
		* This method currently works on CD-ROM drives and similar devices
		*/
		bool ejectDriveMedia();

		/**
		* @return TRUE on success, FALSE on failure
		*
		* This method currently works on all removable storage devices
		*/
		bool ejectDrive();

	protected:
		/**
		* @param a TQString with the disk or partition label, if any
		* @internal
		*/
		void internalSetDiskLabel(TQString dn);

		/**
		* @param a TQString with the disk UUID, if any
		* @internal
		*/
		void internalSetDiskUUID(TQString id);

		/**
		* @param an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		* @internal
		*/
		void internalSetDiskType(TDEDiskDeviceType::TDEDiskDeviceType tf);

		/**
		* @param an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		* @internal
		*/
		void internalSetDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus st);

		/**
		* @param a bool with the media status
		* @internal
		*/
		void internalSetMediaInserted(bool inserted);

		/**
		* @param a TQString with the filesystem name, if any
		* @internal
		*/
		void internalSetFileSystemName(TQString fn);

		/**
		* @param a TQString with the filesystem usage string, if any
		* @internal
		*/
		void internalSetFileSystemUsage(TQString fu);

		/**
		* @param a TQStringList containing system paths to all devices with a lock on this device, if any
		* @internal
		*/
		void internalSetHoldingDevices(TQStringList hd);

		/**
		* @param a TQStringList containing system paths to all devices locked by this device, if any
		* @internal
		*/
		void internalSetSlaveDevices(TQStringList sd);

	private:
		TDEDiskDeviceType::TDEDiskDeviceType m_diskType;
		TDEDiskDeviceStatus::TDEDiskDeviceStatus m_diskStatus;
		TQString m_diskName;
		TQString m_diskUUID;
		TQString m_fileSystemName;
		TQString m_fileSystemUsage;
		bool m_mediaInserted;
		TQString m_mountPath;
		TQStringList m_holdingDevices;
		TQStringList m_slaveDevices;

	friend class TDEHardwareDevices;
};

#endif // _TDESTORAGEDEVICE_H
