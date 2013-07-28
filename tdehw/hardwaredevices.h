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
#ifndef _TDEHARDWAREDEVICES_H
#define _TDEHARDWAREDEVICES_H


// TDE includes
#include <tqobject.h>
#include <tqptrlist.h>
#include <tqmap.h>
#include <tqstring.h>
#include <tqstringlist.h>

#include <tdelibs_export.h>
#include <kicontheme.h>

#include "hwcommontypes.h"

/**
 * Hardware Device Access and Monitoring Library
 *
 * @author Timothy Pearson
 */

struct udev;
struct udev_device;
struct udev_monitor;

class TQSocketNotifier;
class KSimpleDirWatch;

namespace TDEHW {

class TDEGenericDevice;
class TDEStorageDevice;
class TDENetworkDevice;
class TDEBacklightDevice;
class TDEMonitorDevice;
class TDESensorDevice;
class TDERootSystemDevice;
class TDEEventDevice;
class TDEInputDevice;

typedef TQPtrList<TDEGenericDevice> TDEGenericHardwareList;
typedef TQMap<TQString, TQString> TDEDeviceIDMap;

class TDEHW_EXPORT TDEHardwareDevices : public TQObject
{
	Q_OBJECT

	public:	
		/**
		 * Get the global instance of the singleton object. 
		 * @return the global TDEHardwareDevices instance.
		 */
		static TDEHardwareDevices* instance();

		/**
		* Destructor.
		*/
		~TDEHardwareDevices();

		/**
		*  Query all hardware capabilities on all devices
		*  This does not normally need to be called by an application, as
		*  device detection is handled internally and automatically
		*  
		*  A call to this method immediately invalidates any TDEGenericHardwareList
		*  structures returned by listAllPhysicalDevices()
		*  
		*  @return TRUE if successful
		*/
		bool queryHardwareInformation();

		/**
		*  List all hardware capabilities on all devices
		*  @return TDEGenericHardwareList containing all known hardware devices
		*/
		TDEGenericHardwareList listAllPhysicalDevices();

		/**
		*  List all hardware capabilities on all devices
		*  @param a TDEGenericDeviceType::TDEGenericDeviceType specifying the device class
		*  @return TDEGenericHardwareList containing all known hardware devices
		*/
		TDEGenericHardwareList listByDeviceClass(TDEGenericDeviceType::TDEGenericDeviceType cl);

		/**
		*  Return the device with system path @arg syspath, or 0 if no device exists for that path
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findBySystemPath(TQString syspath);

		/**
		*  Return the device with unique ID @arg uid, or 0 if no device exists for that uid
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findByUniqueID(TQString uid);

		/**
		*  Return the device with device node @arg devnode, or 0 if no device exists at that node
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findByDeviceNode(TQString devnode);

		/**
		*  Return the storage device with unique ID @arg uid, or 0 if no device exists for that uid
		*  @return TDEGenericDevice
		*/
		TDEStorageDevice* findDiskByUID(TQString uid);

		/**
		*  Look up the device in the system PCI database
		*  @param vendorid a TQString containing the vendor ID in hexadecimal
		*  @param modelid a TQString containing the model ID in hexadecimal
		*  @param subvendorid a TQString containing the subvendor ID in hexadecimal
		*  @param submodelid a TQString containing the submodel ID in hexadecimal
		*  @return a TQString containing the device name, if found
		*/
		TQString findPCIDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid);

		/**
		*  Look up the device in the system USB database
		*  @param vendorid a TQString containing the vendor ID in hexadecimal
		*  @param modelid a TQString containing the model ID in hexadecimal
		*  @param subvendorid a TQString containing the subvendor ID in hexadecimal
		*  @param submodelid a TQString containing the submodel ID in hexadecimal
		*  @return a TQString containing the device name, if found
		*/
		TQString findUSBDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid);

		/**
		*  Look up the device in the system PNP database
		*  @param pnpid a TQString containing the PNP ID
		*  @return a TQString containing the device name, if found
		*/
		TQString findPNPDeviceName(TQString pnpid);

		/**
		*  Look up the monitor manufacturer in the system display database
		*  @param pnpid a TQString containing the display manufacturer ID
		*  @return a TQString containing the manufacturer name, if found
		*/
		TQString findMonitorManufacturerName(TQString dpyid);

		/**
		*  Get a friendly string describing a device type
		*  @param query a TDEGenericDeviceType::TDEGenericDeviceType specifying a device type
		*  @return a TQString containing the friendly type name
		*/
		TQString getFriendlyDeviceTypeStringFromType(TDEGenericDeviceType::TDEGenericDeviceType query);

		/**
		*  Get an icon for a device type
		*  @param query a TDEGenericDeviceType::TDEGenericDeviceType specifying a device type
		*  @param size a TDEIcon::StdSizes structure specifying the desired icon size
		*  @return a TQPixmap containing the icon for the specified type
		*/
		TQPixmap getDeviceTypeIconFromType(TDEGenericDeviceType::TDEGenericDeviceType query, TDEIcon::StdSizes size);

		/**
		*  Convenience function to obtain the root system device
		*  @return a pointer to a TDERootSystemDevice object
		*/
		TDERootSystemDevice* rootSystemDevice();

		/**
		*  Rescan a hardware device to look for changes
		*  WARNING: This method can be very expensive.  Use with caution!
		*  @param hwdevice TDEGenericDevice* with the device to rescan
		*/
		void rescanDeviceInformation(TDEGenericDevice* hwdevice);

		/**
		*  Rescan a hardware device to look for changes
		*  WARNING: This method can be very expensive.  Use with caution!
		*  The computational expense can be reduced somewhat if the device tree structure
		*  has not changed by calling this method with regenerateDeviceTree = FALSE.
		*  @param hwdevice TDEGenericDevice* with the device to rescan
		*  @param regenerateDeviceTree TRUE to update parent/child links in device tree
		*/
		void rescanDeviceInformation(TDEGenericDevice* hwdevice, bool regenerateDeviceTree);

		/**
		*  Enable or disable automatic state updates of triggerless hardware devices
		*  such as CPUs and network cards.  When enabled, your application will use
		*  additional CPU resources to continually poll triggerless hardware devices.
		*  Automatic updates are disabled by default.
		*  @param enable a bool specifiying whether or not automatic updates should be enabled
		*/
		void setTriggerlessHardwareUpdatesEnabled(bool enable);

		/**
		*  Convert a byte count to human readable form
		*  @param bytes a double containing the number of bytes
		*  @return a TQString containing the human readable byte count
		*/
		static TQString bytesToFriendlySizeString(double bytes);

	signals:
		void hardwareAdded(TDEGenericDevice*);
		void hardwareRemoved(TDEGenericDevice*);
		void hardwareUpdated(TDEGenericDevice*);
		void mountTableModified();
		void hardwareEvent(TDEHardwareEvent::TDEHardwareEvent, TQString uuid);

		/**
		* @param keycode the code of the key that was pressed/released
		* See include/linux/input.h for a complete list of keycodes
		* @param device a TDEEventDevice* with the device that received the event
		*/
		void eventDeviceKeyPressed(unsigned int keycode, TDEEventDevice* device);
	protected:
		/**
		*  Protected constructor of singleton object.
		*/
		TDEHardwareDevices();
		
		static TDEHardwareDevices* _instance;

	private slots:
		void processHotPluggedHardware();
		void processModifiedMounts();
		void processModifiedCPUs();
		void processStatelessDevices();
		void processEventDeviceKeyPressed(unsigned int keycode, TDEEventDevice* edevice);

	private:
		void updateBlacklists(TDEGenericDevice* hwdevice, udev_device* dev);

	private:
		TDEGenericDevice *classifyUnknownDevice(udev_device* dev, TDEGenericDevice* existingdevice=0, bool force_full_classification=true);
		TDEGenericDevice *classifyUnknownDeviceByExternalRules(udev_device* dev, TDEGenericDevice* existingdevice=0, bool classifySubDevices=false);
		void updateExistingDeviceInformation(TDEGenericDevice* existingdevice, udev_device* dev=NULL);

		void updateParentDeviceInformation();
		void updateParentDeviceInformation(TDEGenericDevice* hwdevice);

		void addCoreSystemDevices();

		/**
		* Get the binary monitor EDID for the specified sysfs path
		* @return a TQByteArray containing the EDID
		*/
		TQByteArray getEDID(TQString path);
		
		/**
		* Get the monitor EDID name for the specified sysfs path
		* @return a TQPair<TQString,TQString> containing the monitor vendor and model, if available
		*/
		TQPair<TQString,TQString> getEDIDMonitorName(TQString path);

		struct udev *m_udevStruct;
		struct udev_monitor *m_udevMonitorStruct;
		TDEGenericHardwareList m_deviceList;
		int m_procMountsFd;
		KSimpleDirWatch* m_cpuWatch;
		TQTimer* m_cpuWatchTimer;
		TQTimer* m_deviceWatchTimer;

		TQSocketNotifier* m_devScanNotifier;
		TQSocketNotifier* m_mountScanNotifier;

		TQStringList m_mountTable;
		TQStringList m_cpuInfo;

		TDEDeviceIDMap* pci_id_map;
		TDEDeviceIDMap* usb_id_map;
		TDEDeviceIDMap* pnp_id_map;
		TDEDeviceIDMap* dpy_id_map;

	friend class TDEGenericDevice;
	friend class TDEStorageDevice;
	friend class TDECPUDevice;
};

} // namespace TDEHW

#endif // _TDEHARDWAREDEVICES_H
