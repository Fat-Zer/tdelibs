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

#include <tdehardwaredevices.h>

#include <tqfile.h>
#include <tqdir.h>
#include <tqstringlist.h>
#include <tqsocketnotifier.h>

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktempfile.h>
#include <ksimpledirwatch.h>
#include <kstandarddirs.h>

#include <kapplication.h>
#include <dcopclient.h>

#include <libudev.h>

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

// Backlight devices
#include <linux/fb.h>

// Input devices
#include <linux/input.h>

// Network devices
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

// BEGIN BLOCK
// Copied from include/linux/genhd.h
#define GENHD_FL_REMOVABLE                      1
#define GENHD_FL_MEDIA_CHANGE_NOTIFY            4
#define GENHD_FL_CD                             8
#define GENHD_FL_UP                             16
#define GENHD_FL_SUPPRESS_PARTITION_INFO        32
#define GENHD_FL_EXT_DEVT                       64
#define GENHD_FL_NATIVE_CAPACITY                128
#define GENHD_FL_BLOCK_EVENTS_ON_EXCL_WRITE     256
// END BLOCK

// NOTE TO DEVELOPERS
// This command will greatly help when attempting to find properties to distinguish one device from another
// udevadm info --query=all --path=/sys/....

// This routine is courtsey of an answer on "Stack Overflow"
// It takes an LSB-first int and makes it an MSB-first int (or vice versa)
unsigned int reverse_bits(register unsigned int x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	return((x >> 16) | (x << 16));
}

#define BIT_IS_SET(bits, n) (bits[n >> 3] & (1 << (n & 0x7)))

TDESensorCluster::TDESensorCluster() {
	label = TQString::null;
	current = -1;
	minimum = -1;
	maximum = -1;
	warning = -1;
	critical = -1;
}

TDEGenericDevice::TDEGenericDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) {
	m_deviceType = dt;
	m_deviceName = dn;

	m_parentDevice = 0;
	m_friendlyName = TQString::null;
	m_blacklistedForUpdate = false;
}

TDEGenericDevice::~TDEGenericDevice() {
}

TDEGenericDeviceType::TDEGenericDeviceType TDEGenericDevice::type() {
	return m_deviceType;
}

TQString TDEGenericDevice::name() {
	return m_deviceName;
}

void TDEGenericDevice::internalSetName(TQString dn) {
	m_deviceName = dn;
}

TQString TDEGenericDevice::vendorName() {
	return m_vendorName;
}

void TDEGenericDevice::internalSetVendorName(TQString vn) {
	m_vendorName = vn;
}

TQString TDEGenericDevice::vendorModel() {
	return m_vendorModel;
}

void TDEGenericDevice::internalSetVendorModel(TQString vm) {
	m_vendorModel = vm;
}

TQString TDEGenericDevice::serialNumber() {
	return m_serialNumber;
}

void TDEGenericDevice::internalSetSerialNumber(TQString sn) {
	m_serialNumber = sn;
}

TQString TDEGenericDevice::systemPath() {
	if (!m_systemPath.endsWith("/")) {
		m_systemPath += "/";
	}
	return m_systemPath;
}

void TDEGenericDevice::internalSetSystemPath(TQString sp) {
	m_systemPath = sp;
}

TQString TDEGenericDevice::deviceNode() {
	return m_deviceNode;
}

void TDEGenericDevice::internalSetDeviceNode(TQString sn) {
	m_deviceNode = sn;
}

TQString TDEGenericDevice::deviceBus() {
	return m_deviceBus;
}

void TDEGenericDevice::internalSetDeviceBus(TQString db) {
	m_deviceBus = db;
}

TQString TDEGenericDevice::uniqueID() {
	m_uniqueID = m_systemPath+m_deviceNode;
	return m_uniqueID;
}

TQString TDEGenericDevice::vendorID() {
	return m_vendorID;
}

void TDEGenericDevice::internalSetVendorID(TQString id) {
	m_vendorID = id;
	m_vendorID.replace("0x", "");
}

TQString TDEGenericDevice::modelID() {
	return m_modelID;
}

void TDEGenericDevice::internalSetModelID(TQString id) {
	m_modelID = id;
	m_modelID.replace("0x", "");
}

TQString TDEGenericDevice::vendorEncoded() {
	return m_vendorenc;
}

void TDEGenericDevice::internalSetVendorEncoded(TQString id) {
	m_vendorenc = id;
}

TQString TDEGenericDevice::modelEncoded() {
	return m_modelenc;
}

void TDEGenericDevice::internalSetModelEncoded(TQString id) {
	m_modelenc = id;
}

TQString TDEGenericDevice::subVendorID() {
	return m_subvendorID;
}

void TDEGenericDevice::internalSetSubVendorID(TQString id) {
	m_subvendorID = id;
	m_subvendorID.replace("0x", "");
}

TQString TDEGenericDevice::PCIClass() {
	return m_pciClass;
}

void TDEGenericDevice::internalSetPCIClass(TQString cl) {
	m_pciClass = cl;
	m_pciClass.replace("0x", "");
}

TQString TDEGenericDevice::moduleAlias() {
	return m_modAlias;
}

void TDEGenericDevice::internalSetModuleAlias(TQString ma) {
	m_modAlias = ma;
}

TQString TDEGenericDevice::deviceDriver() {
	return m_deviceDriver;
}

void TDEGenericDevice::internalSetDeviceDriver(TQString dr) {
	m_deviceDriver = dr;
}

TQString TDEGenericDevice::subsystem() {
	return m_subsystem;
}

void TDEGenericDevice::internalSetSubsystem(TQString ss) {
	m_subsystem = ss;
}

TQString TDEGenericDevice::subModelID() {
	return m_submodelID;
}

void TDEGenericDevice::internalSetSubModelID(TQString id) {
	m_submodelID = id;
	m_submodelID.replace("0x", "");
}

void TDEGenericDevice::internalSetParentDevice(TDEGenericDevice* pd) {
	m_parentDevice = pd;
}

TDEGenericDevice* TDEGenericDevice::parentDevice() {
	return m_parentDevice;
}

TQPixmap TDEGenericDevice::icon(KIcon::StdSizes size) {
	return KGlobal::hardwareDevices()->getDeviceTypeIconFromType(type(), size);
}

bool TDEGenericDevice::blacklistedForUpdate() {
	return m_blacklistedForUpdate;
}

void TDEGenericDevice::internalSetBlacklistedForUpdate(bool bl) {
	m_blacklistedForUpdate = bl;
}

TQString TDEGenericDevice::friendlyDeviceType() {
	return KGlobal::hardwareDevices()->getFriendlyDeviceTypeStringFromType(type());
}

TQString TDEGenericDevice::busID() {
	TQString busid = m_systemPath;
	busid = busid.remove(0, busid.findRev("/")+1);
	busid = busid.remove(0, busid.find(":")+1);
	return busid;
}

TQString TDEGenericDevice::friendlyName() {
	if (m_friendlyName.isNull()) {
		if (type() == TDEGenericDeviceType::RootSystem) {
			m_friendlyName = "Linux System";
		}
		else if (type() == TDEGenericDeviceType::Root) {
			TQString friendlyDriverName = systemPath();
			friendlyDriverName.truncate(friendlyDriverName.length()-1);
			friendlyDriverName.remove(0, friendlyDriverName.findRev("/")+1);
			m_friendlyName = friendlyDriverName;
		}
		else if (m_modAlias.lower().startsWith("pci")) {
			m_friendlyName = KGlobal::hardwareDevices()->findPCIDeviceName(m_vendorID, m_modelID, m_subvendorID, m_submodelID);
		}
		else if (m_modAlias.lower().startsWith("usb")) {
			m_friendlyName = KGlobal::hardwareDevices()->findUSBDeviceName(m_vendorID, m_modelID, m_subvendorID, m_submodelID);
		}
		else {
			TQString acpigentype = systemPath();
			acpigentype.truncate(acpigentype.length()-1);
			acpigentype.remove(0, acpigentype.findRev("/")+1);
			TQString pnpgentype = acpigentype;
			pnpgentype.truncate(pnpgentype.find(":"));
			if (pnpgentype.startsWith("PNP")) {
				m_friendlyName = KGlobal::hardwareDevices()->findPNPDeviceName(pnpgentype);
			}
			else if (acpigentype.startsWith("device:")) {
				acpigentype.remove(0, acpigentype.findRev(":")+1);
				acpigentype.prepend("0x");
				m_friendlyName = i18n("ACPI Node %1").arg(acpigentype.toUInt(0,0));
			}
		}
	}

	if (m_friendlyName.isNull()) {
		// Could not identify based on model/vendor codes
		// Try to construct something from the model/vendor strings if they are available
		if (!m_vendorName.isNull() && !m_vendorModel.isNull()) {
			m_friendlyName = m_vendorName + " " + m_vendorModel;
		}
	}

	if (m_friendlyName.isNull()) {
		// Could not identify based on model/vendor
		// Guess by type
		if (type() == TDEGenericDeviceType::CPU) {
			m_friendlyName = name();
		}
		else if (type() == TDEGenericDeviceType::Event) {
			// Use parent node name
			if (m_parentDevice) {
				return m_parentDevice->friendlyName();
			}
			else {
				m_friendlyName = i18n("Generic Event Device");
			}
		}
		else if (type() == TDEGenericDeviceType::Input) {
			// Use parent node name
			if (m_parentDevice) {
				return m_parentDevice->friendlyName();
			}
			else {
				m_friendlyName = i18n("Generic Input Device");
			}
		}
		// Guess by driver
		else if (!m_deviceDriver.isNull()) {
			TQString friendlyDriverName = m_deviceDriver.lower();
			friendlyDriverName[0] = friendlyDriverName[0].upper();
			m_friendlyName = i18n("Generic %1 Device").arg(friendlyDriverName);
		}
		else if (m_systemPath.lower().startsWith("/sys/devices/virtual")) {
			TQString friendlyDriverName = systemPath();
			friendlyDriverName.truncate(friendlyDriverName.length()-1);
			friendlyDriverName.remove(0, friendlyDriverName.findRev("/")+1);
			if (!friendlyDriverName.isNull()) {
				m_friendlyName = i18n("Virtual Device %1").arg(friendlyDriverName);
			}
			else {
				m_friendlyName = i18n("Unknown Virtual Device");
			}
		}
		else {
			// I really have no idea what this peripheral is; say so!
			m_friendlyName = i18n("Unknown Device") + " " + name();
		}
	}

	return m_friendlyName;
}

TDEStorageDevice::TDEStorageDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn), m_mediaInserted(true) {
	m_diskType = TDEDiskDeviceType::Null;
	m_diskStatus = TDEDiskDeviceStatus::Null;
}

TDEStorageDevice::~TDEStorageDevice() {
}

TDEDiskDeviceType::TDEDiskDeviceType TDEStorageDevice::diskType() {
	return m_diskType;
}

void TDEStorageDevice::internalSetDiskType(TDEDiskDeviceType::TDEDiskDeviceType dt) {
	m_diskType = dt;
}

bool TDEStorageDevice::isDiskOfType(TDEDiskDeviceType::TDEDiskDeviceType tf) {
	return ((m_diskType&tf)!=TDEDiskDeviceType::Null);
}

TDEDiskDeviceStatus::TDEDiskDeviceStatus TDEStorageDevice::diskStatus() {
	return m_diskStatus;
}

void TDEStorageDevice::internalSetDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus st) {
	m_diskStatus = st;
}

bool TDEStorageDevice::checkDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus sf) {
	return ((m_diskStatus&sf)!=(TDEDiskDeviceStatus::TDEDiskDeviceStatus)0);
}

TQString TDEStorageDevice::diskLabel() {
	return m_diskName;
}

void TDEStorageDevice::internalSetDiskLabel(TQString dn) {
	m_diskName = dn;
}

bool TDEStorageDevice::mediaInserted() {
	return m_mediaInserted;
}

void TDEStorageDevice::internalSetMediaInserted(bool inserted) {
	m_mediaInserted = inserted;
}

TQString TDEStorageDevice::fileSystemName() {
	return m_fileSystemName;
}

void TDEStorageDevice::internalSetFileSystemName(TQString fn) {
	m_fileSystemName = fn;
}

TQString TDEStorageDevice::fileSystemUsage() {
	return m_fileSystemUsage;
}

void TDEStorageDevice::internalSetFileSystemUsage(TQString fu) {
	m_fileSystemUsage = fu;
}

TQString TDEStorageDevice::diskUUID() {
	return m_diskUUID;
}

void TDEStorageDevice::internalSetDiskUUID(TQString id) {
	m_diskUUID = id;
}

TQStringList TDEStorageDevice::holdingDevices() {
	return m_holdingDevices;
}

void TDEStorageDevice::internalSetHoldingDevices(TQStringList hd) {
	m_holdingDevices = hd;
}

TQStringList TDEStorageDevice::slaveDevices() {
	return m_slaveDevices;
}

void TDEStorageDevice::internalSetSlaveDevices(TQStringList sd) {
	m_slaveDevices = sd;
}

TQString TDEStorageDevice::friendlyName() {
	// Return the actual storage device name
	TQString devicevendorid = vendorEncoded();
	TQString devicemodelid = modelEncoded();

	devicevendorid.replace("\\x20", " ");
	devicemodelid.replace("\\x20", " ");

	devicevendorid = devicevendorid.stripWhiteSpace();
	devicemodelid = devicemodelid.stripWhiteSpace();
	devicevendorid = devicevendorid.simplifyWhiteSpace();
	devicemodelid = devicemodelid.simplifyWhiteSpace();

	TQString devicename = devicevendorid + " " + devicemodelid;

	devicename = devicename.stripWhiteSpace();
	devicename = devicename.simplifyWhiteSpace();

	if (devicename != "") {
		return devicename;
	}

	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		return friendlyDeviceType();
	}

	TQString label = diskLabel();
	if (label.isNull()) {
		if (deviceSize() > 0) {
			if (checkDiskStatus(TDEDiskDeviceStatus::Hotpluggable)) {
				label = i18n("%1 Removable Device").arg(deviceFriendlySize());
			}
			else {
				label = i18n("%1 Fixed Storage Device").arg(deviceFriendlySize());
			}
		}
	}

	if (!label.isNull()) {
		return label;
	}

	return friendlyDeviceType();
}

TQString TDEStorageDevice::friendlyDeviceType() {
	TQString ret = i18n("Hard Disk Drive");

	// Keep this in sync with TDEStorageDevice::icon(KIcon::StdSizes size) below
	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		ret = i18n("Floppy Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::CDROM)) {
		ret = i18n("CDROM Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::CDRW)) {
		ret = i18n("CDRW Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDROM)) {
		ret = i18n("DVD Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRW) || isDiskOfType(TDEDiskDeviceType::DVDRAM)) {
		ret = i18n("DVDRW Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRW)) {
		ret = i18n("DVDRW Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRAM)) {
		ret = i18n("DVDRAM Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Optical)) {
		ret = i18n("Optical Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Zip)) {
		ret = i18n("Zip Drive");
	}

	if (isDiskOfType(TDEDiskDeviceType::HDD)) {
		ret = i18n("Hard Disk Drive");
		if (checkDiskStatus(TDEDiskDeviceStatus::Hotpluggable)) {
			ret = i18n("Removable Storage");
		}
		if (isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
			ret = i18n("Compact Flash");
		}
		if (isDiskOfType(TDEDiskDeviceType::MemoryStick)) {
			ret = i18n("Memory Stick");
		}
	}

	if (isDiskOfType(TDEDiskDeviceType::RAM)) {
		ret = i18n("Random Access Memory");
	}
	if (isDiskOfType(TDEDiskDeviceType::Loop)) {
		ret = i18n("Loop Device");
	}

	return ret;
}

TQPixmap TDEStorageDevice::icon(KIcon::StdSizes size) {
	TQPixmap ret = DesktopIcon("hdd_unmount", size);

	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		ret = DesktopIcon("3floppy_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::CDROM)) {
		ret = DesktopIcon("cdrom_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::CDRW)) {
		ret = DesktopIcon("cdwriter_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDROM)) {
		ret = DesktopIcon("dvd_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRW)) {
		ret = DesktopIcon("dvd_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRAM)) {
		ret = DesktopIcon("dvd_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Optical)) {
		ret = DesktopIcon("cdrom_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Zip)) {
		ret = DesktopIcon("zip_unmount", size);
	}

	if (isDiskOfType(TDEDiskDeviceType::HDD)) {
		ret = DesktopIcon("hdd_unmount", size);
		if (checkDiskStatus(TDEDiskDeviceStatus::Hotpluggable)) {
			ret = DesktopIcon("usbpendrive_unmount", size);
		}
		if (isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
			ret = DesktopIcon("compact_flash_unmount", size);
		}
		if (isDiskOfType(TDEDiskDeviceType::MemoryStick)) {
			ret = DesktopIcon("memory_stick_unmount", size);
		}
	}

	if (isDiskOfType(TDEDiskDeviceType::RAM)) {
		ret = DesktopIcon("memory", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Loop)) {
		ret = DesktopIcon("blockdevice", size);
	}

	return ret;
}

unsigned long TDEStorageDevice::deviceSize() {
	TQString bsnodename = systemPath();
	bsnodename.append("/queue/physical_block_size");
	TQFile bsfile( bsnodename );
	TQString blocksize;
	if ( bsfile.open( IO_ReadOnly ) ) {
		TQTextStream stream( &bsfile );
		blocksize = stream.readLine();
		bsfile.close();
	}
	else {
		// Drat, I can't get a gauranteed block size.  Assume a block size of 512, as everything I have read indicates that /sys/block/<dev>/size is given in terms of a 512 byte block...
		blocksize = "512";
	}

	TQString dsnodename = systemPath();
	dsnodename.append("/size");
	TQFile dsfile( dsnodename );
	TQString devicesize;
	if ( dsfile.open( IO_ReadOnly ) ) {
		TQTextStream stream( &dsfile );
		devicesize = stream.readLine();
		dsfile.close();
	}

	return (blocksize.toULong()*devicesize.toULong());
}

TQString TDEStorageDevice::deviceFriendlySize() {
	return TDEHardwareDevices::bytesToFriendlySizeString(deviceSize());
}

TQString TDEStorageDevice::mountPath() {
	// See if this device node is mounted
	// This requires parsing /proc/mounts, looking for deviceNode()

	// The Device Mapper throws a monkey wrench into this
	// It likes to advertise mounts as /dev/mapper/<something>,
	// where <something> is listed in <system path>/dm/name

	// First, ensure that all device information (mainly holders/slaves) is accurate
	KGlobal::hardwareDevices()->rescanDeviceInformation(this);

	TQString dmnodename = systemPath();
	dmnodename.append("/dm/name");
	TQFile namefile( dmnodename );
	TQString dmaltname;
	if ( namefile.open( IO_ReadOnly ) ) {
		TQTextStream stream( &namefile );
		dmaltname = stream.readLine();
		namefile.close();
	}
	if (!dmaltname.isNull()) {
		dmaltname.prepend("/dev/mapper/");
	}

	TQStringList lines;
	TQFile file( "/proc/mounts" );
	if ( file.open( IO_ReadOnly ) ) {
		TQTextStream stream( &file );
		TQString line;
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			TQStringList mountInfo = TQStringList::split(" ", line, true);
			TQString testNode = *mountInfo.at(0);
			// Check for match
			if ((testNode == deviceNode()) || (testNode == dmaltname)) {
				return *mountInfo.at(1);
			}
			lines += line;
		}
		file.close();
	}

	// While this device is not directly mounted, it could concievably be mounted via the Device Mapper
	// If so, try to retrieve the mount path...
	TQStringList slaveDeviceList = holdingDevices();
	for ( TQStringList::Iterator slavedevit = slaveDeviceList.begin(); slavedevit != slaveDeviceList.end(); ++slavedevit ) {
		// Try to locate this device path in the TDE device tree
		TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();
		TDEGenericDevice *hwdevice = hwdevices->findBySystemPath(*slavedevit);
		if ((hwdevice) && (hwdevice->type() == TDEGenericDeviceType::Disk)) {
			TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
			return sdevice->mountPath();
		}
	}

	return TQString::null;
}

TQString TDEStorageDevice::mountDevice(TQString mediaName, TQString mountOptions, TQString* errRet, int* retcode) {
	int internal_retcode;
	if (!retcode) {
		retcode = &internal_retcode;
	}

	TQString ret = mountPath();

	if (!ret.isNull()) {
		return ret;
	}

	// Create dummy password file
	KTempFile passwordFile(TQString::null, "tmp", 0600);
	passwordFile.setAutoDelete(true);

	TQString command = TQString("pmount -p %1 %2 %3 2>&1").arg(passwordFile.name()).arg(mountOptions).arg(deviceNode());
	if (!mediaName.isNull()) {
		command.append(mediaName);
	}

	FILE *exepipe = popen(command.ascii(), "r");
	if (exepipe) {
		TQString pmount_output;
		char buffer[8092];
		pmount_output = fgets(buffer, sizeof(buffer), exepipe);
		*retcode = pclose(exepipe);
		if (errRet) {
			*errRet = pmount_output;
		}
	}

	// Update internal mount data
	KGlobal::hardwareDevices()->processModifiedMounts();

	ret = mountPath();

	return ret;
}

TQString TDEStorageDevice::mountEncryptedDevice(TQString passphrase, TQString mediaName, TQString mountOptions, TQString* errRet, int* retcode) {
	int internal_retcode;
	if (!retcode) {
		retcode = &internal_retcode;
	}

	TQString ret = mountPath();

	if (!ret.isNull()) {
		return ret;
	}

	// Create dummy password file
	KTempFile passwordFile(TQString::null, "tmp", 0600);
	passwordFile.setAutoDelete(true);
	TQFile* pwFile = passwordFile.file();
	if (!pwFile) {
		return TQString::null;
	}

	pwFile->writeBlock(passphrase.ascii(), passphrase.length());
	pwFile->flush();

	TQString command = TQString("pmount -p %1 %2 %3 2>&1").arg(passwordFile.name()).arg(mountOptions).arg(deviceNode());
	if (!mediaName.isNull()) {
		command.append(mediaName);
	}

	FILE *exepipe = popen(command.ascii(), "r");
	if (exepipe) {
		TQString pmount_output;
		char buffer[8092];
		pmount_output = fgets(buffer, sizeof(buffer), exepipe);
		*retcode = pclose(exepipe);
		if (errRet) {
			*errRet = pmount_output;
		}
	}

	// Update internal mount data
	KGlobal::hardwareDevices()->processModifiedMounts();

	ret = mountPath();

	return ret;
}

bool TDEStorageDevice::unmountDevice(TQString* errRet, int* retcode) {
	int internal_retcode;
	if (!retcode) {
		retcode = &internal_retcode;
	}

	TQString mountpoint = mountPath();

	if (mountpoint.isNull()) {
		return true;
	}

	TQString command = TQString("pumount %1 2>&1").arg(mountpoint);
	FILE *exepipe = popen(command.ascii(), "r");
	if (exepipe) {
		TQString pmount_output;
		char buffer[8092];
		pmount_output = fgets(buffer, sizeof(buffer), exepipe);
		*retcode = pclose(exepipe);
		if (*retcode == 0) {
			return true;
		}
		else {
			if (errRet) {
				*errRet = pmount_output;
			}
		}
	}

	// Update internal mount data
	KGlobal::hardwareDevices()->processModifiedMounts();

	return false;
}

TDECPUDevice::TDECPUDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDECPUDevice::~TDECPUDevice() {
}

double TDECPUDevice::frequency() {
	return m_frequency;
}

void TDECPUDevice::internalSetFrequency(double fr) {
	m_frequency = fr;
}

double TDECPUDevice::minFrequency() {
	return m_minfrequency;
}

void TDECPUDevice::internalSetMinFrequency(double fr) {
	m_minfrequency = fr;
}

double TDECPUDevice::maxFrequency() {
	return m_maxfrequency;
}

void TDECPUDevice::internalSetMaxFrequency(double fr) {
	m_maxfrequency = fr;
}

double TDECPUDevice::transitionLatency() {
	return m_transitionlatency;
}

void TDECPUDevice::internalSetTransitionLatency(double tl) {
	m_transitionlatency = tl;
}

TQString TDECPUDevice::governor() {
	return m_governor;
}

void TDECPUDevice::internalSetGovernor(TQString gr) {
	m_governor = gr;
}

TQString TDECPUDevice::scalingDriver() {
	return m_scalingdriver;
}

void TDECPUDevice::internalSetScalingDriver(TQString dr) {
	m_scalingdriver = dr;
}

TQStringList TDECPUDevice::dependentProcessors() {
	return m_tiedprocs;
}

void TDECPUDevice::internalSetDependentProcessors(TQStringList dp) {
	m_tiedprocs = dp;
}

TQStringList TDECPUDevice::availableFrequencies() {
	return m_frequencies;
}

void TDECPUDevice::internalSetAvailableFrequencies(TQStringList af) {
	m_frequencies = af;
}

TQStringList TDECPUDevice::availableGovernors() {
	return m_governers;
}

void TDECPUDevice::internalSetAvailableGovernors(TQStringList gp) {
	m_governers = gp;
}

bool TDECPUDevice::canSetGovernor() {
	TQString governornode = systemPath() + "/cpufreq/scaling_governor";
	int rval = access (governornode.ascii(), W_OK);
	if (rval == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void TDECPUDevice::setGovernor(TQString gv) {
	TQString governornode = systemPath() + "/cpufreq/scaling_governor";
	TQFile file( governornode );
	if ( file.open( IO_WriteOnly ) ) {
		TQTextStream stream( &file );
		stream << gv.lower();
		file.close();
	}
}

TDESensorDevice::TDESensorDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDESensorDevice::~TDESensorDevice() {
}

TDESensorClusterMap TDESensorDevice::values() {
	return m_sensorValues;
}

void TDESensorDevice::internalSetValues(TDESensorClusterMap cl) {
	m_sensorValues = cl;
}

TDERootSystemDevice::TDERootSystemDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
	m_hibernationSpace = -1;
}

TDERootSystemDevice::~TDERootSystemDevice() {
}

TDESystemFormFactor::TDESystemFormFactor TDERootSystemDevice::formFactor() {
	return m_formFactor;
}

void TDERootSystemDevice::internalSetFormFactor(TDESystemFormFactor::TDESystemFormFactor ff) {
	m_formFactor = ff;
}

TDESystemPowerStateList TDERootSystemDevice::powerStates() {
	return m_powerStates;
}

void TDERootSystemDevice::internalSetPowerStates(TDESystemPowerStateList ps) {
	m_powerStates = ps;
}

TDESystemHibernationMethodList TDERootSystemDevice::hibernationMethods() {
	return m_hibernationMethods;
}

void TDERootSystemDevice::internalSetHibernationMethods(TDESystemHibernationMethodList hm) {
	m_hibernationMethods = hm;
}

TDESystemHibernationMethod::TDESystemHibernationMethod TDERootSystemDevice::hibernationMethod() {
	return m_hibernationMethod;
}

void TDERootSystemDevice::internalSetHibernationMethod(TDESystemHibernationMethod::TDESystemHibernationMethod hm) {
	m_hibernationMethod = hm;
}

unsigned long TDERootSystemDevice::diskSpaceNeededForHibernation() {
	return m_hibernationSpace;
}

void TDERootSystemDevice::internalSetDiskSpaceNeededForHibernation(unsigned long sz) {
	m_hibernationSpace = sz;
}

bool TDERootSystemDevice::canSetHibernationMethod() {
	TQString hibernationnode = "/sys/power/disk";
	int rval = access (hibernationnode.ascii(), W_OK);
	if (rval == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

bool TDERootSystemDevice::canStandby() {
	TQString statenode = "/sys/power/state";
	int rval = access (statenode.ascii(), W_OK);
	if (rval == 0) {
		if (powerStates().contains(TDESystemPowerState::Standby)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

bool TDERootSystemDevice::canSuspend() {
	TQString statenode = "/sys/power/state";
	int rval = access (statenode.ascii(), W_OK);
	if (rval == 0) {
		if (powerStates().contains(TDESystemPowerState::Suspend)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

bool TDERootSystemDevice::canHibernate() {
	TQString statenode = "/sys/power/state";
	int rval = access (statenode.ascii(), W_OK);
	if (rval == 0) {
		if (powerStates().contains(TDESystemPowerState::Hibernate)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

bool TDERootSystemDevice::canPowerOff() {
	// FIXME
	// Can we power down this system?
	// This should probably be checked via DCOP and therefore interface with KDM

	KConfig *config = KGlobal::config();
	config->reparseConfiguration(); // config may have changed in the KControl module
	
	config->setGroup("General" );
	bool maysd = false;
	if (config->readBoolEntry( "offerShutdown", true )/* && DM().canShutdown()*/) {	// FIXME
		maysd = true;
	}

	return maysd;
}

void TDERootSystemDevice::setHibernationMethod(TDESystemHibernationMethod::TDESystemHibernationMethod hm) {
	TQString hibernationnode = "/sys/power/disk";
	TQFile file( hibernationnode );
	if ( file.open( IO_WriteOnly ) ) {
		TQString hibernationCommand;
		if (hm == TDESystemHibernationMethod::Platform) {
			hibernationCommand = "platform";
		}
		if (hm == TDESystemHibernationMethod::Shutdown) {
			hibernationCommand = "shutdown";
		}
		if (hm == TDESystemHibernationMethod::Reboot) {
			hibernationCommand = "reboot";
		}
		if (hm == TDESystemHibernationMethod::TestProc) {
			hibernationCommand = "testproc";
		}
		if (hm == TDESystemHibernationMethod::Test) {
			hibernationCommand = "test";
		}
		TQTextStream stream( &file );
		stream << hibernationCommand;
		file.close();
	}
}

bool TDERootSystemDevice::setPowerState(TDESystemPowerState::TDESystemPowerState ps) {
	if ((ps == TDESystemPowerState::Standby) || (ps == TDESystemPowerState::Suspend) || (ps == TDESystemPowerState::Hibernate)) {
		TQString statenode = "/sys/power/state";
		TQFile file( statenode );
		if ( file.open( IO_WriteOnly ) ) {
			TQString powerCommand;
			if (ps == TDESystemPowerState::Standby) {
				powerCommand = "standby";
			}
			if (ps == TDESystemPowerState::Suspend) {
				powerCommand = "mem";
			}
			if (ps == TDESystemPowerState::Hibernate) {
				powerCommand = "disk";
			}
			TQTextStream stream( &file );
			stream << powerCommand;
			file.close();
			return true;
		}
	}
	else if (ps == TDESystemPowerState::PowerOff) {
		// Power down the system using a DCOP command
		// Values are explained at http://lists.kde.org/?l=kde-linux&m=115770988603387
		TQByteArray data;
		TQDataStream arg(data, IO_WriteOnly);
		arg << (int)0 << (int)2 << (int)2;
		if ( kapp->dcopClient()->send("ksmserver", "default", "logout(int,int,int)", data) ) {
			return true;
		}
		return false;
	}
	else if (ps == TDESystemPowerState::Active) {
		// Ummm...we're already active...
		return true;
	}

	return false;
}

TDEBatteryDevice::TDEBatteryDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDEBatteryDevice::~TDEBatteryDevice() {
}

double TDEBatteryDevice::voltage() {
	return m_currentVoltage;
}

void TDEBatteryDevice::internalSetVoltage(double vt) {
	m_currentVoltage = vt;
}

double TDEBatteryDevice::maximumVoltage() {
	return m_maximumVoltage;
}

void TDEBatteryDevice::internalSetMaximumVoltage(double vt) {
	m_maximumVoltage = vt;
}

double TDEBatteryDevice::minimumVoltage() {
	return m_minimumVoltage;
}

void TDEBatteryDevice::internalSetMinimumVoltage(double vt) {
	m_minimumVoltage = vt;
}

double TDEBatteryDevice::maximumDesignVoltage() {
	return m_maximumDesignVoltage;
}

void TDEBatteryDevice::internalSetMaximumDesignVoltage(double vt) {
	m_maximumDesignVoltage = vt;
}

double TDEBatteryDevice::energy() {
	return m_currentEnergy;
}

void TDEBatteryDevice::internalSetEnergy(double vt) {
	m_currentEnergy = vt;
}

double TDEBatteryDevice::alarmEnergy() {
	return m_alarmEnergy;
}

void TDEBatteryDevice::internalSetAlarmEnergy(double vt) {
	m_alarmEnergy = vt;
}

double TDEBatteryDevice::maximumEnergy() {
	return m_maximumEnergy;
}

void TDEBatteryDevice::internalSetMaximumEnergy(double vt) {
	m_maximumEnergy = vt;
}

double TDEBatteryDevice::maximumDesignEnergy() {
	return m_maximumDesignEnergy;
}

void TDEBatteryDevice::internalSetMaximumDesignEnergy(double vt) {
	m_maximumDesignEnergy = vt;
}

double TDEBatteryDevice::dischargeRate() {
	return m_dischargeRate;
}

void TDEBatteryDevice::internalSetDischargeRate(double vt) {
	m_dischargeRate = vt;
}

TQString TDEBatteryDevice::technology() {
	return m_technology;
}

void TDEBatteryDevice::internalSetTechnology(TQString tc) {
	m_technology = tc;
}

TQString TDEBatteryDevice::status() {
	return m_status;
}

void TDEBatteryDevice::internalSetStatus(TQString tc) {
	m_status = tc;
}

bool TDEBatteryDevice::installed() {
	return m_installed;
}

void TDEBatteryDevice::internalSetInstalled(bool tc) {
	m_installed = tc;
}

double TDEBatteryDevice::chargePercent() {
	return (m_currentEnergy/m_maximumEnergy)*100.0;
}

TDEMainsPowerDevice::TDEMainsPowerDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDEMainsPowerDevice::~TDEMainsPowerDevice() {
}

bool TDEMainsPowerDevice::online() {
	return m_online;
}

void TDEMainsPowerDevice::internalSetOnline(bool tc) {
	m_online = tc;
}

TDENetworkDevice::TDENetworkDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
	m_rxbytes = -1;
	m_txbytes = -1;
	m_rxpackets = -1;
	m_txpackets = -1;
}

TDENetworkDevice::~TDENetworkDevice() {
}

TQString TDENetworkDevice::macAddress() {
	return m_macAddress;
}

void TDENetworkDevice::internalSetMacAddress(TQString ma) {
	m_macAddress = ma;
}

TQString TDENetworkDevice::state() {
	return m_state;
}

void TDENetworkDevice::internalSetState(TQString st) {
	m_state = st;
}

bool TDENetworkDevice::carrierPresent() {
	return m_carrier;
}

void TDENetworkDevice::internalSetCarrierPresent(bool cp) {
	m_carrier = cp;
}

bool TDENetworkDevice::dormant() {
	return m_dormant;
}

void TDENetworkDevice::internalSetDormant(bool dm) {
	m_dormant = dm;
}

TQString TDENetworkDevice::ipV4Address() {
	return m_ipV4Address;
}

void TDENetworkDevice::internalSetIpV4Address(TQString ad) {
	m_ipV4Address = ad;
}

TQString TDENetworkDevice::ipV6Address() {
	return m_ipV6Address;
}

void TDENetworkDevice::internalSetIpV6Address(TQString ad) {
	m_ipV6Address = ad;
}

TQString TDENetworkDevice::ipV4Netmask() {
	return m_ipV4Netmask;
}

void TDENetworkDevice::internalSetIpV4Netmask(TQString nm) {
	m_ipV4Netmask = nm;
}

TQString TDENetworkDevice::ipV6Netmask() {
	return m_ipV6Netmask;
}

void TDENetworkDevice::internalSetIpV6Netmask(TQString nm) {
	m_ipV6Netmask = nm;
}

TQString TDENetworkDevice::ipV4Broadcast() {
	return m_ipV4Broadcast;
}

void TDENetworkDevice::internalSetIpV4Broadcast(TQString br) {
	m_ipV4Broadcast = br;
}

TQString TDENetworkDevice::ipV6Broadcast() {
	return m_ipV6Broadcast;
}

void TDENetworkDevice::internalSetIpV6Broadcast(TQString br) {
	m_ipV6Broadcast = br;
}

TQString TDENetworkDevice::ipV4Destination() {
	return m_ipV4Destination;
}

void TDENetworkDevice::internalSetIpV4Destination(TQString ds) {
	m_ipV4Destination = ds;
}

TQString TDENetworkDevice::ipV6Destination() {
	return m_ipV6Destination;
}

void TDENetworkDevice::internalSetIpV6Destination(TQString ds) {
	m_ipV6Destination = ds;
}

double TDENetworkDevice::rxBytes() {
	return m_rxbytes;
}

void TDENetworkDevice::internalSetRxBytes(double rx) {
	m_rxbytes = rx;
}

double TDENetworkDevice::txBytes() {
	return m_txbytes;
}

void TDENetworkDevice::internalSetTxBytes(double tx) {
	m_txbytes = tx;
}

double TDENetworkDevice::rxPackets() {
	return m_rxpackets;
}

void TDENetworkDevice::internalSetRxPackets(double rx) {
	m_rxpackets = rx;
}

double TDENetworkDevice::txPackets() {
	return m_txpackets;
}

void TDENetworkDevice::internalSetTxPackets(double tx) {
	m_txpackets = tx;
}

TDEBacklightDevice::TDEBacklightDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDEBacklightDevice::~TDEBacklightDevice() {
}

TDEDisplayPowerLevel::TDEDisplayPowerLevel TDEBacklightDevice::powerLevel() {
	return m_powerLevel;
}

void TDEBacklightDevice::internalSetPowerLevel(TDEDisplayPowerLevel::TDEDisplayPowerLevel pl) {
	m_powerLevel = pl;
}

void TDEBacklightDevice::internalSetMaximumRawBrightness(int br) {
	m_maximumBrightness = br;
}

void TDEBacklightDevice::internalSetCurrentRawBrightness(int br) {
	m_currentBrightness = br;
}

int TDEBacklightDevice::brightnessSteps() {
	return m_maximumBrightness + 1;
}

double TDEBacklightDevice::brightnessPercent() {
	return (((m_currentBrightness*1.0)/m_maximumBrightness)*100.0);
}

bool TDEBacklightDevice::canSetBrightness() {
	TQString brightnessnode = systemPath() + "/brightness";
	int rval = access (brightnessnode.ascii(), W_OK);
	if (rval == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

int TDEBacklightDevice::rawBrightness() {
	return m_currentBrightness;
}

void TDEBacklightDevice::setRawBrightness(int br) {
	TQString brightnessnode = systemPath() + "/brightness";
	TQFile file( brightnessnode );
	if ( file.open( IO_WriteOnly ) ) {
		TQString brightnessCommand;
		brightnessCommand = TQString("%1").arg(br);
		TQTextStream stream( &file );
		stream << brightnessCommand;
		file.close();
	}
}

TDEMonitorDevice::TDEMonitorDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDEMonitorDevice::~TDEMonitorDevice() {
}

bool TDEMonitorDevice::connected() {
	return m_connected;
}

void TDEMonitorDevice::internalSetConnected(bool cn) {
	m_connected = cn;
}

bool TDEMonitorDevice::enabled() {
	return m_enabled;
}

void TDEMonitorDevice::internalSetEnabled(bool en) {
	m_enabled = en;
}

TQByteArray TDEMonitorDevice::edid() {
	return m_edid;
}

void TDEMonitorDevice::internalSetEdid(TQByteArray ed) {
	m_edid = ed;
}

TDEResolutionList TDEMonitorDevice::resolutions() {
	return m_resolutions;
}

void TDEMonitorDevice::internalSetResolutions(TDEResolutionList rs) {
	m_resolutions = rs;
}

TQString TDEMonitorDevice::portType() {
	return m_portType;
}

void TDEMonitorDevice::internalSetPortType(TQString pt) {
	m_portType = pt;
}

TDEDisplayPowerLevel::TDEDisplayPowerLevel TDEMonitorDevice::powerLevel() {
	return m_powerLevel;
}

void TDEMonitorDevice::internalSetPowerLevel(TDEDisplayPowerLevel::TDEDisplayPowerLevel pl) {
	m_powerLevel = pl;
}

TDEEventDevice::TDEEventDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
	m_fd = -1;
}

TDEEventDevice::~TDEEventDevice() {
	if (m_fd >= 0) {
		close(m_fd);
	}
}

TDEEventDeviceType::TDEEventDeviceType TDEEventDevice::eventType() {
	return m_eventType;
}

void TDEEventDevice::internalSetEventType(TDEEventDeviceType::TDEEventDeviceType et) {
	m_eventType = et;
}

TDESwitchType::TDESwitchType TDEEventDevice::providedSwitches() {
	return m_providedSwitches;
}

void TDEEventDevice::internalSetProvidedSwitches(TDESwitchType::TDESwitchType sl) {
	m_providedSwitches = sl;
}

TDESwitchType::TDESwitchType TDEEventDevice::activeSwitches() {
	return m_switchActive;
}

void TDEEventDevice::internalSetActiveSwitches(TDESwitchType::TDESwitchType sl) {
	m_switchActive = sl;
}

// Keep this in sync with the TDESwitchType definition in the header
TQStringList TDEEventDevice::friendlySwitchList(TDESwitchType::TDESwitchType switches) {
	TQStringList ret;

	if (switches & TDESwitchType::Lid) {
		ret.append(i18n("Lid Switch"));
	}
	if (switches & TDESwitchType::TabletMode) {
		ret.append(i18n("Tablet Mode"));
	}
	if (switches & TDESwitchType::HeadphoneInsert) {
		ret.append(i18n("Headphone Inserted"));
	}
	if (switches & TDESwitchType::RFKill) {
		ret.append(i18n("Radio Frequency Device Kill Switch"));
	}
	if (switches & TDESwitchType::Radio) {
		ret.append(i18n("Enable Radio"));
	}
	if (switches & TDESwitchType::MicrophoneInsert) {
		ret.append(i18n("Microphone Inserted"));
	}
	if (switches & TDESwitchType::Dock) {
		ret.append(i18n("Docked"));
	}
	if (switches & TDESwitchType::LineOutInsert) {
		ret.append(i18n("Line Out Inserted"));
	}
	if (switches & TDESwitchType::JackPhysicalInsert) {
		ret.append(i18n("Physical Jack Inserted"));
	}
	if (switches & TDESwitchType::VideoOutInsert) {
		ret.append(i18n("Video Out Inserted"));
	}
	if (switches & TDESwitchType::CameraLensCover) {
		ret.append(i18n("Camera Lens Cover"));
	}
	if (switches & TDESwitchType::KeypadSlide) {
		ret.append(i18n("Keypad Slide"));
	}
	if (switches & TDESwitchType::FrontProximity) {
		ret.append(i18n("Front Proximity"));
	}
	if (switches & TDESwitchType::RotateLock) {
		ret.append(i18n("Rotate Lock"));
	}
	if (switches & TDESwitchType::LineInInsert) {
		ret.append(i18n("Line In Inserted"));
	}

	return ret;
}

TDEInputDevice::TDEInputDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDEInputDevice::~TDEInputDevice() {
}

TDEInputDeviceType::TDEInputDeviceType TDEInputDevice::inputType() {
	return m_inputType;
}

void TDEInputDevice::internalSetInputType(TDEInputDeviceType::TDEInputDeviceType it) {
	m_inputType = it;
}

TDEHardwareDevices::TDEHardwareDevices() {
	// Initialize members
	pci_id_map = 0;
	usb_id_map = 0;
	pnp_id_map = 0;
	dpy_id_map = 0;

	// Set up device list
	m_deviceList.setAutoDelete( TRUE );	// the list owns the objects

	// Initialize udev interface
	m_udevStruct = udev_new();
	if (!m_udevStruct) {
		printf("Unable to create udev interface\n\r");
	}

	if (m_udevStruct) {
		// Set up device add/remove monitoring
		m_udevMonitorStruct = udev_monitor_new_from_netlink(m_udevStruct, "udev");
		udev_monitor_filter_add_match_subsystem_devtype(m_udevMonitorStruct, NULL, NULL);
		udev_monitor_enable_receiving(m_udevMonitorStruct);

		m_devScanNotifier = new TQSocketNotifier(udev_monitor_get_fd(m_udevMonitorStruct), TQSocketNotifier::Read, this);
		connect( m_devScanNotifier, TQT_SIGNAL(activated(int)), this, TQT_SLOT(processHotPluggedHardware()) );

		// Read in the current mount table
		// Yes, a race condition exists between this and the mount monitor start below, but it shouldn't be a problem 99.99% of the time
		m_mountTable.clear();
		TQFile file( "/proc/mounts" );
		if ( file.open( IO_ReadOnly ) ) {
			TQTextStream stream( &file );
			while ( !stream.atEnd() ) {
				m_mountTable.append(stream.readLine());
			}
			file.close();
		}

		// Monitor for changed mounts
		m_procMountsFd = open("/proc/mounts", O_RDONLY, 0);
		m_mountScanNotifier = new TQSocketNotifier(m_procMountsFd, TQSocketNotifier::Exception, this);
		connect( m_mountScanNotifier, TQT_SIGNAL(activated(int)), this, TQT_SLOT(processModifiedMounts()) );

		// Read in the current cpu information
		// Yes, a race condition exists between this and the cpu monitor start below, but it shouldn't be a problem 99.99% of the time
		m_cpuInfo.clear();
		TQFile cpufile( "/proc/cpuinfo" );
		if ( cpufile.open( IO_ReadOnly ) ) {
			TQTextStream stream( &cpufile );
			while ( !stream.atEnd() ) {
				m_cpuInfo.append(stream.readLine());
			}
			cpufile.close();
		}

// [FIXME 0.01]
// Apparently the Linux kernel just does not notify userspace applications of CPU frequency changes
// This is STUPID, as it means I have to poll the CPU information structures with a 0.5 second or so timer just to keep the information up to date
#if 0
		// Monitor for changed cpu information
		// Watched directories are set up during the initial CPU scan
		m_cpuWatch = new KSimpleDirWatch(this);
		connect( m_cpuWatch, TQT_SIGNAL(dirty(const TQString &)), this, TQT_SLOT(processModifiedCPUs()) );
#else
		m_cpuWatchTimer = new TQTimer(this);
		connect( m_cpuWatchTimer, SIGNAL(timeout()), this, SLOT(processModifiedCPUs()) );
		TQDir nodezerocpufreq("/sys/devices/system/cpu/cpu0/cpufreq");
		if (nodezerocpufreq.exists()) {
			m_cpuWatchTimer->start( 500, FALSE ); // 0.5 second repeating timer
		}
#endif

		// Some devices do not receive update signals from udev
		// These devices must be polled, and a good polling interval is 1 second
		m_deviceWatchTimer = new TQTimer(this);
		connect( m_deviceWatchTimer, SIGNAL(timeout()), this, SLOT(processStatelessDevices()) );
		m_deviceWatchTimer->start( 1000, FALSE ); // 1 second repeating timer

		// Update internal device information
		queryHardwareInformation();
	}
}

TDEHardwareDevices::~TDEHardwareDevices() {
	// Stop device scanning
	m_deviceWatchTimer->stop();

// [FIXME 0.01]
#if 0
	// Stop CPU scanning
	m_cpuWatch->stopScan();
#else
	m_cpuWatchTimer->stop();
#endif

	// Stop mount scanning
	close(m_procMountsFd);

	// Tear down udev interface
	udev_unref(m_udevStruct);

	// Delete members
	if (pci_id_map) {
		delete pci_id_map;
	}
	if (usb_id_map) {
		delete usb_id_map;
	}
	if (pnp_id_map) {
		delete pnp_id_map;
	}
	if (dpy_id_map) {
		delete dpy_id_map;
	}
}

void TDEHardwareDevices::rescanDeviceInformation(TDEGenericDevice* hwdevice) {
	struct udev_device *dev;
	dev = udev_device_new_from_syspath(m_udevStruct, hwdevice->systemPath().ascii());
	classifyUnknownDevice(dev, hwdevice, false);
	updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
	udev_device_unref(dev);
}

TDEGenericDevice* TDEHardwareDevices::findBySystemPath(TQString syspath) {
	if (!syspath.endsWith("/")) {
		syspath += "/";
	}
	TDEGenericDevice *hwdevice;
	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	TDEGenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		if (hwdevice->systemPath() == syspath) {
			return hwdevice;
		}
	}

	return 0;
}

TDEGenericDevice* TDEHardwareDevices::findByUniqueID(TQString uid) {
	TDEGenericDevice *hwdevice;
	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	TDEGenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		if (hwdevice->uniqueID() == uid) {
			return hwdevice;
		}
	}

	return 0;
}

TDEGenericDevice* TDEHardwareDevices::findByDeviceNode(TQString devnode) {
	TDEGenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->deviceNode() == devnode) {
			return hwdevice;
		}
	}

	return 0;
}

TDEStorageDevice* TDEHardwareDevices::findDiskByUID(TQString uid) {
	TDEGenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->type() == TDEGenericDeviceType::Disk) {
			TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
			if (sdevice->uniqueID() == uid) {
				return sdevice;
			}
		}
	}

	return 0;
}

void TDEHardwareDevices::processHotPluggedHardware() {
	udev_device* dev = udev_monitor_receive_device(m_udevMonitorStruct);
	if (dev) {
		TQString actionevent(udev_device_get_action(dev));
		if (actionevent == "add") {
			TDEGenericDevice* device = classifyUnknownDevice(dev);
	
			// Make sure this device is not a duplicate
			TDEGenericDevice *hwdevice;
			for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
				if (hwdevice->systemPath() == device->systemPath()) {
					delete device;
					device = 0;
					break;
				}
			}
	
			if (device) {
				m_deviceList.append(device);
				updateParentDeviceInformation(device);	// Update parent/child tables for this device
				emit hardwareAdded(device);
			}
		}
		else if (actionevent == "remove") {
			// Delete device from hardware listing
			TQString systempath(udev_device_get_syspath(dev));
			TDEGenericDevice *hwdevice;
			for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
				if (hwdevice->systemPath() == systempath) {
					emit hardwareRemoved(hwdevice);

					// If the device is a storage device and has a slave, update it as well
					if (hwdevice->type() == TDEGenericDeviceType::Disk) {
						TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
						TQStringList slavedevices = sdevice->slaveDevices();
						m_deviceList.remove(hwdevice);
						for ( TQStringList::Iterator slaveit = slavedevices.begin(); slaveit != slavedevices.end(); ++slaveit ) {
							TDEGenericDevice* slavedevice = findBySystemPath(*slaveit);
							if (slavedevice) {
								rescanDeviceInformation(slavedevice);
								emit hardwareUpdated(slavedevice);
							}
						}
					}
					else {
						m_deviceList.remove(hwdevice);
					}

					break;
				}
			}
		}
		else if (actionevent == "change") {
			// Update device and emit change event
			TQString systempath(udev_device_get_syspath(dev));
			TDEGenericDevice *hwdevice;
			for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
				if (hwdevice->systemPath() == systempath) {
					if (!hwdevice->blacklistedForUpdate()) {
						classifyUnknownDevice(dev, hwdevice, false);
						updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
						emit hardwareUpdated(hwdevice);
					}
					break;
				}
			}
		}
	}
}

void TDEHardwareDevices::processModifiedCPUs() {
	// Detect what changed between the old cpu information and the new information,
	// and emit appropriate events

	// Read new CPU information table
	m_cpuInfo.clear();
	TQFile cpufile( "/proc/cpuinfo" );
	if ( cpufile.open( IO_ReadOnly ) ) {
		TQTextStream stream( &cpufile );
		while ( !stream.atEnd() ) {
			m_cpuInfo.append(stream.readLine());
		}
		cpufile.close();
	}

	// Parse CPU information table
	TDECPUDevice *cdevice;
	cdevice = 0;
	bool modified = false;

	TQString curline;
	int processorNumber = 0;
	int processorCount = 0;
	for (TQStringList::Iterator cpuit = m_cpuInfo.begin(); cpuit != m_cpuInfo.end(); ++cpuit) {
		// WARNING This routine assumes that "processor" is always the first entry in /proc/cpuinfo!
		curline = *cpuit;
		if (curline.startsWith("processor")) {
			curline.remove(0, curline.find(":")+1);
			curline = curline.stripWhiteSpace();
			processorNumber = curline.toInt();
			cdevice = dynamic_cast<TDECPUDevice*>(findBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber)));
		}
		if (curline.startsWith("model name")) {
			curline.remove(0, curline.find(":")+1);
			curline = curline.stripWhiteSpace();
			if (cdevice->name() != curline) modified = true;
			cdevice->internalSetName(curline);
		}
		if (curline.startsWith("cpu MHz")) {
			curline.remove(0, curline.find(":")+1);
			curline = curline.stripWhiteSpace();
			if (cdevice->frequency() != curline.toDouble()) modified = true;
			cdevice->internalSetFrequency(curline.toDouble());
		}
		if (curline.startsWith("vendor_id")) {
			curline.remove(0, curline.find(":")+1);
			curline = curline.stripWhiteSpace();
			if (cdevice->vendorName() != curline) modified = true;
			cdevice->internalSetVendorName(curline);
			if (cdevice->vendorEncoded() != curline) modified = true;
			cdevice->internalSetVendorEncoded(curline);
		}
	}

	processorCount = processorNumber+1;

	// Read in other information from cpufreq, if available
	for (processorNumber=0; processorNumber<processorCount; processorNumber++) {
		cdevice = dynamic_cast<TDECPUDevice*>(findBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber)));
		TQDir cpufreq_dir(TQString("/sys/devices/system/cpu/cpu%1/cpufreq").arg(processorNumber));
		TQString scalinggovernor;
		TQString scalingdriver;
		double minfrequency = -1;
		double maxfrequency = -1;
		double trlatency = -1;
		TQStringList affectedcpulist;
		TQStringList frequencylist;
		TQStringList governorlist;
		if (cpufreq_dir.exists()) {
			TQString nodename = cpufreq_dir.path();
			nodename.append("/scaling_governor");
			TQFile scalinggovernorfile(nodename);
			if (scalinggovernorfile.open(IO_ReadOnly)) {
				TQTextStream stream( &scalinggovernorfile );
				scalinggovernor = stream.readLine();
				scalinggovernorfile.close();
			}
			nodename = cpufreq_dir.path();
			nodename.append("/scaling_driver");
			TQFile scalingdriverfile(nodename);
			if (scalingdriverfile.open(IO_ReadOnly)) {
				TQTextStream stream( &scalingdriverfile );
				scalingdriver = stream.readLine();
				scalingdriverfile.close();
			}
			nodename = cpufreq_dir.path();
			nodename.append("/scaling_min_freq");
			TQFile minfrequencyfile(nodename);
			if (minfrequencyfile.open(IO_ReadOnly)) {
				TQTextStream stream( &minfrequencyfile );
				minfrequency = stream.readLine().toDouble()/1000.0;
				minfrequencyfile.close();
			}
			nodename = cpufreq_dir.path();
			nodename.append("/scaling_max_freq");
			TQFile maxfrequencyfile(nodename);
			if (maxfrequencyfile.open(IO_ReadOnly)) {
				TQTextStream stream( &maxfrequencyfile );
				maxfrequency = stream.readLine().toDouble()/1000.0;
				maxfrequencyfile.close();
			}
			nodename = cpufreq_dir.path();
			nodename.append("/cpuinfo_transition_latency");
			TQFile trlatencyfile(nodename);
			if (trlatencyfile.open(IO_ReadOnly)) {
				TQTextStream stream( &trlatencyfile );
				trlatency = stream.readLine().toDouble()/1000.0;
				trlatencyfile.close();
			}
			nodename = cpufreq_dir.path();
			nodename.append("/affected_cpus");
			TQFile tiedcpusfile(nodename);
			if (tiedcpusfile.open(IO_ReadOnly)) {
				TQTextStream stream( &tiedcpusfile );
				affectedcpulist = TQStringList::split(" ", stream.readLine());
				tiedcpusfile.close();
			}
			nodename = cpufreq_dir.path();
			nodename.append("/scaling_available_frequencies");
			TQFile availfreqsfile(nodename);
			if (availfreqsfile.open(IO_ReadOnly)) {
				TQTextStream stream( &availfreqsfile );
				frequencylist = TQStringList::split(" ", stream.readLine());
				availfreqsfile.close();
			}
			nodename = cpufreq_dir.path();
			nodename.append("/scaling_available_governors");
			TQFile availgvrnsfile(nodename);
			if (availgvrnsfile.open(IO_ReadOnly)) {
				TQTextStream stream( &availgvrnsfile );
				governorlist = TQStringList::split(" ", stream.readLine());
				availgvrnsfile.close();
			}
		}

		// Update CPU information structure
		if (cdevice->governor() != scalinggovernor) modified = true;
		cdevice->internalSetGovernor(scalinggovernor);
		if (cdevice->scalingDriver() != scalingdriver) modified = true;
		cdevice->internalSetScalingDriver(scalingdriver);
		if (cdevice->minFrequency() != minfrequency) modified = true;
		cdevice->internalSetMinFrequency(minfrequency);
		if (cdevice->maxFrequency() != maxfrequency) modified = true;
		cdevice->internalSetMaxFrequency(maxfrequency);
		if (cdevice->transitionLatency() != trlatency) modified = true;
		cdevice->internalSetTransitionLatency(trlatency);
		if (cdevice->dependentProcessors().join(" ") != affectedcpulist.join(" ")) modified = true;
		cdevice->internalSetDependentProcessors(affectedcpulist);
		if (cdevice->availableFrequencies().join(" ") != frequencylist.join(" ")) modified = true;
		cdevice->internalSetAvailableFrequencies(frequencylist);
		if (cdevice->availableGovernors().join(" ") != governorlist.join(" ")) modified = true;
		cdevice->internalSetAvailableGovernors(governorlist);
	}

	if (modified) {
		for (processorNumber=0; processorNumber<processorCount; processorNumber++) {
			TDEGenericDevice* hwdevice = findBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber));
			// Signal new information available
			emit hardwareUpdated(hwdevice);
		}
	}
}

void TDEHardwareDevices::processStatelessDevices() {
	// Some devices do not emit changed signals
	// So far, network cards and sensors need to be polled
	TDEGenericDevice *hwdevice;

	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	TDEGenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		if ((hwdevice->type() == TDEGenericDeviceType::RootSystem) || (hwdevice->type() == TDEGenericDeviceType::Network) || (hwdevice->type() == TDEGenericDeviceType::OtherSensor) || (hwdevice->type() == TDEGenericDeviceType::Event)) {
			rescanDeviceInformation(hwdevice);
			emit hardwareUpdated(hwdevice);
		}
	}
}

void TDEHardwareDevices::processModifiedMounts() {
	// Detect what changed between the old mount table and the new one,
	// and emit appropriate events

	TQStringList deletedEntries = m_mountTable;

	// Read in the new mount table
	m_mountTable.clear();
	TQFile file( "/proc/mounts" );
	if ( file.open( IO_ReadOnly ) ) {
		TQTextStream stream( &file );
		while ( !stream.atEnd() ) {
			m_mountTable.append(stream.readLine());
		}
		file.close();
	}

	TQStringList addedEntries = m_mountTable;

	// Remove all entries that are identical in both tables
	processModifiedMounts_removeagain:
	for ( TQStringList::Iterator delit = deletedEntries.begin(); delit != deletedEntries.end(); ++delit ) {
		for ( TQStringList::Iterator addit = addedEntries.begin(); addit != addedEntries.end(); ++addit ) {
			if ((*delit) == (*addit)) {
				deletedEntries.remove(delit);
				addedEntries.remove(addit);
				// Reset iterators to prevent bugs/crashes
				// FIXME
				// Is there any way to completely reset both loops without using goto?
				goto processModifiedMounts_removeagain;
			}
		}
	}

	TQStringList::Iterator it;
	for ( it = addedEntries.begin(); it != addedEntries.end(); ++it ) {
		TQStringList mountInfo = TQStringList::split(" ", (*it), true);
		// Try to find a device that matches the altered node
		TDEGenericDevice* hwdevice = findByDeviceNode(*mountInfo.at(0));
		if (hwdevice) {
			emit hardwareUpdated(hwdevice);
			// If the device is a storage device and has a slave, update it as well
			if (hwdevice->type() == TDEGenericDeviceType::Disk) {
				TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
				TQStringList slavedevices = sdevice->slaveDevices();
				for ( TQStringList::Iterator slaveit = slavedevices.begin(); slaveit != slavedevices.end(); ++slaveit ) {
					TDEGenericDevice* slavedevice = findBySystemPath(*slaveit);
					if (slavedevice) {
						emit hardwareUpdated(slavedevice);
					}
				}
			}
		}
	}
	for ( it = deletedEntries.begin(); it != deletedEntries.end(); ++it ) {
		TQStringList mountInfo = TQStringList::split(" ", (*it), true);
		// Try to find a device that matches the altered node
		TDEGenericDevice* hwdevice = findByDeviceNode(*mountInfo.at(0));
		if (hwdevice) {
			emit hardwareUpdated(hwdevice);
			// If the device is a storage device and has a slave, update it as well
			if (hwdevice->type() == TDEGenericDeviceType::Disk) {
				TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
				TQStringList slavedevices = sdevice->slaveDevices();
				for ( TQStringList::Iterator slaveit = slavedevices.begin(); slaveit != slavedevices.end(); ++slaveit ) {
					TDEGenericDevice* slavedevice = findBySystemPath(*slaveit);
					if (slavedevice) {
						emit hardwareUpdated(slavedevice);
					}
				}
			}
		}
	}

	emit mountTableModified();
}

TDEDiskDeviceType::TDEDiskDeviceType classifyDiskType(udev_device* dev, const TQString &devicebus, const TQString &disktypestring, const TQString &systempath, const TQString &devicevendor, const TQString &devicemodel, const TQString &filesystemtype, const TQString &devicedriver) {
	// Classify a disk device type to the best of our ability
	TDEDiskDeviceType::TDEDiskDeviceType disktype = TDEDiskDeviceType::Null;

	if (devicebus.upper() == "USB") {
		disktype = disktype | TDEDiskDeviceType::USB;
	}

	if (disktypestring.upper() == "ZIP") {
		disktype = disktype | TDEDiskDeviceType::Zip;
	}
	if ((devicevendor.upper() == "IOMEGA") && (devicemodel.upper().contains("ZIP"))) {
		disktype = disktype | TDEDiskDeviceType::Zip;
	}

	if ((devicevendor.upper() == "APPLE") && (devicemodel.upper().contains("IPOD"))) {
		disktype = disktype | TDEDiskDeviceType::MediaDevice;
	}
	if ((devicevendor.upper() == "SANDISK") && (devicemodel.upper().contains("SANSA"))) {
		disktype = disktype | TDEDiskDeviceType::MediaDevice;
	}

	if (disktypestring.upper() == "TAPE") {
		disktype = disktype | TDEDiskDeviceType::Tape;
	}

	if (disktypestring.upper() == "COMPACT_FLASH") {
		disktype = disktype | TDEDiskDeviceType::CompactFlash;
	}

	if (disktypestring.upper() == "MEMORY_STICK") {
		disktype = disktype | TDEDiskDeviceType::MemoryStick;
	}

	if (disktypestring.upper() == "SMART_MEDIA") {
		disktype = disktype | TDEDiskDeviceType::SmartMedia;
	}

	if (disktypestring.upper() == "SD_MMC") {
		disktype = disktype | TDEDiskDeviceType::SDMMC;
	}

	if (disktypestring.upper() == "FLASHKEY") {
		disktype = disktype | TDEDiskDeviceType::Flash;
	}

	if (disktypestring.upper() == "OPTICAL") {
		disktype = disktype | TDEDiskDeviceType::Optical;
	}

	if (disktypestring.upper() == "JAZ") {
		disktype = disktype | TDEDiskDeviceType::Jaz;
	}

	if (disktypestring.upper() == "DISK") {
		disktype = disktype | TDEDiskDeviceType::HDD;
	}
	if (disktypestring.isNull()) {
		// Fallback
		// If we can't recognize the disk type then set it as a simple HDD volume
		disktype = disktype | TDEDiskDeviceType::HDD;
	}

	// Certain combinations of media flags should never be set at the same time as they don't make sense
	// This block is needed as udev is more than happy to provide inconsistent data to us
	if ((disktype & TDEDiskDeviceType::Zip) || (disktype & TDEDiskDeviceType::Floppy) || (disktype & TDEDiskDeviceType::Jaz)) {
		disktype = disktype & ~TDEDiskDeviceType::HDD;
	}

	if (disktypestring.upper() == "CD") {
		disktype = disktype & ~TDEDiskDeviceType::HDD;
		disktype = disktype | TDEDiskDeviceType::Optical;

		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) == "1") {
			disktype = disktype | TDEDiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_CD_RW")) == "1") {
			disktype = disktype | TDEDiskDeviceType::CDRW;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RAM")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDRAM;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_R")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_R_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_R")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_MINUS_R")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_R_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_MINUS_R_DL")) == "1")
			) {
			disktype = disktype | TDEDiskDeviceType::DVDRW;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RW_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_MINUS_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_MINUS_RW_DL")) == "1")
			) {
			disktype = disktype | TDEDiskDeviceType::DVDRW;	// FIXME
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD")) == "1") {
			disktype = disktype | TDEDiskDeviceType::BDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_R")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_R_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_PLUS_R")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_MINUS_R")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_R_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_MINUS_R_DL")) == "1")
			) {
			disktype = disktype | TDEDiskDeviceType::BDRW;	// FIXME
			disktype = disktype & ~TDEDiskDeviceType::BDROM;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_RW_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_PLUS_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_MINUS_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_MINUS_RW_DL")) == "1")
			) {
			disktype = disktype | TDEDiskDeviceType::BDRW;
			disktype = disktype & ~TDEDiskDeviceType::BDROM;
		}
		if (!TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO")).isNull()) {
			disktype = disktype | TDEDiskDeviceType::CDAudio;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_VCD")) == "1") || (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_SDVD")) == "1")) {
			disktype = disktype | TDEDiskDeviceType::CDVideo;
		}
	}

	// Detect RAM and Loop devices, since udev can't seem to...
	if (systempath.startsWith("/sys/devices/virtual/block/ram")) {
		disktype = disktype | TDEDiskDeviceType::RAM;
	}
	if (systempath.startsWith("/sys/devices/virtual/block/loop")) {
		disktype = disktype | TDEDiskDeviceType::Loop;
	}

	if (filesystemtype.upper() == "CRYPTO_LUKS") {
		disktype = disktype | TDEDiskDeviceType::LUKS;
	}
	else if (filesystemtype.upper() == "CRYPTO") {
		disktype = disktype | TDEDiskDeviceType::OtherCrypted;
	}

	return disktype;
}

	// KStandardDirs::kde_default

typedef TQMap<TQString, TQString> KConfigMap;

TQString readUdevAttribute(udev_device* dev, TQString attr) {
	return TQString(udev_device_get_property_value(dev, attr.ascii()));
}

TDEGenericDeviceType::TDEGenericDeviceType readGenericDeviceTypeFromString(TQString query) {
	TDEGenericDeviceType::TDEGenericDeviceType ret = TDEGenericDeviceType::Other;

	// Keep this in sync with the TDEGenericDeviceType definition in the header
	if (query == "Root") {
		ret = TDEGenericDeviceType::Root;
	}
	else if (query == "RootSystem") {
		ret = TDEGenericDeviceType::RootSystem;
	}
	else if (query == "CPU") {
		ret = TDEGenericDeviceType::CPU;
	}
	else if (query == "GPU") {
		ret = TDEGenericDeviceType::GPU;
	}
	else if (query == "RAM") {
		ret = TDEGenericDeviceType::RAM;
	}
	else if (query == "Bus") {
		ret = TDEGenericDeviceType::Bus;
	}
	else if (query == "I2C") {
		ret = TDEGenericDeviceType::I2C;
	}
	else if (query == "MDIO") {
		ret = TDEGenericDeviceType::MDIO;
	}
	else if (query == "Mainboard") {
		ret = TDEGenericDeviceType::Mainboard;
	}
	else if (query == "Disk") {
		ret = TDEGenericDeviceType::Disk;
	}
	else if (query == "SCSI") {
		ret = TDEGenericDeviceType::SCSI;
	}
	else if (query == "StorageController") {
		ret = TDEGenericDeviceType::StorageController;
	}
	else if (query == "Mouse") {
		ret = TDEGenericDeviceType::Mouse;
	}
	else if (query == "Keyboard") {
		ret = TDEGenericDeviceType::Keyboard;
	}
	else if (query == "HID") {
		ret = TDEGenericDeviceType::HID;
	}
	else if (query == "Monitor") {
		ret = TDEGenericDeviceType::Monitor;
	}
	else if (query == "Network") {
		ret = TDEGenericDeviceType::Network;
	}
	else if (query == "Printer") {
		ret = TDEGenericDeviceType::Printer;
	}
	else if (query == "Scanner") {
		ret = TDEGenericDeviceType::Scanner;
	}
	else if (query == "Sound") {
		ret = TDEGenericDeviceType::Sound;
	}
	else if (query == "VideoCapture") {
		ret = TDEGenericDeviceType::VideoCapture;
	}
	else if (query == "IEEE1394") {
		ret = TDEGenericDeviceType::IEEE1394;
	}
	else if (query == "PCMCIA") {
		ret = TDEGenericDeviceType::PCMCIA;
	}
	else if (query == "Camera") {
		ret = TDEGenericDeviceType::Camera;
	}
	else if (query == "Serial") {
		ret = TDEGenericDeviceType::Serial;
	}
	else if (query == "Parallel") {
		ret = TDEGenericDeviceType::Parallel;
	}
	else if (query == "TextIO") {
		ret = TDEGenericDeviceType::TextIO;
	}
	else if (query == "Peripheral") {
		ret = TDEGenericDeviceType::Peripheral;
	}
	else if (query == "Backlight") {
		ret = TDEGenericDeviceType::Backlight;
	}
	else if (query == "Battery") {
		ret = TDEGenericDeviceType::Battery;
	}
	else if (query == "Power") {
		ret = TDEGenericDeviceType::PowerSupply;
	}
	else if (query == "Dock") {
		ret = TDEGenericDeviceType::Dock;
	}
	else if (query == "ThermalSensor") {
		ret = TDEGenericDeviceType::ThermalSensor;
	}
	else if (query == "ThermalControl") {
		ret = TDEGenericDeviceType::ThermalControl;
	}
	else if (query == "Bridge") {
		ret = TDEGenericDeviceType::Bridge;
	}
	else if (query == "Platform") {
		ret = TDEGenericDeviceType::Platform;
	}
	else if (query == "Event") {
		ret = TDEGenericDeviceType::Event;
	}
	else if (query == "Input") {
		ret = TDEGenericDeviceType::Input;
	}
	else if (query == "PNP") {
		ret = TDEGenericDeviceType::PNP;
	}
	else if (query == "OtherACPI") {
		ret = TDEGenericDeviceType::OtherACPI;
	}
	else if (query == "OtherUSB") {
		ret = TDEGenericDeviceType::OtherUSB;
	}
	else if (query == "OtherMultimedia") {
		ret = TDEGenericDeviceType::OtherMultimedia;
	}
	else if (query == "OtherPeripheral") {
		ret = TDEGenericDeviceType::OtherPeripheral;
	}
	else if (query == "OtherSensor") {
		ret = TDEGenericDeviceType::OtherSensor;
	}
	else if (query == "OtherVirtual") {
		ret = TDEGenericDeviceType::OtherVirtual;
	}
	else {
		ret = TDEGenericDeviceType::Other;
	}

	return ret;
}

TDEDiskDeviceType::TDEDiskDeviceType readDiskDeviceSubtypeFromString(TQString query, TDEDiskDeviceType::TDEDiskDeviceType flagsIn=TDEDiskDeviceType::Null) {
	TDEDiskDeviceType::TDEDiskDeviceType ret = flagsIn;

	// Keep this in sync with the TDEDiskDeviceType definition in the header
	if (query == "MediaDevice") {
		ret = ret | TDEDiskDeviceType::MediaDevice;
	}
	if (query == "Floppy") {
		ret = ret | TDEDiskDeviceType::Floppy;
	}
	if (query == "CDROM") {
		ret = ret | TDEDiskDeviceType::CDROM;
	}
	if (query == "CDRW") {
		ret = ret | TDEDiskDeviceType::CDRW;
	}
	if (query == "DVDROM") {
		ret = ret | TDEDiskDeviceType::DVDROM;
	}
	if (query == "DVDRAM") {
		ret = ret | TDEDiskDeviceType::DVDRAM;
	}
	if (query == "DVDRW") {
		ret = ret | TDEDiskDeviceType::DVDRW;
	}
	if (query == "BDROM") {
		ret = ret | TDEDiskDeviceType::BDROM;
	}
	if (query == "BDRW") {
		ret = ret | TDEDiskDeviceType::BDRW;
	}
	if (query == "Zip") {
		ret = ret | TDEDiskDeviceType::Zip;
	}
	if (query == "Jaz") {
		ret = ret | TDEDiskDeviceType::Jaz;
	}
	if (query == "Camera") {
		ret = ret | TDEDiskDeviceType::Camera;
	}
	if (query == "LUKS") {
		ret = ret | TDEDiskDeviceType::LUKS;
	}
	if (query == "OtherCrypted") {
		ret = ret | TDEDiskDeviceType::OtherCrypted;
	}
	if (query == "CDAudio") {
		ret = ret | TDEDiskDeviceType::CDAudio;
	}
	if (query == "CDVideo") {
		ret = ret | TDEDiskDeviceType::CDVideo;
	}
	if (query == "DVDVideo") {
		ret = ret | TDEDiskDeviceType::DVDVideo;
	}
	if (query == "BDVideo") {
		ret = ret | TDEDiskDeviceType::BDVideo;
	}
	if (query == "Flash") {
		ret = ret | TDEDiskDeviceType::Flash;
	}
	if (query == "USB") {
		ret = ret | TDEDiskDeviceType::USB;
	}
	if (query == "Tape") {
		ret = ret | TDEDiskDeviceType::Tape;
	}
	if (query == "HDD") {
		ret = ret | TDEDiskDeviceType::HDD;
	}
	if (query == "Optical") {
		ret = ret | TDEDiskDeviceType::Optical;
	}
	if (query == "RAM") {
		ret = ret | TDEDiskDeviceType::RAM;
	}
	if (query == "Loop") {
		ret = ret | TDEDiskDeviceType::Loop;
	}
	if (query == "CompactFlash") {
		ret = ret | TDEDiskDeviceType::CompactFlash;
	}
	if (query == "MemoryStick") {
		ret = ret | TDEDiskDeviceType::MemoryStick;
	}
	if (query == "SmartMedia") {
		ret = ret | TDEDiskDeviceType::SmartMedia;
	}
	if (query == "SDMMC") {
		ret = ret | TDEDiskDeviceType::SDMMC;
	}
	if (query == "UnlockedCrypt") {
		ret = ret | TDEDiskDeviceType::UnlockedCrypt;
	}

	return ret;
}

TDEGenericDevice* createDeviceObjectForType(TDEGenericDeviceType::TDEGenericDeviceType type) {
	TDEGenericDevice* ret = 0;

	if (type == TDEGenericDeviceType::Disk) {
		ret = new TDEStorageDevice(type);
	}
	else {
		ret = new TDEGenericDevice(type);
	}

	return ret;
}

TDEGenericDevice* TDEHardwareDevices::classifyUnknownDeviceByExternalRules(udev_device* dev, TDEGenericDevice* existingdevice, bool classifySubDevices) {
	// This routine expects to see the hardware config files into <prefix>/share/apps/tdehwlib/deviceclasses/, suffixed with "hwclass"
	TDEGenericDevice* device = existingdevice;
	if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Other);

	// Handle subtype if needed/desired
	// To speed things up we rely on the prior scan results stored in m_externalSubtype
	if (classifySubDevices) {
		if (!device->m_externalRulesFile.isNull()) {
			if (device->type() == TDEGenericDeviceType::Disk) {
				// Disk class
				TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
				TQStringList subtype = device->m_externalSubtype;
				TDEDiskDeviceType::TDEDiskDeviceType desiredSubdeviceType = TDEDiskDeviceType::Null;
				if (subtype.count()>0) {
					for ( TQStringList::Iterator paramit = subtype.begin(); paramit != subtype.end(); ++paramit ) {
						desiredSubdeviceType = readDiskDeviceSubtypeFromString(*paramit, desiredSubdeviceType);
					}
					if (desiredSubdeviceType != sdevice->diskType()) {
						printf("[tdehardwaredevices] Rules file %s used to set device subtype for device at path %s\n\r", device->m_externalRulesFile.ascii(), device->systemPath().ascii()); fflush(stdout);
						sdevice->internalSetDiskType(desiredSubdeviceType);
					}
				}
			}
		}
	}
	else {
		TQStringList hardware_info_directories(KGlobal::dirs()->resourceDirs("data"));
		TQString hardware_info_directory_suffix("tdehwlib/deviceclasses/");
		TQString hardware_info_directory;
	
		// Scan the hardware_info_directory for configuration files
		// For each one, open it with KConfig() and apply its rules to classify the device
		// FIXME
		// Should this also scan up to <n> subdirectories for the files?  That feature might end up being too expensive...

		device->m_externalRulesFile = TQString::null;
		for ( TQStringList::Iterator it = hardware_info_directories.begin(); it != hardware_info_directories.end(); ++it ) {
			hardware_info_directory = (*it);
			hardware_info_directory += hardware_info_directory_suffix;
	
			if (KGlobal::dirs()->exists(hardware_info_directory)) {
				TQDir d(hardware_info_directory);
				d.setFilter( TQDir::Files | TQDir::Hidden );
				
				const TQFileInfoList *list = d.entryInfoList();
				TQFileInfoListIterator it( *list );
				TQFileInfo *fi;
			
				while ((fi = it.current()) != 0) {
					if (fi->extension(false) == "hwclass") {
						bool match = true;
		
						// Open the rules file
						KConfig rulesFile(fi->absFilePath(), true, false);
						rulesFile.setGroup("Conditions");
						KConfigMap conditionmap = rulesFile.entryMap("Conditions");
						KConfigMap::Iterator cndit;
						for (cndit = conditionmap.begin(); cndit != conditionmap.end(); ++cndit) {
							TQStringList conditionList = TQStringList::split(',', cndit.data(), false);
							bool atleastonematch = false;
							for ( TQStringList::Iterator paramit = conditionList.begin(); paramit != conditionList.end(); ++paramit ) {
								if (cndit.key() == "VENDOR_ID") {
									if (device->vendorID() == (*paramit)) {
										atleastonematch = true;
									}
								}
								else if (cndit.key() == "MODEL_ID") {
									if (device->modelID() == (*paramit)) {
										atleastonematch = true;
									}
								}
								else if (cndit.key() == "DRIVER") {
									if (device->deviceDriver() == (*paramit)) {
										atleastonematch = true;
									}
								}
								else if (readUdevAttribute(dev, cndit.key()) == (*paramit)) {
									atleastonematch = true;
								}
							}
							if (!atleastonematch) {
								match = false;
							}
						}
		
						if (match) {
							rulesFile.setGroup("DeviceType");
							TQString gentype = rulesFile.readEntry("GENTYPE");
							TDEGenericDeviceType::TDEGenericDeviceType desiredDeviceType = device->type();
							if (!gentype.isNull()) {
								desiredDeviceType = readGenericDeviceTypeFromString(gentype);
							}
		
							// Handle main type
							if (desiredDeviceType != device->type()) {
								printf("[tdehardwaredevices] Rules file %s used to set device type for device at path %s\n\r", fi->absFilePath().ascii(), device->systemPath().ascii()); fflush(stdout);
								if (m_deviceList.contains(device)) {
									m_deviceList.remove(device);
								}
								else {
									delete device;
								}
								device = createDeviceObjectForType(desiredDeviceType);
							}
	
							// Parse subtype and store in m_externalSubtype for later
							// This speeds things up considerably due to the expense of the file scanning/parsing/matching operation
							device->m_externalSubtype = rulesFile.readListEntry("SUBTYPE", ',');
							device->m_externalRulesFile = fi->absFilePath();

							// Process blacklist entries
							rulesFile.setGroup("DeviceSettings");
							device->internalSetBlacklistedForUpdate(rulesFile.readBoolEntry("UPDATE_BLACKLISTED", device->blacklistedForUpdate()));
						}
					}
					++it;
				}
			}
		}
	}

	return device;
}

TDEGenericDevice* TDEHardwareDevices::classifyUnknownDevice(udev_device* dev, TDEGenericDevice* existingdevice, bool force_full_classification) {
	// Classify device and create TDEHW device object
	TQString devicename;
	TQString devicetype;
	TQString devicedriver;
	TQString devicesubsystem;
	TQString devicenode;
	TQString systempath;
	TQString devicevendorid;
	TQString devicemodelid;
	TQString devicevendoridenc;
	TQString devicemodelidenc;
	TQString devicesubvendorid;
	TQString devicesubmodelid;
	TQString devicetypestring;
	TQString devicetypestring_alt;
	TQString devicepciclass;
	TDEGenericDevice* device = existingdevice;
	if (dev) {
		devicename = (udev_device_get_sysname(dev));
		devicetype = (udev_device_get_devtype(dev));
		devicedriver = (udev_device_get_driver(dev));
		devicesubsystem = (udev_device_get_subsystem(dev));
		devicenode = (udev_device_get_devnode(dev));
		systempath = (udev_device_get_syspath(dev));
		devicevendorid = (udev_device_get_property_value(dev, "ID_VENDOR_ID"));
		devicemodelid = (udev_device_get_property_value(dev, "ID_MODEL_ID"));
		devicevendoridenc = (udev_device_get_property_value(dev, "ID_VENDOR_ENC"));
		devicemodelidenc = (udev_device_get_property_value(dev, "ID_MODEL_ENC"));
		devicesubvendorid = (udev_device_get_property_value(dev, "ID_SUBVENDOR_ID"));
		devicesubmodelid = (udev_device_get_property_value(dev, "ID_SUBMODEL_ID"));
		devicetypestring = (udev_device_get_property_value(dev, "ID_TYPE"));
		devicetypestring_alt = (udev_device_get_property_value(dev, "DEVTYPE"));
		devicepciclass = (udev_device_get_property_value(dev, "PCI_CLASS"));
	}
	else {
		if (device) {
			devicename = device->name();
			devicetype = device->m_udevtype;
			devicedriver = device->deviceDriver();
			devicesubsystem = device->subsystem();
			devicenode = device->deviceNode();
			systempath = device->systemPath();
			devicevendorid = device->vendorID();
			devicemodelid = device->modelID();
			devicevendoridenc = device->vendorEncoded();
			devicemodelidenc = device->modelEncoded();
			devicesubvendorid = device->subVendorID();
			devicesubmodelid = device->subModelID();
			devicetypestring = device->m_udevdevicetypestring;
			devicetypestring_alt = device->udevdevicetypestring_alt;
			devicepciclass = device->PCIClass();
		}
	}

	// FIXME
	// Only a small subset of devices are classified right now
	// Figure out the remaining udev logic to classify the rest!
	// Helpful file: http://www.enlightenment.org/svn/e/trunk/PROTO/enna-explorer/src/bin/udev.c

	bool done = false;
	TQString current_path = systempath;
	TQString devicemodalias = TQString::null;

	while (done == false) {
		TQString malnodename = current_path;
		malnodename.append("/modalias");
		TQFile malfile(malnodename);
		if (malfile.open(IO_ReadOnly)) {
			TQTextStream stream( &malfile );
			devicemodalias = stream.readLine();
			malfile.close();
		}
		if (devicemodalias.startsWith("pci") || devicemodalias.startsWith("usb")) {
			done = true;
		}
		else {
			devicemodalias = TQString::null;
			current_path.truncate(current_path.findRev("/"));
			if (!current_path.startsWith("/sys/devices")) {
				// Abort!
				done = true;
			}
		}
	}

	// Many devices do not provide their vendor/model ID via udev
	// Go after it manually...
	if (devicevendorid.isNull() || devicemodelid.isNull()) {
		if (devicemodalias != TQString::null) {
			// For added fun the device string lengths differ between pci and usb
			if (devicemodalias.startsWith("pci")) {
				int vloc = devicemodalias.find("v");
				int dloc = devicemodalias.find("d", vloc);
				int svloc = devicemodalias.find("sv");
				int sdloc = devicemodalias.find("sd", vloc);

				devicevendorid = devicemodalias.mid(vloc+1, 8).lower();
				devicemodelid = devicemodalias.mid(dloc+1, 8).lower();
				if (svloc != -1) {
					devicesubvendorid = devicemodalias.mid(svloc+1, 8).lower();
					devicesubmodelid = devicemodalias.mid(sdloc+1, 8).lower();
				}
				devicevendorid.remove(0,4);
				devicemodelid.remove(0,4);
				devicesubvendorid.remove(0,4);
				devicesubmodelid.remove(0,4);
			}
			if (devicemodalias.startsWith("usb")) {
				int vloc = devicemodalias.find("v");
				int dloc = devicemodalias.find("p", vloc);
				int svloc = devicemodalias.find("sv");
				int sdloc = devicemodalias.find("sp", vloc);

				devicevendorid = devicemodalias.mid(vloc+1, 4).lower();
				devicemodelid = devicemodalias.mid(dloc+1, 4).lower();
				if (svloc != -1) {
					devicesubvendorid = devicemodalias.mid(svloc+1, 4).lower();
					devicesubmodelid = devicemodalias.mid(sdloc+1, 4).lower();
				}
			}
		}
	}

	// Most of the time udev doesn't barf up a device driver either, so go after it manually...
	if (devicedriver.isNull()) {
		TQString driverSymlink = udev_device_get_syspath(dev);
		TQString driverSymlinkDir = driverSymlink;
		driverSymlink.append("/device/driver");
		driverSymlinkDir.append("/device/");
		TQFileInfo dirfi(driverSymlink);
		if (dirfi.isSymLink()) {
			char* collapsedPath = realpath((driverSymlinkDir + dirfi.readLink()).ascii(), NULL);
			devicedriver = TQString(collapsedPath);
			free(collapsedPath);
			devicedriver.remove(0, devicedriver.findRev("/")+1);
		}
	}

	// udev removes critical leading zeroes in the PCI device class, so go after it manually...
	TQString classnodename = systempath;
	classnodename.append("/class");
	TQFile classfile( classnodename );
	if ( classfile.open( IO_ReadOnly ) ) {
		TQTextStream stream( &classfile );
		devicepciclass = stream.readLine();
		devicepciclass.replace("0x", "");
		devicepciclass = devicepciclass.lower();
		classfile.close();
	}

	// Classify generic device type and create appropriate object

	// Pull out all event special devices and stuff them under Event
	TQString syspath_tail = systempath.lower();
	syspath_tail.remove(0, syspath_tail.findRev("/")+1);
	if (syspath_tail.startsWith("event")) {
		if (!device) device = new TDEEventDevice(TDEGenericDeviceType::Event);
	}
	// Pull out all input special devices and stuff them under Input
	if (syspath_tail.startsWith("input")) {
		if (!device) device = new TDEInputDevice(TDEGenericDeviceType::Input);
	}

	// Check for keyboard
	// Linux doesn't actually ID the keyboard device itself as such, it instead IDs the input device that is underneath the actual keyboard itseld
	// Therefore we need to scan <syspath>/input/input* for the ID_INPUT_KEYBOARD attribute
	bool is_keyboard = false;
	TQString inputtopdirname = udev_device_get_syspath(dev);
	inputtopdirname.append("/input/");
	TQDir inputdir(inputtopdirname);
	inputdir.setFilter(TQDir::All);
	const TQFileInfoList *dirlist = inputdir.entryInfoList();
	if (dirlist) {
		TQFileInfoListIterator inputdirsit(*dirlist);
		TQFileInfo *dirfi;
		while ( (dirfi = inputdirsit.current()) != 0 ) {
			if ((dirfi->fileName() != ".") && (dirfi->fileName() != "..")) {
				struct udev_device *slavedev;
				slavedev = udev_device_new_from_syspath(m_udevStruct, (inputtopdirname + dirfi->fileName()).ascii());
				if (udev_device_get_property_value(slavedev, "ID_INPUT_KEYBOARD") != 0) {
					is_keyboard = true;
				}
				udev_device_unref(slavedev);
			}
			++inputdirsit;
		}
	}
	if (is_keyboard) {
		if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Keyboard);
	}

	// Classify specific known devices
	if (((devicetype == "disk")
		|| (devicetype == "partition")
		|| (devicedriver == "floppy")
		|| (devicesubsystem == "scsi_disk"))
		&& ((devicenode != "")
		)) {
		if (!device) device = new TDEStorageDevice(TDEGenericDeviceType::Disk);
	}
	else if (devicetype.isNull()) {
		if (devicesubsystem == "acpi") {
			// If the ACPI device exposes a system path ending in /PNPxxxx:yy, the device type can be precisely determined
			// See ftp://ftp.microsoft.com/developr/drg/plug-and-play/devids.txt for more information
			TQString pnpgentype = systempath;
			pnpgentype.remove(0, pnpgentype.findRev("/")+1);
			pnpgentype.truncate(pnpgentype.find(":"));
			if (pnpgentype.startsWith("PNP")) {
				// If a device has been classified as belonging to the ACPI subsystem usually there is a "real" device related to it elsewhere in the system
				// Furthermore, the "real" device elsewhere almost always has more functionality exposed via sysfs
				// Therefore all ACPI subsystem devices should be stuffed in the OtherACPI category and largely ignored
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherACPI);
			}
			else {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherACPI);
			}
		}
		else if (devicesubsystem == "input") {
			// Figure out if this device is a mouse, keyboard, or something else
			// Check for mouse
			// udev doesn't reliably help here, so guess from the device name
			if (systempath.contains("/mouse")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Mouse);
			}
			if (!device) {
				// Second mouse check 
				// Look for ID_INPUT_MOUSE property presence
				if (udev_device_get_property_value(dev, "ID_INPUT_MOUSE") != 0) {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Mouse);
				}
			}
			if (!device) {
				// Check for keyboard
				// Look for ID_INPUT_KEYBOARD property presence
				if (udev_device_get_property_value(dev, "ID_INPUT_KEYBOARD") != 0) {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Keyboard);
				}
			}
			if (!device) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::HID);
			}
		}
		else if (devicesubsystem == "tty") {
			if (devicenode.contains("/ttyS")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Serial);
			}
			else {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::TextIO);
			}
		}
		else if (devicesubsystem == "thermal") {
			// FIXME
			// Figure out a way to differentiate between ThermalControl (fans and coolers) and ThermalSensor types
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::ThermalControl);
		}
		else if (devicesubsystem == "hwmon") {
			// FIXME
			// This might pick up thermal sensors
			if (!device) device = new TDESensorDevice(TDEGenericDeviceType::OtherSensor);
		}
	}

	// Try to at least generally classify unclassified devices
	if (device == 0) {
		if (devicesubsystem == "backlight") {
			if (!device) device = new TDEBacklightDevice(TDEGenericDeviceType::Backlight);
		}
		if (systempath.lower().startsWith("/sys/devices/virtual")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherVirtual);
		}
		if ((devicetypestring == "audio")
			|| (devicesubsystem == "sound")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Sound);
		}
		if ((devicesubsystem == "video4linux")
			|| (devicesubsystem == "dvb")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::VideoCapture);
		}
		if ((devicetypestring_alt == "scsi_target")
			|| (devicesubsystem == "scsi_host")
			|| (devicesubsystem == "scsi_disk")
			|| (devicesubsystem == "scsi_device")
			|| (devicesubsystem == "scsi_generic")
			|| (devicesubsystem == "scsi")
			|| (devicesubsystem == "ata_port")
			|| (devicesubsystem == "ata_link")
			|| (devicesubsystem == "ata_disk")
			|| (devicesubsystem == "ata_device")
			|| (devicesubsystem == "ata")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		if (devicesubsystem == "leds") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherACPI);
		}
		if (devicesubsystem == "net") {
			if (!device) device = new TDENetworkDevice(TDEGenericDeviceType::Network);
		}
		if (devicesubsystem == "i2c") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::I2C);
		}
		if (devicesubsystem == "mdio_bus") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::MDIO);
		}
		if (devicesubsystem == "graphics") {
			if (devicenode.isNull()) {	// GPUs do not have associated device nodes
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::GPU);
			}
			else {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
			}
		}
		if ((devicesubsystem == "event_source")
			|| (devicesubsystem == "rtc")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Mainboard);
		}
		if (devicesubsystem == "bsg") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::SCSI);
		}
		if (devicesubsystem == "firewire") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::IEEE1394);
		}
		if (devicesubsystem == "drm") {
			if (devicenode.isNull()) {	// Monitors do not have associated device nodes
				if (!device) device = new TDEMonitorDevice(TDEGenericDeviceType::Monitor);
			}
			else {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
			}
		}
		if (devicesubsystem == "serio") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Serial);
		}
		if (devicesubsystem == "ppdev") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Parallel);
		}
		if (devicesubsystem == "printer") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Printer);
		}
		if (devicesubsystem == "bridge") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Bridge);
		}
		if ((devicesubsystem == "pci_bus")
			|| (devicesubsystem == "pci_express")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Bus);
		}
		if (devicesubsystem == "pcmcia_socket") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::PCMCIA);
		}
		if (devicesubsystem == "platform") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		if (devicesubsystem == "ieee80211") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		if (devicesubsystem == "rfkill") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		if (devicesubsystem == "pnp") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::PNP);
		}
		if ((devicesubsystem == "hid")
			|| (devicesubsystem == "hidraw")
			|| (devicesubsystem == "usbhid")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::HID);
		}
		if (devicesubsystem == "power_supply") {
			TQString powersupplyname(udev_device_get_property_value(dev, "POWER_SUPPLY_NAME"));
			if (powersupplyname.upper().startsWith("AC")) {
				if (!device) device = new TDEMainsPowerDevice(TDEGenericDeviceType::PowerSupply);
			}
			else {
				if (!device) device = new TDEBatteryDevice(TDEGenericDeviceType::Battery);
			}
		}

		// Moderate accuracy classification, if PCI device class is available
		// See http://www.acm.uiuc.edu/sigops/roll_your_own/7.c.1.html for codes and meanings
		if (!devicepciclass.isNull()) {
			// Pre PCI 2.0
			if (devicepciclass.startsWith("0001")) {
				if (devicenode.isNull()) {	// GPUs do not have associated device nodes
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::GPU);
				}
				else {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
				}
			}
			// Post PCI 2.0
			if (devicepciclass.startsWith("01")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::StorageController);
			}
			if (devicepciclass.startsWith("02")) {
				if (!device) device = new TDENetworkDevice(TDEGenericDeviceType::Network);
			}
			if (devicepciclass.startsWith("03")) {
				if (devicenode.isNull()) {	// GPUs do not have associated device nodes
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::GPU);
				}
				else {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
				}
			}
			if (devicepciclass.startsWith("04")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherMultimedia);
			}
			if (devicepciclass.startsWith("05")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::RAM);
			}
			if (devicepciclass.startsWith("06")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Bridge);
			}
			if (devicepciclass.startsWith("0a")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Dock);
			}
			if (devicepciclass.startsWith("0b")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::CPU);
			}
			if (devicepciclass.startsWith("0c")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Serial);
			}
		}

		// Last ditch attempt at classification
		// Likely inaccurate and sweeping
		if ((devicesubsystem == "usb")
			|| (devicesubsystem == "usbmon")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherUSB);
		}
		if (devicesubsystem == "pci") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherPeripheral);
		}
	}

	if (device == 0) {
		// Unhandled
		if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Other);
		printf("[FIXME] UNCLASSIFIED DEVICE name: %s type: %s subsystem: %s driver: %s [Node Path: %s] [Syspath: %s] [%s:%s]\n\r", devicename.ascii(), devicetype.ascii(), devicesubsystem.ascii(), devicedriver.ascii(), devicenode.ascii(), udev_device_get_syspath(dev), devicevendorid.ascii(), devicemodelid.ascii()); fflush(stdout);
	}

	// Root devices are special
	if ((device->type() == TDEGenericDeviceType::Root) || (device->type() == TDEGenericDeviceType::RootSystem)) {
		systempath = device->systemPath();
	}

	// Set preliminary basic device information
	device->internalSetName(devicename);
	device->internalSetDeviceNode(devicenode);
	device->internalSetSystemPath(systempath);
	device->internalSetVendorID(devicevendorid);
	device->internalSetModelID(devicemodelid);
	device->internalSetVendorEncoded(devicevendoridenc);
	device->internalSetModelEncoded(devicemodelidenc);
	device->internalSetSubVendorID(devicesubvendorid);
	device->internalSetSubModelID(devicesubmodelid);
	device->internalSetModuleAlias(devicemodalias);
	device->internalSetDeviceDriver(devicedriver);
	device->internalSetSubsystem(devicesubsystem);
	device->internalSetPCIClass(devicepciclass);

	updateBlacklists(device, dev);

	if (force_full_classification) {
		// Check external rules for possible device type overrides
		device = classifyUnknownDeviceByExternalRules(dev, device, false);
	}

	if (device->type() == TDEGenericDeviceType::Disk) {
		bool removable = false;
		bool hotpluggable = false;

		// We can get the removable flag, but we have no idea if the device has the ability to notify on media insertion/removal
		// If there is no such notification possible, then we should not set the removable flag
		// udev can be such an amazing pain at times
		// It exports a /capabilities node with no info on what the bits actually mean
		// This information is very poorly documented as a set of #defines in include/linux/genhd.h
		// We are specifically interested in GENHD_FL_REMOVABLE and GENHD_FL_MEDIA_CHANGE_NOTIFY
		// The "removable" flag should also really be renamed to "hotpluggable", as that is far more precise...
		TQString capabilitynodename = systempath;
		capabilitynodename.append("/capability");
		TQFile capabilityfile( capabilitynodename );
		unsigned int capabilities = 0;
		if ( capabilityfile.open( IO_ReadOnly ) ) {
			TQTextStream stream( &capabilityfile );
			TQString capabilitystring;
			capabilitystring = stream.readLine();
			capabilities = capabilitystring.toUInt();
			capabilityfile.close();
		}
		if (capabilities & GENHD_FL_REMOVABLE) {
			// FIXME
			// For added fun this is not always true; i.e. GENHD_FL_REMOVABLE can be set when the device cannot be hotplugged (floppy drives).
			hotpluggable = true;
		}
		if (capabilities & GENHD_FL_MEDIA_CHANGE_NOTIFY) {
			removable = true;
		}

		// See if any other devices are exclusively using this device, such as the Device Mapper
		TQStringList holdingDeviceNodes;
		TQString holdersnodename = udev_device_get_syspath(dev);
		holdersnodename.append("/holders/");
		TQDir holdersdir(holdersnodename);
		holdersdir.setFilter(TQDir::All);
		const TQFileInfoList *dirlist = holdersdir.entryInfoList();
		if (dirlist) {
			TQFileInfoListIterator holdersdirit(*dirlist);
			TQFileInfo *dirfi;
			while ( (dirfi = holdersdirit.current()) != 0 ) {
				if (dirfi->isSymLink()) {
					char* collapsedPath = realpath((holdersnodename + dirfi->readLink()).ascii(), NULL);
					holdingDeviceNodes.append(TQString(collapsedPath));
					free(collapsedPath);
				}
				++holdersdirit;
			}
		}

		// See if any other physical devices underlie this device, for example when the Device Mapper is in use
		TQStringList slaveDeviceNodes;
		TQString slavesnodename = udev_device_get_syspath(dev);
		slavesnodename.append("/slaves/");
		TQDir slavedir(slavesnodename);
		slavedir.setFilter(TQDir::All);
		dirlist = slavedir.entryInfoList();
		if (dirlist) {
			TQFileInfoListIterator slavedirit(*dirlist);
			TQFileInfo *dirfi;
			while ( (dirfi = slavedirit.current()) != 0 ) {
				if (dirfi->isSymLink()) {
					char* collapsedPath = realpath((slavesnodename + dirfi->readLink()).ascii(), NULL);
					slaveDeviceNodes.append(TQString(collapsedPath));
					free(collapsedPath);
				}
				++slavedirit;
			}
		}

		// Determine generic disk information
		TQString devicevendor(udev_device_get_property_value(dev, "ID_VENDOR"));
		TQString devicemodel(udev_device_get_property_value(dev, "ID_MODEL"));
		TQString devicebus(udev_device_get_property_value(dev, "ID_BUS"));

		// Get disk specific info
		TQString disklabel(udev_device_get_property_value(dev, "ID_FS_LABEL"));
		TQString diskuuid(udev_device_get_property_value(dev, "ID_FS_UUID"));
		TQString filesystemtype(udev_device_get_property_value(dev, "ID_FS_TYPE"));
		TQString filesystemusage(udev_device_get_property_value(dev, "ID_FS_USAGE"));

		device->internalSetVendorName(devicevendor);
		device->internalSetVendorModel(devicemodel);
		device->internalSetDeviceBus(devicebus);

		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);

		TDEDiskDeviceType::TDEDiskDeviceType disktype = sdevice->diskType();
		TDEDiskDeviceStatus::TDEDiskDeviceStatus diskstatus = sdevice->diskStatus();

		if (force_full_classification) {
			disktype = classifyDiskType(dev, devicebus, devicetypestring, systempath, devicevendor, devicemodel, filesystemtype, devicedriver);
			sdevice->internalSetDiskType(disktype);
			device = classifyUnknownDeviceByExternalRules(dev, device, true);	// Check external rules for possible subtype overrides
			disktype = sdevice->diskType();						// The type can be overridden by an external rule
		}

		if ((disktype & TDEDiskDeviceType::CDROM)
			|| (disktype & TDEDiskDeviceType::CDRW)
			|| (disktype & TDEDiskDeviceType::DVDROM)
			|| (disktype & TDEDiskDeviceType::DVDRAM)
			|| (disktype & TDEDiskDeviceType::DVDRW)
			|| (disktype & TDEDiskDeviceType::BDROM)
			|| (disktype & TDEDiskDeviceType::BDRW)
			|| (disktype & TDEDiskDeviceType::CDAudio)
			|| (disktype & TDEDiskDeviceType::CDVideo)
			|| (disktype & TDEDiskDeviceType::DVDVideo)
			|| (disktype & TDEDiskDeviceType::BDVideo)
			) {
			// These drives are guaranteed to be optical
			disktype = disktype | TDEDiskDeviceType::Optical;
		}

		if (disktype & TDEDiskDeviceType::Floppy) {
			// Floppy drives don't work well under udev
			// I have to look for the block device name manually
			TQString floppyblknodename = systempath;
			floppyblknodename.append("/block");
			TQDir floppyblkdir(floppyblknodename);
			floppyblkdir.setFilter(TQDir::All);
			const TQFileInfoList *floppyblkdirlist = floppyblkdir.entryInfoList();
			if (floppyblkdirlist) {
				TQFileInfoListIterator floppyblkdirit(*floppyblkdirlist);
				TQFileInfo *dirfi;
				while ( (dirfi = floppyblkdirit.current()) != 0 ) {
					if ((dirfi->fileName() != ".") && (dirfi->fileName() != "..")) {
						// Does this routine work with more than one floppy drive in the system?
						devicenode = TQString("/dev/").append(dirfi->fileName());
					}
					++floppyblkdirit;
				}
			}
	
			// Some interesting information can be gleaned from the CMOS type file
			// 0 : Defaults
			// 1 : 5 1/4 DD
			// 2 : 5 1/4 HD
			// 3 : 3 1/2 DD
			// 4 : 3 1/2 HD
			// 5 : 3 1/2 ED
			// 6 : 3 1/2 ED
			// 16 : unknown or not installed
			TQString floppycmsnodename = systempath;
			floppycmsnodename.append("/cmos");
			TQFile floppycmsfile( floppycmsnodename );
			TQString cmosstring;
			if ( floppycmsfile.open( IO_ReadOnly ) ) {
				TQTextStream stream( &floppycmsfile );
				cmosstring = stream.readLine();
				floppycmsfile.close();
			}
			// FIXME
			// Do something with the information in cmosstring

			if (devicenode.isNull()) {
				// This floppy drive cannot be mounted, so ignore it
				disktype = disktype & ~TDEDiskDeviceType::Floppy;
			}
		}

		if (devicetypestring.upper() == "CD") {
			if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_STATE")).upper() == "BLANK") {
				diskstatus = diskstatus | TDEDiskDeviceStatus::Blank;
			}
			sdevice->internalSetMediaInserted(!(TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) == "0"));
		}

		if (disktype & TDEDiskDeviceType::Zip) {
			// A Zip drive does not advertise its status via udev, but it can be guessed from the size parameter
			TQString zipnodename = systempath;
			zipnodename.append("/size");
			TQFile namefile( zipnodename );
			TQString zipsize;
			if ( namefile.open( IO_ReadOnly ) ) {
				TQTextStream stream( &namefile );
				zipsize = stream.readLine();
				namefile.close();
			}
			if (!zipsize.isNull()) {
				sdevice->internalSetMediaInserted((zipsize.toInt() != 0));
			}
		}

		if (removable) {
			diskstatus = diskstatus | TDEDiskDeviceStatus::Removable;
		}
		if (hotpluggable) {
			diskstatus = diskstatus | TDEDiskDeviceStatus::Hotpluggable;
		}

		if ((filesystemtype.upper() != "CRYPTO_LUKS") && (filesystemtype.upper() != "CRYPTO")  && (!filesystemtype.isNull())) {
			diskstatus = diskstatus | TDEDiskDeviceStatus::ContainsFilesystem;
		}

		// Set mountable flag if device is likely to be mountable
		diskstatus = diskstatus | TDEDiskDeviceStatus::Mountable;
		if ((devicetypestring.upper().isNull()) && (disktype & TDEDiskDeviceType::HDD)) {
			diskstatus = diskstatus & ~TDEDiskDeviceStatus::Mountable;
		}
		if (removable) {
			if (sdevice->mediaInserted()) {
				diskstatus = diskstatus | TDEDiskDeviceStatus::Inserted;
			}
			else {
				diskstatus = diskstatus & ~TDEDiskDeviceStatus::Mountable;
			}
		}

		if (holdingDeviceNodes.count() > 0) {
			diskstatus = diskstatus | TDEDiskDeviceStatus::UsedByDevice;
		}

		if (slaveDeviceNodes.count() > 0) {
			diskstatus = diskstatus | TDEDiskDeviceStatus::UsesDevice;
		}

		// See if any slaves were crypted
		for ( TQStringList::Iterator slaveit = slaveDeviceNodes.begin(); slaveit != slaveDeviceNodes.end(); ++slaveit ) {
			struct udev_device *slavedev;
			slavedev = udev_device_new_from_syspath(m_udevStruct, (*slaveit).ascii());
			TQString slavediskfstype(udev_device_get_property_value(slavedev, "ID_FS_TYPE"));
			if ((slavediskfstype.upper() == "CRYPTO_LUKS") || (slavediskfstype.upper() == "CRYPTO")) {
				disktype = disktype | TDEDiskDeviceType::UnlockedCrypt;
				// Set disk type based on parent device
				disktype = disktype | classifyDiskType(slavedev, TQString(udev_device_get_property_value(dev, "ID_BUS")), TQString(udev_device_get_property_value(dev, "ID_TYPE")), (*slaveit), TQString(udev_device_get_property_value(dev, "ID_VENDOR")), TQString(udev_device_get_property_value(dev, "ID_MODEL")), TQString(udev_device_get_property_value(dev, "ID_FS_TYPE")), TQString(udev_device_get_driver(dev)));
			}
			udev_device_unref(slavedev);
		}

		sdevice->internalSetDiskType(disktype);
		sdevice->internalSetDiskUUID(diskuuid);
		sdevice->internalSetDiskStatus(diskstatus);
		sdevice->internalSetFileSystemName(filesystemtype);
		sdevice->internalSetFileSystemUsage(filesystemusage);
		sdevice->internalSetSlaveDevices(slaveDeviceNodes);
		sdevice->internalSetHoldingDevices(holdingDeviceNodes);

		// Clean up disk label
		if ((sdevice->isDiskOfType(TDEDiskDeviceType::CDROM))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDRW))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDROM))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRW))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDROM))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDRW))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDVideo))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDVideo))
			|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDVideo))
			) {
			if (disklabel == "" && sdevice->diskLabel().isNull()) {
				// Read the volume label in via volname, since udev couldn't be bothered to do this on its own
				FILE *exepipe = popen(((TQString("volname %1").arg(devicenode).ascii())), "r");
				if (exepipe) {
					char buffer[8092];
					disklabel = fgets(buffer, sizeof(buffer), exepipe);
					pclose(exepipe);
				}
			}
		}

		sdevice->internalSetDiskLabel(disklabel);
	}

	if (device->type() == TDEGenericDeviceType::Network) {
		// Network devices don't have devices nodes per se, but we can at least return the Linux network name...
		TQString potentialdevicenode = systempath;
		potentialdevicenode.remove(0, potentialdevicenode.findRev("/")+1);
		TQString potentialparentnode = systempath;
		potentialparentnode.remove(0, potentialparentnode.findRev("/", potentialparentnode.findRev("/")-1)+1);
		if (potentialparentnode.startsWith("net/")) {
			devicenode = potentialdevicenode;
		}

		if (devicenode.isNull()) {
			// Platform device, not a physical device
			// HACK
			// This only works because devices of type Platform only access the TDEGenericDevice class!
			device->m_deviceType = TDEGenericDeviceType::Platform;
		}
		else {
			// Gather network device information
			TDENetworkDevice* ndevice = dynamic_cast<TDENetworkDevice*>(device);
			TQString valuesnodename = systempath + "/";
			TQDir valuesdir(valuesnodename);
			valuesdir.setFilter(TQDir::All);
			TQString nodename;
			const TQFileInfoList *dirlist = valuesdir.entryInfoList();
			if (dirlist) {
				TQFileInfoListIterator valuesdirit(*dirlist);
				TQFileInfo *dirfi;
				while ( (dirfi = valuesdirit.current()) != 0 ) {
					nodename = dirfi->fileName();
					TQFile file( valuesnodename + nodename );
					if ( file.open( IO_ReadOnly ) ) {
						TQTextStream stream( &file );
						TQString line;
						line = stream.readLine();
						if (nodename == "address") {
							ndevice->internalSetMacAddress(line);
						}
						if (nodename == "carrier") {
							ndevice->internalSetCarrierPresent(line.toInt());
						}
						if (nodename == "dormant") {
							ndevice->internalSetDormant(line.toInt());
						}
						if (nodename == "operstate") {
							TQString friendlyState = line.lower();
							friendlyState[0] = friendlyState[0].upper();
							ndevice->internalSetState(friendlyState);
						}
						file.close();
					}
					++valuesdirit;
				}
			}
			// Gather connection information such as IP addresses
			if (ndevice->state().upper() == "UP") {
				struct ifaddrs *ifaddr, *ifa;
				int family, s;
				char host[NI_MAXHOST];

				if (getifaddrs(&ifaddr) != -1) {
					for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
						if (ifa->ifa_addr == NULL) {
							continue;
						}
						
						family = ifa->ifa_addr->sa_family;

						if (TQString(ifa->ifa_name) == devicenode) {
							if ((family == AF_INET) || (family == AF_INET6)) {
								s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
								if (s == 0) {
									TQString address(host);
									if (family == AF_INET) {
										ndevice->internalSetIpV4Address(address);
									}
									if (family == AF_INET6) {
										address.truncate(address.findRev("%"));
										ndevice->internalSetIpV6Address(address);
									}
								}
								s = getnameinfo(ifa->ifa_netmask, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
								if (s == 0) {
									TQString address(host);
									if (family == AF_INET) {
										ndevice->internalSetIpV4Netmask(address);
									}
									if (family == AF_INET6) {
										address.truncate(address.findRev("%"));
										ndevice->internalSetIpV6Netmask(address);
									}
								}
								s = getnameinfo(ifa->ifa_ifu.ifu_broadaddr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
								if (s == 0) {
									TQString address(host);
									if (family == AF_INET) {
										ndevice->internalSetIpV4Broadcast(address);
									}
									if (family == AF_INET6) {
										address.truncate(address.findRev("%"));
										ndevice->internalSetIpV6Broadcast(address);
									}
								}
								s = getnameinfo(ifa->ifa_ifu.ifu_dstaddr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
								if (s == 0) {
									TQString address(host);
									if (family == AF_INET) {
										ndevice->internalSetIpV4Destination(address);
									}
									if (family == AF_INET6) {
										address.truncate(address.findRev("%"));
										ndevice->internalSetIpV6Destination(address);
									}
								}
							}
						}
					}
				}
				
				freeifaddrs(ifaddr);

				// Gather statistics
				TQString valuesnodename = systempath + "/statistics/";
				TQDir valuesdir(valuesnodename);
				valuesdir.setFilter(TQDir::All);
				TQString nodename;
				const TQFileInfoList *dirlist = valuesdir.entryInfoList();
				if (dirlist) {
					TQFileInfoListIterator valuesdirit(*dirlist);
					TQFileInfo *dirfi;
					while ( (dirfi = valuesdirit.current()) != 0 ) {
						nodename = dirfi->fileName();
						TQFile file( valuesnodename + nodename );
						if ( file.open( IO_ReadOnly ) ) {
							TQTextStream stream( &file );
							TQString line;
							line = stream.readLine();
							if (nodename == "rx_bytes") {
								ndevice->internalSetRxBytes(line.toDouble());
							}
							if (nodename == "tx_bytes") {
								ndevice->internalSetTxBytes(line.toDouble());
							}
							if (nodename == "rx_packets") {
								ndevice->internalSetRxPackets(line.toDouble());
							}
							if (nodename == "tx_packets") {
								ndevice->internalSetTxPackets(line.toDouble());
							}
							file.close();
						}
						++valuesdirit;
					}
				}
			}
		}
	}

	if ((device->type() == TDEGenericDeviceType::OtherSensor) || (device->type() == TDEGenericDeviceType::ThermalSensor)) {
		// Populate all sensor values
		TDESensorClusterMap sensors;
		TQString valuesnodename = systempath + "/";
		TQDir valuesdir(valuesnodename);
		valuesdir.setFilter(TQDir::All);
		TQString nodename;
		const TQFileInfoList *dirlist = valuesdir.entryInfoList();
		if (dirlist) {
			TQFileInfoListIterator valuesdirit(*dirlist);
			TQFileInfo *dirfi;
			while ( (dirfi = valuesdirit.current()) != 0 ) {
				nodename = dirfi->fileName();
				if (nodename.contains("_")) {
					TQFile file( valuesnodename + nodename );
					if ( file.open( IO_ReadOnly ) ) {
						TQTextStream stream( &file );
						TQString line;
						line = stream.readLine();
						TQStringList sensornodelist = TQStringList::split("_", nodename);
						TQString sensornodename = *(sensornodelist.at(0));
						TQString sensornodetype = *(sensornodelist.at(1));
						double lineValue = line.toDouble();
						if (!sensornodename.contains("fan")) {
							lineValue = lineValue / 1000.0;
						}
						if (sensornodetype == "label") {
							sensors[sensornodename].label = line;
						}
						if (sensornodetype == "input") {
							sensors[sensornodename].current = lineValue;
						}
						if (sensornodetype == "min") {
							sensors[sensornodename].minimum = lineValue;
						}
						if (sensornodetype == "max") {
							sensors[sensornodename].maximum = lineValue;
						}
						if (sensornodetype == "warn") {
							sensors[sensornodename].warning = lineValue;
						}
						if (sensornodetype == "crit") {
							sensors[sensornodename].critical = lineValue;
						}
						file.close();
					}
				}
				++valuesdirit;
			}
		}

		TDESensorDevice* sdevice = dynamic_cast<TDESensorDevice*>(device);
		sdevice->internalSetValues(sensors);
	}

	if (device->type() == TDEGenericDeviceType::Battery) {
		// Populate all battery values
		TDEBatteryDevice* bdevice = dynamic_cast<TDEBatteryDevice*>(device);
		TQString valuesnodename = systempath + "/";
		TQDir valuesdir(valuesnodename);
		valuesdir.setFilter(TQDir::All);
		TQString nodename;
		const TQFileInfoList *dirlist = valuesdir.entryInfoList();
		if (dirlist) {
			TQFileInfoListIterator valuesdirit(*dirlist);
			TQFileInfo *dirfi;
			while ( (dirfi = valuesdirit.current()) != 0 ) {
				nodename = dirfi->fileName();
				TQFile file( valuesnodename + nodename );
				if ( file.open( IO_ReadOnly ) ) {
					TQTextStream stream( &file );
					TQString line;
					line = stream.readLine();
					if (nodename == "alarm") {
						bdevice->internalSetAlarmEnergy(line.toDouble()/1000000.0);
					}
					if (nodename == "energy_full") {
						bdevice->internalSetMaximumEnergy(line.toDouble()/1000000.0);
					}
					if (nodename == "energy_full_design") {
						bdevice->internalSetMaximumDesignEnergy(line.toDouble()/1000000.0);
					}
					if (nodename == "energy_now") {
						bdevice->internalSetEnergy(line.toDouble()/1000000.0);
					}
					if (nodename == "manufacturer") {
						bdevice->internalSetVendorName(line.stripWhiteSpace());
					}
					if (nodename == "model_name") {
						bdevice->internalSetVendorModel(line.stripWhiteSpace());
					}
					if (nodename == "power_now") {
						bdevice->internalSetDischargeRate(line.toDouble()/1000000.0);
					}
					if (nodename == "present") {
						bdevice->internalSetInstalled(line.toInt());
					}
					if (nodename == "serial_number") {
						bdevice->internalSetSerialNumber(line.stripWhiteSpace());
					}
					if (nodename == "status") {
						bdevice->internalSetStatus(line);
					}
					if (nodename == "technology") {
						bdevice->internalSetTechnology(line);
					}
					if (nodename == "voltage_min_design") {
						bdevice->internalSetMinimumVoltage(line.toDouble()/1000000.0);
					}
					if (nodename == "voltage_now") {
						bdevice->internalSetVoltage(line.toDouble()/1000000.0);
					}
					file.close();
				}
				++valuesdirit;
			}
		}
	}

	if (device->type() == TDEGenericDeviceType::PowerSupply) {
		// Populate all power supply values
		TDEMainsPowerDevice* pdevice = dynamic_cast<TDEMainsPowerDevice*>(device);
		TQString valuesnodename = systempath + "/";
		TQDir valuesdir(valuesnodename);
		valuesdir.setFilter(TQDir::All);
		TQString nodename;
		const TQFileInfoList *dirlist = valuesdir.entryInfoList();
		if (dirlist) {
			TQFileInfoListIterator valuesdirit(*dirlist);
			TQFileInfo *dirfi;
			while ( (dirfi = valuesdirit.current()) != 0 ) {
				nodename = dirfi->fileName();
				TQFile file( valuesnodename + nodename );
				if ( file.open( IO_ReadOnly ) ) {
					TQTextStream stream( &file );
					TQString line;
					line = stream.readLine();
					if (nodename == "manufacturer") {
						pdevice->internalSetVendorName(line.stripWhiteSpace());
					}
					if (nodename == "model_name") {
						pdevice->internalSetVendorModel(line.stripWhiteSpace());
					}
					if (nodename == "online") {
						pdevice->internalSetOnline(line.toInt());
					}
					if (nodename == "serial_number") {
						pdevice->internalSetSerialNumber(line.stripWhiteSpace());
					}
					file.close();
				}
				++valuesdirit;
			}
		}
	}

	if (device->type() == TDEGenericDeviceType::Backlight) {
		// Populate all backlight values
		TDEBacklightDevice* bdevice = dynamic_cast<TDEBacklightDevice*>(device);
		TQString valuesnodename = systempath + "/";
		TQDir valuesdir(valuesnodename);
		valuesdir.setFilter(TQDir::All);
		TQString nodename;
		const TQFileInfoList *dirlist = valuesdir.entryInfoList();
		if (dirlist) {
			TQFileInfoListIterator valuesdirit(*dirlist);
			TQFileInfo *dirfi;
			while ( (dirfi = valuesdirit.current()) != 0 ) {
				nodename = dirfi->fileName();
				TQFile file( valuesnodename + nodename );
				if ( file.open( IO_ReadOnly ) ) {
					TQTextStream stream( &file );
					TQString line;
					line = stream.readLine();
					if (nodename == "bl_power") {
						TDEDisplayPowerLevel::TDEDisplayPowerLevel pl = TDEDisplayPowerLevel::On;
						int rpl = line.toInt();
						if (rpl == FB_BLANK_UNBLANK) {
							pl = TDEDisplayPowerLevel::On;
						}
						else if (rpl == FB_BLANK_POWERDOWN) {
							pl = TDEDisplayPowerLevel::Off;
						}
						bdevice->internalSetPowerLevel(pl);
					}
					if (nodename == "max_brightness") {
						bdevice->internalSetMaximumRawBrightness(line.toInt());
					}
					if (nodename == "actual_brightness") {
						bdevice->internalSetCurrentRawBrightness(line.toInt());
					}
					file.close();
				}
				++valuesdirit;
			}
		}
	}

	if (device->type() == TDEGenericDeviceType::Monitor) {
		TDEMonitorDevice* mdevice = dynamic_cast<TDEMonitorDevice*>(device);
		TQString valuesnodename = systempath + "/";
		TQDir valuesdir(valuesnodename);
		valuesdir.setFilter(TQDir::All);
		TQString nodename;
		const TQFileInfoList *dirlist = valuesdir.entryInfoList();
		if (dirlist) {
			TQFileInfoListIterator valuesdirit(*dirlist);
			TQFileInfo *dirfi;
			while ( (dirfi = valuesdirit.current()) != 0 ) {
				nodename = dirfi->fileName();
				TQFile file( valuesnodename + nodename );
				if ( file.open( IO_ReadOnly ) ) {
					TQTextStream stream( &file );
					TQString line;
					line = stream.readLine();
					if (nodename == "status") {
						mdevice->internalSetConnected(line.lower() == "connected");
					}
					if (nodename == "enabled") {
						mdevice->internalSetEnabled(line.lower() == "enabled");
					}
					if (nodename == "modes") {
						TQStringList resinfo;
						TQStringList resolutionsStringList = line.upper();
						while ((!stream.atEnd()) && (!line.isNull())) {
							line = stream.readLine();
							if (!line.isNull()) {
								resolutionsStringList.append(line.upper());
							}
						}
						TDEResolutionList resolutions;
						resolutions.clear();
						for (TQStringList::Iterator it = resolutionsStringList.begin(); it != resolutionsStringList.end(); ++it) {
							resinfo = TQStringList::split('X', *it, true);
							resolutions.append(TDEResolutionPair((*(resinfo.at(0))).toUInt(), (*(resinfo.at(1))).toUInt()));
						}
						mdevice->internalSetResolutions(resolutions);
					}
					if (nodename == "dpms") {
						TDEDisplayPowerLevel::TDEDisplayPowerLevel pl = TDEDisplayPowerLevel::On;
						if (line == "On") {
							pl = TDEDisplayPowerLevel::On;
						}
						else if (line == "Standby") {
							pl = TDEDisplayPowerLevel::Standby;
						}
						else if (line == "Suspend") {
							pl = TDEDisplayPowerLevel::Suspend;
						}
						else if (line == "Off") {
							pl = TDEDisplayPowerLevel::Off;
						}
						mdevice->internalSetPowerLevel(pl);
					}
					file.close();
				}
				++valuesdirit;
			}
		}

		TQString genericPortName = mdevice->systemPath();
		genericPortName.remove(0, genericPortName.find("-")+1);
		genericPortName.truncate(genericPortName.findRev("-"));
		mdevice->internalSetPortType(genericPortName);

		if (mdevice->connected()) {
			TQPair<TQString,TQString> monitor_info = getEDIDMonitorName(device->systemPath());
			if (!monitor_info.first.isNull()) {
				mdevice->internalSetVendorName(monitor_info.first);
				mdevice->internalSetVendorModel(monitor_info.second);
				mdevice->m_friendlyName = monitor_info.first + " " + monitor_info.second;
			}
			mdevice->internalSetEdid(getEDID(mdevice->systemPath()));
		}
		else {
			mdevice->m_friendlyName = i18n("Disconnected %1 Port").arg(genericPortName);
			mdevice->internalSetEdid(TQByteArray());
			mdevice->internalSetResolutions(TDEResolutionList());
		}

		// FIXME
		// Much of the code in libkrandr should be integrated into/interfaced with this library
	}

	if (device->type() == TDEGenericDeviceType::RootSystem) {
		// Try to obtain as much generic information about this system as possible
		TDERootSystemDevice* rdevice = dynamic_cast<TDERootSystemDevice*>(device);

		// Guess at my form factor
		// dmidecode would tell me this, but is somewhat unreliable
		TDESystemFormFactor::TDESystemFormFactor formfactor = TDESystemFormFactor::Desktop;
		if (listByDeviceClass(TDEGenericDeviceType::Backlight).count() > 0) {	// Is this really a good way to determine if a machine is a laptop?
			formfactor = TDESystemFormFactor::Laptop;
		}
		rdevice->internalSetFormFactor(formfactor);

		TQString valuesnodename = "/sys/power/";
		TQDir valuesdir(valuesnodename);
		valuesdir.setFilter(TQDir::All);
		TQString nodename;
		const TQFileInfoList *dirlist = valuesdir.entryInfoList();
		if (dirlist) {
			TQFileInfoListIterator valuesdirit(*dirlist);
			TQFileInfo *dirfi;
			while ( (dirfi = valuesdirit.current()) != 0 ) {
				nodename = dirfi->fileName();
				TQFile file( valuesnodename + nodename );
				if ( file.open( IO_ReadOnly ) ) {
					TQTextStream stream( &file );
					TQString line;
					line = stream.readLine();
					if (nodename == "state") {
						TDESystemPowerStateList powerstates;
						// Always assume that these two fully on/fully off states are available
						powerstates.append(TDESystemPowerState::Active);
						powerstates.append(TDESystemPowerState::PowerOff);
						if (line.contains("standby")) {
							powerstates.append(TDESystemPowerState::Standby);
						}
						if (line.contains("mem")) {
							powerstates.append(TDESystemPowerState::Suspend);
						}
						if (line.contains("disk")) {
							powerstates.append(TDESystemPowerState::Hibernate);
						}
						rdevice->internalSetPowerStates(powerstates);
					}
					if (nodename == "disk") {
						// Get list of available hibernation methods
						TDESystemHibernationMethodList hibernationmethods;
						if (line.contains("platform")) {
							hibernationmethods.append(TDESystemHibernationMethod::Platform);
						}
						if (line.contains("shutdown")) {
							hibernationmethods.append(TDESystemHibernationMethod::Shutdown);
						}
						if (line.contains("reboot")) {
							hibernationmethods.append(TDESystemHibernationMethod::Reboot);
						}
						if (line.contains("testproc")) {
							hibernationmethods.append(TDESystemHibernationMethod::TestProc);
						}
						if (line.contains("test")) {
							hibernationmethods.append(TDESystemHibernationMethod::Test);
						}
						rdevice->internalSetHibernationMethods(hibernationmethods);

						// Get current hibernation method
						line.truncate(line.findRev("]"));
						line.remove(0, line.findRev("[")+1);
						TDESystemHibernationMethod::TDESystemHibernationMethod hibernationmethod = TDESystemHibernationMethod::Unsupported;
						if (line.contains("platform")) {
							hibernationmethod = TDESystemHibernationMethod::Platform;
						}
						if (line.contains("shutdown")) {
							hibernationmethod = TDESystemHibernationMethod::Shutdown;
						}
						if (line.contains("reboot")) {
							hibernationmethod = TDESystemHibernationMethod::Reboot;
						}
						if (line.contains("testproc")) {
							hibernationmethod = TDESystemHibernationMethod::TestProc;
						}
						if (line.contains("test")) {
							hibernationmethod = TDESystemHibernationMethod::Test;
						}
						rdevice->internalSetHibernationMethod(hibernationmethod);
					}
					if (nodename == "image_size") {
						rdevice->internalSetDiskSpaceNeededForHibernation(line.toULong());
					}
					file.close();
				}
				++valuesdirit;
			}
		}
	}

	// NOTE
	// Keep these two handlers (Event and Input) in sync!

	if (device->type() == TDEGenericDeviceType::Event) {
		// Try to obtain as much type information about this event device as possible
		TDEEventDevice* edevice = dynamic_cast<TDEEventDevice*>(device);
		if (edevice->systemPath().contains("PNP0C0D")) {
			edevice->internalSetEventType(TDEEventDeviceType::ACPILidSwitch);
		}
		else if (edevice->systemPath().contains("PNP0C0E")) {
			edevice->internalSetEventType(TDEEventDeviceType::ACPISleepButton);
		}
		else if (edevice->systemPath().contains("PNP0C0C")) {
			edevice->internalSetEventType(TDEEventDeviceType::ACPIPowerButton);
		}
		else {
			edevice->internalSetEventType(TDEEventDeviceType::Unknown);
		}
	}

	if (device->type() == TDEGenericDeviceType::Input) {
		// Try to obtain as much type information about this input device as possible
		TDEInputDevice* idevice = dynamic_cast<TDEInputDevice*>(device);
		if (idevice->systemPath().contains("PNP0C0D")) {
			idevice->internalSetInputType(TDEInputDeviceType::ACPILidSwitch);
		}
		else if (idevice->systemPath().contains("PNP0C0E")) {
			idevice->internalSetInputType(TDEInputDeviceType::ACPISleepButton);
		}
		else if (idevice->systemPath().contains("PNP0C0C")) {
			idevice->internalSetInputType(TDEInputDeviceType::ACPIPowerButton);
		}
		else {
			idevice->internalSetInputType(TDEInputDeviceType::Unknown);
		}
	}

	if (device->type() == TDEGenericDeviceType::Event) {
		// Try to obtain as much specific information about this event device as possible
		TDEEventDevice* edevice = dynamic_cast<TDEEventDevice*>(device);
		int r;
		char switches[SW_CNT];

		// Figure out which switch types are supported, if any
		TDESwitchType::TDESwitchType supportedSwitches = TDESwitchType::Null;
		if (edevice->m_fd < 0) {
			edevice->m_fd = open(edevice->deviceNode().ascii(), O_RDONLY);
		}
		r = ioctl(edevice->m_fd, EVIOCGBIT(EV_SW, sizeof(switches)), switches);
		if (r > 0) {
			if (BIT_IS_SET(switches, SW_LID)) {
				supportedSwitches = supportedSwitches | TDESwitchType::Lid;
			}
			if (BIT_IS_SET(switches, SW_TABLET_MODE)) {
				supportedSwitches = supportedSwitches | TDESwitchType::TabletMode;
			}
			if (BIT_IS_SET(switches, SW_RFKILL_ALL)) {
				supportedSwitches = supportedSwitches | TDESwitchType::RFKill;
			}
			if (BIT_IS_SET(switches, SW_RADIO)) {
				supportedSwitches = supportedSwitches | TDESwitchType::Radio;
			}
			if (BIT_IS_SET(switches, SW_MICROPHONE_INSERT)) {
				supportedSwitches = supportedSwitches | TDESwitchType::MicrophoneInsert;
			}
			if (BIT_IS_SET(switches, SW_DOCK)) {
				supportedSwitches = supportedSwitches | TDESwitchType::Dock;
			}
			if (BIT_IS_SET(switches, SW_LINEOUT_INSERT)) {
				supportedSwitches = supportedSwitches | TDESwitchType::LineOutInsert;
			}
			if (BIT_IS_SET(switches, SW_JACK_PHYSICAL_INSERT)) {
				supportedSwitches = supportedSwitches | TDESwitchType::JackPhysicalInsert;
			}
			if (BIT_IS_SET(switches, SW_VIDEOOUT_INSERT)) {
				supportedSwitches = supportedSwitches | TDESwitchType::VideoOutInsert;
			}
#if 0	// Some old kernels don't provide these defines... [FIXME]
			if (BIT_IS_SET(switches, SW_CAMERA_LENS_COVER)) {
				supportedSwitches = supportedSwitches | TDESwitchType::CameraLensCover;
			}
			if (BIT_IS_SET(switches, SW_KEYPAD_SLIDE)) {
				supportedSwitches = supportedSwitches | TDESwitchType::KeypadSlide;
			}
			if (BIT_IS_SET(switches, SW_FRONT_PROXIMITY)) {
				supportedSwitches = supportedSwitches | TDESwitchType::FrontProximity;
			}
			if (BIT_IS_SET(switches, SW_ROTATE_LOCK)) {
				supportedSwitches = supportedSwitches | TDESwitchType::RotateLock;
			}
			if (BIT_IS_SET(switches, SW_LINEIN_INSERT)) {
				supportedSwitches = supportedSwitches | TDESwitchType::LineInInsert;
			}
#endif
		}
		edevice->internalSetProvidedSwitches(supportedSwitches);

		// Figure out which switch types are active, if any
		TDESwitchType::TDESwitchType activeSwitches = TDESwitchType::Null;
		r = ioctl(edevice->m_fd, EVIOCGSW(sizeof(switches)), switches);
		if (r > 0) {
			if (BIT_IS_SET(switches, SW_LID)) {
				activeSwitches = activeSwitches | TDESwitchType::Lid;
			}
			if (BIT_IS_SET(switches, SW_TABLET_MODE)) {
				activeSwitches = activeSwitches | TDESwitchType::TabletMode;
			}
			if (BIT_IS_SET(switches, SW_RFKILL_ALL)) {
				activeSwitches = activeSwitches | TDESwitchType::RFKill;
			}
			if (BIT_IS_SET(switches, SW_RADIO)) {
				activeSwitches = activeSwitches | TDESwitchType::Radio;
			}
			if (BIT_IS_SET(switches, SW_MICROPHONE_INSERT)) {
				activeSwitches = activeSwitches | TDESwitchType::MicrophoneInsert;
			}
			if (BIT_IS_SET(switches, SW_DOCK)) {
				activeSwitches = activeSwitches | TDESwitchType::Dock;
			}
			if (BIT_IS_SET(switches, SW_LINEOUT_INSERT)) {
				activeSwitches = activeSwitches | TDESwitchType::LineOutInsert;
			}
			if (BIT_IS_SET(switches, SW_JACK_PHYSICAL_INSERT)) {
				activeSwitches = activeSwitches | TDESwitchType::JackPhysicalInsert;
			}
			if (BIT_IS_SET(switches, SW_VIDEOOUT_INSERT)) {
				activeSwitches = activeSwitches | TDESwitchType::VideoOutInsert;
			}
#if 0	// Some old kernels don't provide these defines... [FIXME]
			if (BIT_IS_SET(switches, SW_CAMERA_LENS_COVER)) {
				activeSwitches = activeSwitches | TDESwitchType::CameraLensCover;
			}
			if (BIT_IS_SET(switches, SW_KEYPAD_SLIDE)) {
				activeSwitches = activeSwitches | TDESwitchType::KeypadSlide;
			}
			if (BIT_IS_SET(switches, SW_FRONT_PROXIMITY)) {
				activeSwitches = activeSwitches | TDESwitchType::FrontProximity;
			}
			if (BIT_IS_SET(switches, SW_ROTATE_LOCK)) {
				activeSwitches = activeSwitches | TDESwitchType::RotateLock;
			}
			if (BIT_IS_SET(switches, SW_LINEIN_INSERT)) {
				activeSwitches = activeSwitches | TDESwitchType::LineInInsert;
			}
#endif
		}
		edevice->internalSetActiveSwitches(activeSwitches);
	}

	// Root devices are still special
	if ((device->type() == TDEGenericDeviceType::Root) || (device->type() == TDEGenericDeviceType::RootSystem)) {
		systempath = device->systemPath();
	}

	// Set basic device information again, as some information may have changed
	device->internalSetName(devicename);
	device->internalSetDeviceNode(devicenode);
	device->internalSetSystemPath(systempath);
	device->internalSetVendorID(devicevendorid);
	device->internalSetModelID(devicemodelid);
	device->internalSetVendorEncoded(devicevendoridenc);
	device->internalSetModelEncoded(devicemodelidenc);
	device->internalSetSubVendorID(devicesubvendorid);
	device->internalSetSubModelID(devicesubmodelid);
	device->internalSetDeviceDriver(devicedriver);
	device->internalSetSubsystem(devicesubsystem);
	device->internalSetPCIClass(devicepciclass);

	// Internal use only!
	device->m_udevtype = devicetype;
	device->m_udevdevicetypestring = devicetypestring;
	device->udevdevicetypestring_alt = devicetypestring_alt;

	return device;
}

void TDEHardwareDevices::updateBlacklists(TDEGenericDevice* hwdevice, udev_device* dev) {
	// HACK
	// I am lucky enough to have a Flash drive that spams udev continually with device change events
	// I imagine I am not the only one, so here is a section in which specific devices can be blacklisted!

	// For "U3 System" fake CD
	if ((hwdevice->vendorID() == "08ec") && (hwdevice->modelID() == "0020") && (TQString(udev_device_get_property_value(dev, "ID_TYPE")) == "cd")) {
		hwdevice->internalSetBlacklistedForUpdate(true);
	}
}

bool TDEHardwareDevices::queryHardwareInformation() {
	if (!m_udevStruct) {
		return false;
	}

	// Prepare the device list for repopulation
	m_deviceList.clear();
	addCoreSystemDevices();

	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;

	// Create a list of all devices
	enumerate = udev_enumerate_new(m_udevStruct);
	udev_enumerate_add_match_subsystem(enumerate, NULL);
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	// Get detailed information on each detected device
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;

		// Get the filename of the /sys entry for the device and create a udev_device object (dev) representing it
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(m_udevStruct, path);

		TDEGenericDevice* device = classifyUnknownDevice(dev);

		// Make sure this device is not a duplicate
		TDEGenericDevice *hwdevice;
		for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
			if (hwdevice->systemPath() == device->systemPath()) {
				delete device;
				device = 0;
				break;
			}
		}

		if (device) {
			m_deviceList.append(device);
		}

		udev_device_unref(dev);
	}

	// Free the enumerator object
	udev_enumerate_unref(enumerate);

	// Update parent/child tables for all devices
	updateParentDeviceInformation();

	return true;
}

void TDEHardwareDevices::updateParentDeviceInformation(TDEGenericDevice* hwdevice) {
	// Scan for the first path up the sysfs tree that is available in the main hardware table
	bool done = false;
	TQString current_path = hwdevice->systemPath();
	TDEGenericDevice* parentdevice = 0;

	if (current_path.endsWith("/")) {
		current_path.truncate(current_path.findRev("/"));
	}
	while (done == false) {
		current_path.truncate(current_path.findRev("/"));
		if (current_path.startsWith("/sys/devices")) {
			if (current_path.endsWith("/")) {
				current_path.truncate(current_path.findRev("/"));
			}
			parentdevice = findBySystemPath(current_path);
			if (parentdevice) {
				done = true;
			}
		}
		else {
			// Abort!
			done = true;
		}
	}

	hwdevice->internalSetParentDevice(parentdevice);
}

void TDEHardwareDevices::updateParentDeviceInformation() {
	TDEGenericDevice *hwdevice;

	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	TDEGenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		updateParentDeviceInformation(hwdevice);
	}
}

void TDEHardwareDevices::addCoreSystemDevices() {
	TDEGenericDevice *hwdevice;

	// Add the Main Root System Device, which provides all other devices
	hwdevice = new TDERootSystemDevice(TDEGenericDeviceType::RootSystem);
	hwdevice->internalSetSystemPath("/sys/devices");
	m_deviceList.append(hwdevice);
	rescanDeviceInformation(hwdevice);

	// Add core top-level devices in /sys/devices to the hardware listing
	TQStringList holdingDeviceNodes;
	TQString devicesnodename = "/sys/devices";
	TQDir devicesdir(devicesnodename);
	devicesdir.setFilter(TQDir::All);
	TQString nodename;
	const TQFileInfoList *dirlist = devicesdir.entryInfoList();
	if (dirlist) {
		TQFileInfoListIterator devicesdirit(*dirlist);
		TQFileInfo *dirfi;
		while ( (dirfi = devicesdirit.current()) != 0 ) {
			nodename = dirfi->fileName();
			if (nodename != "." && nodename != "..") {
				hwdevice = new TDEGenericDevice(TDEGenericDeviceType::Root);
				hwdevice->internalSetSystemPath(dirfi->absFilePath());
				m_deviceList.append(hwdevice);
			}
			++devicesdirit;
		}
	}

	// Handle CPUs, which are currently handled terribly by udev
	// Parse /proc/cpuinfo to extract some information about the CPUs
	hwdevice = 0;
	TQStringList lines;
	TQFile file( "/proc/cpuinfo" );
	if ( file.open( IO_ReadOnly ) ) {
		TQTextStream stream( &file );
		TQString line;
		int processorNumber = -1;
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			// WARNING This routine assumes that "processor" is always the first entry in /proc/cpuinfo!
			// FIXME Parse all available information, such as frequency, etc.
			if (line.startsWith("processor")) {
				line.remove(0, line.find(":")+1);
				line = line.stripWhiteSpace();
				processorNumber = line.toInt();
				hwdevice = new TDECPUDevice(TDEGenericDeviceType::CPU);
				hwdevice->internalSetSystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber));
				m_deviceList.append(hwdevice);
#if 0
				// Set up CPU information monitor
				// The only way CPU information can be changed is if something changes in the cpufreq node
				// This may change in the future, but for now it is a fairly good assumption
				m_cpuWatch->addDir(TQString("/sys/devices/system/cpu/cpu%1/cpufreq").arg(processorNumber));
#endif
			}
			lines += line;
		}
		file.close();
	}

	// Populate CPU information
	processModifiedCPUs();
}

TQString TDEHardwareDevices::findPCIDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid) {
	TQString vendorName = TQString::null;
	TQString modelName = TQString::null;
	TQString friendlyName = TQString::null;

	if (!pci_id_map) {
		pci_id_map = new TDEDeviceIDMap;

		TQString database_filename = "/usr/share/pci.ids";
		if (!TQFile::exists(database_filename)) {
			database_filename = "/usr/share/misc/pci.ids";
		}
		if (!TQFile::exists(database_filename)) {
			printf("[tdehardwaredevices] Unable to locate PCI information database pci.ids\n\r"); fflush(stdout);
			return i18n("Unknown PCI Device");
		}
	
		TQFile database(database_filename);
		if (database.open(IO_ReadOnly)) {
			TQTextStream stream(&database);
			TQString line;
			TQString vendorID;
			TQString modelID;
			TQString subvendorID;
			TQString submodelID;
			TQString deviceMapKey;
			TQStringList devinfo;
			while (!stream.atEnd()) {
				line = stream.readLine();
				if ((!line.upper().startsWith("\t")) && (!line.upper().startsWith("#"))) {
					line.replace("\t", "");
					devinfo = TQStringList::split(' ', line, false);
					vendorID = *(devinfo.at(0));
					vendorName = line;
					vendorName.remove(0, vendorName.find(" "));
					vendorName = vendorName.stripWhiteSpace();
					modelName = TQString::null;
					deviceMapKey = vendorID.lower() + ":::";
				}
				else {
					if ((line.upper().startsWith("\t")) && (!line.upper().startsWith("\t\t"))) {
						line.replace("\t", "");
						devinfo = TQStringList::split(' ', line, false);
						modelID = *(devinfo.at(0));
						modelName = line;
						modelName.remove(0, modelName.find(" "));
						modelName = modelName.stripWhiteSpace();
						deviceMapKey = vendorID.lower() + ":" + modelID.lower() + "::";
					}
					else {
						if (line.upper().startsWith("\t\t")) {
							line.replace("\t", "");
							devinfo = TQStringList::split(' ', line, false);
							subvendorID = *(devinfo.at(0));
							submodelID = *(devinfo.at(1));
							modelName = line;
							modelName.remove(0, modelName.find(" "));
							modelName = modelName.stripWhiteSpace();
							modelName.remove(0, modelName.find(" "));
							modelName = modelName.stripWhiteSpace();
							deviceMapKey = vendorID.lower() + ":" + modelID.lower() + ":" + subvendorID.lower() + ":" + submodelID.lower();
						}
					}
				}
				if (modelName.isNull()) {
					pci_id_map->insert(deviceMapKey, "***UNKNOWN DEVICE*** " + vendorName, true);
				}
				else {
					pci_id_map->insert(deviceMapKey, vendorName + " " + modelName, true);
				}
			}
			database.close();
		}
		else {
			printf("[tdehardwaredevices] Unable to open PCI information database %s\n\r", database_filename.ascii()); fflush(stdout);
		}
	}

	if (pci_id_map) {
		TQString deviceName;
		TQString deviceMapKey = vendorid.lower() + ":" + modelid.lower() + ":" + subvendorid.lower() + ":" + submodelid.lower();

		deviceName = (*pci_id_map)[deviceMapKey];
		if (deviceName.isNull() || deviceName.startsWith("***UNKNOWN DEVICE*** ")) {
			deviceMapKey = vendorid.lower() + ":" + modelid.lower() + ":" + subvendorid.lower() + ":";
			deviceName = (*pci_id_map)[deviceMapKey];
			if (deviceName.isNull() || deviceName.startsWith("***UNKNOWN DEVICE*** ")) {
				deviceMapKey = vendorid.lower() + ":" + modelid.lower() + "::";
				deviceName = (*pci_id_map)[deviceMapKey];
			}
		}

		if (deviceName.startsWith("***UNKNOWN DEVICE*** ")) {
			deviceName.replace("***UNKNOWN DEVICE*** ", "");
			deviceName.prepend(i18n("Unknown PCI Device") + " ");
			if (subvendorid.isNull()) {
				deviceName.append(TQString(" [%1:%2]").arg(vendorid.lower()).arg(modelid.lower()));
			}
			else {
				deviceName.append(TQString(" [%1:%2] [%3:%4]").arg(vendorid.lower()).arg(modelid.lower()).arg(subvendorid.lower()).arg(submodelid.lower()));
			}
		}

		return deviceName;
	}
	else {
		return i18n("Unknown PCI Device");
	}
}

TQString TDEHardwareDevices::findUSBDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid) {
	TQString vendorName = TQString::null;
	TQString modelName = TQString::null;
	TQString friendlyName = TQString::null;

	if (!usb_id_map) {
		usb_id_map = new TDEDeviceIDMap;

		TQString database_filename = "/usr/share/usb.ids";
		if (!TQFile::exists(database_filename)) {
			database_filename = "/usr/share/misc/usb.ids";
		}
		if (!TQFile::exists(database_filename)) {
			printf("[tdehardwaredevices] Unable to locate USB information database usb.ids\n\r"); fflush(stdout);
			return i18n("Unknown USB Device");
		}
	
		TQFile database(database_filename);
		if (database.open(IO_ReadOnly)) {
			TQTextStream stream(&database);
			TQString line;
			TQString vendorID;
			TQString modelID;
			TQString subvendorID;
			TQString submodelID;
			TQString deviceMapKey;
			TQStringList devinfo;
			while (!stream.atEnd()) {
				line = stream.readLine();
				if ((!line.upper().startsWith("\t")) && (!line.upper().startsWith("#"))) {
					line.replace("\t", "");
					devinfo = TQStringList::split(' ', line, false);
					vendorID = *(devinfo.at(0));
					vendorName = line;
					vendorName.remove(0, vendorName.find(" "));
					vendorName = vendorName.stripWhiteSpace();
					modelName = TQString::null;
					deviceMapKey = vendorID.lower() + ":::";
				}
				else {
					if ((line.upper().startsWith("\t")) && (!line.upper().startsWith("\t\t"))) {
						line.replace("\t", "");
						devinfo = TQStringList::split(' ', line, false);
						modelID = *(devinfo.at(0));
						modelName = line;
						modelName.remove(0, modelName.find(" "));
						modelName = modelName.stripWhiteSpace();
						deviceMapKey = vendorID.lower() + ":" + modelID.lower() + "::";
					}
					else {
						if (line.upper().startsWith("\t\t")) {
							line.replace("\t", "");
							devinfo = TQStringList::split(' ', line, false);
							subvendorID = *(devinfo.at(0));
							submodelID = *(devinfo.at(1));
							modelName = line;
							modelName.remove(0, modelName.find(" "));
							modelName = modelName.stripWhiteSpace();
							modelName.remove(0, modelName.find(" "));
							modelName = modelName.stripWhiteSpace();
							deviceMapKey = vendorID.lower() + ":" + modelID.lower() + ":" + subvendorID.lower() + ":" + submodelID.lower();
						}
					}
				}
				if (modelName.isNull()) {
					usb_id_map->insert(deviceMapKey, "***UNKNOWN DEVICE*** " + vendorName, true);
				}
				else {
					usb_id_map->insert(deviceMapKey, vendorName + " " + modelName, true);
				}
			}
			database.close();
		}
		else {
			printf("[tdehardwaredevices] Unable to open USB information database %s\n\r", database_filename.ascii()); fflush(stdout);
		}
	}

	if (usb_id_map) {
		TQString deviceName;
		TQString deviceMapKey = vendorid.lower() + ":" + modelid.lower() + ":" + subvendorid.lower() + ":" + submodelid.lower();

		deviceName = (*usb_id_map)[deviceMapKey];
		if (deviceName.isNull() || deviceName.startsWith("***UNKNOWN DEVICE*** ")) {
			deviceMapKey = vendorid.lower() + ":" + modelid.lower() + ":" + subvendorid.lower() + ":";
			deviceName = (*usb_id_map)[deviceMapKey];
			if (deviceName.isNull() || deviceName.startsWith("***UNKNOWN DEVICE*** ")) {
				deviceMapKey = vendorid.lower() + ":" + modelid.lower() + "::";
				deviceName = (*usb_id_map)[deviceMapKey];
			}
		}

		if (deviceName.startsWith("***UNKNOWN DEVICE*** ")) {
			deviceName.replace("***UNKNOWN DEVICE*** ", "");
			deviceName.prepend(i18n("Unknown USB Device") + " ");
			if (subvendorid.isNull()) {
				deviceName.append(TQString(" [%1:%2]").arg(vendorid.lower()).arg(modelid.lower()));
			}
			else {
				deviceName.append(TQString(" [%1:%2] [%3:%4]").arg(vendorid.lower()).arg(modelid.lower()).arg(subvendorid.lower()).arg(submodelid.lower()));
			}
		}

		return deviceName;
	}
	else {
		return i18n("Unknown USB Device");
	}
}

TQString TDEHardwareDevices::findPNPDeviceName(TQString pnpid) {
	TQString friendlyName = TQString::null;

	if (!pnp_id_map) {
		pnp_id_map = new TDEDeviceIDMap;

		TQStringList hardware_info_directories(KGlobal::dirs()->resourceDirs("data"));
		TQString hardware_info_directory_suffix("tdehwlib/pnpdev/");
		TQString hardware_info_directory;
		TQString database_filename;

		for ( TQStringList::Iterator it = hardware_info_directories.begin(); it != hardware_info_directories.end(); ++it ) {
			hardware_info_directory = (*it);
			hardware_info_directory += hardware_info_directory_suffix;
	
			if (KGlobal::dirs()->exists(hardware_info_directory)) {
				database_filename = hardware_info_directory + "pnp.ids";
				if (TQFile::exists(database_filename)) {
					break;
				}
			}
		}

		if (!TQFile::exists(database_filename)) {
			printf("[tdehardwaredevices] Unable to locate PNP information database pnp.ids\n\r"); fflush(stdout);
			return i18n("Unknown PNP Device");
		}
	
		TQFile database(database_filename);
		if (database.open(IO_ReadOnly)) {
			TQTextStream stream(&database);
			TQString line;
			TQString pnpID;
			TQString vendorName;
			TQString deviceMapKey;
			TQStringList devinfo;
			while (!stream.atEnd()) {
				line = stream.readLine();
				if ((!line.upper().startsWith("\t")) && (!line.upper().startsWith("#"))) {
					devinfo = TQStringList::split('\t', line, false);
					if (devinfo.count() > 1) {
						pnpID = *(devinfo.at(0));
						vendorName = *(devinfo.at(1));;
						vendorName = vendorName.stripWhiteSpace();
						deviceMapKey = pnpID.upper().stripWhiteSpace();
						if (!deviceMapKey.isNull()) {
							pnp_id_map->insert(deviceMapKey, vendorName, true);
						}
					}
				}
			}
			database.close();
		}
		else {
			printf("[tdehardwaredevices] Unable to open PNP information database %s\n\r", database_filename.ascii()); fflush(stdout);
		}
	}

	if (pnp_id_map) {
		TQString deviceName;

		deviceName = (*pnp_id_map)[pnpid];

		return deviceName;
	}
	else {
		return i18n("Unknown PNP Device");
	}
}

TQString TDEHardwareDevices::findMonitorManufacturerName(TQString dpyid) {
	TQString friendlyName = TQString::null;

	if (!dpy_id_map) {
		dpy_id_map = new TDEDeviceIDMap;

		TQStringList hardware_info_directories(KGlobal::dirs()->resourceDirs("data"));
		TQString hardware_info_directory_suffix("tdehwlib/pnpdev/");
		TQString hardware_info_directory;
		TQString database_filename;

		for ( TQStringList::Iterator it = hardware_info_directories.begin(); it != hardware_info_directories.end(); ++it ) {
			hardware_info_directory = (*it);
			hardware_info_directory += hardware_info_directory_suffix;
	
			if (KGlobal::dirs()->exists(hardware_info_directory)) {
				database_filename = hardware_info_directory + "dpy.ids";
				if (TQFile::exists(database_filename)) {
					break;
				}
			}
		}

		if (!TQFile::exists(database_filename)) {
			printf("[tdehardwaredevices] Unable to locate monitor information database dpy.ids\n\r"); fflush(stdout);
			return i18n("Unknown Monitor Device");
		}
	
		TQFile database(database_filename);
		if (database.open(IO_ReadOnly)) {
			TQTextStream stream(&database);
			TQString line;
			TQString dpyID;
			TQString vendorName;
			TQString deviceMapKey;
			TQStringList devinfo;
			while (!stream.atEnd()) {
				line = stream.readLine();
				if ((!line.upper().startsWith("\t")) && (!line.upper().startsWith("#"))) {
					devinfo = TQStringList::split('\t', line, false);
					if (devinfo.count() > 1) {
						dpyID = *(devinfo.at(0));
						vendorName = *(devinfo.at(1));;
						vendorName = vendorName.stripWhiteSpace();
						deviceMapKey = dpyID.upper().stripWhiteSpace();
						if (!deviceMapKey.isNull()) {
							dpy_id_map->insert(deviceMapKey, vendorName, true);
						}
					}
				}
			}
			database.close();
		}
		else {
			printf("[tdehardwaredevices] Unable to open monitor information database %s\n\r", database_filename.ascii()); fflush(stdout);
		}
	}

	if (dpy_id_map) {
		TQString deviceName;

		deviceName = (*dpy_id_map)[dpyid];

		return deviceName;
	}
	else {
		return i18n("Unknown Monitor Device");
	}
}

TQPair<TQString,TQString> TDEHardwareDevices::getEDIDMonitorName(TQString path) {
	TQPair<TQString,TQString> edid;
	TQByteArray binaryedid = getEDID(path);
	if (binaryedid.isNull()) {
		return TQPair<TQString,TQString>(TQString::null, TQString::null);
	}

	// Get the manufacturer ID
	unsigned char letter_1 = ((binaryedid[8]>>2) & 0x1F) + 0x40;
	unsigned char letter_2 = (((binaryedid[8] & 0x03) << 3) | ((binaryedid[9]>>5) & 0x07)) + 0x40;
	unsigned char letter_3 = (binaryedid[9] & 0x1F) + 0x40;
	TQChar qletter_1 = TQChar(letter_1);
	TQChar qletter_2 = TQChar(letter_2);
	TQChar qletter_3 = TQChar(letter_3);
	TQString manufacturer_id = TQString("%1%2%3").arg(qletter_1).arg(qletter_2).arg(qletter_3);

	// Get the model ID
	unsigned int raw_model_id = (((binaryedid[10] << 8) | binaryedid[11]) << 16) & 0xFFFF0000;
	// Reverse the bit order
	unsigned int model_id = reverse_bits(raw_model_id);

	// Try to get the model name
	bool has_friendly_name = false;
	unsigned char descriptor_block[18];
	int i;
	for (i=72;i<90;i++) {
		descriptor_block[i-72] = binaryedid[i] & 0xFF;
	}
	if ((descriptor_block[0] != 0) || (descriptor_block[1] != 0) || (descriptor_block[3] != 0xFC)) {
		for (i=90;i<108;i++) {
			descriptor_block[i-90] = binaryedid[i] & 0xFF;
		}
		if ((descriptor_block[0] != 0) || (descriptor_block[1] != 0) || (descriptor_block[3] != 0xFC)) {
			for (i=108;i<126;i++) {
				descriptor_block[i-108] = binaryedid[i] & 0xFF;
			}
		}
	}

	TQString monitor_name;
	if ((descriptor_block[0] == 0) && (descriptor_block[1] == 0) && (descriptor_block[3] == 0xFC)) {
		char* pos = strchr((char *)(descriptor_block+5), '\n');
		if (pos) {
			*pos = 0;
			has_friendly_name = true;
			monitor_name = TQString((char *)(descriptor_block+5));
		}
		else {
			has_friendly_name = false;
		}
	}

	// Look up manufacturer name
	TQString manufacturer_name = findMonitorManufacturerName(manufacturer_id);
	if (manufacturer_name.isNull()) {
		manufacturer_name = manufacturer_id;
	}

	if (has_friendly_name) {
		edid.first = TQString("%1").arg(manufacturer_name);
		edid.second = TQString("%2").arg(monitor_name);
	}
	else {
		edid.first = TQString("%1").arg(manufacturer_name);
		edid.second = TQString("0x%2").arg(model_id, 0, 16);
	}

	return edid;
}

TQByteArray TDEHardwareDevices::getEDID(TQString path) {
	TQFile file(TQString("%1/edid").arg(path));
	if (!file.open (IO_ReadOnly)) {
		return TQByteArray();
	}
	TQByteArray binaryedid = file.readAll();
	file.close();
	return binaryedid;
}

TQString TDEHardwareDevices::getFriendlyDeviceTypeStringFromType(TDEGenericDeviceType::TDEGenericDeviceType query) {
	TQString ret = "Unknown Device";

	// Keep this in sync with the TDEGenericDeviceType definition in the header
	if (query == TDEGenericDeviceType::Root) {
		ret = i18n("Root");
	}
	else if (query == TDEGenericDeviceType::RootSystem) {
		ret = i18n("System Root");
	}
	else if (query == TDEGenericDeviceType::CPU) {
		ret = i18n("CPU");
	}
	else if (query == TDEGenericDeviceType::GPU) {
		ret = i18n("Graphics Processor");
	}
	else if (query == TDEGenericDeviceType::RAM) {
		ret = i18n("RAM");
	}
	else if (query == TDEGenericDeviceType::Bus) {
		ret = i18n("Bus");
	}
	else if (query == TDEGenericDeviceType::I2C) {
		ret = i18n("I2C Bus");
	}
	else if (query == TDEGenericDeviceType::MDIO) {
		ret = i18n("MDIO Bus");
	}
	else if (query == TDEGenericDeviceType::Mainboard) {
		ret = i18n("Mainboard");
	}
	else if (query == TDEGenericDeviceType::Disk) {
		ret = i18n("Disk");
	}
	else if (query == TDEGenericDeviceType::SCSI) {
		ret = i18n("SCSI");
	}
	else if (query == TDEGenericDeviceType::StorageController) {
		ret = i18n("Storage Controller");
	}
	else if (query == TDEGenericDeviceType::Mouse) {
		ret = i18n("Mouse");
	}
	else if (query == TDEGenericDeviceType::Keyboard) {
		ret = i18n("Keyboard");
	}
	else if (query == TDEGenericDeviceType::HID) {
		ret = i18n("HID");
	}
	else if (query == TDEGenericDeviceType::Monitor) {
		ret = i18n("Monitor and Display");
	}
	else if (query == TDEGenericDeviceType::Network) {
		ret = i18n("Network");
	}
	else if (query == TDEGenericDeviceType::Printer) {
		ret = i18n("Printer");
	}
	else if (query == TDEGenericDeviceType::Scanner) {
		ret = i18n("Scanner");
	}
	else if (query == TDEGenericDeviceType::Sound) {
		ret = i18n("Sound");
	}
	else if (query == TDEGenericDeviceType::VideoCapture) {
		ret = i18n("Video Capture");
	}
	else if (query == TDEGenericDeviceType::IEEE1394) {
		ret = i18n("IEEE1394");
	}
	else if (query == TDEGenericDeviceType::PCMCIA) {
		ret = i18n("PCMCIA");
	}
	else if (query == TDEGenericDeviceType::Camera) {
		ret = i18n("Camera");
	}
	else if (query == TDEGenericDeviceType::TextIO) {
		ret = i18n("Text I/O");
	}
	else if (query == TDEGenericDeviceType::Serial) {
		ret = i18n("Serial Communications Controller");
	}
	else if (query == TDEGenericDeviceType::Parallel) {
		ret = i18n("Parallel Port");
	}
	else if (query == TDEGenericDeviceType::Peripheral) {
		ret = i18n("Peripheral");
	}
	else if (query == TDEGenericDeviceType::Backlight) {
		ret = i18n("Backlight");
	}
	else if (query == TDEGenericDeviceType::Battery) {
		ret = i18n("Battery");
	}
	else if (query == TDEGenericDeviceType::PowerSupply) {
		ret = i18n("Power Supply");
	}
	else if (query == TDEGenericDeviceType::Dock) {
		ret = i18n("Docking Station");
	}
	else if (query == TDEGenericDeviceType::ThermalSensor) {
		ret = i18n("Thermal Sensor");
	}
	else if (query == TDEGenericDeviceType::ThermalControl) {
		ret = i18n("Thermal Control");
	}
	else if (query == TDEGenericDeviceType::Bridge) {
		ret = i18n("Bridge");
	}
	else if (query == TDEGenericDeviceType::Platform) {
		ret = i18n("Platform");
	}
	else if (query == TDEGenericDeviceType::Event) {
		ret = i18n("Platform Event");
	}
	else if (query == TDEGenericDeviceType::Input) {
		ret = i18n("Platform Input");
	}
	else if (query == TDEGenericDeviceType::PNP) {
		ret = i18n("Plug and Play");
	}
	else if (query == TDEGenericDeviceType::OtherACPI) {
		ret = i18n("Other ACPI");
	}
	else if (query == TDEGenericDeviceType::OtherUSB) {
		ret = i18n("Other USB");
	}
	else if (query == TDEGenericDeviceType::OtherMultimedia) {
		ret = i18n("Other Multimedia");
	}
	else if (query == TDEGenericDeviceType::OtherPeripheral) {
		ret = i18n("Other Peripheral");
	}
	else if (query == TDEGenericDeviceType::OtherSensor) {
		ret = i18n("Other Sensor");
	}
	else if (query == TDEGenericDeviceType::OtherVirtual) {
		ret = i18n("Other Virtual");
	}
	else {
		ret = i18n("Unknown Device");
	}

	return ret;
}

TQPixmap TDEHardwareDevices::getDeviceTypeIconFromType(TDEGenericDeviceType::TDEGenericDeviceType query, KIcon::StdSizes size) {
	TQPixmap ret = DesktopIcon("misc", size);

// 	// Keep this in sync with the TDEGenericDeviceType definition in the header
	if (query == TDEGenericDeviceType::Root) {
		ret = DesktopIcon("kcmdevices", size);
	}
	else if (query == TDEGenericDeviceType::RootSystem) {
		ret = DesktopIcon("kcmdevices", size);
	}
	else if (query == TDEGenericDeviceType::CPU) {
		ret = DesktopIcon("kcmprocessor", size);
	}
	else if (query == TDEGenericDeviceType::GPU) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::RAM) {
		ret = DesktopIcon("memory", size);
	}
	else if (query == TDEGenericDeviceType::Bus) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::I2C) {
		ret = DesktopIcon("input_devices_settings", size);
	}
	else if (query == TDEGenericDeviceType::MDIO) {
		ret = DesktopIcon("input_devices_settings", size);
	}
	else if (query == TDEGenericDeviceType::Mainboard) {
		ret = DesktopIcon("kcmpci", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::Disk) {
		ret = DesktopIcon("hdd_unmount", size);
	}
	else if (query == TDEGenericDeviceType::SCSI) {
		ret = DesktopIcon("kcmscsi", size);
	}
	else if (query == TDEGenericDeviceType::StorageController) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::Mouse) {
		ret = DesktopIcon("mouse", size);
	}
	else if (query == TDEGenericDeviceType::Keyboard) {
		ret = DesktopIcon("keyboard", size);
	}
	else if (query == TDEGenericDeviceType::HID) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::Monitor) {
		ret = DesktopIcon("background", size);
	}
	else if (query == TDEGenericDeviceType::Network) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::Printer) {
		ret = DesktopIcon("printer1", size);
	}
	else if (query == TDEGenericDeviceType::Scanner) {
		ret = DesktopIcon("scanner", size);
	}
	else if (query == TDEGenericDeviceType::Sound) {
		ret = DesktopIcon("kcmsound", size);
	}
	else if (query == TDEGenericDeviceType::VideoCapture) {
		ret = DesktopIcon("tv", size);		// FIXME
	}
	else if (query == TDEGenericDeviceType::IEEE1394) {
		ret = DesktopIcon("ieee1394", size);
	}
	else if (query == TDEGenericDeviceType::PCMCIA) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::Camera) {
		ret = DesktopIcon("camera", size);
	}
	else if (query == TDEGenericDeviceType::Serial) {
		ret = DesktopIcon("input_devices_settings", size);
	}
	else if (query == TDEGenericDeviceType::Parallel) {
		ret = DesktopIcon("input_devices_settings", size);
	}
	else if (query == TDEGenericDeviceType::TextIO) {
		ret = DesktopIcon("chardevice", size);
	}
	else if (query == TDEGenericDeviceType::Peripheral) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::Backlight) {
		ret = DesktopIcon("kscreensaver", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::Battery) {
		ret = DesktopIcon("energy", size);
	}
	else if (query == TDEGenericDeviceType::PowerSupply) {
		ret = DesktopIcon("energy", size);
	}
	else if (query == TDEGenericDeviceType::Dock) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::ThermalSensor) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::ThermalControl) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::Bridge) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::Platform) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else if (query == TDEGenericDeviceType::Event) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else if (query == TDEGenericDeviceType::Input) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else if (query == TDEGenericDeviceType::PNP) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else if (query == TDEGenericDeviceType::OtherACPI) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::OtherUSB) {
		ret = DesktopIcon("usb", size);
	}
	else if (query == TDEGenericDeviceType::OtherMultimedia) {
		ret = DesktopIcon("kcmsound", size);
	}
	else if (query == TDEGenericDeviceType::OtherPeripheral) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::OtherSensor) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::OtherVirtual) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else {
		ret = DesktopIcon("hwinfo", size);
	}

	return ret;
}

TDERootSystemDevice* TDEHardwareDevices::rootSystemDevice() {
	TDEGenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->type() == TDEGenericDeviceType::RootSystem) {
			return dynamic_cast<TDERootSystemDevice*>(hwdevice);
		}
	}

	return 0;
}

TQString TDEHardwareDevices::bytesToFriendlySizeString(double bytes) {
	TQString prettystring;

	prettystring = TQString("%1B").arg(bytes);

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1KB").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1MB").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1GB").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1TB").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1PB").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1EB").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1ZB").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1YB").arg(bytes, 0, 'f', 1);
	}

	return prettystring;
}

TDEGenericHardwareList TDEHardwareDevices::listByDeviceClass(TDEGenericDeviceType::TDEGenericDeviceType cl) {
	TDEGenericHardwareList ret;
	ret.setAutoDelete(false);

	TDEGenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->type() == cl) {
			ret.append(hwdevice);
		}
	}

	return ret;
}

TDEGenericHardwareList TDEHardwareDevices::listAllPhysicalDevices() {
	TDEGenericHardwareList ret = m_deviceList;
	ret.setAutoDelete(false);

	return ret;
}

#include "tdehardwaredevices.moc"