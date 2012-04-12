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
	Battery,
	PowerSupply,
	Dock,
	ThermalSensor,
	ThermalControl,
	Bridge,
	Platform,
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
		* @return a TQString with the serial number, if any
		*/
		TQString &serialNumber();

		/**
		* @param a TQString with the serial number, if any
		*/
		void setSerialNumber(TQString sn);

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
		* @return a TQString with the encoded vendor, if any
		*/
		TQString &vendorEncoded();

		/**
		* @param a TQString with the encoded vendor, if any
		*/
		void setVendorEncoded(TQString id);

		/**
		* @return a TQString with the encoded model, if any
		*/
		TQString &modelEncoded();

		/**
		* @param a TQString with the encoded model, if any
		*/
		void setModelEncoded(TQString id);

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
		virtual TQString friendlyDeviceType();

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

		/**
		*  @return a TQString containing the friendly type name
		*
		* This method overrides TDEGenericDevice::friendlyDeviceType()
		*/
		TQString friendlyDeviceType();

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
		double &frequency();

		/**
		* @param a double with the current CPU frequency in MHz, if available
		*/
		void setFrequency(double fr);

		/**
		* @return a double with the minimum CPU frequency in MHz, if available
		*/
		double &minFrequency();

		/**
		* @param a double with the minimum CPU frequency in MHz, if available
		*/
		void setMinFrequency(double fr);

		/**
		* @return a double with the maximum CPU frequency in MHz, if available
		*/
		double &maxFrequency();

		/**
		* @param a double with the maximum CPU frequency in MHz, if available
		*/
		void setMaxFrequency(double fr);

		/**
		* @return a double with the transition latency in ns, if available
		*/
		double &transitionLatency();

		/**
		* @param a double with the transition latency in ns, if available
		*/
		void setTransitionLatency(double tl);

		/**
		* @return a TQString with the current CPU governor policy, if available
		*/
		TQString &governor();

		/**
		* @param a TQString with the current CPU governor policy, if available
		*/
		void setGovernor(TQString gr);

		/**
		* @return a TQString with the current CPU scaling driver, if available
		*/
		TQString &scalingDriver();

		/**
		* @param a TQString with the current CPU scaling driver, if available
		*/
		void setScalingDriver(TQString dr);

		/**
		* @return a TQStringList with the IDs of all processors that are dependent on the frequency/power settings of this one, if available
		*/
		TQStringList &dependentProcessors();

		/**
		* @param a TQStringList with the IDs of all processors that are dependent on the frequency/power settings of this one, if available
		*/
		void setDependentProcessors(TQStringList dp);

		/**
		* @return a TQStringList with all valid scaling frequencies in Hz, if available
		*/
		TQStringList &availableFrequencies();

		/**
		* @param a TQStringList with all valid scaling frequencies in Hz, if available
		*/
		void setAvailableFrequencies(TQStringList af);

	private:
		double m_frequency;
		double m_minfrequency;
		double m_maxfrequency;
		double m_transitionlatency;
		TQString m_governor;
		TQString m_scalingdriver;
		TQStringList m_tiedprocs;
		TQStringList m_frequencies;
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
		* @param a double with the current battery voltage, if available
		*/
		void setVoltage(double vt);

		/**
		* @return a double with the minimum battery voltage, if available
		*/
		double minimumVoltage();

		/**
		* @param a double with the minimum battery voltage, if available
		*/
		void setMinimumVoltage(double vt);

		/**
		* @return a double with the maximum battery voltage, if available
		*/
		double maximumVoltage();

		/**
		* @param a double with the maximum battery voltage, if available
		*/
		void setMaximumVoltage(double vt);

		/**
		* @return a double with the designed maximum battery voltage, if available
		*/
		double maximumDesignVoltage();

		/**
		* @param a double with the designed maximum battery voltage, if available
		*/
		void setMaximumDesignVoltage(double vt);

		/**
		* @return a double with the current battery energy in watt-hours, if available
		*/
		double energy();

		/**
		* @param a double with the current battery energy in watt-hours, if available
		*/
		void setEnergy(double vt);

		/**
		* @return a double with the current battery alarm energy in watt-hours, if available
		*/
		double alarmEnergy();

		/**
		* @param a double with the current battery alarm energy in watt-hours, if available
		*/
		void setAlarmEnergy(double vt);

		/**
		* @return a double with the maximum battery energy in watt-hours, if available
		*/
		double maximumEnergy();

		/**
		* @param a double with the maximum battery energy in watt-hours, if available
		*/
		void setMaximumEnergy(double vt);

		/**
		* @return a double with the designed maximum battery energy in watt-hours, if available
		*/
		double maximumDesignEnergy();

		/**
		* @param a double with the designed maximum battery energy in watt-hours, if available
		*/
		void setMaximumDesignEnergy(double vt);

		/**
		* @return a double with the current battery discharge rate in volt-hours, if available
		*/
		double dischargeRate();

		/**
		* @param a double with the current battery discharge rate in volt-hours, if available
		*/
		void setDischargeRate(double vt);

		/**
		* @return a TQString with the battery technology, if available
		*/
		TQString &technology();

		/**
		* @param a TQString with the battery technology, if available
		*/
		void setTechnology(TQString tc);

		/**
		* @return a TQString with the battery status, if available
		*/
		TQString &status();

		/**
		* @param a TQString with the battery status, if available
		*/
		void setStatus(TQString tc);

		/**
		* @return TRUE if the battery is installed
		*/
		bool installed();

		/**
		* @param TRUE if the battery is installed
		*/
		void setInstalled(bool tc);

		/**
		* @return a double with the current battery charge in percent, if available
		*/
		double chargePercent();

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
		TQString m_technology;
		TQString m_status;
		bool m_installed;
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

		/**
		* @param TRUE if power supply is online via mains power, FALSE if not
		*/
		void setOnline(bool vt);

	private:
		bool m_online;
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
		* @param ma a TQString containing the network device's MAC address
		*/
		void setMacAddress(TQString ma);

		/**
		* @return a TQString containing the network device's operational state
		*/
		TQString state();

		/**
		* @param st a TQString containing the network device's operational state
		*/
		void setState(TQString st);

		/**
		* @return TRUE if carrier is present, FALSE if not
		*/
		bool carrierPresent();

		/**
		* @param TRUE if carrier is present, FALSE if not
		*/
		void setCarrierPresent(bool cp);

		/**
		* @return TRUE if device is dormant, FALSE if not
		*/
		bool dormant();

		/**
		* @param TRUE if device is dormant, FALSE if not
		*/
		void setDormant(bool dm);

		/**
		* @return a TQString containing the network device's IPv4 address
		*/
		TQString ipV4Address();

		/**
		* @param ad a TQString containing the network device's IPv4 address
		*/
		void setIpV4Address(TQString ad);

		/**
		* @return a TQString containing the network device's IPv6 address
		*/
		TQString ipV6Address();

		/**
		* @param ad a TQString containing the network device's IPv6 address
		*/
		void setIpV6Address(TQString ad);

		/**
		* @return a TQString containing the network device's IPv4 netmask
		*/
		TQString ipV4Netmask();

		/**
		* @param nm a TQString containing the network device's IPv4 netmask
		*/
		void setIpV4Netmask(TQString nm);

		/**
		* @return a TQString containing the network device's IPv6 netmask
		*/
		TQString ipV6Netmask();

		/**
		* @param nm a TQString containing the network device's IPv6 netmask
		*/
		void setIpV6Netmask(TQString nm);

		/**
		* @return a TQString containing the network device's IPv4 broadcast
		*/
		TQString ipV4Broadcast();

		/**
		* @param br a TQString containing the network device's IPv4 broadcast
		*/
		void setIpV4Broadcast(TQString br);

		/**
		* @return a TQString containing the network device's IPv6 broadcast
		*/
		TQString ipV6Broadcast();

		/**
		* @param br a TQString containing the network device's IPv6 broadcast
		*/
		void setIpV6Broadcast(TQString br);

		/**
		* @return a TQString containing the network device's IPv4 destination
		*/
		TQString ipV4Destination();

		/**
		* @param ds a TQString containing the network device's IPv4 destination
		*/
		void setIpV4Destination(TQString ds);

		/**
		* @return a TQString containing the network device's IPv6 destination
		*/
		TQString ipV6Destination();

		/**
		* @param ds a TQString containing the network device's IPv6 destination
		*/
		void setIpV6Destination(TQString ds);

		/**
		* @return a double with the number of received bytes, if available
		*/
		double rxBytes();

		/**
		* @param rx a double with the number of received bytes, if available
		*/
		void setRxBytes(double rx);

		/**
		* @return a double with the number of transmitted bytes, if available
		*/
		double txBytes();

		/**
		* @param tx a double with the number of transmitted bytes, if available
		*/
		void setTxBytes(double tx);

		/**
		* @return a double with the number of received packets, if available
		*/
		double rxPackets();

		/**
		* @param rx a double with the number of received packets, if available
		*/
		void setRxPackets(double rx);

		/**
		* @return a double with the number of transmitted packets, if available
		*/
		double txPackets();

		/**
		* @param tx a double with the number of transmitted packets, if available
		*/
		void setTxPackets(double tx);

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

		/**
		* @param a TDESensorClusterMap with the current sensor values
		*/
		void setValues(TDESensorClusterMap cl);

	private:
		TDESensorClusterMap m_sensorValues;
};

typedef TQPtrList<TDEGenericDevice> TDEGenericHardwareList;
typedef TQMap<TQString, TQString> TDEDeviceIDMap;

class TQSocketNotifier;
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

	private slots:
		void processHotPluggedHardware();
		void processModifiedMounts();
		void processModifiedCPUs();
		void processStatelessDevices();

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

	friend class TDEGenericDevice;
	friend class TDEStorageDevice;
};

#endif