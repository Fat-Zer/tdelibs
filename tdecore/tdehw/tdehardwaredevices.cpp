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

#include "tdehardwaredevices.h"

#include <tqfile.h>
#include <tqdir.h>
#include <tqtimer.h>
#include <tqsocketnotifier.h>
#include <tqstringlist.h>

#include <tdeconfig.h>
#include <kstandarddirs.h>

#include <tdeglobal.h>
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

#include "kiconloader.h"

#include "tdegenericdevice.h"
#include "tdestoragedevice.h"
#include "tdecpudevice.h"
#include "tdebatterydevice.h"
#include "tdemainspowerdevice.h"
#include "tdenetworkdevice.h"
#include "tdebacklightdevice.h"
#include "tdemonitordevice.h"
#include "tdesensordevice.h"
#include "tderootsystemdevice.h"
#include "tdeeventdevice.h"
#include "tdeinputdevice.h"

// Compile-time configuration
#include "config.h"

// Profiling stuff
//#define CPUPROFILING
//#define STATELESSPROFILING

#include <time.h>
timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

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

#if defined(WITH_TDEHWLIB_DAEMONS) || defined(WITH_UDISKS) || defined(WITH_UDISKS2) || defined(WITH_NETWORK_MANAGER_BACKEND)
#include <tqdbusvariant.h>
#include <tqdbusdata.h>
// Convenience method for tdehwlib DBUS calls
// FIXME
// Should probably be part of dbus-1-tqt
TQT_DBusData convertDBUSDataToVariantData(TQT_DBusData object) {
	TQT_DBusVariant variant;
	variant.value = object;
	variant.signature = variant.value.buildDBusSignature();
	return TQT_DBusData::fromVariant(variant);
}
#endif // defined(WITH_UDISKS) || defined(WITH_UDISKS2) || defined(WITH_NETWORK_MANAGER_BACKEND)

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

		// Special case for battery polling (longer delay, 5 seconds)
		m_batteryWatchTimer = new TQTimer(this);
		connect( m_batteryWatchTimer, SIGNAL(timeout()), this, SLOT(processBatteryDevices()) );

		// Update internal device information
		queryHardwareInformation();
	}
}

TDEHardwareDevices::~TDEHardwareDevices() {
	// Stop device scanning
	m_deviceWatchTimer->stop();
	m_batteryWatchTimer->stop();

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

void TDEHardwareDevices::setTriggerlessHardwareUpdatesEnabled(bool enable) {
	if (enable) {
		TQDir nodezerocpufreq("/sys/devices/system/cpu/cpu0/cpufreq");
		if (nodezerocpufreq.exists()) {
			m_cpuWatchTimer->start( 500, FALSE ); // 0.5 second repeating timer
		}
		m_batteryWatchTimer->stop(); // Battery devices are included in stateless devices
		m_deviceWatchTimer->start( 1000, FALSE ); // 1 second repeating timer
	}
	else {
		m_cpuWatchTimer->stop();
		m_deviceWatchTimer->stop();
	}
}

void TDEHardwareDevices::setBatteryUpdatesEnabled(bool enable) {
	if (enable) {
		TQDir nodezerocpufreq("/sys/devices/system/cpu/cpu0/cpufreq");
		if (nodezerocpufreq.exists()) {
			m_cpuWatchTimer->start( 500, FALSE ); // 0.5 second repeating timer
		}
		m_batteryWatchTimer->start( 5000, FALSE ); // 5 second repeating timer
	}
	else {
		m_cpuWatchTimer->stop();
		m_batteryWatchTimer->stop();
	}
}

void TDEHardwareDevices::rescanDeviceInformation(TDEGenericDevice* hwdevice) {
	rescanDeviceInformation(hwdevice, true);
}

void TDEHardwareDevices::rescanDeviceInformation(TDEGenericDevice* hwdevice, bool regenerateDeviceTree) {
	struct udev_device *dev;
	dev = udev_device_new_from_syspath(m_udevStruct, hwdevice->systemPath().ascii());
	updateExistingDeviceInformation(hwdevice);
	if (regenerateDeviceTree) {
		updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
	}
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

TDECPUDevice* TDEHardwareDevices::findCPUBySystemPath(TQString syspath, bool inCache=true) {
	TDECPUDevice* cdevice;
	
	// Look for the device in the cache first
	if(inCache && !m_cpuByPathCache.isEmpty()) {
		cdevice = m_cpuByPathCache.find(syspath);
		if(cdevice) {
			return cdevice;
		}
	}

	// If the CPU was not found in cache, we need to parse the entire device list to get it.
	cdevice = dynamic_cast<TDECPUDevice*>(findBySystemPath(syspath));
	if(cdevice) {
		if(inCache) {
			m_cpuByPathCache.insert(syspath, cdevice); // Add the device to the cache
		}
		return cdevice;
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
				emit hardwareEvent(TDEHardwareEvent::HardwareAdded, device->uniqueID());
			}
		}
		else if (actionevent == "remove") {
			// Delete device from hardware listing
			TQString systempath(udev_device_get_syspath(dev));
			systempath += "/";
			TDEGenericDevice *hwdevice;
			for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
				if (hwdevice->systemPath() == systempath) {
					// Temporarily disable auto-deletion to ensure object validity when calling the Removed events below
					m_deviceList.setAutoDelete(false);

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
								emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, slavedevice->uniqueID());
							}
						}
					}
					else {
						m_deviceList.remove(hwdevice);
					}

					emit hardwareRemoved(hwdevice);
					emit hardwareEvent(TDEHardwareEvent::HardwareRemoved, hwdevice->uniqueID());

					// Reenable auto-deletion and delete the removed device object
					m_deviceList.setAutoDelete(true);
					delete hwdevice;

					break;
				}
			}
		}
		else if (actionevent == "change") {
			// Update device and emit change event
			TQString systempath(udev_device_get_syspath(dev));
			systempath += "/";
			TDEGenericDevice *hwdevice;
			for (hwdevice = m_deviceList.first(); hwdevice; hwdevice = m_deviceList.next()) {
				if (hwdevice->systemPath() == systempath) {
					if (!hwdevice->blacklistedForUpdate()) {
						classifyUnknownDevice(dev, hwdevice, false);
						updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
						emit hardwareUpdated(hwdevice);
						emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, hwdevice->uniqueID());
					}
				}
				else if ((hwdevice->type() == TDEGenericDeviceType::Monitor)
						&& (hwdevice->systemPath().contains(systempath))) {
					if (!hwdevice->blacklistedForUpdate()) {
						struct udev_device *slavedev;
						slavedev = udev_device_new_from_syspath(m_udevStruct, hwdevice->systemPath().ascii());
						classifyUnknownDevice(slavedev, hwdevice, false);
						udev_device_unref(slavedev);
						updateParentDeviceInformation(hwdevice);	// Update parent/child tables for this device
						emit hardwareUpdated(hwdevice);
						emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, hwdevice->uniqueID());
					}
				}
			}
		}
	}
}

void TDEHardwareDevices::processModifiedCPUs() {
	// Detect what changed between the old cpu information and the new information,
	// and emit appropriate events

#ifdef CPUPROFILING
	timespec time1, time2, time3;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	time3 = time1;
	printf("TDEHardwareDevices::processModifiedCPUs() : begin at '%u'\n", time1.tv_nsec);
#endif

	// Read new CPU information table
	m_cpuInfo.clear();
	TQFile cpufile( "/proc/cpuinfo" );
	if ( cpufile.open( IO_ReadOnly ) ) {
		TQTextStream stream( &cpufile );
		// Using read() instead of readLine() inside a loop is 4 times faster !
		m_cpuInfo = TQStringList::split('\n', stream.read(), true);
		cpufile.close();
	}

#ifdef CPUPROFILING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	printf("TDEHardwareDevices::processModifiedCPUs() : checkpoint1 at %u [%u]\n", time2.tv_nsec, diff(time1,time2).tv_nsec);
	time1 = time2;
#endif

	// Ensure "processor" is the first entry in each block and determine which cpuinfo type is in use
	bool cpuinfo_format_x86 = true;
	bool cpuinfo_format_arm = false;

	TQString curline1;
	TQString curline2;
	int blockNumber = 0;
	TQStringList::Iterator blockBegin = m_cpuInfo.begin();
	for (TQStringList::Iterator cpuit1 = m_cpuInfo.begin(); cpuit1 != m_cpuInfo.end(); ++cpuit1) {
		curline1 = *cpuit1;
		if (!(*blockBegin).startsWith("processor")) {
			bool found = false;
			TQStringList::Iterator cpuit2;
			for (cpuit2 = blockBegin; cpuit2 != m_cpuInfo.end(); ++cpuit2) {
				curline2 = *cpuit2;
				if (curline2.startsWith("processor")) {
					found = true;
					break;
				}
				else if (curline2 == NULL || curline2 == "") {
					break;
				}
			}
			if (found) {
				m_cpuInfo.insert(blockBegin, (*cpuit2));
			}
			else if(blockNumber == 0) {
				m_cpuInfo.insert(blockBegin, "processor : 0");
			}
		}
		if (curline1 == NULL || curline1 == "") {
			blockNumber++;
			blockBegin = cpuit1;
			blockBegin++;
		}
		else if (curline1.startsWith("Processor")) {
			cpuinfo_format_x86 = false;
			cpuinfo_format_arm = true;
		}
	}

#ifdef CPUPROFILING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	printf("TDEHardwareDevices::processModifiedCPUs() : checkpoint2 at %u [%u]\n", time2.tv_nsec, diff(time1,time2).tv_nsec);
	time1 = time2;
#endif

	// Parse CPU information table
	TDECPUDevice *cdevice;
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
				curline.remove(0, curline.find(":")+2);
				processorNumber = curline.toInt();
				if (!cdevice) {
					cdevice = dynamic_cast<TDECPUDevice*>(findCPUBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber)));
				}
				if (cdevice) {
					if (cdevice->coreNumber() != processorNumber) {
						modified = true;
						cdevice->internalSetCoreNumber(processorNumber);
					}
				}
			}
			else if (cdevice && curline.startsWith("model name")) {
				curline.remove(0, curline.find(":")+2);
				if (cdevice->name() != curline) {
					modified = true;
					cdevice->internalSetName(curline);
				}
			}
			else if (cdevice && curline.startsWith("cpu MHz")) {
				curline.remove(0, curline.find(":")+2);
				if (cdevice->frequency() != curline.toDouble()) {
					modified = true;
					cdevice->internalSetFrequency(curline.toDouble());
				}
				have_frequency = true;
			}
			else if (cdevice && curline.startsWith("vendor_id")) {
				curline.remove(0, curline.find(":")+2);
				if (cdevice->vendorName() != curline) {
					modified = true;
					cdevice->internalSetVendorName(curline);
				}
				if (cdevice->vendorEncoded() != curline) {
					modified = true;
					cdevice->internalSetVendorEncoded(curline);
				}
			}
			else if (curline == NULL || curline == "") {
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
				curline.remove(0, curline.find(":")+2);
				modelName = curline;
			}
			else if (curline.startsWith("Hardware")) {
				curline.remove(0, curline.find(":")+2);
				vendorName = curline;
			}
			else if (curline.startsWith("Serial")) {
				curline.remove(0, curline.find(":")+2);
				serialNumber = curline;
			}
		}
		for (TQStringList::Iterator cpuit = m_cpuInfo.begin(); cpuit != m_cpuInfo.end(); ++cpuit) {
			curline = *cpuit;
			if (curline.startsWith("processor")) {
				curline.remove(0, curline.find(":")+2);
				processorNumber = curline.toInt();
				if (!cdevice) {
					cdevice = dynamic_cast<TDECPUDevice*>(findCPUBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber)));
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
			if (curline == NULL || curline == "") {
				cdevice = 0;
			}
		}
	}

	processorCount = processorNumber+1;

#ifdef CPUPROFILING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	printf("TDEHardwareDevices::processModifiedCPUs() : checkpoint3 at %u [%u]\n", time2.tv_nsec, diff(time1,time2).tv_nsec);
	time1 = time2;
#endif

	TDECPUDevice* firstCPU = NULL;
	
	// Read in other information from cpufreq, if available
	for (processorNumber=0; processorNumber<processorCount; processorNumber++) {
		cdevice = dynamic_cast<TDECPUDevice*>(findCPUBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber)));
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
			TQString nodename;
			if ((processorNumber == 0) || (!firstCPU)) {
				// Remember the first CPU options so that we can reuse it later.
				firstCPU = cdevice;

				nodename = cpufreq_dir.path();
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
			// Other CPU should have the same values as the first one. Simply copy them.
			else {
				scalinggovernor = firstCPU->governor();
				scalingdriver = firstCPU->scalingDriver();
				minfrequency = firstCPU->minFrequency();
				maxfrequency = firstCPU->maxFrequency();
				trlatency = firstCPU->transitionLatency();
				frequencylist = firstCPU->availableFrequencies();
				governorlist = firstCPU->availableGovernors();
			}

			// The following data are different on each CPU
			nodename = cpufreq_dir.path();
			nodename.append("/affected_cpus");
			TQFile tiedcpusfile(nodename);
			if (tiedcpusfile.open(IO_ReadOnly)) {
				TQTextStream stream( &tiedcpusfile );
				affectedcpulist = TQStringList::split(" ", stream.readLine());
				tiedcpusfile.close();
			}

			// We may already have the CPU Mhz information in '/proc/cpuinfo'
			if (!have_frequency) {
				nodename = cpufreq_dir.path();
				nodename.append("/cpuinfo_cur_freq");
				TQFile cpufreqfile(nodename);
				if (cpufreqfile.open(IO_ReadOnly)) {
					TQTextStream stream( &cpufreqfile );
					if (cdevice) {
						cdevice->internalSetFrequency(stream.readLine().toDouble()/1000.0);
					}
					cpufreqfile.close();
					have_frequency = true;
				}
			}

			bool minfrequencyFound = false;
			bool maxfrequencyFound = false;
			TQStringList::Iterator freqit;
			for ( freqit = frequencylist.begin(); freqit != frequencylist.end(); ++freqit ) {
				double thisfrequency = (*freqit).toDouble()/1000.0;
				if (thisfrequency == minfrequency) {
					minfrequencyFound = true;
				}
				if (thisfrequency == maxfrequency) {
					maxfrequencyFound = true;
				}

			}
			if (!minfrequencyFound) {
				int minFrequencyInt = (minfrequency*1000.0);
				frequencylist.prepend(TQString("%1").arg(minFrequencyInt));
			}
			if (!maxfrequencyFound) {
				int maxfrequencyInt = (maxfrequency*1000.0);
				frequencylist.append(TQString("%1").arg(maxfrequencyInt));
			}

#ifdef CPUPROFILING
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
			printf("TDEHardwareDevices::processModifiedCPUs() : checkpoint3.%u at %u [%u]\n", processorNumber, time2.tv_nsec, diff(time1,time2).tv_nsec);
			time1 = time2;
#endif
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
			if (cdevice->governor() != scalinggovernor) {
				modified = true;
				cdevice->internalSetGovernor(scalinggovernor);
			}
			if (cdevice->scalingDriver() != scalingdriver) {
				modified = true;
				cdevice->internalSetScalingDriver(scalingdriver);
			}
			if (cdevice->minFrequency() != minfrequency) {
				modified = true;
				cdevice->internalSetMinFrequency(minfrequency);
			}
			if (cdevice->maxFrequency() != maxfrequency) {
				modified = true;
				cdevice->internalSetMaxFrequency(maxfrequency);
			}
			if (cdevice->transitionLatency() != trlatency) {
				modified = true;
				cdevice->internalSetTransitionLatency(trlatency);
			}
			if (cdevice->dependentProcessors().join(" ") != affectedcpulist.join(" ")) {
				modified = true;
				cdevice->internalSetDependentProcessors(affectedcpulist);
			}
			if (cdevice->availableFrequencies().join(" ") != frequencylist.join(" ")) {
				modified = true;
				cdevice->internalSetAvailableFrequencies(frequencylist);
			}
			if (cdevice->availableGovernors().join(" ") != governorlist.join(" ")) {
				modified = true;
				cdevice->internalSetAvailableGovernors(governorlist);
			}
		}
	}

#ifdef CPUPROFILING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	printf("TDEHardwareDevices::processModifiedCPUs() : checkpoint4 at %u [%u]\n", time2.tv_nsec, diff(time1,time2).tv_nsec);
	time1 = time2;
#endif

	if (modified) {
		for (processorNumber=0; processorNumber<processorCount; processorNumber++) {
			TDEGenericDevice* hwdevice = findCPUBySystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber));
			// Signal new information available
			emit hardwareUpdated(hwdevice);
			emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, hwdevice->uniqueID());
		}
	}
	
#ifdef CPUPROFILING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	printf("TDEHardwareDevices::processModifiedCPUs() : end at %u [%u]\n", time2.tv_nsec, diff(time1,time2).tv_nsec);
	printf("TDEHardwareDevices::processModifiedCPUs() : total time: %u\n", diff(time3,time2).tv_nsec);
#endif
}

void TDEHardwareDevices::processStatelessDevices() {
	// Some devices do not emit changed signals
	// So far, network cards and sensors need to be polled
	TDEGenericDevice *hwdevice;

#ifdef STATELESSPROFILING
	timespec time1, time2, time3;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	printf("TDEHardwareDevices::processStatelessDevices() : begin at '%u'\n", time1.tv_nsec);
	time3 = time1;
#endif

	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	TDEGenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		if ((hwdevice->type() == TDEGenericDeviceType::RootSystem) || (hwdevice->type() == TDEGenericDeviceType::Network) || (hwdevice->type() == TDEGenericDeviceType::OtherSensor) || (hwdevice->type() == TDEGenericDeviceType::Event) || (hwdevice->type() == TDEGenericDeviceType::Battery) || (hwdevice->type() == TDEGenericDeviceType::PowerSupply)) {
			rescanDeviceInformation(hwdevice, false);
			emit hardwareUpdated(hwdevice);
			emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, hwdevice->uniqueID());
#ifdef STATELESSPROFILING
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
			printf("TDEHardwareDevices::processStatelessDevices() : '%s' finished at %u [%u]\n", (hwdevice->name()).ascii(), time2.tv_nsec, diff(time1,time2).tv_nsec);
			time1 = time2;
#endif
		}
	}

#ifdef STATELESSPROFILING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	printf("TDEHardwareDevices::processStatelessDevices() : end at '%u'\n", time2.tv_nsec);
	printf("TDEHardwareDevices::processStatelessDevices() : took '%u'\n", diff(time3,time2).tv_nsec);
#endif
}

void TDEHardwareDevices::processBatteryDevices() {
	TDEGenericDevice *hwdevice;

	// We can't use m_deviceList directly as m_deviceList can only have one iterator active against it at any given time
	TDEGenericHardwareList devList = listAllPhysicalDevices();
	for ( hwdevice = devList.first(); hwdevice; hwdevice = devList.next() ) {
		if (hwdevice->type() == TDEGenericDeviceType::Battery) {
			rescanDeviceInformation(hwdevice, false);
			emit hardwareUpdated(hwdevice);
			emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, hwdevice->uniqueID());
		}
	}
}


void TDEHardwareDevices::processEventDeviceKeyPressed(unsigned int keycode, TDEEventDevice* edevice) {
	emit eventDeviceKeyPressed(keycode, edevice);
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
			emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, hwdevice->uniqueID());
			// If the device is a storage device and has a slave, update it as well
			if (hwdevice->type() == TDEGenericDeviceType::Disk) {
				TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
				TQStringList slavedevices = sdevice->slaveDevices();
				for ( TQStringList::Iterator slaveit = slavedevices.begin(); slaveit != slavedevices.end(); ++slaveit ) {
					TDEGenericDevice* slavedevice = findBySystemPath(*slaveit);
					if (slavedevice) {
						emit hardwareUpdated(slavedevice);
						emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, slavedevice->uniqueID());
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
			emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, hwdevice->uniqueID());
			// If the device is a storage device and has a slave, update it as well
			if (hwdevice->type() == TDEGenericDeviceType::Disk) {
				TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
				TQStringList slavedevices = sdevice->slaveDevices();
				for ( TQStringList::Iterator slaveit = slavedevices.begin(); slaveit != slavedevices.end(); ++slaveit ) {
					TDEGenericDevice* slavedevice = findBySystemPath(*slaveit);
					if (slavedevice) {
						emit hardwareUpdated(slavedevice);
						emit hardwareEvent(TDEHardwareEvent::HardwareUpdated, slavedevice->uniqueID());
					}
				}
			}
		}
	}

	emit mountTableModified();
	emit hardwareEvent(TDEHardwareEvent::MountTableModified, TQString());
}

TDEDiskDeviceType::TDEDiskDeviceType classifyDiskType(udev_device* dev, const TQString devicenode, const TQString devicebus, const TQString disktypestring, const TQString systempath, const TQString devicevendor, const TQString devicemodel, const TQString filesystemtype, const TQString devicedriver) {
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

	if ((disktypestring.upper() == "COMPACT_FLASH")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_CF")) == "1")) {
		disktype = disktype | TDEDiskDeviceType::CompactFlash;
	}

	if ((disktypestring.upper() == "MEMORY_STICK")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_MS")) == "1")) {
		disktype = disktype | TDEDiskDeviceType::MemoryStick;
	}

	if ((disktypestring.upper() == "SMART_MEDIA")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_SM")) == "1")) {
		disktype = disktype | TDEDiskDeviceType::SmartMedia;
	}

	if ((disktypestring.upper() == "SD_MMC")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_SD")) == "1")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_SDHC")) == "1")
		|| (TQString(udev_device_get_property_value(dev, "ID_DRIVE_FLASH_MMC")) == "1")) {
		disktype = disktype | TDEDiskDeviceType::SDMMC;
	}

	if ((disktypestring.upper() == "FLASHKEY")
		|| (TQString(udev_device_get_property_value(dev, " ID_DRIVE_FLASH")) == "1")) {
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
	if ((disktype & TDEDiskDeviceType::Zip) || (disktype & TDEDiskDeviceType::Floppy) || (disktype & TDEDiskDeviceType::Jaz) || (disktype & TDEDiskDeviceType::Tape)) {
		disktype = disktype & ~TDEDiskDeviceType::HDD;
	}

	if (disktypestring.upper() == "CD") {
		disktype = disktype & ~TDEDiskDeviceType::HDD;
		disktype = disktype | TDEDiskDeviceType::Optical;

		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) == "1") {
			disktype = disktype | TDEDiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_CD_R")) == "1") {
			disktype = disktype | TDEDiskDeviceType::CDR;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_CD_RW")) == "1") {
			disktype = disktype | TDEDiskDeviceType::CDRW;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDR;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_MRW")) == "1") {
			disktype = disktype | TDEDiskDeviceType::CDMRRW;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDR;
			disktype = disktype & ~TDEDiskDeviceType::CDRW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_MRW_W")) == "1") {
			disktype = disktype | TDEDiskDeviceType::CDMRRWW;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDR;
			disktype = disktype & ~TDEDiskDeviceType::CDRW;
			disktype = disktype & ~TDEDiskDeviceType::CDMRRW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_MO")) == "1") {
			disktype = disktype | TDEDiskDeviceType::CDMO;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDR;
			disktype = disktype & ~TDEDiskDeviceType::CDRW;
			disktype = disktype & ~TDEDiskDeviceType::CDMRRW;
			disktype = disktype & ~TDEDiskDeviceType::CDMRRWW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RAM")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDRAM;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_R")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDR;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_R_DL")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::DVDR;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_R")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDPLUSR;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::DVDR;
			disktype = disktype & ~TDEDiskDeviceType::DVDRDL;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_R_DL")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDPLUSRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::DVDR;
			disktype = disktype & ~TDEDiskDeviceType::DVDRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSR;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RW")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDRW;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::DVDR;
			disktype = disktype & ~TDEDiskDeviceType::DVDRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSR;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSRDL;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_RW_DL")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDRWDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::DVDR;
			disktype = disktype & ~TDEDiskDeviceType::DVDRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSR;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDRW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDPLUSRW;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::DVDR;
			disktype = disktype & ~TDEDiskDeviceType::DVDRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSR;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDRW;
			disktype = disktype & ~TDEDiskDeviceType::DVDRWDL;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD_PLUS_RW_DL")) == "1") {
			disktype = disktype | TDEDiskDeviceType::DVDPLUSRWDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDROM;
			disktype = disktype & ~TDEDiskDeviceType::DVDR;
			disktype = disktype & ~TDEDiskDeviceType::DVDRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSR;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSRDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDRW;
			disktype = disktype & ~TDEDiskDeviceType::DVDRWDL;
			disktype = disktype & ~TDEDiskDeviceType::DVDPLUSRW;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD")) == "1") {
			disktype = disktype | TDEDiskDeviceType::BDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_R")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_R_DL")) == "1")	// FIXME There is no official udev attribute for this type of disc (yet!)
			) {
			disktype = disktype | TDEDiskDeviceType::BDR;
			disktype = disktype & ~TDEDiskDeviceType::BDROM;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_RE")) == "1")
			|| (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD_RE_DL")) == "1")	// FIXME There is no official udev attribute for this type of disc (yet!)
			) {
			disktype = disktype | TDEDiskDeviceType::BDRW;
			disktype = disktype & ~TDEDiskDeviceType::BDROM;
			disktype = disktype & ~TDEDiskDeviceType::BDR;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_HDDVD")) == "1") {
			disktype = disktype | TDEDiskDeviceType::HDDVDROM;
			disktype = disktype & ~TDEDiskDeviceType::CDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_HDDVD_R")) == "1") {
			disktype = disktype | TDEDiskDeviceType::HDDVDR;
			disktype = disktype & ~TDEDiskDeviceType::HDDVDROM;
		}
		if (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_HDDVD_RW")) == "1") {
			disktype = disktype | TDEDiskDeviceType::HDDVDRW;
			disktype = disktype & ~TDEDiskDeviceType::HDDVDROM;
			disktype = disktype & ~TDEDiskDeviceType::HDDVDR;
		}
		if (!TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO")).isNull()) {
			disktype = disktype | TDEDiskDeviceType::CDAudio;
		}
		if ((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_VCD")) == "1") || (TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA_SDVD")) == "1")) {
			disktype = disktype | TDEDiskDeviceType::CDVideo;
		}

		if ((disktype & TDEDiskDeviceType::DVDROM)
			|| (disktype & TDEDiskDeviceType::DVDRAM)
			|| (disktype & TDEDiskDeviceType::DVDR)
			|| (disktype & TDEDiskDeviceType::DVDRW)
			|| (disktype & TDEDiskDeviceType::DVDRDL)
			|| (disktype & TDEDiskDeviceType::DVDRWDL)
			|| (disktype & TDEDiskDeviceType::DVDPLUSR)
			|| (disktype & TDEDiskDeviceType::DVDPLUSRW)
			|| (disktype & TDEDiskDeviceType::DVDPLUSRDL)
			|| (disktype & TDEDiskDeviceType::DVDPLUSRWDL)
			) {
				// Every VideoDVD must have a VIDEO_TS.IFO file
				// Read this info via tdeiso_info, since udev couldn't be bothered to check DVD type on its own
				int retcode = system(TQString("tdeiso_info --exists=ISO9660/VIDEO_TS/VIDEO_TS.IFO %1").arg(devicenode).ascii());
				if (retcode == 0) {
					disktype = disktype | TDEDiskDeviceType::DVDVideo;
				}
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

	// TDEStandardDirs::kde_default

typedef TQMap<TQString, TQString> TDEConfigMap;

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
	else if (query == "Modem") {
		ret = TDEGenericDeviceType::Modem;
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
	else if (query == "Bluetooth") {
		ret = TDEGenericDeviceType::BlueTooth;
	}
	else if (query == "Bridge") {
		ret = TDEGenericDeviceType::Bridge;
	}
	else if (query == "Platform") {
		ret = TDEGenericDeviceType::Platform;
	}
	else if (query == "Cryptography") {
		ret = TDEGenericDeviceType::Cryptography;
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
	if (query == "CDR") {
		ret = ret | TDEDiskDeviceType::CDR;
	}
	if (query == "CDRW") {
		ret = ret | TDEDiskDeviceType::CDRW;
	}
	if (query == "CDMO") {
		ret = ret | TDEDiskDeviceType::CDMO;
	}
	if (query == "CDMRRW") {
		ret = ret | TDEDiskDeviceType::CDMRRW;
	}
	if (query == "CDMRRWW") {
		ret = ret | TDEDiskDeviceType::CDMRRWW;
	}
	if (query == "DVDROM") {
		ret = ret | TDEDiskDeviceType::DVDROM;
	}
	if (query == "DVDRAM") {
		ret = ret | TDEDiskDeviceType::DVDRAM;
	}
	if (query == "DVDR") {
		ret = ret | TDEDiskDeviceType::DVDR;
	}
	if (query == "DVDRW") {
		ret = ret | TDEDiskDeviceType::DVDRW;
	}
	if (query == "DVDRDL") {
		ret = ret | TDEDiskDeviceType::DVDRDL;
	}
	if (query == "DVDRWDL") {
		ret = ret | TDEDiskDeviceType::DVDRWDL;
	}
	if (query == "DVDPLUSR") {
		ret = ret | TDEDiskDeviceType::DVDPLUSR;
	}
	if (query == "DVDPLUSRW") {
		ret = ret | TDEDiskDeviceType::DVDPLUSRW;
	}
	if (query == "DVDPLUSRDL") {
		ret = ret | TDEDiskDeviceType::DVDPLUSRDL;
	}
	if (query == "DVDPLUSRWDL") {
		ret = ret | TDEDiskDeviceType::DVDPLUSRWDL;
	}
	if (query == "BDROM") {
		ret = ret | TDEDiskDeviceType::BDROM;
	}
	if (query == "BDR") {
		ret = ret | TDEDiskDeviceType::BDR;
	}
	if (query == "BDRW") {
		ret = ret | TDEDiskDeviceType::BDRW;
	}
	if (query == "HDDVDROM") {
		ret = ret | TDEDiskDeviceType::HDDVDROM;
	}
	if (query == "HDDVDR") {
		ret = ret | TDEDiskDeviceType::HDDVDR;
	}
	if (query == "HDDVDRW") {
		ret = ret | TDEDiskDeviceType::HDDVDRW;
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
							TDEGenericDeviceType::TDEGenericDeviceType desiredDeviceType = device->type();
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
		|| (devicesubsystem == "scsi_disk")
		|| (devicesubsystem == "scsi_tape"))
		&& ((devicenode != "")
		)) {
		if (!device) device = new TDEStorageDevice(TDEGenericDeviceType::Disk);
	}
	else if (devicetype == "host") {
		if (devicesubsystem == "bluetooth") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::BlueTooth);
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
		else if (devicesubsystem == "usb-serial") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Serial);
		}
		else if ((devicesubsystem == "spi_master")
			|| (devicesubsystem == "spidev")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Serial);
		}
		else if (devicesubsystem == "spi") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		else if (devicesubsystem == "watchdog") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		else if (devicesubsystem == "node") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		else if (devicesubsystem == "regulator") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		else if (devicesubsystem == "memory") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		else if (devicesubsystem == "clockevents") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
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
		else if (devicesubsystem == "virtio") {
			if (devicedriver == "virtio_blk") {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::SCSI);
			}
			if (devicedriver == "virtio_net") {
				if (!device) device = new TDENetworkDevice(TDEGenericDeviceType::Network);
			}
			if (devicedriver == "virtio_balloon") {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::RAM);
			}
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
		if (systempath.lower().startsWith("/sys/module/")
			|| (systempath.lower().startsWith("/sys/kernel/"))) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);	// FIXME Should go into a new kernel module category when the tdelibs ABI can be broken again
		}
		if ((devicetypestring == "audio")
			|| (devicesubsystem == "sound")
			|| (devicesubsystem == "ac97")) {
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
			|| (devicetypestring_alt == "sas_target")
			|| (devicesubsystem == "sas_host")
			|| (devicesubsystem == "sas_port")
			|| (devicesubsystem == "sas_device")
			|| (devicesubsystem == "sas_expander")
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
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		if (devicesubsystem == "infiniband") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Peripheral);
		}
		if ((devicesubsystem == "infiniband_cm")
			|| (devicesubsystem == "infiniband_mad")
			|| (devicesubsystem == "infiniband_verbs")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		if (devicesubsystem == "infiniband_srp") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::SCSI);
		}
		if ((devicesubsystem == "enclosure")
			|| (devicesubsystem == "clocksource")
			|| (devicesubsystem == "amba")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
		if ((devicesubsystem == "ipmi")
			|| (devicesubsystem == "ipmi_si")) {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Mainboard);
		}
		if (devicesubsystem == "misc") {
			if (devicedriver.startsWith("tpm_")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Cryptography);
			}
			else {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
			}
		}
		if (devicesubsystem == "leds") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherACPI);
		}
		if (devicesubsystem == "net") {
			if (!device) device = new TDENetworkDevice(TDEGenericDeviceType::Network);
		}
		if ((devicesubsystem == "i2c")
			|| (devicesubsystem == "i2c-dev")) {
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
		if (devicesubsystem == "tifm_adapter") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::StorageController);
		}
		if (devicesubsystem == "mmc_host") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::StorageController);
		}
		if (devicesubsystem == "mmc") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
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
			if (devicedriver.contains("atkbd")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Keyboard);
			}
			else if (devicedriver.contains("mouse")) {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Mouse);
			}
			else {
				if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Serial);
			}
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
		if (devicesubsystem == "machinecheck") {
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
			TQString devicepcisubclass = devicepciclass;
			devicepcisubclass = devicepcisubclass.remove(0,2);
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
			if (devicepciclass.startsWith("07")) {
				if (devicepcisubclass.startsWith("03")) {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Modem);
				}
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
			|| (devicesubsystem == "usbmisc")
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
						device = new TDEStorageDevice(TDEGenericDeviceType::Disk);
						TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
						sdevice->internalSetDiskType(TDEDiskDeviceType::Camera);
						TQString parentsyspathudev = systempath;
						parentsyspathudev.truncate(parentsyspathudev.length()-1);	// Remove trailing slash
						parentsyspathudev.truncate(parentsyspathudev.findRev("/"));
						struct udev_device *parentdev;
						parentdev = udev_device_new_from_syspath(m_udevStruct, parentsyspathudev.ascii());
						devicenode = (udev_device_get_devnode(parentdev));
					}
				}
				else {
					if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherUSB);
				}
		}
		if (devicesubsystem == "pci") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::OtherPeripheral);
		}
		if (devicesubsystem == "cpu") {
			if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Platform);
		}
	}

	if (device == 0) {
		// Unhandled
		if (!device) device = new TDEGenericDevice(TDEGenericDeviceType::Other);
		printf("[FIXME] UNCLASSIFIED DEVICE name: %s type: %s subsystem: %s driver: %s [Node Path: %s] [Syspath: %s] [%s:%s]\n", devicename.ascii(), devicetype.ascii(), devicesubsystem.ascii(), devicedriver.ascii(), devicenode.ascii(), udev_device_get_syspath(dev), devicevendorid.ascii(), devicemodelid.ascii()); fflush(stdout);
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

void TDEHardwareDevices::updateExistingDeviceInformation(TDEGenericDevice* existingdevice, udev_device* dev) {
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

	if (device->type() == TDEGenericDeviceType::Disk) {
		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
		if (sdevice->diskType() & TDEDiskDeviceType::Camera) {
			// PictBridge cameras are special and should not be classified by standard rules
			sdevice->internalSetDiskStatus(TDEDiskDeviceStatus::Removable);
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
			TQString disklabel(TQString::fromLocal8Bit(udev_device_get_property_value(dev, "ID_FS_LABEL")));
			TQString diskuuid(udev_device_get_property_value(dev, "ID_FS_UUID"));
			TQString filesystemtype(udev_device_get_property_value(dev, "ID_FS_TYPE"));
			TQString filesystemusage(udev_device_get_property_value(dev, "ID_FS_USAGE"));
	
			device->internalSetVendorName(devicevendor);
			device->internalSetVendorModel(devicemodel);
			device->internalSetDeviceBus(devicebus);
	
			TDEDiskDeviceType::TDEDiskDeviceType disktype = sdevice->diskType();
			TDEDiskDeviceStatus::TDEDiskDeviceStatus diskstatus = TDEDiskDeviceStatus::Null;
	
			disktype = classifyDiskType(dev, devicenode, devicebus, devicetypestring, systempath, devicevendor, devicemodel, filesystemtype, devicedriver);
			sdevice->internalSetDiskType(disktype);
			device = classifyUnknownDeviceByExternalRules(dev, device, true);	// Check external rules for possible subtype overrides
			disktype = sdevice->diskType();						// The type can be overridden by an external rule

			if (TQString(udev_device_get_property_value(dev, "UDISKS_IGNORE")) == "1") {
				diskstatus = diskstatus | TDEDiskDeviceStatus::Hidden;
			}
	
			if ((disktype & TDEDiskDeviceType::CDROM)
				|| (disktype & TDEDiskDeviceType::CDR)
				|| (disktype & TDEDiskDeviceType::CDRW)
				|| (disktype & TDEDiskDeviceType::CDMO)
				|| (disktype & TDEDiskDeviceType::CDMRRW)
				|| (disktype & TDEDiskDeviceType::CDMRRWW)
				|| (disktype & TDEDiskDeviceType::DVDROM)
				|| (disktype & TDEDiskDeviceType::DVDRAM)
				|| (disktype & TDEDiskDeviceType::DVDR)
				|| (disktype & TDEDiskDeviceType::DVDRW)
				|| (disktype & TDEDiskDeviceType::DVDRDL)
				|| (disktype & TDEDiskDeviceType::DVDRWDL)
				|| (disktype & TDEDiskDeviceType::DVDPLUSR)
				|| (disktype & TDEDiskDeviceType::DVDPLUSRW)
				|| (disktype & TDEDiskDeviceType::DVDPLUSRDL)
				|| (disktype & TDEDiskDeviceType::DVDPLUSRWDL)
				|| (disktype & TDEDiskDeviceType::BDROM)
				|| (disktype & TDEDiskDeviceType::BDR)
				|| (disktype & TDEDiskDeviceType::BDRW)
				|| (disktype & TDEDiskDeviceType::HDDVDROM)
				|| (disktype & TDEDiskDeviceType::HDDVDR)
				|| (disktype & TDEDiskDeviceType::HDDVDRW)
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
				sdevice->internalSetMediaInserted((TQString(udev_device_get_property_value(dev, "ID_CDROM_MEDIA")) != ""));
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
	
			if ((filesystemtype.upper() != "CRYPTO_LUKS") && (filesystemtype.upper() != "CRYPTO") && (filesystemtype.upper() != "SWAP") && (!filesystemtype.isNull())) {
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
			// Swap partitions cannot be mounted
			if (filesystemtype.upper() == "SWAP") {
				diskstatus = diskstatus & ~TDEDiskDeviceStatus::Mountable;
			}
			// If certain disk types do not report the presence of a filesystem, they are likely not mountable
			if ((disktype & TDEDiskDeviceType::HDD) || (disktype & TDEDiskDeviceType::Optical)) {
				if (!(diskstatus & TDEDiskDeviceStatus::ContainsFilesystem)) {
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
			if ((sdevice->isDiskOfType(TDEDiskDeviceType::CDROM))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDR))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDRW))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDMO))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDMRRW))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDMRRWW))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDROM))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRAM))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDR))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRW))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRDL))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRWDL))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDPLUSR))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDPLUSRW))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDPLUSRDL))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDPLUSRWDL))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDROM))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDR))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDRW))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::HDDVDROM))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::HDDVDR))
				|| (sdevice->isDiskOfType(TDEDiskDeviceType::HDDVDRW))
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
	}

	if (device->type() == TDEGenericDeviceType::Network) {
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
						else if (nodename == "carrier") {
							ndevice->internalSetCarrierPresent(line.toInt());
						}
						else if (nodename == "dormant") {
							ndevice->internalSetDormant(line.toInt());
						}
						else if (nodename == "operstate") {
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
									else if (family == AF_INET6) {
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
									else if (family == AF_INET6) {
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
									else if (family == AF_INET6) {
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
									else if (family == AF_INET6) {
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
							else if (nodename == "tx_bytes") {
								ndevice->internalSetTxBytes(line.toDouble());
							}
							else if (nodename == "rx_packets") {
								ndevice->internalSetRxPackets(line.toDouble());
							}
							else if (nodename == "tx_packets") {
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
						else if (sensornodetype == "input") {
							sensors[sensornodename].current = lineValue;
						}
						else if (sensornodetype == "min") {
							sensors[sensornodename].minimum = lineValue;
						}
						else if (sensornodetype == "max") {
							sensors[sensornodename].maximum = lineValue;
						}
						else if (sensornodetype == "warn") {
							sensors[sensornodename].warning = lineValue;
						}
						else if (sensornodetype == "crit") {
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
					else if (nodename == "charge_full" || nodename == "energy_full") {
						bdevice->internalSetMaximumEnergy(line.toDouble()/1000000.0);
					}
					else if (nodename == "charge_full_design" || nodename == "energy_full_design") {
						bdevice->internalSetMaximumDesignEnergy(line.toDouble()/1000000.0);
					}
					else if (nodename == "charge_now" || nodename == "energy_now") {
						bdevice->internalSetEnergy(line.toDouble()/1000000.0);
					}
					else if (nodename == "manufacturer") {
						bdevice->internalSetVendorName(line.stripWhiteSpace());
					}
					else if (nodename == "model_name") {
						bdevice->internalSetVendorModel(line.stripWhiteSpace());
					}
					else if (nodename == "power_now" || nodename == "current_now") {
						bdevice->internalSetDischargeRate(line.toDouble()/1000000.0);
					}
					else if (nodename == "present") {
						bdevice->internalSetInstalled(line.toInt());
					}
					else if (nodename == "serial_number") {
						bdevice->internalSetSerialNumber(line.stripWhiteSpace());
					}
					else if (nodename == "status") {
						bdevice->internalSetStatus(line);
					}
					else if (nodename == "technology") {
						bdevice->internalSetTechnology(line);
					}
					else if (nodename == "voltage_min_design") {
						bdevice->internalSetMinimumVoltage(line.toDouble()/1000000.0);
					}
					else if (nodename == "voltage_now") {
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
					else if (nodename == "model_name") {
						pdevice->internalSetVendorModel(line.stripWhiteSpace());
					}
					else if (nodename == "online") {
						pdevice->internalSetOnline(line.toInt());
					}
					else if (nodename == "serial_number") {
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
					else if (nodename == "max_brightness") {
						bdevice->internalSetMaximumRawBrightness(line.toInt());
					}
					else if (nodename == "actual_brightness") {
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
					else if (nodename == "enabled") {
						mdevice->internalSetEnabled(line.lower() == "enabled");
					}
					else if (nodename == "modes") {
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
					else if (nodename == "dpms") {
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
			else {
				mdevice->m_friendlyName = i18n("Generic %1 Device").arg(genericPortName);
			}
			mdevice->internalSetEdid(getEDID(mdevice->systemPath()));
		}
		else {
			mdevice->m_friendlyName = i18n("Disconnected %1 Port").arg(genericPortName);
			mdevice->internalSetEdid(TQByteArray());
			mdevice->internalSetResolutions(TDEResolutionList());
		}

		// FIXME
		// Much of the code in libtderandr should be integrated into/interfaced with this library
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
						if (line.contains("freeze")) {
							powerstates.append(TDESystemPowerState::Freeze);
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
		else if (edevice->systemPath().contains("PNP0C0E") || edevice->systemPath().contains("/LNXSLPBN")) {
			edevice->internalSetEventType(TDEEventDeviceType::ACPISleepButton);
		}
		else if (edevice->systemPath().contains("PNP0C0C") || edevice->systemPath().contains("/LNXPWRBN")) {
			edevice->internalSetEventType(TDEEventDeviceType::ACPIPowerButton);
		}
		else if (edevice->systemPath().contains("_acpi")) {
			edevice->internalSetEventType(TDEEventDeviceType::ACPIOtherInput);
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
		else if (idevice->systemPath().contains("PNP0C0E") || idevice->systemPath().contains("/LNXSLPBN")) {
			idevice->internalSetInputType(TDEInputDeviceType::ACPISleepButton);
		}
		else if (idevice->systemPath().contains("PNP0C0C") || idevice->systemPath().contains("/LNXPWRBN")) {
			idevice->internalSetInputType(TDEInputDeviceType::ACPIPowerButton);
		}
		else if (idevice->systemPath().contains("_acpi")) {
			idevice->internalSetInputType(TDEInputDeviceType::ACPIOtherInput);
		}
		else {
			idevice->internalSetInputType(TDEInputDeviceType::Unknown);
		}
	}

	if (device->type() == TDEGenericDeviceType::Event) {
		// Try to obtain as much specific information about this event device as possible
		TDEEventDevice* edevice = dynamic_cast<TDEEventDevice*>(device);

		// Try to open input event device
		if (edevice->m_fd < 0 && access (edevice->deviceNode().ascii(), R_OK) == 0) {
			edevice->m_fd = open(edevice->deviceNode().ascii(), O_RDONLY);
		}

		// Start monitoring of input event device
		edevice->internalStartMonitoring(this);
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

	if (temp_udev_device) {
		udev_device_unref(dev);
	}
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

	emit hardwareEvent(TDEHardwareEvent::HardwareListModified, TQString());

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
				hwdevice = new TDECPUDevice(TDEGenericDeviceType::CPU);
				hwdevice->internalSetSystemPath(TQString("/sys/devices/system/cpu/cpu%1").arg(processorNumber));
				m_deviceList.append(hwdevice);
			}
		}
		++it;
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

TQString TDEHardwareDevices::findPNPDeviceName(TQString pnpid) {
	TQString friendlyName = TQString::null;

	if (!pnp_id_map) {
		pnp_id_map = new TDEDeviceIDMap;

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

TQString TDEHardwareDevices::findMonitorManufacturerName(TQString dpyid) {
	TQString friendlyName = TQString::null;

	if (!dpy_id_map) {
		dpy_id_map = new TDEDeviceIDMap;

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
	else if (query == TDEGenericDeviceType::Modem) {
		ret = i18n("Modem");
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
	else if (query == TDEGenericDeviceType::BlueTooth) {
		ret = i18n("Bluetooth");
	}
	else if (query == TDEGenericDeviceType::Bridge) {
		ret = i18n("Bridge");
	}
	else if (query == TDEGenericDeviceType::Platform) {
		ret = i18n("Platform");
	}
	else if (query == TDEGenericDeviceType::Cryptography) {
		ret = i18n("Cryptography");
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

TQPixmap TDEHardwareDevices::getDeviceTypeIconFromType(TDEGenericDeviceType::TDEGenericDeviceType query, TDEIcon::StdSizes size) {
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
		ret = DesktopIcon("preferences-desktop-peripherals", size);
	}
	else if (query == TDEGenericDeviceType::MDIO) {
		ret = DesktopIcon("preferences-desktop-peripherals", size);
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
	else if (query == TDEGenericDeviceType::Modem) {
		ret = DesktopIcon("kcmpci", size);
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
		ret = DesktopIcon("camera-photo", size);
	}
	else if (query == TDEGenericDeviceType::Serial) {
		ret = DesktopIcon("preferences-desktop-peripherals", size);
	}
	else if (query == TDEGenericDeviceType::Parallel) {
		ret = DesktopIcon("preferences-desktop-peripherals", size);
	}
	else if (query == TDEGenericDeviceType::TextIO) {
		ret = DesktopIcon("chardevice", size);
	}
	else if (query == TDEGenericDeviceType::Peripheral) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::Backlight) {
		ret = DesktopIcon("tdescreensaver", size);	// FIXME
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
	else if (query == TDEGenericDeviceType::BlueTooth) {
		ret = DesktopIcon("kcmpci", size);	// FIXME
	}
	else if (query == TDEGenericDeviceType::Bridge) {
		ret = DesktopIcon("kcmpci", size);
	}
	else if (query == TDEGenericDeviceType::Platform) {
		ret = DesktopIcon("preferences-system", size);
	}
	else if (query == TDEGenericDeviceType::Cryptography) {
		ret = DesktopIcon("password", size);
	}
	else if (query == TDEGenericDeviceType::Event) {
		ret = DesktopIcon("preferences-system", size);
	}
	else if (query == TDEGenericDeviceType::Input) {
		ret = DesktopIcon("preferences-system", size);
	}
	else if (query == TDEGenericDeviceType::PNP) {
		ret = DesktopIcon("preferences-system", size);
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
		ret = DesktopIcon("preferences-system", size);
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
