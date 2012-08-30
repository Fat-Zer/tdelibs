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
#ifndef _TDENETWORKBACKEND_NETWORKMANAGER_H
#define _TDENETWORKBACKEND_NETWORKMANAGER_H

#include "tdenetworkconnections.h"

//====================================================================================================
// General NetworkManager DBUS service paths
//====================================================================================================
#define	NM_DBUS_PATH				"/org/freedesktop/NetworkManager"
#define NM_DBUS_PATH_SETTINGS			"/org/freedesktop/NetworkManager/Settings"
#define NM_DBUS_PATH_SETTINGS_CONNECTION	"/org/freedesktop/NetworkManager/Settings/Connection"

#define	NM_DBUS_SERVICE				"org.freedesktop.NetworkManager"
#define NM_DBUS_SETTINGS_SERVICE		"org.freedesktop.NetworkManager.Settings"
#define NM_DBUS_SETTINGS_CONNECTION_SERVICE	"org.freedesktop.NetworkManager.Settings.Connection"
//====================================================================================================

//====================================================================================================
// These defines MUST be kept in sync with their respective introspection XML files
//====================================================================================================
#define NM_DEVICE_TYPE_UNKNOWN		0
#define NM_DEVICE_TYPE_ETHERNET		1
#define NM_DEVICE_TYPE_WIFI		2
#define NM_DEVICE_TYPE_UNUSED1		3
#define NM_DEVICE_TYPE_UNUSED2		4
#define NM_DEVICE_TYPE_BT		5
#define NM_DEVICE_TYPE_OLPC_MESH	6
#define NM_DEVICE_TYPE_WIMAX		7
#define NM_DEVICE_TYPE_MODEM		8
#define NM_DEVICE_TYPE_INFINIBAND	9
#define NM_DEVICE_TYPE_BOND		10
#define NM_DEVICE_TYPE_VLAN		11
#define NM_DEVICE_TYPE_ADSL		12
//====================================================================================================
#define NM_STATE_UNKNOWN		0
#define NM_STATE_ASLEEP			10
#define NM_STATE_DISCONNECTED		20
#define NM_STATE_DISCONNECTING		30
#define NM_STATE_CONNECTING		40
#define NM_STATE_CONNECTED_LOCAL	50
#define NM_STATE_CONNECTED_SITE		60
#define NM_STATE_CONNECTED_GLOBAL	70
//====================================================================================================
#define NM_DEVICE_STATE_UNKNOWN		0
#define NM_DEVICE_STATE_UNMANAGED	10
#define NM_DEVICE_STATE_UNAVAILABLE	20
#define NM_DEVICE_STATE_DISCONNECTED	30
#define NM_DEVICE_STATE_PREPARE		40
#define NM_DEVICE_STATE_CONFIG		50
#define NM_DEVICE_STATE_NEED_AUTH	60
#define NM_DEVICE_STATE_IP_CONFIG	70
#define NM_DEVICE_STATE_IP_CHECK	80
#define NM_DEVICE_STATE_SECONDARIES	90
#define NM_DEVICE_STATE_ACTIVATED	100
#define NM_DEVICE_STATE_DEACTIVATING	110
#define NM_DEVICE_STATE_FAILED		120
//====================================================================================================
#define NM_DEVICE_CAP_NONE		0
#define NM_DEVICE_CAP_NM_SUPPORTED	1
#define NM_DEVICE_CAP_CARRIER_DETECT	2
//====================================================================================================
#define NM_EAP_FAST_PROVISIONING_DISABLED	0
#define NM_EAP_FAST_PROVISIONING_UNAUTHONLY	1
#define NM_EAP_FAST_PROVISIONING_AUTHONLY	2
#define NM_EAP_FAST_PROVISIONING_BOTH		3
//====================================================================================================
#define NM_PASSWORD_SECRET_NONE		0
#define NM_PASSWORD_SECRET_AGENTOWNED	1
#define NM_PASSWORD_SECRET_NOTSAVED	2
#define NM_PASSWORD_SECRET_NOTREQUIRED	4
//====================================================================================================
#define NM_ACCESS_POINT_CAP_NONE	0x0
#define NM_ACCESS_POINT_CAP_PRIVACY	0x1
//====================================================================================================
#define NM_ACCESS_POINT_SEC_NONE		0x0
#define NM_ACCESS_POINT_SEC_PAIR_WEP40		0x1
#define NM_ACCESS_POINT_SEC_PAIR_WEP104		0x2
#define NM_ACCESS_POINT_SEC_PAIR_TKIP		0x4
#define NM_ACCESS_POINT_SEC_PAIR_CCMP		0x8
#define NM_ACCESS_POINT_SEC_GROUP_WEP40		0x10
#define NM_ACCESS_POINT_SEC_GROUP_WEP104	0x20
#define NM_ACCESS_POINT_SEC_GROUP_TKIP		0x40
#define NM_ACCESS_POINT_SEC_GROUP_CCMP		0x80
#define NM_ACCESS_POINT_SEC_KEY_MGMT_PSK	0x100
#define NM_ACCESS_POINT_SEC_KEY_MGMT_802_1X	0x200
//====================================================================================================
#define NM_WEP_TYPE_HEXADECIMAL		1
#define NM_WEP_TYPE_PASSPHRASE		2
//====================================================================================================

class TDENetworkConnectionManager_BackendNMPrivate;

class TDECORE_EXPORT TDENetworkConnectionManager_BackendNM : public TDENetworkConnectionManager
{
	Q_OBJECT

	public:
		TDENetworkConnectionManager_BackendNM(TQString macAddress);
		~TDENetworkConnectionManager_BackendNM();

		virtual TDENetworkConnectionType::TDENetworkConnectionType connectionType();
		virtual TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags backendStatus();
		virtual TDENetworkDeviceInformation deviceInformation();

		virtual void loadConnectionInformation();
		virtual bool loadConnectionSecrets(TQString uuid);
		virtual bool saveConnection(TDENetworkConnection* connection);
		virtual bool deleteConnection(TQString uuid);

		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus initiateConnection(TQString uuid);
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus checkConnectionStatus(TQString uuid);
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus deactivateConnection(TQString uuid);

		virtual TDENetworkHWNeighborList* siteSurvey();

		virtual bool networkingEnabled();
		virtual bool wiFiHardwareEnabled();

		virtual bool enableWiFi(bool enable);
		virtual bool wiFiEnabled();

	private:
		void internalProcessGlobalStateChanged(TQ_UINT32 state);
		void internalProcessDeviceStateChanged(TQ_UINT32 newState, TQ_UINT32 oldState, TQ_UINT32 reason);
		TDENetworkConnectionType::TDENetworkConnectionType nmDeviceTypeToTDEDeviceType(TQ_UINT32 nmType);
		TQString deviceInterfaceString(TQString macAddress);
		bool loadConnectionSecretsForGroup(TQString uuid, TQString group);
		TDENetworkWiFiAPInfo* getAccessPointDetails(TQString dbusPath);

	private:
		TDENetworkConnectionManager_BackendNMPrivate* d;
};

#endif // _TDENETWORKBACKEND_NETWORKMANAGER_H