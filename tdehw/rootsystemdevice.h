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

#ifndef _TDEROOTSYSTEMDEVICE_H
#define _TDEROOTSYSTEMDEVICE_H

#include "genericdevice.h"
#include "hwcommontypes.h"

namespace TDEHW {

namespace SystemFormFactor {
enum SystemFormFactor {
	Unclassified,
	Desktop,
	Laptop,
	Server,
	Other = 0x80000000
};
};

namespace SystemPowerState {
enum SystemPowerState {
	Active,
	Standby,
	Suspend,
	Hibernate,
	PowerOff,
	Reboot
};
};

namespace SystemHibernationMethod {
enum SystemHibernationMethod {
	Unsupported,
	Platform,
	Shutdown,
	Reboot,
	TestProc,
	Test
};
};

typedef TQValueList<SystemPowerState::SystemPowerState> SystemPowerStateList;
typedef TQValueList<SystemHibernationMethod::SystemHibernationMethod> SystemHibernationMethodList;

class TDEHW_EXPORT RootSystemDevice : public GenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		RootSystemDevice(GenericDeviceType::GenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~RootSystemDevice();

		/**
		* @return a SystemFormFactor::SystemFormFactor with the system's form factor
		*/
		SystemFormFactor::SystemFormFactor formFactor();

		/**
		* @return a SystemPowerStateList with all available power states
		*/
		SystemPowerStateList powerStates();

		/**
		* @return a SystemHibernationMethodList with all available hibernation methods
		*/
		SystemHibernationMethodList hibernationMethods();

		/**
		* @return a SystemHibernationMethod::SystemHibernationMethod with the current hibernation method
		*/
		SystemHibernationMethod::SystemHibernationMethod hibernationMethod();

		/**
		* @return an unsigned long with the number of bytes required to hibernate
		*/
		unsigned long diskSpaceNeededForHibernation();

		/**
		* @return TRUE if permissions allow the hibernation method to be set, FALSE if not
		*/
		bool canSetHibernationMethod();

		/**
		* @return TRUE if hardware and permissions allow the system to enter standby, FALSE if not
		*/
		bool canStandby();

		/**
		* @return TRUE if hardware and permissions allow the system to be suspended, FALSE if not
		*/
		bool canSuspend();

		/**
		* @return TRUE if hardware and permissions allow the system to be hibernated, FALSE if not
		*/
		bool canHibernate();

		/**
		* @return TRUE if permissions allow the system to be powered down, FALSE if not
		*/
		bool canPowerOff();

		/**
		* @return TRUE if permissions allow the system to be rebooted, FALSE if not
		*/
		bool canReboot();

		/**
		* @param hm a SystemHibernationMethod::SystemHibernationMethod with the desired hibernation method
		*/
		void setHibernationMethod(SystemHibernationMethod::SystemHibernationMethod hm);

		/**
		* @param ps a SystemPowerState::SystemPowerState with the desired power state
		* @return TRUE if power state was set
		*/
		bool setPowerState(SystemPowerState::SystemPowerState ps);

	protected:
		/**
		* @param ff a SystemFormFactor::SystemFormFactor with the system's form factor
		* @internal
		*/
		void internalSetFormFactor(SystemFormFactor::SystemFormFactor ff);

		/**
		* @param ps a SystemPowerStateList with all available power states
		* @internal
		*/
		void internalSetPowerStates(SystemPowerStateList ps);

		/**
		* @param hm a SystemHibernationMethodList with all available hibernation methods
		* @internal
		*/
		void internalSetHibernationMethods(SystemHibernationMethodList hm);

		/**
		* @param hm a SystemHibernationMethod::SystemHibernationMethod with the current hibernation method
		* @internal
		*/
		void internalSetHibernationMethod(SystemHibernationMethod::SystemHibernationMethod hm);

		/**
		* @param sz an unsigned long with the number of bytes required to hibernate
		* @internal
		*/
		void internalSetDiskSpaceNeededForHibernation(unsigned long sz);

	private:
		SystemFormFactor::SystemFormFactor m_formFactor;
		SystemPowerStateList m_powerStates;
		SystemHibernationMethodList m_hibernationMethods;
		SystemHibernationMethod::SystemHibernationMethod m_hibernationMethod;
		unsigned long m_hibernationSpace;

	friend class HardwareDevices;
};

} // namespace TDEHW

#endif // _TDEROOTSYSTEMDEVICE_H
