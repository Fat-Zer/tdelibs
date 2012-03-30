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

#include <kglobal.h>
#include <ktempfile.h>

#include <libudev.h>

#include <fcntl.h>
#include <poll.h>

// NOTE TO DEVELOPERS
// This command will greatly help when attempting to find properties to distinguish one device from another
// udevadm info --query=all --path=/sys/....

TDEGenericDevice::TDEGenericDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) {
	m_deviceType = dt;
	m_deviceName = dn;
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

TDEStorageDevice::TDEStorageDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn), m_mediaInserted(true) {
}

TDEStorageDevice::~TDEStorageDevice() {
}

TDEDiskDeviceType::TDEDiskDeviceType TDEStorageDevice::diskType() {
	return m_diskType;
}

void TDEStorageDevice::setDiskType(TDEDiskDeviceType::TDEDiskDeviceType dt) {
	m_diskType = dt;
}

TDEDiskDeviceStatus::TDEDiskDeviceStatus TDEStorageDevice::diskStatus() {
	return m_diskStatus;
}

void TDEStorageDevice::setDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus st) {
	m_diskStatus = st;
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
	m_slaveDevices = hd;
}

TQStringList &TDEStorageDevice::slaveDevices() {
	return m_slaveDevices;
}

void TDEStorageDevice::setSlaveDevices(TQStringList sd) {
	m_slaveDevices = sd;
}

TQString TDEStorageDevice::mountPath() {
	// See if this device node is mounted
	// This requires parsing /proc/mounts, looking for deviceNode()

	// The Device Mapper throws a monkey wrench into this
	// It likes to advertise mounts as /dev/mapper/<something>,
	// where <something> is listed in <system path>/dm/name
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
			if ((*mountInfo.at(0) == deviceNode()) || (*mountInfo.at(0) == dmaltname)) {
				return *mountInfo.at(1);
			}
			lines += line;
		}
		file.close();
	}

	// While this device is not directly mounted, it could concievably be mounted via the Device Mapper
	// If so, try to retrieve the mount path...
	TQStringList slaveDeviceList = slaveDevices();
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

TQString TDEStorageDevice::mountDevice(TQString mediaName) {
	TQString ret = mountPath();

	if (!ret.isNull()) {
		return ret;
	}

	// Create dummy password file
	KTempFile passwordFile(TQString::null, "tmp", 0600);
	passwordFile.setAutoDelete(true);

	TQString command = TQString("pmount -p %1 %2").arg(passwordFile.name()).arg(deviceNode());
	if (!mediaName.isNull()) {
		command.append(mediaName);
	}

	if (system(command.ascii()) == 0) {
		ret = mountPath();
	}

	return ret;
}

TQString TDEStorageDevice::mountEncryptedDevice(TQString passphrase, TQString mediaName) {
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

	TQString command = TQString("pmount -p %1 %2").arg(passwordFile.name()).arg(deviceNode());
	if (!mediaName.isNull()) {
		command.append(mediaName);
	}

	if (system(command.ascii()) == 0) {
		ret = mountPath();
	}

	return ret;
}

bool TDEStorageDevice::unmountDevice() {
	TQString mountpoint = mountPath();

	if (mountpoint.isNull()) {
		return true;
	}

	TQString command = TQString("pumount %1").arg(mountpoint);
	if (system(command.ascii()) == 0) {
		return true;
	}

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

		m_devScanTimer = new TQTimer();
		connect( m_devScanTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(checkForHotPluggedHardware()) );
		m_devScanTimer->start(1, TRUE);

		// Monitor for changed mounts
		m_mountScanTimer = new TQTimer();
		connect( m_mountScanTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(checkForModifiedMounts()) );
		m_mountScanTimer->start(1, TRUE);

		// Update internal device information
		queryHardwareInformation();
	}
}

TDEHardwareDevices::~TDEHardwareDevices() {
	// Stop hardware scanning
	m_devScanTimer->stop();
	delete m_devScanTimer;

	// Stop mount scanning
	m_mountScanTimer->stop();
	delete m_mountScanTimer;

	// Tear down udev interface
	udev_unref(m_udevStruct);
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

void TDEHardwareDevices::checkForHotPluggedHardware() {
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
				emit hardwareAdded(*device);
			}
		}
		else if (actionevent == "remove") {
			// Delete device from hardware listing
			TQString systempath(udev_device_get_syspath(dev));
			TDEGenericDevice *hwdevice;
			for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
				if (hwdevice->systemPath() == systempath) {
					m_deviceList.remove(hwdevice);
					emit hardwareAdded(*hwdevice);
					break;
				}
			}
		}
	}

	// Continue scanning for added/removed hardware
	m_devScanTimer->start(1, TRUE);
}

void TDEHardwareDevices::checkForModifiedMounts() {
	int mfd = open("/proc/mounts", O_RDONLY, 0);
	struct pollfd pfd;
	int rv;
	
	pfd.fd = mfd;
	pfd.events = POLLERR | POLLPRI;
	pfd.revents = 0;
	while ((rv = poll(&pfd, 1, 5)) >= 0) {
		if (pfd.revents & POLLERR) {
			emit mountTableModified();
		}
	
		pfd.revents = 0;
	}

	// Continue scanning for changed mounts
	m_mountScanTimer->start(1, TRUE);
}

TDEGenericDevice* TDEHardwareDevices::classifyUnknownDevice(udev_device* dev) {
	// Classify device and create TDEW device object
	TQString devicename(udev_device_get_sysname(dev));
	TQString devicetype(udev_device_get_devtype(dev));
	TQString devicedriver(udev_device_get_driver(dev));
	TQString devicesubsystem(udev_device_get_subsystem(dev));
	TQString devicenode(udev_device_get_devnode(dev));
	TQString systempath(udev_device_get_syspath(dev));
	bool removable = false;
	TDEGenericDevice* device = 0;

	// FIXME
	// Only a small subset of devices are classified right now
	// Figure out the remaining udev logic to classify the rest!
	// Helpful file: http://www.enlightenment.org/svn/e/trunk/PROTO/enna-explorer/src/bin/udev.c

	if ((devicetype == "disk") || (devicetype == "partition")) {
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

		device = new TDEStorageDevice(TDEGenericDeviceType::Disk);

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

		TDEDiskDeviceType::TDEDiskDeviceType disktype = (TDEDiskDeviceType::TDEDiskDeviceType)0;
		TDEDiskDeviceStatus::TDEDiskDeviceStatus diskstatus = (TDEDiskDeviceStatus::TDEDiskDeviceStatus)0;

		if (devicebus.upper() == "USB") {
			disktype = disktype | TDEDiskDeviceType::USB;
		}

		if (disktypestring.upper() == "ZIP") {
			disktype = disktype | TDEDiskDeviceType::Zip;
		}
		if ((devicevendor.upper() == "IOMEGA") && (devicemodel.upper().contains("ZIP"))) {
			disktype = disktype | TDEDiskDeviceType::Zip;
		}

		if (disktypestring.upper() == "FLOPPY") {
			disktype = disktype | TDEDiskDeviceType::Floppy;
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

		if (disktypestring.upper() == "CD") {
			if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) == "1") {
				disktype = disktype | TDEDiskDeviceType::CDROM;
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
			if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_STATE")).upper() == "BLANK") {
				diskstatus = diskstatus | TDEDiskDeviceStatus::Blank;
			}

			sdevice->setMediaInserted(!(TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) == "0"));
		}

		if (removable) {
			diskstatus = diskstatus | TDEDiskDeviceStatus::Removable;
		}

		if (filesystemtype.upper() == "CRYPTO_LUKS") {
			disktype = disktype | TDEDiskDeviceType::LUKS;
		}
		else if (filesystemtype.upper() == "CRYPTO") {
			disktype = disktype | TDEDiskDeviceType::OtherCrypted;
		}

		// Detect RAM and Loop devices, since udev can't seem to...
		if (systempath.startsWith("/sys/devices/virtual/block/ram")) {
			disktype = disktype | TDEDiskDeviceType::RAM;
		}
		if (systempath.startsWith("/sys/devices/virtual/block/loop")) {
			disktype = disktype | TDEDiskDeviceType::Loop;
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
			}
			udev_device_unref(slavedev);
		}

		sdevice->setDiskType(disktype);
		sdevice->setDiskUUID(diskuuid);
		sdevice->setDiskLabel(disklabel);
		sdevice->setDiskStatus(diskstatus);
		sdevice->setFileSystemName(filesystemtype);
		sdevice->setFileSystemUsage(filesystemusage);
		sdevice->setSlaveDevices(slaveDeviceNodes);
		sdevice->setHoldingDevices(holdingDeviceNodes);

		printf("DISK DEVICE name: %s type: %s subsystem: %s vendor: %s model: %s bus: %s label: %s filesystem: %s disk type: %s disk type flags: 0x%08x media inserted: %d [Node Path: %s] [Syspath: %s]\n\r\n\r", devicename.ascii(), devicetype.ascii(), devicesubsystem.ascii(), devicevendor.ascii(), devicemodel.ascii(), devicebus.ascii(), disklabel.ascii(), filesystemtype.ascii(), disktypestring.ascii(), disktype, sdevice->mediaInserted(), devicenode.ascii(), udev_device_get_syspath(dev)); fflush(stdout);
	}
	else if (devicetype.isNull()) {
		if (devicesubsystem == "acpi") {
			device = new TDEGenericDevice(TDEGenericDeviceType::OtherACPI);
		}
		else if (devicesubsystem == "input") {
			// Figure out if this device is a mouse, keyboard, or something else
			// Check for mouse
			// udev doesn't reliably help here, so guess from the device name
			if (systempath.contains("/mouse")) {
				device = new TDEGenericDevice(TDEGenericDeviceType::Mouse);
			}
			if (!device) {
				// Second mouse check 
				// Look for ID_INPUT_MOUSE property presence
				if (udev_device_get_property_value(dev, "ID_INPUT_MOUSE") != 0) {
					device = new TDEGenericDevice(TDEGenericDeviceType::Mouse);
				}
			}
			if (!device) {
				// Check for keyboard
				// Look for ID_INPUT_KEYBOARD property presence
				if (udev_device_get_property_value(dev, "ID_INPUT_KEYBOARD") != 0) {
					device = new TDEGenericDevice(TDEGenericDeviceType::Keyboard);
				}
			}
			if (!device) {
				device = new TDEGenericDevice(TDEGenericDeviceType::HID);
			}
		}
		else if (devicesubsystem == "tty") {
			device = new TDEGenericDevice(TDEGenericDeviceType::TextIO);
		}
		else if (devicesubsystem == "thermal") {
			// FIXME
			// Figure out a way to differentiate between ThermalControl (fans and coolers) and ThermalSensor types
			device = new TDEGenericDevice(TDEGenericDeviceType::ThermalControl);
		}
		else if (devicesubsystem == "hwmon") {
			// FIXME
			// This might pick up thermal sensors
			device = new TDEGenericDevice(TDEGenericDeviceType::OtherSensor);
		}
	}

	if (device == 0) {
		// Unhandled
		device = new TDEGenericDevice(TDEGenericDeviceType::Other);
		printf("[FIXME] UNCLASSIFIED DEVICE name: %s type: %s subsystem: %s driver: %s [Node Path: %s] [Syspath: %s]\n\r", devicename.ascii(), devicetype.ascii(), devicesubsystem.ascii(), devicedriver.ascii(), devicenode.ascii(), udev_device_get_syspath(dev)); fflush(stdout);
	}
	device->setName(devicename);
	device->setDeviceNode(devicenode);
	device->setSystemPath(systempath);

	return device;
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