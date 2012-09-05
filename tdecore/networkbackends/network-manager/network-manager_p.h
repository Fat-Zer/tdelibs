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

#ifndef _TDENETWORKBACKEND_NETWORKMANAGER_P_H
#define _TDENETWORKBACKEND_NETWORKMANAGER_P_H

/* TQt headers */
#include <tqvaluelist.h>
#include <tqapplication.h>
#include <tqtimer.h>

/* TDE headers */
#include <kdebug.h>
#include <klocale.h>

/* TQDbus headers */
#include <tqdbusconnection.h>
#include <tqdbusobjectpath.h>
#include <tqdbusdata.h>
#include <tqdbuserror.h>
#include <tqdbusdatamap.h>
#include <tqdbusvariant.h>
#include <tqdbusdatalist.h>

/* NM headers */
//#include <NetworkManager.h>

/* Proxy headers */
#include "networkmanagerproxy.h"
#include "networkmanagersettings.h"
#include "connectionsettings.h"
#include "deviceproxy.h"
#include "ethernetproxy.h"
#include "infinibandproxy.h"
#include "wifiproxy.h"
#include "wimaxproxy.h"
#include "bluetoothproxy.h"
#include "olpcmeshproxy.h"
#include "activeconnectionproxy.h"
#include "accesspointproxy.h"

typedef TQT_DBusDataMap<TQString> TQT_DBusTQStringDataMap;
typedef TQValueList<TQT_DBusData> TQT_DBusDataValueList;

typedef TQMap<uint, bool> NMAsyncCallIDMap;
typedef TQMap<uint, TQT_DBusTQStringDataMap> NMAsyncSettingsResponseMap;
typedef TQMap<uint, TQT_DBusObjectPath> NMAddConnectionAsyncResponseMap;

typedef TQValueList<TQT_DBusObjectPath> TQT_DBusObjectPathList;

class TDENetworkConnectionManager_BackendNM;

class TDENetworkConnectionManager_BackendNMPrivate : public TQObject
{
	Q_OBJECT

	public:
		TDENetworkConnectionManager_BackendNMPrivate(TDENetworkConnectionManager_BackendNM* parent) : m_networkManagerProxy(NULL), m_networkManagerSettings(NULL), m_networkDeviceProxy(NULL), m_wiFiDeviceProxy(NULL), m_parent(parent) {}

	public:
		DBus::NetworkManagerProxy* m_networkManagerProxy;
		DBus::SettingsInterface* m_networkManagerSettings;
		DBus::DeviceProxy* m_networkDeviceProxy;
		DBus::WiFiDeviceProxy* m_wiFiDeviceProxy;
		NMAsyncCallIDMap nmConnectionSettingsAsyncCallWaiting;
		NMAsyncSettingsResponseMap nmConnectionSettingsAsyncSettingsResponse;
		NMAddConnectionAsyncResponseMap nmAddConnectionAsyncResponse;

	public slots:
		void processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&);
		void processConnectionSettingsUpdateAsyncReply(int);
		void processAddConnectionAsyncReply(int, const TQT_DBusObjectPath&);

		void internalProcessGlobalStateChanged(TQ_UINT32 state);
		void internalProcessDeviceStateChanged(TQ_UINT32 newState, TQ_UINT32 oldState, TQ_UINT32 reason);
		void internalProcessWiFiAccessPointAdded(const TQT_DBusObjectPath&);
		void internalProcessWiFiAccessPointRemoved(const TQT_DBusObjectPath&);

	private:
		TDENetworkConnectionManager_BackendNM* m_parent;
};

#endif // _TDENETWORKBACKEND_NETWORKMANAGER_P_H