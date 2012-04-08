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

#include <libudev.h>

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

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

TQString &TDEGenericDevice::name() {
	return m_deviceName;
}

void TDEGenericDevice::setName(TQString dn) {
	m_deviceName = dn;
}

TQString &TDEGenericDevice::vendorName() {
	return m_vendorName;
}

void TDEGenericDevice::setVendorName(TQString vn) {
	m_vendorName = vn;
}

TQString &TDEGenericDevice::vendorModel() {
	return m_vendorModel;
}

void TDEGenericDevice::setVendorModel(TQString vm) {
	m_vendorModel = vm;
}

TQString &TDEGenericDevice::systemPath() {
	return m_systemPath;
}

void TDEGenericDevice::setSystemPath(TQString sp) {
	m_systemPath = sp;
}

TQString &TDEGenericDevice::deviceNode() {
	return m_deviceNode;
}

void TDEGenericDevice::setDeviceNode(TQString sn) {
	m_deviceNode = sn;
}

TQString &TDEGenericDevice::deviceBus() {
	return m_deviceBus;
}

void TDEGenericDevice::setDeviceBus(TQString db) {
	m_deviceBus = db;
}

TQString TDEGenericDevice::uniqueID() {
	m_uniqueID = m_systemPath+m_deviceNode;
	return m_uniqueID;
}

TQString &TDEGenericDevice::vendorID() {
	return m_vendorID;
}

void TDEGenericDevice::setVendorID(TQString id) {
	m_vendorID = id;
	m_vendorID.replace("0x", "");
}

TQString &TDEGenericDevice::modelID() {
	return m_modelID;
}

void TDEGenericDevice::setModelID(TQString id) {
	m_modelID = id;
	m_modelID.replace("0x", "");
}

TQString &TDEGenericDevice::vendorEncoded() {
	return m_vendorenc;
}

void TDEGenericDevice::setVendorEncoded(TQString id) {
	m_vendorenc = id;
}

TQString &TDEGenericDevice::modelEncoded() {
	return m_modelenc;
}

void TDEGenericDevice::setModelEncoded(TQString id) {
	m_modelenc = id;
}

TQString &TDEGenericDevice::subVendorID() {
	return m_subvendorID;
}

void TDEGenericDevice::setSubVendorID(TQString id) {
	m_subvendorID = id;
	m_subvendorID.replace("0x", "");
}

TQString &TDEGenericDevice::PCIClass() {
	return m_pciClass;
}

void TDEGenericDevice::setPCIClass(TQString cl) {
	m_pciClass = cl;
	m_pciClass.replace("0x", "");
}

TQString &TDEGenericDevice::moduleAlias() {
	return m_modAlias;
}

void TDEGenericDevice::setModuleAlias(TQString ma) {
	m_modAlias = ma;
}

TQString &TDEGenericDevice::deviceDriver() {
	return m_deviceDriver;
}

void TDEGenericDevice::setDeviceDriver(TQString dr) {
	m_deviceDriver = dr;
}

TQString &TDEGenericDevice::subsystem() {
	return m_subsystem;
}

void TDEGenericDevice::setSubsystem(TQString ss) {
	m_subsystem = ss;
}

TQString &TDEGenericDevice::subModelID() {
	return m_submodelID;
}

void TDEGenericDevice::setSubModelID(TQString id) {
	m_submodelID = id;
	m_submodelID.replace("0x", "");
}

void TDEGenericDevice::setParentDevice(TDEGenericDevice* pd) {
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

void TDEGenericDevice::setBlacklistedForUpdate(bool bl) {
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
		if (type() == TDEGenericDeviceType::Root) {
			TQString friendlyDriverName = m_systemPath;
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
			TQString pnpgentype = systemPath();
			pnpgentype.remove(0, pnpgentype.findRev("/")+1);
			pnpgentype.truncate(pnpgentype.find(":"));
			if (pnpgentype.startsWith("PNP")) {
				m_friendlyName = KGlobal::hardwareDevices()->findPNPDeviceName(pnpgentype);
			}
		}
	}

	if (m_friendlyName.isNull()) {
		// Could not identify based on model/vendor
		// Guess by type
		if (type() == TDEGenericDeviceType::CPU) {
			m_friendlyName = name();
		}
		// Guess by driver
		else if (!m_deviceDriver.isNull()) {
			TQString friendlyDriverName = m_deviceDriver.lower();
			friendlyDriverName[0] = friendlyDriverName[0].upper();
			m_friendlyName = i18n("Generic %1 Device").arg(friendlyDriverName);
		}
		else if (m_systemPath.lower().startsWith("/sys/devices/virtual")) {
			TQString friendlyDriverName = m_systemPath;
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

void TDEStorageDevice::setDiskType(TDEDiskDeviceType::TDEDiskDeviceType dt) {
	m_diskType = dt;
}

bool TDEStorageDevice::isDiskOfType(TDEDiskDeviceType::TDEDiskDeviceType tf) {
	return ((m_diskType&tf)!=TDEDiskDeviceType::Null);
}

TDEDiskDeviceStatus::TDEDiskDeviceStatus TDEStorageDevice::diskStatus() {
	return m_diskStatus;
}

void TDEStorageDevice::setDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus st) {
	m_diskStatus = st;
}

bool TDEStorageDevice::checkDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus sf) {
	return ((m_diskStatus&sf)!=(TDEDiskDeviceStatus::TDEDiskDeviceStatus)0);
}

TQString &TDEStorageDevice::diskLabel() {
	return m_diskName;
}

void TDEStorageDevice::setDiskLabel(TQString dn) {
	m_diskName = dn;
}

bool TDEStorageDevice::mediaInserted() {
	return m_mediaInserted;
}

void TDEStorageDevice::setMediaInserted(bool inserted) {
	m_mediaInserted = inserted;
}

TQString &TDEStorageDevice::fileSystemName() {
	return m_fileSystemName;
}

void TDEStorageDevice::setFileSystemName(TQString fn) {
	m_fileSystemName = fn;
}

TQString &TDEStorageDevice::fileSystemUsage() {
	return m_fileSystemUsage;
}

void TDEStorageDevice::setFileSystemUsage(TQString fu) {
	m_fileSystemUsage = fu;
}

TQString &TDEStorageDevice::diskUUID() {
	return m_diskUUID;
}

void TDEStorageDevice::setDiskUUID(TQString id) {
	m_diskUUID = id;
}

TQStringList &TDEStorageDevice::holdingDevices() {
	return m_holdingDevices;
}

void TDEStorageDevice::setHoldingDevices(TQStringList hd) {
	m_holdingDevices = hd;
}

TQStringList &TDEStorageDevice::slaveDevices() {
	return m_slaveDevices;
}

void TDEStorageDevice::setSlaveDevices(TQStringList sd) {
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
	double bytes = deviceSize();
	TQString prettystring;

	prettystring = TQString("%1b").arg(bytes);

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1Kb").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1Mb").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1Gb").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1Tb").arg(bytes, 0, 'f', 1);
	}

	if (bytes > 1024) {
		bytes = bytes / 1024;
		prettystring = TQString("%1Pb").arg(bytes, 0, 'f', 1);
	}

	return prettystring;
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

double &TDECPUDevice::frequency() {
	return m_frequency;
}

void TDECPUDevice::setFrequency(double fr) {
	m_frequency = fr;
}

double &TDECPUDevice::minFrequency() {
	return m_minfrequency;
}

void TDECPUDevice::setMinFrequency(double fr) {
	m_minfrequency = fr;
}

double &TDECPUDevice::maxFrequency() {
	return m_maxfrequency;
}

void TDECPUDevice::setMaxFrequency(double fr) {
	m_maxfrequency = fr;
}

double &TDECPUDevice::transitionLatency() {
	return m_transitionlatency;
}

void TDECPUDevice::setTransitionLatency(double tl) {
	m_transitionlatency = tl;
}

TQString &TDECPUDevice::governor() {
	return m_governor;
}

void TDECPUDevice::setGovernor(TQString gr) {
	m_governor = gr;
}

TQString &TDECPUDevice::scalingDriver() {
	return m_scalingdriver;
}

void TDECPUDevice::setScalingDriver(TQString dr) {
	m_scalingdriver = dr;
}

TQStringList &TDECPUDevice::dependentProcessors() {
	return m_tiedprocs;
}

void TDECPUDevice::setDependentProcessors(TQStringList dp) {
	m_tiedprocs = dp;
}

TQStringList &TDECPUDevice::availableFrequencies() {
	return m_frequencies;
}

void TDECPUDevice::setAvailableFrequencies(TQStringList af) {
	m_frequencies = af;
}

TDEHardwareDevices::TDEHardwareDevices() {
	// Initialize members
	pci_id_map = 0;
	usb_id_map = 0;
	pnp_id_map = 0;

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

		// Update internal device information
		queryHardwareInformation();
	}
}

TDEHardwareDevices::~TDEHardwareDevices() {
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
}

void TDEHardwareDevices::rescanDeviceInformation(TDEGenericDevice* hwdevice) {
	struct udev_device *dev;
	dev = udev_device_new_from_syspath(m_udevStruct, hwdevice->systemPath().ascii());
	classifyUnknownDevice(dev, hwdevice, false);
	updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
	udev_device_unref(dev);
}

TDEGenericDevice* TDEHardwareDevices::findBySystemPath(TQString syspath) {
	TDEGenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->systemPath() == syspath) {
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
			cdevice->setName(curline);
		}
		if (curline.startsWith("cpu MHz")) {
			curline.remove(0, curline.find(":")+1);
			curline = curline.stripWhiteSpace();
			if (cdevice->frequency() != curline.toDouble()) modified = true;
			cdevice->setFrequency(curline.toDouble());
		}
		if (curline.startsWith("vendor_id")) {
			curline.remove(0, curline.find(":")+1);
			curline = curline.stripWhiteSpace();
			if (cdevice->vendorName() != curline) modified = true;
			cdevice->setVendorName(curline);
			if (cdevice->vendorEncoded() != curline) modified = true;
			cdevice->setVendorEncoded(curline);
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
		}

		// Update CPU information structure
		if (cdevice->governor() != scalinggovernor) modified = true;
		cdevice->setGovernor(scalinggovernor);
		if (cdevice->scalingDriver() != scalingdriver) modified = true;
		cdevice->setScalingDriver(scalingdriver);
		if (cdevice->minFrequency() != minfrequency) modified = true;
		cdevice->setMinFrequency(minfrequency);
		if (cdevice->maxFrequency() != maxfrequency) modified = true;
		cdevice->setMaxFrequency(maxfrequency);
		if (cdevice->transitionLatency() != trlatency) modified = true;
		cdevice->setTransitionLatency(trlatency);
		if (cdevice->dependentProcessors().join(" ") != affectedcpulist.join(" ")) modified = true;
		cdevice->setDependentProcessors(affectedcpulist);
		if (cdevice->availableFrequencies().join(" ") != frequencylist.join(" ")) modified = true;
		cdevice->setAvailableFrequencies(frequencylist);
	}

	if (modified) {
		for (processorNumber=0; processorNumber<processorCount; processorNumber++) {
			TDEGenericDevice* hwdevice = findBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber));
			// Signal new information available
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
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RW_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_MINUS_RW")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW_DL")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_MINUS_RW_DL")) == "1")
			) {
			disktype = disktype | TDEDiskDeviceType::DVDRW;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD")) == "1") {
			disktype = disktype | TDEDiskDeviceType::BDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
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
	else if (query == "Battery") {
		ret = TDEGenericDeviceType::Battery;
	}
	else if (query == "Power") {
		ret = TDEGenericDeviceType::Power;
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
						sdevice->setDiskType(desiredSubdeviceType);
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
							device->setBlacklistedForUpdate(rulesFile.readBoolEntry("UPDATE_BLACKLISTED", device->blacklistedForUpdate()));
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
	TQString devicename(udev_device_get_sysname(dev));
	TQString devicetype(udev_device_get_devtype(dev));
	TQString devicedriver(udev_device_get_driver(dev));
	TQString devicesubsystem(udev_device_get_subsystem(dev));
	TQString devicenode(udev_device_get_devnode(dev));
	TQString systempath(udev_device_get_syspath(dev));
	TQString devicevendorid(udev_device_get_property_value(dev, "ID_VENDOR_ID"));
	TQString devicemodelid(udev_device_get_property_value(dev, "ID_MODEL_ID"));
	TQString devicevendoridenc(udev_device_get_property_value(dev, "ID_VENDOR_ENC"));
	TQString devicemodelidenc(udev_device_get_property_value(dev, "ID_MODEL_ENC"));
	TQString devicesubvendorid(udev_device_get_property_value(dev, "ID_SUBVENDOR_ID"));
	TQString devicesubmodelid(udev_device_get_property_value(dev, "ID_SUBMODEL_ID"));
	TQString devicetypestring(udev_device_get_property_value(dev, "ID_TYPE"));
	TQString devicetypestring_alt(udev_device_get_property_value(dev, "DEVTYPE"));
	TQString devicepciclass(udev_device_get_property_value(dev, "PCI_CLASS"));
	TDEGenericDevice* device = existingdevice;

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
			int vloc = devicemodalias.find("v");
			int dloc = devicemodalias.find("d", vloc);
			int svloc = devicemodalias.find("sv");
			int sdloc = devicemodalias.find("sd", vloc);
			// For added fun the device string lengths differ between pci and usb
			if (devicemodalias.startsWith("pci")) {
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

	// Pull out all event special devices and stuff them under Platform
	TQString syspath_tail = systempath.lower();
	syspath_tail.remove(0, syspath_tail.findRev("/")+1);
	if (syspath_tail.startsWith("event")) {
		if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
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
				// We support a limited number of specific PNP IDs here
				if (pnpgentype == "PNP0C0A") {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Battery);
				}
				else if (pnpgentype == "PNP0C0B") {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::ThermalControl);
				}
				else if (pnpgentype == "PNP0C0C") {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Power);
				}
				else if (pnpgentype == "PNP0C0D") {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Power);
				}
				else if (pnpgentype == "PNP0C0E") {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Power);
				}
				else if (pnpgentype == "PNP0C11") {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::ThermalSensor);
				}
				else {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherACPI);
				}
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
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherSensor);
		}
	}

	// Try to at least generally classify unclassified devices
	if (device == 0) {
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
		if (devicesubsystem == "net") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Network);
		}
		if (devicesubsystem == "i2c") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::I2C);
		}
		if (devicesubsystem == "mdio_bus") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::MDIO);
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
		if (devicesubsystem == "serio") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Serial);
		}
		if (devicesubsystem == "ppdev") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Parallel);
		}
		if (devicesubsystem == "bridge") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Bridge);
		}
		if ((devicesubsystem == "pci_bus")
			|| (devicesubsystem == "pci_express")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Bus);
		}
		if (devicesubsystem == "platform") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		if (devicesubsystem == "pnp") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::PNP);
		}
		if ((devicesubsystem == "hid")
			|| (devicesubsystem == "hidraw")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::HID);
		}
		if (devicesubsystem == "power_supply") {
			TQString powersupplyname(udev_device_get_property_value(dev, "POWER_SUPPLY_NAME"));
			if (powersupplyname.upper().startsWith("AC")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Power);
			}
			else {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Battery);
			}
		}
		if (devicesubsystem == "backlight") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Power);
		}

		// Moderate accuracy classification, if PCI device class is available
		// See http://www.acm.uiuc.edu/sigops/roll_your_own/7.c.1.html for codes and meanings
		if (!devicepciclass.isNull()) {
			// Pre PCI 2.0
			if (devicepciclass.startsWith("0001")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::GPU);
			}
			// Post PCI 2.0
			if (devicepciclass.startsWith("01")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::StorageController);
			}
			if (devicepciclass.startsWith("02")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Network);
			}
			if (devicepciclass.startsWith("03")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::GPU);
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

	// Set preliminary basic device information
	device->setName(devicename);
	device->setDeviceNode(devicenode);
	device->setSystemPath(systempath);
	device->setVendorID(devicevendorid);
	device->setModelID(devicemodelid);
	device->setVendorEncoded(devicevendoridenc);
	device->setModelEncoded(devicemodelidenc);
	device->setSubVendorID(devicesubvendorid);
	device->setSubModelID(devicesubmodelid);
	device->setModuleAlias(devicemodalias);
	device->setDeviceDriver(devicedriver);
	device->setSubsystem(devicesubsystem);
	device->setPCIClass(devicepciclass);

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

		device->setVendorName(devicevendor);
		device->setVendorModel(devicemodel);
		device->setDeviceBus(devicebus);

		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);

		TDEDiskDeviceType::TDEDiskDeviceType disktype = sdevice->diskType();
		TDEDiskDeviceStatus::TDEDiskDeviceStatus diskstatus = sdevice->diskStatus();

		if (force_full_classification) {
			disktype = classifyDiskType(dev, devicebus, devicetypestring, systempath, devicevendor, devicemodel, filesystemtype, devicedriver);
			sdevice->setDiskType(disktype);
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
			sdevice->setMediaInserted(!(TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) == "0"));
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
				sdevice->setMediaInserted((zipsize.toInt() != 0));
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

		sdevice->setDiskType(disktype);
		sdevice->setDiskUUID(diskuuid);
		sdevice->setDiskStatus(diskstatus);
		sdevice->setFileSystemName(filesystemtype);
		sdevice->setFileSystemUsage(filesystemusage);
		sdevice->setSlaveDevices(slaveDeviceNodes);
		sdevice->setHoldingDevices(holdingDeviceNodes);

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

		sdevice->setDiskLabel(disklabel);
	}

	if (device->type() == TDEGenericDeviceType::Network) {
		// Network devices don't have devices nodes per se, but we can at least return the Linux network name...
		devicenode = systempath;
		devicenode.remove(0, devicenode.findRev("/")+1);
	}

	// Set basic device information again, as some information may have changed
	device->setName(devicename);
	device->setDeviceNode(devicenode);
	device->setSystemPath(systempath);
	device->setVendorID(devicevendorid);
	device->setModelID(devicemodelid);
	device->setVendorEncoded(devicevendoridenc);
	device->setModelEncoded(devicemodelidenc);
	device->setSubVendorID(devicesubvendorid);
	device->setSubModelID(devicesubmodelid);
	device->setDeviceDriver(devicedriver);
	device->setSubsystem(devicesubsystem);
	device->setPCIClass(devicepciclass);

	return device;
}

void TDEHardwareDevices::updateBlacklists(TDEGenericDevice* hwdevice, udev_device* dev) {
	// HACK
	// I am lucky enough to have a Flash drive that spams udev continually with device change events
	// I imagine I am not the only one, so here is a section in which specific devices can be blacklisted!

	// For "U3 System" fake CD
	if ((hwdevice->vendorID() == "08ec") && (hwdevice->modelID() == "0020") && (TQString(udev_device_get_property_value(dev, "ID_TYPE")) == "cd")) {
		hwdevice->setBlacklistedForUpdate(true);
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

	hwdevice->setParentDevice(parentdevice);
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
	// Add core top-level devices in /sys/devices to the hardware listing
	TQStringList holdingDeviceNodes;
	TQString devicesnodename = "/sys/devices";
	TQDir devicesdir(devicesnodename);
	devicesdir.setFilter(TQDir::All);
	TQString nodename;
	TDEGenericDevice *hwdevice;
	const TQFileInfoList *dirlist = devicesdir.entryInfoList();
	if (dirlist) {
		TQFileInfoListIterator devicesdirit(*dirlist);
		TQFileInfo *dirfi;
		while ( (dirfi = devicesdirit.current()) != 0 ) {
			nodename = dirfi->fileName();
			if (nodename != "." && nodename != "..") {
				hwdevice = new TDEGenericDevice(TDEGenericDeviceType::Root);
				hwdevice->setSystemPath(dirfi->absFilePath());
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
				hwdevice->setSystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber));
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

TQString TDEHardwareDevices::getFriendlyDeviceTypeStringFromType(TDEGenericDeviceType::TDEGenericDeviceType query) {
	TQString ret = "Unknown Device";

	// Keep this in sync with the TDEGenericDeviceType definition in the header
	if (query == TDEGenericDeviceType::Root) {
		ret = i18n("Root");
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
	else if (query == TDEGenericDeviceType::Battery) {
		ret = i18n("Battery");
	}
	else if (query == TDEGenericDeviceType::Power) {
		ret = i18n("Power Device");
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
	else if (query == TDEGenericDeviceType::Battery) {
		ret = DesktopIcon("energy", size);
	}
	else if (query == TDEGenericDeviceType::Power) {
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

TQPtrList<TDEGenericDevice> TDEHardwareDevices::listByDeviceClass(TDEGenericDeviceType::TDEGenericDeviceType cl) {
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