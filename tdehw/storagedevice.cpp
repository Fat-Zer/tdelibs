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

#include "storagedevice.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

#include <tqpixmap.h>
#include <tqfile.h>

#include <tdelocale.h> 
#include <kiconloader.h> 
#include <tdetempfile.h> 

#include "hardwaredevices.h" 

#include "config.h"

// uDisks2 integration
#if defined(WITH_UDISKS) || defined(WITH_UDISKS2)
 	#include <tqdbusdata.h>
 	#include <tqdbusmessage.h>
 	#include <tqdbusproxy.h>
 	#include <tqdbusvariant.h>
 	#include <tqdbusconnection.h>
 	#include <tqdbuserror.h>
 	#include <tqdbusdatamap.h>
 	#include <tqdbusobjectpath.h>
#endif // defined(WITH_UDISKS) || defined(WITH_UDISKS2)
#if defined(WITH_UDISKS)
 	#include "tqdbusdatalist.h"
#endif // ddefined(WITH_UDISKS)

using namespace TDEHW;

StorageDevice::StorageDevice(GenericDeviceType::GenericDeviceType dt, TQString dn) : GenericDevice(dt, dn), m_mediaInserted(true) {
	m_diskType = DiskDeviceType::Null;
	m_diskStatus = DiskDeviceStatus::Null;
}

StorageDevice::~StorageDevice() {
}

DiskDeviceType::DiskDeviceType StorageDevice::diskType() {
	return m_diskType;
}

void StorageDevice::internalSetDiskType(DiskDeviceType::DiskDeviceType dt) {
	m_diskType = dt;
}

bool StorageDevice::isDiskOfType(DiskDeviceType::DiskDeviceType tf) {
	return ((m_diskType&tf)!=DiskDeviceType::Null);
}

DiskDeviceStatus::DiskDeviceStatus StorageDevice::diskStatus() {
	return m_diskStatus;
}

void StorageDevice::internalSetDiskStatus(DiskDeviceStatus::DiskDeviceStatus st) {
	m_diskStatus = st;
}

bool StorageDevice::checkDiskStatus(DiskDeviceStatus::DiskDeviceStatus sf) {
	return ((m_diskStatus&sf)!=(DiskDeviceStatus::DiskDeviceStatus)0);
}

bool StorageDevice::lockDriveMedia(bool lock) {
	int fd = open(deviceNode().ascii(), O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		return false;
	}
	if (ioctl(fd, CDROM_LOCKDOOR, (lock)?1:0) != 0) {
		close(fd);
		return false;
	}
	else {
		close(fd);
		return true;
	}
}

bool ejectDriveUDisks(StorageDevice* sdevice) {
#ifdef WITH_UDISKS
	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = sdevice->deviceNode();
		blockDeviceString.replace("/dev/", "");
		blockDeviceString = "/org/freedesktop/UDisks/devices/" + blockDeviceString;

		// Eject the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks", blockDeviceString, "org.freedesktop.UDisks.Device", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			TQT_DBusDataList options;
			params << TQT_DBusData::fromList(options);
			TQT_DBusMessage reply = driveControl.sendWithReply("DriveEject", params, &error);
			if (error.isValid()) {
				// Error!
				printf("[ERROR] %s\n", error.name().ascii()); fflush(stdout);
				return FALSE;
			}
			else {
				return TRUE;
			}
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
#else // WITH_UDISKS
	return FALSE;
#endif // WITH_UDISKS
}

bool ejectDriveUDisks2(StorageDevice* sdevice) {
#ifdef WITH_UDISKS2
	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = sdevice->deviceNode();
		blockDeviceString.replace("/dev/", "");
		blockDeviceString = "/org/freedesktop/UDisks2/block_devices/" + blockDeviceString;
		TQT_DBusProxy hardwareControl("org.freedesktop.UDisks2", blockDeviceString, "org.freedesktop.DBus.Properties", dbusConn);
		if (hardwareControl.canSend()) {
			// get associated udisks2 drive path
			TQT_DBusError error;
			TQValueList<TQT_DBusData> params;
			params << TQT_DBusData::fromString("org.freedesktop.UDisks2.Block") << TQT_DBusData::fromString("Drive");
			TQT_DBusMessage reply = hardwareControl.sendWithReply("Get", params, &error);
			if (error.isValid()) {
				// Error!
				printf("[ERROR] %s\n", error.name().ascii()); fflush(stdout);
				return FALSE;
			}
			else {
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					TQT_DBusObjectPath driveObjectPath = reply[0].toVariant().value.toObjectPath();
					if (!driveObjectPath.isValid()) {
						return FALSE;
					}
	
					error = TQT_DBusError();
					TQT_DBusProxy driveInformation("org.freedesktop.UDisks2", driveObjectPath, "org.freedesktop.DBus.Properties", dbusConn);
					// can eject?
					TQValueList<TQT_DBusData> params;
					params << TQT_DBusData::fromString("org.freedesktop.UDisks2.Drive") << TQT_DBusData::fromString("Ejectable");
					TQT_DBusMessage reply = driveInformation.sendWithReply("Get", params, &error);
					if (error.isValid()) {
						// Error!
						printf("[ERROR] %s\n", error.name().ascii()); fflush(stdout);
						return FALSE;
					}
					if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
						bool ejectable = reply[0].toVariant().value.toBool();
						if (!ejectable) {
							return FALSE;
						}
	
						// Eject the drive!
						TQT_DBusProxy driveControl("org.freedesktop.UDisks2", driveObjectPath, "org.freedesktop.UDisks2.Drive", dbusConn);
						TQValueList<TQT_DBusData> params;
						TQT_DBusDataMap<TQString> options(TQT_DBusData::Variant);
						params << TQT_DBusData::fromStringKeyMap(options);
						TQT_DBusMessage reply = driveControl.sendWithReply("Eject", params, &error);
						if (error.isValid()) {
							// Error!
							printf("[ERROR] %s\n", error.name().ascii()); fflush(stdout);
							return FALSE;
						}
						else {
							return TRUE;
						}
					}
					else {
						return FALSE;
					}
				}
				else {
					return FALSE;
				}
			}
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
#else // WITH_UDISKS2
	return FALSE;
#endif // WITH_UDISKS2
}

bool StorageDevice::ejectDrive() {
	if (ejectDriveUDisks2(this)) {
		return TRUE;
	}
	else {
#ifdef WITH_UDISKS2
		printf("[tdehwlib] Failed to eject drive '%s' via udisks2, falling back to alternate mechanism\n", deviceNode().ascii());
#endif // WITH_UDISKS2
		if (ejectDriveUDisks(this)) {
			return TRUE;
		}
		else {
#ifdef WITH_UDISKS
			printf("[tdehwlib] Failed to eject drive '%s' via udisks, falling back to alternate mechanism\n", deviceNode().ascii());
#endif // WITH_UDISKS
			TQString command = TQString("eject -v '%1' 2>&1").arg(deviceNode());
	
			FILE *exepipe = popen(command.ascii(), "r");
			if (exepipe) {
				TQString pmount_output;
				char buffer[8092];
				pmount_output = fgets(buffer, sizeof(buffer), exepipe);
				int retcode = pclose(exepipe);
				if (retcode == 0) {
					return TRUE;
				}
				else {
					printf("[tdehwlib] Failed to eject drive '%s' via 'eject' command\n", deviceNode().ascii());
					return FALSE;
				}
			}
			else {
				printf("[tdehwlib] Failed to eject drive '%s' via 'eject' command\n", deviceNode().ascii());
				return FALSE;
			}
		}
	}
}

bool StorageDevice::ejectDriveMedia() {
	int fd = open(deviceNode().ascii(), O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		return false;
	}
	if (ioctl(fd, CDROMEJECT) != 0) {
		close(fd);
		return false;
	}
	else {
		close(fd);
		return true;
	}
}

TQString StorageDevice::diskLabel() {
	return m_diskName;
}

void StorageDevice::internalSetDiskLabel(TQString dn) {
	m_diskName = dn;
}

bool StorageDevice::mediaInserted() {
	return m_mediaInserted;
}

void StorageDevice::internalSetMediaInserted(bool inserted) {
	m_mediaInserted = inserted;
}

TQString StorageDevice::fileSystemName() {
	return m_fileSystemName;
}

void StorageDevice::internalSetFileSystemName(TQString fn) {
	m_fileSystemName = fn;
}

TQString StorageDevice::fileSystemUsage() {
	return m_fileSystemUsage;
}

void StorageDevice::internalSetFileSystemUsage(TQString fu) {
	m_fileSystemUsage = fu;
}

TQString StorageDevice::diskUUID() {
	return m_diskUUID;
}

void StorageDevice::internalSetDiskUUID(TQString id) {
	m_diskUUID = id;
}

TQStringList StorageDevice::holdingDevices() {
	return m_holdingDevices;
}

void StorageDevice::internalSetHoldingDevices(TQStringList hd) {
	m_holdingDevices = hd;
}

TQStringList StorageDevice::slaveDevices() {
	return m_slaveDevices;
}

void StorageDevice::internalSetSlaveDevices(TQStringList sd) {
	m_slaveDevices = sd;
}

TQString StorageDevice::friendlyName() {
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

	if (isDiskOfType(DiskDeviceType::Camera)) {
		return GenericDevice::friendlyName();
	}

	if (isDiskOfType(DiskDeviceType::Floppy)) {
		return friendlyDeviceType();
	}

	TQString label = diskLabel();
	if (label.isNull()) {
		if (deviceSize() > 0) {
			if (checkDiskStatus(DiskDeviceStatus::Hotpluggable)) {
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

TQString StorageDevice::friendlyDeviceType() {
	TQString ret = i18n("Hard Disk Drive");

	// Keep this in sync with StorageDevice::icon(TDEIcon::StdSizes size) below
	if (isDiskOfType(DiskDeviceType::Floppy)) {
		ret = i18n("Floppy Drive");
	}
	if (isDiskOfType(DiskDeviceType::Optical)) {
		ret = i18n("Optical Drive");
	}
	if (isDiskOfType(DiskDeviceType::CDROM)) {
		ret = i18n("CDROM Drive");
	}
	if (isDiskOfType(DiskDeviceType::CDRW)) {
		ret = i18n("CDRW Drive");
	}
	if (isDiskOfType(DiskDeviceType::DVDROM)) {
		ret = i18n("DVD Drive");
	}
	if (isDiskOfType(DiskDeviceType::DVDRW)) {
		ret = i18n("DVDRW Drive");
	}
	if (isDiskOfType(DiskDeviceType::DVDRAM)) {
		ret = i18n("DVDRAM Drive");
	}
	if (isDiskOfType(DiskDeviceType::Zip)) {
		ret = i18n("Zip Drive");
	}
	if (isDiskOfType(DiskDeviceType::Tape)) {
		ret = i18n("Tape Drive");
	}
	if (isDiskOfType(DiskDeviceType::Camera)) {
		ret = i18n("Digital Camera");
	}

	if (isDiskOfType(DiskDeviceType::HDD)) {
		ret = i18n("Hard Disk Drive");
		if (checkDiskStatus(DiskDeviceStatus::Hotpluggable)) {
			ret = i18n("Removable Storage");
		}
		if (isDiskOfType(DiskDeviceType::CompactFlash)) {
			ret = i18n("Compact Flash");
		}
		if (isDiskOfType(DiskDeviceType::MemoryStick)) {
			ret = i18n("Memory Stick");
		}
		if (isDiskOfType(DiskDeviceType::SmartMedia)) {
			ret = i18n("Smart Media");
		}
		if (isDiskOfType(DiskDeviceType::SDMMC)) {
			ret = i18n("Secure Digital");
		}
	}

	if (isDiskOfType(DiskDeviceType::RAM)) {
		ret = i18n("Random Access Memory");
	}
	if (isDiskOfType(DiskDeviceType::Loop)) {
		ret = i18n("Loop Device");
	}

	return ret;
}

TQPixmap StorageDevice::icon(TDEIcon::StdSizes size) {
	TQPixmap ret = DesktopIcon("hdd_unmount", size);

	if (isDiskOfType(DiskDeviceType::Floppy)) {
		ret = DesktopIcon("3floppy_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::Optical)) {
		ret = DesktopIcon("cdrom_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::CDROM)) {
		ret = DesktopIcon("cdrom_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::CDRW)) {
		ret = DesktopIcon("cdwriter_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::DVDROM)) {
		ret = DesktopIcon("dvd_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::DVDRW)) {
		ret = DesktopIcon("dvd_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::DVDRAM)) {
		ret = DesktopIcon("dvd_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::Zip)) {
		ret = DesktopIcon("zip_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::Tape)) {
		ret = DesktopIcon("tape_unmount", size);
	}
	if (isDiskOfType(DiskDeviceType::Camera)) {
		ret = DesktopIcon("camera_unmount");
	}

	if (isDiskOfType(DiskDeviceType::HDD)) {
		ret = DesktopIcon("hdd_unmount", size);
		if (checkDiskStatus(DiskDeviceStatus::Hotpluggable)) {
			ret = DesktopIcon("usbpendrive_unmount", size);
		}
		if (isDiskOfType(DiskDeviceType::CompactFlash)) {
			ret = DesktopIcon("compact_flash_unmount", size);
		}
		if (isDiskOfType(DiskDeviceType::MemoryStick)) {
			ret = DesktopIcon("memory_stick_unmount", size);
		}
		if (isDiskOfType(DiskDeviceType::SmartMedia)) {
			ret = DesktopIcon("smart_media_unmount", size);
		}
		if (isDiskOfType(DiskDeviceType::SDMMC)) {
			ret = DesktopIcon("sd_mmc_unmount", size);
		}
	}

	if (isDiskOfType(DiskDeviceType::RAM)) {
		ret = DesktopIcon("memory", size);
	}
	if (isDiskOfType(DiskDeviceType::Loop)) {
		ret = DesktopIcon("blockdevice", size);
	}

	return ret;
}

unsigned long long StorageDevice::deviceSize() {
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
		// Drat, I can't get a guaranteed block size.  Assume a block size of 512, as everything I have read indicates that /sys/block/<dev>/size is given in terms of a 512 byte block...
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

	return ((unsigned long long)blocksize.toULong()*(unsigned long long)devicesize.toULong());
}

TQString StorageDevice::deviceFriendlySize() {
	return HardwareDevices::bytesToFriendlySizeString(deviceSize());
}

TQString StorageDevice::mountPath() {
	// See if this device node is mounted
	// This requires parsing /proc/mounts, looking for deviceNode()

	// The Device Mapper throws a monkey wrench into this
	// It likes to advertise mounts as /dev/mapper/<something>,
	// where <something> is listed in <system path>/dm/name

	// First, ensure that all device information (mainly holders/slaves) is accurate
	HardwareDevices::instance()->rescanDeviceInformation(this);

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
			if ((testNode == deviceNode()) || (testNode == dmaltname) || (testNode == ("/dev/disk/by-uuid/" + diskUUID()))) {
				TQString ret = *mountInfo.at(1);
				ret.replace("\\040", " ");
				return ret;
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
		HardwareDevices *hwdevices = HardwareDevices::instance();
		GenericDevice *hwdevice = hwdevices->findBySystemPath(*slavedevit);
		if ((hwdevice) && (hwdevice->type() == GenericDeviceType::Disk)) {
			StorageDevice* sdevice = static_cast<StorageDevice*>(hwdevice);
			return sdevice->mountPath();
		}
	}

	return TQString::null;
}

TQString StorageDevice::mountDevice(TQString mediaName, StorageMountOptions mountOptions, TQString* errRet, int* retcode) {
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

	TQString optionString;
	if (mountOptions["ro"] == "true") {
		optionString.append(" -r");
	}
	
	if (mountOptions["atime"] != "true") {
		optionString.append(" -A");
	}
	
	if (mountOptions["utf8"] == "true") {
		optionString.append(" -c utf8");
	}
	
	if (mountOptions["sync"] == "true") {
		optionString.append(" -s");
	}

	if (mountOptions.contains("filesystem")) {
		optionString.append(TQString(" -t %1").arg(mountOptions["filesystem"]));
	}

	if (mountOptions.contains("locale")) {
		optionString.append(TQString(" -c %1").arg(mountOptions["locale"]));
	}

	TQString passFileName = passwordFile.name();
	TQString devNode = deviceNode();
	passFileName.replace("'", "'\\''");
	devNode.replace("'", "'\\''");
	mediaName.replace("'", "'\\''");
	TQString command = TQString("pmount -p '%1' %2 '%3' '%4' 2>&1").arg(passFileName).arg(optionString).arg(devNode).arg(mediaName);

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
	HardwareDevices::instance()->processModifiedMounts();

	ret = mountPath();

	return ret;
}

TQString StorageDevice::mountEncryptedDevice(TQString passphrase, TQString mediaName, StorageMountOptions mountOptions, TQString* errRet, int* retcode) {
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

	TQString optionString;
	if (mountOptions["ro"] == "true") {
		optionString.append(" -r");
	}
	
	if (mountOptions["atime"] != "true") {
		optionString.append(" -A");
	}
	
	if (mountOptions["utf8"] == "true") {
		optionString.append(" -c utf8");
	}
	
	if (mountOptions["sync"] == "true") {
		optionString.append(" -s");
	}

	if (mountOptions.contains("filesystem")) {
		optionString.append(TQString(" -t %1").arg(mountOptions["filesystem"]));
	}

	if (mountOptions.contains("locale")) {
		optionString.append(TQString(" -c %1").arg(mountOptions["locale"]));
	}

	TQString passFileName = passwordFile.name();
	TQString devNode = deviceNode();
	passFileName.replace("'", "'\\''");
	devNode.replace("'", "'\\''");
	mediaName.replace("'", "'\\''");
	TQString command = TQString("pmount -p '%1' %2 '%3' '%4' 2>&1").arg(passFileName).arg(optionString).arg(devNode).arg(mediaName);

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
	HardwareDevices::instance()->processModifiedMounts();

	ret = mountPath();

	return ret;
}

bool StorageDevice::unmountDevice(TQString* errRet, int* retcode) {
	int internal_retcode;
	if (!retcode) {
		retcode = &internal_retcode;
	}

	TQString mountpoint = mountPath();

	if (mountpoint.isNull()) {
		return true;
	}

	mountpoint.replace("'", "'\\''");
	TQString command = TQString("pumount '%1' 2>&1").arg(mountpoint);
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
	HardwareDevices::instance()->processModifiedMounts();

	return false;
}

TQString StorageDevice::determineFileSystemType(TQString path) {
	TQStringList mountTable;
	TQString prevPath = path;
	dev_t prevDev = 0;
	int pos;
	struct stat directory_info;
	if (path.startsWith("/")) {
		stat(path.ascii(), &directory_info);
		prevDev = directory_info.st_dev;
		// Walk the directory tree up to the root, checking for any change in st_dev
		// If a change is found, the previous value of path is the mount point itself
		while (path != "/") {
			pos = path.findRev("/", -1, TRUE);
			if (pos < 0) {
				break;
			}
			path = path.mid(0, pos);
			if (path == "") {
				path = "/";
			}
			stat(path.ascii(), &directory_info);
			if (directory_info.st_dev != prevDev) {
				break;
			}
			prevPath = path;
			prevDev = directory_info.st_dev;
		}
	}

	// Read in mount table
	mountTable.clear();
	TQFile file( "/proc/mounts" );
	if ( file.open( IO_ReadOnly ) ) {
		TQTextStream stream( &file );
		while ( !stream.atEnd() ) {
			mountTable.append(stream.readLine());
		}
		file.close();
	}

	// Parse mount table
	TQStringList::Iterator it;
	for ( it = mountTable.begin(); it != mountTable.end(); ++it ) {
		TQStringList mountInfo = TQStringList::split(" ", (*it), true);
		if ((*mountInfo.at(1)) == prevPath) {
			return (*mountInfo.at(2));
		}
	}

	// Unknown file system type
	return TQString::null;
}

#include "storagedevice.moc"
