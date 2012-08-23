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
#include "tdenetworkconnections.h"

/*================================================================================================*/
/* TDENetworkIPConfiguration                                                                      */
/*================================================================================================*/

TDENetworkIPConfiguration::TDENetworkIPConfiguration() {
	valid = false;
	connectionFlags = TDENetworkIPConfigurationFlags::Invalid;
}

TDENetworkIPConfiguration::~TDENetworkIPConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkWiFiDeviceInfo                                                                      */
/*================================================================================================*/

TDENetworkWiFiDeviceInfo::TDENetworkWiFiDeviceInfo() {
	valid = false;
}

TDENetworkWiFiDeviceInfo::~TDENetworkWiFiDeviceInfo() {
	//
}

/*================================================================================================*/
/* TDENetworkDeviceInformation                                                                      */
/*================================================================================================*/

TDENetworkDeviceInformation::TDENetworkDeviceInformation() {
	valid = false;
	statusFlags = TDENetworkConnectionStatus::Invalid;
	capabilityFlags = TDENetworkDeviceCapabilityFlags::None;
}

TDENetworkDeviceInformation::~TDENetworkDeviceInformation() {
	//
}

/*================================================================================================*/
/* TDENetworkHWNeighbor                                                                      */
/*================================================================================================*/

TDENetworkHWNeighbor::TDENetworkHWNeighbor() {
	valid = false;
}

TDENetworkHWNeighbor::~TDENetworkHWNeighbor() {
	//
}

/*================================================================================================*/
/* TDENetworkWiFiAPInfo                                                                      */
/*================================================================================================*/

TDENetworkWiFiAPInfo::TDENetworkWiFiAPInfo() : TDENetworkHWNeighbor() {
	//
}

TDENetworkWiFiAPInfo::~TDENetworkWiFiAPInfo() {
	//
}

/*================================================================================================*/
/* TDENetworkConnection                                                                           */
/*================================================================================================*/

TDENetworkConnection::TDENetworkConnection() : TQObject() {
	//
}

TDENetworkConnection::~TDENetworkConnection() {
	//
}

/*================================================================================================*/
/* TDEWiFiConnection                                                                              */
/*================================================================================================*/

TDEWiFiConnection::TDEWiFiConnection() : TDENetworkConnection() {
	//
}

TDEWiFiConnection::~TDEWiFiConnection() {
	//
}

/*================================================================================================*/
/* TDENetworkConnectionManager                                                                    */
/*================================================================================================*/

TDENetworkConnectionManager::TDENetworkConnectionManager(TQString macAddress) : TQObject(), m_connectionList(NULL), m_macAddress(macAddress) {
	//
}

TDENetworkConnectionManager::~TDENetworkConnectionManager() {
	//
}

TQString TDENetworkConnectionManager::deviceMACAddress() {
	return m_macAddress;
}

/*================================================================================================*/
/* End                                                                                            */
/*================================================================================================*/

#include "tdenetworkconnections.moc"