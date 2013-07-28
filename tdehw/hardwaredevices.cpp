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

#include "hardwaredevices.h"

#include <tqfile.h>
#include <tqdir.h>
#include <tqtimer.h>
#include <tqsocketnotifier.h>
#include <tqstringlist.h>

#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <dcopclient.h>

extern "C" {
#include <libudev.h>
}

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Network devices
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>

// Backlight devices
#include <linux/fb.h>

// Input devices
#include <linux/input.h>

#include <kiconloader.h>

#include "genericdevice.h"
#include "storagedevice.h"
#include "cpudevice.h"
#include "batterydevice.h"
#include "mainspowerdevice.h"
#include "networkdevice.h"
#include "backlightdevice.h"
#include "monitordevice.h"
#include "sensordevice.h"
#include "rootsystemdevice.h"
#include "eventdevice.h"
#include "inputdevice.h"

// Compile-time configuration
#include "config.h"

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

using namespace TDEHW;

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

HardwareDevices* HardwareDevices::_instance = 0;

HardwareDevices* HardwareDevices::instance() {
	static KStaticDeleter<HardwareDevices> deleter;

	if( !_instance ) {
		deleter.setObject(_instance, new HardwareDevices);
	}

	return _instance;
}

HardwareDevices::HardwareDevices() {
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
		printf("Unable to create udev interface\n");
	}

	if (m_udevStruct) {
		// Set up device add/remove monitoring
		m_udevMonitorStruct = udev_monitor_new_from_netlink(m_udevStruct, "udev");
		udev_monitor_filter_add_match_subsystem_devtype(m_udevMonitorStruct, NULL, NULL);
		udev_monitor_enable_receiving(m_udevMonitorStruct);

		int udevmonitorfd = udev_monitor_get_fd(m_udevMonitorStruct);
		if (udevmonitorfd >= 0) {
			m_devScanNotifier = new TQSocketNotifier(udevmonitorfd, TQSocketNotifier::Read, this);
			connect( m_devScanNotifier, TQT_SIGNAL(activated(int)), this, TQT_SLOT(processHotPluggedHardware()) );
		}

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
		if (m_procMountsFd >= 0) {
			m_mountScanNotifier = new TQSocketNotifier(m_procMountsFd, TQSocketNotifier::Exception, this);
			connect( m_mountScanNotifier, TQT_SIGNAL(activated(int)), this, TQT_SLOT(processModifiedMounts()) );
		}

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
#endif

		// Some devices do not receive update signals from udev
		// These devices must be polled, and a good polling interval is 1 second
		m_deviceWatchTimer = new TQTimer(this);
		connect( m_deviceWatchTimer, SIGNAL(timeout()), this, SLOT(processStatelessDevices()) );

		// Update internal device information
		queryHardwareInformation();
	}
}

HardwareDevices::~HardwareDevices() {
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

void HardwareDevices::setTriggerlessHardwareUpdatesEnabled(bool enable) {
	if (enable) {
		TQDir nodezerocpufreq("/sys/devices/system/cpu/cpu0/cpufreq");
		if (nodezerocpufreq.exists()) {
			m_cpuWatchTimer->start( 500, FALSE ); // 0.5 second repeating timer
		}
		m_deviceWatchTimer->start( 1000, FALSE ); // 1 second repeating timer
	}
	else {
		m_cpuWatchTimer->stop();
		m_deviceWatchTimer->stop();
	}
}

void HardwareDevices::rescanDeviceInformation(GenericDevice* hwdevice) {
	rescanDeviceInformation(hwdevice, true);
}

void HardwareDevices::rescanDeviceInformation(GenericDevice* hwdevice, bool regenerateDeviceTree) {
	struct udev_device *dev;
	dev = udev_device_new_from_syspath(m_udevStruct, hwdevice->systemPath().ascii());
	updateExistingDeviceInformation(hwdevice);
	if (regenerateDeviceTree) {
		updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
	}
	udev_device_unref(dev);
}

GenericDevice* HardwareDevices::findBySystemPath(TQString syspath) {
	if (!syspath.endsWith("/")) {
		syspath += "/";
	}
	GenericDevice *hwdevice;
	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	GenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		if (hwdevice->systemPath() == syspath) {
			return hwdevice;
		}
	}

	return 0;
}

GenericDevice* HardwareDevices::findByUniqueID(TQString uid) {
	GenericDevice *hwdevice;
	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	GenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		if (hwdevice->uniqueID() == uid) {
			return hwdevice;
		}
	}

	return 0;
}

GenericDevice* HardwareDevices::findByDeviceNode(TQString devnode) {
	GenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->deviceNode() == devnode) {
			return hwdevice;
		}
	}

	return 0;
}

StorageDevice* HardwareDevices::findDiskByUID(TQString uid) {
	GenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->type() == GenericDeviceType::Disk) {
			StorageDevice* sdevice = static_cast<StorageDevice*>(hwdevice);
			if (sdevice->uniqueID() == uid) {
				return sdevice;
			}
		}
	}

	return 0;
}

void HardwareDevices::processHotPluggedHardware() {
	udev_device* dev = udev_monitor_receive_device(m_udevMonitorStruct);
	if (dev) {
		TQString actionevent(udev_device_get_action(dev));
		if (actionevent == "add") {
			GenericDevice* device = classifyUnknownDevice(dev);

			// Make sure this device is not a duplicate
			GenericDevice *hwdevice;
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
				emit hardwareEvent(HardwareEvent::HardwareAdded, device->uniqueID());
			}
		}
		else if (actionevent == "remove") {
			// Delete device from hardware listing
			TQString systempath(udev_device_get_syspath(dev));
			systempath += "/";
			GenericDevice *hwdevice;
			for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
				if (hwdevice->systemPath() == systempath) {
					emit hardwareRemoved(hwdevice);
					emit hardwareEvent(HardwareEvent::HardwareRemoved, hwdevice->uniqueID());

					// If the device is a storage device and has a slave, update it as well
					if (hwdevice->type() == GenericDeviceType::Disk) {
						StorageDevice* sdevice = static_cast<StorageDevice*>(hwdevice);
						TQStringList slavedevices = sdevice->slaveDevices();
						m_deviceList.remove(hwdevice);
						for ( TQStringList::Iterator slaveit = slavedevices.begin(); slaveit != slavedevices.end(); ++slaveit ) {
							GenericDevice* slavedevice = findBySystemPath(*slaveit);
							if (slavedevice) {
								rescanDeviceInformation(slavedevice);
								emit hardwareUpdated(slavedevice);
								emit hardwareEvent(HardwareEvent::HardwareUpdated, slavedevice->uniqueID());
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
			systempath += "/";
			GenericDevice *hwdevice;
			for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
				if (hwdevice->systemPath() == systempath) {
					if (!hwdevice->blacklistedForUpdate()) {
						classifyUnknownDevice(dev, hwdevice, false);
						updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
						emit hardwareUpdated(hwdevice);
						emit hardwareEvent(HardwareEvent::HardwareUpdated, hwdevice->uniqueID());
					}
				}
				else if ((hwdevice->type() == GenericDeviceType::Monitor)
						&& (hwdevice->systemPath().contains(systempath))) {
					if (!hwdevice->blacklistedForUpdate()) {
						struct udev_device *slavedev;
						slavedev = udev_device_new_from_syspath(m_udevStruct, hwdevice->systemPath().ascii());
						classifyUnknownDevice(slavedev, hwdevice, false);
						udev_device_unref(slavedev);
						updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
						emit hardwareUpdated(hwdevice);
						emit hardwareEvent(HardwareEvent::HardwareUpdated, hwdevice->uniqueID());
					}
				}
			}
		}
	}
}

void HardwareDevices::processModifiedCPUs() {
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

	// Ensure "processor" is the first entry in each block and determine which cpuinfo type is in use
	bool cpuinfo_format_x86 = true;
	bool cpuinfo_format_arm = false;

	TQString curline1;
	TQString curline2;
	int blockNumber = 0;
	TQStringList::Iterator blockBegin = m_cpuInfo.begin();
	for (TQStringList::Iterator cpuit1 = m_cpuInfo.begin(); cpuit1 != m_cpuInfo.end(); ++cpuit1) {
		curline1 = *cpuit1;
		curline1 = curline1.stripWhiteSpace();
		if (!(*blockBegin).startsWith("processor")) {
			bool found = false;
			TQStringList::Iterator cpuit2;
			for (cpuit2 = blockBegin; cpuit2 != m_cpuInfo.end(); ++cpuit2) {
				curline2 = *cpuit2;
				curline2 = curline2.stripWhiteSpace();
				if (curline2.startsWith("processor")) {
					found = true;
					break;
				}
				else if (curline2 == "") {
					break;
				}
			}
			if (found) {
				m_cpuInfo.insert(blockBegin, (*cpuit2));
			}
			else {
				m_cpuInfo.insert(blockBegin, "processor : 0");
			}
		}
		if (curline1 == "") {
			blockNumber++;
			blockBegin = cpuit1;
			blockBegin++;
		}
		if (curline1.startsWith("Processor")) {
			cpuinfo_format_x86 = false;
			cpuinfo_format_arm = true;
		}
	}

	// Parse CPU information table
	CPUDevice *cdevice;
	cdevice = 0;
	bool modified = false;
	bool have_frequency = false;

	TQString curline;
	int processorNumber = 0;
	int processorCount = 0;

	if (cpuinfo_format_x86) {
		// ===================================================================================================================================
		// x86/x86_64
		// ===================================================================================================================================
		TQStringList::Iterator cpuit;
		for (cpuit = m_cpuInfo.begin(); cpuit != m_cpuInfo.end(); ++cpuit) {
			curline = *cpuit;
			if (curline.startsWith("processor")) {
				curline.remove(0, curline.find(":")+1);
				curline = curline.stripWhiteSpace();
				processorNumber = curline.toInt();
				if (!cdevice) cdevice = dynamic_cast<CPUDevice*>(findBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber)));
				if (cdevice) {
					if (cdevice->coreNumber() != processorNumber) modified = true;
					cdevice->internalSetCoreNumber(processorNumber);
				}
			}
			if (curline.startsWith("model name")) {
				curline.remove(0, curline.find(":")+1);
				curline = curline.stripWhiteSpace();
				if (cdevice) {
					if (cdevice->name() != curline) modified = true;
					cdevice->internalSetName(curline);
				}
			}
			if (curline.startsWith("cpu MHz")) {
				curline.remove(0, curline.find(":")+1);
				curline = curline.stripWhiteSpace();
				if (cdevice) {
					if (cdevice->frequency() != curline.toDouble()) modified = true;
					cdevice->internalSetFrequency(curline.toDouble());
					have_frequency = true;
				}
			}
			if (curline.startsWith("vendor_id")) {
				curline.remove(0, curline.find(":")+1);
				curline = curline.stripWhiteSpace();
				if (cdevice) {
					if (cdevice->vendorName() != curline) modified = true;
					cdevice->internalSetVendorName(curline);
					if (cdevice->vendorEncoded() != curline) modified = true;
					cdevice->internalSetVendorEncoded(curline);
				}
			}
			curline = curline.stripWhiteSpace();
			if (curline == "") {
				cdevice = 0;
			}
		}
	}
	else if (cpuinfo_format_arm) {
		// ===================================================================================================================================
		// ARM
		// ===================================================================================================================================
		TQStringList::Iterator cpuit;
		TQString modelName;
		TQString vendorName;
		TQString serialNumber;
		for (cpuit = m_cpuInfo.begin(); cpuit != m_cpuInfo.end(); ++cpuit) {
			curline = *cpuit;
			if (curline.startsWith("Processor")) {
				curline.remove(0, curline.find(":")+1);
				curline = curline.stripWhiteSpace();
				modelName = curline;
			}
			if (curline.startsWith("Hardware")) {
				curline.remove(0, curline.find(":")+1);
				curline = curline.stripWhiteSpace();
				vendorName = curline;
			}
			if (curline.startsWith("Serial")) {
				curline.remove(0, curline.find(":")+1);
				curline = curline.stripWhiteSpace();
				serialNumber = curline;
			}
		}
		for (TQStringList::Iterator cpuit = m_cpuInfo.begin(); cpuit != m_cpuInfo.end(); ++cpuit) {
			curline = *cpuit;
			if (curline.startsWith("processor")) {
				curline.remove(0, curline.find(":")+1);
				curline = curline.stripWhiteSpace();
				processorNumber = curline.toInt();
				if (!cdevice) {
					cdevice = dynamic_cast<CPUDevice*>(findBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber)));
					if (cdevice) {
						// Set up CPU information structures
						if (cdevice->coreNumber() != processorNumber) modified = true;
						cdevice->internalSetCoreNumber(processorNumber);
						if (cdevice->name() != modelName) modified = true;
						cdevice->internalSetName(modelName);
						if (cdevice->vendorName() != vendorName) modified = true;
						cdevice->internalSetVendorName(vendorName);
						if (cdevice->vendorEncoded() != vendorName) modified = true;
						cdevice->internalSetVendorEncoded(vendorName);
						if (cdevice->serialNumber() != serialNumber) modified = true;
						cdevice->internalSetSerialNumber(serialNumber);
					}
				}
			}
			curline = curline.stripWhiteSpace();
			if (curline == "") {
				cdevice = 0;
			}
		}
	}

	processorCount = processorNumber+1;

	// Read in other information from cpufreq, if available
	for (processorNumber=0; processorNumber<processorCount; processorNumber++) {
		cdevice = dynamic_cast<CPUDevice*>(findBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber)));
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
			nodename.append("/cpuinfo_min_freq");
			TQFile minfrequencyfile(nodename);
			if (minfrequencyfile.open(IO_ReadOnly)) {
				TQTextStream stream( &minfrequencyfile );
				minfrequency = stream.readLine().toDouble()/1000.0;
				minfrequencyfile.close();
			}
			nodename = cpufreq_dir.path();
			nodename.append("/cpuinfo_max_freq");
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

			if (!have_frequency) {
				nodename = cpufreq_dir.path();
				nodename.append("/cpuinfo_cur_freq");
				TQFile cpufreqfile(nodename);
				if (cpufreqfile.open(IO_ReadOnly)) {
					TQTextStream stream( &cpufreqfile );
					if (cdevice) cdevice->internalSetFrequency(stream.readLine().toDouble()/1000.0);
					cpufreqfile.close();
					have_frequency = true;
				}
			}

			bool frequencyFound;
			TQStringList::Iterator freqit;
			frequencyFound = false;
			for ( freqit = frequencylist.begin(); freqit != frequencylist.end(); ++freqit ) {
				double thisfrequency = (*freqit).toDouble()/1000.0;
				if (thisfrequency == minfrequency) {
					frequencyFound = true;
				}
			}
			if (!frequencyFound) {
				int minFrequencyInt = (minfrequency*1000.0);
				frequencylist.prepend(TQString("%1").arg(minFrequencyInt));
			}
			frequencyFound = false;
			for ( freqit = frequencylist.begin(); freqit != frequencylist.end(); ++freqit ) {
				double thisfrequency = (*freqit).toDouble()/1000.0;
				if (thisfrequency == maxfrequency) {
					frequencyFound = true;
				}
			}
			if (!frequencyFound) {
				int maxfrequencyInt = (maxfrequency*1000.0);
				frequencylist.append(TQString("%1").arg(maxfrequencyInt));
			}
		}
		else {
			if (have_frequency) {
				if (cdevice) {
					minfrequency = cdevice->frequency();
					maxfrequency = cdevice->frequency();
				}
			}
		}

		// Update CPU information structure
		if (cdevice) {
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
	}

	if (modified) {
		for (processorNumber=0; processorNumber<processorCount; processorNumber++) {
			GenericDevice* hwdevice = findBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber));
			// Signal new information available
			emit hardwareUpdated(hwdevice);
			emit hardwareEvent(HardwareEvent::HardwareUpdated, hwdevice->uniqueID());
		}
	}
}

void HardwareDevices::processStatelessDevices() {
	// Some devices do not emit changed signals
	// So far, network cards and sensors need to be polled
	GenericDevice *hwdevice;

	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	GenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		if ((hwdevice->type() == GenericDeviceType::RootSystem) || (hwdevice->type() == GenericDeviceType::Network) || (hwdevice->type() == GenericDeviceType::OtherSensor) || (hwdevice->type() == GenericDeviceType::Event) || (hwdevice->type() == GenericDeviceType::Battery) || (hwdevice->type() == GenericDeviceType::PowerSupply)) {
			rescanDeviceInformation(hwdevice, false);
			emit hardwareUpdated(hwdevice);
			emit hardwareEvent(HardwareEvent::HardwareUpdated, hwdevice->uniqueID());
		}
	}
}

void HardwareDevices::processEventDeviceKeyPressed(unsigned int keycode, EventDevice* edevice) {
	emit eventDeviceKeyPressed(keycode, edevice);
}

void HardwareDevices::processModifiedMounts() {
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
		GenericDevice* hwdevice = findByDeviceNode(*mountInfo.at(0));
		if (hwdevice) {
			emit hardwareUpdated(hwdevice);
			emit hardwareEvent(HardwareEvent::HardwareUpdated, hwdevice->uniqueID());
			// If the device is a storage device and has a slave, update it as well
			if (hwdevice->type() == GenericDeviceType::Disk) {
				StorageDevice* sdevice = static_cast<StorageDevice*>(hwdevice);
				TQStringList slavedevices = sdevice->slaveDevices();
				for ( TQStringList::Iterator slaveit = slavedevices.begin(); slaveit != slavedevices.end(); ++slaveit ) {
					GenericDevice* slavedevice = findBySystemPath(*slaveit);
					if (slavedevice) {
						emit hardwareUpdated(slavedevice);
						emit hardwareEvent(HardwareEvent::HardwareUpdated, slavedevice->uniqueID());
					}
				}
			}
		}
	}
	for ( it = deletedEntries.begin(); it != deletedEntries.end(); ++it ) {
		TQStringList mountInfo = TQStringList::split(" ", (*it), true);
		// Try to find a device that matches the altered node
		GenericDevice* hwdevice = findByDeviceNode(*mountInfo.at(0));
		if (hwdevice) {
			emit hardwareUpdated(hwdevice);
			emit hardwareEvent(HardwareEvent::HardwareUpdated, hwdevice->uniqueID());
			// If the device is a storage device and has a slave, update it as well
			if (hwdevice->type() == GenericDeviceType::Disk) {
				StorageDevice* sdevice = static_cast<StorageDevice*>(hwdevice);
				TQStringList slavedevices = sdevice->slaveDevices();
				for ( TQStringList::Iterator slaveit = slavedevices.begin(); slaveit != slavedevices.end(); ++slaveit ) {
					GenericDevice* slavedevice = findBySystemPath(*slaveit);
					if (slavedevice) {
						emit hardwareUpdated(slavedevice);
						emit hardwareEvent(HardwareEvent::HardwareUpdated, slavedevice->uniqueID());
					}
				}
			}
		}
	}

	emit mountTableModified();
	emit hardwareEvent(HardwareEvent::MountTableModified, TQString());
}

DiskDeviceType::DiskDeviceType classifyDiskType(udev_device* dev, const TQString devicenode, const TQString devicebus, const TQString disktypestring, const TQString systempath, const TQString devicevendor, const TQString devicemodel, const TQString filesystemtype, const TQString devicedriver) {
	// Classify a disk device type to the best of our ability
	DiskDeviceType::DiskDeviceType disktype = DiskDeviceType::Null;

	if (devicebus.upper() == "USB") {
		disktype = disktype | DiskDeviceType::USB;
	}

	if (disktypestring.upper() == "ZIP") {
		disktype = disktype | DiskDeviceType::Zip;
	}
	if ((devicevendor.upper() == "IOMEGA") && (devicemodel.upper().contains("ZIP"))) {
		disktype = disktype | DiskDeviceType::Zip;
	}

	if ((devicevendor.upper() == "APPLE") && (devicemodel.upper().contains("IPOD"))) {
		disktype = disktype | DiskDeviceType::MediaDevice;
	}
	if ((devicevendor.upper() == "SANDISK") && (devicemodel.upper().contains("SANSA"))) {
		disktype = disktype | DiskDeviceType::MediaDevice;
	}

	if (disktypestring.upper() == "TAPE") {
		disktype = disktype | DiskDeviceType::Tape;
	}

	if ((disktypestring.upper() == "COMPACT_FLASH")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_CF")) == "1")) {
		disktype = disktype | DiskDeviceType::CompactFlash;
	}

	if ((disktypestring.upper() == "MEMORY_STICK")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_MS")) == "1")) {
		disktype = disktype | DiskDeviceType::MemoryStick;
	}

	if ((disktypestring.upper() == "SMART_MEDIA")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_SM")) == "1")) {
		disktype = disktype | DiskDeviceType::SmartMedia;
	}

	if ((disktypestring.upper() == "SD_MMC")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_SD")) == "1")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_SDHC")) == "1")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_MMC")) == "1")) {
		disktype = disktype | DiskDeviceType::SDMMC;
	}

	if ((disktypestring.upper() == "FLASHKEY")
		|| (TQString(udev_device_get_property_value(dev, " ID_DRIVE_FLASH")) == "1")) {
		disktype = disktype | DiskDeviceType::Flash;
	}

	if (disktypestring.upper() == "OPTICAL") {
		disktype = disktype | DiskDeviceType::Optical;
	}

	if (disktypestring.upper() == "JAZ") {
		disktype = disktype | DiskDeviceType::Jaz;
	}

	if (disktypestring.upper() == "DISK") {
		disktype = disktype | DiskDeviceType::HDD;
	}
	if (disktypestring.isNull()) {
		// Fallback
		// If we can't recognize the disk type then set it as a simple HDD volume
		disktype = disktype | DiskDeviceType::HDD;
	}

	// Certain combinations of media flags should never be set at the same time as they don't make sense
	// This block is needed as udev is more than happy to provide inconsistent data to us
	if ((disktype & DiskDeviceType::Zip) || (disktype & DiskDeviceType::Floppy) || (disktype & DiskDeviceType::Jaz) || (disktype & DiskDeviceType::Tape)) {
		disktype = disktype & ~DiskDeviceType::HDD;
	}

	if (disktypestring.upper() == "CD") {
		disktype = disktype & ~DiskDeviceType::HDD;
		disktype = disktype | DiskDeviceType::Optical;

		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) == "1") {
			disktype = disktype | DiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_CD_R")) == "1") {
			disktype = disktype | DiskDeviceType::CDR;
			disktype = disktype & ~DiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_CD_RW")) == "1") {
			disktype = disktype | DiskDeviceType::CDRW;
			disktype = disktype & ~DiskDeviceType::CDROM;
			disktype = disktype & ~DiskDeviceType::CDR;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_MRW")) == "1") {
			disktype = disktype | DiskDeviceType::CDMRRW;
			disktype = disktype & ~DiskDeviceType::CDROM;
			disktype = disktype & ~DiskDeviceType::CDR;
			disktype = disktype & ~DiskDeviceType::CDRW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_MRW_W")) == "1") {
			disktype = disktype | DiskDeviceType::CDMRRWW;
			disktype = disktype & ~DiskDeviceType::CDROM;
			disktype = disktype & ~DiskDeviceType::CDR;
			disktype = disktype & ~DiskDeviceType::CDRW;
			disktype = disktype & ~DiskDeviceType::CDMRRW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_MO")) == "1") {
			disktype = disktype | DiskDeviceType::CDMO;
			disktype = disktype & ~DiskDeviceType::CDROM;
			disktype = disktype & ~DiskDeviceType::CDR;
			disktype = disktype & ~DiskDeviceType::CDRW;
			disktype = disktype & ~DiskDeviceType::CDMRRW;
			disktype = disktype & ~DiskDeviceType::CDMRRWW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD")) == "1") {
			disktype = disktype | DiskDeviceType::DVDROM;
			disktype = disktype & ~DiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RAM")) == "1") {
			disktype = disktype | DiskDeviceType::DVDRAM;
			disktype = disktype & ~DiskDeviceType::DVDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_R")) == "1") {
			disktype = disktype | DiskDeviceType::DVDR;
			disktype = disktype & ~DiskDeviceType::DVDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_R_DL")) == "1") {
			disktype = disktype | DiskDeviceType::DVDRDL;
			disktype = disktype & ~DiskDeviceType::DVDROM;
			disktype = disktype & ~DiskDeviceType::DVDR;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_R")) == "1") {
			disktype = disktype | DiskDeviceType::DVDPLUSR;
			disktype = disktype & ~DiskDeviceType::DVDROM;
			disktype = disktype & ~DiskDeviceType::DVDR;
			disktype = disktype & ~DiskDeviceType::DVDRDL;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_R_DL")) == "1") {
			disktype = disktype | DiskDeviceType::DVDPLUSRDL;
			disktype = disktype & ~DiskDeviceType::DVDROM;
			disktype = disktype & ~DiskDeviceType::DVDR;
			disktype = disktype & ~DiskDeviceType::DVDRDL;
			disktype = disktype & ~DiskDeviceType::DVDPLUSR;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RW")) == "1") {
			disktype = disktype | DiskDeviceType::DVDRW;
			disktype = disktype & ~DiskDeviceType::DVDROM;
			disktype = disktype & ~DiskDeviceType::DVDR;
			disktype = disktype & ~DiskDeviceType::DVDRDL;
			disktype = disktype & ~DiskDeviceType::DVDPLUSR;
			disktype = disktype & ~DiskDeviceType::DVDPLUSRDL;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RW_DL")) == "1") {
			disktype = disktype | DiskDeviceType::DVDRWDL;
			disktype = disktype & ~DiskDeviceType::DVDROM;
			disktype = disktype & ~DiskDeviceType::DVDR;
			disktype = disktype & ~DiskDeviceType::DVDRDL;
			disktype = disktype & ~DiskDeviceType::DVDPLUSR;
			disktype = disktype & ~DiskDeviceType::DVDPLUSRDL;
			disktype = disktype & ~DiskDeviceType::DVDRW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW")) == "1") {
			disktype = disktype | DiskDeviceType::DVDPLUSRW;
			disktype = disktype & ~DiskDeviceType::DVDROM;
			disktype = disktype & ~DiskDeviceType::DVDR;
			disktype = disktype & ~DiskDeviceType::DVDRDL;
			disktype = disktype & ~DiskDeviceType::DVDPLUSR;
			disktype = disktype & ~DiskDeviceType::DVDPLUSRDL;
			disktype = disktype & ~DiskDeviceType::DVDRW;
			disktype = disktype & ~DiskDeviceType::DVDRWDL;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW_DL")) == "1") {
			disktype = disktype | DiskDeviceType::DVDPLUSRWDL;
			disktype = disktype & ~DiskDeviceType::DVDROM;
			disktype = disktype & ~DiskDeviceType::DVDR;
			disktype = disktype & ~DiskDeviceType::DVDRDL;
			disktype = disktype & ~DiskDeviceType::DVDPLUSR;
			disktype = disktype & ~DiskDeviceType::DVDPLUSRDL;
			disktype = disktype & ~DiskDeviceType::DVDRW;
			disktype = disktype & ~DiskDeviceType::DVDRWDL;
			disktype = disktype & ~DiskDeviceType::DVDPLUSRW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD")) == "1") {
			disktype = disktype | DiskDeviceType::BDROM;
			disktype = disktype & ~DiskDeviceType::CDROM;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_R")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_R_DL")) == "1")	// FIXME There is no official udev attribute for this type of disc (yet!)
			) {
			disktype = disktype | DiskDeviceType::BDR;
			disktype = disktype & ~DiskDeviceType::BDROM;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_RE")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_RE_DL")) == "1")	// FIXME There is no official udev attribute for this type of disc (yet!)
			) {
			disktype = disktype | DiskDeviceType::BDRW;
			disktype = disktype & ~DiskDeviceType::BDROM;
			disktype = disktype & ~DiskDeviceType::BDR;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_HDDVD")) == "1") {
			disktype = disktype | DiskDeviceType::HDDVDROM;
			disktype = disktype & ~DiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_HDDVD_R")) == "1") {
			disktype = disktype | DiskDeviceType::HDDVDR;
			disktype = disktype & ~DiskDeviceType::HDDVDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_HDDVD_RW")) == "1") {
			disktype = disktype | DiskDeviceType::HDDVDRW;
			disktype = disktype & ~DiskDeviceType::HDDVDROM;
			disktype = disktype & ~DiskDeviceType::HDDVDR;
		}
		if (!TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO")).isNull()) {
			disktype = disktype | DiskDeviceType::CDAudio;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_VCD")) == "1") || (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_SDVD")) == "1")) {
			disktype = disktype | DiskDeviceType::CDVideo;
		}

		if ((disktype & DiskDeviceType::DVDROM)
			|| (disktype & DiskDeviceType::DVDRAM)
			|| (disktype & DiskDeviceType::DVDR)
			|| (disktype & DiskDeviceType::DVDRW)
			|| (disktype & DiskDeviceType::DVDRDL)
			|| (disktype & DiskDeviceType::DVDRWDL)
			|| (disktype & DiskDeviceType::DVDPLUSR)
			|| (disktype & DiskDeviceType::DVDPLUSRW)
			|| (disktype & DiskDeviceType::DVDPLUSRDL)
			|| (disktype & DiskDeviceType::DVDPLUSRWDL)
			) {
				// Every VideoDVD must have a VIDEO_TS.IFO file
				// Read this info via tdeiso_info, since udev couldn't be bothered to check DVD type on its own
				int retcode = system(TQString("tdeiso_info --exists=ISO9660/VIDEO_TS/VIDEO_TS.IFO %1").arg(devicenode).ascii());
				if (retcode == 0) {
					disktype = disktype | DiskDeviceType::DVDVideo;
				}
		}

	}

	// Detect RAM and Loop devices, since udev can't seem to...
	if (systempath.startsWith("/sys/devices/virtual/block/ram")) {
		disktype = disktype | DiskDeviceType::RAM;
	}
	if (systempath.startsWith("/sys/devices/virtual/block/loop")) {
		disktype = disktype | DiskDeviceType::Loop;
	}

	if (filesystemtype.upper() == "CRYPTO_LUKS") {
		disktype = disktype | DiskDeviceType::LUKS;
	}
	else if (filesystemtype.upper() == "CRYPTO") {
		disktype = disktype | DiskDeviceType::OtherCrypted;
	}

	return disktype;
}

	// TDEStandardDirs::kde_default

typedef TQMap<TQString, TQString> TDEConfigMap;

TQString readUdevAttribute(udev_device* dev, TQString attr) {
	return TQString(udev_device_get_property_value(dev, attr.ascii()));
}

GenericDeviceType::GenericDeviceType readGenericDeviceTypeFromString(TQString query) {
	GenericDeviceType::GenericDeviceType ret = GenericDeviceType::Other;

	// Keep this in sync with the GenericDeviceType definition in the header
	if (query == "Root") {
		ret = GenericDeviceType::Root;
	}
	else if (query == "RootSystem") {
		ret = GenericDeviceType::RootSystem;
	}
	else if (query == "CPU") {
		ret = GenericDeviceType::CPU;
	}
	else if (query == "GPU") {
		ret = GenericDeviceType::GPU;
	}
	else if (query == "RAM") {
		ret = GenericDeviceType::RAM;
	}
	else if (query == "Bus") {
		ret = GenericDeviceType::Bus;
	}
	else if (query == "I2C") {
		ret = GenericDeviceType::I2C;
	}
	else if (query == "MDIO") {
		ret = GenericDeviceType::MDIO;
	}
	else if (query == "Mainboard") {
		ret = GenericDeviceType::Mainboard;
	}
	else if (query == "Disk") {
		ret = GenericDeviceType::Disk;
	}
	else if (query == "SCSI") {
		ret = GenericDeviceType::SCSI;
	}
	else if (query == "StorageController") {
		ret = GenericDeviceType::StorageController;
	}
	else if (query == "Mouse") {
		ret = GenericDeviceType::Mouse;
	}
	else if (query == "Keyboard") {
		ret = GenericDeviceType::Keyboard;
	}
	else if (query == "HID") {
		ret = GenericDeviceType::HID;
	}
	else if (query == "Modem") {
		ret = GenericDeviceType::Modem;
	}
	else if (query == "Monitor") {
		ret = GenericDeviceType::Monitor;
	}
	else if (query == "Network") {
		ret = GenericDeviceType::Network;
	}
	else if (query == "Printer") {
		ret = GenericDeviceType::Printer;
	}
	else if (query == "Scanner") {
		ret = GenericDeviceType::Scanner;
	}
	else if (query == "Sound") {
		ret = GenericDeviceType::Sound;
	}
	else if (query == "VideoCapture") {
		ret = GenericDeviceType::VideoCapture;
	}
	else if (query == "IEEE1394") {
		ret = GenericDeviceType::IEEE1394;
	}
	else if (query == "PCMCIA") {
		ret = GenericDeviceType::PCMCIA;
	}
	else if (query == "Camera") {
		ret = GenericDeviceType::Camera;
	}
	else if (query == "Serial") {
		ret = GenericDeviceType::Serial;
	}
	else if (query == "Parallel") {
		ret = GenericDeviceType::Parallel;
	}
	else if (query == "TextIO") {
		ret = GenericDeviceType::TextIO;
	}
	else if (query == "Peripheral") {
		ret = GenericDeviceType::Peripheral;
	}
	else if (query == "Backlight") {
		ret = GenericDeviceType::Backlight;
	}
	else if (query == "Battery") {
		ret = GenericDeviceType::Battery;
	}
	else if (query == "Power") {
		ret = GenericDeviceType::PowerSupply;
	}
	else if (query == "Dock") {
		ret = GenericDeviceType::Dock;
	}
	else if (query == "ThermalSensor") {
		ret = GenericDeviceType::ThermalSensor;
	}
	else if (query == "ThermalControl") {
		ret = GenericDeviceType::ThermalControl;
	}
	else if (query == "Bluetooth") {
		ret = GenericDeviceType::BlueTooth;
	}
	else if (query == "Bridge") {
		ret = GenericDeviceType::Bridge;
	}
	else if (query == "Platform") {
		ret = GenericDeviceType::Platform;
	}
	else if (query == "Cryptography") {
		ret = GenericDeviceType::Cryptography;
	}
	else if (query == "Event") {
		ret = GenericDeviceType::Event;
	}
	else if (query == "Input") {
		ret = GenericDeviceType::Input;
	}
	else if (query == "PNP") {
		ret = GenericDeviceType::PNP;
	}
	else if (query == "OtherACPI") {
		ret = GenericDeviceType::OtherACPI;
	}
	else if (query == "OtherUSB") {
		ret = GenericDeviceType::OtherUSB;
	}
	else if (query == "OtherMultimedia") {
		ret = GenericDeviceType::OtherMultimedia;
	}
	else if (query == "OtherPeripheral") {
		ret = GenericDeviceType::OtherPeripheral;
	}
	else if (query == "OtherSensor") {
		ret = GenericDeviceType::OtherSensor;
	}
	else if (query == "OtherVirtual") {
		ret = GenericDeviceType::OtherVirtual;
	}
	else {
		ret = GenericDeviceType::Other;
	}

	return ret;
}

DiskDeviceType::DiskDeviceType readDiskDeviceSubtypeFromString(TQString query, DiskDeviceType::DiskDeviceType flagsIn=DiskDeviceType::Null) {
	DiskDeviceType::DiskDeviceType ret = flagsIn;

	// Keep this in sync with the DiskDeviceType definition in the header
	if (query == "MediaDevice") {
		ret = ret | DiskDeviceType::MediaDevice;
	}
	if (query == "Floppy") {
		ret = ret | DiskDeviceType::Floppy;
	}
	if (query == "CDROM") {
		ret = ret | DiskDeviceType::CDROM;
	}
	if (query == "CDR") {
		ret = ret | DiskDeviceType::CDR;
	}
	if (query == "CDRW") {
		ret = ret | DiskDeviceType::CDRW;
	}
	if (query == "CDMO") {
		ret = ret | DiskDeviceType::CDMO;
	}
	if (query == "CDMRRW") {
		ret = ret | DiskDeviceType::CDMRRW;
	}
	if (query == "CDMRRWW") {
		ret = ret | DiskDeviceType::CDMRRWW;
	}
	if (query == "DVDROM") {
		ret = ret | DiskDeviceType::DVDROM;
	}
	if (query == "DVDRAM") {
		ret = ret | DiskDeviceType::DVDRAM;
	}
	if (query == "DVDR") {
		ret = ret | DiskDeviceType::DVDR;
	}
	if (query == "DVDRW") {
		ret = ret | DiskDeviceType::DVDRW;
	}
	if (query == "DVDRDL") {
		ret = ret | DiskDeviceType::DVDRDL;
	}
	if (query == "DVDRWDL") {
		ret = ret | DiskDeviceType::DVDRWDL;
	}
	if (query == "DVDPLUSR") {
		ret = ret | DiskDeviceType::DVDPLUSR;
	}
	if (query == "DVDPLUSRW") {
		ret = ret | DiskDeviceType::DVDPLUSRW;
	}
	if (query == "DVDPLUSRDL") {
		ret = ret | DiskDeviceType::DVDPLUSRDL;
	}
	if (query == "DVDPLUSRWDL") {
		ret = ret | DiskDeviceType::DVDPLUSRWDL;
	}
	if (query == "BDROM") {
		ret = ret | DiskDeviceType::BDROM;
	}
	if (query == "BDR") {
		ret = ret | DiskDeviceType::BDR;
	}
	if (query == "BDRW") {
		ret = ret | DiskDeviceType::BDRW;
	}
	if (query == "HDDVDROM") {
		ret = ret | DiskDeviceType::HDDVDROM;
	}
	if (query == "HDDVDR") {
		ret = ret | DiskDeviceType::HDDVDR;
	}
	if (query == "HDDVDRW") {
		ret = ret | DiskDeviceType::HDDVDRW;
	}
	if (query == "Zip") {
		ret = ret | DiskDeviceType::Zip;
	}
	if (query == "Jaz") {
		ret = ret | DiskDeviceType::Jaz;
	}
	if (query == "Camera") {
		ret = ret | DiskDeviceType::Camera;
	}
	if (query == "LUKS") {
		ret = ret | DiskDeviceType::LUKS;
	}
	if (query == "OtherCrypted") {
		ret = ret | DiskDeviceType::OtherCrypted;
	}
	if (query == "CDAudio") {
		ret = ret | DiskDeviceType::CDAudio;
	}
	if (query == "CDVideo") {
		ret = ret | DiskDeviceType::CDVideo;
	}
	if (query == "DVDVideo") {
		ret = ret | DiskDeviceType::DVDVideo;
	}
	if (query == "BDVideo") {
		ret = ret | DiskDeviceType::BDVideo;
	}
	if (query == "Flash") {
		ret = ret | DiskDeviceType::Flash;
	}
	if (query == "USB") {
		ret = ret | DiskDeviceType::USB;
	}
	if (query == "Tape") {
		ret = ret | DiskDeviceType::Tape;
	}
	if (query == "HDD") {
		ret = ret | DiskDeviceType::HDD;
	}
	if (query == "Optical") {
		ret = ret | DiskDeviceType::Optical;
	}
	if (query == "RAM") {
		ret = ret | DiskDeviceType::RAM;
	}
	if (query == "Loop") {
		ret = ret | DiskDeviceType::Loop;
	}
	if (query == "CompactFlash") {
		ret = ret | DiskDeviceType::CompactFlash;
	}
	if (query == "MemoryStick") {
		ret = ret | DiskDeviceType::MemoryStick;
	}
	if (query == "SmartMedia") {
		ret = ret | DiskDeviceType::SmartMedia;
	}
	if (query == "SDMMC") {
		ret = ret | DiskDeviceType::SDMMC;
	}
	if (query == "UnlockedCrypt") {
		ret = ret | DiskDeviceType::UnlockedCrypt;
	}

	return ret;
}

GenericDevice* createDeviceObjectForType(GenericDeviceType::GenericDeviceType type) {
	GenericDevice* ret = 0;

	if (type == GenericDeviceType::Disk) {
		ret = new StorageDevice(type);
	}
	else {
		ret = new GenericDevice(type);
	}

	return ret;
}

GenericDevice* HardwareDevices::classifyUnknownDeviceByExternalRules(udev_device* dev, GenericDevice* existingdevice, bool classifySubDevices) {
	// This routine expects to see the hardware config files into <prefix>/share/apps/tdehwlib/deviceclasses/, suffixed with "hwclass"
	GenericDevice* device = existingdevice;
	if (!device) device = new GenericDevice(GenericDeviceType::Other);

	// Handle subtype if needed/desired
	// To speed things up we rely on the prior scan results stored in m_externalSubtype
	if (classifySubDevices) {
		if (!device->m_externalRulesFile.isNull()) {
			if (device->type() == GenericDeviceType::Disk) {
				// Disk class
				StorageDevice* sdevice = static_cast<StorageDevice*>(device);
				TQStringList subtype = device->m_externalSubtype;
				DiskDeviceType::DiskDeviceType desiredSubdeviceType = DiskDeviceType::Null;
				if (subtype.count()>0) {
					for ( TQStringList::Iterator paramit = subtype.begin(); paramit != subtype.end(); ++paramit ) {
						desiredSubdeviceType = readDiskDeviceSubtypeFromString(*paramit, desiredSubdeviceType);
					}
					if (desiredSubdeviceType != sdevice->diskType()) {
						printf("[tdehardwaredevices] Rules file %s used to set device subtype for device at path %s\n", device->m_externalRulesFile.ascii(), device->systemPath().ascii()); fflush(stdout);
						sdevice->internalSetDiskType(desiredSubdeviceType);
					}
				}
			}
		}
	}
	else {
		TQStringList hardware_info_directories(TDEGlobal::dirs()->resourceDirs("data"));
		TQString hardware_info_directory_suffix("tdehwlib/deviceclasses/");
		TQString hardware_info_directory;

		// Scan the hardware_info_directory for configuration files
		// For each one, open it with TDEConfig() and apply its rules to classify the device
		// FIXME
		// Should this also scan up to <n> subdirectories for the files?  That feature might end up being too expensive...

		device->m_externalRulesFile = TQString::null;
		for ( TQStringList::Iterator it = hardware_info_directories.begin(); it != hardware_info_directories.end(); ++it ) {
			hardware_info_directory = (*it);
			hardware_info_directory += hardware_info_directory_suffix;

			if (TDEGlobal::dirs()->exists(hardware_info_directory)) {
				TQDir d(hardware_info_directory);
				d.setFilter( TQDir::Files | TQDir::Hidden );

				const TQFileInfoList *list = d.entryInfoList();
				TQFileInfoListIterator it( *list );
				TQFileInfo *fi;

				while ((fi = it.current()) != 0) {
					if (fi->extension(false) == "hwclass") {
						bool match = true;

						// Open the rules file
						TDEConfig rulesFile(fi->absFilePath(), true, false);
						rulesFile.setGroup("Conditions");
						TDEConfigMap conditionmap = rulesFile.entryMap("Conditions");
						TDEConfigMap::Iterator cndit;
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
							GenericDeviceType::GenericDeviceType desiredDeviceType = device->type();
							if (!gentype.isNull()) {
								desiredDeviceType = readGenericDeviceTypeFromString(gentype);
							}
		
							// Handle main type
							if (desiredDeviceType != device->type()) {
								printf("[tdehardwaredevices] Rules file %s used to set device type for device at path %s\n", fi->absFilePath().ascii(), device->systemPath().ascii()); fflush(stdout);
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

GenericDevice* HardwareDevices::classifyUnknownDevice(udev_device* dev, GenericDevice* existingdevice, bool force_full_classification) {
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
	GenericDevice* device = existingdevice;
	bool temp_udev_device = !dev;
	if (dev) {
		devicename = (udev_device_get_sysname(dev));
		devicetype = (udev_device_get_devtype(dev));
		devicedriver = (udev_device_get_driver(dev));
		devicesubsystem = (udev_device_get_subsystem(dev));
		devicenode = (udev_device_get_devnode(dev));
		systempath = (udev_device_get_syspath(dev));
		systempath += "/";
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
		TQString syspathudev = systempath;
		syspathudev.truncate(syspathudev.length()-1);	// Remove trailing slash
		dev = udev_device_new_from_syspath(m_udevStruct, syspathudev.ascii());
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
	// Worse, sometimes udev provides an invalid model ID!
	// Go after it manually if needed...
	if (devicevendorid.isNull() || devicemodelid.isNull() || devicemodelid.contains("/")) {
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
	syspath_tail.truncate(syspath_tail.length()-1);
	syspath_tail.remove(0, syspath_tail.findRev("/")+1);
	if (syspath_tail.startsWith("event")) {
		if (!device) device = new EventDevice(GenericDeviceType::Event);
	}
	// Pull out all input special devices and stuff them under Input
	if (syspath_tail.startsWith("input")) {
		if (!device) device = new InputDevice(GenericDeviceType::Input);
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
		if (!device) device = new GenericDevice(GenericDeviceType::Keyboard);
	}

	// Classify specific known devices
	if (((devicetype == "disk")
		|| (devicetype == "partition")
		|| (devicedriver == "floppy")
		|| (devicesubsystem == "scsi_disk")
		|| (devicesubsystem == "scsi_tape"))
		&& ((devicenode != "")
		)) {
		if (!device) device = new StorageDevice(GenericDeviceType::Disk);
	}
	else if (devicetype == "host") {
		if (devicesubsystem == "bluetooth") {
			if (!device) device = new GenericDevice(GenericDeviceType::BlueTooth);
		}
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
				if (!device) device = new GenericDevice(GenericDeviceType::OtherACPI);
			}
			else {
				if (!device) device = new GenericDevice(GenericDeviceType::OtherACPI);
			}
		}
		else if (devicesubsystem == "input") {
			// Figure out if this device is a mouse, keyboard, or something else
			// Check for mouse
			// udev doesn't reliably help here, so guess from the device name
			if (systempath.contains("/mouse")) {
				if (!device) device = new GenericDevice(GenericDeviceType::Mouse);
			}
			if (!device) {
				// Second mouse check
				// Look for ID_INPUT_MOUSE property presence
				if (udev_device_get_property_value(dev, "ID_INPUT_MOUSE") != 0) {
					if (!device) device = new GenericDevice(GenericDeviceType::Mouse);
				}
			}
			if (!device) {
				// Check for keyboard
				// Look for ID_INPUT_KEYBOARD property presence
				if (udev_device_get_property_value(dev, "ID_INPUT_KEYBOARD") != 0) {
					if (!device) device = new GenericDevice(GenericDeviceType::Keyboard);
				}
			}
			if (!device) {
				if (!device) device = new GenericDevice(GenericDeviceType::HID);
			}
		}
		else if (devicesubsystem == "tty") {
			if (devicenode.contains("/ttyS")) {
				if (!device) device = new GenericDevice(GenericDeviceType::Serial);
			}
			else {
				if (!device) device = new GenericDevice(GenericDeviceType::TextIO);
			}
		}
		else if (devicesubsystem == "usb-serial") {
			if (!device) device = new GenericDevice(GenericDeviceType::Serial);
		}
		else if ((devicesubsystem == "spi_master")
			|| (devicesubsystem == "spidev")) {
			if (!device) device = new GenericDevice(GenericDeviceType::Serial);
		}
		else if (devicesubsystem == "spi") {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		else if (devicesubsystem == "thermal") {
			// FIXME
			// Figure out a way to differentiate between ThermalControl (fans and coolers) and ThermalSensor types
			if (!device) device = new GenericDevice(GenericDeviceType::ThermalControl);
		}
		else if (devicesubsystem == "hwmon") {
			// FIXME
			// This might pick up thermal sensors
			if (!device) device = new SensorDevice(GenericDeviceType::OtherSensor);
		}
		else if (devicesubsystem == "virtio") {
			if (devicedriver == "virtio_blk") {
				if (!device) device = new GenericDevice(GenericDeviceType::SCSI);
			}
			if (devicedriver == "virtio_net") {
				if (!device) device = new NetworkDevice(GenericDeviceType::Network);
			}
			if (devicedriver == "virtio_balloon") {
				if (!device) device = new GenericDevice(GenericDeviceType::RAM);
			}
		}
	}

	// Try to at least generally classify unclassified devices
	if (device == 0) {
		if (devicesubsystem == "backlight") {
			if (!device) device = new BacklightDevice(GenericDeviceType::Backlight);
		}
		if (systempath.lower().startsWith("/sys/devices/virtual")) {
			if (!device) device = new GenericDevice(GenericDeviceType::OtherVirtual);
		}
		if (systempath.lower().startsWith("/sys/module/")
			|| (systempath.lower().startsWith("/sys/kernel/"))) {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);	// FIXME Should go into a new kernel module category when the tdelibs ABI can be broken again
		}
		if ((devicetypestring == "audio")
			|| (devicesubsystem == "sound")
			|| (devicesubsystem == "ac97")) {
			if (!device) device = new GenericDevice(GenericDeviceType::Sound);
		}
		if ((devicesubsystem == "video4linux")
			|| (devicesubsystem == "dvb")) {
			if (!device) device = new GenericDevice(GenericDeviceType::VideoCapture);
		}
		if ((devicetypestring_alt == "scsi_target")
			|| (devicesubsystem == "scsi_host")
			|| (devicesubsystem == "scsi_disk")
			|| (devicesubsystem == "scsi_device")
			|| (devicesubsystem == "scsi_generic")
			|| (devicesubsystem == "scsi")
			|| (devicetypestring_alt == "sas_target")
			|| (devicesubsystem == "sas_host")
			|| (devicesubsystem == "sas_port")
			|| (devicesubsystem == "sas_device")
			|| (devicesubsystem == "sas_generic")
			|| (devicesubsystem == "sas_phy")
			|| (devicesubsystem == "sas_end_device")
			|| (devicesubsystem == "spi_transport")
			|| (devicesubsystem == "spi_host")
			|| (devicesubsystem == "ata_port")
			|| (devicesubsystem == "ata_link")
			|| (devicesubsystem == "ata_disk")
			|| (devicesubsystem == "ata_device")
			|| (devicesubsystem == "ata")) {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		if (devicesubsystem == "infiniband") {
			if (!device) device = new GenericDevice(GenericDeviceType::Peripheral);
		}
		if ((devicesubsystem == "infiniband_cm")
			|| (devicesubsystem == "infiniband_mad")) {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		if ((devicesubsystem == "enclosure")
			|| (devicesubsystem == "clocksource")
			|| (devicesubsystem == "amba")) {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		if (devicesubsystem == "ipmi_si") {
			if (!device) device = new GenericDevice(GenericDeviceType::Mainboard);
		}
		if (devicesubsystem == "misc") {
			if (devicedriver.startsWith("tpm_")) {
				if (!device) device = new GenericDevice(GenericDeviceType::Cryptography);
			}
		}
		if (devicesubsystem == "leds") {
			if (!device) device = new GenericDevice(GenericDeviceType::OtherACPI);
		}
		if (devicesubsystem == "net") {
			if (!device) device = new NetworkDevice(GenericDeviceType::Network);
		}
		if ((devicesubsystem == "i2c")
			|| (devicesubsystem == "i2c-dev")) {
			if (!device) device = new GenericDevice(GenericDeviceType::I2C);
		}
		if (devicesubsystem == "mdio_bus") {
			if (!device) device = new GenericDevice(GenericDeviceType::MDIO);
		}
		if (devicesubsystem == "graphics") {
			if (devicenode.isNull()) {	// GPUs do not have associated device nodes
				if (!device) device = new GenericDevice(GenericDeviceType::GPU);
			}
			else {
				if (!device) device = new GenericDevice(GenericDeviceType::Platform);
			}
		}
		if (devicesubsystem == "tifm_adapter") {
			if (!device) device = new GenericDevice(GenericDeviceType::StorageController);
		}
		if (devicesubsystem == "mmc_host") {
			if (!device) device = new GenericDevice(GenericDeviceType::StorageController);
		}
		if (devicesubsystem == "mmc") {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		if ((devicesubsystem == "event_source")
			|| (devicesubsystem == "rtc")) {
			if (!device) device = new GenericDevice(GenericDeviceType::Mainboard);
		}
		if (devicesubsystem == "bsg") {
			if (!device) device = new GenericDevice(GenericDeviceType::SCSI);
		}
		if (devicesubsystem == "firewire") {
			if (!device) device = new GenericDevice(GenericDeviceType::IEEE1394);
		}
		if (devicesubsystem == "drm") {
			if (devicenode.isNull()) {	// Monitors do not have associated device nodes
				if (!device) device = new MonitorDevice(GenericDeviceType::Monitor);
			}
			else {
				if (!device) device = new GenericDevice(GenericDeviceType::Platform);
			}
		}
		if (devicesubsystem == "serio") {
			if (devicedriver.contains("atkbd")) {
				if (!device) device = new GenericDevice(GenericDeviceType::Keyboard);
			}
			else if (devicedriver.contains("mouse")) {
				if (!device) device = new GenericDevice(GenericDeviceType::Mouse);
			}
			else {
				if (!device) device = new GenericDevice(GenericDeviceType::Serial);
			}
		}
		if (devicesubsystem == "ppdev") {
			if (!device) device = new GenericDevice(GenericDeviceType::Parallel);
		}
		if (devicesubsystem == "printer") {
			if (!device) device = new GenericDevice(GenericDeviceType::Printer);
		}
		if (devicesubsystem == "bridge") {
			if (!device) device = new GenericDevice(GenericDeviceType::Bridge);
		}
		if ((devicesubsystem == "pci_bus")
			|| (devicesubsystem == "pci_express")) {
			if (!device) device = new GenericDevice(GenericDeviceType::Bus);
		}
		if (devicesubsystem == "pcmcia_socket") {
			if (!device) device = new GenericDevice(GenericDeviceType::PCMCIA);
		}
		if (devicesubsystem == "platform") {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		if (devicesubsystem == "ieee80211") {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		if (devicesubsystem == "rfkill") {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		if (devicesubsystem == "machinecheck") {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
		if (devicesubsystem == "pnp") {
			if (!device) device = new GenericDevice(GenericDeviceType::PNP);
		}
		if ((devicesubsystem == "hid")
			|| (devicesubsystem == "hidraw")
			|| (devicesubsystem == "usbhid")) {
			if (!device) device = new GenericDevice(GenericDeviceType::HID);
		}
		if (devicesubsystem == "power_supply") {
			TQString powersupplyname(udev_device_get_property_value(dev, "POWER_SUPPLY_NAME"));
			if (powersupplyname.upper().startsWith("AC")) {
				if (!device) device = new MainsPowerDevice(GenericDeviceType::PowerSupply);
			}
			else {
				if (!device) device = new BatteryDevice(GenericDeviceType::Battery);
			}
		}

		// Moderate accuracy classification, if PCI device class is available
		// See http://www.acm.uiuc.edu/sigops/roll_your_own/7.c.1.html for codes and meanings
		if (!devicepciclass.isNull()) {
			// Pre PCI 2.0
			if (devicepciclass.startsWith("0001")) {
				if (devicenode.isNull()) {	// GPUs do not have associated device nodes
					if (!device) device = new GenericDevice(GenericDeviceType::GPU);
				}
				else {
					if (!device) device = new GenericDevice(GenericDeviceType::Platform);
				}
			}
			// Post PCI 2.0
			TQString devicepcisubclass = devicepciclass;
			devicepcisubclass = devicepcisubclass.remove(0,2);
			if (devicepciclass.startsWith("01")) {
				if (!device) device = new GenericDevice(GenericDeviceType::StorageController);
			}
			if (devicepciclass.startsWith("02")) {
				if (!device) device = new NetworkDevice(GenericDeviceType::Network);
			}
			if (devicepciclass.startsWith("03")) {
				if (devicenode.isNull()) {	// GPUs do not have associated device nodes
					if (!device) device = new GenericDevice(GenericDeviceType::GPU);
				}
				else {
					if (!device) device = new GenericDevice(GenericDeviceType::Platform);
				}
			}
			if (devicepciclass.startsWith("04")) {
				if (!device) device = new GenericDevice(GenericDeviceType::OtherMultimedia);
			}
			if (devicepciclass.startsWith("05")) {
				if (!device) device = new GenericDevice(GenericDeviceType::RAM);
			}
			if (devicepciclass.startsWith("06")) {
				if (!device) device = new GenericDevice(GenericDeviceType::Bridge);
			}
			if (devicepciclass.startsWith("07")) {
				if (devicepcisubclass.startsWith("03")) {
					if (!device) device = new GenericDevice(GenericDeviceType::Modem);
				}
			}
			if (devicepciclass.startsWith("0a")) {
				if (!device) device = new GenericDevice(GenericDeviceType::Dock);
			}
			if (devicepciclass.startsWith("0b")) {
				if (!device) device = new GenericDevice(GenericDeviceType::CPU);
			}
			if (devicepciclass.startsWith("0c")) {
				if (!device) device = new GenericDevice(GenericDeviceType::Serial);
			}
		}

		// Last ditch attempt at classification
		// Likely inaccurate and sweeping
		if ((devicesubsystem == "usb")
			|| (devicesubsystem == "usb_device")
			|| (devicesubsystem == "usbmon")) {
				// Get USB interface class for further classification
				int usbInterfaceClass = -1;
				{
					TQFile ifaceprotofile(current_path + "/bInterfaceClass");
					if (ifaceprotofile.open(IO_ReadOnly)) {
						TQTextStream stream( &ifaceprotofile );
						usbInterfaceClass = stream.readLine().toUInt();
						ifaceprotofile.close();
					}
				}
				// Get USB interface subclass for further classification
				int usbInterfaceSubClass = -1;
				{
					TQFile ifaceprotofile(current_path + "/bInterfaceSubClass");
					if (ifaceprotofile.open(IO_ReadOnly)) {
						TQTextStream stream( &ifaceprotofile );
						usbInterfaceSubClass = stream.readLine().toUInt();
						ifaceprotofile.close();
					}
				}
				// Get USB interface protocol for further classification
				int usbInterfaceProtocol = -1;
				{
					TQFile ifaceprotofile(current_path + "/bInterfaceProtocol");
					if (ifaceprotofile.open(IO_ReadOnly)) {
						TQTextStream stream( &ifaceprotofile );
						usbInterfaceProtocol = stream.readLine().toUInt();
						ifaceprotofile.close();
					}
				}
				if ((usbInterfaceClass == 6) && (usbInterfaceSubClass == 1) && (usbInterfaceProtocol == 1)) {
					// PictBridge
					if (!device) {
						device = new StorageDevice(GenericDeviceType::Disk);
						StorageDevice* sdevice = static_cast<StorageDevice*>(device);
						sdevice->internalSetDiskType(DiskDeviceType::Camera);
						TQString parentsyspathudev = systempath;
						parentsyspathudev.truncate(parentsyspathudev.length()-1);	// Remove trailing slash
						parentsyspathudev.truncate(parentsyspathudev.findRev("/"));
						struct udev_device *parentdev;
						parentdev = udev_device_new_from_syspath(m_udevStruct, parentsyspathudev.ascii());
						devicenode = (udev_device_get_devnode(parentdev));
					}
				}
				else {
					if (!device) device = new GenericDevice(GenericDeviceType::OtherUSB);
				}
		}
		if (devicesubsystem == "pci") {
			if (!device) device = new GenericDevice(GenericDeviceType::OtherPeripheral);
		}
		if (devicesubsystem == "cpu") {
			if (!device) device = new GenericDevice(GenericDeviceType::Platform);
		}
	}

	if (device == 0) {
		// Unhandled
		if (!device) device = new GenericDevice(GenericDeviceType::Other);
		printf("[FIXME] UNCLASSIFIED DEVICE name: %s type: %s subsystem: %s driver: %s [Node Path: %s] [Syspath: %s] [%s:%s]\n", devicename.ascii(), devicetype.ascii(), devicesubsystem.ascii(), devicedriver.ascii(), devicenode.ascii(), udev_device_get_syspath(dev), devicevendorid.ascii(), devicemodelid.ascii()); fflush(stdout);
	}

	// Root devices are special
	if ((device->type() == GenericDeviceType::Root) || (device->type() == GenericDeviceType::RootSystem)) {
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

	// Internal use only!
	device->m_udevtype = devicetype;
	device->m_udevdevicetypestring = devicetypestring;
	device->udevdevicetypestring_alt = devicetypestring_alt;

	updateExistingDeviceInformation(device, dev);

	if (temp_udev_device) {
		udev_device_unref(dev);
	}

	return device;
}

void HardwareDevices::updateExistingDeviceInformation(GenericDevice* existingdevice, udev_device* dev) {
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
	GenericDevice* device = existingdevice;
	bool temp_udev_device = !dev;

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

	if (!dev) {
		TQString syspathudev = systempath;
		syspathudev.truncate(syspathudev.length()-1);	// Remove trailing slash
		dev = udev_device_new_from_syspath(m_udevStruct, syspathudev.ascii());
	}

	if (device->type() == GenericDeviceType::Disk) {
		StorageDevice* sdevice = static_cast<StorageDevice*>(device);
		if (sdevice->diskType() & DiskDeviceType::Camera) {
			// PictBridge cameras are special and should not be classified by standard rules
			sdevice->internalSetDiskStatus(DiskDeviceStatus::Removable);
			sdevice->internalSetFileSystemName("pictbridge");
		}
		else {
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
	
			DiskDeviceType::DiskDeviceType disktype = sdevice->diskType();
			DiskDeviceStatus::DiskDeviceStatus diskstatus = DiskDeviceStatus::Null;
	
			disktype = classifyDiskType(dev, devicenode, devicebus, devicetypestring, systempath, devicevendor, devicemodel, filesystemtype, devicedriver);
			sdevice->internalSetDiskType(disktype);
			device = classifyUnknownDeviceByExternalRules(dev, device, true);	// Check external rules for possible subtype overrides
			disktype = sdevice->diskType();						// The type can be overridden by an external rule
	
			if ((disktype & DiskDeviceType::CDROM)
				|| (disktype & DiskDeviceType::CDR)
				|| (disktype & DiskDeviceType::CDRW)
				|| (disktype & DiskDeviceType::CDMO)
				|| (disktype & DiskDeviceType::CDMRRW)
				|| (disktype & DiskDeviceType::CDMRRWW)
				|| (disktype & DiskDeviceType::DVDROM)
				|| (disktype & DiskDeviceType::DVDRAM)
				|| (disktype & DiskDeviceType::DVDR)
				|| (disktype & DiskDeviceType::DVDRW)
				|| (disktype & DiskDeviceType::DVDRDL)
				|| (disktype & DiskDeviceType::DVDRWDL)
				|| (disktype & DiskDeviceType::DVDPLUSR)
				|| (disktype & DiskDeviceType::DVDPLUSRW)
				|| (disktype & DiskDeviceType::DVDPLUSRDL)
				|| (disktype & DiskDeviceType::DVDPLUSRWDL)
				|| (disktype & DiskDeviceType::BDROM)
				|| (disktype & DiskDeviceType::BDR)
				|| (disktype & DiskDeviceType::BDRW)
				|| (disktype & DiskDeviceType::HDDVDROM)
				|| (disktype & DiskDeviceType::HDDVDR)
				|| (disktype & DiskDeviceType::HDDVDRW)
				|| (disktype & DiskDeviceType::CDAudio)
				|| (disktype & DiskDeviceType::CDVideo)
				|| (disktype & DiskDeviceType::DVDVideo)
				|| (disktype & DiskDeviceType::BDVideo)
				) {
				// These drives are guaranteed to be optical
				disktype = disktype | DiskDeviceType::Optical;
			}
	
			if (disktype & DiskDeviceType::Floppy) {
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
					disktype = disktype & ~DiskDeviceType::Floppy;
				}
			}
	
			if (devicetypestring.upper() == "CD") {
				if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_STATE")).upper() == "BLANK") {
					diskstatus = diskstatus | DiskDeviceStatus::Blank;
				}
				sdevice->internalSetMediaInserted((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) != ""));
			}
	
			if (disktype & DiskDeviceType::Zip) {
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
				diskstatus = diskstatus | DiskDeviceStatus::Removable;
			}
			if (hotpluggable) {
				diskstatus = diskstatus | DiskDeviceStatus::Hotpluggable;
			}
	
			if ((filesystemtype.upper() != "CRYPTO_LUKS") && (filesystemtype.upper() != "CRYPTO") && (filesystemtype.upper() != "SWAP") && (!filesystemtype.isNull())) {
				diskstatus = diskstatus | DiskDeviceStatus::ContainsFilesystem;
			}
	
			// Set mountable flag if device is likely to be mountable
			diskstatus = diskstatus | DiskDeviceStatus::Mountable;
			if ((devicetypestring.upper().isNull()) && (disktype & DiskDeviceType::HDD)) {
				diskstatus = diskstatus & ~DiskDeviceStatus::Mountable;
			}
			if (removable) {
				if (sdevice->mediaInserted()) {
					diskstatus = diskstatus | DiskDeviceStatus::Inserted;
				}
				else {
					diskstatus = diskstatus & ~DiskDeviceStatus::Mountable;
				}
			}
			// Swap partitions cannot be mounted
			if (filesystemtype.upper() == "SWAP") {
				diskstatus = diskstatus & ~DiskDeviceStatus::Mountable;
			}
			// If certain disk types do not report the presence of a filesystem, they are likely not mountable
			if ((disktype & DiskDeviceType::HDD) || (disktype & DiskDeviceType::Optical)) {
				if (!(diskstatus & DiskDeviceStatus::ContainsFilesystem)) {
					diskstatus = diskstatus & ~DiskDeviceStatus::Mountable;
				}
			}
	
			if (holdingDeviceNodes.count() > 0) {
				diskstatus = diskstatus | DiskDeviceStatus::UsedByDevice;
			}
	
			if (slaveDeviceNodes.count() > 0) {
				diskstatus = diskstatus | DiskDeviceStatus::UsesDevice;
			}
	
			// See if any slaves were crypted
			for ( TQStringList::Iterator slaveit = slaveDeviceNodes.begin(); slaveit != slaveDeviceNodes.end(); ++slaveit ) {
				struct udev_device *slavedev;
				slavedev = udev_device_new_from_syspath(m_udevStruct, (*slaveit).ascii());
				TQString slavediskfstype(udev_device_get_property_value(slavedev, "ID_FS_TYPE"));
				if ((slavediskfstype.upper() == "CRYPTO_LUKS") || (slavediskfstype.upper() == "CRYPTO")) {
					disktype = disktype | DiskDeviceType::UnlockedCrypt;
					// Set disk type based on parent device
					disktype = disktype | classifyDiskType(slavedev, devicenode, TQString(udev_device_get_property_value(dev, "ID_BUS")), TQString(udev_device_get_property_value(dev, "ID_TYPE")), (*slaveit), TQString(udev_device_get_property_value(dev, "ID_VENDOR")), TQString(udev_device_get_property_value(dev, "ID_MODEL")), TQString(udev_device_get_property_value(dev, "ID_FS_TYPE")), TQString(udev_device_get_driver(dev)));
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
			if ((sdevice->isDiskOfType(DiskDeviceType::CDROM))
				|| (sdevice->isDiskOfType(DiskDeviceType::CDR))
				|| (sdevice->isDiskOfType(DiskDeviceType::CDRW))
				|| (sdevice->isDiskOfType(DiskDeviceType::CDMO))
				|| (sdevice->isDiskOfType(DiskDeviceType::CDMRRW))
				|| (sdevice->isDiskOfType(DiskDeviceType::CDMRRWW))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDROM))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDRAM))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDR))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDRW))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDRDL))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDRWDL))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDPLUSR))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDPLUSRW))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDPLUSRDL))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDPLUSRWDL))
				|| (sdevice->isDiskOfType(DiskDeviceType::BDROM))
				|| (sdevice->isDiskOfType(DiskDeviceType::BDR))
				|| (sdevice->isDiskOfType(DiskDeviceType::BDRW))
				|| (sdevice->isDiskOfType(DiskDeviceType::HDDVDROM))
				|| (sdevice->isDiskOfType(DiskDeviceType::HDDVDR))
				|| (sdevice->isDiskOfType(DiskDeviceType::HDDVDRW))
				|| (sdevice->isDiskOfType(DiskDeviceType::CDAudio))
				|| (sdevice->isDiskOfType(DiskDeviceType::CDVideo))
				|| (sdevice->isDiskOfType(DiskDeviceType::DVDVideo))
				|| (sdevice->isDiskOfType(DiskDeviceType::BDVideo))
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
	}

	if (device->type() == GenericDeviceType::Network) {
		// Network devices don't have devices nodes per se, but we can at least return the Linux network name...
		TQString potentialdevicenode = systempath;
		if (potentialdevicenode.endsWith("/")) potentialdevicenode.truncate(potentialdevicenode.length()-1);
		potentialdevicenode.remove(0, potentialdevicenode.findRev("/")+1);
		TQString potentialparentnode = systempath;
		if (potentialparentnode.endsWith("/")) potentialparentnode.truncate(potentialparentnode.length()-1);
		potentialparentnode.remove(0, potentialparentnode.findRev("/", potentialparentnode.findRev("/")-1)+1);
		if (potentialparentnode.startsWith("net/")) {
			devicenode = potentialdevicenode;
		}

		if (devicenode.isNull()) {
			// Platform device, not a physical device
			// HACK
			// This only works because devices of type Platform only access the GenericDevice class!
			device->m_deviceType = GenericDeviceType::Platform;
		}
		else {
			// Gather network device information
			NetworkDevice* ndevice = dynamic_cast<NetworkDevice*>(device);
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

	if ((device->type() == GenericDeviceType::OtherSensor) || (device->type() == GenericDeviceType::ThermalSensor)) {
		// Populate all sensor values
		SensorClusterMap sensors;
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

		SensorDevice* sdevice = dynamic_cast<SensorDevice*>(device);
		sdevice->internalSetValues(sensors);
	}

	if (device->type() == GenericDeviceType::Battery) {
		// Populate all battery values
		BatteryDevice* bdevice = dynamic_cast<BatteryDevice*>(device);
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
					if (nodename == "charge_full" || nodename == "energy_full") {
						bdevice->internalSetMaximumEnergy(line.toDouble()/1000000.0);
					}
					if (nodename == "charge_full_design" || nodename == "energy_full_design") {
						bdevice->internalSetMaximumDesignEnergy(line.toDouble()/1000000.0);
					}
					if (nodename == "charge_now" || nodename == "energy_now") {
						bdevice->internalSetEnergy(line.toDouble()/1000000.0);
					}
					if (nodename == "manufacturer") {
						bdevice->internalSetVendorName(line.stripWhiteSpace());
					}
					if (nodename == "model_name") {
						bdevice->internalSetVendorModel(line.stripWhiteSpace());
					}
					if (nodename == "power_now" || nodename == "current_now") {
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

		// Calculate time remaining
		// Discharge rate is in watt-hours
		// Energy is in watt-hours
		// Therefore, energy/rate = time in hours
		// Convert to seconds...
		bdevice->internalSetTimeRemaining((bdevice->energy()/bdevice->dischargeRate())*60*60);
	}

	if (device->type() == GenericDeviceType::PowerSupply) {
		// Populate all power supply values
		MainsPowerDevice* pdevice = dynamic_cast<MainsPowerDevice*>(device);
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

	if (device->type() == GenericDeviceType::Backlight) {
		// Populate all backlight values
		BacklightDevice* bdevice = dynamic_cast<BacklightDevice*>(device);
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
						DisplayPowerLevel::DisplayPowerLevel pl = DisplayPowerLevel::On;
						int rpl = line.toInt();
						if (rpl == FB_BLANK_UNBLANK) {
							pl = DisplayPowerLevel::On;
						}
						else if (rpl == FB_BLANK_POWERDOWN) {
							pl = DisplayPowerLevel::Off;
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

	if (device->type() == GenericDeviceType::Monitor) {
		MonitorDevice* mdevice = dynamic_cast<MonitorDevice*>(device);
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
						ResolutionList resolutions;
						resolutions.clear();
						for (TQStringList::Iterator it = resolutionsStringList.begin(); it != resolutionsStringList.end(); ++it) {
							resinfo = TQStringList::split('X', *it, true);
							resolutions.append(ResolutionPair((*(resinfo.at(0))).toUInt(), (*(resinfo.at(1))).toUInt()));
						}
						mdevice->internalSetResolutions(resolutions);
					}
					if (nodename == "dpms") {
						DisplayPowerLevel::DisplayPowerLevel pl = DisplayPowerLevel::On;
						if (line == "On") {
							pl = DisplayPowerLevel::On;
						}
						else if (line == "Standby") {
							pl = DisplayPowerLevel::Standby;
						}
						else if (line == "Suspend") {
							pl = DisplayPowerLevel::Suspend;
						}
						else if (line == "Off") {
							pl = DisplayPowerLevel::Off;
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
			else {
				mdevice->m_friendlyName = i18n("Generic %1 Device").arg(genericPortName);
			}
			mdevice->internalSetEdid(getEDID(mdevice->systemPath()));
		}
		else {
			mdevice->m_friendlyName = i18n("Disconnected %1 Port").arg(genericPortName);
			mdevice->internalSetEdid(TQByteArray());
			mdevice->internalSetResolutions(ResolutionList());
		}

		// FIXME
		// Much of the code in libtderandr should be integrated into/interfaced with this library
	}

	if (device->type() == GenericDeviceType::RootSystem) {
		// Try to obtain as much generic information about this system as possible
		RootSystemDevice* rdevice = dynamic_cast<RootSystemDevice*>(device);

		// Guess at my form factor
		// dmidecode would tell me this, but is somewhat unreliable
		SystemFormFactor::SystemFormFactor formfactor = SystemFormFactor::Desktop;
		if (listByDeviceClass(GenericDeviceType::Backlight).count() > 0) {	// Is this really a good way to determine if a machine is a laptop?
			formfactor = SystemFormFactor::Laptop;
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
						SystemPowerStateList powerstates;
						// Always assume that these two fully on/fully off states are available
						powerstates.append(SystemPowerState::Active);
						powerstates.append(SystemPowerState::PowerOff);
						if (line.contains("standby")) {
							powerstates.append(SystemPowerState::Standby);
						}
						if (line.contains("mem")) {
							powerstates.append(SystemPowerState::Suspend);
						}
						if (line.contains("disk")) {
							powerstates.append(SystemPowerState::Hibernate);
						}
						rdevice->internalSetPowerStates(powerstates);
					}
					if (nodename == "disk") {
						// Get list of available hibernation methods
						SystemHibernationMethodList hibernationmethods;
						if (line.contains("platform")) {
							hibernationmethods.append(SystemHibernationMethod::Platform);
						}
						if (line.contains("shutdown")) {
							hibernationmethods.append(SystemHibernationMethod::Shutdown);
						}
						if (line.contains("reboot")) {
							hibernationmethods.append(SystemHibernationMethod::Reboot);
						}
						if (line.contains("testproc")) {
							hibernationmethods.append(SystemHibernationMethod::TestProc);
						}
						if (line.contains("test")) {
							hibernationmethods.append(SystemHibernationMethod::Test);
						}
						rdevice->internalSetHibernationMethods(hibernationmethods);

						// Get current hibernation method
						line.truncate(line.findRev("]"));
						line.remove(0, line.findRev("[")+1);
						SystemHibernationMethod::SystemHibernationMethod hibernationmethod = SystemHibernationMethod::Unsupported;
						if (line.contains("platform")) {
							hibernationmethod = SystemHibernationMethod::Platform;
						}
						if (line.contains("shutdown")) {
							hibernationmethod = SystemHibernationMethod::Shutdown;
						}
						if (line.contains("reboot")) {
							hibernationmethod = SystemHibernationMethod::Reboot;
						}
						if (line.contains("testproc")) {
							hibernationmethod = SystemHibernationMethod::TestProc;
						}
						if (line.contains("test")) {
							hibernationmethod = SystemHibernationMethod::Test;
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

	if (device->type() == GenericDeviceType::Event) {
		// Try to obtain as much type information about this event device as possible
		EventDevice* edevice = dynamic_cast<EventDevice*>(device);
		if (edevice->systemPath().contains("PNP0C0D")) {
			edevice->internalSetEventType(EventDeviceType::ACPILidSwitch);
		}
		else if (edevice->systemPath().contains("PNP0C0E") || edevice->systemPath().contains("/LNXSLPBN")) {
			edevice->internalSetEventType(EventDeviceType::ACPISleepButton);
		}
		else if (edevice->systemPath().contains("PNP0C0C") || edevice->systemPath().contains("/LNXPWRBN")) {
			edevice->internalSetEventType(EventDeviceType::ACPIPowerButton);
		}
		else if (edevice->systemPath().contains("_acpi")) {
			edevice->internalSetEventType(EventDeviceType::ACPIOtherInput);
		}
		else {
			edevice->internalSetEventType(EventDeviceType::Unknown);
		}
	}

	if (device->type() == GenericDeviceType::Input) {
		// Try to obtain as much type information about this input device as possible
		InputDevice* idevice = dynamic_cast<InputDevice*>(device);
		if (idevice->systemPath().contains("PNP0C0D")) {
			idevice->internalSetInputType(InputDeviceType::ACPILidSwitch);
		}
		else if (idevice->systemPath().contains("PNP0C0E") || idevice->systemPath().contains("/LNXSLPBN")) {
			idevice->internalSetInputType(InputDeviceType::ACPISleepButton);
		}
		else if (idevice->systemPath().contains("PNP0C0C") || idevice->systemPath().contains("/LNXPWRBN")) {
			idevice->internalSetInputType(InputDeviceType::ACPIPowerButton);
		}
		else if (idevice->systemPath().contains("_acpi")) {
			idevice->internalSetInputType(InputDeviceType::ACPIOtherInput);
		}
		else {
			idevice->internalSetInputType(InputDeviceType::Unknown);
		}
	}

	if (device->type() == GenericDeviceType::Event) {
		// Try to obtain as much specific information about this event device as possible
		EventDevice* edevice = dynamic_cast<EventDevice*>(device);
		int r;
		char switches[SW_CNT];

		// Figure out which switch types are supported, if any
		SwitchType::SwitchType supportedSwitches = SwitchType::Null;
		if (edevice->m_fd < 0) {
			edevice->m_fd = open(edevice->deviceNode().ascii(), O_RDONLY);
		}
		r = ioctl(edevice->m_fd, EVIOCGBIT(EV_SW, sizeof(switches)), switches);
		if (r > 0) {
			if (BIT_IS_SET(switches, SW_LID)) {
				supportedSwitches = supportedSwitches | SwitchType::Lid;
			}
			if (BIT_IS_SET(switches, SW_TABLET_MODE)) {
				supportedSwitches = supportedSwitches | SwitchType::TabletMode;
			}
			if (BIT_IS_SET(switches, SW_RFKILL_ALL)) {
				supportedSwitches = supportedSwitches | SwitchType::RFKill;
			}
			if (BIT_IS_SET(switches, SW_RADIO)) {
				supportedSwitches = supportedSwitches | SwitchType::Radio;
			}
			if (BIT_IS_SET(switches, SW_MICROPHONE_INSERT)) {
				supportedSwitches = supportedSwitches | SwitchType::MicrophoneInsert;
			}
			if (BIT_IS_SET(switches, SW_DOCK)) {
				supportedSwitches = supportedSwitches | SwitchType::Dock;
			}
			if (BIT_IS_SET(switches, SW_LINEOUT_INSERT)) {
				supportedSwitches = supportedSwitches | SwitchType::LineOutInsert;
			}
			if (BIT_IS_SET(switches, SW_JACK_PHYSICAL_INSERT)) {
				supportedSwitches = supportedSwitches | SwitchType::JackPhysicalInsert;
			}
			if (BIT_IS_SET(switches, SW_VIDEOOUT_INSERT)) {
				supportedSwitches = supportedSwitches | SwitchType::VideoOutInsert;
			}
#if 0	// Some old kernels don't provide these defines... [FIXME]
			if (BIT_IS_SET(switches, SW_CAMERA_LENS_COVER)) {
				supportedSwitches = supportedSwitches | SwitchType::CameraLensCover;
			}
			if (BIT_IS_SET(switches, SW_KEYPAD_SLIDE)) {
				supportedSwitches = supportedSwitches | SwitchType::KeypadSlide;
			}
			if (BIT_IS_SET(switches, SW_FRONT_PROXIMITY)) {
				supportedSwitches = supportedSwitches | SwitchType::FrontProximity;
			}
			if (BIT_IS_SET(switches, SW_ROTATE_LOCK)) {
				supportedSwitches = supportedSwitches | SwitchType::RotateLock;
			}
			if (BIT_IS_SET(switches, SW_LINEIN_INSERT)) {
				supportedSwitches = supportedSwitches | SwitchType::LineInInsert;
			}
#endif
			// Keep in sync with ACPI Event/Input identification routines above
			if (edevice->systemPath().contains("PNP0C0D")) {
				supportedSwitches = supportedSwitches | SwitchType::Lid;
			}
			if (edevice->systemPath().contains("PNP0C0E") || edevice->systemPath().contains("/LNXSLPBN")) {
				supportedSwitches = supportedSwitches | SwitchType::SleepButton;
			}
			if (edevice->systemPath().contains("PNP0C0C") || edevice->systemPath().contains("/LNXPWRBN")) {
				supportedSwitches = supportedSwitches | SwitchType::PowerButton;
			}
		}
		edevice->internalSetProvidedSwitches(supportedSwitches);

		// Figure out which switch types are active, if any
		SwitchType::SwitchType activeSwitches = SwitchType::Null;
		r = ioctl(edevice->m_fd, EVIOCGSW(sizeof(switches)), switches);
		if (r > 0) {
			if (BIT_IS_SET(switches, SW_LID)) {
				activeSwitches = activeSwitches | SwitchType::Lid;
			}
			if (BIT_IS_SET(switches, SW_TABLET_MODE)) {
				activeSwitches = activeSwitches | SwitchType::TabletMode;
			}
			if (BIT_IS_SET(switches, SW_RFKILL_ALL)) {
				activeSwitches = activeSwitches | SwitchType::RFKill;
			}
			if (BIT_IS_SET(switches, SW_RADIO)) {
				activeSwitches = activeSwitches | SwitchType::Radio;
			}
			if (BIT_IS_SET(switches, SW_MICROPHONE_INSERT)) {
				activeSwitches = activeSwitches | SwitchType::MicrophoneInsert;
			}
			if (BIT_IS_SET(switches, SW_DOCK)) {
				activeSwitches = activeSwitches | SwitchType::Dock;
			}
			if (BIT_IS_SET(switches, SW_LINEOUT_INSERT)) {
				activeSwitches = activeSwitches | SwitchType::LineOutInsert;
			}
			if (BIT_IS_SET(switches, SW_JACK_PHYSICAL_INSERT)) {
				activeSwitches = activeSwitches | SwitchType::JackPhysicalInsert;
			}
			if (BIT_IS_SET(switches, SW_VIDEOOUT_INSERT)) {
				activeSwitches = activeSwitches | SwitchType::VideoOutInsert;
			}
#if 0	// Some old kernels don't provide these defines... [FIXME]
			if (BIT_IS_SET(switches, SW_CAMERA_LENS_COVER)) {
				activeSwitches = activeSwitches | SwitchType::CameraLensCover;
			}
			if (BIT_IS_SET(switches, SW_KEYPAD_SLIDE)) {
				activeSwitches = activeSwitches | SwitchType::KeypadSlide;
			}
			if (BIT_IS_SET(switches, SW_FRONT_PROXIMITY)) {
				activeSwitches = activeSwitches | SwitchType::FrontProximity;
			}
			if (BIT_IS_SET(switches, SW_ROTATE_LOCK)) {
				activeSwitches = activeSwitches | SwitchType::RotateLock;
			}
			if (BIT_IS_SET(switches, SW_LINEIN_INSERT)) {
				activeSwitches = activeSwitches | SwitchType::LineInInsert;
			}
#endif
		}
		edevice->internalSetActiveSwitches(activeSwitches);

		edevice->internalStartFdMonitoring(this);
	}

	// Root devices are still special
	if ((device->type() == GenericDeviceType::Root) || (device->type() == GenericDeviceType::RootSystem)) {
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

	if (temp_udev_device) {
		udev_device_unref(dev);
	}
}

void HardwareDevices::updateBlacklists(GenericDevice* hwdevice, udev_device* dev) {
	// HACK
	// I am lucky enough to have a Flash drive that spams udev continually with device change events
	// I imagine I am not the only one, so here is a section in which specific devices can be blacklisted!

	// For "U3 System" fake CD
	if ((hwdevice->vendorID() == "08ec") && (hwdevice->modelID() == "0020") && (TQString(udev_device_get_property_value(dev, "ID_TYPE")) == "cd")) {
		hwdevice->internalSetBlacklistedForUpdate(true);
	}
}

bool HardwareDevices::queryHardwareInformation() {
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

		GenericDevice* device = classifyUnknownDevice(dev);

		// Make sure this device is not a duplicate
		GenericDevice *hwdevice;
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

	emit hardwareEvent(HardwareEvent::HardwareListModified, TQString());

	return true;
}

void HardwareDevices::updateParentDeviceInformation(GenericDevice* hwdevice) {
	// Scan for the first path up the sysfs tree that is available in the main hardware table
	bool done = false;
	TQString current_path = hwdevice->systemPath();
	GenericDevice* parentdevice = 0;

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

void HardwareDevices::updateParentDeviceInformation() {
	GenericDevice *hwdevice;

	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	GenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		updateParentDeviceInformation(hwdevice);
	}
}

void HardwareDevices::addCoreSystemDevices() {
	GenericDevice *hwdevice;

	// Add the Main Root System Device, which provides all other devices
	hwdevice = new RootSystemDevice(GenericDeviceType::RootSystem);
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
				hwdevice = new GenericDevice(GenericDeviceType::Root);
				hwdevice->internalSetSystemPath(dirfi->absFilePath());
				m_deviceList.append(hwdevice);
			}
			++devicesdirit;
		}
	}

	// Handle CPUs, which are currently handled terribly by udev
	// Parse /proc/cpuinfo to extract some information about the CPUs
	hwdevice = 0;
	TQDir d("/sys/devices/system/cpu/");
	d.setFilter( TQDir::Dirs );
	const TQFileInfoList *list = d.entryInfoList();
	TQFileInfoListIterator it( *list );
	TQFileInfo *fi;
	while ((fi = it.current()) != 0) {
		TQString directoryName = fi->fileName();
		if (directoryName.startsWith("cpu")) {
			directoryName = directoryName.remove(0,3);
			bool isInt;
			int processorNumber = directoryName.toUInt(&isInt, 10);
			if (isInt) {
				hwdevice = new CPUDevice(GenericDeviceType::CPU);
				hwdevice->internalSetSystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber));
				m_deviceList.append(hwdevice);
			}
		}
		++it;
	}

	// Populate CPU information
	processModifiedCPUs();
}

TQString HardwareDevices::findPCIDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid) {
	TQString vendorName = TQString::null;
	TQString modelName = TQString::null;
	TQString friendlyName = TQString::null;

	if (!pci_id_map) {
		pci_id_map = new DeviceIDMap;

		TQString database_filename = "/usr/share/pci.ids";
		if (!TQFile::exists(database_filename)) {
			database_filename = "/usr/share/misc/pci.ids";
		}
		if (!TQFile::exists(database_filename)) {
			printf("[tdehardwaredevices] Unable to locate PCI information database pci.ids\n"); fflush(stdout);
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
			printf("[tdehardwaredevices] Unable to open PCI information database %s\n", database_filename.ascii()); fflush(stdout);
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

TQString HardwareDevices::findUSBDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid) {
	TQString vendorName = TQString::null;
	TQString modelName = TQString::null;
	TQString friendlyName = TQString::null;

	if (!usb_id_map) {
		usb_id_map = new DeviceIDMap;

		TQString database_filename = "/usr/share/usb.ids";
		if (!TQFile::exists(database_filename)) {
			database_filename = "/usr/share/misc/usb.ids";
		}
		if (!TQFile::exists(database_filename)) {
			printf("[tdehardwaredevices] Unable to locate USB information database usb.ids\n"); fflush(stdout);
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
			printf("[tdehardwaredevices] Unable to open USB information database %s\n", database_filename.ascii()); fflush(stdout);
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

TQString HardwareDevices::findPNPDeviceName(TQString pnpid) {
	TQString friendlyName = TQString::null;

	if (!pnp_id_map) {
		pnp_id_map = new DeviceIDMap;

		TQStringList hardware_info_directories(TDEGlobal::dirs()->resourceDirs("data"));
		TQString hardware_info_directory_suffix("tdehwlib/pnpdev/");
		TQString hardware_info_directory;
		TQString database_filename;

		for ( TQStringList::Iterator it = hardware_info_directories.begin(); it != hardware_info_directories.end(); ++it ) {
			hardware_info_directory = (*it);
			hardware_info_directory += hardware_info_directory_suffix;
	
			if (TDEGlobal::dirs()->exists(hardware_info_directory)) {
				database_filename = hardware_info_directory + "pnp.ids";
				if (TQFile::exists(database_filename)) {
					break;
				}
			}
		}

		if (!TQFile::exists(database_filename)) {
			printf("[tdehardwaredevices] Unable to locate PNP information database pnp.ids\n"); fflush(stdout);
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
			printf("[tdehardwaredevices] Unable to open PNP information database %s\n", database_filename.ascii()); fflush(stdout);
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

TQString HardwareDevices::findMonitorManufacturerName(TQString dpyid) {
	TQString friendlyName = TQString::null;

	if (!dpy_id_map) {
		dpy_id_map = new DeviceIDMap;

		TQStringList hardware_info_directories(TDEGlobal::dirs()->resourceDirs("data"));
		TQString hardware_info_directory_suffix("tdehwlib/pnpdev/");
		TQString hardware_info_directory;
		TQString database_filename;

		for ( TQStringList::Iterator it = hardware_info_directories.begin(); it != hardware_info_directories.end(); ++it ) {
			hardware_info_directory = (*it);
			hardware_info_directory += hardware_info_directory_suffix;
	
			if (TDEGlobal::dirs()->exists(hardware_info_directory)) {
				database_filename = hardware_info_directory + "dpy.ids";
				if (TQFile::exists(database_filename)) {
					break;
				}
			}
		}

		if (!TQFile::exists(database_filename)) {
			printf("[tdehardwaredevices] Unable to locate monitor information database dpy.ids\n"); fflush(stdout);
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
			printf("[tdehardwaredevices] Unable to open monitor information database %s\n", database_filename.ascii()); fflush(stdout);
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

TQPair<TQString,TQString> HardwareDevices::getEDIDMonitorName(TQString path) {
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

TQByteArray HardwareDevices::getEDID(TQString path) {
	TQFile file(TQString("%1/edid").arg(path));
	if (!file.open (IO_ReadOnly)) {
		return TQByteArray();
	}
	TQByteArray binaryedid = file.readAll();
	file.close();
	return binaryedid;
}

TQString HardwareDevices::getFriendlyDeviceTypeStringFromType(GenericDeviceType::GenericDeviceType query) {
	TQString ret = "Unknown Device";

	// Keep this in sync with the GenericDeviceType definition in the header
	if (query == GenericDeviceType::Root) {
		ret = i18n("Root");
	}
	else if (query == GenericDeviceType::RootSystem) {
		ret = i18n("System Root");
	}
	else if (query == GenericDeviceType::CPU) {
		ret = i18n("CPU");
	}
	else if (query == GenericDeviceType::GPU) {
		ret = i18n("Graphics Processor");
	}
	else if (query == GenericDeviceType::RAM) {
		ret = i18n("RAM");
	}
	else if (query == GenericDeviceType::Bus) {
		ret = i18n("Bus");
	}
	else if (query == GenericDeviceType::I2C) {
		ret = i18n("I2C Bus");
	}
	else if (query == GenericDeviceType::MDIO) {
		ret = i18n("MDIO Bus");
	}
	else if (query == GenericDeviceType::Mainboard) {
		ret = i18n("Mainboard");
	}
	else if (query == GenericDeviceType::Disk) {
		ret = i18n("Disk");
	}
	else if (query == GenericDeviceType::SCSI) {
		ret = i18n("SCSI");
	}
	else if (query == GenericDeviceType::StorageController) {
		ret = i18n("Storage Controller");
	}
	else if (query == GenericDeviceType::Mouse) {
		ret = i18n("Mouse");
	}
	else if (query == GenericDeviceType::Keyboard) {
		ret = i18n("Keyboard");
	}
	else if (query == GenericDeviceType::HID) {
		ret = i18n("HID");
	}
	else if (query == GenericDeviceType::Modem) {
		ret = i18n("Modem");
	}
	else if (query == GenericDeviceType::Monitor) {
		ret = i18n("Monitor and Display");
	}
	else if (query == GenericDeviceType::Network) {
		ret = i18n("Network");
	}
	else if (query == GenericDeviceType::Printer) {
		ret = i18n("Printer");
	}
	else if (query == GenericDeviceType::Scanner) {
		ret = i18n("Scanner");
	}
	else if (query == GenericDeviceType::Sound) {
		ret = i18n("Sound");
	}
	else if (query == GenericDeviceType::VideoCapture) {
		ret = i18n("Video Capture");
	}
	else if (query == GenericDeviceType::IEEE1394) {
		ret = i18n("IEEE1394");
	}
	else if (query == GenericDeviceType::PCMCIA) {
		ret = i18n("PCMCIA");
	}
	else if (query == GenericDeviceType::Camera) {
		ret = i18n("Camera");
	}
	else if (query == GenericDeviceType::TextIO) {
		ret = i18n("Text I/O");
	}
	else if (query == GenericDeviceType::Serial) {
		ret = i18n("Serial Communications Controller");
	}
	else if (query == GenericDeviceType::Parallel) {
		ret = i18n("Parallel Port");
	}
	else if (query == GenericDeviceType::Peripheral) {
		ret = i18n("Peripheral");
	}
	else if (query == GenericDeviceType::Backlight) {
		ret = i18n("Backlight");
	}
	else if (query == GenericDeviceType::Battery) {
		ret = i18n("Battery");
	}
	else if (query == GenericDeviceType::PowerSupply) {
		ret = i18n("Power Supply");
	}
	else if (query == GenericDeviceType::Dock) {
		ret = i18n("Docking Station");
	}
	else if (query == GenericDeviceType::ThermalSensor) {
		ret = i18n("Thermal Sensor");
	}
	else if (query == GenericDeviceType::ThermalControl) {
		ret = i18n("Thermal Control");
	}
	else if (query == GenericDeviceType::BlueTooth) {
		ret = i18n("Bluetooth");
	}
	else if (query == GenericDeviceType::Bridge) {
		ret = i18n("Bridge");
	}
	else if (query == GenericDeviceType::Platform) {
		ret = i18n("Platform");
	}
	else if (query == GenericDeviceType::Cryptography) {
		ret = i18n("Cryptography");
	}
	else if (query == GenericDeviceType::Event) {
		ret = i18n("Platform Event");
	}
	else if (query == GenericDeviceType::Input) {
		ret = i18n("Platform Input");
	}
	else if (query == GenericDeviceType::PNP) {
		ret = i18n("Plug and Play");
	}
	else if (query == GenericDeviceType::OtherACPI) {
		ret = i18n("Other ACPI");
	}
	else if (query == GenericDeviceType::OtherUSB) {
		ret = i18n("Other USB");
	}
	else if (query == GenericDeviceType::OtherMultimedia) {
		ret = i18n("Other Multimedia");
	}
	else if (query == GenericDeviceType::OtherPeripheral) {
		ret = i18n("Other Peripheral");
	}
	else if (query == GenericDeviceType::OtherSensor) {
		ret = i18n("Other Sensor");
	}
	else if (query == GenericDeviceType::OtherVirtual) {
		ret = i18n("Other Virtual");
	}
	else {
		ret = i18n("Unknown Device");
	}

	return ret;
}

TQPixmap HardwareDevices::getDeviceTypeIconFromType(GenericDeviceType::GenericDeviceType query, TDEIcon::StdSizes size) {
	TQPixmap ret = DesktopIcon("misc", size);

// 	// Keep this in sync with the GenericDeviceType definition in the header
	if (query == GenericDeviceType::Root) {
		ret = DesktopIcon("kcmdevices", size);
	}
	else if (query == GenericDeviceType::RootSystem) {
		ret = DesktopIcon("kcmdevices", size);
	}
	else if (query == GenericDeviceType::CPU) {
		ret = DesktopIcon("kcmprocessor", size);
	}
	else if (query == GenericDeviceType::GPU) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == GenericDeviceType::RAM) {
		ret = DesktopIcon("memory", size);
	}
	else if (query == GenericDeviceType::Bus) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == GenericDeviceType::I2C) {
		ret = DesktopIcon("input_devices_settings", size);
	}
	else if (query == GenericDeviceType::MDIO) {
		ret = DesktopIcon("input_devices_settings", size);
	}
	else if (query == GenericDeviceType::Mainboard) {
		ret = DesktopIcon("kcmpci", size);	// FIXME
	}
	else if (query == GenericDeviceType::Disk) {
		ret = DesktopIcon("hdd_unmount", size);
	}
	else if (query == GenericDeviceType::SCSI) {
		ret = DesktopIcon("kcmscsi", size);
	}
	else if (query == GenericDeviceType::StorageController) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == GenericDeviceType::Mouse) {
		ret = DesktopIcon("mouse", size);
	}
	else if (query == GenericDeviceType::Keyboard) {
		ret = DesktopIcon("keyboard", size);
	}
	else if (query == GenericDeviceType::HID) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == GenericDeviceType::Modem) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == GenericDeviceType::Monitor) {
		ret = DesktopIcon("background", size);
	}
	else if (query == GenericDeviceType::Network) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == GenericDeviceType::Printer) {
		ret = DesktopIcon("printer1", size);
	}
	else if (query == GenericDeviceType::Scanner) {
		ret = DesktopIcon("scanner", size);
	}
	else if (query == GenericDeviceType::Sound) {
		ret = DesktopIcon("kcmsound", size);
	}
	else if (query == GenericDeviceType::VideoCapture) {
		ret = DesktopIcon("tv", size);		// FIXME
	}
	else if (query == GenericDeviceType::IEEE1394) {
		ret = DesktopIcon("ieee1394", size);
	}
	else if (query == GenericDeviceType::PCMCIA) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == GenericDeviceType::Camera) {
		ret = DesktopIcon("camera", size);
	}
	else if (query == GenericDeviceType::Serial) {
		ret = DesktopIcon("input_devices_settings", size);
	}
	else if (query == GenericDeviceType::Parallel) {
		ret = DesktopIcon("input_devices_settings", size);
	}
	else if (query == GenericDeviceType::TextIO) {
		ret = DesktopIcon("chardevice", size);
	}
	else if (query == GenericDeviceType::Peripheral) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == GenericDeviceType::Backlight) {
		ret = DesktopIcon("tdescreensaver", size);	// FIXME
	}
	else if (query == GenericDeviceType::Battery) {
		ret = DesktopIcon("energy", size);
	}
	else if (query == GenericDeviceType::PowerSupply) {
		ret = DesktopIcon("energy", size);
	}
	else if (query == GenericDeviceType::Dock) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == GenericDeviceType::ThermalSensor) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == GenericDeviceType::ThermalControl) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == GenericDeviceType::BlueTooth) {
		ret = DesktopIcon("kcmpci", size);	// FIXME
	}
	else if (query == GenericDeviceType::Bridge) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == GenericDeviceType::Platform) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else if (query == GenericDeviceType::Cryptography) {
		ret = DesktopIcon("password", size);
	}
	else if (query == GenericDeviceType::Event) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else if (query == GenericDeviceType::Input) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else if (query == GenericDeviceType::PNP) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else if (query == GenericDeviceType::OtherACPI) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == GenericDeviceType::OtherUSB) {
		ret = DesktopIcon("usb", size);
	}
	else if (query == GenericDeviceType::OtherMultimedia) {
		ret = DesktopIcon("kcmsound", size);
	}
	else if (query == GenericDeviceType::OtherPeripheral) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == GenericDeviceType::OtherSensor) {
		ret = DesktopIcon("kcmdevices", size);	// FIXME
	}
	else if (query == GenericDeviceType::OtherVirtual) {
		ret = DesktopIcon("kcmsystem", size);
	}
	else {
		ret = DesktopIcon("hwinfo", size);
	}

	return ret;
}

RootSystemDevice* HardwareDevices::rootSystemDevice() {
	GenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->type() == GenericDeviceType::RootSystem) {
			return dynamic_cast<RootSystemDevice*>(hwdevice);
		}
	}

	return 0;
}

TQString HardwareDevices::bytesToFriendlySizeString(double bytes) {
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

GenericHardwareList HardwareDevices::listByDeviceClass(GenericDeviceType::GenericDeviceType cl) {
	GenericHardwareList ret;
	ret.setAutoDelete(false);

	GenericDevice *hwdevice;
	for ( hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next() ) {
		if (hwdevice->type() == cl) {
			ret.append(hwdevice);
		}
	}

	return ret;
}

GenericHardwareList HardwareDevices::listAllPhysicalDevices() {
	GenericHardwareList ret = m_deviceList;
	ret.setAutoDelete(false);

	return ret;
}

#include "hardwaredevices.moc"
