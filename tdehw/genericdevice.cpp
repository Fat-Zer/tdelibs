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

#include "genericdevice.h"

#include <tqpixmap.h>

#include <tdelocale.h> 

#include "hardwaredevices.h" 

#include "config.h"

using namespace TDEHW;

GenericDevice::GenericDevice(GenericDeviceType::GenericDeviceType dt, TQString dn) : TQObject() {
	m_deviceType = dt;
	m_deviceName = dn;

	m_parentDevice = 0;
	m_friendlyName = TQString::null;
	m_blacklistedForUpdate = false;
}

GenericDevice::~GenericDevice() {
}

GenericDeviceType::GenericDeviceType GenericDevice::type() {
	return m_deviceType;
}

TQString GenericDevice::name() {
	return m_deviceName;
}

void GenericDevice::internalSetName(TQString dn) {
	m_deviceName = dn;
}

TQString GenericDevice::vendorName() {
	return m_vendorName;
}

void GenericDevice::internalSetVendorName(TQString vn) {
	m_vendorName = vn;
}

TQString GenericDevice::vendorModel() {
	return m_vendorModel;
}

void GenericDevice::internalSetVendorModel(TQString vm) {
	m_vendorModel = vm;
}

TQString GenericDevice::serialNumber() {
	return m_serialNumber;
}

void GenericDevice::internalSetSerialNumber(TQString sn) {
	m_serialNumber = sn;
}

TQString GenericDevice::systemPath() {
	if (!m_systemPath.endsWith("/")) {
		m_systemPath += "/";
	}
	return m_systemPath;
}

void GenericDevice::internalSetSystemPath(TQString sp) {
	m_systemPath = sp;
}

TQString GenericDevice::deviceNode() {
	return m_deviceNode;
}

void GenericDevice::internalSetDeviceNode(TQString sn) {
	m_deviceNode = sn;
}

TQString GenericDevice::deviceBus() {
	return m_deviceBus;
}

void GenericDevice::internalSetDeviceBus(TQString db) {
	m_deviceBus = db;
}

TQString GenericDevice::uniqueID() {
	m_uniqueID = m_systemPath+m_deviceNode;
	return m_uniqueID;
}

TQString GenericDevice::vendorID() {
	return m_vendorID;
}

void GenericDevice::internalSetVendorID(TQString id) {
	m_vendorID = id;
	m_vendorID.replace("0x", "");
}

TQString GenericDevice::modelID() {
	return m_modelID;
}

void GenericDevice::internalSetModelID(TQString id) {
	m_modelID = id;
	m_modelID.replace("0x", "");
}

TQString GenericDevice::vendorEncoded() {
	return m_vendorenc;
}

void GenericDevice::internalSetVendorEncoded(TQString id) {
	m_vendorenc = id;
}

TQString GenericDevice::modelEncoded() {
	return m_modelenc;
}

void GenericDevice::internalSetModelEncoded(TQString id) {
	m_modelenc = id;
}

TQString GenericDevice::subVendorID() {
	return m_subvendorID;
}

void GenericDevice::internalSetSubVendorID(TQString id) {
	m_subvendorID = id;
	m_subvendorID.replace("0x", "");
}

TQString GenericDevice::PCIClass() {
	return m_pciClass;
}

void GenericDevice::internalSetPCIClass(TQString cl) {
	m_pciClass = cl;
	m_pciClass.replace("0x", "");
}

TQString GenericDevice::moduleAlias() {
	return m_modAlias;
}

void GenericDevice::internalSetModuleAlias(TQString ma) {
	m_modAlias = ma;
}

TQString GenericDevice::deviceDriver() {
	return m_deviceDriver;
}

void GenericDevice::internalSetDeviceDriver(TQString dr) {
	m_deviceDriver = dr;
}

TQString GenericDevice::subsystem() {
	return m_subsystem;
}

void GenericDevice::internalSetSubsystem(TQString ss) {
	m_subsystem = ss;
}

TQString GenericDevice::subModelID() {
	return m_submodelID;
}

void GenericDevice::internalSetSubModelID(TQString id) {
	m_submodelID = id;
	m_submodelID.replace("0x", "");
}

void GenericDevice::internalSetParentDevice(GenericDevice* pd) {
	m_parentDevice = pd;
}

GenericDevice* GenericDevice::parentDevice() {
	return m_parentDevice;
}

TQPixmap GenericDevice::icon(TDEIcon::StdSizes size) {
	return HardwareDevices::instance()->getDeviceTypeIconFromType(type(), size);
}

bool GenericDevice::blacklistedForUpdate() {
	return m_blacklistedForUpdate;
}

void GenericDevice::internalSetBlacklistedForUpdate(bool bl) {
	m_blacklistedForUpdate = bl;
}

TQString GenericDevice::friendlyDeviceType() {
	return HardwareDevices::instance()->getFriendlyDeviceTypeStringFromType(type());
}

TQString GenericDevice::busID() {
	TQString busid = m_systemPath;
	busid = busid.remove(0, busid.findRev("/")+1);
	busid = busid.remove(0, busid.find(":")+1);
	return busid;
}

TQString GenericDevice::friendlyName() {
	if (m_friendlyName.isNull()) {
		if (type() == GenericDeviceType::RootSystem) {
			m_friendlyName = "Linux System";
		}
		else if (type() == GenericDeviceType::Root) {
			TQString friendlyDriverName = systemPath();
			friendlyDriverName.truncate(friendlyDriverName.length()-1);
			friendlyDriverName.remove(0, friendlyDriverName.findRev("/")+1);
			m_friendlyName = friendlyDriverName;
		}
		else if (m_modAlias.lower().startsWith("pci")) {
			m_friendlyName = HardwareDevices::instance()->findPCIDeviceName(m_vendorID, m_modelID, m_subvendorID, m_submodelID);
		}
		else if (m_modAlias.lower().startsWith("usb")) {
			m_friendlyName = HardwareDevices::instance()->findUSBDeviceName(m_vendorID, m_modelID, m_subvendorID, m_submodelID);
		}
		else {
			TQString acpigentype = systemPath();
			acpigentype.truncate(acpigentype.length()-1);
			acpigentype.remove(0, acpigentype.findRev("/")+1);
			TQString pnpgentype = acpigentype;
			pnpgentype.truncate(pnpgentype.find(":"));
			if (pnpgentype.startsWith("PNP")) {
				m_friendlyName = HardwareDevices::instance()->findPNPDeviceName(pnpgentype);
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
		if (type() == GenericDeviceType::CPU) {
			m_friendlyName = name();
		}
		else if (type() == GenericDeviceType::Event) {
			// Use parent node name
			if (m_parentDevice) {
				return m_parentDevice->friendlyName();
			}
			else {
				m_friendlyName = i18n("Generic Event Device");
			}
		}
		else if (type() == GenericDeviceType::Input) {
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

#include "genericdevice.moc"
