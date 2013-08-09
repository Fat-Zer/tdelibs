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
#include <tqdatetime.h>
#include <tqstringlist.h>
#include <tqhostaddress.h>

#include <kiconloader.h>
#include <tdelibs_export.h>

class TQTimer;

namespace TDEHW {
  
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
	}																			\
																				\
	inline x &operator|=(x& a, const x& b)															\
	{																			\
		a = static_cast<x>(static_cast<int>(a) | static_cast<int>(b));											\
		return a;																	\
	}																			\
																				\
	inline x &operator&=(x& a, const x& b)															\
	{																			\
		a = static_cast<x>(static_cast<int>(a) & static_cast<int>(b));											\
		return a;																	\
	}

class NetworkDevice;

typedef TQValueList<TQ_UINT8> NetworkByteList;
typedef TQValueList<TQHostAddress> NetworkAddressList;
typedef TQMap<TQString, TQString> NetworkSettingsMap;
typedef TQMap<TQ_UINT32, TQ_UINT32> NetworkPriorityMap;

namespace NetworkDeviceType {
	enum NetworkDeviceType {
		BackendOnly,
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

namespace NetworkConnectionType {
	enum NetworkConnectionType {
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
		VPN,
		Other,
		Last = Other
	};
};

namespace NetworkConnectionErrorFlags {
	enum NetworkConnectionErrorFlags {
		NoError				= 0x00000000,
		InvalidConnectionSetting	= 0x00000001,
		InvalidIPv4Setting		= 0x00000002,
		InvalidIPv6Setting		= 0x00000004,
		InvalidEAPSetting		= 0x00000008,
		InvalidEAPKey			= 0x00000010,
		InvalidWirelessSetting		= 0x00000020,
		InvalidWirelessKey		= 0x00000040
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkConnectionErrorFlags)
};

typedef TQMap<NetworkConnectionErrorFlags::NetworkConnectionErrorFlags, TQString> NetworkErrorStringMap;

namespace NetworkInfinibandTransportMode {
	enum NetworkInfinibandTransportMode {
		Datagram,
		Connected,
		Other,
		Last = Other
	};
};

namespace WiFiMode {
	enum WiFiMode {
		AdHoc,
		Infrastructure,
		Other,
		Last = Other
	};
};

namespace WiFiFrequencyBand {
	enum WiFiFrequencyBand {
		None,
		Band2_4GHz,
		Band5GHz,
		Other,
		Last = Other
	};
};

namespace BluetoothConnectionType {
	enum BluetoothConnectionType {
		DUN,
		PAN,
		Other,
		Last = Other
	};
};

namespace ModemConnectionType {
	enum ModemConnectionType {
		CDMA,
		GSM,
		Other,
		Last = Other
	};
};

namespace GSMNetworkType {
	enum GSMNetworkType {
		Any,
		Only3G,
		GPRSEdge,
		Prefer3G,
		Prefer2G,
		Other,
		Last = Other
	};
};

namespace NetworkParity {
	enum NetworkParity {
		None,
		Even,
		Odd,
		Other,
		Last = Other
	};
};

namespace NetworkSlaveDeviceType {
	enum NetworkSlaveDeviceType {
		None,
		Bond,
		Other,
		Last = Other
	};
};

namespace NetworkGlobalEventType {
	enum NetworkGlobalEventType {
		ConnectionListChanged,
		Other,
		Last = Other
	};
};

namespace NetworkVPNEventType {
	enum NetworkVPNEventType {
		LoginBanner,
		Failure,
		Other,
		Last = Other
	};
};

namespace NetworkDeviceEventType {
	enum NetworkDeviceEventType {
		BitRateChanged,
		Failure,
		Other,
		Last = Other
	};
};

namespace NetworkAPEventType {
	enum NetworkAPEventType {
		Discovered,
		Lost,
		SignalStrengthChanged,
		AccessPointChanged,
		Other,
		Last = Other
	};
};

namespace NetworkGlobalManagerFlags {
	enum NetworkGlobalManagerFlags {
		Unknown			= 0x00000000,
		Disconnected		= 0x00000001,
		Connected		= 0x00000002,
		EstablishingLink	= 0x00000004,
		DeactivatingLink	= 0x00000008,
		LinkLocalAccess		= 0x00000010,
		SiteLocalAccess		= 0x00000020,
		GlobalAccess		= 0x00000040,
		Sleeping		= 0x00000080,
		BackendUnavailable	= 0x00000100,
		VPNUnknown		= 0x00000200,
		VPNEstablishingLink	= 0x00000400,
		VPNNeedAuthorization	= 0x00000800,
		VPNConfiguringProtocols	= 0x00001000,
		VPNVerifyingProtocols	= 0x00002000,
		VPNConnected		= 0x00004000,
		VPNFailed		= 0x00008000,
		VPNDisconnected		= 0x00010000,
		GlobalMask		= 0x000001ff,
		VPNMask			= 0x0001fe00
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkGlobalManagerFlags)
};

namespace NetworkDeviceCapabilityFlags {
	enum NetworkDeviceCapabilityFlags {
		None			= 0x00000000,
		Supported		= 0x00000001,
		CanDetectLink		= 0x00000002
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkDeviceCapabilityFlags)
};

namespace NetworkPPPFlags {
	enum NetworkPPPFlags {
		None			= 0x00000000,
		DisableEAP		= 0x00000001,
		DisablePAP		= 0x00000002,
		DisableCHAP		= 0x00000004,
		DisableMSCHAP		= 0x00000008,
		DisableMSCHAPv2		= 0x00000010,
		AllowBSDCompression	= 0x00000020,
		AllowDeflateCompression	= 0x00000040,
		AllowVJCompression	= 0x00000080,
		RequireMPPE		= 0x00000100,
		RequireMPPE128		= 0x00000200,
		StatefulMPPE		= 0x00000400,
		UseHardwareFlowControl	= 0x00000800
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkPPPFlags)
};

namespace NetworkWiFiAPFlags {
	enum NetworkWiFiAPFlags {
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
		KeyManagement80211	= 0x00000400,
		GeneralFlagsMask	= 0x00000001,
		EncryptionFlagsMask	= 0xfffffffe
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkWiFiAPFlags)
};

namespace NetworkWiFiClientFlags {
	enum NetworkWiFiClientFlags {
		None			= 0x00000000,
		CipherWEP40		= 0x00000002,
		CipherWEP104		= 0x00000004,
		CipherTKIP		= 0x00000008,
		CipherCCMP		= 0x00000010,
		CipherWPA		= 0x00000020,
		CipherRSN		= 0x00000040
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkWiFiClientFlags)
};

namespace NetworkVPNType {
	enum NetworkVPNType {
		OpenVPN,
		PPTP,
		StrongSwan,
		VPNC,
		Other,
		Last = Other
	};
};

typedef TQValueList<NetworkVPNType::NetworkVPNType> NetworkVPNTypeList;

namespace NetworkWiFiConnectionCipher {
	enum NetworkWiFiConnectionCipher {
		None,
		CipherWEP40,
		CipherWEP104,
		CipherTKIP,
		CipherCCMP,
		CipherWPA,
		CipherRSN,
		Any
	};
};

typedef TQValueList<NetworkWiFiConnectionCipher::NetworkWiFiConnectionCipher> NetworkWiFiConnectionCipherList;

namespace NetworkWepKeyType {
	enum NetworkWepKeyType {
		Hexadecimal,
		Ascii,
		Passphrase
	};
};

namespace NetworkVLANFlags {
	enum NetworkVLANFlags {
		None			= 0x00000000,
		ReorderPacketHeaders	= 0x00000001,
		UseGVRP			= 0x00000002,
		LooseBinding		= 0x00000004
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkVLANFlags)
};

namespace NetworkWiFiKeyType {
	enum NetworkWiFiKeyType {
		WEP,
		DynamicWEP,
		WPAAdHoc,
		WPAInfrastructure,
		WPAEnterprise,
		Other,
		Last = Other
	};
};

namespace NetworkWiFiAuthType {
	enum NetworkWiFiAuthType {
		Open,
		Shared,
		LEAP,
		Other,
		Last = Other
	};
};

namespace NetworkIEEE8021xType {
	enum NetworkIEEE8021xType {
		None,
		LEAP,
		MD5,
		PAP,
		CHAP,
		MSCHAP,
		MSCHAPV2,
		Fast,
		PSK,
		PAX,
		SAKE,
		GPSK,
		TLS,
		PEAP,
		TTLS,
		SIM,
		GTC,
		OTP
	};
};

typedef TQValueList<NetworkIEEE8021xType::NetworkIEEE8021xType> NetworkIEEE8021xTypeList;

namespace NetworkIEEE8021xFastFlags {
	enum NetworkIEEE8021xFastFlags {
		None			= 0x00000000,
		AllowUnauthenticated	= 0x00000001,
		AllowAuthenticated	= 0x00000002
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkIEEE8021xFastFlags)
};

namespace NetworkWiFiWPAVersionFlags {
	enum NetworkWiFiWPAVersionFlags {
		None			= 0x00000000,
		WPA			= 0x00000001,
		RSN			= 0x00000002,
		Any			= 0x00000003
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkWiFiWPAVersionFlags)
};

namespace NetworkPasswordHandlingFlags {
	enum NetworkPasswordHandlingFlags {
		None			= 0x00000000,
		NoSave			= 0x00000001,
		NoPrompt		= 0x00000002,
		ExternalStorage		= 0x00000004
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkPasswordHandlingFlags)
};

namespace NetworkConnectionStatus {
	enum NetworkConnectionStatus {
		None			= 0x00000000,
		Invalid			= 0x00000001,
		Disconnected		= 0x00000002,
		Connected		= 0x00000004,
		LinkUnavailable		= 0x00000008,
		EstablishingLink	= 0x00000010,
		DeactivatingLink	= 0x00000020,
		ConfiguringProtocols	= 0x00000040,
		Reconnecting		= 0x00000080,
		LinkLost		= 0x00000100,
		LinkLocalAccess		= 0x00000200,
		SiteLocalAccess		= 0x00000400,
		GlobalAccess		= 0x00000800,
		UnManaged		= 0x00001000,
		NeedAuthorization	= 0x00002000,
		Failed			= 0x00004000,
		VerifyingProtocols	= 0x00008000,
		DependencyWait		= 0x00010000
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkConnectionStatus)
};

namespace NetworkIPConfigurationFlags {
	enum NetworkIPConfigurationFlags {
		None					= 0x00000000,
		IPV4DHCPIP				= 0x00000001,
		IPV4DHCPDNS				= 0x00000002,
		IPV4DHCPRoutes				= 0x00000004,
		IPV4LocalOnly				= 0x00000008,
		IPV4StartConnectionSharingServer	= 0x00000010,
		IPV4Disabled				= 0x00000020,
		IPV4MayUseAsDefaultRoute		= 0x00000040,
		IPV6DHCPIP				= 0x00000080,
		IPV6DHCPDNS				= 0x00000100,
		IPV6DHCPRoutes				= 0x00000200,
		IPV6LocalOnly				= 0x00000400,
		IPV6StartConnectionSharingServer	= 0x00000800,
		IPV6Disabled				= 0x00001000,
		IPV6MayUseAsDefaultRoute		= 0x00002000
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(NetworkIPConfigurationFlags)
};

class TDEHW_EXPORT NetworkSearchDomain
{
	public:
		NetworkSearchDomain();
		NetworkSearchDomain(TQString domain, bool ipv6=false);
		~NetworkSearchDomain();

		TQString searchDomain();
		void setSearchDomain(TQString domain, bool ipv6=false);

		bool isIPv4();
		bool isIPv6();

	private:
		TQString m_domain;
		bool m_isIPV6;
};

typedef TQValueList<NetworkSearchDomain> NetworkSearchDomainList;

class TDEHW_EXPORT NetMask
{
	public:
		NetMask();
		NetMask(TQ_UINT32 netmask);
		NetMask(TQ_UINT8* netmask);
		~NetMask();

		void fromCIDRMask(unsigned char mask, bool ipv6=false);
		unsigned char toCIDRMask();
		void fromString(TQString mask);
		TQString toString();

		bool isIPv4();
		bool isIPv6();

	private:
		TQ_UINT32 m_ipv4NetMask;
		TQHostAddress m_ipv6NetMask;
		bool m_isIPV6;
};

class TDEHW_EXPORT MACAddress
{
	public:
		MACAddress();
		MACAddress(NetworkByteList address);
		~MACAddress();

		NetworkByteList address();
		void setAddress(NetworkByteList address);
		TQString toString();
		void fromString(TQString address);
		bool isValid();

	private:
		bool m_isValid;
		NetworkByteList m_macAddress;

		friend bool operator==(const MACAddress &a1, const MACAddress &a2);
		friend bool operator<(const MACAddress &a1, const MACAddress &a2);
};

TDEHW_EXPORT bool operator==(const MACAddress &a1, const MACAddress &a2);
TDEHW_EXPORT bool operator<(const MACAddress &a1, const MACAddress &a2);

typedef TQValueList<MACAddress> MACAddressList;

class TDEHW_EXPORT NetworkSingleIPConfiguration
{
	public:
		NetworkSingleIPConfiguration();
		~NetworkSingleIPConfiguration();

		bool isIPv4();
		bool isIPv6();

	public:
		bool valid;
		TQHostAddress ipAddress;
		NetMask networkMask;
		TQHostAddress gateway;
};

typedef TQValueList<NetworkSingleIPConfiguration> NetworkSingleIPConfigurationList;

class TDEHW_EXPORT NetworkSingleRouteConfiguration
{
	public:
		NetworkSingleRouteConfiguration();
		~NetworkSingleRouteConfiguration();

		bool isIPv4();
		bool isIPv6();

	public:
		bool valid;
		TQHostAddress ipAddress;
		NetMask networkMask;
		TQHostAddress gateway;
		TQ_UINT32 metric;
};

typedef TQValueList<NetworkSingleRouteConfiguration> NetworkSingleRouteConfigurationList;

class TDEHW_EXPORT NetworkIEEE8021xConfiguration
{
	public:
		NetworkIEEE8021xConfiguration();
		~NetworkIEEE8021xConfiguration();

	public:
		bool valid;
		bool allowedValid;
		bool secretsValid;
		NetworkIEEE8021xType::NetworkIEEE8021xType type;
		TQString userName;
		TQString anonymousUserName;
		TQString pacFileName;
		TQByteArray caCertificate;
		TQString additionalCAFilesPath;
		TQString authServerCertSubjectMatch;
		TQStringList alternateAuthServerCertSubjectMatch;
		TQByteArray clientCertificate;
		TQString forcePEAPVersion;
		TQString forcePEAPLabel;
		NetworkIEEE8021xFastFlags::NetworkIEEE8021xFastFlags fastProvisioningFlags;
		NetworkIEEE8021xType::NetworkIEEE8021xType phase2NonEAPAuthMethod;
		NetworkIEEE8021xType::NetworkIEEE8021xType phase2EAPAuthMethod;
		NetworkIEEE8021xTypeList allowedPhase2NonEAPMethods;
		NetworkIEEE8021xTypeList allowedPhase2EAPMethods;
		TQByteArray phase2CaCertificate;
		TQString phase2CaFilesPath;
		TQString phase2AuthServerCertSubjectMatch;
		TQStringList phase2AlternateAuthServerCertSubjectMatch;
		TQByteArray phase2ClientCertificate;
		TQString password;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags passwordFlags;
		TQByteArray binaryPassword;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags binaryPasswordFlags;
		TQByteArray privateKey;
		TQString privateKeyPassword;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags privateKeyPasswordFlags;
		TQByteArray phase2PrivateKey;
		TQString phase2PrivateKeyPassword;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags phase2PrivateKeyPasswordFlags;
		bool forceSystemCaCertificates;
};

class TDEHW_EXPORT NetworkPPPConfiguration
{
	public:
		NetworkPPPConfiguration();
		virtual ~NetworkPPPConfiguration();

	public:
		bool valid;
		bool requireServerAuthentication;
		NetworkPPPFlags::NetworkPPPFlags flags;
		TQ_UINT32 baudRate;
		TQ_UINT32 mru;
		TQ_UINT32 mtu;
		TQ_UINT32 lcpEchoPingInterval;
		TQ_UINT32 lcpEchoFailureThreshold;
};

class TDEHW_EXPORT NetworkPPPOEConfiguration
{
	public:
		NetworkPPPOEConfiguration();
		virtual ~NetworkPPPOEConfiguration();

	public:
		bool valid;
		bool secretsValid;
		TQString networkServiceProvider;
		TQString username;
		TQString password;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags passwordFlags;
};

class TDEHW_EXPORT NetworkSerialConfiguration
{
	public:
		NetworkSerialConfiguration();
		virtual ~NetworkSerialConfiguration();

	public:
		bool valid;
		TQ_UINT32 baudRate;
		TQ_UINT32 byteWidth;
		NetworkParity::NetworkParity parity;
		TQ_UINT32 stopBits;
		TQ_UINT64 txDelay;
};

class TDEHW_EXPORT NetworkCDMAConfiguration
{
	public:
		NetworkCDMAConfiguration();
		virtual ~NetworkCDMAConfiguration();

	public:
		bool valid;
		bool secretsValid;
		TQString providerDataNumber;
		TQString username;
		TQString password;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags passwordFlags;
};

class TDEHW_EXPORT NetworkGSMConfiguration
{
	public:
		NetworkGSMConfiguration();
		virtual ~NetworkGSMConfiguration();

	public:
		bool valid;
		bool secretsValid;
		TQString providerDataNumber;
		TQString username;
		TQString password;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags passwordFlags;
		TQString accessPointName;
		TQString networkID;
		GSMNetworkType::GSMNetworkType networkType;
		TQString pin;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags pinFlags;
		TQ_UINT32 allowedFrequencyBands;
		bool allowRoaming;
};
		

class TDENetworkWiFiSecurityConfiguration
{
	public:
		TDENetworkWiFiSecurityConfiguration();
		~TDENetworkWiFiSecurityConfiguration();

	public:
		bool valid;
		bool secretsValid;
		NetworkWiFiKeyType::NetworkWiFiKeyType keyType;
		NetworkWiFiAuthType::NetworkWiFiAuthType authType;
		NetworkWiFiWPAVersionFlags::NetworkWiFiWPAVersionFlags wpaVersion;
		NetworkWiFiConnectionCipher::NetworkWiFiConnectionCipher cipher;
		TQString wepKey0;
		TQString wepKey1;
		TQString wepKey2;
		TQString wepKey3;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags wepKeyFlags;
		TQ_UINT32 wepKeyIndex;
		NetworkWepKeyType::NetworkWepKeyType wepKeyType;
		NetworkWiFiConnectionCipherList allowedPairWiseCiphers;
		NetworkWiFiConnectionCipherList allowedGroupWiseCiphers;
		TQString psk;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags pskFlags;
		TQString leapUsername;
		TQString leapPassword;
		NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags leapPasswordFlags;
};

class TDEHW_EXPORT NetworkIPConfiguration
{
	public:
		NetworkIPConfiguration();
		~NetworkIPConfiguration();

	public:
		bool valid;
		NetworkIPConfigurationFlags::NetworkIPConfigurationFlags connectionFlags;
		NetworkSingleIPConfigurationList ipConfigurations;
		NetworkSingleRouteConfigurationList routeConfigurations;
		TQHostAddress broadcast;
		TQHostAddress destination;
		NetworkAddressList resolvers;
		NetworkSearchDomainList searchDomains;
		TQString dhcpClientIdentifier;
};

class TDEHW_EXPORT NetworkWiFiDeviceInfo
{
	public:
		NetworkWiFiDeviceInfo();
		~NetworkWiFiDeviceInfo();

	public:
		bool valid;
		MACAddress hwAddress;
		MACAddress permanentHWAddress;
		WiFiMode::WiFiMode operatingMode;
		unsigned int bitrate;
		MACAddress activeAccessPointBSSID;
		NetworkWiFiClientFlags::NetworkWiFiClientFlags wirelessFlags;
};

class TDEHW_EXPORT NetworkDeviceInformation
{
	public:
		NetworkDeviceInformation();
		~NetworkDeviceInformation();

	public:
		bool valid;
		NetworkConnectionStatus::NetworkConnectionStatus statusFlags;
		TQString UUID;
		TQString backendDriver;
		TQString backendDriverVersion;
		TQString firmwareVersion;
		NetworkDeviceCapabilityFlags::NetworkDeviceCapabilityFlags capabilityFlags;
		NetworkIPConfiguration ipConfiguration;
		bool managed;
		bool autoConnect;
		bool firmwareMissing;
		NetworkDeviceType::NetworkDeviceType deviceType;
		TQString activeConnectionUUID;
		NetworkWiFiDeviceInfo wiFiInfo;
};

class TDEHW_EXPORT NetworkHWNeighbor
{
	public:
		NetworkHWNeighbor();
		virtual ~NetworkHWNeighbor();

	public:
		bool valid;
};

class TDEHW_EXPORT NetworkWiFiAPInfo : public NetworkHWNeighbor
{
	public:
		NetworkWiFiAPInfo();
		virtual ~NetworkWiFiAPInfo();

	public:
		TQString friendlySSID() const;

	public:
		TQByteArray SSID;
		NetworkWiFiAPFlags::NetworkWiFiAPFlags wpaFlags;
		NetworkWiFiAPFlags::NetworkWiFiAPFlags rsnFlags;
		unsigned int frequency;
		MACAddress BSSID;
		unsigned int maxBitrate;
		double signalQuality;
};

typedef TQPtrList< NetworkHWNeighbor > NetworkHWNeighborList;

class TDEHW_EXPORT NetworkConnection
{
	public:
		NetworkConnection();
		virtual ~NetworkConnection();

	public:
		NetworkConnectionType::NetworkConnectionType type();

	public:
		TQString UUID;
		TQString friendlyName;
		NetworkIPConfiguration ipConfig;
		MACAddress lockedHWAddress;
		MACAddress manualHWAddress;
		bool readOnly;
		bool autoConnect;
		bool fullDuplex;
		bool requireIPV4;
		bool requireIPV6;
		TQ_UINT32 mtu;
		NetworkIEEE8021xConfiguration eapConfig;
		NetworkPPPConfiguration pppConfig;
		NetworkPPPOEConfiguration pppoeConfig;
		NetworkSerialConfiguration serialConfig;
		TQStringList authorizedUsers;
		TQString masterConnectionUUID;
		NetworkSlaveDeviceType::NetworkSlaveDeviceType slaveType;
		TQDateTime lastKnownConnection;
};

class TDEHW_EXPORT WiredEthernetConnection : public NetworkConnection
{
	public:
		WiredEthernetConnection();
		virtual ~WiredEthernetConnection();
};

class TDEHW_EXPORT WiFiConnection : public NetworkConnection
{
	public:
		WiFiConnection();
		virtual ~WiFiConnection();

	public:
		TQByteArray SSID;
		WiFiMode::WiFiMode operatingMode;
		WiFiFrequencyBand::WiFiFrequencyBand bandRestriction;
		TQ_INT32 channelRestriction;
		TQ_INT32 bitRateRestriction;
		TQ_INT32 powerRestriction;
		MACAddress accessPointRestriction;
		MACAddressList blacklistedBSSIDs;
		MACAddressList heardBSSIDs;
		bool isHiddenNetwork;
		bool securityRequired;
		TDENetworkWiFiSecurityConfiguration securitySettings;
};

class TDEHW_EXPORT WiredInfinibandConnection : public NetworkConnection
{
	public:
		WiredInfinibandConnection();
		virtual ~WiredInfinibandConnection();

	public:
		NetworkInfinibandTransportMode::NetworkInfinibandTransportMode transportMode;
};

class TDEHW_EXPORT VPNConnection : public NetworkConnection
{
	public:
		VPNConnection();
		virtual ~VPNConnection();

	public:
		TQString vpnPluginID;
		TQString lockedUserName;
		NetworkSettingsMap pluginData;
		bool secretsValid;
		NetworkSettingsMap pluginSecrets;
};

class TDEHW_EXPORT WiMaxConnection : public NetworkConnection
{
	public:
		WiMaxConnection();
		virtual ~WiMaxConnection();

	public:
		TQString networkServiceProvider;
};

class TDEHW_EXPORT VLANConnection : public NetworkConnection
{
	public:
		VLANConnection();
		virtual ~VLANConnection();

	public:
		TQString kernelName;
		TQString parentConnectionUUID;
		TQ_UINT32 vlanID;
		NetworkVLANFlags::NetworkVLANFlags vlanFlags;
		NetworkPriorityMap ingressPriorityMap;
		NetworkPriorityMap egressPriorityMap;
};

class TDEHW_EXPORT OLPCMeshConnection : public NetworkConnection
{
	public:
		OLPCMeshConnection();
		virtual ~OLPCMeshConnection();

	public:
		TQByteArray SSID;
		TQ_INT32 channel;
		TQByteArray anycastDHCPHWAddress;
};

class TDEHW_EXPORT BluetoothConnection : public NetworkConnection
{
	public:
		BluetoothConnection();
		virtual ~BluetoothConnection();

	public:
		BluetoothConnectionType::BluetoothConnectionType type;
};

class TDEHW_EXPORT ModemConnection : public NetworkConnection
{
	public:
		ModemConnection();
		virtual ~ModemConnection();

	public:
		ModemConnectionType::ModemConnectionType type;
		NetworkCDMAConfiguration cdmaConfig;
		NetworkGSMConfiguration gsmConfig;
};

typedef TQPtrList< NetworkConnection > NetworkConnectionList;

/**
* INTERNAL CLASS
*/
class TDENetworkEventQueueEvent_Private
{
	public:
		int eventType;
		NetworkGlobalManagerFlags::NetworkGlobalManagerFlags newState;
		NetworkGlobalManagerFlags::NetworkGlobalManagerFlags previousState;
		NetworkConnectionStatus::NetworkConnectionStatus newConnStatus;
		NetworkConnectionStatus::NetworkConnectionStatus previousConnStatus;
		MACAddress BSSID;
		TQString message;
		TQString hwAddress;
		NetworkAPEventType::NetworkAPEventType apevent;
		NetworkDeviceEventType::NetworkDeviceEventType ndevent;
		NetworkVPNEventType::NetworkVPNEventType vpnevent;
		NetworkGlobalEventType::NetworkGlobalEventType globalevent;
};
typedef TQValueList<TDENetworkEventQueueEvent_Private> NetworkEventQueueEvent_PrivateList;

class TDEHW_EXPORT NetworkConnectionManager : public TQObject
{
	Q_OBJECT

	public:
		/**
		*  Constructor.
		*  @param macAddress The MAC address of the hardware device
		*  If an empty MAC address is passed, this object will make global networking backend
		*  methods available exclusively (NetworkDeviceType::BackendOnly).
		*/
		NetworkConnectionManager(TQString macAddress);
		
		/**
		* Destructor.
		*/
		~NetworkConnectionManager();

		/**
		* @return a TQString containing the name of the backend in use
		*/
		virtual TQString backendName() = 0;

		/**
		* @return the type of connection supported by this device
		*/
		virtual NetworkDeviceType::NetworkDeviceType deviceType() = 0;

		/**
		* @return A NetworkGlobalManagerFlags enum value with the current status of the networking backend.
		*/
		virtual NetworkGlobalManagerFlags::NetworkGlobalManagerFlags backendStatus() = 0;

		/**
		* @return A NetworkDeviceInformation object containing the current configuration and status of the network device.
		*/
		virtual NetworkDeviceInformation deviceInformation() = 0;

		/**
		* @return A NetworkDeviceInformation object containing a (limited) current status of the network device.
		* Only the following object fields are populated:
		* statusFlags
		* UUID
		* activeConnectionUUID
		* valid
		*/
		virtual NetworkDeviceInformation deviceStatus() = 0;

		/**
		* Loads all connection information from the configuration backend
		* Secret information must be loaded separately via a call to
		* loadConnectionSecrets(TQString uuid) after this method has been
		* executed at least once.
		*/
		virtual void loadConnectionInformation() = 0;

		/**
		* @param connection a pointer to a NetworkConnection object containing a
		* connection in which to load the values allowed by the backend.
		* This is normally called as part of loadConnectionInformation(), but should
		* manually be called immediately after creation of a new NetworkConnection object.
		*/
		virtual void loadConnectionAllowedValues(NetworkConnection* connection) = 0;

		/**
		* @param uuid a TQString conntaining the UUID of a connection for which to
		* load secrets from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool loadConnectionSecrets(TQString uuid) = 0;

		/**
		* @param connection a pointer to a NetworkConnection object containing a
		* connection to save to the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool saveConnection(NetworkConnection* connection) = 0;

		/**
		* @param uuid a TQString conntaining the UUID of a connection to
		* delete from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool deleteConnection(TQString uuid) = 0;

		/**
		* @param connection a pointer to a NetworkConnection object containing a
		* connection for which to verify integrity of all settings.
		* @param type a pointer to an NetworkConnectionErrorFlags::NetworkConnectionErrorFlags
		* which will be filled with the generic error type code if provided.
		* @param reason a pointer to a NetworkErrorStringMap which will be filled with translated
		* strings containing the reason for the failure if provided.
		* @return true on success, false if invalid settings are detected.
		*/
		virtual bool verifyConnectionSettings(NetworkConnection* connection, NetworkConnectionErrorFlags::NetworkConnectionErrorFlags* type=NULL, NetworkErrorStringMap* reason=NULL) = 0;

		/**
		* Initiates a connection with UUID @param uuid.
		* @return A NetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		* Note that if this manager's type is not NetworkDeviceType::BackendOnly, the connection
		* will be initiated on the internal device specified when this object was created
		*/
		virtual NetworkConnectionStatus::NetworkConnectionStatus initiateConnection(TQString uuid) = 0;

		/**
		* Checks the status of a connection with UUID @param uuid.
		* @return A NetworkConnectionStatus enum value with the current connection status
		*/
		virtual NetworkConnectionStatus::NetworkConnectionStatus checkConnectionStatus(TQString uuid) = 0;

		/**
		* Disconnects a connection with UUID @param uuid.
		* @return A NetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		*/
		virtual NetworkConnectionStatus::NetworkConnectionStatus deactivateConnection(TQString uuid) = 0;

		/**
		* @return a TQStringList object containing all valid network settings
		* Each string has the form "TDENetworkConfigObject::member"
		* If a setting is not in this list, it is not supported by the backend in use
		*/
		virtual TQStringList validSettings() = 0;

		/**
		* @return a NetworkHWNeighborList object containing the result of a site survey;
		* i.e. all nearby access points or devices. This function only returns valid information
		* if the underlying network device supports site surveys.
		*
		* Note that the returned list is internally managed and must not be deleted!
		* Also note that pointers in the list may become invalid on subsequent calls to
		* siteSurvey().
		*/
		virtual NetworkHWNeighborList* siteSurvey() = 0;

		/**
		* @return a TQStringList containing the UUIDs of all physical devices used by the connection
		* with UUID @param uuid.
		* This function may return an empty list if the connection is inactive, this behaviour is
		* dependend on the specific network backend in use.
		*/
		virtual TQStringList connectionPhysicalDeviceUUIDs(TQString uuid) = 0;

		/**
		* @return a NetworkVPNTypeList object containing all supported VPN types
		* If a type is not in this list, it is not supported by the backend in use
		*/
		virtual NetworkVPNTypeList availableVPNTypes() = 0;

		/**
		* @return true if networking is enabled, false if not.
		*/
		virtual bool networkingEnabled() = 0;

		/**
		* @param enable true to enable networking, false to disable it.
		* @return true on success, false on failure.
		*/
		virtual bool enableNetworking(bool enable) = 0;

		/**
		* @return true if WiFi hardware is enabled, false if not.
		*/
		virtual bool wiFiHardwareEnabled() = 0;

		/**
		* @param enable true to enable WiFi, false to disable it.
		* @return true on success, false on failure.
		*/
		virtual bool enableWiFi(bool enable) = 0;

		/**
		* @return true if WiFi is enabled, false if not.
		*/
		virtual bool wiFiEnabled() = 0;

		/**
		* @return a list of UUIDs of the default network devices, or an empty list if no such devices exist.
		* The default network devices are normally the devices holding the highest priority default route.
		*/
		virtual TQStringList defaultNetworkDevices() = 0;

	signals:
		/**
		* Emitted whenever the state of the system's connection changes
		* If previous state data was unavailable, @param previousState will contain NetworkConnectionStatus::Invalid
		*/
		void networkConnectionStateChanged(TDEHW::NetworkGlobalManagerFlags::NetworkGlobalManagerFlags newState, TDEHW::NetworkGlobalManagerFlags::NetworkGlobalManagerFlags previousState);

		/**
		* Emitted whenever the state of a device changes
		* If previous state data was unavailable, @param previousState will contain NetworkConnectionStatus::Invalid
		* If the global connection state has changed, @param hwAddress will be empty, otherwise it will contain the MAC address
		* of the networking hardware that has changed state.
		*/
		void networkDeviceStateChanged(TDEHW::NetworkConnectionStatus::NetworkConnectionStatus newState, TDEHW::NetworkConnectionStatus::NetworkConnectionStatus previousState, TQString hwAddress);

		/**
		* Emitted whenever the status of a wireless access point changes
		* The event type that caused the signal is available in @param event
		*/
		void accessPointStatusChanged(MACAddress BSSID, TDEHW::NetworkAPEventType::NetworkAPEventType event);

		/**
		* Emitted whenever a network device event occurs
		* The event type that caused the signal is available in @param event
		* @param message contains additional information if available
		*/
		void networkDeviceEvent(TDEHW::NetworkDeviceEventType::NetworkDeviceEventType event, TQString message);

		/**
		* Emitted whenever a VPN-related event occurs
		* The event type that caused the signal is available in @param event
		* @param message contains additional information if available
		*/
		void vpnEvent(TDEHW::NetworkVPNEventType::NetworkVPNEventType event, TQString message);

		/**
		* Emitted whenever a global network management event occurs
		* The event type that caused the signal is available in @param event
		*/
		void networkManagementEvent(TDEHW::NetworkGlobalEventType::NetworkGlobalEventType event);

	public:
		/**
		* @return a NetworkConnectionList object containing a list of all
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
		virtual NetworkConnectionList* connections();

		/**
		* @return the MAC address of this device
		*/
		TQString deviceMACAddress();

		/**
		* @return a pointer to a NetworkConnection object with the specified @param uuid,
		* or a NULL pointer if no such connection exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		NetworkConnection* findConnectionByUUID(TQString uuid);

		/**
		* @return a pointer to a NetworkDevice object with the specified @param uuid,
		* or a NULL pointer if no such device exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		NetworkDevice* findDeviceByUUID(TQString uuid);

		/**
		* @return a pointer to a NetworkWiFiAPInfo object with the specified @param bssid,
		* or a NULL pointer if no such access point exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		NetworkWiFiAPInfo* findAccessPointByBSSID(MACAddress bssid);

		/**
		* @return a string containing the friendly name of the connection type @param type given
		*/
		static TQString friendlyConnectionTypeName(NetworkConnectionType::NetworkConnectionType type);

		/**
		* @return true if @param address is valid, false if not
		*/
		static bool validateIPAddress(TQHostAddress address);

		/**
		* @return true if @param netmask is valid, false if not
		*/
		static bool validateIPNeworkMask(TQHostAddress netmask);

	protected:
		/**
		* @internal Safely clears out the master connection list and deletes all member objects
		*/
		void clearNetworkConnectionList();
		
		/**
		* @internal Safely clears out the neighboring devices list and deletes all member objects
		*/
		void clearNetworkHWNeighborList();

		/**
		* @internal This method must be called by the network backend whenever a connection changes state
		* It emits the appropriate signals to notify client applications of the state change
		*/
		void internalNetworkConnectionStateChanged(NetworkGlobalManagerFlags::NetworkGlobalManagerFlags newState);

		/**
		* @internal This method must be called by the network backend whenever a device changes state
		* It emits the appropriate signals to notify client applications of the state change
		*/
		void internalNetworkDeviceStateChanged(NetworkConnectionStatus::NetworkConnectionStatus newState, TQString hwAddress=TQString::null);

		/**
		* @internal This method must be called by the network backend whenever a wireless access point changes state
		* It emits the appropriate signals to notify client applications of the state change
		*/
		void internalAccessPointStatusChanged(MACAddress BSSID, NetworkAPEventType::NetworkAPEventType event);

		/**
		* @internal This method must be called by the network backend whenever a device event occurs
		* It emits the appropriate signals to notify client applications of the network device event
		*/
		void internalNetworkDeviceEvent(NetworkDeviceEventType::NetworkDeviceEventType event, TQString message);

		/**
		* @internal This method must be called by the network backend whenever a VPN event occurs
		* It emits the appropriate signals to notify client applications of the network device event
		*/
		void internalVpnEvent(NetworkVPNEventType::NetworkVPNEventType event, TQString message);

		/**
		* @internal This method must be called by the network backend whenever it changes state
		* It emits the appropriate signals to notify client applications of the state change
		*/
		void internalNetworkManagementEvent(NetworkGlobalEventType::NetworkGlobalEventType event);

	protected slots:
		void emitQueuedSignals();

	protected:
		NetworkConnectionList* m_connectionList;
		NetworkHWNeighborList* m_hwNeighborList;
		TQString m_macAddress;
		NetworkGlobalManagerFlags::NetworkGlobalManagerFlags m_prevConnectionStatus;
		TQMap<TQString, NetworkConnectionStatus::NetworkConnectionStatus> m_prevDeviceStatus;
		TQTimer* m_emissionTimer;
		NetworkEventQueueEvent_PrivateList m_globalEventQueueEventList;
};

class TDEHW_EXPORT GlobalNetworkManager : public TQObject
{
	Q_OBJECT

	public:
		/**
		*  Constructor.
		*/
		GlobalNetworkManager();
		
		/**
		* Destructor.
		*/
		~GlobalNetworkManager();

		/**
		* @return a TQString containing the name of the backend in use
		*/
		virtual TQString backendName();

		/**
		* @return A NetworkGlobalManagerFlags enum value with the current status of the networking backend.
		*/
		virtual NetworkGlobalManagerFlags::NetworkGlobalManagerFlags backendStatus();

		/**
		* Loads all connection information from the configuration backend
		* Secret information must be loaded separately via a call to
		* loadConnectionSecrets(TQString uuid) after this method has been
		* executed at least once.
		*/
		virtual void loadConnectionInformation();

		/**
		* @param connection a pointer to a NetworkConnection object containing a
		* connection in which to load the values allowed by the backend.
		* This is normally called as part of loadConnectionInformation(), but should
		* manually be called immediately after creation of a new NetworkConnection object.
		*/
		virtual void loadConnectionAllowedValues(NetworkConnection* connection);

		/**
		* @param uuid a TQString conntaining the UUID of a connection for which to
		* load secrets from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool loadConnectionSecrets(TQString uuid);

		/**
		* @param connection a pointer to a NetworkConnection object containing a
		* connection to save to the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool saveConnection(NetworkConnection* connection);

		/**
		* @param uuid a TQString conntaining the UUID of a connection to
		* delete from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool deleteConnection(TQString uuid);

		/**
		* @param connection a pointer to a NetworkConnection object containing a
		* connection for which to verify integrity of all settings.
		* @param type a pointer to an NetworkConnectionErrorFlags::NetworkConnectionErrorFlags
		* which will be filled with the generic error type code if provided.
		* @param reason a pointer to a NetworkErrorStringMap which will be filled with translated
		* strings containing the reason for the failure if provided.
		* @return true on success, false if invalid settings are detected.
		*/
		virtual bool verifyConnectionSettings(NetworkConnection* connection, NetworkConnectionErrorFlags::NetworkConnectionErrorFlags* type=NULL, NetworkErrorStringMap* reason=NULL);

		/**
		* Initiates a connection with UUID @param uuid.
		* @return A NetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		*/
		virtual NetworkConnectionStatus::NetworkConnectionStatus initiateConnection(TQString uuid);

		/**
		* Checks the status of a connection with UUID @param uuid.
		* @return A NetworkConnectionStatus enum value with the current connection status
		*/
		virtual NetworkConnectionStatus::NetworkConnectionStatus checkConnectionStatus(TQString uuid);

		/**
		* Disconnects a connection with UUID @param uuid.
		* @return A NetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		*/
		virtual NetworkConnectionStatus::NetworkConnectionStatus deactivateConnection(TQString uuid);

		/**
		* @return a TQStringList object containing all valid network settings
		* Each string has the form "TDENetworkConfigObject::member"
		* If a setting is not in this list, it is not supported by the backend in use
		*/
		virtual TQStringList validSettings();

		/**
		* @return a NetworkHWNeighborList object containing the result of a site survey;
		* i.e. all nearby access points or devices. This function only returns valid information
		* if the underlying network device supports site surveys.
		*
		* Note that the returned list is internally managed and must not be deleted!
		* Also note that pointers in the list may become invalid on subsequent calls to
		* siteSurvey().
		*/
		virtual NetworkHWNeighborList* siteSurvey();

		/**
		* @return a TQStringList containing the UUIDs of all physical devices used by the connection
		* with UUID @param uuid.
		* This function may return an empty list if the connection is inactive, this behaviour is
		* dependend on the specific network backend in use.
		*/
		virtual TQStringList connectionPhysicalDeviceUUIDs(TQString uuid);

		/**
		* @return a NetworkVPNTypeList object containing all supported VPN types
		* If a type is not in this list, it is not supported by the backend in use
		*/
		virtual NetworkVPNTypeList availableVPNTypes();

		/**
		* @return true if networking is enabled, false if not.
		*/
		virtual bool networkingEnabled();

		/**
		* @param enable true to enable networking, false to disable it.
		* @return true on success, false on failure.
		*/
		virtual bool enableNetworking(bool enable);

		/**
		* @return true if WiFi hardware is enabled, false if not.
		*/
		virtual bool wiFiHardwareEnabled();

		/**
		* @param enable true to enable WiFi, false to disable it.
		* @return true on success, false on failure.
		*/
		virtual bool enableWiFi(bool enable);

		/**
		* @return true if WiFi is enabled, false if not.
		*/
		virtual bool wiFiEnabled();

		/**
		* @return a list of UUIDs of the default network devices, or an empty list if no such devices exist.
		* The default network devices are normally the devices holding the highest priority default route.
		*/
		virtual TQStringList defaultNetworkDevices();

	signals:
		/**
		* Emitted whenever the state of the system's connection changes
		* If previous state data was unavailable, @param previousState will contain NetworkConnectionStatus::Invalid
		*/
		void networkConnectionStateChanged(TDEHW::NetworkGlobalManagerFlags::NetworkGlobalManagerFlags newState, TDEHW::NetworkGlobalManagerFlags::NetworkGlobalManagerFlags previousState);

		/**
		* Emitted whenever the state of a device changes
		* If previous state data was unavailable, @param previousState will contain NetworkConnectionStatus::Invalid
		* If the global connection state has changed, @param hwAddress will be empty, otherwise it will contain the MAC address
		* of the networking hardware that has changed state.
		*/
		void networkDeviceStateChanged(TDEHW::NetworkConnectionStatus::NetworkConnectionStatus newState, TDEHW::NetworkConnectionStatus::NetworkConnectionStatus previousState, TQString hwAddress);

		/**
		* Emitted whenever the status of a wireless access point changes
		* The event type that caused the signal is available in @param event
		*/
		void accessPointStatusChanged(MACAddress BSSID, TDEHW::NetworkAPEventType::NetworkAPEventType event);

		/**
		* Emitted whenever a VPN-related event occurs
		* The event type that caused the signal is available in @param event
		* @param message contains additional information if available
		*/
		void vpnEvent(TDEHW::NetworkVPNEventType::NetworkVPNEventType event, TQString message);

		/**
		* Emitted whenever a global network management event occurs
		* The event type that caused the signal is available in @param event
		*/
		void networkManagementEvent(TDEHW::NetworkGlobalEventType::NetworkGlobalEventType event);

	public:
		/**
		* @return a NetworkConnectionList object containing a list of all
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
		virtual NetworkConnectionList* connections();

		/**
		* @return a pointer to a NetworkConnection object with the specified @param uuid,
		* or a NULL pointer if no such connection exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		NetworkConnection* findConnectionByUUID(TQString uuid);

		/**
		* @return a pointer to a NetworkDevice object with the specified @param uuid,
		* or a NULL pointer if no such device exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		NetworkDevice* findDeviceByUUID(TQString uuid);

		/**
		* @return a pointer to a NetworkWiFiAPInfo object with the specified @param bssid,
		* or a NULL pointer if no such access point exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		NetworkWiFiAPInfo* findAccessPointByBSSID(MACAddress bssid);

	private:
		NetworkConnectionManager* m_internalConnectionManager;
};

} // namespace TDEHW

#endif // _TDENETWORKCONNECTIONS_H
