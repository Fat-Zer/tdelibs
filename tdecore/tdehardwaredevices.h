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
#include <tqobject.h>
#include <tqstring.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include "kiconloader.h"
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

// Keep readGenericDeviceTypeFromString(), getFriendlyDeviceTypeStringFromType(), and getDeviceTypeIconFromType() in tdehardwaredevices.cpp in sync with this enum
namespace TDEGenericDeviceType {
enum TDEGenericDeviceType {
	Root,
	CPU,
	GPU,
	RAM,
	Bus,
	I2C,
	MDIO,
	Mainboard,
	Disk,
	SCSI,
	StorageController,
	Mouse,
	Keyboard,
	HID,
	Network,
	Printer,
	Scanner,
	Sound,
	VideoCapture,
	IEEE1394,
	Camera,
	TextIO,
	Serial,
	Parallel,
	Peripheral,
	Battery,
	Power,
	Dock,
	ThermalSensor,
	ThermalControl,
	Bridge,
	Platform,
	PNP,
	OtherACPI,
	OtherUSB,
	OtherMultimedia,
	OtherPeripheral,
	OtherSensor,
	OtherVirtual,
	Other,
	Last = Other
};
};

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
		* @return a TQString with a friendly name
		*
		* While TDE tries very hard to generate and return a friendly name for this device,
		* sometimes the best it will be able to do is "Unknown Device [xxxx:yyyy]"
		*/
		virtual TQString friendlyName();

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

		/**
		* @return true if this device has been blacklisted for update actions
		*/
		bool blacklistedForUpdate();

		/**
		* @param bl true if this device has been blacklisted for update actions
		*/
		void setBlacklistedForUpdate(bool bl);

		/**
		* @return a TQString containing a unique identifier for this device
		*/
		TQString uniqueID();

		/**
		* @return a TQString with the vendor ID, if any
		*/
		TQString &vendorID();

		/**
		* @param a TQString with the vendor ID, if any
		*/
		void setVendorID(TQString id);

		/**
		* @return a TQString with the model ID, if any
		*/
		TQString &modelID();

		/**
		* @param a TQString with the model ID, if any
		*/
		void setModelID(TQString id);

		/**
		* @return a TQString with the subvendor ID, if any
		*/
		TQString &subVendorID();

		/**
		* @param a TQString with the subvendor ID, if any
		*/
		void setSubVendorID(TQString id);

		/**
		* @return a TQString with the submodel ID, if any
		*/
		TQString &subModelID();

		/**
		* @param a TQString with the submodel ID, if any
		*/
		void setSubModelID(TQString id);

		/**
		* @return a TQString with the PCI device class, if any
		*/
		TQString &PCIClass();

		/**
		* @param a TQString with the PCI device class, if any
		*/
		void setPCIClass(TQString cl);

		/**
		* @return a TQString with the module alias string, if any
		*/
		TQString &moduleAlias();

		/**
		* @param a TQString with the module alias string, if any
		*/
		void setModuleAlias(TQString ma);

		/**
		* @return a TQString with the device driver, if any
		*/
		TQString &deviceDriver();

		/**
		* @param a TQString with the device driver, if any
		*/
		void setDeviceDriver(TQString dr);

		/**
		* @return a TQString with the subsystem type, if any
		*/
		TQString &subsystem();

		/**
		* @param a TQString with the subsystem type, if any
		*/
		void setSubsystem(TQString ss);

		/**
		* @param a TDEGenericDevice* with the parent device, if any
		*/
		void setParentDevice(TDEGenericDevice* pd);

		/**
		* @return a TDEGenericDevice* with the parent device, if any
		*/
		TDEGenericDevice* parentDevice();

		/**
		*  @return a TQString containing the friendly type name
		*/
		TQString friendlyDeviceType();

		/**
		*  @return a TQString containing the device bus ID, if any
		*/
		TQString busID();

		/**
		*  Get an icon for this device
		*  @param size a KIcon::StdSizes structure specifying the desired icon size
		*  @return a TQPixmap containing the icon for the specified type
		*/
		virtual TQPixmap icon(KIcon::StdSizes size);

	private:
		TDEGenericDeviceType::TDEGenericDeviceType m_deviceType;
		TQString m_deviceName;
		TQString m_systemPath;
		TQString m_deviceNode;
		TQString m_vendorName;
		TQString m_vendorModel;
		TQString m_deviceBus;
		TQString m_uniqueID;
		TQString m_vendorID;
		TQString m_modelID;
		TQString m_subvendorID;
		TQString m_submodelID;
		TQString m_pciClass;
		TQString m_modAlias;
		TQString m_deviceDriver;
		TQString m_subsystem;
		TQString m_friendlyName;
		bool m_blacklistedForUpdate;
		TDEGenericDevice* m_parentDevice;

		// Internal use only!
		TQStringList m_externalSubtype;
		TQString m_externalRulesFile;

	friend class TDEHardwareDevices;
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
		* @param an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		*/
		bool isDiskOfType(TDEDiskDeviceType::TDEDiskDeviceType tf);

		/**
		* @return an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		*/
		TDEDiskDeviceStatus::TDEDiskDeviceStatus diskStatus();

		/**
		* @param an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		*/
		void setDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus st);

		/**
		* @param an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		*/
		bool checkDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus sf);

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
		* @return a TQStringList containing system paths to all devices with a lock on this device, if any
		*/
		TQStringList &holdingDevices();

		/**
		* @param a TQStringList containing system paths to all devices with a lock on this device, if any
		*/
		void setHoldingDevices(TQStringList hd);

		/**
		* @return a TQStringList containing system paths to all devices locked by this device, if any
		*/
		TQStringList &slaveDevices();

		/**
		* @param a TQStringList containing system paths to all devices locked by this device, if any
		*/
		void setSlaveDevices(TQStringList sd);

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
		unsigned long deviceSize();

		/**
		* @return a TQString with the device size in human readable form
		*/
		TQString deviceFriendlySize();

		/**
		* Get an icon for this device
		* @param size a KIcon::StdSizes structure specifying the desired icon size
		* @return a TQPixmap containing the icon for the specified type
		*
		* This method overrides TDEGenericDevice::icon(KIcon::StdSizes size)
		*/
		TQPixmap icon(KIcon::StdSizes size);

		/**
		* @return a TQString with a friendly name
		*
		* This method overrides TDEGenericDevice::friendlyName()
		*/
		TQString friendlyName();

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
};

typedef TQPtrList<TDEGenericDevice> TDEGenericHardwareList;
typedef TQMap<TQString, TQString> TDEDeviceIDMap;

class TQSocketNotifier;

class TDECORE_EXPORT TDEHardwareDevices : public TQObject
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
		TQPtrList<TDEGenericDevice> listAllPhysicalDevices();

		/**
		*  List all hardware capabilities on all devices
		*  @param a TDEGenericDeviceType::TDEGenericDeviceType specifying the device class
		*  @return TQPtrList<TDEGenericDevice> containing all known hardware devices
		*/
		TQPtrList<TDEGenericDevice> listByDeviceClass(TDEGenericDeviceType::TDEGenericDeviceType cl);

		/**
		*  Return the device with system path @arg syspath, or 0 if no device exists for that path
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findBySystemPath(TQString syspath);

		/**
		*  Return the device with device node @arg devnode, or 0 if no device exists at that node
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findByDeviceNode(TQString devnode);

		/**
		*  Return the storage device with unique ID @arg uid, or 0 if no device exists for that uid
		*  @return TDEGenericDevice
		*/
		TDEStorageDevice* findDiskByUID(TQString uid);

		/**
		*  Look up the device in the system PCI database
		*  @param vendorid a TQString containing the vendor ID in hexadecimal
		*  @param modelid a TQString containing the model ID in hexadecimal
		*  @param subvendorid a TQString containing the subvendor ID in hexadecimal
		*  @param submodelid a TQString containing the submodel ID in hexadecimal
		*  @return a TQString containing the device name, if found
		*/
		TQString findPCIDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid);

		/**
		*  Look up the device in the system USB database
		*  @param vendorid a TQString containing the vendor ID in hexadecimal
		*  @param modelid a TQString containing the model ID in hexadecimal
		*  @param subvendorid a TQString containing the subvendor ID in hexadecimal
		*  @param submodelid a TQString containing the submodel ID in hexadecimal
		*  @return a TQString containing the device name, if found
		*/
		TQString findUSBDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid);

		/**
		*  Get a friendly string describing a device type
		*  @param query a TDEGenericDeviceType::TDEGenericDeviceType specifying a device type
		*  @return a TQString containing the friendly type name
		*/
		TQString getFriendlyDeviceTypeStringFromType(TDEGenericDeviceType::TDEGenericDeviceType query);

		/**
		*  Get an icon for a device type
		*  @param query a TDEGenericDeviceType::TDEGenericDeviceType specifying a device type
		*  @param size a KIcon::StdSizes structure specifying the desired icon size
		*  @return a TQPixmap containing the icon for the specified type
		*/
		TQPixmap getDeviceTypeIconFromType(TDEGenericDeviceType::TDEGenericDeviceType query, KIcon::StdSizes size);

	signals:
		void hardwareAdded(TDEGenericDevice*);
		void hardwareRemoved(TDEGenericDevice*);
		void hardwareUpdated(TDEGenericDevice*);
		void mountTableModified();

	private slots:
		void processHotPluggedHardware();
		void processModifiedMounts();

	private:
		void rescanDeviceInformation(TDEGenericDevice* hwdevice);
		void updateBlacklists(TDEGenericDevice* hwdevice, udev_device* dev);

	private:
		TDEGenericDevice *classifyUnknownDevice(udev_device* dev, TDEGenericDevice* existingdevice=0, bool force_full_classification=true);
		TDEGenericDevice *classifyUnknownDeviceByExternalRules(udev_device* dev, TDEGenericDevice* existingdevice=0, bool classifySubDevices=false);

		void updateParentDeviceInformation();
		void updateParentDeviceInformation(TDEGenericDevice* hwdevice);

		void addCoreSystemDevices();

		struct udev *m_udevStruct;
		struct udev_monitor *m_udevMonitorStruct;
		TDEGenericHardwareList m_deviceList;
		int m_procMountsFd;

		TQSocketNotifier* m_devScanNotifier;
		TQSocketNotifier* m_mountScanNotifier;

		TQStringList m_mountTable;

		TDEDeviceIDMap* pci_id_map;
		TDEDeviceIDMap* usb_id_map;

	friend class TDEGenericDevice;
	friend class TDEStorageDevice;
};

#endif