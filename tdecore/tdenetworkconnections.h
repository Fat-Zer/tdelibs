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
#ifndef _TDENETWORKCONNECTIONS_H
#define _TDENETWORKCONNECTIONS_H

// TDE includes
#include <tqobject.h>
#include <tqstring.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include "kiconloader.h"
#include "tdelibs_export.h"

#define CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(x)														\
	inline x operator|(x a, x b)																\
	{																			\
		return static_cast<x>(static_cast<int>(a) | static_cast<int>(b));										\
	}																			\
																				\
	inline x operator&(x a, x b)																\
	{																			\
		return static_cast<x>(static_cast<int>(a) & static_cast<int>(b));										\
	}																			\
																				\
	inline x operator~(x a)																	\
	{																			\
		return static_cast<x>(~static_cast<int>(a));													\
	}

namespace TDENetworkConnectionType {
enum TDENetworkConnectionType {
	WiredEthernet,
	WiFi,
	Bluetooth,
	OLPCMesh,
	WiMax,
	Modem,
	Infiniband,
	Bond,
	VLAN,
	ADSL,
	Other,
	Last = Other
};
};

namespace TDEWiFiMode {
enum TDEWiFiMode {
	AdHoc,
	Infrastructure,
	Other,
	Last = Other
};
};

namespace TDENetworkGlobalManagerFlags {
	enum TDENetworkGlobalManagerFlags {
		Unknown			= 0x00000000,
		Disconnected		= 0x00000001,
		Connected		= 0x00000002,
		EstablishingLink	= 0x00000004,
		DeactivatingLink	= 0x00000008,
		LinkLocalAccess		= 0x00000010,
		SiteLocalAccess		= 0x00000020,
		GlobalAccess		= 0x00000040,
		Sleeping		= 0x00000080,
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkGlobalManagerFlags)
};

namespace TDENetworkDeviceCapabilityFlags {
	enum TDENetworkDeviceCapabilityFlags {
		None			= 0x00000000,
		Supported		= 0x00000001,
		CanDetectLink		= 0x00000002
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkDeviceCapabilityFlags)
};

namespace TDENetworkWiFiAPFlags {
	enum TDENetworkWiFiAPFlags {
		None			= 0x00000000,
		PrivacySupport		= 0x00000001,
		PairWEP40		= 0x00000002,
		PairWEP104		= 0x00000004,
		PairTKIP		= 0x00000008,
		PairCCMP		= 0x00000010,
		GroupWEP40		= 0x00000020,
		GroupWEP104		= 0x00000040,
		GroupTKIP		= 0x00000080,
		GroupCCMP		= 0x00000100,
		KeyManagementPSK	= 0x00000200,
		KeyManagement80211	= 0x00000400
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkWiFiAPFlags)
};

namespace TDENetworkWiFiClientFlags {
	enum TDENetworkWiFiClientFlags {
		None			= 0x00000000,
		CipherWEP40		= 0x00000002,
		CipherWEP104		= 0x00000004,
		CipherTKIP		= 0x00000008,
		CipherCCMP		= 0x00000010,
		CipherWPA		= 0x00000020,
		CipherRSN		= 0x00000040
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkWiFiClientFlags)
};

namespace TDENetworkWiFiConnectionCipher {
	enum TDENetworkWiFiConnectionCipher {
		None,
		CipherWEP40,
		CipherWEP104,
		CipherTKIP,
		CipherCCMP,
		CipherWPA,
		CipherRSN
	};
};

namespace TDENetworkConnectionStatus {
	enum TDENetworkConnectionStatus {
		Invalid			= 0x00000000,
		Disconnected		= 0x00000001,
		Connected		= 0x00000002,
		LinkUnavailable		= 0x00000004,
		EstablishingLink	= 0x00000008,
		ConfiguringProtocols	= 0x00000010,
		Reconnecting		= 0x00000020,
		LinkLost		= 0x00000040,
		LinkLocalAccess		= 0x00000080,
		SiteLocalAccess		= 0x00000100,
		GlobalAccess		= 0x00000200,
		UnManaged		= 0x00000400,
		NeedAuthorization	= 0x00000800,
		Failed			= 0x00001000,
		VerifyingProtocols	= 0x00002000,
		DependencyWait		= 0x00004000
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkConnectionStatus)
};

namespace TDENetworkIPConfigurationFlags {
	enum TDENetworkIPConfigurationFlags {
		Invalid			= 0x00000000,
		IPV4			= 0x00000001,
		IPV6			= 0x00000002,
		DHCP			= 0x00000004,
		StaticIP		= 0x00000008
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkIPConfigurationFlags)
};

class TDECORE_EXPORT TDENetworkIPConfiguration
{
	public:
		TDENetworkIPConfiguration();
		~TDENetworkIPConfiguration();

	public:
		bool valid;
		TDENetworkIPConfigurationFlags::TDENetworkIPConfigurationFlags connectionFlags;
		TQString ipAddress;
		TQString networkMask;
		TQString gateway;
		TQString broadcast;
		TQString destination;
};

class TDECORE_EXPORT TDENetworkWiFiDeviceInfo
{
	public:
		TDENetworkWiFiDeviceInfo();
		~TDENetworkWiFiDeviceInfo();

	public:
		bool valid;
		TQString hwAddress;
		TQString permanentHWAddress;
		TDEWiFiMode::TDEWiFiMode operatingMode;
		unsigned int bitrate;
		TQString activeAccessPointBSSID;
		TDENetworkWiFiClientFlags::TDENetworkWiFiClientFlags wirelessFlags;
};

class TDECORE_EXPORT TDENetworkDeviceInformation
{
	public:
		TDENetworkDeviceInformation();
		~TDENetworkDeviceInformation();

	public:
		bool valid;
		TDENetworkConnectionStatus::TDENetworkConnectionStatus statusFlags;
		TQString UUID;
		TQString backendDriver;
		TQString backendDriverVersion;
		TQString firmwareVersion;
		TDENetworkDeviceCapabilityFlags::TDENetworkDeviceCapabilityFlags capabilityFlags;
		TDENetworkIPConfiguration ipConfiguration;
		bool managed;
		bool autoConnect;
		bool firmwareMissing;
		TDENetworkConnectionType::TDENetworkConnectionType deviceType;
		TDENetworkWiFiDeviceInfo wirelessInfo;
};

class TDECORE_EXPORT TDENetworkHWNeighbor
{
	public:
		TDENetworkHWNeighbor();
		~TDENetworkHWNeighbor();

	public:
		bool valid;
};

class TDECORE_EXPORT TDENetworkWiFiAPInfo : public TDENetworkHWNeighbor
{
	public:
		TDENetworkWiFiAPInfo();
		~TDENetworkWiFiAPInfo();

	public:
		TQString SSID;
		TDENetworkWiFiAPFlags::TDENetworkWiFiAPFlags flags;
		unsigned int frequency;
		TQString BSSID;
		unsigned int maxBitrate;
		double signalQuality;
};

typedef TQPtrList< TDENetworkHWNeighbor > TDENetworkHWNeighborList;

class TDECORE_EXPORT TDENetworkConnection : public TQObject
{
	Q_OBJECT

	public:
		TDENetworkConnection();
		~TDENetworkConnection();

	public:
		TQString UUID;
		TQString friendlyName;
		TDENetworkIPConfiguration ipConfig;
		bool autoConnect;
};

class TDECORE_EXPORT TDEWiFiConnection : public TDENetworkConnection
{
	Q_OBJECT

	public:
		TDEWiFiConnection();
		~TDEWiFiConnection();

	public:
		TQString SSID;
		TDENetworkWiFiConnectionCipher::TDENetworkWiFiConnectionCipher cipher;
		TQString key1;
		TQString key2;
		TQString key3;
		TQString key4;
};

typedef TQPtrList< TDENetworkConnection > TDENetworkConnectionList;

class TDECORE_EXPORT TDENetworkConnectionManager : public TQObject
{
	Q_OBJECT

	public:
		/**
		*  Constructor.
		*  @param macAddress The MAC address of the hardware device
		*/
		TDENetworkConnectionManager(TQString macAddress);
		
		/**
		* Destructor.
		*/
		~TDENetworkConnectionManager();

		/**
		* @return the MAC address of this device
		*/
		TQString deviceMACAddress();

		/**
		* @return the type of connection supported by this device
		*/
		virtual TDENetworkConnectionType::TDENetworkConnectionType connectionType() = 0;

		/**
		* @return A TDENetworkGlobalManagerFlags enum value with the current status of the networking backend.
		*/
		virtual TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags backendStatus() = 0;

		/**
		* @return A TDENetworkDeviceInformation object containing the current status of the network device.
		*/
		virtual TDENetworkDeviceInformation deviceInformation() = 0;

		/**
		* Loads all connection information from the configuration backend
		*/
		virtual void loadConnectionInformation() = 0;

		/**
		* @param connection a TDENetworkConnection object containing a
		* connection to save to the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool saveConnection(TDENetworkConnection connection) = 0;

		/**
		* @param uuid a TQString conntaining the UUID of a connection to
		* delete from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool deleteConnection(TQString uuid) = 0;

		/**
		* @return a TDENetworkConnectionList object containing a list of all
		* possible connections this connection manager is aware of, regardless
		* of current state or availability.
		*
		* loadConnectionInformation() should be called at least once before calling
		* this method, in order to update internal connection information from the
		* configuration backend.
		*
		* Note that the returned list is internally managed and must not be deleted!
		* Also note that pointers in the list may become invalid on subsequent calls to
		* loadConnectionInformation(), saveConnection(), deleteConnection(), or connections().
		*/
		virtual TDENetworkConnectionList* connections() = 0;

		/**
		* Initiates a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus initiateConnection(TQString uuid) = 0;

		/**
		* Checks the status of a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus checkConnectionStatus(TQString uuid) = 0;

		/**
		* Disconnects a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus deactivateConnection(TQString uuid) = 0;

		/**
		* @return a TDENetworkHWNeighborList object containing the result of a site survey;
		* i.e. all nearby access points or devices. This function only returns valid information
		* if the underlying network device supports site surveys.
		*
		* Note that the returned list is internally managed and must not be deleted!
		* Also note that pointers in the list may become invalid on subsequent calls to
		* siteSurvey().
		*/
		virtual TDENetworkHWNeighborList* siteSurvey() = 0;

	private:
		TDENetworkConnectionList* m_connectionList;
		TQString m_macAddress;
};

#endif // _TDENETWORKCONNECTIONS_H