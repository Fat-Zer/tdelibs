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

#include "tdestoragedevice.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

#include <tqpixmap.h>
#include <tqfile.h>

#include "tdelocale.h" 
#include "tdeglobal.h" 
#include "kiconloader.h" 
#include "tdetempfile.h" 

#include "tdehardwaredevices.h" 

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

bool TDEStorageDevice::lockDriveMedia(bool lock) {
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

bool ejectDriveUDisks(TDEStorageDevice* sdevice) {
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

bool ejectDriveUDisks2(TDEStorageDevice* sdevice) {
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

bool TDEStorageDevice::ejectDrive() {
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

bool TDEStorageDevice::ejectDriveMedia() {
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

	if (isDiskOfType(TDEDiskDeviceType::Camera)) {
		return TDEGenericDevice::friendlyName();
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

	// Keep this in sync with TDEStorageDevice::icon(TDEIcon::StdSizes size) below
	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		ret = i18n("Floppy Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Optical)) {
		ret = i18n("Optical Drive");
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
	if (isDiskOfType(TDEDiskDeviceType::DVDRW)) {
		ret = i18n("DVDRW Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRAM)) {
		ret = i18n("DVDRAM Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Zip)) {
		ret = i18n("Zip Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Tape)) {
		ret = i18n("Tape Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Camera)) {
		ret = i18n("Digital Camera");
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
		if (isDiskOfType(TDEDiskDeviceType::SmartMedia)) {
			ret = i18n("Smart Media");
		}
		if (isDiskOfType(TDEDiskDeviceType::SDMMC)) {
			ret = i18n("Secure Digital");
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

TQPixmap TDEStorageDevice::icon(TDEIcon::StdSizes size) {
	TQPixmap ret = DesktopIcon("hdd_unmount", size);

	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		ret = DesktopIcon("3floppy_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Optical)) {
		ret = DesktopIcon("cdrom_unmount", size);
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
	if (isDiskOfType(TDEDiskDeviceType::Tape)) {
		ret = DesktopIcon("tape_unmount", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Camera)) {
		ret = DesktopIcon("camera_unmount");
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
		if (isDiskOfType(TDEDiskDeviceType::SmartMedia)) {
			ret = DesktopIcon("smart_media_unmount", size);
		}
		if (isDiskOfType(TDEDiskDeviceType::SDMMC)) {
			ret = DesktopIcon("sd_mmc_unmount", size);
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

unsigned long long TDEStorageDevice::deviceSize() {
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
	TDEGlobal::hardwareDevices()->rescanDeviceInformation(this);

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
		TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
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

	TQString passFileName = passwordFile.name();
	TQString devNode = deviceNode();
	passFileName.replace("'", "'\\''");
	devNode.replace("'", "'\\''");
	mediaName.replace("'", "'\\''");
	TQString command = TQString("pmount -p '%1' %2 '%3' '%4' 2>&1").arg(passFileName).arg(mountOptions).arg(devNode).arg(mediaName);

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
	TDEGlobal::hardwareDevices()->processModifiedMounts();

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

	TQString passFileName = passwordFile.name();
	TQString devNode = deviceNode();
	passFileName.replace("'", "'\\''");
	devNode.replace("'", "'\\''");
	mediaName.replace("'", "'\\''");
	TQString command = TQString("pmount -p '%1' %2 '%3' '%4' 2>&1").arg(passFileName).arg(mountOptions).arg(devNode).arg(mediaName);

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
	TDEGlobal::hardwareDevices()->processModifiedMounts();

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
	TDEGlobal::hardwareDevices()->processModifiedMounts();

	return false;
}

#include "tdestoragedevice.moc"