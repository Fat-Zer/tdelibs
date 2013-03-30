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

class TDENetworkConnectionManager;

namespace TDEHardwareEvent {
enum TDEHardwareEvent {
	HardwareListModified,
	MountTableModified,
	HardwareAdded,
	HardwareRemoved,
	HardwareUpdated,
	Other,
	Last = Other
};
};

// Keep readGenericDeviceTypeFromString(), getFriendlyDeviceTypeStringFromType(), and getDeviceTypeIconFromType() in tdehardwaredevices.cpp in sync with this enum
namespace TDEGenericDeviceType {
enum TDEGenericDeviceType {
	Root,
	RootSystem,
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
	Modem,
	Monitor,
	Network,
	Printer,
	Scanner,
	Sound,
	VideoCapture,
	IEEE1394,
	PCMCIA,
	Camera,
	TextIO,
	Serial,
	Parallel,
	Peripheral,
	Backlight,
	Battery,
	PowerSupply,
	Dock,
	ThermalSensor,
	ThermalControl,
	BlueTooth,
	Bridge,
	Platform,
	Cryptography,
	Event,
	Input,
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

class TDECORE_EXPORT TDESensorCluster
{
	public:
		/**
		*  Constructor.
		*/
		TDESensorCluster();

		TQString label;
		double current;
		double minimum;
		double maximum;
		double warning;
		double critical;
};

class TDECORE_EXPORT TDEGenericDevice : public TQObject
{
	Q_OBJECT

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
		TQString name();

		/**
		* @return a TQString with the vendor name, if any
		*/
		TQString vendorName();

		/**
		* @return a TQString with the vendor model, if any
		*/
		TQString vendorModel();

		/**
		* @return a TQString with the serial number, if any
		*/
		TQString serialNumber();

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
		TQString deviceBus();

		/**
		* @return a TQString with the system path, if any
		*
		* This method is non-portable, so be careful!
		*/
		TQString systemPath();

		/**
		* @return a TQString with the system device node, if any
		*
		* This method is non-portable, so be careful!
		*/
		TQString deviceNode();

		/**
		* @return true if this device has been blacklisted for update actions
		*/
		bool blacklistedForUpdate();

		/**
		* @return a TQString containing a unique identifier for this device
		*/
		TQString uniqueID();

		/**
		* @return a TQString with the vendor ID, if any
		*/
		TQString vendorID();

		/**
		* @return a TQString with the model ID, if any
		*/
		TQString modelID();

		/**
		* @return a TQString with the encoded vendor, if any
		*/
		TQString vendorEncoded();

		/**
		* @return a TQString with the encoded model, if any
		*/
		TQString modelEncoded();

		/**
		* @return a TQString with the subvendor ID, if any
		*/
		TQString subVendorID();

		/**
		* @return a TQString with the submodel ID, if any
		*/
		TQString subModelID();

		/**
		* @return a TQString with the PCI device class, if any
		*/
		TQString PCIClass();

		/**
		* @return a TQString with the module alias string, if any
		*/
		TQString moduleAlias();

		/**
		* @return a TQString with the device driver, if any
		*/
		TQString deviceDriver();

		/**
		* @return a TQString with the subsystem type, if any
		*/
		TQString subsystem();

		/**
		* @return a TDEGenericDevice* with the parent device, if any
		*/
		TDEGenericDevice* parentDevice();

		/**
		*  @return a TQString containing the friendly type name
		*/
		virtual TQString friendlyDeviceType();

		/**
		*  @return a TQString containing the device bus ID, if any
		*/
		TQString busID();

		/**
		*  Get an icon for this device
		*  @param size a TDEIcon::StdSizes structure specifying the desired icon size
		*  @return a TQPixmap containing the icon for the specified type
		*/
		virtual TQPixmap icon(TDEIcon::StdSizes size);

	protected:
		/**
		* @param a TQString with the device name, if any
		* @internal
		*/
		void internalSetName(TQString dn);

		/**
		* @param a TQString with the vendor name, if any
		* @internal
		*/
		void internalSetVendorName(TQString vn);

		/**
		* @param a TQString with the vendor model, if any
		* @internal
		*/
		void internalSetVendorModel(TQString vm);

		/**
		* @param a TQString with the serial number, if any
		* @internal
		*/
		void internalSetSerialNumber(TQString sn);

		/**
		* @param a TQString with the device bus name, if any
		* @internal
		*/
		void internalSetDeviceBus(TQString db);

		/**
		* @param a TQString with the system path, if any
		* @internal
		*
		* This method is non-portable, so be careful!
		*/
		void internalSetSystemPath(TQString sp);

		/**
		* @param a TQString with the system device node, if any
		* @internal
		*
		* This method is non-portable, so be careful!
		*/
		void internalSetDeviceNode(TQString sn);

		/**
		* @param bl true if this device has been blacklisted for update actions
		* @internal
		*/
		void internalSetBlacklistedForUpdate(bool bl);

		/**
		* @param a TQString with the vendor ID, if any
		* @internal
		*/
		void internalSetVendorID(TQString id);

		/**
		* @param a TQString with the model ID, if any
		* @internal
		*/
		void internalSetModelID(TQString id);

		/**
		* @param a TQString with the encoded vendor, if any
		* @internal
		*/
		void internalSetVendorEncoded(TQString id);

		/**
		* @param a TQString with the encoded model, if any
		* @internal
		*/
		void internalSetModelEncoded(TQString id);

		/**
		* @param a TQString with the subvendor ID, if any
		* @internal
		*/
		void internalSetSubVendorID(TQString id);

		/**
		* @param a TQString with the submodel ID, if any
		* @internal
		*/
		void internalSetSubModelID(TQString id);

		/**
		* @param a TQString with the PCI device class, if any
		* @internal
		*/
		void internalSetPCIClass(TQString cl);

		/**
		* @param a TQString with the module alias string, if any
		* @internal
		*/
		void internalSetModuleAlias(TQString ma);

		/**
		* @param a TQString with the device driver, if any
		* @internal
		*/
		void internalSetDeviceDriver(TQString dr);

		/**
		* @param a TQString with the subsystem type, if any
		* @internal
		*/
		void internalSetSubsystem(TQString ss);

		/**
		* @param a TDEGenericDevice* with the parent device, if any
		* @internal
		*/
		void internalSetParentDevice(TDEGenericDevice* pd);

	private:
		TDEGenericDeviceType::TDEGenericDeviceType m_deviceType;
		TQString m_deviceName;
		TQString m_systemPath;
		TQString m_deviceNode;
		TQString m_vendorName;
		TQString m_vendorModel;
		TQString m_serialNumber;
		TQString m_deviceBus;
		TQString m_uniqueID;
		TQString m_vendorID;
		TQString m_modelID;
		TQString m_vendorenc;
		TQString m_modelenc;
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
		TQString m_udevtype;
		TQString m_udevdevicetypestring;
		TQString udevdevicetypestring_alt;

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
		unsigned long deviceSize();

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

class TDECORE_EXPORT TDECPUDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDECPUDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDECPUDevice();

		/**
		* @return a double with the current CPU frequency in MHz, if available
		*/
		double frequency();

		/**
		* @return a double with the minimum CPU frequency in MHz, if available
		*/
		double minFrequency();

		/**
		* @return a double with the maximum CPU frequency in MHz, if available
		*/
		double maxFrequency();

		/**
		* @return a double with the transition latency in ns, if available
		*/
		double transitionLatency();

		/**
		* @return a TQString with the current CPU governor policy, if available
		*/
		TQString governor();

		/**
		* @return a TQString with the current CPU scaling driver, if available
		*/
		TQString scalingDriver();

		/**
		* @return a TQStringList with the IDs of all processors that are dependent on the frequency/power settings of this one, if available
		*/
		TQStringList dependentProcessors();

		/**
		* @return a TQStringList with all valid scaling frequencies in Hz, if available
		*/
		TQStringList availableFrequencies();

		/**
		* @return a TQStringList with all available governor policies, if available
		*/
		TQStringList availableGovernors();

		/**
		* @return TRUE if permissions allow the CPU governor to be set, FALSE if not
		*/
		bool canSetGovernor();

		/**
		* @param gv a TQString with the new CPU governor policy name
		*/
		void setGovernor(TQString gv);

		/**
		* @return TRUE if permissions allow the CPU maximum frequency to be set, FALSE if not
		*/
		bool canSetMaximumScalingFrequency();

		/**
		* @param gv a double with the new CPU maximum frequency in MHz
		*/
		void setMaximumScalingFrequency(double fr);

		/**
		* @return an integer with the core number, starting at 0
		*/
		int coreNumber();

	protected:
		/**
		* @param fr a double with the current CPU frequency in MHz, if available
		* @internal
		*/
		void internalSetFrequency(double fr);

		/**
		* @param fr a double with the minimum CPU frequency in MHz, if available
		* @internal
		*/
		void internalSetMinFrequency(double fr);

		/**
		* @param fr a double with the maximum CPU frequency in MHz, if available
		* @internal
		*/
		void internalSetMaxFrequency(double fr);

		/**
		* @param tl a double with the transition latency in ns, if available
		* @internal
		*/
		void internalSetTransitionLatency(double tl);

		/**
		* @param gr a TQString with the current CPU governor policy, if available
		* @internal
		*/
		void internalSetGovernor(TQString gr);

		/**
		* @param dr a TQString with the current CPU scaling driver, if available
		* @internal
		*/
		void internalSetScalingDriver(TQString dr);

		/**
		* @param dp a TQStringList with the IDs of all processors that are dependent on the frequency/power settings of this one, if available
		* @internal
		*/
		void internalSetDependentProcessors(TQStringList dp);

		/**
		* @param af a TQStringList with all valid scaling frequencies in Hz, if available
		* @internal
		*/
		void internalSetAvailableFrequencies(TQStringList af);

		/**
		* @param gp a TQStringList with all available governor policies, if available
		* @internal
		*/
		void internalSetAvailableGovernors(TQStringList gp);

		/**
		* @param cn an integer with the core number, starting at 0
		* @internal
		*/
		void internalSetCoreNumber(int cn);

	private:
		double m_frequency;
		double m_minfrequency;
		double m_maxfrequency;
		double m_transitionlatency;
		TQString m_governor;
		TQString m_scalingdriver;
		TQStringList m_tiedprocs;
		TQStringList m_frequencies;
		TQStringList m_governers;
		int m_corenumber;

	friend class TDEHardwareDevices;
};

namespace TDEBatteryStatus {
enum TDEBatteryStatus {
	Charging,
	Discharging,
	Full,
	Unknown = 0x80000000
};
};

class TDECORE_EXPORT TDEBatteryDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEBatteryDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEBatteryDevice();

		/**
		* @return a double with the current battery voltage, if available
		*/
		double voltage();

		/**
		* @return a double with the minimum battery voltage, if available
		*/
		double minimumVoltage();

		/**
		* @return a double with the maximum battery voltage, if available
		*/
		double maximumVoltage();

		/**
		* @return a double with the designed maximum battery voltage, if available
		*/
		double maximumDesignVoltage();

		/**
		* @return a double with the current battery energy in watt-hours, if available
		*/
		double energy();

		/**
		* @return a double with the current battery alarm energy in watt-hours, if available
		*/
		double alarmEnergy();

		/**
		* @return a double with the maximum battery energy in watt-hours, if available
		*/
		double maximumEnergy();

		/**
		* @return a double with the designed maximum battery energy in watt-hours, if available
		*/
		double maximumDesignEnergy();

		/**
		* @return a double with the current battery discharge rate in watt-hours, if available
		*/
		double dischargeRate();

		/**
		* @return a double with the current battery discharge time remaining in seconds, if available
		*/
		double timeRemaining();

		/**
		* @return a TQString with the battery technology, if available
		*/
		TQString technology();

		/**
		* @return a TDEBatteryStatus::TDEBatteryStatus with the current battery status
		*/
		TDEBatteryStatus::TDEBatteryStatus status();

		/**
		* @return TRUE if the battery is installed
		*/
		bool installed();

		/**
		* @return a double with the current battery charge in percent, if available
		*/
		double chargePercent();

	protected:
		/**
		* @param a double with the current battery voltage, if available
		* @internal
		*/
		void internalSetVoltage(double vt);

		/**
		* @param a double with the minimum battery voltage, if available
		* @internal
		*/
		void internalSetMinimumVoltage(double vt);

		/**
		* @param a double with the maximum battery voltage, if available
		* @internal
		*/
		void internalSetMaximumVoltage(double vt);

		/**
		* @param a double with the designed maximum battery voltage, if available
		* @internal
		*/
		void internalSetMaximumDesignVoltage(double vt);

		/**
		* @param a double with the current battery energy in watt-hours, if available
		* @internal
		*/
		void internalSetEnergy(double vt);

		/**
		* @param a double with the current battery alarm energy in watt-hours, if available
		* @internal
		*/
		void internalSetAlarmEnergy(double vt);

		/**
		* @param a double with the maximum battery energy in watt-hours, if available
		* @internal
		*/
		void internalSetMaximumEnergy(double vt);

		/**
		* @param a double with the designed maximum battery energy in watt-hours, if available
		* @internal
		*/
		void internalSetMaximumDesignEnergy(double vt);

		/**
		* @param a double with the current battery discharge rate in volt-hours, if available
		* @internal
		*/
		void internalSetDischargeRate(double vt);

		/**
		* @param a double with the current battery discharge time remaining in seconds, if available
		* @internal
		*/
		void internalSetTimeRemaining(double tr);

		/**
		* @param a TQString with the battery technology, if available
		* @internal
		*/
		void internalSetTechnology(TQString tc);

		/**
		* @param a TQString with the battery status, if available
		* @internal
		*/
		void internalSetStatus(TQString tc);

		/**
		* @param TRUE if the battery is installed
		* @internal
		*/
		void internalSetInstalled(bool tc);

	private:
		double m_currentVoltage;
		double m_minimumVoltage;
		double m_maximumVoltage;
		double m_maximumDesignVoltage;
		double m_alarmEnergy;
		double m_currentEnergy;
		double m_maximumEnergy;
		double m_maximumDesignEnergy;
		double m_dischargeRate;
		double m_timeRemaining;
		TQString m_technology;
		TDEBatteryStatus::TDEBatteryStatus m_status;
		bool m_installed;

	friend class TDEHardwareDevices;
};

class TDECORE_EXPORT TDEMainsPowerDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEMainsPowerDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEMainsPowerDevice();

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

	friend class TDEHardwareDevices;
};

class TDECORE_EXPORT TDENetworkDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDENetworkDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDENetworkDevice();

		/**
		* @return a TQString containing the network device's MAC address
		*/
		TQString macAddress();

		/**
		* @return a TQString containing the network device's operational state
		*/
		TQString state();

		/**
		* @return TRUE if carrier is present, FALSE if not
		*/
		bool carrierPresent();

		/**
		* @return TRUE if device is dormant, FALSE if not
		*/
		bool dormant();

		/**
		* @return a TQString containing the network device's IPv4 address
		*/
		TQString ipV4Address();

		/**
		* @return a TQString containing the network device's IPv6 address
		*/
		TQString ipV6Address();

		/**
		* @return a TQString containing the network device's IPv4 netmask
		*/
		TQString ipV4Netmask();

		/**
		* @return a TQString containing the network device's IPv6 netmask
		*/
		TQString ipV6Netmask();

		/**
		* @return a TQString containing the network device's IPv4 broadcast
		*/
		TQString ipV4Broadcast();

		/**
		* @return a TQString containing the network device's IPv6 broadcast
		*/
		TQString ipV6Broadcast();

		/**
		* @return a TQString containing the network device's IPv4 destination
		*/
		TQString ipV4Destination();

		/**
		* @return a TQString containing the network device's IPv6 destination
		*/
		TQString ipV6Destination();

		/**
		* @return a double with the number of received bytes, if available
		*/
		double rxBytes();

		/**
		* @return a double with the number of transmitted bytes, if available
		*/
		double txBytes();

		/**
		* @return a double with the number of received packets, if available
		*/
		double rxPackets();

		/**
		* @return a double with the number of transmitted packets, if available
		*/
		double txPackets();

		/**
		* @return a pointer to a TDENetworkConnectionManager object, if available
		*/
		TDENetworkConnectionManager* connectionManager();

	protected:
		/**
		* @param ma a TQString containing the network device's MAC address
		* @internal
		*/
		void internalSetMacAddress(TQString ma);

		/**
		* @param st a TQString containing the network device's operational state
		* @internal
		*/
		void internalSetState(TQString st);

		/**
		* @param TRUE if carrier is present, FALSE if not
		* @internal
		*/
		void internalSetCarrierPresent(bool cp);

		/**
		* @param TRUE if device is dormant, FALSE if not
		* @internal
		*/
		void internalSetDormant(bool dm);

		/**
		* @param ad a TQString containing the network device's IPv4 address
		* @internal
		*/
		void internalSetIpV4Address(TQString ad);

		/**
		* @param ad a TQString containing the network device's IPv6 address
		* @internal
		*/
		void internalSetIpV6Address(TQString ad);

		/**
		* @param nm a TQString containing the network device's IPv4 netmask
		* @internal
		*/
		void internalSetIpV4Netmask(TQString nm);

		/**
		* @param nm a TQString containing the network device's IPv6 netmask
		* @internal
		*/
		void internalSetIpV6Netmask(TQString nm);

		/**
		* @param br a TQString containing the network device's IPv4 broadcast
		* @internal
		*/
		void internalSetIpV4Broadcast(TQString br);

		/**
		* @param br a TQString containing the network device's IPv6 broadcast
		* @internal
		*/
		void internalSetIpV6Broadcast(TQString br);

		/**
		* @param ds a TQString containing the network device's IPv4 destination
		* @internal
		*/
		void internalSetIpV4Destination(TQString ds);

		/**
		* @param ds a TQString containing the network device's IPv6 destination
		* @internal
		*/
		void internalSetIpV6Destination(TQString ds);

		/**
		* @param rx a double with the number of received bytes, if available
		* @internal
		*/
		void internalSetRxBytes(double rx);

		/**
		* @param tx a double with the number of transmitted bytes, if available
		* @internal
		*/
		void internalSetTxBytes(double tx);

		/**
		* @param rx a double with the number of received packets, if available
		* @internal
		*/
		void internalSetRxPackets(double rx);

		/**
		* @param tx a double with the number of transmitted packets, if available
		* @internal
		*/
		void internalSetTxPackets(double tx);

		/**
		* @param mgr a pointer to a TDENetworkConnectionManager object, if available
		*/
		void internalSetConnectionManager(TDENetworkConnectionManager* mgr);

	private:
		TQString m_macAddress;
		TQString m_state;
		bool m_carrier;
		bool m_dormant;
		TQString m_ipV4Address;
		TQString m_ipV6Address;
		TQString m_ipV4Netmask;
		TQString m_ipV6Netmask;
		TQString m_ipV4Broadcast;
		TQString m_ipV6Broadcast;
		TQString m_ipV4Destination;
		TQString m_ipV6Destination;
		double m_rxbytes;
		double m_txbytes;
		double m_rxpackets;
		double m_txpackets;
		TDENetworkConnectionManager* m_connectionManager;

	friend class TDEHardwareDevices;
};

namespace TDEDisplayPowerLevel {
enum TDEDisplayPowerLevel {
	On,
	Standby,
	Suspend,
	Off
};
};

class TDECORE_EXPORT TDEBacklightDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEBacklightDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEBacklightDevice();

		/**
		* @return a TDEDisplayPowerLevel::TDEDisplayPowerLevel with the current power level
		*/
		TDEDisplayPowerLevel::TDEDisplayPowerLevel powerLevel();

		/**
		* @return an integer with the number of discrete control steps available
		*/
		int brightnessSteps();

		/**
		* @return a double with the current brightness percentage
		*/
		double brightnessPercent();

		/**
		* @return TRUE if permissions allow brightness can be set, FALSE if not
		*/
		bool canSetBrightness();

		/**
		* @return an int with the current raw brightness
		*/
		int rawBrightness();

		/**
		* @param br an integer with the new raw brightness value
		*/
		void setRawBrightness(int br);

	protected:
		/**
		* @param pl a TDEDisplayPowerLevel::TDEDisplayPowerLevel with the current power level
		* @internal
		*/
		void internalSetPowerLevel(TDEDisplayPowerLevel::TDEDisplayPowerLevel pl);

		/**
		* @param br an integer with the maximum raw brightness value
		* @internal
		*/
		void internalSetMaximumRawBrightness(int br);

		/**
		* @param br an integer with the current raw brightness value
		* @internal
		*/
		void internalSetCurrentRawBrightness(int br);

	private:
		TDEDisplayPowerLevel::TDEDisplayPowerLevel m_powerLevel;
		int m_currentBrightness;
		int m_maximumBrightness;

	friend class TDEHardwareDevices;
};

typedef TQPair<unsigned int, unsigned int> TDEResolutionPair;
typedef TQValueList< TDEResolutionPair > TDEResolutionList;

class TDECORE_EXPORT TDEMonitorDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEMonitorDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEMonitorDevice();

		/**
		* @return TRUE if a monitor is connected, FALSE if not
		*/
		bool connected();

		/**
		* @return TRUE if this port is enabled, FALSE if not
		*/
		bool enabled();

		/**
		* @return a TQByteArray containing this monitor's EDID information
		*/
		TQByteArray edid();

		/**
		* @return a TDEResolutionList containing this monitor's supported resolutions
		*/
		TDEResolutionList resolutions();

		/**
		* @return a TQString containing the display port type
		*/
		TQString portType();

		/**
		* @return a TDEDisplayPowerLevel::TDEDisplayPowerLevel with the current power level
		*/
		TDEDisplayPowerLevel::TDEDisplayPowerLevel powerLevel();

	protected:
		/**
		* @param TRUE if a monitor is connected, FALSE if not
		* @internal
		*/
		void internalSetConnected(bool cn);

		/**
		* @param TRUE if this port is enabled, FALSE if not
		* @internal
		*/
		void internalSetEnabled(bool en);

		/**
		* @param ed a TQByteArray containing this monitor's EDID information
		* @internal
		*/
		void internalSetEdid(TQByteArray ed);

		/**
		* @param rs a TDEResolutionList containing this monitor's supported resolutions
		* @internal
		*/
		void internalSetResolutions(TDEResolutionList rs);

		/**
		* @param pt a TQString containing the display port type
		* @internal
		*/
		void internalSetPortType(TQString pt);

		/**
		* @param pl a TDEDisplayPowerLevel::TDEDisplayPowerLevel with the current power level
		* @internal
		*/
		void internalSetPowerLevel(TDEDisplayPowerLevel::TDEDisplayPowerLevel pl);

	private:
		bool m_connected;
		bool m_enabled;
		TQByteArray m_edid;
		TDEResolutionList m_resolutions;
		TQString m_portType;
		TDEDisplayPowerLevel::TDEDisplayPowerLevel m_powerLevel;

	friend class TDEHardwareDevices;
};


typedef TQMap<TQString, TDESensorCluster> TDESensorClusterMap;

class TDECORE_EXPORT TDESensorDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDESensorDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDESensorDevice();

		/**
		* @return a TDESensorClusterMap with the current sensor values
		*/
		TDESensorClusterMap values();

	protected:
		/**
		* @param a TDESensorClusterMap with the current sensor values
		* @internal
		*/
		void internalSetValues(TDESensorClusterMap cl);

	private:
		TDESensorClusterMap m_sensorValues;

	friend class TDEHardwareDevices;
};

namespace TDESystemFormFactor {
enum TDESystemFormFactor {
	Unclassified,
	Desktop,
	Laptop,
	Server,
	Other = 0x80000000
};
};

namespace TDESystemPowerState {
enum TDESystemPowerState {
	Active,
	Standby,
	Suspend,
	Hibernate,
	PowerOff
};
};

namespace TDESystemHibernationMethod {
enum TDESystemHibernationMethod {
	Unsupported,
	Platform,
	Shutdown,
	Reboot,
	TestProc,
	Test
};
};

typedef TQValueList<TDESystemPowerState::TDESystemPowerState> TDESystemPowerStateList;
typedef TQValueList<TDESystemHibernationMethod::TDESystemHibernationMethod> TDESystemHibernationMethodList;

class TDECORE_EXPORT TDERootSystemDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDERootSystemDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDERootSystemDevice();

		/**
		* @return a TDESystemFormFactor::TDESystemFormFactor with the system's form factor
		*/
		TDESystemFormFactor::TDESystemFormFactor formFactor();

		/**
		* @return a TDESystemPowerStateList with all available power states
		*/
		TDESystemPowerStateList powerStates();

		/**
		* @return a TDESystemHibernationMethodList with all available hibernation methods
		*/
		TDESystemHibernationMethodList hibernationMethods();

		/**
		* @return a TDESystemHibernationMethod::TDESystemHibernationMethod with the current hibernation method
		*/
		TDESystemHibernationMethod::TDESystemHibernationMethod hibernationMethod();

		/**
		* @return an unsigned long with the number of bytes required to hibernate
		*/
		unsigned long diskSpaceNeededForHibernation();

		/**
		* @return TRUE if permissions allow the hibernation method to be set, FALSE if not
		*/
		bool canSetHibernationMethod();

		/**
		* @return TRUE if hardware and permissions allow the system to enter standby, FALSE if not
		*/
		bool canStandby();

		/**
		* @return TRUE if hardware and permissions allow the system to be suspended, FALSE if not
		*/
		bool canSuspend();

		/**
		* @return TRUE if hardware and permissions allow the system to be hibernated, FALSE if not
		*/
		bool canHibernate();

		/**
		* @return TRUE if permissions allow the system to be powered down, FALSE if not
		*/
		bool canPowerOff();

		/**
		* @param hm a TDESystemHibernationMethod::TDESystemHibernationMethod with the desired hibernation method
		*/
		void setHibernationMethod(TDESystemHibernationMethod::TDESystemHibernationMethod hm);

		/**
		* @param ps a TDESystemPowerState::TDESystemPowerState with the desired power state
		* @return TRUE if power state was set
		*/
		bool setPowerState(TDESystemPowerState::TDESystemPowerState ps);

	protected:
		/**
		* @param ff a TDESystemFormFactor::TDESystemFormFactor with the system's form factor
		* @internal
		*/
		void internalSetFormFactor(TDESystemFormFactor::TDESystemFormFactor ff);

		/**
		* @param ps a TDESystemPowerStateList with all available power states
		* @internal
		*/
		void internalSetPowerStates(TDESystemPowerStateList ps);

		/**
		* @param hm a TDESystemHibernationMethodList with all available hibernation methods
		* @internal
		*/
		void internalSetHibernationMethods(TDESystemHibernationMethodList hm);

		/**
		* @param hm a TDESystemHibernationMethod::TDESystemHibernationMethod with the current hibernation method
		* @internal
		*/
		void internalSetHibernationMethod(TDESystemHibernationMethod::TDESystemHibernationMethod hm);

		/**
		* @param sz an unsigned long with the number of bytes required to hibernate
		* @internal
		*/
		void internalSetDiskSpaceNeededForHibernation(unsigned long sz);

	private:
		TDESystemFormFactor::TDESystemFormFactor m_formFactor;
		TDESystemPowerStateList m_powerStates;
		TDESystemHibernationMethodList m_hibernationMethods;
		TDESystemHibernationMethod::TDESystemHibernationMethod m_hibernationMethod;
		unsigned long m_hibernationSpace;

	friend class TDEHardwareDevices;
};

namespace TDEEventDeviceType {
enum TDEEventDeviceType {
	Unknown,
	ACPILidSwitch,
	ACPISleepButton,
	ACPIPowerButton,
	Other = 0x80000000
};
};

// Keep friendlySwitchList() in tdehardwaredevices.cpp in sync with this enum
namespace TDESwitchType {
enum TDESwitchType {
	Null			= 0x00000000,
	Lid			= 0x00000001,
	TabletMode		= 0x00000002,
	HeadphoneInsert		= 0x00000004,
	RFKill			= 0x00000008,
	Radio			= 0x00000010,
	MicrophoneInsert	= 0x00000020,
	Dock			= 0x00000040,
	LineOutInsert		= 0x00000080,
	JackPhysicalInsert	= 0x00000100,
	VideoOutInsert		= 0x00000200,
	CameraLensCover		= 0x00000400,
	KeypadSlide		= 0x00000800,
	FrontProximity		= 0x00001000,
	RotateLock		= 0x00002000,
	LineInInsert		= 0x00004000
};

inline TDESwitchType operator|(TDESwitchType a, TDESwitchType b)
{
	return static_cast<TDESwitchType>(static_cast<int>(a) | static_cast<int>(b));
}

inline TDESwitchType operator&(TDESwitchType a, TDESwitchType b)
{
	return static_cast<TDESwitchType>(static_cast<int>(a) & static_cast<int>(b));
}

inline TDESwitchType operator~(TDESwitchType a)
{
	return static_cast<TDESwitchType>(~static_cast<int>(a));
}
};

class TQSocketNotifier;

class TDECORE_EXPORT TDEEventDevice : public TDEGenericDevice
{
	Q_OBJECT

	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEEventDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEEventDevice();

		/**
		* @return a TDEEventDeviceType::TDEEventDeviceType with the event device type, if known
		*/
		TDEEventDeviceType::TDEEventDeviceType eventType();

		/**
		* @return a TDESwitchType::TDESwitchType with all switches provided by this device
		*/
		TDESwitchType::TDESwitchType providedSwitches();

		/**
		* @return a TDESwitchType::TDESwitchType with all active switches provided by this device
		*/
		TDESwitchType::TDESwitchType activeSwitches();

		/**
		* @param switches a TDESwitchType::TDESwitchType with any switch flags set
		* @return a TQStringList with friendly names for all set switch flags
		*/
		static TQStringList friendlySwitchList(TDESwitchType::TDESwitchType switches);

	protected:
		/**
		* @param et a TDEEventDeviceType::TDEEventDeviceType with the event device type, if known
		* @internal
		*/
		void internalSetEventType(TDEEventDeviceType::TDEEventDeviceType et);

		/**
		* @param sl a TDESwitchType::TDESwitchType with all switches provided by this device
		* @internal
		*/
		void internalSetProvidedSwitches(TDESwitchType::TDESwitchType sl);

		/**
		* @param sl a TDESwitchType::TDESwitchType with all active switches provided by this device
		* @internal
		*/
		void internalSetActiveSwitches(TDESwitchType::TDESwitchType sl);

		/**
		* @param hwmanager the master hardware manager
		* @internal
		*/
		void internalStartFdMonitoring(TDEHardwareDevices* hwmanager);

	protected slots:
		void eventReceived();

	signals:
		/**
		* @param keycode the code of the key that was pressed/released
		* See include/linux/input.h for a complete list of keycodes
		* @param device a TDEEventDevice* with the device that received the event
		*/
		void keyPressed(unsigned int keycode, TDEEventDevice* device);

	private:
		TDEEventDeviceType::TDEEventDeviceType m_eventType;
		TDESwitchType::TDESwitchType m_providedSwitches;
		TDESwitchType::TDESwitchType m_switchActive;

		int m_fd;
		bool m_fdMonitorActive;
		TQSocketNotifier* m_eventNotifier;

	friend class TDEHardwareDevices;
};

namespace TDEInputDeviceType {
enum TDEInputDeviceType {
	Unknown,
	ACPILidSwitch,
	ACPISleepButton,
	ACPIPowerButton,
	Other = 0x80000000
};
};

class TDECORE_EXPORT TDEInputDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEInputDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEInputDevice();

		/**
		* @return a TDEInputDeviceType::TDEInputDeviceType with the input device type, if known
		*/
		TDEInputDeviceType::TDEInputDeviceType inputType();

	protected:
		/**
		* @param it a TDEInputDeviceType::TDEInputDeviceType with the input device type, if known
		* @internal
		*/
		void internalSetInputType(TDEInputDeviceType::TDEInputDeviceType it);

	private:
		TDEInputDeviceType::TDEInputDeviceType m_inputType;

	friend class TDEHardwareDevices;
};

typedef TQPtrList<TDEGenericDevice> TDEGenericHardwareList;
typedef TQMap<TQString, TQString> TDEDeviceIDMap;

class KSimpleDirWatch;

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
		*  @return TDEGenericHardwareList containing all known hardware devices
		*/
		TDEGenericHardwareList listAllPhysicalDevices();

		/**
		*  List all hardware capabilities on all devices
		*  @param a TDEGenericDeviceType::TDEGenericDeviceType specifying the device class
		*  @return TDEGenericHardwareList containing all known hardware devices
		*/
		TDEGenericHardwareList listByDeviceClass(TDEGenericDeviceType::TDEGenericDeviceType cl);

		/**
		*  Return the device with system path @arg syspath, or 0 if no device exists for that path
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findBySystemPath(TQString syspath);

		/**
		*  Return the device with unique ID @arg uid, or 0 if no device exists for that uid
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findByUniqueID(TQString uid);

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
		*  Look up the device in the system PNP database
		*  @param pnpid a TQString containing the PNP ID
		*  @return a TQString containing the device name, if found
		*/
		TQString findPNPDeviceName(TQString pnpid);

		/**
		*  Look up the monitor manufacturer in the system display database
		*  @param pnpid a TQString containing the display manufacturer ID
		*  @return a TQString containing the manufacturer name, if found
		*/
		TQString findMonitorManufacturerName(TQString dpyid);

		/**
		*  Get a friendly string describing a device type
		*  @param query a TDEGenericDeviceType::TDEGenericDeviceType specifying a device type
		*  @return a TQString containing the friendly type name
		*/
		TQString getFriendlyDeviceTypeStringFromType(TDEGenericDeviceType::TDEGenericDeviceType query);

		/**
		*  Get an icon for a device type
		*  @param query a TDEGenericDeviceType::TDEGenericDeviceType specifying a device type
		*  @param size a TDEIcon::StdSizes structure specifying the desired icon size
		*  @return a TQPixmap containing the icon for the specified type
		*/
		TQPixmap getDeviceTypeIconFromType(TDEGenericDeviceType::TDEGenericDeviceType query, TDEIcon::StdSizes size);

		/**
		*  Convenience function to obtain the root system device
		*  @return a pointer to a TDERootSystemDevice object
		*/
		TDERootSystemDevice* rootSystemDevice();

		/**
		*  Rescan a hardware device to look for changes
		*  WARNING: This method can be very expensive.  Use with caution!
		*  @param hwdevice TDEGenericDevice* with the device to rescan
		*/
		void rescanDeviceInformation(TDEGenericDevice* hwdevice);

		/**
		*  Convert a byte count to human readable form
		*  @param bytes a double containing the number of bytes
		*  @return a TQString containing the human readable byte count
		*/
		static TQString bytesToFriendlySizeString(double bytes);

	signals:
		void hardwareAdded(TDEGenericDevice*);
		void hardwareRemoved(TDEGenericDevice*);
		void hardwareUpdated(TDEGenericDevice*);
		void mountTableModified();
		void hardwareEvent(TDEHardwareEvent::TDEHardwareEvent, TQString uuid);

		/**
		* @param keycode the code of the key that was pressed/released
		* See include/linux/input.h for a complete list of keycodes
		* @param device a TDEEventDevice* with the device that received the event
		*/
		void eventDeviceKeyPressed(unsigned int keycode, TDEEventDevice* device);

	private slots:
		void processHotPluggedHardware();
		void processModifiedMounts();
		void processModifiedCPUs();
		void processStatelessDevices();
		void processEventDeviceKeyPressed(unsigned int keycode, TDEEventDevice* edevice);

	private:
		void updateBlacklists(TDEGenericDevice* hwdevice, udev_device* dev);

	private:
		TDEGenericDevice *classifyUnknownDevice(udev_device* dev, TDEGenericDevice* existingdevice=0, bool force_full_classification=true);
		TDEGenericDevice *classifyUnknownDeviceByExternalRules(udev_device* dev, TDEGenericDevice* existingdevice=0, bool classifySubDevices=false);

		void updateParentDeviceInformation();
		void updateParentDeviceInformation(TDEGenericDevice* hwdevice);

		void addCoreSystemDevices();

		/**
		* Get the binary monitor EDID for the specified sysfs path
		* @return a TQByteArray containing the EDID
		*/
		TQByteArray getEDID(TQString path);
		
		/**
		* Get the monitor EDID name for the specified sysfs path
		* @return a TQPair<TQString,TQString> containing the monitor vendor and model, if available
		*/
		TQPair<TQString,TQString> getEDIDMonitorName(TQString path);

		struct udev *m_udevStruct;
		struct udev_monitor *m_udevMonitorStruct;
		TDEGenericHardwareList m_deviceList;
		int m_procMountsFd;
		KSimpleDirWatch* m_cpuWatch;
		TQTimer* m_cpuWatchTimer;
		TQTimer* m_deviceWatchTimer;

		TQSocketNotifier* m_devScanNotifier;
		TQSocketNotifier* m_mountScanNotifier;

		TQStringList m_mountTable;
		TQStringList m_cpuInfo;

		TDEDeviceIDMap* pci_id_map;
		TDEDeviceIDMap* usb_id_map;
		TDEDeviceIDMap* pnp_id_map;
		TDEDeviceIDMap* dpy_id_map;

	friend class TDEGenericDevice;
	friend class TDEStorageDevice;
	friend class TDECPUDevice;
};

#endif // _TDEHARDWAREDEVICES_H
