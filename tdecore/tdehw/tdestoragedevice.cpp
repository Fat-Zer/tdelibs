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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

#include <tqregexp.h>
#include <tqpixmap.h>
#include <tqfile.h>

#include "kdebug.h"
#include "tdelocale.h"
#include "tdeglobal.h"
#include "kiconloader.h"
#include "tdetempfile.h"
#include "kstandarddirs.h"

#include "tdehardwaredevices.h"

#include "config.h"

#if defined(WITH_CRYPTSETUP)
	#ifdef CRYPTSETUP_OLD_API
		#define class cryptsetup_class
		#define CRYPT_SLOT_INVALID INVALID
		#define CRYPT_SLOT_INACTIVE INACTIVE
		#define CRYPT_SLOT_ACTIVE ACTIVE
		#define CRYPT_SLOT_BUSY BUSY
		#define CRYPT_SLOT_ACTIVE_LAST ACTIVE
		#include <libcryptsetup.h>
		#undef class
	#else
		#include <libcryptsetup.h>
	#endif
#endif

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

#if defined(WITH_UDISKS) || defined(WITH_UDISKS2)
	// Defined in tdehardwaredevices.cpp
	TQT_DBusData convertDBUSDataToVariantData(TQT_DBusData);
#endif // defined(WITH_UDISKS) || defined(WITH_UDISKS2)

#if defined(WITH_CRYPTSETUP)
int TDEStorageDevice::cryptsetup_password_entry_callback(const char *msg, char *buf, size_t length, void *usrptr) {
	TDEStorageDevice* sdevice = (TDEStorageDevice*)usrptr;

	unsigned int passwd_len = sdevice->m_cryptDevicePassword.size();
	if (passwd_len < length) {
		memcpy(buf, sdevice->m_cryptDevicePassword.data(), length);
		return passwd_len;
	}
	else {
		return -1;
	}
}
#endif

TDEStorageDevice::TDEStorageDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn), m_mediaInserted(true), m_cryptDevice(NULL) {
	m_diskType = TDEDiskDeviceType::Null;
	m_diskStatus = TDEDiskDeviceStatus::Null;
}

TDEStorageDevice::~TDEStorageDevice() {
#if defined(WITH_CRYPTSETUP)
	if (m_cryptDevice) {
		crypt_free(m_cryptDevice);
		m_cryptDevice = NULL;
	}
#endif
}

TDEDiskDeviceType::TDEDiskDeviceType TDEStorageDevice::diskType() {
	return m_diskType;
}

void TDEStorageDevice::internalGetLUKSKeySlotStatus() {
#if defined(WITH_CRYPTSETUP)
	unsigned int i;
	crypt_keyslot_info keyslot_status;
	TDELUKSKeySlotStatus::TDELUKSKeySlotStatus tde_keyslot_status;

	m_cryptKeyslotStatus.clear();
	for (i = 0; i < m_cryptKeySlotCount; i++) {
		keyslot_status = crypt_keyslot_status(m_cryptDevice, i);
		tde_keyslot_status = TDELUKSKeySlotStatus::Invalid;
		if (keyslot_status == CRYPT_SLOT_INACTIVE) {
			tde_keyslot_status = TDELUKSKeySlotStatus::Inactive;
		}
		else if (keyslot_status == CRYPT_SLOT_ACTIVE) {
			tde_keyslot_status = TDELUKSKeySlotStatus::Active;
		}
		else if (keyslot_status == CRYPT_SLOT_ACTIVE_LAST) {
			tde_keyslot_status = TDELUKSKeySlotStatus::Active | TDELUKSKeySlotStatus::Last;
		}
		m_cryptKeyslotStatus.append(tde_keyslot_status);
	}
#endif
}

void TDEStorageDevice::internalInitializeLUKSIfNeeded() {
#if defined(WITH_CRYPTSETUP)
	int ret;

	if (m_diskType & TDEDiskDeviceType::LUKS) {
		if (!m_cryptDevice) {
			TQString node = deviceNode();
			if (node != "") {
				ret = crypt_init(&m_cryptDevice, node.ascii());
				if (ret == 0) {
					crypt_set_password_callback(m_cryptDevice, TDEStorageDevice::cryptsetup_password_entry_callback, this);
					ret = crypt_load(m_cryptDevice, NULL, NULL);
					if (ret == 0) {
						int keyslot_count;
#ifdef CRYPTSETUP_OLD_API
						kdWarning() << "TDEStorageDevice: The version of libcryptsetup that TDE was compiled against was too old!  Most LUKS features will not function" << endl;
						m_cryptDeviceType = TQString::null;
						keyslot_count = 0;
#else
						m_cryptDeviceType = crypt_get_type(m_cryptDevice);
						keyslot_count = crypt_keyslot_max(m_cryptDeviceType.ascii());
#endif
						if (keyslot_count < 0) {
							m_cryptKeySlotCount = 0;
						}
						else {
							m_cryptKeySlotCount = keyslot_count;
						}
						internalGetLUKSKeySlotStatus();
					}
				}
				else {
					m_cryptDevice = NULL;
				}
			}
		}
	}
	else {
		if (m_cryptDevice) {
			crypt_free(m_cryptDevice);
			m_cryptDevice = NULL;
		}
	}
#endif
}

void TDEStorageDevice::cryptSetOperationsUnlockPassword(TQByteArray password) {
	m_cryptDevicePassword = password;
}

void TDEStorageDevice::cryptClearOperationsUnlockPassword() {
	m_cryptDevicePassword.resize(0);
}

bool TDEStorageDevice::cryptOperationsUnlockPasswordSet() {
	if (m_cryptDevicePassword.size() > 0) {
		return true;
	}
	else {
		return false;
	}
}

TDELUKSResult::TDELUKSResult TDEStorageDevice::cryptAddKey(unsigned int keyslot, TQByteArray password) {
#if defined(WITH_CRYPTSETUP)
	int ret;

	if (m_cryptDevice) {
		if (keyslot < m_cryptKeySlotCount) {
			ret = crypt_keyslot_add_by_passphrase(m_cryptDevice, keyslot, m_cryptDevicePassword.data(), m_cryptDevicePassword.size(), password.data(), password.size());
			if (ret < 0) {
				return TDELUKSResult::KeyslotOpFailed;
			}
			else {
				internalGetLUKSKeySlotStatus();
				return TDELUKSResult::Success;
			}
		}
		else {
			return TDELUKSResult::InvalidKeyslot;
		}
	}
	else {
		return TDELUKSResult::LUKSNotFound;
	}
#else
	return TDELUKSResult::LUKSNotSupported;
#endif
}

TDELUKSResult::TDELUKSResult TDEStorageDevice::cryptDelKey(unsigned int keyslot) {
#if defined(WITH_CRYPTSETUP)
	int ret;

	if (m_cryptDevice) {
		if (keyslot < m_cryptKeySlotCount) {
			ret = crypt_keyslot_destroy(m_cryptDevice, keyslot);
			if (ret < 0) {
				return TDELUKSResult::KeyslotOpFailed;
			}
			else {
				internalGetLUKSKeySlotStatus();
				return TDELUKSResult::Success;
			}
		}
		else {
			return TDELUKSResult::InvalidKeyslot;
		}
	}
	else {
		return TDELUKSResult::LUKSNotFound;
	}
#else
	return TDELUKSResult::LUKSNotSupported;
#endif
}

unsigned int TDEStorageDevice::cryptKeySlotCount() {
	return m_cryptKeySlotCount;
}

TDELUKSKeySlotStatusList TDEStorageDevice::cryptKeySlotStatus() {
	return m_cryptKeyslotStatus;
}

TQString TDEStorageDevice::cryptKeySlotFriendlyName(TDELUKSKeySlotStatus::TDELUKSKeySlotStatus status) {
	if (status & TDELUKSKeySlotStatus::Inactive) {
		return i18n("Inactive");
	}
	else if (status & TDELUKSKeySlotStatus::Active) {
		return i18n("Active");
	}
	else {
		return i18n("Unknown");
	}
}

void TDEStorageDevice::internalSetDeviceNode(TQString sn) {
	TDEGenericDevice::internalSetDeviceNode(sn);
	internalInitializeLUKSIfNeeded();
}

void TDEStorageDevice::internalSetDiskType(TDEDiskDeviceType::TDEDiskDeviceType dt) {
	m_diskType = dt;
	internalInitializeLUKSIfNeeded();
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
				printf("[ERROR][tdehwlib] ejectDriveUDisks: %s\n", error.name().ascii()); fflush(stdout);
				return FALSE;
			}
			else {
				return TRUE;
			}
		}
	}
#endif // WITH_UDISKS
	return FALSE;
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
				printf("[ERROR][tdehwlib] ejectDriveUDisks2: %s\n", error.name().ascii()); fflush(stdout);
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
						printf("[ERROR][tdehwlib] ejectDriveUDisks2: %s\n", error.name().ascii()); fflush(stdout);
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
							printf("[ERROR][tdehwlib] ejectDriveUDisks2: %s\n", error.name().ascii()); fflush(stdout);
							return FALSE;
						}
						else {
							return TRUE;
						}
					}
				}
			}
		}
	}
#endif // WITH_UDISKS2
	return FALSE;
}

int mountDriveUDisks(TQString deviceNode, TQString fileSystemType, TQStringList mountOptions, TQString* errStr = NULL) {
#ifdef WITH_UDISKS
	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString = "/org/freedesktop/UDisks/devices/" + blockDeviceString;

		// Mount the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks", blockDeviceString, "org.freedesktop.UDisks.Device", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			params << TQT_DBusData::fromString(fileSystemType);
			params << TQT_DBusData::fromList(TQT_DBusDataList(mountOptions));
			TQT_DBusMessage reply = driveControl.sendWithReply("FilesystemMount", params, &error);
			if (error.isValid()) {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					// Service not installed or unavailable
					return -2;
				}
				if (errStr) {
					*errStr = error.name() + ": " + error.message();
				}
				else {
					printf("[ERROR][tdehwlib] mountDriveUDisks: %s\n", error.name().ascii()); fflush(stdout);
				}
				return -1;
			}
			else {
				return 0;
			}
		}
		else {
			return -2;
		}
	}
#endif // WITH_UDISKS
	return -2;
}

int mountDriveUDisks2(TQString deviceNode, TQString fileSystemType, TQString mountOptions, TQString* errStr = NULL) {
#ifdef WITH_UDISKS2
	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString = "/org/freedesktop/UDisks2/block_devices/" + blockDeviceString;

		// Mount the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks2", blockDeviceString, "org.freedesktop.UDisks2.Filesystem", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			TQMap<TQString, TQT_DBusData> optionsMap;
			if (fileSystemType != "") {
				optionsMap["fstype"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(fileSystemType));
			}
			optionsMap["options"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(mountOptions));
			params << TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(optionsMap));
			TQT_DBusMessage reply = driveControl.sendWithReply("Mount", params, &error);
			if (error.isValid()) {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					// Service not installed or unavailable
					return -2;
				}
				if (errStr) {
					*errStr = error.name() + ": " + error.message();
				}
				else {
					printf("[ERROR][tdehwlib] mountDriveUDisks2: %s\n", error.name().ascii()); fflush(stdout);
				}
				return -1;
			}
			else {
				return 0;
			}
		}
		else {
			return -2;
		}
	}
#endif // WITH_UDISKS2
	return -2;
}

int unMountDriveUDisks(TQString deviceNode, TQStringList unMountOptions, TQString* errStr = NULL) {
#ifdef WITH_UDISKS
	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString = "/org/freedesktop/UDisks/devices/" + blockDeviceString;

		// Mount the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks", blockDeviceString, "org.freedesktop.UDisks.Device", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			params << TQT_DBusData::fromList(TQT_DBusDataList(unMountOptions));
			TQT_DBusMessage reply = driveControl.sendWithReply("FilesystemUnmount", params, &error);
			if (error.isValid()) {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					// Service not installed or unavailable
					return -2;
				}
				if (errStr) {
					*errStr = error.name() + ": " + error.message();
				}
				else {
					printf("[ERROR][tdehwlib] unMountDriveUDisks: %s\n", error.name().ascii()); fflush(stdout);
				}
				return -1;
			}
			else {
				return 0;
			}
		}
		else {
			return -2;
		}
	}
#endif // WITH_UDISKS
	return -2;
}

int unMountDriveUDisks2(TQString deviceNode, TQString unMountOptions, TQString* errStr = NULL) {
#ifdef WITH_UDISKS2
	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString = "/org/freedesktop/UDisks2/block_devices/" + blockDeviceString;

		// Mount the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks2", blockDeviceString, "org.freedesktop.UDisks2.Filesystem", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			TQMap<TQString, TQT_DBusData> optionsMap;
			optionsMap["options"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(unMountOptions));
			params << TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(optionsMap));
			TQT_DBusMessage reply = driveControl.sendWithReply("Unmount", params, &error);
			if (error.isValid()) {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					// Service not installed or unavailable
					return -2;
				}
				if (errStr) {
					*errStr = error.name() + ": " + error.message();
				}
				else {
					printf("[ERROR][tdehwlib] unMountDriveUDisks2: %s\n", error.name().ascii()); fflush(stdout);
				}
				return -1;
			}
			else {
				return 0;
			}
		}
		else {
			return -2;
		}
	}
#endif // WITH_UDISKS2
	return -2;
}

bool TDEStorageDevice::ejectDrive() {
#ifdef WITH_UDISKS2
	if (!(TDEGlobal::dirs()->findExe("udisksctl").isEmpty())) {
		if (ejectDriveUDisks2(this)) {
			return TRUE;
		}
		else {
			printf("[tdehwlib] Failed to eject drive '%s' via udisks2, falling back to alternate mechanism\n", deviceNode().ascii());
			fflush(stdout);
		}
	}
#endif // WITH_UDISKS2

#ifdef WITH_UDISKS
	if (!(TDEGlobal::dirs()->findExe("udisks").isEmpty())) {
		if (ejectDriveUDisks(this)) {
			return TRUE;
		}
		else {
			printf("[tdehwlib] Failed to eject drive '%s' via udisks, falling back to alternate mechanism\n", deviceNode().ascii());
			fflush(stdout);
		}
	}
#endif // WITH_UDISKS

	if (!(TDEGlobal::dirs()->findExe("eject").isEmpty())) {
		TQString command = TQString("eject -v '%1' 2>&1").arg(deviceNode());

		FILE *exepipe = popen(command.ascii(), "r");
		if (exepipe) {
			TQString eject_output;
			TQTextStream ts(exepipe, IO_ReadOnly);
			eject_output = ts.read();
			int retcode = pclose(exepipe);
			if (retcode == 0) {
				return TRUE;
			}
		}
		printf("[tdehwlib] Failed to eject drive '%s' via 'eject' command\n", deviceNode().ascii());
		fflush(stdout);
	}

	return FALSE;
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

TQString decodeHexEncoding(TQString str) {
	TQRegExp hexEncRegExp("\\\\x[0-9A-Fa-f]{1,2}");
	hexEncRegExp.setMinimal(false);
	hexEncRegExp.setCaseSensitive(true);
	int s = -1;

	while((s = hexEncRegExp.search(str, s+1))>=0){
		str.replace(s, hexEncRegExp.cap(0).length(), TQChar((char)strtol(hexEncRegExp.cap(0).mid(2).ascii(), NULL, 16)));
	}

	return str;
}

TQString TDEStorageDevice::friendlyName() {
	// Return the actual storage device name
	TQString devicevendorid = vendorEncoded();
	TQString devicemodelid = modelEncoded();

	devicevendorid = decodeHexEncoding(devicevendorid);
	devicemodelid = decodeHexEncoding(devicemodelid);

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

TQString TDEStorageDevice::detailedFriendlyName() {
	return TQString("%1 [%2]").arg(friendlyName()).arg(deviceNode());
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
	TQPixmap ret = DesktopIcon("drive-harddisk", size);

	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		ret = DesktopIcon("media-floppy-3_5", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Optical)) {
		ret = DesktopIcon("media-optical-cdrom", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::CDROM)) {
		ret = DesktopIcon("media-optical-cdrom", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::CDRW)) {
		ret = DesktopIcon("media-optical-cdwriter", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDROM)) {
		ret = DesktopIcon("media-optical-dvd", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRW)) {
		ret = DesktopIcon("media-optical-dvd", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRAM)) {
		ret = DesktopIcon("media-optical-dvd", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Zip)) {
		ret = DesktopIcon("media-floppy-zip", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Tape)) {
		ret = DesktopIcon("media-tape", size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Camera)) {
		ret = DesktopIcon("camera_unmount", size);
	}

	if (isDiskOfType(TDEDiskDeviceType::HDD)) {
		ret = DesktopIcon("drive-harddisk", size);
		if (checkDiskStatus(TDEDiskDeviceStatus::Hotpluggable)) {
			ret = DesktopIcon("media-flash-usb", size);
		}
		if (isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
			ret = DesktopIcon("media-flash-compact_flash", size);
		}
		if (isDiskOfType(TDEDiskDeviceType::MemoryStick)) {
			ret = DesktopIcon("media-flash-memory_stick", size);
		}
		if (isDiskOfType(TDEDiskDeviceType::SmartMedia)) {
			ret = DesktopIcon("media-flash-smart_media", size);
		}
		if (isDiskOfType(TDEDiskDeviceType::SDMMC)) {
			ret = DesktopIcon("media-flash-sd_mmc", size);
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
	// While at first glance it would seem that checking /queue/physical_block_size would be needed to get an accurate device size, in reality Linux
	// appears to only ever report the device size in 512 byte units.  This does not appear to be documented anywhere!
	TQString blocksize = "512";

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

TQString TDEStorageDevice::mountDevice(TQString mediaName, TDEStorageMountOptions mountOptions, TQString* errRet, int* retcode) {
	int internal_retcode;
	if (!retcode) {
		retcode = &internal_retcode;
	}

	TQString ret = mountPath();

	// Device is already mounted
	if (!ret.isNull()) {
		return ret;
	}

	TQString command;
	TQString devNode = deviceNode();
	devNode.replace("'", "'\\''");
	mediaName.replace("'", "'\\''");

#if defined(WITH_UDISKS2) || defined(WITH_UDISKS)
	// Prepare filesystem options for mount
	TQStringList udisksOptions;
	TQString optionString;

	if (mountOptions["ro"] == "true") {
		udisksOptions.append("ro");
	}

	if (mountOptions["atime"] != "true") {
		udisksOptions.append("noatime");
	}

	if (mountOptions["sync"] == "true") {
		udisksOptions.append("sync");
	}

	if(  (mountOptions["filesystem"] == "fat")
	  || (mountOptions["filesystem"] == "vfat")
	  || (mountOptions["filesystem"] == "msdos")
	  || (mountOptions["filesystem"] == "umsdos")
	) {
		if (mountOptions.contains("shortname")) {
			udisksOptions.append(TQString("shortname=%1").arg(mountOptions["shortname"]));
		}
	}

	if( (mountOptions["filesystem"] == "jfs")) {
		if (mountOptions["utf8"] == "true") {
			// udisks/udisks2 for now does not support option iocharset= for jfs
			// udisksOptions.append("iocharset=utf8");
		}
	}

	if( (mountOptions["filesystem"] == "ntfs-3g") ) {
		if (mountOptions.contains("locale")) {
			udisksOptions.append(TQString("locale=%1").arg(mountOptions["locale"]));
		}
	}

	if(  (mountOptions["filesystem"] == "ext3")
	  || (mountOptions["filesystem"] == "ext4")
	) {
		if (mountOptions.contains("journaling")) {
			// udisks/udisks2 for now does not support option data= for ext3/ext4
			// udisksOptions.append(TQString("data=%1").arg(mountOptions["journaling"]));
		}
	}

	for (TQStringList::Iterator it = udisksOptions.begin(); it != udisksOptions.end(); ++it) {
		optionString.append(",");
		optionString.append(*it);
	}

	if (!optionString.isEmpty()) {
		optionString.remove(0, 1);
	}
#endif // defined(WITH_UDISKS2) || defined(WITH_UDISKS)

#ifdef WITH_UDISKS2
	if(command.isEmpty()) {
		// Try to use UDISKS v2 via DBUS, if available
		TQString errorString;
		TQString fileSystemType;

		if (mountOptions.contains("filesystem") && !mountOptions["filesystem"].isEmpty()) {
			fileSystemType = mountOptions["filesystem"];
		}

		int uDisks2Ret = mountDriveUDisks2(devNode, fileSystemType, optionString, &errorString);
		if (uDisks2Ret == 0) {
			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			ret = mountPath();
			return ret;
		}
		else if (uDisks2Ret == -1) {
			if (errRet) {
				*errRet = errorString;
			}

			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			ret = mountPath();
			return ret;
		}
		else {
			// The UDISKS v2 DBUS service was either not available or was unusable; try another method...
			command = TQString::null;
		}
	}
#endif // WITH_UDISKS2

#ifdef WITH_UDISKS
	if(command.isEmpty()) {
		// Try to use UDISKS v1 via DBUS, if available
		TQString errorString;
		TQString fileSystemType;

		if (mountOptions.contains("filesystem") && !mountOptions["filesystem"].isEmpty()) {
			fileSystemType = mountOptions["filesystem"];
		}

		int uDisksRet = mountDriveUDisks(devNode, fileSystemType, udisksOptions, &errorString);
		if (uDisksRet == 0) {
			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			ret = mountPath();
			return ret;
		}
		else if (uDisksRet == -1) {
			if (errRet) {
				*errRet = errorString;
			}

			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			ret = mountPath();
			return ret;
		}
		else {
			// The UDISKS v1 DBUS service was either not available or was unusable; try another method...
			command = TQString::null;
		}
	}
#endif // WITH_UDISKS

	if(command.isEmpty()) {
		// Use 'pmount' command, if available
		TQString pmountProg = TDEGlobal::dirs()->findExe("pmount");
		if (!pmountProg.isEmpty()) {
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

			if (mountOptions.contains("filesystem") && !mountOptions["filesystem"].isEmpty()) {
				optionString.append(TQString(" -t %1").arg(mountOptions["filesystem"]));
			}

			if (mountOptions.contains("locale")) {
				optionString.append(TQString(" -c %1").arg(mountOptions["locale"]));
			}

			TQString passFileName = passwordFile.name();
			passFileName.replace("'", "'\\''");

			command = TQString("pmount -p '%1' %2 '%3' '%4' 2>&1").arg(passFileName).arg(optionString).arg(devNode).arg(mediaName);
		}
	}

	if(command.isEmpty()) {
		if (errRet) {
			*errRet = i18n("No supported mounting methods were detected on your system");
		}
		return ret;
	}

	FILE *exepipe = popen(command.local8Bit(), "r");
	if (exepipe) {
		TQString mount_output;
		TQTextStream* ts = new TQTextStream(exepipe, IO_ReadOnly);
		mount_output = ts->read();
		delete ts;
		*retcode = pclose(exepipe);
		if (errRet) {
			*errRet = mount_output;
		}
	}

	// Update internal mount data
	TDEGlobal::hardwareDevices()->processModifiedMounts();

	ret = mountPath();

	return ret;
}

TQString TDEStorageDevice::mountEncryptedDevice(TQString passphrase, TQString mediaName, TDEStorageMountOptions mountOptions, TQString* errRet, int* retcode) {
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

	if (mountOptions.contains("filesystem") && !mountOptions["filesystem"].isEmpty()) {
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

	FILE *exepipe = popen(command.local8Bit(), "r");
	if (exepipe) {
		TQString mount_output;
		TQTextStream* ts = new TQTextStream(exepipe, IO_ReadOnly);
		mount_output = ts->read();
		delete ts;
		*retcode = pclose(exepipe);
		if (errRet) {
			*errRet = mount_output;
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
	TQString devNode = deviceNode();

	if (mountpoint.isNull()) {
		return true;
	}

	mountpoint.replace("'", "'\\''");

	TQString command;

#ifdef WITH_UDISKS2
	if(command.isEmpty()) {
		// Try to use UDISKS v2 via DBUS, if available
		TQString errorString;
		int unMountUDisks2Ret = unMountDriveUDisks2(devNode, TQString::null, &errorString);
		if (unMountUDisks2Ret == 0) {
			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			return true;
		}
		else if (unMountUDisks2Ret == -1) {
			if (errRet) {
				*errRet = errorString;
			}

			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			return false;
		}
		else {
			// The UDISKS v2 DBUS service was either not available or was unusable; try another method...
			command = TQString::null;
		}
	}
#endif // WITH_UDISKS2
#ifdef WITH_UDISKS
	if(command.isEmpty()) {
		// Try to use UDISKS v1 via DBUS, if available
		TQString errorString;
		int unMountUDisksRet = unMountDriveUDisks(devNode, TQStringList(), &errorString);
		if (unMountUDisksRet == 0) {
			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			return true;
		}
		else if (unMountUDisksRet == -1) {
			if (errRet) {
				*errRet = errorString;
			}

			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			return false;
		}
		else {
			// The UDISKS v1 DBUS service was either not available or was unusable; try another method...
			command = TQString::null;
		}
	}
#endif // WITH_UDISKS
	if(command.isEmpty() &&
	   !(TDEGlobal::dirs()->findExe("pumount").isEmpty())) {
		command = TQString("pumount '%1' 2>&1").arg(mountpoint);
	}

	if(command.isEmpty()) {
		if (errRet) {
			*errRet = i18n("No supported unmounting methods were detected on your system");
		}
		return true;
	}

	FILE *exepipe = popen(command.local8Bit(), "r");
	if (exepipe) {
		TQString umount_output;
		TQTextStream* ts = new TQTextStream(exepipe, IO_ReadOnly);
		umount_output = ts->read();
		delete ts;
		*retcode = pclose(exepipe);
		if (*retcode == 0) {
			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();

			return true;
		}
		else {
			if (errRet) {
				*errRet = umount_output;
			}
		}
	}

	// Update internal mount data
	TDEGlobal::hardwareDevices()->processModifiedMounts();

	return false;
}

TQString TDEStorageDevice::determineFileSystemType(TQString path) {
	TQStringList mountTable;
	TQString prevPath = path;
	dev_t prevDev = 0;
	int pos;
	struct stat directory_info;
	if (path.startsWith("/")) {
		stat(path.local8Bit(), &directory_info);
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
			stat(path.local8Bit(), &directory_info);
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

#include "tdestoragedevice.moc"
