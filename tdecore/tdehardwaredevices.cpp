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

#include <libudev.h>

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

TDEHardwareDevices::TDEHardwareDevices() {
	// Set up device list
	m_deviceList.setAutoDelete( TRUE );	// the list owns the objects

	// Initialize udev interface
	m_udevStruct = udev_new();
	if (!m_udevStruct) {
		printf("Unable to create udev interface\n\r");
	}

	// Update internal device information
	queryHardwareInformation();
}

TDEHardwareDevices::~TDEHardwareDevices() {
	// Tear down udev interface
	udev_unref(m_udevStruct);
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
	
		// Classify device and create TDEW device object
		TQString devicename(udev_device_get_sysname(dev));
		TQString devicetype(udev_device_get_devtype(dev));
		TQString devicedriver(udev_device_get_driver(dev));
		TQString devicesubsystem(udev_device_get_subsystem(dev));
		TQString devicenode(udev_device_get_devnode(dev));
		TQString systempath(udev_device_get_syspath(dev));
		bool removable = false;
		TDEGenericDevice* device = 0;

		if (devicetype == "disk") {
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
			if (removable) {
				device = new TDEGenericDevice(TDEGenericDeviceType::RemovableDisk);
			}
			else {
				device = new TDEGenericDevice(TDEGenericDeviceType::FixedDisk);
			}
		}
		else if (devicetype.isNull()) {
			if (devicesubsystem == "acpi") {
				device = new TDEGenericDevice(TDEGenericDeviceType::OtherACPI);
			}
			else if (devicesubsystem == "input") {
				device = new TDEGenericDevice(TDEGenericDeviceType::HID);
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
		m_deviceList.append(device);
	}

	// Free the enumerator object
	udev_enumerate_unref(enumerate);

	return true;
}

TDEGenericHardwareList &TDEHardwareDevices::listAllPhysicalDevices() {
	return m_deviceList;
}