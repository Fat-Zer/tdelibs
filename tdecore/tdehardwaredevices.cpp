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
#include <kconfig.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

#include <libudev.h>

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

// NOTE TO DEVELOPERS
// This command will greatly help when attempting to find properties to distinguish one device from another
// udevadm info --query=all --path=/sys/....

TDEGenericDevice::TDEGenericDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) {
	m_deviceType = dt;
	m_deviceName = dn;

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
}

TQString &TDEGenericDevice::modelID() {
	return m_modelID;
}

void TDEGenericDevice::setModelID(TQString id) {
	m_modelID = id;
}

bool TDEGenericDevice::blacklistedForUpdate() {
	return m_blacklistedForUpdate;
}

void TDEGenericDevice::setBlacklistedForUpdate(bool bl) {
	m_blacklistedForUpdate = bl;
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

TDEHardwareDevices::TDEHardwareDevices() {
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

		// Update internal device information
		queryHardwareInformation();
	}
}

TDEHardwareDevices::~TDEHardwareDevices() {
	// Stop mount scanning
	close(m_procMountsFd);

	// Tear down udev interface
	udev_unref(m_udevStruct);
}

void TDEHardwareDevices::rescanDeviceInformation(TDEGenericDevice* hwdevice) {
	struct udev_device *dev;
	dev = udev_device_new_from_syspath(m_udevStruct, hwdevice->systemPath().ascii());
	classifyUnknownDevice(dev, hwdevice, false);
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
						emit hardwareUpdated(hwdevice);
					}
					break;
				}
			}
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
		disktype = disktype | TDEDiskDeviceType::Optical;
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
	if (query == "CPU") {
		ret = TDEGenericDeviceType::CPU;
	}
	else if (query == "GPU") {
		ret = TDEGenericDeviceType::GPU;
	}
	else if (query == "RAM") {
		ret = TDEGenericDeviceType::RAM;
	}
	else if (query == "Mainboard") {
		ret = TDEGenericDeviceType::Mainboard;
	}
	else if (query == "Disk") {
		ret = TDEGenericDeviceType::Disk;
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
	else if (query == "IEEE1394") {
		ret = TDEGenericDeviceType::IEEE1394;
	}
	else if (query == "Camera") {
		ret = TDEGenericDeviceType::Camera;
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
	else if (query == "ThermalSensor") {
		ret = TDEGenericDeviceType::ThermalSensor;
	}
	else if (query == "ThermalControl") {
		ret = TDEGenericDeviceType::ThermalControl;
	}
	else if (query == "OtherACPI") {
		ret = TDEGenericDeviceType::OtherACPI;
	}
	else if (query == "OtherUSB") {
		ret = TDEGenericDeviceType::OtherUSB;
	}
	else if (query == "OtherPeripheral") {
		ret = TDEGenericDeviceType::OtherPeripheral;
	}
	else if (query == "OtherSensor") {
		ret = TDEGenericDeviceType::OtherSensor;
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
	// Classify device and create TDEW device object
	TQString devicename(udev_device_get_sysname(dev));
	TQString devicetype(udev_device_get_devtype(dev));
	TQString devicedriver(udev_device_get_driver(dev));
	TQString devicesubsystem(udev_device_get_subsystem(dev));
	TQString devicenode(udev_device_get_devnode(dev));
	TQString systempath(udev_device_get_syspath(dev));
	TQString devicevendorid(udev_device_get_property_value(dev, "ID_VENDOR_ID"));
	TQString devicemodelid(udev_device_get_property_value(dev, "ID_MODEL_ID"));
	bool removable = false;
	TDEGenericDevice* device = existingdevice;

	// FIXME
	// Only a small subset of devices are classified right now
	// Figure out the remaining udev logic to classify the rest!
	// Helpful file: http://www.enlightenment.org/svn/e/trunk/PROTO/enna-explorer/src/bin/udev.c

	// Many devices do not provide their vendor/model ID via udev
	// Go after it manually...
	if (devicevendorid.isNull() || devicemodelid.isNull()) {
		bool done = false;
		TQString current_path = systempath;
		TQString modalias_string = TQString::null;;

		while (done == false) {
			TQString malnodename = current_path;
			malnodename.append("/modalias");
			TQFile malfile(malnodename);
			if (malfile.open(IO_ReadOnly)) {
				TQTextStream stream( &malfile );
				modalias_string = stream.readLine();
				malfile.close();
			}
			if (modalias_string.startsWith("pci") || modalias_string.startsWith("usb")) {
				done = true;
			}
			else {
				modalias_string = TQString::null;
				current_path.truncate(current_path.findRev("/"));
				if (!current_path.startsWith("/sys/devices")) {
					// Abort!
					done = true;
				}
			}
		}

		if (modalias_string != TQString::null) {
			int vloc = modalias_string.find("v");
			int dloc = modalias_string.find("d", vloc);
			// For added fun the device string lengths differ between pci and usb
			if (modalias_string.startsWith("pci")) {
				devicevendorid = modalias_string.mid(vloc+1, 8).lower();
				devicemodelid = modalias_string.mid(dloc+1, 8).lower();
				devicevendorid.remove(0,4);
				devicemodelid.remove(0,4);
			}
			if (modalias_string.startsWith("usb")) {
				devicevendorid = modalias_string.mid(vloc+1, 4).lower();
				devicemodelid = modalias_string.mid(dloc+1, 4).lower();
			}
		}
	}

	// Classify generic device type and create appropriate object
	if ((devicetype == "disk")
		|| (devicetype == "partition")
		|| (devicedriver == "floppy")
		) {
		if (!device) device = new TDEStorageDevice(TDEGenericDeviceType::Disk);
	}
	else if (devicetype.isNull()) {
		if (devicesubsystem == "acpi") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherACPI);
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
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::TextIO);
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

	updateBlacklists(device, dev);

	if (force_full_classification) {
		// Check external rules for possible device type overrides
		device = classifyUnknownDeviceByExternalRules(dev, device, false);
	}

	if (device->type() == TDEGenericDeviceType::Disk) {
		// Determine if disk is removable
		TQString removablenodename = udev_device_get_syspath(dev);
		removablenodename.append("/removable");
		FILE *fp = fopen(removablenodename.ascii(),"r");
		if (fp) {
			if (fgetc(fp) == '1') {
				removable = true;
			}
			fclose(fp);
		}

		// See if any other devices are exclusively using this device, such as the Device Mapper|
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
		TQString disktypestring(udev_device_get_property_value(dev, "ID_TYPE"));
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
			disktype = classifyDiskType(dev, devicebus, disktypestring, systempath, devicevendor, devicemodel, filesystemtype, devicedriver);
			sdevice->setDiskType(disktype);
			device = classifyUnknownDeviceByExternalRules(dev, device, true);	// Check external rules for possible subtype overrides
			disktype = sdevice->diskType();						// The type can be overridden by an external rule
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

		if (disktypestring.upper() == "CD") {
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

		if ((filesystemtype.upper() != "CRYPTO_LUKS") && (filesystemtype.upper() != "CRYPTO")  && (!filesystemtype.isNull())) {
			diskstatus = diskstatus | TDEDiskDeviceStatus::ContainsFilesystem;
		}

		// Set mountable flag if device is likely to be mountable
		diskstatus = diskstatus | TDEDiskDeviceStatus::Mountable;
		if ((!disktypestring.upper().isNull()) && (disktype & TDEDiskDeviceType::HDD)) {
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

	// Set basic device information again, as some information may have changed
	device->setName(devicename);
	device->setDeviceNode(devicenode);
	device->setSystemPath(systempath);
	device->setVendorID(devicevendorid);
	device->setModelID(devicemodelid);

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

	return true;
}

TDEGenericHardwareList &TDEHardwareDevices::listAllPhysicalDevices() {
	return m_deviceList;
}

#include "tdehardwaredevices.moc"