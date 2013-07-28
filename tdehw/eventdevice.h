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

#ifndef _TDEEVENTDEVICE_H
#define _TDEEVENTDEVICE_H

#include "genericdevice.h"

class TQSocketNotifier;

namespace TDEHW {

class HardwareDevices;

namespace EventDeviceType {
enum EventDeviceType {
	Unknown,
	ACPILidSwitch,
	ACPISleepButton,
	ACPIPowerButton,
	Other = 0x80000000
};
};

// Keep friendlySwitchList() in hardwaredevices.cpp in sync with this enum
namespace SwitchType {
enum SwitchType {
	Null			= 0x00000000,
	Lid			= 0x00000001,
	TabletMode		= 0x00000002,
	HeadphoneInsert		= 0x00000004,
	RFKill			= 0x00000008,
	Radio			= 0x00000010,
	MicrophoneInsert	= 0x00000020,
	Dock			= 0x00000040,
	LineOutInsert		= 0x00000080,
	JackPhysicalInsert	= 0x00000100,
	VideoOutInsert		= 0x00000200,
	CameraLensCover		= 0x00000400,
	KeypadSlide		= 0x00000800,
	FrontProximity		= 0x00001000,
	RotateLock		= 0x00002000,
	LineInInsert		= 0x00004000
};

inline SwitchType operator|(SwitchType a, SwitchType b)
{
	return static_cast<SwitchType>(static_cast<int>(a) | static_cast<int>(b));
}

inline SwitchType operator&(SwitchType a, SwitchType b)
{
	return static_cast<SwitchType>(static_cast<int>(a) & static_cast<int>(b));
}

inline SwitchType operator~(SwitchType a)
{
	return static_cast<SwitchType>(~static_cast<int>(a));
}
};

class TDEHW_EXPORT EventDevice : public GenericDevice
{
	Q_OBJECT

	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		EventDevice(GenericDeviceType::GenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~EventDevice();

		/**
		* @return a EventDeviceType::EventDeviceType with the event device type, if known
		*/
		EventDeviceType::EventDeviceType eventType();

		/**
		* @return a SwitchType::SwitchType with all switches provided by this device
		*/
		SwitchType::SwitchType providedSwitches();

		/**
		* @return a SwitchType::SwitchType with all active switches provided by this device
		*/
		SwitchType::SwitchType activeSwitches();

		/**
		* @param switches a SwitchType::SwitchType with any switch flags set
		* @return a TQStringList with friendly names for all set switch flags
		*/
		static TQStringList friendlySwitchList(SwitchType::SwitchType switches);

	protected:
		/**
		* @param et a EventDeviceType::EventDeviceType with the event device type, if known
		* @internal
		*/
		void internalSetEventType(EventDeviceType::EventDeviceType et);

		/**
		* @param sl a SwitchType::SwitchType with all switches provided by this device
		* @internal
		*/
		void internalSetProvidedSwitches(SwitchType::SwitchType sl);

		/**
		* @param sl a SwitchType::SwitchType with all active switches provided by this device
		* @internal
		*/
		void internalSetActiveSwitches(SwitchType::SwitchType sl);

		/**
		* @param hwmanager the master hardware manager
		* @internal
		*/
		void internalStartFdMonitoring(HardwareDevices* hwmanager);

	protected slots:
		void eventReceived();

	signals:
		/**
		* @param keycode the code of the key that was pressed/released
		* See include/linux/input.h for a complete list of keycodes
		* @param device a EventDevice* with the device that received the event
		*/
		void keyPressed(unsigned int keycode, EventDevice* device);

	private:
		EventDeviceType::EventDeviceType m_eventType;
		SwitchType::SwitchType m_providedSwitches;
		SwitchType::SwitchType m_switchActive;

		int m_fd;
		bool m_fdMonitorActive;
		TQSocketNotifier* m_eventNotifier;

	friend class HardwareDevices;
};

} // namespace TDEHW

#endif // _TDEEVENTDEVICE_H
