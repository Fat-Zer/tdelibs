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

#ifndef _TDEMONITORDEVICE_H
#define _TDEMONITORDEVICE_H

#include "genericdevice.h"
#include "hwcommontypes.h"

namespace TDEHW {

typedef TQPair<unsigned int, unsigned int> ResolutionPair;
typedef TQValueList< ResolutionPair > ResolutionList;

class TDEHW_EXPORT MonitorDevice : public GenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		MonitorDevice(GenericDeviceType::GenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~MonitorDevice();

		/**
		* @return TRUE if a monitor is connected, FALSE if not
		*/
		bool connected();

		/**
		* @return TRUE if this port is enabled, FALSE if not
		*/
		bool enabled();

		/**
		* @return a TQByteArray containing this monitor's EDID information
		*/
		TQByteArray edid();

		/**
		* @return a ResolutionList containing this monitor's supported resolutions
		*/
		ResolutionList resolutions();

		/**
		* @return a TQString containing the display port type
		*/
		TQString portType();

		/**
		* @return a DisplayPowerLevel::DisplayPowerLevel with the current power level
		*/
		DisplayPowerLevel::DisplayPowerLevel powerLevel();

	protected:
		/**
		* @param TRUE if a monitor is connected, FALSE if not
		* @internal
		*/
		void internalSetConnected(bool cn);

		/**
		* @param TRUE if this port is enabled, FALSE if not
		* @internal
		*/
		void internalSetEnabled(bool en);

		/**
		* @param ed a TQByteArray containing this monitor's EDID information
		* @internal
		*/
		void internalSetEdid(TQByteArray ed);

		/**
		* @param rs a ResolutionList containing this monitor's supported resolutions
		* @internal
		*/
		void internalSetResolutions(ResolutionList rs);

		/**
		* @param pt a TQString containing the display port type
		* @internal
		*/
		void internalSetPortType(TQString pt);

		/**
		* @param pl a DisplayPowerLevel::DisplayPowerLevel with the current power level
		* @internal
		*/
		void internalSetPowerLevel(DisplayPowerLevel::DisplayPowerLevel pl);

	private:
		bool m_connected;
		bool m_enabled;
		TQByteArray m_edid;
		ResolutionList m_resolutions;
		TQString m_portType;
		DisplayPowerLevel::DisplayPowerLevel m_powerLevel;

	friend class HardwareDevices;
};


} //namespace TDEHW

#endif // _TDEMONITORDEVICE_H
