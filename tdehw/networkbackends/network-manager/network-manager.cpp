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

#include <tqdir.h>

#include <tqdbusmessage.h>

#include "tdeconfig.h"
#include "hardwaredevices.h"
#include "networkdevice.h"

#include "network-manager.h"
#include "network-manager_p.h"

// #define DEBUG_NETWORK_MANAGER_COMMUNICATIONS

#define PRINT_ERROR(x) printf("[TDE NM Backend ERROR] [%s:%d] %s\n", __FILE__, __LINE__, x.ascii());

#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
#define PRINT_WARNING(x) printf("[TDE NM Backend WARNING] [%s:%d] %s\n", __FILE__, __LINE__, x.ascii());
#else
#define PRINT_WARNING(x)
#endif

#define UPDATE_STRING_SETTING_IF_VALID(string, key, settingsMap)	if (!string.isNull()) settingsMap[key] = convertDBUSDataToVariantData(TQT_DBusData::fromString(string));	\
									else settingsMap.remove(key);

#define NM_ASYNC_TIMEOUT_MS 1000
// Give the user 5 minutes to authenticate to DBUS before timing out
#define NM_ASYNC_SECRETS_INTERACTION_TIMEOUT_MS (5*60*1000)

// #define WAIT_FOR_OPERATION_BEFORE_RETURNING 1
#define USE_ASYNC_DBUS_CONNECTION_COMMAND_CALLS 1

using namespace TDEHW;

TQ_UINT32 reverseIPV4ByteOrder(TQ_UINT32 address) {
	TQ_UINT32 ret;
	unsigned char valuearray[4];
	valuearray[0] = (address & 0xff000000) >> 24;
	valuearray[1] = (address & 0x00ff0000) >> 16;
	valuearray[2] = (address & 0x0000ff00) >> 8;
	valuearray[3] = (address & 0x000000ff) >> 0;
	ret = 0;
	ret = ret | (valuearray[0] << 0);
	ret = ret | (valuearray[1] << 8);
	ret = ret | (valuearray[2] << 16);
	ret = ret | (valuearray[3] << 24);
	return ret;
}

TQT_DBusData convertDBUSDataToVariantData(TQT_DBusData object) {
	TQT_DBusVariant variant;
	variant.value = object;
	variant.signature = variant.value.buildDBusSignature();
	return TQT_DBusData::fromVariant(variant);
}

void printDBUSObjectStructure(TQT_DBusData object, int level=0, TQString mapKey=TQString::null) {
	int i;
	TQString levelIndent = "";
	for (i=0; i<level; i++) {
		levelIndent = levelIndent + " ";
	}
	TQCString signature = object.buildDBusSignature();

	if (object.type() == TQT_DBusData::String) {
		printf("%s%s\t%s%s'%s'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", object.toString().ascii()); fflush(stdout);
	}
	else if (object.type() == TQT_DBusData::Bool) {
		printf("%s%s\t%s%s'%s'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", (object.toBool())?"true":"false"); fflush(stdout);
	}
	else if (object.type() == TQT_DBusData::Byte) {
		printf("%s%s\t%s%s'%d'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", object.toByte()); fflush(stdout);
	}
	else if (object.type() == TQT_DBusData::Int16) {
		printf("%s%s\t%s%s'%d'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", object.toInt16()); fflush(stdout);
	}
	else if (object.type() == TQT_DBusData::UInt16) {
		printf("%s%s\t%s%s'%d'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", object.toUInt16()); fflush(stdout);
	}
	else if (object.type() == TQT_DBusData::Int32) {
		printf("%s%s\t%s%s'%d'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", object.toInt32()); fflush(stdout);
	}
	else if (object.type() == TQT_DBusData::UInt32) {
		printf("%s%s\t%s%s'%d'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", object.toUInt32()); fflush(stdout);
	}
	else if (object.type() == TQT_DBusData::Int64) {
		printf("%s%s\t%s%s'%lld'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", object.toInt64()); fflush(stdout);
	}
	else if (object.type() == TQT_DBusData::UInt64) {
		printf("%s%s\t%s%s'%lld'\n", levelIndent.ascii(), signature.data(), (mapKey.isNull())?"":mapKey.ascii(), (mapKey.isNull())?"":" = ", object.toUInt64()); fflush(stdout);
	}
	else {
		printf("%s%s\n", levelIndent.ascii(), signature.data()); fflush(stdout);
	}

	if (object.type() == TQT_DBusData::Map) {
		// HACK
		// NetworkManager currently uses string key maps exclusively as far as I can tell, so this should be adequate for the time being
		TQMap<TQString, TQT_DBusData> outerMap = object.toStringKeyMap().toTQMap();
		TQMap<TQString, TQT_DBusData>::const_iterator it;
        	for (it = outerMap.begin(); it != outerMap.end(); ++it) {
			printDBUSObjectStructure(*it, level+1, it.key());
		}
	}
	else if (object.type() == TQT_DBusData::List) {
		TQT_DBusDataValueList valueList = object.toTQValueList();
		TQT_DBusDataValueList::const_iterator it;
		for (it = valueList.begin(); it != valueList.end(); ++it) {
			printDBUSObjectStructure(*it, level+1);
		}
	}
	else if (object.type() == TQT_DBusData::Variant) {
		TQT_DBusVariant dataValueVariant = object.toVariant();
		TQT_DBusData dataValue = dataValueVariant.value;
		printDBUSObjectStructure(dataValue, level+1, mapKey);
	}
}

NetworkGlobalManagerFlags::NetworkGlobalManagerFlags nmGlobalStateToTDEGlobalState(TQ_UINT32 nmType) {
	NetworkGlobalManagerFlags::NetworkGlobalManagerFlags ret = NetworkGlobalManagerFlags::Unknown;

	if (nmType == NM_STATE_UNKNOWN) {
		ret |= NetworkGlobalManagerFlags::Unknown;
	}
	else if (nmType == NM_STATE_ASLEEP) {
		ret |= NetworkGlobalManagerFlags::Disconnected;
		ret |= NetworkGlobalManagerFlags::Sleeping;
	}
	else if (nmType == NM_STATE_DISCONNECTED) {
		ret |= NetworkGlobalManagerFlags::Disconnected;
	}
	else if (nmType == NM_STATE_DISCONNECTING) {
		ret |= NetworkGlobalManagerFlags::Connected;
		ret |= NetworkGlobalManagerFlags::DeactivatingLink;
	}
	else if (nmType == NM_STATE_CONNECTING) {
		ret |= NetworkGlobalManagerFlags::Disconnected;
		ret |= NetworkGlobalManagerFlags::EstablishingLink;
	}
	else if (nmType == NM_STATE_CONNECTED_LOCAL) {
		ret |= NetworkGlobalManagerFlags::Connected;
		ret |= NetworkGlobalManagerFlags::LinkLocalAccess;
	}
	else if (nmType == NM_STATE_CONNECTED_SITE) {
		ret |= NetworkGlobalManagerFlags::Connected;
		ret |= NetworkGlobalManagerFlags::SiteLocalAccess;
	}
	else if (nmType == NM_STATE_CONNECTED_GLOBAL) {
		ret |= NetworkGlobalManagerFlags::Connected;
		ret |= NetworkGlobalManagerFlags::GlobalAccess;
	}

	return ret;
}

NetworkGlobalManagerFlags::NetworkGlobalManagerFlags nmVPNStateToTDEGlobalState(TQ_UINT32 nmType) {
	NetworkGlobalManagerFlags::NetworkGlobalManagerFlags ret = NetworkGlobalManagerFlags::Unknown;

	if (nmType == NM_VPN_STATE_UNKNOWN) {
		ret |= NetworkGlobalManagerFlags::VPNUnknown;
	}
	else if (nmType == NM_VPN_STATE_PREPARE) {
		ret |= NetworkGlobalManagerFlags::VPNDisconnected;
		ret |= NetworkGlobalManagerFlags::VPNEstablishingLink;
	}
	else if (nmType == NM_VPN_STATE_NEED_AUTH) {
		ret |= NetworkGlobalManagerFlags::VPNDisconnected;
		ret |= NetworkGlobalManagerFlags::VPNNeedAuthorization;
	}
	else if (nmType == NM_VPN_STATE_CONNECT) {
		ret |= NetworkGlobalManagerFlags::VPNDisconnected;
		ret |= NetworkGlobalManagerFlags::VPNConfiguringProtocols;
	}
	else if (nmType == NM_VPN_STATE_IP_CONFIG_GET) {
		ret |= NetworkGlobalManagerFlags::VPNDisconnected;
		ret |= NetworkGlobalManagerFlags::VPNVerifyingProtocols;
	}
	else if (nmType == NM_VPN_STATE_ACTIVATED) {
		ret |= NetworkGlobalManagerFlags::VPNConnected;
	}
	else if (nmType == NM_VPN_STATE_FAILED) {
		ret |= NetworkGlobalManagerFlags::VPNDisconnected;
		ret |= NetworkGlobalManagerFlags::VPNFailed;
	}
	else if (nmType == NM_VPN_STATE_DISCONNECTED) {
		ret |= NetworkGlobalManagerFlags::VPNDisconnected;
	}

	return ret;
}

NetworkConnectionStatus::NetworkConnectionStatus nmDeviceStateToTDEDeviceState(TQ_UINT32 nmType) {
	NetworkConnectionStatus::NetworkConnectionStatus ret = NetworkConnectionStatus::None;

	if (nmType == NM_DEVICE_STATE_UNKNOWN) {
		ret |= NetworkConnectionStatus::Invalid;
	}
	else if (nmType == NM_DEVICE_STATE_UNMANAGED) {
		ret |= NetworkConnectionStatus::UnManaged;
	}
	else if (nmType == NM_DEVICE_STATE_UNAVAILABLE) {
		ret |= NetworkConnectionStatus::Disconnected;
		ret |= NetworkConnectionStatus::LinkUnavailable;
	}
	else if (nmType == NM_DEVICE_STATE_DISCONNECTED) {
		ret |= NetworkConnectionStatus::Disconnected;
	}
	else if (nmType == NM_DEVICE_STATE_PREPARE) {
		ret |= NetworkConnectionStatus::Disconnected;
		ret |= NetworkConnectionStatus::EstablishingLink;
	}
	else if (nmType == NM_DEVICE_STATE_CONFIG) {
		ret |= NetworkConnectionStatus::Disconnected;
		ret |= NetworkConnectionStatus::EstablishingLink;
	}
	else if (nmType == NM_DEVICE_STATE_NEED_AUTH) {
		ret |= NetworkConnectionStatus::Disconnected;
		ret |= NetworkConnectionStatus::NeedAuthorization;
	}
	else if (nmType == NM_DEVICE_STATE_IP_CONFIG) {
		ret |= NetworkConnectionStatus::Disconnected;
		ret |= NetworkConnectionStatus::ConfiguringProtocols;
	}
	else if (nmType == NM_DEVICE_STATE_IP_CHECK) {
		ret |= NetworkConnectionStatus::Disconnected;
		ret |= NetworkConnectionStatus::VerifyingProtocols;
	}
	else if (nmType == NM_DEVICE_STATE_SECONDARIES) {
		ret |= NetworkConnectionStatus::Disconnected;
		ret |= NetworkConnectionStatus::DependencyWait;
	}
	else if (nmType == NM_DEVICE_STATE_ACTIVATED) {
		ret |= NetworkConnectionStatus::Connected;
	}
	else if (nmType == NM_DEVICE_STATE_DEACTIVATING) {
		ret |= NetworkConnectionStatus::Connected;
		ret |= NetworkConnectionStatus::DeactivatingLink;
	}
	else if (nmType == NM_DEVICE_STATE_FAILED) {
		ret |= NetworkConnectionStatus::Disconnected;
		ret |= NetworkConnectionStatus::Failed;
	}

	return ret;
}

NetworkDeviceType::NetworkDeviceType NetworkConnectionManager_BackendNM::nmDeviceTypeToTDEDeviceType(TQ_UINT32 nmType) {
	NetworkDeviceType::NetworkDeviceType ret = NetworkDeviceType::Other;

	if (nmType == NM_DEVICE_TYPE_UNKNOWN) {
		ret = NetworkDeviceType::Other;
	}
	else if (nmType == NM_DEVICE_TYPE_ETHERNET) {
		ret = NetworkDeviceType::WiredEthernet;
	}
	else if (nmType == NM_DEVICE_TYPE_WIFI) {
		ret = NetworkDeviceType::WiFi;
	}
	else if (nmType == NM_DEVICE_TYPE_UNUSED1) {
	}
	else if (nmType == NM_DEVICE_TYPE_UNUSED2) {
	}
	else if (nmType == NM_DEVICE_TYPE_BT) {
		ret = NetworkDeviceType::Bluetooth;
	}
	else if (nmType == NM_DEVICE_TYPE_OLPC_MESH) {
		ret = NetworkDeviceType::OLPCMesh;
	}
	else if (nmType == NM_DEVICE_TYPE_WIMAX) {
		ret = NetworkDeviceType::WiMax;
	}
	else if (nmType == NM_DEVICE_TYPE_MODEM) {
		ret = NetworkDeviceType::Modem;
	}
	else if (nmType == NM_DEVICE_TYPE_INFINIBAND) {
		ret = NetworkDeviceType::Infiniband;
	}
	else if (nmType == NM_DEVICE_TYPE_BOND) {
		ret = NetworkDeviceType::Bond;
	}
	else if (nmType == NM_DEVICE_TYPE_VLAN) {
		ret = NetworkDeviceType::VLAN;
	}
	else if (nmType == NM_DEVICE_TYPE_ADSL) {
		ret = NetworkDeviceType::ADSL;
	}

	return ret;
}

NetworkConnectionType::NetworkConnectionType nmConnectionTypeToTDEConnectionType(TQString nm) {
	NetworkConnectionType::NetworkConnectionType ret = NetworkConnectionType::Other;

	if (nm.lower() == "802-3-ethernet") {
		ret = NetworkConnectionType::WiredEthernet;
	}
	else if (nm.lower() == "infiniband") {
		ret = NetworkConnectionType::Infiniband;
	}
	else if (nm.lower() == "802-11-wireless") {
		ret = NetworkConnectionType::WiFi;
	}
	else if (nm.lower() == "vpn") {
		ret = NetworkConnectionType::VPN;
	}
	else if (nm.lower() == "wimax") {
		ret = NetworkConnectionType::WiMax;
	}
	else if (nm.lower() == "vlan") {
		ret = NetworkConnectionType::VLAN;
	}
	else if (nm.lower() == "802-11-olpc-mesh") {
		ret = NetworkConnectionType::OLPCMesh;
	}
	else if (nm.lower() == "bluetooth") {
		ret = NetworkConnectionType::Bluetooth;
	}
	else if (nm.lower() == "cdma") {
		ret = NetworkConnectionType::Modem;
	}
	else if (nm.lower() == "gsm") {
		ret = NetworkConnectionType::Modem;
	}

	return ret;
}

TQString tdeConnectionTypeToNMConnectionType(NetworkConnectionType::NetworkConnectionType type, ModemConnectionType::ModemConnectionType modemType=ModemConnectionType::Other) {
	TQString ret;

	if (type == NetworkConnectionType::WiredEthernet) {
		ret = "802-3-ethernet";
	}
	else if (type == NetworkConnectionType::Infiniband) {
		ret = "infiniband";
	}
	else if (type == NetworkConnectionType::WiFi) {
		ret = "802-11-wireless";
	}
	else if (type == NetworkConnectionType::VPN) {
		ret = "vpn";
	}
	else if (type == NetworkConnectionType::WiMax) {
		ret = "wimax";
	}
	else if (type == NetworkConnectionType::VLAN) {
		ret = "vlan";
	}
	else if (type == NetworkConnectionType::OLPCMesh) {
		ret = "802-11-olpc-mesh";
	}
	else if (type == NetworkConnectionType::Bluetooth) {
		ret = "bluetooth";
	}
	else if (type == NetworkConnectionType::Modem) {
		if (modemType == ModemConnectionType::CDMA) {
			ret = "cdma";
		}
		else if (modemType == ModemConnectionType::GSM) {
			ret = "gsm";
		}
	}

	return ret;
}

NetworkIEEE8021xType::NetworkIEEE8021xType nmEAPTypeToTDEEAPType(TQString nm) {
	NetworkIEEE8021xType::NetworkIEEE8021xType ret = NetworkIEEE8021xType::None;

	if (nm.lower() == "") {
		ret = NetworkIEEE8021xType::None;
	}
	else if (nm.lower() == "leap") {
		ret = NetworkIEEE8021xType::LEAP;
	}
	else if (nm.lower() == "md5") {
		ret = NetworkIEEE8021xType::MD5;
	}
	else if (nm.lower() == "pap") {
		ret = NetworkIEEE8021xType::PAP;
	}
	else if (nm.lower() == "chap") {
		ret = NetworkIEEE8021xType::CHAP;
	}
	else if (nm.lower() == "mschap") {
		ret = NetworkIEEE8021xType::MSCHAP;
	}
	else if (nm.lower() == "mschapv2") {
		ret = NetworkIEEE8021xType::MSCHAPV2;
	}
	else if (nm.lower() == "fast") {
		ret = NetworkIEEE8021xType::Fast;
	}
	else if (nm.lower() == "psk") {
		ret = NetworkIEEE8021xType::PSK;
	}
	else if (nm.lower() == "pax") {
		ret = NetworkIEEE8021xType::PAX;
	}
	else if (nm.lower() == "sake") {
		ret = NetworkIEEE8021xType::SAKE;
	}
	else if (nm.lower() == "gpsk") {
		ret = NetworkIEEE8021xType::GPSK;
	}
	else if (nm.lower() == "tls") {
		ret = NetworkIEEE8021xType::TLS;
	}
	else if (nm.lower() == "peap") {
		ret = NetworkIEEE8021xType::PEAP;
	}
	else if (nm.lower() == "ttls") {
		ret = NetworkIEEE8021xType::TTLS;
	}
	else if (nm.lower() == "sim") {
		ret = NetworkIEEE8021xType::SIM;
	}
	else if (nm.lower() == "gtc") {
		ret = NetworkIEEE8021xType::GTC;
	}
	else if (nm.lower() == "otp") {
		ret = NetworkIEEE8021xType::OTP;
	}
	else {
		PRINT_ERROR(TQString("unknown EAP type %s requested in existing connection").arg(nm.lower()))
	}

	return ret;
}

TQString tdeEAPTypeToNMEAPType(NetworkIEEE8021xType::NetworkIEEE8021xType eaptype) {
	TQString ret = "";

	if (eaptype == NetworkIEEE8021xType::None) {
		ret = "";
	}
	else if (eaptype == NetworkIEEE8021xType::LEAP) {
		ret = "leap";
	}
	else if (eaptype == NetworkIEEE8021xType::MD5) {
		ret = "md5";
	}
	else if (eaptype == NetworkIEEE8021xType::PAP) {
		ret = "pap";
	}
	else if (eaptype == NetworkIEEE8021xType::CHAP) {
		ret = "chap";
	}
	else if (eaptype == NetworkIEEE8021xType::MSCHAP) {
		ret = "mschap";
	}
	else if (eaptype == NetworkIEEE8021xType::MSCHAPV2) {
		ret = "mschapv2";
	}
	else if (eaptype == NetworkIEEE8021xType::Fast) {
		ret = "fast";
	}
	else if (eaptype == NetworkIEEE8021xType::PSK) {
		ret = "psk";
	}
	else if (eaptype == NetworkIEEE8021xType::PAX) {
		ret = "pax";
	}
	else if (eaptype == NetworkIEEE8021xType::SAKE) {
		ret = "sake";
	}
	else if (eaptype == NetworkIEEE8021xType::GPSK) {
		ret = "gpsk";
	}
	else if (eaptype == NetworkIEEE8021xType::TLS) {
		ret = "tls";
	}
	else if (eaptype == NetworkIEEE8021xType::PEAP) {
		ret = "peap";
	}
	else if (eaptype == NetworkIEEE8021xType::TTLS) {
		ret = "ttls";
	}
	else if (eaptype == NetworkIEEE8021xType::SIM) {
		ret = "sim";
	}
	else if (eaptype == NetworkIEEE8021xType::GTC) {
		ret = "gtc";
	}
	else if (eaptype == NetworkIEEE8021xType::OTP) {
		ret = "otp";
	}
	else {
		PRINT_ERROR(TQString("unknown TDE EAP type %d requested in new or updated connection").arg(eaptype))
	}

	return ret;
}

NetworkIEEE8021xFastFlags::NetworkIEEE8021xFastFlags nmEAPFastFlagsToTDEEAPFastFlags(TQString nm) {
	NetworkIEEE8021xFastFlags::NetworkIEEE8021xFastFlags ret = NetworkIEEE8021xFastFlags::AllowUnauthenticated | NetworkIEEE8021xFastFlags::AllowAuthenticated;

	unsigned int nm_int = nm.toUInt();
	if (nm_int == NM_EAP_FAST_PROVISIONING_DISABLED) {
		ret = NetworkIEEE8021xFastFlags::None;
	}
	else if (nm_int == NM_EAP_FAST_PROVISIONING_UNAUTHONLY) {
		ret = NetworkIEEE8021xFastFlags::AllowUnauthenticated;
	}
	else if (nm_int == NM_EAP_FAST_PROVISIONING_AUTHONLY) {
		ret = NetworkIEEE8021xFastFlags::AllowAuthenticated;
	}
	else if (nm_int == NM_EAP_FAST_PROVISIONING_BOTH) {
		ret = NetworkIEEE8021xFastFlags::AllowUnauthenticated | NetworkIEEE8021xFastFlags::AllowAuthenticated;
	}
	else {
		PRINT_ERROR(TQString("unknown EAP fast flag %s requested in existing connection").arg(nm.lower()))
	}

	return ret;
}

TQString tdeEAPFastFlagsToNMEAPFastFlags(NetworkIEEE8021xFastFlags::NetworkIEEE8021xFastFlags eaptype) {
	TQString ret = "";

	if ((eaptype & NetworkIEEE8021xFastFlags::AllowUnauthenticated) && (eaptype & NetworkIEEE8021xFastFlags::AllowAuthenticated)) {
		ret = TQString("%1").arg(NM_EAP_FAST_PROVISIONING_BOTH);
	}
	else if (eaptype & NetworkIEEE8021xFastFlags::AllowAuthenticated) {
		ret = TQString("%1").arg(NM_EAP_FAST_PROVISIONING_AUTHONLY);
	}
	else if (eaptype & NetworkIEEE8021xFastFlags::AllowUnauthenticated) {
		ret = TQString("%1").arg(NM_EAP_FAST_PROVISIONING_UNAUTHONLY);
	}
	else {
		ret = TQString("%1").arg(NM_EAP_FAST_PROVISIONING_DISABLED);
	}

	return ret;
}

WiFiMode::WiFiMode nmWiFiModeToWiFiMode(TQString nm) {
	WiFiMode::WiFiMode ret = WiFiMode::Infrastructure;

	if (nm.lower() == "infrastructure") {
		ret = WiFiMode::Infrastructure;
	}
	else if (nm.lower() == "adhoc") {
		ret = WiFiMode::AdHoc;
	}

	return ret;
}

TQString tdeWiFiModeToNMWiFiMode(WiFiMode::WiFiMode mode) {
	TQString ret;

	if (mode == WiFiMode::Infrastructure) {
		ret = "infrastructure";
	}
	else if (mode == WiFiMode::AdHoc) {
		ret = "adhoc";
	}

	return ret;
}

WiFiMode::WiFiMode nmWiFiModeToWiFiMode(TQ_UINT32 nm) {
	WiFiMode::WiFiMode ret = WiFiMode::Infrastructure;

	if (nm == NM_802_11_MODE_INFRASTRUCTURE) {
		ret = WiFiMode::Infrastructure;
	}
	else if (nm == NM_802_11_MODE_ADHOC) {
		ret = WiFiMode::AdHoc;
	}

	return ret;
}

NetworkWiFiClientFlags::NetworkWiFiClientFlags tdeWiFiFlagsToNMWiFiFlags(TQ_UINT32 nm) {
	NetworkWiFiClientFlags::NetworkWiFiClientFlags ret = NetworkWiFiClientFlags::None;

	if (nm & NM_802_11_DEVICE_CAP_CIPHER_WEP40) {
		ret | NetworkWiFiClientFlags::CipherWEP40;
	}
	if (nm & NM_802_11_DEVICE_CAP_CIPHER_WEP104) {
		ret | NetworkWiFiClientFlags::CipherWEP104;
	}
	if (nm & NM_802_11_DEVICE_CAP_CIPHER_TKIP) {
		ret | NetworkWiFiClientFlags::CipherTKIP;
	}
	if (nm & NM_802_11_DEVICE_CAP_CIPHER_CCMP) {
		ret | NetworkWiFiClientFlags::CipherCCMP;
	}
	if (nm & NM_802_11_DEVICE_CAP_WPA) {
		ret | NetworkWiFiClientFlags::CipherWPA;
	}
	if (nm & NM_802_11_DEVICE_CAP_RSN) {
		ret | NetworkWiFiClientFlags::CipherRSN;
	}

	return ret;
}

BluetoothConnectionType::BluetoothConnectionType nmBluetoothModeToTDEBluetoothMode(TQString nm) {
	BluetoothConnectionType::BluetoothConnectionType ret = BluetoothConnectionType::PAN;

	if (nm.lower() == "dun") {
		ret = BluetoothConnectionType::DUN;
	}
	else if (nm.lower() == "panu") {
		ret = BluetoothConnectionType::PAN;
	}

	return ret;
}

TQString tdeBluetoothModeToNMBluetoothMode(BluetoothConnectionType::BluetoothConnectionType type) {
	TQString ret;

	if (type == BluetoothConnectionType::DUN) {
		ret = "dun";
	}
	else if (type == BluetoothConnectionType::PAN) {
		ret = "panu";
	}

	return ret;
}

GSMNetworkType::GSMNetworkType nmGSMModeToTDEGSMMode(TQ_INT32 nm) {
	GSMNetworkType::GSMNetworkType ret = GSMNetworkType::Any;

	if (nm == NM_GSM_3G_ONLY) {
		ret = GSMNetworkType::Only3G;
	}
	else if (nm == NM_GSM_GPRS_EDGE_ONLY) {
		ret = GSMNetworkType::GPRSEdge;
	}
	else if (nm == NM_GSM_PREFER_3G) {
		ret = GSMNetworkType::Prefer3G;
	}
	else if (nm == NM_GSM_PREFER_2G) {
		ret = GSMNetworkType::Prefer2G;
	}

	return ret;
}

TQ_INT32 tdeGSMModeToNMGSMMode(GSMNetworkType::GSMNetworkType type) {
	TQ_INT32 ret = -1;

	if (type == GSMNetworkType::Only3G) {
		ret = NM_GSM_3G_ONLY;
	}
	else if (type == GSMNetworkType::GPRSEdge) {
		ret = NM_GSM_GPRS_EDGE_ONLY;
	}
	else if (type == GSMNetworkType::Prefer3G) {
		ret = NM_GSM_PREFER_3G;
	}
	else if (type == GSMNetworkType::Prefer2G) {
		ret = NM_GSM_PREFER_2G;
	}

	return ret;
}

WiFiFrequencyBand::WiFiFrequencyBand nmWiFiFrequencyBandToWiFiFrequencyBand(TQString nm) {
	WiFiFrequencyBand::WiFiFrequencyBand ret = WiFiFrequencyBand::Other;

	if (nm.lower() == "") {
		ret = WiFiFrequencyBand::None;
	}
	else if (nm.lower() == "bg") {
		ret = WiFiFrequencyBand::Band2_4GHz;
	}
	else if (nm.lower() == "a") {
		ret = WiFiFrequencyBand::Band5GHz;
	}

	return ret;
}

TQString tdeWiFiFrequencyBandToNMWiFiFrequencyBand(WiFiFrequencyBand::WiFiFrequencyBand mode) {
	TQString ret;

	if (mode == WiFiFrequencyBand::None) {
		ret = TQString::null;
	}
	else if (mode == WiFiFrequencyBand::Band2_4GHz) {
		ret = "bg";
	}
	else if (mode == WiFiFrequencyBand::Band5GHz) {
		ret = "a";
	}

	return ret;
}

NetworkWiFiKeyType::NetworkWiFiKeyType nmWiFiKeyTypeToTDEWiFiKeyType(TQString nm) {
	NetworkWiFiKeyType::NetworkWiFiKeyType ret = NetworkWiFiKeyType::Other;

	if (nm.lower() == "none") {
		ret = NetworkWiFiKeyType::WEP;
	}
	else if (nm.lower() == "ieee8021x") {
		ret = NetworkWiFiKeyType::DynamicWEP;
	}
	else if (nm.lower() == "wpa-none") {
		ret = NetworkWiFiKeyType::WPAAdHoc;
	}
	else if (nm.lower() == "wpa-psk") {
		ret = NetworkWiFiKeyType::WPAInfrastructure;
	}
	else if (nm.lower() == "wpa-eap") {
		ret = NetworkWiFiKeyType::WPAEnterprise;
	}

	return ret;
}

TQString tdeWiFiKeyTypeToNMWiFiKeyType(NetworkWiFiKeyType::NetworkWiFiKeyType type) {
	TQString ret;

	if (type == NetworkWiFiKeyType::WEP) {
		return "none";
	}
	else if (type == NetworkWiFiKeyType::DynamicWEP) {
		return "ieee8021x";
	}
	else if (type == NetworkWiFiKeyType::WPAAdHoc) {
		return "wpa-none";
	}
	else if (type == NetworkWiFiKeyType::WPAInfrastructure) {
		return "wpa-psk";
	}
	else if (type == NetworkWiFiKeyType::WPAEnterprise) {
		return "wpa-eap";
	}

	return ret;
}

NetworkWiFiAuthType::NetworkWiFiAuthType nmWiFiAuthTypeToTDEWiFiAuthType(TQString nm) {
	NetworkWiFiAuthType::NetworkWiFiAuthType ret = NetworkWiFiAuthType::Other;

	if (nm.lower() == "open") {
		ret = NetworkWiFiAuthType::Open;
	}
	else if (nm.lower() == "shared") {
		ret = NetworkWiFiAuthType::Shared;
	}
	else if (nm.lower() == "leap") {
		ret = NetworkWiFiAuthType::LEAP;
	}

	return ret;
}

TQString tdeWiFiAuthTypeToNMWiFiAuthType(NetworkWiFiAuthType::NetworkWiFiAuthType type) {
	TQString ret;

	if (type == NetworkWiFiAuthType::Open) {
		return "open";
	}
	else if (type == NetworkWiFiAuthType::Shared) {
		return "shared";
	}
	else if (type == NetworkWiFiAuthType::LEAP) {
		return "leap";
	}

	return ret;
}

NetworkWiFiWPAVersionFlags::NetworkWiFiWPAVersionFlags nmWiFiWPAVersionToTDEWiFiWPAVersion(TQStringList nm) {
	NetworkWiFiWPAVersionFlags::NetworkWiFiWPAVersionFlags ret = NetworkWiFiWPAVersionFlags::None;

	if ((nm.contains("wpa") && nm.contains("rsn")) || (nm.count() < 1)) {
		ret |= NetworkWiFiWPAVersionFlags::Any;
	}
	else if (nm.contains("wpa")) {
		ret |= NetworkWiFiWPAVersionFlags::WPA;
	}
	else if (nm.contains("rsn")) {
		ret |= NetworkWiFiWPAVersionFlags::RSN;
	}

	return ret;
}

TQStringList tdeWiFiWPAVersionToNMWiFiWPAVersion(NetworkWiFiWPAVersionFlags::NetworkWiFiWPAVersionFlags type) {
	TQStringList ret;

	if (type & NetworkWiFiWPAVersionFlags::WPA) {
		ret.append("wpa");
	}
	if (type & NetworkWiFiWPAVersionFlags::RSN) {
		ret.append("rsn");
	}

	return ret;
}

NetworkWiFiConnectionCipher::NetworkWiFiConnectionCipher nmWiFiCipherToTDEWiFiCipher(TQString nm) {
	NetworkWiFiConnectionCipher::NetworkWiFiConnectionCipher ret = NetworkWiFiConnectionCipher::None;

	if (nm.lower() == "wep40") {
		ret = NetworkWiFiConnectionCipher::CipherWEP40;
	}
	else if (nm.lower() == "wep104") {
		ret = NetworkWiFiConnectionCipher::CipherWEP104;
	}
	else if (nm.lower() == "tkip") {
		ret = NetworkWiFiConnectionCipher::CipherTKIP;
	}
	else if (nm.lower() == "ccmp") {
		ret = NetworkWiFiConnectionCipher::CipherCCMP;
	}

	return ret;
}

TQString tdeWiFiCipherToNMWiFiCipher(NetworkWiFiConnectionCipher::NetworkWiFiConnectionCipher cipher) {
	TQString ret;

	if (cipher == NetworkWiFiConnectionCipher::CipherWEP40) {
		ret = "wep40";
	}
	else if (cipher == NetworkWiFiConnectionCipher::CipherWEP104) {
		ret = "wep104";
	}
	else if (cipher == NetworkWiFiConnectionCipher::CipherTKIP) {
		ret = "tkip";
	}
	else if (cipher == NetworkWiFiConnectionCipher::CipherCCMP) {
		ret = "ccmp";
	}

	return ret;
}

NetworkSlaveDeviceType::NetworkSlaveDeviceType nmSlaveTypeToTDESlaveType(TQString nm) {
	NetworkSlaveDeviceType::NetworkSlaveDeviceType ret = NetworkSlaveDeviceType::None;

	if (nm.lower() == "bond") {
		ret = NetworkSlaveDeviceType::Bond;
	}

	return ret;
}

TQString tdeSlaveTypeToNMSlaveType(NetworkSlaveDeviceType::NetworkSlaveDeviceType slavetype) {
	TQString ret;

	if (slavetype == NetworkSlaveDeviceType::Bond) {
		ret = "bond";
	}

	return ret;
}

NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags nmPasswordFlagsToTDEPasswordFlags(unsigned int nm) {
	NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags ret = NetworkPasswordHandlingFlags::None;

	if (nm & NM_PASSWORD_SECRET_AGENTOWNED) {
		ret |= NetworkPasswordHandlingFlags::ExternalStorage;
	}
	if (nm & NM_PASSWORD_SECRET_NOTSAVED) {
		ret |= NetworkPasswordHandlingFlags::NoSave;
	}
	if (nm & NM_PASSWORD_SECRET_NOTREQUIRED) {
		ret |= NetworkPasswordHandlingFlags::NoPrompt;
	}

	return ret;
}

unsigned int tdePasswordFlagsToNMPasswordFlags(NetworkPasswordHandlingFlags::NetworkPasswordHandlingFlags flags) {
	unsigned int ret = 0;

	if (flags & NetworkPasswordHandlingFlags::ExternalStorage) {
		ret |= NM_PASSWORD_SECRET_AGENTOWNED;
	}
	if (flags & NetworkPasswordHandlingFlags::NoSave) {
		ret |= NM_PASSWORD_SECRET_NOTSAVED;
	}
	if (flags & NetworkPasswordHandlingFlags::NoPrompt) {
		ret |= NM_PASSWORD_SECRET_NOTREQUIRED;
	}

	return ret;
}

NetworkVLANFlags::NetworkVLANFlags nmVLANFlagsToTDEVLANFlags(unsigned int nm) {
	NetworkVLANFlags::NetworkVLANFlags ret = NetworkVLANFlags::None;

	if (nm & NM_VLAN_REORDER_PACKET_HEADERS) {
		ret |= NetworkVLANFlags::ReorderPacketHeaders;
	}
	if (nm & NM_VLAN_USE_GVRP) {
		ret |= NetworkVLANFlags::UseGVRP;
	}
	if (nm & NM_VLAN_LOOSE_BINDING) {
		ret |= NetworkVLANFlags::LooseBinding;
	}

	return ret;
}

unsigned int tdeVLANFlagsToNMVLANFlags(NetworkVLANFlags::NetworkVLANFlags flags) {
	unsigned int ret = 0;

	if (flags & NetworkVLANFlags::ReorderPacketHeaders) {
		ret |= NM_VLAN_REORDER_PACKET_HEADERS;
	}
	if (flags & NetworkVLANFlags::UseGVRP) {
		ret |= NM_VLAN_USE_GVRP;
	}
	if (flags & NetworkVLANFlags::LooseBinding) {
		ret |= NM_VLAN_LOOSE_BINDING;
	}

	return ret;
}

NetworkParity::NetworkParity nmParityToTDEParity(char nm) {
	NetworkParity::NetworkParity ret = NetworkParity::None;

	if (nm == 'E') {
		ret = NetworkParity::Even;
	}
	else if (nm == 'o') {
		ret = NetworkParity::Odd;
	}

	return ret;
}

char tdeParityToNMParity(NetworkParity::NetworkParity parity) {
	char ret = 'n';

	if (parity == NetworkParity::Even) {
		ret = 'E';
	}
	else if (parity == NetworkParity::Odd) {
		ret = 'o';
	}

	return ret;
}

NetworkWepKeyType::NetworkWepKeyType nmWepKeyTypeToTDEWepKeyType(unsigned int nm, TQString key=TQString::null) {
	NetworkWepKeyType::NetworkWepKeyType ret = NetworkWepKeyType::Hexadecimal;

	if (nm == NM_WEP_TYPE_HEXADECIMAL) {
		if (key.isNull()) {
			ret = NetworkWepKeyType::Hexadecimal;
		}
		else {
			if ((key.length() == 10) || (key.length() == 26)) {
				ret = NetworkWepKeyType::Hexadecimal;
			}
			else {
				ret = NetworkWepKeyType::Ascii;
			}
		}
	}
	else if (nm == NM_WEP_TYPE_PASSPHRASE) {
		ret = NetworkWepKeyType::Passphrase;
	}

	return ret;
}

unsigned int tdeWepKeyTypeToNMWepKeyType(NetworkWepKeyType::NetworkWepKeyType type) {
	unsigned int ret = 0;

	if (type == NetworkWepKeyType::Hexadecimal) {
		ret = NM_WEP_TYPE_HEXADECIMAL;
	}
	else if (type == NetworkWepKeyType::Ascii) {
		ret = NM_WEP_TYPE_HEXADECIMAL;
	}
	else if (type == NetworkWepKeyType::Passphrase) {
		ret = NM_WEP_TYPE_PASSPHRASE;
	}

	return ret;
}

NetworkDeviceCapabilityFlags::NetworkDeviceCapabilityFlags nmCapabilityFlagsToTDECapabilityFlags(unsigned int nm) {
	NetworkDeviceCapabilityFlags::NetworkDeviceCapabilityFlags ret = NetworkDeviceCapabilityFlags::None;

	if (nm & NM_DEVICE_CAP_NM_SUPPORTED) {
		ret |= NetworkDeviceCapabilityFlags::Supported;
	}
	if (nm & NM_DEVICE_CAP_CARRIER_DETECT) {
		ret |= NetworkDeviceCapabilityFlags::CanDetectLink;
	}

	return ret;
}

unsigned int tdeCapabilityFlagsToNMCapabilityFlags(NetworkDeviceCapabilityFlags::NetworkDeviceCapabilityFlags flags) {
	unsigned int ret = 0;

	if (flags & NetworkDeviceCapabilityFlags::Supported) {
		ret |= NM_DEVICE_CAP_NM_SUPPORTED;
	}
	if (flags & NetworkDeviceCapabilityFlags::CanDetectLink) {
		ret |= NM_DEVICE_CAP_CARRIER_DETECT;
	}

	return ret;
}

NetworkWiFiAPFlags::NetworkWiFiAPFlags nmAPSecFlagsToTDEAPSecFlags(unsigned int genflags, unsigned int nm) {
	NetworkWiFiAPFlags::NetworkWiFiAPFlags ret = NetworkWiFiAPFlags::None;

	if (genflags & NM_ACCESS_POINT_CAP_PRIVACY) {
		ret |= NetworkWiFiAPFlags::PrivacySupport;
	}

	if (nm & NM_ACCESS_POINT_SEC_PAIR_WEP40) {
		ret |= NetworkWiFiAPFlags::PairWEP40;
	}
	if (nm & NM_ACCESS_POINT_SEC_PAIR_WEP104) {
		ret |= NetworkWiFiAPFlags::PairWEP104;
	}
	if (nm & NM_ACCESS_POINT_SEC_PAIR_TKIP) {
		ret |= NetworkWiFiAPFlags::PairTKIP;
	}
	if (nm & NM_ACCESS_POINT_SEC_PAIR_CCMP) {
		ret |= NetworkWiFiAPFlags::PairCCMP;
	}
	if (nm & NM_ACCESS_POINT_SEC_GROUP_WEP40) {
		ret |= NetworkWiFiAPFlags::GroupWEP40;
	}
	if (nm & NM_ACCESS_POINT_SEC_GROUP_WEP104) {
		ret |= NetworkWiFiAPFlags::GroupWEP104;
	}
	if (nm & NM_ACCESS_POINT_SEC_GROUP_TKIP) {
		ret |= NetworkWiFiAPFlags::GroupTKIP;
	}
	if (nm & NM_ACCESS_POINT_SEC_GROUP_CCMP) {
		ret |= NetworkWiFiAPFlags::GroupCCMP;
	}
	if (nm & NM_ACCESS_POINT_SEC_KEY_MGMT_PSK) {
		ret |= NetworkWiFiAPFlags::KeyManagementPSK;
	}
	if (nm & NM_ACCESS_POINT_SEC_KEY_MGMT_802_1X) {
		ret |= NetworkWiFiAPFlags::KeyManagement80211;
	}

	return ret;
}

unsigned int tdeAPSecFlagsToNMAPGenSecFlags(NetworkWiFiAPFlags::NetworkWiFiAPFlags flags) {
	unsigned int ret = 0;

	if (flags & NetworkWiFiAPFlags::PrivacySupport) {
		ret |= NM_ACCESS_POINT_CAP_PRIVACY;
	}

	return ret;
}

unsigned int tdeAPSecFlagsToNMAPSecFlags(NetworkWiFiAPFlags::NetworkWiFiAPFlags flags) {
	unsigned int ret = 0;

	if (flags & NetworkWiFiAPFlags::PairWEP40) {
		ret |= NM_ACCESS_POINT_SEC_PAIR_WEP40;
	}
	if (flags & NetworkWiFiAPFlags::PairWEP104) {
		ret |= NM_ACCESS_POINT_SEC_PAIR_WEP104;
	}
	if (flags & NetworkWiFiAPFlags::PairTKIP) {
		ret |= NM_ACCESS_POINT_SEC_PAIR_TKIP;
	}
	if (flags & NetworkWiFiAPFlags::PairCCMP) {
		ret |= NM_ACCESS_POINT_SEC_PAIR_CCMP;
	}
	if (flags & NetworkWiFiAPFlags::GroupWEP40) {
		ret |= NM_ACCESS_POINT_SEC_GROUP_WEP40;
	}
	if (flags & NetworkWiFiAPFlags::GroupWEP104) {
		ret |= NM_ACCESS_POINT_SEC_GROUP_WEP104;
	}
	if (flags & NetworkWiFiAPFlags::GroupTKIP) {
		ret |= NM_ACCESS_POINT_SEC_GROUP_TKIP;
	}
	if (flags & NetworkWiFiAPFlags::GroupCCMP) {
		ret |= NM_ACCESS_POINT_SEC_GROUP_CCMP;
	}
	if (flags & NetworkWiFiAPFlags::KeyManagementPSK) {
		ret |= NM_ACCESS_POINT_SEC_KEY_MGMT_PSK;
	}
	if (flags & NetworkWiFiAPFlags::KeyManagement80211) {
		ret |= NM_ACCESS_POINT_SEC_KEY_MGMT_802_1X;
	}

	return ret;
}

NetworkInfinibandTransportMode::NetworkInfinibandTransportMode nmIBTransportToTDEIBTransport(TQString nm) {
	NetworkInfinibandTransportMode::NetworkInfinibandTransportMode ret = NetworkInfinibandTransportMode::Other;

	if (nm.lower() == "datagram") {
		ret = NetworkInfinibandTransportMode::Datagram;
	}
	else if (nm.lower() == "connected") {
		ret = NetworkInfinibandTransportMode::Connected;
	}

	return ret;
}

TQString tdeIBTransportToNMIBTransport(NetworkInfinibandTransportMode::NetworkInfinibandTransportMode mode) {
	TQString ret;

	if (mode == NetworkInfinibandTransportMode::Datagram) {
		ret = "datagram";
	}
	else if (mode == NetworkInfinibandTransportMode::Connected) {
		ret = "connected";
	}

	return ret;
}

TQString NetworkConnectionManager_BackendNM::deviceInterfaceString(TQString macAddress) {
	if (d->m_networkManagerProxy) {
		TQT_DBusObjectPathList devices;
		TQT_DBusError error;
		bool ret;
		ret = d->m_networkManagerProxy->GetDevices(devices, error);
		if (ret) {
			TQT_DBusObjectPathList::iterator it;
			for (it = devices.begin(); it != devices.end(); ++it) {
				DBus::DeviceProxy genericDevice(NM_DBUS_SERVICE, (*it));
				genericDevice.setConnection(TQT_DBusConnection::systemBus());
				NetworkDeviceType::NetworkDeviceType deviceType = nmDeviceTypeToTDEDeviceType(genericDevice.getDeviceType(error));
				if (error.isValid()) {
					// Error!
					PRINT_ERROR((error.name() + ": " + error.message()))
					break;
				}
				else if (deviceType == NetworkDeviceType::WiredEthernet) {
					DBus::EthernetDeviceProxy ethernetDevice(NM_DBUS_SERVICE, (*it));
					ethernetDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = ethernetDevice.getPermHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == NetworkDeviceType::Infiniband) {
					DBus::InfinibandDeviceProxy infinibandDevice(NM_DBUS_SERVICE, (*it));
					infinibandDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = infinibandDevice.getHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == NetworkDeviceType::WiFi) {
					DBus::WiFiDeviceProxy wiFiDevice(NM_DBUS_SERVICE, (*it));
					wiFiDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = wiFiDevice.getPermHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == NetworkDeviceType::WiMax) {
					DBus::WiMaxDeviceProxy wiMaxDevice(NM_DBUS_SERVICE, (*it));
					wiMaxDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = wiMaxDevice.getHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == NetworkDeviceType::OLPCMesh) {
					DBus::OlpcMeshDeviceProxy olpcMeshDevice(NM_DBUS_SERVICE, (*it));
					olpcMeshDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = olpcMeshDevice.getHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == NetworkDeviceType::Bluetooth) {
					DBus::BluetoothDeviceProxy bluetoothDevice(NM_DBUS_SERVICE, (*it));
					bluetoothDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = bluetoothDevice.getHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				// FIXME
				// Add other supported device types here
			}
			return "";
		}
		else {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return "";
		}
	}
	else {
		return "";
	}
}

TQString macAddressForGenericDevice(TQT_DBusObjectPath path) {
	TQT_DBusError error;

	DBus::DeviceProxy genericDevice(NM_DBUS_SERVICE, path);
	genericDevice.setConnection(TQT_DBusConnection::systemBus());
	TQ_UINT32 deviceType = genericDevice.getDeviceType(error);
	if (error.isValid()) {
		// Error!
		PRINT_ERROR((error.name() + ": " + error.message()))
		return TQString();
	}
	else if (deviceType == NM_DEVICE_TYPE_ETHERNET) {
		DBus::EthernetDeviceProxy ethernetDevice(NM_DBUS_SERVICE, path);
		ethernetDevice.setConnection(TQT_DBusConnection::systemBus());
		TQString candidateMACAddress = ethernetDevice.getPermHwAddress(error);
		if (!error.isValid()) {
			return candidateMACAddress.lower();
		}
	}
	else if (deviceType == NM_DEVICE_TYPE_INFINIBAND) {
		DBus::InfinibandDeviceProxy infinibandDevice(NM_DBUS_SERVICE, path);
		infinibandDevice.setConnection(TQT_DBusConnection::systemBus());
		TQString candidateMACAddress = infinibandDevice.getHwAddress(error);
		if (!error.isValid()) {
			return candidateMACAddress.lower();
		}
	}
	else if (deviceType == NM_DEVICE_TYPE_WIFI) {
		DBus::WiFiDeviceProxy wiFiDevice(NM_DBUS_SERVICE, path);
		wiFiDevice.setConnection(TQT_DBusConnection::systemBus());
		TQString candidateMACAddress = wiFiDevice.getPermHwAddress(error);
		if (!error.isValid()) {
			return candidateMACAddress.lower();
		}
	}
	else if (deviceType == NM_DEVICE_TYPE_WIMAX) {
		DBus::WiMaxDeviceProxy wiMaxDevice(NM_DBUS_SERVICE, path);
		wiMaxDevice.setConnection(TQT_DBusConnection::systemBus());
		TQString candidateMACAddress = wiMaxDevice.getHwAddress(error);
		if (!error.isValid()) {
			return candidateMACAddress.lower();
		}
	}
	else if (deviceType == NM_DEVICE_TYPE_OLPC_MESH) {
		DBus::OlpcMeshDeviceProxy olpcMeshDevice(NM_DBUS_SERVICE, path);
		olpcMeshDevice.setConnection(TQT_DBusConnection::systemBus());
		TQString candidateMACAddress = olpcMeshDevice.getHwAddress(error);
		if (!error.isValid()) {
			return candidateMACAddress.lower();
		}
	}
	else if (deviceType == NM_DEVICE_TYPE_BT) {
		DBus::BluetoothDeviceProxy bluetoothDevice(NM_DBUS_SERVICE, path);
		bluetoothDevice.setConnection(TQT_DBusConnection::systemBus());
		TQString candidateMACAddress = bluetoothDevice.getHwAddress(error);
		if (!error.isValid()) {
			return candidateMACAddress.lower();
		}
	}
	// FIXME
	// Add other supported device types here

	return TQString::null;
}

TQString tdeDeviceUUIDForMACAddress(TQString macAddress) {
	HardwareDevices *hwdevices = HardwareDevices::instance();
	if (!hwdevices) {
		return TQString::null;
	}

	GenericHardwareList devices = hwdevices->listByDeviceClass(GenericDeviceType::Network);
	for (GenericHardwareList::iterator it = devices.begin(); it != devices.end(); ++it) {
		NetworkDevice* dev = dynamic_cast<NetworkDevice*>(*it);
		if (dev) {
			if (macAddress.lower() == dev->macAddress().lower()) {
				return dev->uniqueID();
			}
		}
	}

	return TQString::null;
}

NetworkConnectionManager_BackendNM_DBusSignalReceiver::NetworkConnectionManager_BackendNM_DBusSignalReceiver(NetworkConnectionManager_BackendNMPrivate* parent) : m_parent(parent) {
	//
}

NetworkConnectionManager_BackendNM_DBusSignalReceiver::~NetworkConnectionManager_BackendNM_DBusSignalReceiver() {
	//
}

void NetworkConnectionManager_BackendNM_DBusSignalReceiver::dbusSignal(const TQT_DBusMessage& message) {
	if (message.type() == TQT_DBusMessage::SignalMessage) {
		TQString interface = message.interface();
		TQString sender = message.sender();
		TQString member = message.member();
		TQString path = message.path();

// 		printf("[DEBUG] In dbusSignal: sender: %s, member: %s, interface: %s, path: %s, parent path: %s\n", sender.ascii(), member.ascii(), interface.ascii(), path.ascii(), m_parent->m_dbusDeviceString.ascii()); fflush(stdout);

		if (interface == NM_VPN_DBUS_CONNECTION_SERVICE) {
			if (member == "VpnStateChanged") {
				// Demarshal data
				TQ_UINT32 state = message[0].toUInt32();
				TQ_UINT32 reason = message[1].toUInt32();
				if (state == NM_VPN_STATE_FAILED) {
					m_parent->internalProcessVPNFailure(reason);
				}
			}
		}
		else if (interface == NM_DBUS_DEVICE_SERVICE) {
			if (path == m_parent->m_dbusDeviceString) {
				if (member == "StateChanged") {
					// Demarshal data
					TQ_UINT32 new_state = message[0].toUInt32();
					TQ_UINT32 old_state = message[1].toUInt32();
					TQ_UINT32 reason = message[2].toUInt32();
					m_parent->internalProcessDeviceStateChanged(new_state, old_state, reason);
				}
			}
		}
	}
}

NetworkConnectionManager_BackendNM::NetworkConnectionManager_BackendNM(TQString macAddress) : NetworkConnectionManager(macAddress) {
	d = new NetworkConnectionManager_BackendNMPrivate(this);

	// Set up proxy interfaces
	d->m_networkManagerProxy = new DBus::NetworkManagerProxy(NM_DBUS_SERVICE, NM_DBUS_PATH);
	d->m_networkManagerProxy->setConnection(TQT_DBusConnection::systemBus());
	d->m_networkManagerSettings = new DBus::SettingsInterface(NM_DBUS_SERVICE, NM_DBUS_PATH_SETTINGS);
	d->m_networkManagerSettings->setConnection(TQT_DBusConnection::systemBus());
	d->m_vpnProxy = new DBus::VPNPluginProxy(NM_VPN_DBUS_PLUGIN_SERVICE, NM_VPN_DBUS_PLUGIN_PATH);
	d->m_vpnProxy->setConnection(TQT_DBusConnection::systemBus());

	d->m_dbusDeviceString = deviceInterfaceString(macAddress);
	if (d->m_dbusDeviceString != "") {
		d->m_networkDeviceProxy = new DBus::DeviceProxy(NM_DBUS_SERVICE, d->m_dbusDeviceString);
		d->m_networkDeviceProxy->setConnection(TQT_DBusConnection::systemBus());
		if (deviceType() == NetworkDeviceType::WiFi) {
			d->m_wiFiDeviceProxy = new DBus::WiFiDeviceProxy(NM_DBUS_SERVICE, d->m_dbusDeviceString);
			d->m_wiFiDeviceProxy->setConnection(TQT_DBusConnection::systemBus());
		}
	}

	// Connect global signals
	connect(d->m_networkManagerProxy, SIGNAL(StateChanged(TQ_UINT32)), d, SLOT(internalProcessGlobalStateChanged(TQ_UINT32)));

	// Connect VPN signals
	if (d->m_vpnProxy) {
		connect(d->m_vpnProxy, SIGNAL(StateChanged(TQ_UINT32)), d, SLOT(internalProcessVPNStateChanged(TQ_UINT32)));
		connect(d->m_vpnProxy, SIGNAL(LoginBanner(const TQString&)), d, SLOT(internalProcessVPNLoginBanner(const TQString&)));
		connect(d->m_vpnProxy, SIGNAL(Failure(TQ_UINT32)), d, SLOT(internalProcessVPNFailure(TQ_UINT32)));
	}

	// Connect local signals
	if (d->m_networkDeviceProxy) {
		connect(d->m_networkDeviceProxy, SIGNAL(StateChanged(TQ_UINT32, TQ_UINT32, TQ_UINT32)), d, SLOT(internalProcessDeviceStateChanged(TQ_UINT32, TQ_UINT32, TQ_UINT32)));
	}
	if (d->m_wiFiDeviceProxy) {
		connect(d->m_wiFiDeviceProxy, SIGNAL(AccessPointAdded(const TQT_DBusObjectPath&)), d, SLOT(internalProcessWiFiAccessPointAdded(const TQT_DBusObjectPath&)));
		connect(d->m_wiFiDeviceProxy, SIGNAL(AccessPointRemoved(const TQT_DBusObjectPath&)), d, SLOT(internalProcessWiFiAccessPointRemoved(const TQT_DBusObjectPath&)));
		connect(d->m_wiFiDeviceProxy, SIGNAL(PropertiesChanged(const TQMap<TQString, TQT_DBusVariant>&)), d, SLOT(internalProcessWiFiPropertiesChanged(const TQMap<TQString, TQT_DBusVariant>&)));
	}

	// Create public lists
	m_connectionList = new NetworkConnectionList;
	m_hwNeighborList = new NetworkHWNeighborList;

	// Run site survey to populate neighbor list with initial data
	siteSurvey();
}

NetworkConnectionManager_BackendNM::~NetworkConnectionManager_BackendNM() {
	// Destroy public lists
	clearNetworkConnectionList();
	delete m_connectionList;
	clearNetworkHWNeighborList();
	delete m_hwNeighborList;

	// Tear down proxy interfaces
	if (d->m_networkManagerProxy) delete d->m_networkManagerProxy;
	if (d->m_networkManagerSettings) delete d->m_networkManagerSettings;
	if (d->m_networkDeviceProxy) delete d->m_networkDeviceProxy;

	delete d;
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessGlobalStateChanged(TQ_UINT32 state) {
	m_parent->internalNetworkConnectionStateChanged(m_parent->backendStatus());
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessVPNStateChanged(TQ_UINT32 state) {
	m_parent->internalNetworkConnectionStateChanged(m_parent->backendStatus());
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessVPNLoginBanner(const TQString& banner) {
	m_parent->internalVpnEvent(NetworkVPNEventType::LoginBanner, banner);
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessVPNFailure(TQ_UINT32 reason) {
	// FIXME
	// This should provide a plain-text interpretation of the NetworkManager-specific error code
	m_parent->internalVpnEvent(NetworkVPNEventType::Failure, TQString("VPN connection attempt failed!<br>NetworkManager returned error %1.").arg(reason));
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessDeviceStateChanged(TQ_UINT32 newState, TQ_UINT32 oldState, TQ_UINT32 reason) {
	Q_UNUSED(oldState)

	if (m_prevDeviceState == newState) return;
	m_prevDeviceState = newState;

	if (newState == NM_DEVICE_STATE_FAILED) {
		TQString errorString;
		if (reason == NM_DEVICE_STATE_REASON_NONE) {
			errorString = TQString("Connection attempt failed!");
		}
		else if (reason == NM_DEVICE_STATE_REASON_UNKNOWN) {
			errorString = TQString("Connection attempt failed!<br>Unknown error detected.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_NOW_MANAGED) {
			errorString = TQString("Connection attempt failed!<br>Network device is now managed.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_NOW_UNMANAGED) {
			errorString = TQString("Connection attempt failed!<br>Network device is now unmanaged.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_CONFIG_FAILED) {
			errorString = TQString("Connection attempt failed!<br>Configuration failed.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_IP_CONFIG_UNAVAILABLE) {
			errorString = TQString("Connection attempt failed!<br>IP configuration unavailable.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_IP_CONFIG_EXPIRED) {
			errorString = TQString("Connection attempt failed!<br>IP configuration expired.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_NO_SECRETS) {
			errorString = i18n("Connection attempt failed!<br>Secrets were required to establish a connection, but no secrets were available.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SUPPLICANT_DISCONNECT) {
			errorString = TQString("Connection attempt failed!<br>The supplicant was disconnected while attempting to establish a wireless connection.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SUPPLICANT_CONFIG_FAILED) {
			errorString = TQString("Connection attempt failed!<br>Supplicant configuration failed while attempting to establish a wireless connection.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SUPPLICANT_FAILED) {
			errorString = i18n("Connection attempt failed!<br>The supplicant failed while attempting to establish a wireless connection.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SUPPLICANT_TIMEOUT) {
			errorString = i18n("Connection attempt failed!<br>The supplicant timed out while attempting to establish a wireless connection.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_PPP_START_FAILED) {
			errorString = i18n("Connection attempt failed!<br>The PPP client failed to start.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_PPP_DISCONNECT) {
			errorString = i18n("Connection attempt failed!<br>The PPP client was disconnected.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_PPP_FAILED) {
			errorString = i18n("Connection attempt failed!<br>Unknown PPP failure.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_DHCP_START_FAILED) {
			errorString = i18n("Connection attempt failed!<br>The DHCP client failed to start.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_DHCP_ERROR) {
			errorString = i18n("Connection attempt failed!<br>The DHCP client encountered an error.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_DHCP_FAILED) {
			errorString = i18n("Connection attempt failed!<br>Uknown DHCP failure.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SHARED_START_FAILED) {
			errorString = i18n("Connection attempt failed!<br>The connection sharing service failed to start.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SHARED_FAILED) {
			errorString = i18n("Connection attempt failed!<br>The connection sharing service encountered an error.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_AUTOIP_START_FAILED) {
			errorString = i18n("Connection attempt failed!<br>The AutoIP service failed to start.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_AUTOIP_ERROR) {
			errorString = i18n("Connection attempt failed!<br>The AutoIP service encountered an error.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_AUTOIP_FAILED) {
			errorString = i18n("Connection attempt failed!<br>Unknown AutoIP failure.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_MODEM_BUSY) {
			errorString = i18n("Connection attempt failed!<br>Modem was busy.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_MODEM_NO_DIAL_TONE) {
			errorString = i18n("Connection attempt failed!<br>No dial tone.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_MODEM_NO_CARRIER) {
			errorString = i18n("Connection attempt failed!<br>No carrier detected.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_MODEM_DIAL_TIMEOUT) {
			errorString = i18n("Connection attempt failed!<br>Modem timed out while dialing.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_MODEM_DIAL_FAILED) {
			errorString = i18n("Connection attempt failed!<br>The modem failed to dial.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_MODEM_INIT_FAILED) {
			errorString = i18n("Connection attempt failed!<br>Modem initialization failed.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_APN_FAILED) {
			errorString = i18n("Connection attempt failed!<br>GSM APN failure.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_REGISTRATION_NOT_SEARCHING) {
			errorString = i18n("Connection attempt failed!<br>GSM registration failed to search for networks.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_REGISTRATION_DENIED) {
			errorString = i18n("Connection attempt failed!<br>GSM registration attempt was rejected.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_REGISTRATION_TIMEOUT) {
			errorString = i18n("Connection attempt failed!<br>GSM registration attempt timed out.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_REGISTRATION_FAILED) {
			errorString = i18n("Connection attempt failed!<br>GSM registration attempt failed.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_PIN_CHECK_FAILED) {
			errorString = i18n("Connection attempt failed!<br>GSM PIN check failed.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_FIRMWARE_MISSING) {
			errorString = i18n("Connection attempt failed!<br>Network device firmware is missing.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_REMOVED) {
			errorString = i18n("Connection attempt failed!<br>Network device was removed.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SLEEPING) {
			errorString = i18n("Connection attempt failed!<br>Network device is sleeping.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_CONNECTION_REMOVED) {
			errorString = i18n("Connection attempt failed!<br>Connection was removed.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_USER_REQUESTED) {
			errorString = i18n("Connection attempt failed!<br>User requested device disconnection.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_CARRIER) {
			errorString = i18n("Connection attempt failed!<br>Carrier or link status changed.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_CONNECTION_ASSUMED) {
			errorString = i18n("Connection attempt failed!<br>Device and/or connection already active.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SUPPLICANT_AVAILABLE) {
			errorString = i18n("Connection attempt failed!<br>The supplicant is now available.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_MODEM_NOT_FOUND) {
			errorString = i18n("Connection attempt failed!<br>Requested modem was not found.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_BT_FAILED) {
			errorString = i18n("Connection attempt failed!<br>Bluetooth connection timeout.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_SIM_NOT_INSERTED) {
			errorString = i18n("Connection attempt failed!<br>GSM SIM not inserted.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_SIM_PIN_REQUIRED) {
			errorString = i18n("Connection attempt failed!<br>GSM PIN required.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_SIM_PUK_REQUIRED) {
			errorString = i18n("Connection attempt failed!<br>GSM PUK required.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_GSM_SIM_WRONG) {
			errorString = i18n("Connection attempt failed!<br>GSM SIM incorrect.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_INFINIBAND_MODE) {
			errorString = i18n("Connection attempt failed!<br>Incorrect Infiniband mode.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_DEPENDENCY_FAILED) {
			errorString = i18n("Connection attempt failed!<br>Dependency failure.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_BR2684_FAILED) {
			errorString = i18n("Connection attempt failed!<br>Unknown bridge failure.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_MODEM_MANAGER_UNAVAILABLE) {
			errorString = i18n("Connection attempt failed!<br>ModemManager not available.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SSID_NOT_FOUND) {
			errorString = i18n("Connection attempt failed!<br>SSID not found.");
		}
		else if (reason == NM_DEVICE_STATE_REASON_SECONDARY_CONNECTION_FAILED) {
			errorString = i18n("Connection attempt failed!<br>Secondary connection failure.");
		}
		else {
			// FIXME
			// This should provide a plain-text interpretation of the NetworkManager-specific error code
			errorString = TQString("Connection attempt failed!<br>NetworkManager returned error %1.").arg(reason);
		}
		m_parent->internalNetworkDeviceEvent(NetworkDeviceEventType::Failure, errorString);
	}

	m_parent->internalNetworkDeviceStateChanged(nmDeviceStateToTDEDeviceState(newState), m_parent->m_macAddress);
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessWiFiAccessPointAdded(const TQT_DBusObjectPath& dbuspath) {
	NetworkWiFiAPInfo* apInfo = m_parent->getAccessPointDetails(dbuspath);
	if (apInfo) {
		if (!m_accessPointProxyList.contains(dbuspath)) {
			// Set up monitoring object
			DBus::AccessPointProxy* apProxy = new DBus::AccessPointProxy(NM_DBUS_SERVICE, dbuspath);
			apProxy->setConnection(TQT_DBusConnection::systemBus());
			connect(apProxy, SIGNAL(PropertiesChanged(const TQMap<TQString, TQT_DBusVariant>&)), this, SLOT(internalProcessAPPropertiesChanged(const TQMap<TQString, TQT_DBusVariant>&)));
			m_accessPointProxyList[dbuspath] = (apProxy);

			// Notify client applications
			m_parent->internalAccessPointStatusChanged(apInfo->BSSID, NetworkAPEventType::Discovered);
		}
		delete apInfo;
	}
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessWiFiAccessPointRemoved(const TQT_DBusObjectPath& dbuspath) {
	NetworkWiFiAPInfo* apInfo = m_parent->getAccessPointDetails(dbuspath);
	if (apInfo) {
		// Notify client applications
		m_parent->internalAccessPointStatusChanged(apInfo->BSSID, NetworkAPEventType::Lost);
		delete apInfo;

		// Destroy related monitoring object
		DBus::AccessPointProxy* apProxy = m_accessPointProxyList[dbuspath];
		m_accessPointProxyList.remove(dbuspath);
		if (apProxy) {
			delete apProxy;
		}
	}
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessWiFiPropertiesChanged(const TQMap<TQString, TQT_DBusVariant>& props) {
	if (m_wiFiDeviceProxy) {
		if (props.contains("ActiveAccessPoint")) {
			TQT_DBusError error;
			NetworkWiFiAPInfo* apInfo = m_parent->getAccessPointDetails(m_wiFiDeviceProxy->getActiveAccessPoint(error));
			if (apInfo) {
				m_parent->internalAccessPointStatusChanged(apInfo->BSSID, NetworkAPEventType::AccessPointChanged);
			}
		}
		else if (props.contains("Bitrate")) {
			m_parent->internalNetworkDeviceEvent(NetworkDeviceEventType::BitRateChanged, TQString::null);
		}
	}
}

void NetworkConnectionManager_BackendNMPrivate::internalProcessAPPropertiesChanged(const TQMap<TQString, TQT_DBusVariant>& props) {
	const DBus::AccessPointProxy* apProxy = dynamic_cast<const DBus::AccessPointProxy*>(sender());
	if (apProxy) {
		TQT_DBusError error;
		MACAddress BSSID;
		BSSID.fromString(apProxy->getHwAddress(error));
		if (props.contains("Strength")) {
			m_parent->internalAccessPointStatusChanged(BSSID, NetworkAPEventType::SignalStrengthChanged);
		}
	}
}

NetworkDeviceType::NetworkDeviceType NetworkConnectionManager_BackendNM::deviceType() {
	if (m_macAddress == "") {
		return NetworkDeviceType::BackendOnly;
	}
	else {
		if (d->m_dbusDeviceString != "") {
			// Query NM for the device type
			TQT_DBusError error;
			d->m_dbusDeviceString = deviceInterfaceString(m_macAddress);
			DBus::DeviceProxy genericDevice(NM_DBUS_SERVICE, d->m_dbusDeviceString);
			genericDevice.setConnection(TQT_DBusConnection::systemBus());
			NetworkDeviceType::NetworkDeviceType ret = nmDeviceTypeToTDEDeviceType(genericDevice.getDeviceType(error));
			if (error.isValid()) {
				// Error!
				PRINT_ERROR((error.name() + ": " + error.message()))
				return NetworkDeviceType::Other;
			}
			else {
				return ret;
			}
		}
		else {
			// Error!
			PRINT_ERROR(TQString("Invalid DBUS device string '%1'").arg(d->m_dbusDeviceString))
			return NetworkDeviceType::Other;
		}
	}
}

NetworkConnectionType::NetworkConnectionType NetworkConnectionManager_BackendNM::connectionType(TQString dbusPath) {
	NetworkConnectionType::NetworkConnectionType connType = NetworkConnectionType::Other;
	TQ_UINT32 ret;
	TQT_DBusError error;

#ifndef USE_ASYNC_DBUS_CALLS
	// Obtain connection settings from the path specified
	DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, dbusPath);
	connectionSettings.setConnection(TQT_DBusConnection::systemBus());
	TQT_DBusTQStringDataMap connectionSettingsMap;
	ret = connectionSettings.GetSettings(connectionSettingsMap, error);
	if (ret && error.isValid()) {
		ret = 0;
		PRINT_ERROR((error.name() + ": " + error.message()))
	}
	if (ret) {
#else // USE_ASYNC_DBUS_CALLS
	// Obtain connection settings from the path specified
	DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, dbusPath);
	connectionSettings.setConnection(TQT_DBusConnection::systemBus());
	connect(&connectionSettings, SIGNAL(GetSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)), d, SLOT(processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)));
	int asyncCallID;
	ret = connectionSettings.GetSettingsAsync(asyncCallID, error);
	if (ret && error.isValid()) {
		ret = 0;
		PRINT_ERROR((error.name() + ": " + error.message()))
	}
	if (ret) {
		// Wait for the asynchronous call to return...
		d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
		TQTimer nmCallTimeoutTimer;
		nmCallTimeoutTimer.start(NM_ASYNC_TIMEOUT_MS, TRUE);
		while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
			tqApp->processEvents();
			if (!nmCallTimeoutTimer.isActive()) {
				PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
				break;
			}
		}
		TQT_DBusTQStringDataMap connectionSettingsMap = d->nmConnectionSettingsAsyncSettingsResponse[asyncCallID];
		if (d->nmConnectionSettingsAsyncSettingsErrorResponse.contains(asyncCallID)) {
			PRINT_ERROR((d->nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallID].name() + ": " + d->nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallID].message()));
			d->nmConnectionSettingsAsyncSettingsErrorResponse.remove(asyncCallID);
		}
		d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
		if (d->nmConnectionSettingsAsyncSettingsResponse.contains(asyncCallID)) {
			d->nmConnectionSettingsAsyncSettingsResponse.remove(asyncCallID);
		}
#endif // USE_ASYNC_DBUS_CALLS

		// Parse settings to find connection type
		TQT_DBusTQStringDataMap::const_iterator it2;
		for (it2 = connectionSettingsMap.begin(); it2 != connectionSettingsMap.end(); ++it2) {
			TQString outerKeyValue = it2.key();
			TQT_DBusData dataValue = it2.data();

			TQT_DBusTQStringDataMap nestedConnectionSettingsMap = dataValue.toStringKeyMap();
			TQT_DBusTQStringDataMap::const_iterator it3;
			for (it3 = nestedConnectionSettingsMap.begin(); it3 != nestedConnectionSettingsMap.end(); ++it3) {
				TQString keyValue = it3.key();
				TQT_DBusData dataValue = it3.data();
				if (dataValue.type() == TQT_DBusData::Variant) {
					TQT_DBusVariant dataValueVariant = dataValue.toVariant();
					TQT_DBusData dataValue2 = dataValueVariant.value;
					if (dataValue2.type() != TQT_DBusData::Variant) {
						if (outerKeyValue.lower() == "connection") {
							if (keyValue.lower() == "type") {
								connType = nmConnectionTypeToTDEConnectionType(dataValue2.toString());
							}
						}
					}
				}
			}
		}
	}

	return connType;
}

TQString NetworkConnectionManager_BackendNM::backendName() {
	return i18n("NetworkManager");
}

NetworkGlobalManagerFlags::NetworkGlobalManagerFlags NetworkConnectionManager_BackendNM::backendStatus() {
	if (d->m_networkManagerProxy) {
		TQ_UINT32 ret;
		TQT_DBusError error;
		ret = d->m_networkManagerProxy->getState(error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return NetworkGlobalManagerFlags::BackendUnavailable;
		}
		else {
			NetworkGlobalManagerFlags::NetworkGlobalManagerFlags globalFlags = nmGlobalStateToTDEGlobalState(ret);
			NetworkGlobalManagerFlags::NetworkGlobalManagerFlags vpnFlags = NetworkGlobalManagerFlags::Unknown;
			if (d->m_vpnProxy) {
				ret = d->m_vpnProxy->getState(error);
				if (error.isValid()) {
					// Error!
					bool print_error = true;
					if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
						if (d->vpn_service_error_notified) {
							print_error = false;
						}
						else {
							d->vpn_service_error_notified = true;
						}
					}
					if (print_error) {
						PRINT_ERROR(TQString("Attempting to access the network-manager VPN service returned: %1").arg(error.name() + ": " + error.message()))
					}
					vpnFlags = NetworkGlobalManagerFlags::VPNUnknown;
				}
				else {
					vpnFlags = nmVPNStateToTDEGlobalState(ret);
				}
			}
			return globalFlags | vpnFlags;
		}
	}
	else {
		return NetworkGlobalManagerFlags::BackendUnavailable;
	}
}

NetworkDeviceInformation NetworkConnectionManager_BackendNM::deviceInformation() {
	TQT_DBusError error;
	NetworkDeviceInformation ret;

	if (d->m_networkDeviceProxy) {
		ret.statusFlags = nmDeviceStateToTDEDeviceState(d->m_networkDeviceProxy->getState(error));
		ret.UUID = d->m_networkDeviceProxy->getUdi(error);
		ret.backendDriver = d->m_networkDeviceProxy->getDriver(error);
		ret.backendDriverVersion = d->m_networkDeviceProxy->getDriverVersion(error);
		ret.firmwareVersion = d->m_networkDeviceProxy->getFirmwareVersion(error);
		ret.capabilityFlags = nmCapabilityFlagsToTDECapabilityFlags(d->m_networkDeviceProxy->getCapabilities(error));
		// ipConfiguration is not filled in, as the TDE HW library provides complementary functionality and is more reliable/easier to use and maintain
		ret.managed = d->m_networkDeviceProxy->getManaged(error);
		ret.autoConnect = d->m_networkDeviceProxy->getAutoconnect(error);
		ret.firmwareMissing = d->m_networkDeviceProxy->getFirmwareMissing(error);
		ret.deviceType = nmDeviceTypeToTDEDeviceType(d->m_networkDeviceProxy->getDeviceType(error));
		if (error.isValid()) {
			// Error!
			bool print_error = true;
			if (error.name() == "org.freedesktop.DBus.Error.AccessDenied") {
				if (error.message().contains("org.freedesktop.NetworkManager.Device")) {
					// Unable to determine if device allows autoconnect
					// Assume true!
					ret.autoConnect = true;
					if (d->device_autoconnect_error_notified) {
						print_error = false;
					}
					else {
						d->device_autoconnect_error_notified = true;
					}
				}
			}
			if (print_error) {
				PRINT_ERROR((error.name() + ": " + error.message()))
			}

			// Reset error object to avoid spurious error messages on the command line
			error = TQT_DBusError();
		}

		// Populate wiFiInfo
		if ((deviceType() == NetworkDeviceType::WiFi) && (d->m_wiFiDeviceProxy)) {
			ret.wiFiInfo.valid = true;
			ret.wiFiInfo.hwAddress.fromString(d->m_wiFiDeviceProxy->getHwAddress(error));
			ret.wiFiInfo.permanentHWAddress.fromString(d->m_wiFiDeviceProxy->getPermHwAddress(error));
			ret.wiFiInfo.operatingMode = nmWiFiModeToWiFiMode(d->m_wiFiDeviceProxy->getMode(error));
			ret.wiFiInfo.bitrate = d->m_wiFiDeviceProxy->getBitrate(error);
			NetworkWiFiAPInfo* apInfo = getAccessPointDetails(d->m_wiFiDeviceProxy->getActiveAccessPoint(error));
			if (error.isValid()) {
				PRINT_ERROR((error.name() + ": " + error.message()))

				// Reset error object to avoid spurious error messages on the command line
				error = TQT_DBusError();
			}
			if (apInfo) {
				ret.wiFiInfo.activeAccessPointBSSID = apInfo->BSSID;
				NetworkWiFiAPInfo* neighborListAPInfo = findAccessPointByBSSID(ret.wiFiInfo.activeAccessPointBSSID);
				if (neighborListAPInfo) {
					*neighborListAPInfo = *apInfo;
				}
				delete apInfo;
			}
			else {
				ret.wiFiInfo.activeAccessPointBSSID = MACAddress();
			}
			ret.wiFiInfo.wirelessFlags = tdeWiFiFlagsToNMWiFiFlags(d->m_wiFiDeviceProxy->getWirelessCapabilities(error));
		}
		else {
			ret.wiFiInfo.valid = false;
		}

		// Get active connection UUID
		TQT_DBusObjectPath connectionPath = d->m_networkDeviceProxy->getActiveConnection(error);
		if (!error.isValid()) {
			DBus::ActiveConnectionProxy activeConnection(NM_DBUS_SERVICE, connectionPath);
			activeConnection.setConnection(TQT_DBusConnection::systemBus());
			ret.activeConnectionUUID = activeConnection.getUuid(error);
			if (error.isValid()) {
				ret.activeConnectionUUID = TQString::null;
			}
		}

		ret.valid = true;
	}

	return ret;
}

NetworkDeviceInformation NetworkConnectionManager_BackendNM::deviceStatus() {
	TQT_DBusError error;
	NetworkDeviceInformation ret;

	if (d->m_networkDeviceProxy) {
		ret.statusFlags = nmDeviceStateToTDEDeviceState(d->m_networkDeviceProxy->getState(error));
		ret.UUID = d->m_networkDeviceProxy->getUdi(error);

		// Get active connection UUID
		TQT_DBusObjectPath connectionPath = d->m_networkDeviceProxy->getActiveConnection(error);
		if (!error.isValid()) {
			DBus::ActiveConnectionProxy activeConnection(NM_DBUS_SERVICE, connectionPath);
			activeConnection.setConnection(TQT_DBusConnection::systemBus());
			ret.activeConnectionUUID = activeConnection.getUuid(error);
			if (error.isValid()) {
				ret.activeConnectionUUID = TQString::null;
			}
		}

		ret.valid = true;
	}

	return ret;
}

void NetworkConnectionManager_BackendNMPrivate::processConnectionSettingsAsyncReply(int asyncCallId, const TQT_DBusDataMap<TQString>& settings) {
	nmConnectionSettingsAsyncCallWaiting[asyncCallId] = false;
	nmConnectionSettingsAsyncSettingsResponse[asyncCallId] = settings;
}

void NetworkConnectionManager_BackendNMPrivate::processConnectionSettingsUpdateAsyncReply(int asyncCallId) {
	nmConnectionSettingsAsyncCallWaiting[asyncCallId] = false;
}

void NetworkConnectionManager_BackendNMPrivate::processAddConnectionAsyncReply(int asyncCallId, const TQT_DBusObjectPath& path) {
	nmConnectionSettingsAsyncCallWaiting[asyncCallId] = false;
	nmAddConnectionAsyncResponse[asyncCallId] = path;
}

void NetworkConnectionManager_BackendNMPrivate::processConnectionSettingsAsyncError(int asyncCallId, const TQT_DBusError error) {
	nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallId] = error;
}

void NetworkConnectionManager_BackendNMPrivate::processConnectionSettingsUpdateAsyncError(int asyncCallId, const TQT_DBusError error) {
	nmConnectionSettingsUpdateAsyncSettingsErrorResponse[asyncCallId] = error;
}

void NetworkConnectionManager_BackendNMPrivate::processAddConnectionAsyncError(int asyncCallId, const TQT_DBusError error) {
	nmAddConnectionAsyncErrorResponse[asyncCallId] = error;
}

void NetworkConnectionManager_BackendNM::loadConnectionInformation() {
	if (d->nonReentrantCallActive) return;

	d->nonReentrantCallActive = true;

	MACAddress deviceMACAddress;
	deviceMACAddress.fromString(m_macAddress);

	if (d->m_networkManagerSettings) {
		clearNetworkConnectionList();
		TQT_DBusObjectPathList connections;
		TQT_DBusError error;
		bool ret;
		int state;
		ret = d->m_networkManagerSettings->ListConnections(connections, error);
		if (ret) {
			TQT_DBusObjectPathList::iterator it;
			for (it = connections.begin(); it != connections.end(); ++it) {
				NetworkConnection* connection;
				WiredEthernetConnection* ethernetConnection = NULL;
				WiredInfinibandConnection* infinibandConnection = NULL;
				WiFiConnection* wiFiConnection = NULL;
				VPNConnection* vpnConnection = NULL;
				WiMaxConnection* wiMaxConnection = NULL;
				VLANConnection* vlanConnection = NULL;
				OLPCMeshConnection* olpcMeshConnection = NULL;
				BluetoothConnection* bluetoothConnection = NULL;
				ModemConnection* modemConnection = NULL;
				NetworkConnectionType::NetworkConnectionType connType = connectionType((*it));
				if (connType == NetworkConnectionType::WiredEthernet) {
					connection = ethernetConnection = new WiredEthernetConnection;
				}
				else if (connType == NetworkConnectionType::Infiniband) {
					connection = infinibandConnection = new WiredInfinibandConnection;
				}
				else if (connType == NetworkConnectionType::WiFi) {
					connection = wiFiConnection = new WiFiConnection;
				}
				else if (connType == NetworkConnectionType::VPN) {
					connection = vpnConnection = new VPNConnection;
				}
				else if (connType == NetworkConnectionType::WiMax) {
					connection = wiMaxConnection = new WiMaxConnection;
				}
				else if (connType == NetworkConnectionType::VLAN) {
					connection = vlanConnection = new VLANConnection;
				}
				else if (connType == NetworkConnectionType::OLPCMesh) {
					connection = olpcMeshConnection = new OLPCMeshConnection;
				}
				else if (connType == NetworkConnectionType::Bluetooth) {
					connection = bluetoothConnection = new BluetoothConnection;
				}
				else if (connType == NetworkConnectionType::Modem) {
					connection = modemConnection = new ModemConnection;
				}
				else {
					connection = new NetworkConnection;
				}
				// Set up defaults
				connection->ipConfig.connectionFlags =	NetworkIPConfigurationFlags::IPV4DHCPIP			| \
									NetworkIPConfigurationFlags::IPV4DHCPDNS			| \
									NetworkIPConfigurationFlags::IPV4DHCPRoutes			| \
									NetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute	| \
									NetworkIPConfigurationFlags::IPV6DHCPIP			| \
									NetworkIPConfigurationFlags::IPV6DHCPDNS			| \
									NetworkIPConfigurationFlags::IPV6DHCPRoutes			| \
									NetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute;
				// Set up NM-specific defaults
				// Keep in sync with latest NM default settings!
				// NM 0.9 setting descriptions and default values are available at:
				// http://projects.gnome.org/NetworkManager/developers/api/09/ref-settings.html
				connection->autoConnect = true;

				if (wiFiConnection) {
					wiFiConnection->securitySettings.authType = NetworkWiFiAuthType::Open;
				}

#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
				printf("[network-manager comm debug] %s\n", (*it).data()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS

#ifndef USE_ASYNC_DBUS_CALLS
				// Obtain connection settings from the path specified
				DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, (*it));
				connectionSettings.setConnection(TQT_DBusConnection::systemBus());
				TQT_DBusTQStringDataMap connectionSettingsMap;
				ret = connectionSettings.GetSettings(connectionSettingsMap, error);
				if (ret && error.isValid()) {
					ret = 0;
					PRINT_ERROR((error.name() + ": " + error.message()))
				}
				if (ret) {
#else // USE_ASYNC_DBUS_CALLS
				// Obtain connection settings from the path specified
				DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, (*it));
				connectionSettings.setConnection(TQT_DBusConnection::systemBus());
				connect(&connectionSettings, SIGNAL(GetSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)), d, SLOT(processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)));
				connect(&connectionSettings, SIGNAL(AsyncErrorResponseDetected(int, const TQT_DBusError)), d, SLOT(processConnectionSettingsAsyncError(int, const TQT_DBusError)));
				int asyncCallID;
				ret = connectionSettings.GetSettingsAsync(asyncCallID, error);
				if (ret && error.isValid()) {
					ret = 0;
					PRINT_ERROR((error.name() + ": " + error.message()))
				}
				if (ret) {
					// Wait for the asynchronous call to return...
					d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
					TQTimer nmCallTimeoutTimer;
					nmCallTimeoutTimer.start(NM_ASYNC_TIMEOUT_MS, TRUE);
					while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
						tqApp->processEvents();
						if (!nmCallTimeoutTimer.isActive()) {
							PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
							break;
						}
					}
					TQT_DBusTQStringDataMap connectionSettingsMap = d->nmConnectionSettingsAsyncSettingsResponse[asyncCallID];
					if (d->nmConnectionSettingsAsyncSettingsErrorResponse.contains(asyncCallID)) {
						PRINT_ERROR((d->nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallID].name() + ": " + d->nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallID].message()));
						d->nmConnectionSettingsAsyncSettingsErrorResponse.remove(asyncCallID);
					}
					d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
					if (d->nmConnectionSettingsAsyncSettingsResponse.contains(asyncCallID)) {
						d->nmConnectionSettingsAsyncSettingsResponse.remove(asyncCallID);
					}
#endif // USE_ASYNC_DBUS_CALLS

#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
					printf("[network-manager comm debug] received DBUS object structure map follows:\n"); fflush(stdout);
					printDBUSObjectStructure(TQT_DBusData::fromStringKeyMap(connectionSettingsMap));
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS

					// Parse settings
					TQT_DBusTQStringDataMap::const_iterator it2;
					for (it2 = connectionSettingsMap.begin(); it2 != connectionSettingsMap.end(); ++it2) {
						TQString outerKeyValue = it2.key();
						TQT_DBusData dataValue = it2.data();
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
						printf("[network-manager comm debug] [%s]\n", outerKeyValue.ascii()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
						TQT_DBusTQStringDataMap nestedConnectionSettingsMap = dataValue.toStringKeyMap();
						TQT_DBusTQStringDataMap::const_iterator it3;
						for (it3 = nestedConnectionSettingsMap.begin(); it3 != nestedConnectionSettingsMap.end(); ++it3) {
							TQString keyValue = it3.key();
							TQT_DBusData dataValue = it3.data();
							if (dataValue.type() != TQT_DBusData::Variant) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
								printf("[network-manager comm debug] %s = %s (type %d(%s))\n", keyValue.ascii(), dataValue.toString().ascii(), dataValue.type(), dataValue.typeName()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
								// No NM settings are known which use this style
							}
							else {
								TQT_DBusVariant dataValueVariant = dataValue.toVariant();
								TQT_DBusData dataValue2 = dataValueVariant.value;
								if (dataValue2.type() != TQT_DBusData::Variant) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
									printf("[network-manager comm debug] %s = %s (type %d(%s), signature %s)\n", keyValue.ascii(), dataValue2.toString().ascii(), dataValue2.type(), dataValue2.typeName(), dataValueVariant.signature.ascii()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
									// Most NM settings use this style
									if (outerKeyValue.lower() == "connection") {
										if (keyValue.lower() == "id") {
											connection->friendlyName = dataValue2.toString();
										}
										else if (keyValue.lower() == "uuid") {
											connection->UUID = dataValue2.toString().lower();
										}
										else if (keyValue.lower() == "permissions") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQString authString = innerDataValue.toString();
												TQStringList pieces = TQStringList::split(":", authString);
												if (pieces[0].lower() == "user") {
													connection->authorizedUsers.append(pieces[1]);
												}
											}
										}
										else if (keyValue.lower() == "autoconnect") {
											connection->autoConnect = dataValue2.toBool();
										}
										else if (keyValue.lower() == "read-only") {
											connection->readOnly = dataValue2.toBool();
										}
										else if (keyValue.lower() == "master") {
											connection->masterConnectionUUID = dataValue2.toString().lower();
										}
										else if (keyValue.lower() == "slave-type") {
											connection->slaveType = nmSlaveTypeToTDESlaveType(dataValue2.toString());
										}
										else if (keyValue.lower() == "timestamp") {
											connection->lastKnownConnection.setTime_t(dataValue2.toUInt64());
										}
									}
									else if (outerKeyValue.lower() == "802-1x") {
										if (keyValue.lower() == "eap") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											state = 0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												if (state == 0) {
													// EAP type
													connection->eapConfig.type = nmEAPTypeToTDEEAPType(innerDataValue.toString());
												}
												state++;
											}
										}
										else if (keyValue.lower() == "identity") {
											connection->eapConfig.userName = dataValue2.toString();
										}
										else if (keyValue.lower() == "anonymous-identity") {
											connection->eapConfig.anonymousUserName = dataValue2.toString();
										}
										else if (keyValue.lower() == "pac-file") {
											connection->eapConfig.pacFileName = dataValue2.toString();
										}
										else if (keyValue.lower() == "ca-cert") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count=0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												count++;
												connection->eapConfig.caCertificate.resize(count+1);
												connection->eapConfig.caCertificate[count] = innerDataValue.toByte();
											}
										}
										else if (keyValue.lower() == "ca-path") {
											connection->eapConfig.additionalCAFilesPath = dataValue2.toString();
										}
										else if (keyValue.lower() == "subject-match") {
											connection->eapConfig.authServerCertSubjectMatch = dataValue2.toString();
										}
										else if (keyValue.lower() == "altsubject-matches") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												connection->eapConfig.alternateAuthServerCertSubjectMatch.append(innerDataValue.toString());
											}
										}
										else if (keyValue.lower() == "client-cert") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count=0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												count++;
												connection->eapConfig.clientCertificate.resize(count+1);
												connection->eapConfig.clientCertificate[count] = innerDataValue.toByte();
											}
										}
										else if (keyValue.lower() == "phase1-peapver") {
											connection->eapConfig.forcePEAPVersion = dataValue2.toString();
										}
										else if (keyValue.lower() == "phase1-peaplabel") {
											connection->eapConfig.forcePEAPLabel = dataValue2.toString();
										}
										else if (keyValue.lower() == "phase1-fast-provisioning") {
											connection->eapConfig.fastProvisioningFlags = nmEAPFastFlagsToTDEEAPFastFlags(dataValue2.toString());
										}
										else if (keyValue.lower() == "phase2-auth") {
											connection->eapConfig.phase2NonEAPAuthMethod = nmEAPTypeToTDEEAPType(dataValue2.toString());
										}
										else if (keyValue.lower() == "phase2-autheap") {
											connection->eapConfig.phase2EAPAuthMethod = nmEAPTypeToTDEEAPType(dataValue2.toString());
										}
										else if (keyValue.lower() == "phase2-ca-cert") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count=0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												count++;
												connection->eapConfig.phase2CaCertificate.resize(count+1);
												connection->eapConfig.phase2CaCertificate[count] = innerDataValue.toByte();
											}
										}
										else if (keyValue.lower() == "phase2-ca-path") {
											connection->eapConfig.phase2CaFilesPath = dataValue2.toString();
										}
										else if (keyValue.lower() == "phase2-subject-match") {
											connection->eapConfig.phase2AuthServerCertSubjectMatch = dataValue2.toString();
										}
										else if (keyValue.lower() == "phase2-altsubject-matches") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												connection->eapConfig.phase2AlternateAuthServerCertSubjectMatch.append(innerDataValue.toString());
											}
										}
										else if (keyValue.lower() == "phase2-client-cert") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count=0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												count++;
												connection->eapConfig.phase2ClientCertificate.resize(count+1);
												connection->eapConfig.phase2ClientCertificate[count] = innerDataValue.toByte();
											}
										}
										else if (keyValue.lower() == "password-flags") {
											connection->eapConfig.passwordFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "password-raw-flags") {
											connection->eapConfig.binaryPasswordFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "private-key") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count=0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												count++;
												connection->eapConfig.privateKey.resize(count+1);
												connection->eapConfig.privateKey[count] = innerDataValue.toByte();
											}
										}
										else if (keyValue.lower() == "private-key-password-flags") {
											connection->eapConfig.privateKeyPasswordFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "phase2-private-key") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count=0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												count++;
												connection->eapConfig.phase2PrivateKey.resize(count+1);
												connection->eapConfig.phase2PrivateKey[count] = innerDataValue.toByte();
											}
										}
										else if (keyValue.lower() == "phase2-private-key-password-flags") {
											connection->eapConfig.phase2PrivateKeyPasswordFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "system-ca-certs") {
											connection->eapConfig.forceSystemCaCertificates = dataValue2.toBool();
										}
										connection->eapConfig.valid = true;
									}
									else if (outerKeyValue.lower() == "802-3-ethernet") {
										if (keyValue.lower() == "duplex") {
											connection->fullDuplex = (dataValue2.toString().lower() == "full")?true:false;
										}
										else if (keyValue.lower() == "mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											NetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->lockedHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "cloned-mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											NetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->manualHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "mtu") {
											connection->mtu = dataValue2.toUInt32();
										}
									}
									else if (outerKeyValue.lower() == "infiniband") {
										if (keyValue.lower() == "mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											NetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->lockedHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "mtu") {
											connection->mtu = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "transport-mode") {
											infinibandConnection->transportMode = nmIBTransportToTDEIBTransport(dataValue2.toString());
										}
									}
									else if (outerKeyValue.lower() == "802-11-wireless") {
										if (keyValue.lower() == "ssid") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count = 0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												wiFiConnection->SSID.resize(count+1);
												wiFiConnection->SSID[count] = innerDataValue.toByte();
												count++;
											}
										}
										else if (keyValue.lower() == "mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											NetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->lockedHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "cloned-mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											NetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->manualHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "mtu") {
											connection->mtu = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "mode") {
											wiFiConnection->operatingMode = nmWiFiModeToWiFiMode(dataValue2.toString());
										}
										else if (keyValue.lower() == "band") {
											wiFiConnection->bandRestriction = nmWiFiFrequencyBandToWiFiFrequencyBand(dataValue2.toString());
										}
										else if (keyValue.lower() == "channel") {
											wiFiConnection->channelRestriction = dataValue2.toUInt32();
											if (wiFiConnection->channelRestriction == 0) wiFiConnection->channelRestriction = -1;
										}
										else if (keyValue.lower() == "rate") {
											wiFiConnection->bitRateRestriction = dataValue2.toUInt32()*1000;
											if (wiFiConnection->bitRateRestriction == 0) wiFiConnection->bitRateRestriction = -1;
										}
										else if (keyValue.lower() == "tx-power") {
											wiFiConnection->powerRestriction = dataValue2.toUInt32();
											if (wiFiConnection->powerRestriction == 0) wiFiConnection->powerRestriction = -1;
										}
										else if (keyValue.lower() == "bssid") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											NetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											wiFiConnection->accessPointRestriction.setAddress(macAddress);
										}
										else if (keyValue.lower() == "mac-address-blacklist") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												MACAddress hwAddress;
												hwAddress.fromString(innerDataValue.toString());
												wiFiConnection->blacklistedBSSIDs.append(hwAddress);
											}
										}
										else if (keyValue.lower() == "seen-bssids") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												MACAddress hwAddress;
												hwAddress.fromString(innerDataValue.toString());
												wiFiConnection->heardBSSIDs.append(hwAddress);
											}
										}
										else if (keyValue.lower() == "security") {
											TQString setting;
											if (setting.lower() == "802-11-wireless-security") {
												wiFiConnection->securityRequired = true;
											}
											else {
												wiFiConnection->securityRequired = false;
											}
										}
										else if (keyValue.lower() == "hidden") {
											wiFiConnection->isHiddenNetwork = dataValue2.toBool();
										}
									}
									else if ((outerKeyValue.lower() == "802-11-wireless-security") && (wiFiConnection)) {
										if (keyValue.lower() == "key-mgmt") {
											wiFiConnection->securitySettings.keyType = nmWiFiKeyTypeToTDEWiFiKeyType(dataValue2.toString());
										}
										else if (keyValue.lower() == "wep-tx-keyidx") {
											wiFiConnection->securitySettings.wepKeyIndex = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "auth-alg") {
											wiFiConnection->securitySettings.authType = nmWiFiAuthTypeToTDEWiFiAuthType(dataValue2.toString());
										}
										else if (keyValue.lower() == "proto") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											TQStringList strings;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												strings.append(innerDataValue.toString());
											}
											wiFiConnection->securitySettings.wpaVersion = nmWiFiWPAVersionToTDEWiFiWPAVersion(strings);
										}
										else if (keyValue.lower() == "pairwise") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											TQStringList strings;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												wiFiConnection->securitySettings.allowedPairWiseCiphers.append(nmWiFiCipherToTDEWiFiCipher(innerDataValue.toString()));
											}
											if ((wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherWEP40))
												|| (wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherWEP104))
												|| (wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherTKIP))
												|| (wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherCCMP))) {
												wiFiConnection->securitySettings.allowedPairWiseCiphers.append(NetworkWiFiConnectionCipher::Any);
											}
										}
										else if (keyValue.lower() == "group") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											TQStringList strings;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(nmWiFiCipherToTDEWiFiCipher(innerDataValue.toString()));
											}
											if ((wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherWEP40))
												|| (wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherWEP104))
												|| (wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherTKIP))
												|| (wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherCCMP))) {
												wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(NetworkWiFiConnectionCipher::Any);
											}
										}
										else if (keyValue.lower() == "leap-username") {
											wiFiConnection->securitySettings.leapUsername = dataValue2.toString();
										}
										else if (keyValue.lower() == "wep-key-flags") {
											wiFiConnection->securitySettings.wepKeyFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "wep-key-type") {
											wiFiConnection->securitySettings.wepKeyType = nmWepKeyTypeToTDEWepKeyType(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "psk-flags") {
											wiFiConnection->securitySettings.pskFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "leap-password-flags") {
											wiFiConnection->securitySettings.leapPasswordFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										wiFiConnection->securitySettings.valid = true;
									}
									else if (outerKeyValue.lower() == "vpn") {
										if (keyValue.lower() == "service-type") {
											TQString plugin = dataValue2.toString();
											plugin.replace("org.freedesktop.NetworkManager.", "");
											vpnConnection->vpnPluginID = plugin;
										}
										else if (keyValue.lower() == "user-name") {
											vpnConnection->lockedUserName = dataValue2.toString();
										}
										else if (keyValue.lower() == "data") {
											vpnConnection->pluginData.clear();
											TQT_DBusTQStringDataMap nestedConnectionSettingsMap = dataValue2.toStringKeyMap();
											TQT_DBusTQStringDataMap::const_iterator it4;
											for (it4 = nestedConnectionSettingsMap.begin(); it4 != nestedConnectionSettingsMap.end(); ++it4) {
												TQString keyValue4 = it4.key();
												TQT_DBusData dataValue4 = it4.data();
												if (dataValue4.type() == TQT_DBusData::String) {
													vpnConnection->pluginData[keyValue4] = dataValue4.toString();
												}
											}
										}
									}
									else if (outerKeyValue.lower() == "wimax") {
										if (keyValue.lower() == "mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											NetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->lockedHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "network-name") {
											wiMaxConnection->networkServiceProvider = dataValue2.toString();
										}
									}
									else if (outerKeyValue.lower() == "vlan") {
										if (keyValue.lower() == "interface-name") {
											vlanConnection->kernelName = dataValue2.toString();
										}
										else if (keyValue.lower() == "parent") {
											vlanConnection->parentConnectionUUID = dataValue2.toString();
										}
										else if (keyValue.lower() == "id") {
											vlanConnection->vlanID = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "flags") {
											vlanConnection->vlanFlags = nmVLANFlagsToTDEVLANFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "ingress-priority-map") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQStringList pieces = TQStringList::split(":", innerDataValue.toString(), TRUE);
												vlanConnection->ingressPriorityMap[pieces[0].toUInt()] = pieces[1].toUInt();;
											}
										}
										else if (keyValue.lower() == "egress-priority-map") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQStringList pieces = TQStringList::split(":", innerDataValue.toString(), TRUE);
												vlanConnection->egressPriorityMap[pieces[0].toUInt()] = pieces[1].toUInt();;
											}
										}
									}
									else if (outerKeyValue.lower() == "serial") {
										if (keyValue.lower() == "baud") {
											connection->serialConfig.baudRate = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "bits") {
											connection->serialConfig.byteWidth = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "parity") {
											connection->serialConfig.parity = nmParityToTDEParity(dataValue2.toByte());
										}
										else if (keyValue.lower() == "stopbits") {
											connection->serialConfig.stopBits = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "send-delay") {
											connection->serialConfig.txDelay = dataValue2.toUInt64();
										}
										connection->serialConfig.valid = true;
									}
									else if (outerKeyValue.lower() == "ppp") {
										if (keyValue.lower() == "noauth") {
											connection->pppConfig.requireServerAuthentication = !(dataValue2.toBool());
										}
										else if (keyValue.lower() == "refuse-eap") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::DisableEAP;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::DisableEAP;
										}
										else if (keyValue.lower() == "refuse-pap") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::DisablePAP;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::DisablePAP;
										}
										else if (keyValue.lower() == "refuse-chap") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::DisableCHAP;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::DisableCHAP;
										}
										else if (keyValue.lower() == "refuse-mschap") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::DisableMSCHAP;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::DisableMSCHAP;
										}
										else if (keyValue.lower() == "refuse-mschapv2") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::DisableMSCHAPv2;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::DisableMSCHAPv2;
										}
										else if (keyValue.lower() == "nobsdcomp") {
											if (dataValue2.toBool())	connection->pppConfig.flags &= ~NetworkPPPFlags::AllowBSDCompression;
											else				connection->pppConfig.flags |=  NetworkPPPFlags::AllowBSDCompression;
										}
										else if (keyValue.lower() == "nodeflate") {
											if (dataValue2.toBool())	connection->pppConfig.flags &= ~NetworkPPPFlags::AllowDeflateCompression;
											else				connection->pppConfig.flags |=  NetworkPPPFlags::AllowDeflateCompression;
										}
										else if (keyValue.lower() == "no-vj-comp") {
											if (dataValue2.toBool())	connection->pppConfig.flags &= ~NetworkPPPFlags::AllowVJCompression;
											else				connection->pppConfig.flags |=  NetworkPPPFlags::AllowVJCompression;
										}
										else if (keyValue.lower() == "require-mppe") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::RequireMPPE;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::RequireMPPE;
										}
										else if (keyValue.lower() == "require-mppe-128") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::RequireMPPE128;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::RequireMPPE128;
										}
										else if (keyValue.lower() == "mppe-stateful") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::StatefulMPPE;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::StatefulMPPE;
										}
										else if (keyValue.lower() == "crtscts") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  NetworkPPPFlags::UseHardwareFlowControl;
											else				connection->pppConfig.flags &= ~NetworkPPPFlags::UseHardwareFlowControl;
										}
										else if (keyValue.lower() == "baud") {
											connection->pppConfig.baudRate = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "mru") {
											connection->pppConfig.mru = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "mtu") {
											connection->pppConfig.mtu = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "lcp-echo-interval") {
											connection->pppConfig.lcpEchoPingInterval = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "lcp-echo-failure") {
											connection->pppConfig.lcpEchoFailureThreshold = dataValue2.toUInt32();
										}
										connection->pppConfig.valid = true;
									}
									else if (outerKeyValue.lower() == "pppoe") {
										if (keyValue.lower() == "service") {
											connection->pppoeConfig.networkServiceProvider = dataValue2.toString();
										}
										else if (keyValue.lower() == "username") {
											connection->pppoeConfig.username = dataValue2.toString();
										}
										else if (keyValue.lower() == "password-flags") {
											connection->pppoeConfig.passwordFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										connection->pppoeConfig.secretsValid = true;
									}
									else if ((outerKeyValue.lower() == "802-11-olpc-mesh") && (olpcMeshConnection)) {
										if (keyValue.lower() == "ssid") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count = 0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												olpcMeshConnection->SSID.resize(count+1);
												olpcMeshConnection->SSID[count] = innerDataValue.toByte();
												count++;
											}
										}
										else if (keyValue.lower() == "channel") {
											olpcMeshConnection->channel = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "dhcp-anycast-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											int count = 0;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												olpcMeshConnection->anycastDHCPHWAddress.resize(count+1);
												olpcMeshConnection->anycastDHCPHWAddress[count] = innerDataValue.toByte();
												count++;
											}
										}
									}
									else if ((outerKeyValue.lower() == "bluetooth") && (bluetoothConnection)) {
										if (keyValue.lower() == "bdaddr") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											NetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->lockedHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "type") {
											bluetoothConnection->type = nmBluetoothModeToTDEBluetoothMode(dataValue2.toString());
										}
									}
									else if ((outerKeyValue.lower() == "cdma") && (modemConnection)) {
										if (keyValue.lower() == "number") {
											modemConnection->cdmaConfig.providerDataNumber = dataValue2.toString();
										}
										else if (keyValue.lower() == "username") {
											modemConnection->cdmaConfig.username = dataValue2.toString();
										}
										else if (keyValue.lower() == "password-flags") {
											modemConnection->cdmaConfig.passwordFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										modemConnection->type = ModemConnectionType::CDMA;
										modemConnection->cdmaConfig.valid = true;
									}
									else if ((outerKeyValue.lower() == "gsm") && (modemConnection)) {
										if (keyValue.lower() == "number") {
											modemConnection->gsmConfig.providerDataNumber = dataValue2.toString();
										}
										else if (keyValue.lower() == "username") {
											modemConnection->gsmConfig.username = dataValue2.toString();
										}
										else if (keyValue.lower() == "password-flags") {
											modemConnection->gsmConfig.passwordFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "apn") {
											modemConnection->gsmConfig.accessPointName = dataValue2.toString();
										}
										else if (keyValue.lower() == "network-id") {
											modemConnection->gsmConfig.networkID = dataValue2.toString();
										}
										else if (keyValue.lower() == "network-type") {
											modemConnection->gsmConfig.networkType = nmGSMModeToTDEGSMMode(dataValue2.toInt32());
										}
										else if (keyValue.lower() == "pin-flags") {
											modemConnection->gsmConfig.pinFlags = nmPasswordFlagsToTDEPasswordFlags(dataValue2.toUInt32());
										}
										else if (keyValue.lower() == "allowed-bands") {
											modemConnection->gsmConfig.allowedFrequencyBands = dataValue2.toUInt32();
										}
										else if (keyValue.lower() == "home-only") {
											modemConnection->gsmConfig.allowRoaming = !dataValue2.toBool();
										}
										modemConnection->type = ModemConnectionType::GSM;
										modemConnection->gsmConfig.valid = true;
									}
									else if (outerKeyValue.lower() == "ipv4") {
										if (keyValue.lower() == "addresses") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQT_DBusDataValueList innerValueList = innerDataValue.toTQValueList();
												TQT_DBusDataValueList::const_iterator it5;
												state = 0;
												NetworkSingleIPConfiguration ipConfig;
												for (it5 = innerValueList.begin(); it5 != innerValueList.end(); ++it5) {
													TQT_DBusData innerMostDataValue = *it5;
													if (state == 0) {
														// Address
														ipConfig.ipAddress = TQHostAddress(reverseIPV4ByteOrder(innerMostDataValue.toUInt32()));
													}
													else if (state == 1) {
														// Network mask
														ipConfig.networkMask.fromCIDRMask(innerMostDataValue.toUInt32());
													}
													else if (state == 2) {
														// Gateway
														ipConfig.gateway = TQHostAddress(reverseIPV4ByteOrder(innerMostDataValue.toUInt32()));
													}
													state++;
												}
												ipConfig.valid = true;
												connection->ipConfig.ipConfigurations.append(ipConfig);
											}
										}
										else if (keyValue.lower() == "dhcp-client-id") {
											connection->ipConfig.dhcpClientIdentifier = dataValue2.toString();
										}
										else if (keyValue.lower() == "dns") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												connection->ipConfig.resolvers.append(TQHostAddress(reverseIPV4ByteOrder(innerDataValue.toUInt32())));
											}
										}
										else if (keyValue.lower() == "dns-search") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												connection->ipConfig.searchDomains.append(NetworkSearchDomain(innerDataValue.toString(), false));
											}
										}
										else if (keyValue.lower() == "ignore-auto-dns") {
											bool nm_static_dns = dataValue2.toBool();
											if (nm_static_dns) {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV4DHCPDNS;
											}
											else {
												connection->ipConfig.connectionFlags |= NetworkIPConfigurationFlags::IPV4DHCPDNS;
											}
										}
										else if (keyValue.lower() == "may-fail") {
											bool nm_may_fail = dataValue2.toBool();
											connection->requireIPV4 = !nm_may_fail;
										}
										else if (keyValue.lower() == "method") {
											TQString nm_method = dataValue2.toString().lower();
											if (nm_method == "auto") {
												connection->ipConfig.connectionFlags |= NetworkIPConfigurationFlags::IPV4DHCPIP;
											}
											else if (nm_method == "manual") {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV4DHCPIP;
											}
											else if (nm_method == "link-local") {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV4LocalOnly;
											}
											else if (nm_method == "shared") {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV4StartConnectionSharingServer;
											}
											else if (nm_method == "disabled") {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV4Disabled;
											}
										}
										else if (keyValue.lower() == "ignore-auto-routes") {
											bool nm_static_routes = dataValue2.toBool();
											if (nm_static_routes) {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV4DHCPRoutes;
											}
											else {
												connection->ipConfig.connectionFlags |= NetworkIPConfigurationFlags::IPV4DHCPRoutes;
											}
										}
										else if (keyValue.lower() == "never-default") {
											bool nm_can_default_route = !dataValue2.toBool();
											if (nm_can_default_route) {
												connection->ipConfig.connectionFlags |= NetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute;
											}
											else {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute;
											}
										}
										else if (keyValue.lower() == "routes") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQT_DBusDataValueList innerValueList = innerDataValue.toTQValueList();
												TQT_DBusDataValueList::const_iterator it5;
												state = 0;
												NetworkSingleRouteConfiguration routeConfig;
												for (it5 = innerValueList.begin(); it5 != innerValueList.end(); ++it5) {
													TQT_DBusData innerMostDataValue = *it5;
													if (state == 0) {
														// Address
														routeConfig.ipAddress = TQHostAddress(reverseIPV4ByteOrder(innerMostDataValue.toUInt32()));
													}
													else if (state == 1) {
														// Network mask
														routeConfig.networkMask.fromCIDRMask(innerMostDataValue.toUInt32());
													}
													else if (state == 2) {
														// Gateway
														routeConfig.gateway = TQHostAddress(reverseIPV4ByteOrder(innerMostDataValue.toUInt32()));
													}
													else if (state == 3) {
														// Metric
														routeConfig.metric = innerMostDataValue.toUInt32();
													}
													state++;
												}
												routeConfig.valid = true;
												connection->ipConfig.routeConfigurations.append(routeConfig);
											}
										}
										connection->ipConfig.valid = true;
									}
									else if (outerKeyValue.lower() == "ipv6") {
										if (keyValue.lower() == "addresses") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQT_DBusDataValueList innerValueList = innerDataValue.toStruct();
												TQT_DBusDataValueList::const_iterator it5;
												NetworkSingleIPConfiguration ipConfig;
												// Address
												TQT_DBusDataValueList innerMostValueList;
												innerMostValueList = innerValueList[0].toTQValueList();
												TQ_UINT8 nm_v6address[16];
												unsigned char nm_addr_ptr = 0;
												memset(nm_v6address, 0, sizeof(TQ_UINT8)*16);
												for (it5 = innerMostValueList.begin(); it5 != innerMostValueList.end(); ++it5) {
													TQT_DBusData innerMostDataValue = *it5;
													if (nm_addr_ptr < 16) {
														nm_v6address[nm_addr_ptr] = innerMostDataValue.toByte();
														nm_addr_ptr++;
													}
												}
												ipConfig.ipAddress = TQHostAddress(nm_v6address);

												// Netmask
												ipConfig.networkMask.fromCIDRMask(innerValueList[1].toUInt32(), true);

												// Gateway
												memset(nm_v6address, 0, sizeof(TQ_UINT8)*16);
												for (it5 = innerMostValueList.begin(); it5 != innerMostValueList.end(); ++it5) {
													TQT_DBusData innerMostDataValue = *it5;
													if (nm_addr_ptr < 16) {
														nm_v6address[nm_addr_ptr] = innerMostDataValue.toByte();
														nm_addr_ptr++;
													}
												}
												ipConfig.gateway = TQHostAddress(nm_v6address);

												ipConfig.valid = true;
												connection->ipConfig.ipConfigurations.append(ipConfig);
											}
										}
										else if (keyValue.lower() == "dns") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQT_DBusDataValueList innerValueList = innerDataValue.toTQValueList();
												TQT_DBusDataValueList::const_iterator it5;
												TQ_UINT8 nm_v6address[16];
												unsigned char nm_addr_ptr = 0;
												memset(nm_v6address, 0, sizeof(TQ_UINT8)*16);
												for (it5 = innerValueList.begin(); it5 != innerValueList.end(); ++it5) {
													TQT_DBusData innerMostDataValue = *it5;
													if (nm_addr_ptr < 16) {
														nm_v6address[nm_addr_ptr] = innerMostDataValue.toByte();
														nm_addr_ptr++;
													}
												}
												connection->ipConfig.resolvers.append(TQHostAddress(nm_v6address));
											}
										}
										else if (keyValue.lower() == "dns-search") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQT_DBusDataValueList innerValueList = innerDataValue.toTQValueList();
												TQT_DBusDataValueList::const_iterator it5;
												connection->ipConfig.searchDomains.append(NetworkSearchDomain(innerDataValue.toString(), true));
											}
										}
										else if (keyValue.lower() == "ignore-auto-dns") {
											bool nm_static_dns = dataValue2.toBool();
											if (nm_static_dns) {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV4DHCPDNS;
											}
											else {
												connection->ipConfig.connectionFlags |= NetworkIPConfigurationFlags::IPV4DHCPDNS;
											}
										}
										else if (keyValue.lower() == "may-fail") {
											bool nm_may_fail = dataValue2.toBool();
											connection->requireIPV6 = !nm_may_fail;
										}
										else if (keyValue.lower() == "method") {
											TQString nm_method = dataValue2.toString().lower();
											if (nm_method == "auto") {
												connection->ipConfig.connectionFlags |= NetworkIPConfigurationFlags::IPV6DHCPIP;
											}
											else if (nm_method == "manual") {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV6DHCPIP;
											}
											else if (nm_method == "link-local") {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV6LocalOnly;
											}
											else if (nm_method == "shared") {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV6StartConnectionSharingServer;
											}
											else if (nm_method == "ignore") {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV6Disabled;
											}
										}
										else if (keyValue.lower() == "ignore-auto-routes") {
											bool nm_static_routes = dataValue2.toBool();
											if (nm_static_routes) {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV6DHCPRoutes;
											}
											else {
												connection->ipConfig.connectionFlags |= NetworkIPConfigurationFlags::IPV6DHCPRoutes;
											}
										}
										else if (keyValue.lower() == "never-default") {
											bool nm_can_default_route = !dataValue2.toBool();
											if (nm_can_default_route) {
												connection->ipConfig.connectionFlags |= NetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute;
											}
											else {
												connection->ipConfig.connectionFlags &= ~NetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute;
											}
										}
										else if (keyValue.lower() == "routes") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQT_DBusDataValueList innerValueList = innerDataValue.toStruct();
												TQT_DBusDataValueList::const_iterator it5;
												NetworkSingleRouteConfiguration routeConfig;
												// Address
												TQT_DBusDataValueList innerMostValueList;
												innerMostValueList = innerValueList[0].toTQValueList();
												TQ_UINT8 nm_v6address[16];
												unsigned char nm_addr_ptr = 0;
												memset(nm_v6address, 0, sizeof(TQ_UINT8)*16);
												for (it5 = innerMostValueList.begin(); it5 != innerMostValueList.end(); ++it5) {
													TQT_DBusData innerMostDataValue = *it5;
													if (nm_addr_ptr < 16) {
														nm_v6address[nm_addr_ptr] = innerMostDataValue.toByte();
														nm_addr_ptr++;
													}
												}
												routeConfig.ipAddress = TQHostAddress(nm_v6address);

												// Netmask
												routeConfig.networkMask.fromCIDRMask(innerValueList[1].toUInt32(), true);

												// Gateway
												innerMostValueList = innerValueList[2].toTQValueList();
												nm_addr_ptr = 0;
												memset(nm_v6address, 0, sizeof(TQ_UINT8)*16);
												for (it5 = innerMostValueList.begin(); it5 != innerMostValueList.end(); ++it5) {
													TQT_DBusData innerMostDataValue = *it5;
													if (nm_addr_ptr < 16) {
														nm_v6address[nm_addr_ptr] = innerMostDataValue.toByte();
														nm_addr_ptr++;
													}
												}
												routeConfig.gateway = TQHostAddress(nm_v6address);

												// Metric
												routeConfig.metric = innerValueList[3].toUInt32();

												routeConfig.valid = true;
												connection->ipConfig.routeConfigurations.append(routeConfig);
											}
										}
										connection->ipConfig.valid = true;
									}
								}
								else {
									// FIXME
									// There are several advanced properties which appear to use string maps
									// For example, s390-options
									// Support should eventually be added for these, e.g. in a backend-specific Advanced tab somewhere
								}
							}
						}
					}

					// If the connection's MAC matches my MAC, or if the connection is not locked to any MAC address,
					// or if this manager object is not locked to a device, then add this connection to the list
					if ((deviceMACAddress == connection->lockedHWAddress) || (!connection->lockedHWAddress.isValid()) || (!deviceMACAddress.isValid())) {
						loadConnectionAllowedValues(connection);
						m_connectionList->append(connection);
					}
				}
				else {
					// Error!
					PRINT_ERROR((error.name() + ": " + error.message()))
				}
			}
		}
		else {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
		}
		internalNetworkManagementEvent(NetworkGlobalEventType::ConnectionListChanged);
	}

	d->nonReentrantCallActive = false;
}

void NetworkConnectionManager_BackendNM::loadConnectionAllowedValues(NetworkConnection* connection) {
	if (connection) {
		// Insert all allowed EAP phase 2 methods
		connection->eapConfig.allowedPhase2NonEAPMethods.clear();
		connection->eapConfig.allowedPhase2NonEAPMethods.append(NetworkIEEE8021xType::MD5);
		connection->eapConfig.allowedPhase2NonEAPMethods.append(NetworkIEEE8021xType::MSCHAPV2);
		connection->eapConfig.allowedPhase2NonEAPMethods.append(NetworkIEEE8021xType::OTP);
		connection->eapConfig.allowedPhase2NonEAPMethods.append(NetworkIEEE8021xType::GTC);
		connection->eapConfig.allowedPhase2NonEAPMethods.append(NetworkIEEE8021xType::TLS);

		connection->eapConfig.allowedPhase2EAPMethods.clear();
		connection->eapConfig.allowedPhase2EAPMethods.append(NetworkIEEE8021xType::PAP);
		connection->eapConfig.allowedPhase2EAPMethods.append(NetworkIEEE8021xType::CHAP);
		connection->eapConfig.allowedPhase2EAPMethods.append(NetworkIEEE8021xType::MSCHAP);
		connection->eapConfig.allowedPhase2EAPMethods.append(NetworkIEEE8021xType::MSCHAPV2);
		connection->eapConfig.allowedPhase2EAPMethods.append(NetworkIEEE8021xType::OTP);
		connection->eapConfig.allowedPhase2EAPMethods.append(NetworkIEEE8021xType::GTC);
		connection->eapConfig.allowedPhase2EAPMethods.append(NetworkIEEE8021xType::TLS);

		connection->eapConfig.allowedValid = true;
	}
}

// NOTE
// While this separate separate routine is needed to get the secrets, note that secrets must
// be saved using the same connection map save routine that all other settings use above.
bool NetworkConnectionManager_BackendNM::loadConnectionSecrets(TQString uuid) {
	NetworkConnection* connection = findConnectionByUUID(uuid);
	if (!connection) {
		PRINT_ERROR(TQString("Unable to locate connection with uuid '%1' in local database.  Did you run loadConnectionInformation() first?"));
		return FALSE;
	}
	//WiredEthernetConnection* ethernetConnection = dynamic_cast<WiredEthernetConnection*>(connection);
	//WiredInfinibandConnection* infinibandConnection = dynamic_cast<WiredInfinibandConnection*>(connection);
	WiFiConnection* wiFiConnection = dynamic_cast<WiFiConnection*>(connection);
	VPNConnection* vpnConnection = dynamic_cast<VPNConnection*>(connection);
	//WiMaxConnection* wiMaxConnection = dynamic_cast<WiMaxConnection*>(connection);
	//VLANConnection* vlanConnection = dynamic_cast<VLANConnection*>(connection);
	//OLPCMeshConnection* olpcMeshConnection = dynamic_cast<VLANConnection*>(connection);
	//BluetoothConnection* bluetoothConnection = dynamic_cast<BluetoothConnection*>(connection);
	ModemConnection* modemConnection = dynamic_cast<ModemConnection*>(connection);

	bool ret = TRUE;
	ret = ret && loadConnectionSecretsForGroup(uuid, "802-1x");
	if (wiFiConnection) {
		ret = ret && loadConnectionSecretsForGroup(uuid, "802-11-wireless-security");
	}
	if (vpnConnection) {
		ret = ret && loadConnectionSecretsForGroup(uuid, "vpn");
	}
	ret = ret && loadConnectionSecretsForGroup(uuid, "pppoe");
	if (modemConnection) {
		ret = ret && loadConnectionSecretsForGroup(uuid, "cdma");
		ret = ret && loadConnectionSecretsForGroup(uuid, "gsm");
	}
	return ret;
}

bool NetworkConnectionManager_BackendNM::loadConnectionSecretsForGroup(TQString uuid, TQString group) {
	NetworkConnection* connection = findConnectionByUUID(uuid);
	if (!connection) {
		PRINT_ERROR(TQString("Unable to locate connection with uuid '%1' in local database.  Did you run loadConnectionInformation() first?"));
		return FALSE;
	}
	//WiredEthernetConnection* ethernetConnection = dynamic_cast<WiredEthernetConnection*>(connection);
	//WiredInfinibandConnection* infinibandConnection = dynamic_cast<WiredInfinibandConnection*>(connection);
	WiFiConnection* wiFiConnection = dynamic_cast<WiFiConnection*>(connection);
	VPNConnection* vpnConnection = dynamic_cast<VPNConnection*>(connection);
	//WiMaxConnection* wiMaxConnection = dynamic_cast<WiMaxConnection*>(connection);
	//VLANConnection* vlanConnection = dynamic_cast<VLANConnection*>(connection);
	//OLPCMeshConnection* olpcMeshConnection = dynamic_cast<VLANConnection*>(connection);
	//BluetoothConnection* bluetoothConnection = dynamic_cast<BluetoothConnection*>(connection);
	ModemConnection* modemConnection = dynamic_cast<ModemConnection*>(connection);
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool ret;
	TQT_DBusTQStringDataMap connectionSecretsMap(TQT_DBusData::String);
	ret = d->m_networkManagerSettings->GetConnectionByUuid(uuid, existingConnection, error);
	if (ret) {
#ifndef USE_ASYNC_DBUS_CALLS
		// Obtain connection settings from the path specified
		DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
		connectionSettings.setConnection(TQT_DBusConnection::systemBus());
		ret = connectionSettings.GetSecrets(group, connectionSecretsMap, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR((error.name() + ": " + error.message()))
		}
		if (ret) {
#else // USE_ASYNC_DBUS_CALLS
		// Obtain connection secrets from the path specified
		DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
		connectionSettings.setConnection(TQT_DBusConnection::systemBus());
		connect(&connectionSettings, SIGNAL(GetSecretsAsyncReply(int, const TQT_DBusDataMap<TQString>&)), d, SLOT(processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)));
		int asyncCallID;
		ret = connectionSettings.GetSecretsAsync(asyncCallID, group, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR((error.name() + ": " + error.message()))
		}
		if (ret) {
			// Wait for the asynchronous call to return...
			d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
			TQTimer nmCallTimeoutTimer;
			nmCallTimeoutTimer.start(NM_ASYNC_SECRETS_INTERACTION_TIMEOUT_MS, TRUE);
			while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
				tqApp->processEvents();
				if (!nmCallTimeoutTimer.isActive()) {
					PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
					break;
				}
			}
			connectionSecretsMap = d->nmConnectionSettingsAsyncSettingsResponse[asyncCallID];
			if (d->nmConnectionSettingsAsyncSettingsErrorResponse.contains(asyncCallID)) {
				PRINT_ERROR((d->nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallID].name() + ": " + d->nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallID].message()));
				d->nmConnectionSettingsAsyncSettingsErrorResponse.remove(asyncCallID);
			}
			d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
			if (d->nmConnectionSettingsAsyncSettingsResponse.contains(asyncCallID)) {
				d->nmConnectionSettingsAsyncSettingsResponse.remove(asyncCallID);
			}
#endif // USE_ASYNC_DBUS_CALLS

#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
			printf("[network-manager comm debug] received DBUS object structure map follows:\n"); fflush(stdout);
			printDBUSObjectStructure(TQT_DBusData::fromStringKeyMap(connectionSecretsMap));
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS

			// Parse settings
			TQT_DBusTQStringDataMap::const_iterator it2;
			for (it2 = connectionSecretsMap.begin(); it2 != connectionSecretsMap.end(); ++it2) {
				TQString outerKeyValue = it2.key();
				TQT_DBusData dataValue = it2.data();
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
				printf("[network-manager comm debug] [%s]\n", outerKeyValue.ascii()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
				TQT_DBusTQStringDataMap nestedConnectionSettingsMap = dataValue.toStringKeyMap();
				TQT_DBusTQStringDataMap::const_iterator it3;
				for (it3 = nestedConnectionSettingsMap.begin(); it3 != nestedConnectionSettingsMap.end(); ++it3) {
					TQString keyValue = it3.key();
					TQT_DBusData dataValue = it3.data();
					if (dataValue.type() != TQT_DBusData::Variant) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
						printf("[network-manager comm debug] %s = %s (type %d(%s))\n", keyValue.ascii(), dataValue.toString().ascii(), dataValue.type(), dataValue.typeName()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
						// No NM settings are known which use this style
					}
					else {
						TQT_DBusVariant dataValueVariant = dataValue.toVariant();
						TQT_DBusData dataValue2 = dataValueVariant.value;
						if (dataValue2.type() != TQT_DBusData::Variant) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
							printf("[network-manager comm debug] %s = %s (type %d(%s), signature %s)\n", keyValue.ascii(), dataValue2.toString().ascii(), dataValue2.type(), dataValue2.typeName(), dataValueVariant.signature.ascii()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
							// Most NM settings use this style
							if (outerKeyValue.lower() == "802-1x") {
								if (keyValue.lower() == "password") {
									connection->eapConfig.password = dataValue2.toString();
								}
								else if (keyValue.lower() == "password-raw") {
									TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
									TQT_DBusDataValueList::const_iterator it4;
									int count=0;
									for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
										TQT_DBusData innerDataValue = *it4;
										count++;
										connection->eapConfig.binaryPassword.resize(count+1);
										connection->eapConfig.binaryPassword[count] = innerDataValue.toByte();
									}
								}
								else if (keyValue.lower() == "private-key-password") {
									connection->eapConfig.privateKeyPassword = dataValue2.toString();
								}
								else if (keyValue.lower() == "phase2-private-key-password") {
									connection->eapConfig.phase2PrivateKeyPassword = dataValue2.toString();
								}
								connection->eapConfig.secretsValid = true;
							}
							if ((outerKeyValue.lower() == "802-11-wireless-security") && (wiFiConnection)) {
								if (keyValue.lower() == "wep-key0") {
									wiFiConnection->securitySettings.wepKey0 = dataValue2.toString();
									wiFiConnection->securitySettings.wepKeyType = nmWepKeyTypeToTDEWepKeyType(tdeWepKeyTypeToNMWepKeyType(wiFiConnection->securitySettings.wepKeyType), wiFiConnection->securitySettings.wepKey0);
								}
								else if (keyValue.lower() == "wep-key1") {
									wiFiConnection->securitySettings.wepKey1 = dataValue2.toString();
									wiFiConnection->securitySettings.wepKeyType = nmWepKeyTypeToTDEWepKeyType(tdeWepKeyTypeToNMWepKeyType(wiFiConnection->securitySettings.wepKeyType), wiFiConnection->securitySettings.wepKey1);
								}
								else if (keyValue.lower() == "wep-key2") {
									wiFiConnection->securitySettings.wepKey2 = dataValue2.toString();
									wiFiConnection->securitySettings.wepKeyType = nmWepKeyTypeToTDEWepKeyType(tdeWepKeyTypeToNMWepKeyType(wiFiConnection->securitySettings.wepKeyType), wiFiConnection->securitySettings.wepKey2);
								}
								else if (keyValue.lower() == "wep-key3") {
									wiFiConnection->securitySettings.wepKey3 = dataValue2.toString();
									wiFiConnection->securitySettings.wepKeyType = nmWepKeyTypeToTDEWepKeyType(tdeWepKeyTypeToNMWepKeyType(wiFiConnection->securitySettings.wepKeyType), wiFiConnection->securitySettings.wepKey3);
								}
								else if (keyValue.lower() == "psk") {
									wiFiConnection->securitySettings.psk = dataValue2.toString();
								}
								else if (keyValue.lower() == "eap-password") {
									wiFiConnection->securitySettings.leapPassword = dataValue2.toString();
								}
							}
							if ((outerKeyValue.lower() == "vpn") && (vpnConnection)) {
								if (keyValue.lower() == "secrets") {
									TQT_DBusTQStringDataMap nestedConnectionSettingsMap = dataValue2.toStringKeyMap();
									TQT_DBusTQStringDataMap::const_iterator it4;
									for (it4 = nestedConnectionSettingsMap.begin(); it4 != nestedConnectionSettingsMap.end(); ++it4) {
										vpnConnection->pluginSecrets.clear();
										TQString keyValue4 = it4.key();
										TQT_DBusData dataValue4 = it4.data();
										if (dataValue4.type() == TQT_DBusData::String) {
											vpnConnection->pluginSecrets[keyValue4] = dataValue4.toString();
										}
									}
									vpnConnection->secretsValid = true;
								}
							}
							if (outerKeyValue.lower() == "pppoe") {
								if (keyValue.lower() == "password") {
									connection->pppoeConfig.password = dataValue2.toString();
								}
								connection->pppoeConfig.secretsValid = true;
							}
							if (outerKeyValue.lower() == "cdma") {
								if (keyValue.lower() == "password") {
									modemConnection->cdmaConfig.password = dataValue2.toString();
								}
								modemConnection->cdmaConfig.secretsValid = true;
							}
							if (outerKeyValue.lower() == "gsm") {
								if (keyValue.lower() == "password") {
									modemConnection->gsmConfig.password = dataValue2.toString();
								}
								else if (keyValue.lower() == "pin") {
									modemConnection->gsmConfig.pin = dataValue2.toString();
								}
								modemConnection->gsmConfig.secretsValid = true;
							}
						}
					}
				}
			}
			return TRUE;
		}
		else {
			PRINT_ERROR(TQString("Unable to load secrets for connection with uuid '%1'").arg(uuid))
			return FALSE;
		}
	}
	else {
		PRINT_WARNING(TQString("connection for provided uuid '%1' was not found").arg(uuid));
		return FALSE;
	}
}

bool NetworkConnectionManager_BackendNM::saveConnection(NetworkConnection* connection) {
	bool timed_out = FALSE;
	bool command_failed = FALSE;

	if (!connection) {
		PRINT_ERROR(TQString("connection cannot be NULL!"));
		return FALSE;
	}

	// If the UUID is blank, generate a new UUID for this connection and also guarantee that it it truly unique
	if (connection->UUID == "") {
		bool unique = false;
		while (!unique) {
			connection->UUID = TQUuid::createUuid().toString();
			connection->UUID.replace("{", "");
			connection->UUID.replace("}", "");
			if (!findConnectionByUUID(connection->UUID)) {
				unique = true;
			}
		}
	}

	// Find path for connection with specified UUID, if it exists
	// This is so that any settings that we are not aware of can be loaded now and preserved through the update operation
	WiredEthernetConnection* ethernetConnection = dynamic_cast<WiredEthernetConnection*>(connection);
	WiredInfinibandConnection* infinibandConnection = dynamic_cast<WiredInfinibandConnection*>(connection);
	WiFiConnection* wiFiConnection = dynamic_cast<WiFiConnection*>(connection);
	VPNConnection* vpnConnection = dynamic_cast<VPNConnection*>(connection);
	WiMaxConnection* wiMaxConnection = dynamic_cast<WiMaxConnection*>(connection);
	VLANConnection* vlanConnection = dynamic_cast<VLANConnection*>(connection);
	OLPCMeshConnection* olpcMeshConnection = dynamic_cast<OLPCMeshConnection*>(connection);
	BluetoothConnection* bluetoothConnection = dynamic_cast<BluetoothConnection*>(connection);
	ModemConnection* modemConnection = dynamic_cast<ModemConnection*>(connection);
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool ret;
	bool existing;
	TQT_DBusTQStringDataMap connectionSettingsMap(TQT_DBusData::String);
	existing = false;
	ret = d->m_networkManagerSettings->GetConnectionByUuid(connection->UUID, existingConnection, error);
	if (ret) {
#ifndef USE_ASYNC_DBUS_CALLS
		// Obtain connection settings from the path specified
		DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
		connectionSettings.setConnection(TQT_DBusConnection::systemBus());
		ret = connectionSettings.GetSettings(connectionSettingsMap, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR((error.name() + ": " + error.message()))
		}
		if (ret) {
#else // USE_ASYNC_DBUS_CALLS
		// Obtain connection settings from the path specified
		DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
		connectionSettings.setConnection(TQT_DBusConnection::systemBus());
		connect(&connectionSettings, SIGNAL(GetSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)), d, SLOT(processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)));
		int asyncCallID;
		ret = connectionSettings.GetSettingsAsync(asyncCallID, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR((error.name() + ": " + error.message()))
		}
		if (ret) {
			// Wait for the asynchronous call to return...
			d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
			TQTimer nmCallTimeoutTimer;
			nmCallTimeoutTimer.start(NM_ASYNC_TIMEOUT_MS, TRUE);
			while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
				tqApp->processEvents();
				if (!nmCallTimeoutTimer.isActive()) {
					PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
					timed_out = true;
					break;
				}
			}
			connectionSettingsMap = d->nmConnectionSettingsAsyncSettingsResponse[asyncCallID];
			if (d->nmConnectionSettingsAsyncSettingsErrorResponse.contains(asyncCallID)) {
				PRINT_ERROR((d->nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallID].name() + ": " + d->nmConnectionSettingsAsyncSettingsErrorResponse[asyncCallID].message()));
				d->nmConnectionSettingsAsyncSettingsErrorResponse.remove(asyncCallID);
			}
			d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
			if (d->nmConnectionSettingsAsyncSettingsResponse.contains(asyncCallID)) {
				d->nmConnectionSettingsAsyncSettingsResponse.remove(asyncCallID);
			}
#endif // USE_ASYNC_DBUS_CALLS
			existing = true;
		}
	}

	// Create and/or update settings map from provided connection information
	// We start at the outermost layer and work our way inwards, in a structure which should match the parser in loadConnectionInformation() very closely
	bool groupValid;
	TQT_DBusData dbusData;
	TQT_DBusData innerDbusData;
	TQMap<TQString, TQT_DBusData> outerMap = connectionSettingsMap.toTQMap();
	{
		groupValid = false;
		dbusData = outerMap["connection"];
		{
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				settingsMap["id"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(connection->friendlyName));
				{
					TQString type;
					if (ethernetConnection) type = "802-3-ethernet";
					else if (infinibandConnection) type = "infiniband";
					else if (wiFiConnection) type = "802-11-wireless";
					else if (vpnConnection) type = "vpn";
					else if (wiMaxConnection) type = "wimax";
					else if (vlanConnection) type = "vlan";
					else if (olpcMeshConnection) type = "802-11-olpc-mesh";
					else if (bluetoothConnection) type = "bluetooth";
					else if (modemConnection) {
						if (modemConnection->type == ModemConnectionType::CDMA) {
							type = "cdma";
						}
						else if (modemConnection->type == ModemConnectionType::GSM) {
							type = "gsm";
						}
					}
					if (!type.isNull()) settingsMap["type"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(type));
				}
				settingsMap["uuid"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(connection->UUID));
				{
					TQT_DBusDataValueList valueList;
					{
						for (TQStringList::Iterator it = connection->authorizedUsers.begin(); it != connection->authorizedUsers.end(); ++it) {
							TQString assembledString = TQString("user:%1:").arg(*it);
							valueList.append(TQT_DBusData::fromString(assembledString));
						}
					}
					if (valueList.count() > 0) settingsMap["permissions"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("permissions");
				}
				settingsMap["autoconnect"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->autoConnect));
				settingsMap["read-only"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->readOnly));
				UPDATE_STRING_SETTING_IF_VALID(connection->masterConnectionUUID, "master", settingsMap)
				{
					TQString slaveType = tdeSlaveTypeToNMSlaveType(connection->slaveType);
					if (slaveType != "") settingsMap["slave-type"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(slaveType));
					else settingsMap.remove("slave-type");
				}
				// settingsMap["timestamp"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt64(connection->lastKnownConnection.toTime_t()));	// Probably read-only to us
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("connection", dbusData, TRUE); else outerMap.remove("connection");

		groupValid = false;
		dbusData = outerMap["802-1x"];
		{
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						// WARNING: The exact order of the data in the list is critical, therefore extreme caution should be exercised when modifying the code below!
						// EAP type
						valueList.append(TQT_DBusData::fromString(tdeEAPTypeToNMEAPType(connection->eapConfig.type)));
					}
					settingsMap["eap"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				else {
					settingsMap.remove("eap");
				}
				if (connection->eapConfig.valid) {
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.userName, "identity", settingsMap)
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.anonymousUserName, "anonymous-identity", settingsMap)
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.pacFileName, "pac-file", settingsMap)
				}
				else {
					settingsMap.remove("identity");
					settingsMap.remove("anonymous-identity");
					settingsMap.remove("pac-file");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						unsigned int count;
						for (count=0; count<connection->eapConfig.caCertificate.count(); count++) {
							valueList.append(TQT_DBusData::fromByte(connection->eapConfig.caCertificate[count]));
						}
					}
					if (valueList.count() > 0) settingsMap["ca-cert"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("ca-cert");
				}
				else {
					settingsMap.remove("ca-cert");
				}
				if (connection->eapConfig.valid) {
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.additionalCAFilesPath, "ca-path", settingsMap)
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.authServerCertSubjectMatch, "subject-match", settingsMap)
				}
				else {
					settingsMap.remove("ca-path");
					settingsMap.remove("subject-match");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						for (TQStringList::Iterator it = connection->eapConfig.alternateAuthServerCertSubjectMatch.begin(); it != connection->eapConfig.alternateAuthServerCertSubjectMatch.end(); ++it) {
							valueList.append(TQT_DBusData::fromString(*it));
						}
					}
					if (valueList.count() > 0) settingsMap["altsubject-matches"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("altsubject-matches");
				}
				else {
					settingsMap.remove("altsubject-matches");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						unsigned int count;
						for (count=0; count<connection->eapConfig.clientCertificate.count(); count++) {
							valueList.append(TQT_DBusData::fromByte(connection->eapConfig.clientCertificate[count]));
						}
					}
					if (valueList.count() > 0) settingsMap["client-cert"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("client-cert");
				}
				else {
					settingsMap.remove("client-cert");
				}
				if (connection->eapConfig.valid) {
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.forcePEAPVersion, "phase1-peapver", settingsMap)
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.forcePEAPLabel, "phase1-peaplabel", settingsMap)
					UPDATE_STRING_SETTING_IF_VALID(tdeEAPFastFlagsToNMEAPFastFlags(connection->eapConfig.fastProvisioningFlags), "phase1-fast-provisioning", settingsMap)
				}
				else {
					settingsMap.remove("phase1-peapver");
					settingsMap.remove("phase1-peaplabel");
					settingsMap.remove("phase1-fast-provisioning");
				}
				if (connection->eapConfig.valid) {
					settingsMap["phase2-auth"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(tdeEAPTypeToNMEAPType(connection->eapConfig.phase2NonEAPAuthMethod)));
					settingsMap["phase2-autheap"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(tdeEAPTypeToNMEAPType(connection->eapConfig.phase2EAPAuthMethod)));
				}
				else {
					settingsMap.remove("phase2-auth");
					settingsMap.remove("phase2-autheap");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						unsigned int count;
						for (count=0; count<connection->eapConfig.phase2CaCertificate.count(); count++) {
							valueList.append(TQT_DBusData::fromByte(connection->eapConfig.phase2CaCertificate[count]));
						}
					}
					if (valueList.count() > 0) settingsMap["phase2-ca-cert"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("phase2-ca-cert");
				}
				else {
					settingsMap.remove("phase2-ca-cert");
				}
				if (connection->eapConfig.valid) {
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.phase2CaFilesPath, "phase2-ca-path", settingsMap)
					UPDATE_STRING_SETTING_IF_VALID(connection->eapConfig.phase2AuthServerCertSubjectMatch, "phase2-subject-match", settingsMap)
				}
				else {
					settingsMap.remove("phase2-ca-path");
					settingsMap.remove("phase2-subject-match");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						for (TQStringList::Iterator it = connection->eapConfig.phase2AlternateAuthServerCertSubjectMatch.begin(); it != connection->eapConfig.phase2AlternateAuthServerCertSubjectMatch.end(); ++it) {
							valueList.append(TQT_DBusData::fromString(*it));
						}
					}
					if (valueList.count() > 0) settingsMap["phase2-altsubject-matches"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("phase2-altsubject-matches");
				}
				else {
					settingsMap.remove("phase2-altsubject-matches");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						unsigned int count;
						for (count=0; count<connection->eapConfig.phase2ClientCertificate.count(); count++) {
							valueList.append(TQT_DBusData::fromByte(connection->eapConfig.phase2ClientCertificate[count]));
						}
					}
					if (valueList.count() > 0) settingsMap["phase2-client-cert"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("phase2-client-cert");
				}
				else {
					settingsMap.remove("phase2-client-cert");
				}
				if (connection->eapConfig.valid) {
					settingsMap["password-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(connection->eapConfig.passwordFlags)));
					settingsMap["password-raw-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(connection->eapConfig.binaryPasswordFlags)));
				}
				else {
					settingsMap.remove("password-flags");
					settingsMap.remove("password-raw-flags");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						unsigned int count;
						for (count=0; count<connection->eapConfig.privateKey.count(); count++) {
							valueList.append(TQT_DBusData::fromByte(connection->eapConfig.privateKey[count]));
						}
					}
					if (valueList.count() > 0) settingsMap["private-key"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("private-key");
				}
				else {
					settingsMap.remove("private-key");
				}
				if (connection->eapConfig.valid) {
					settingsMap["private-key-password-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(connection->eapConfig.privateKeyPasswordFlags)));
				}
				else {
					settingsMap.remove("private-key-password-flags");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						unsigned int count;
						for (count=0; count<connection->eapConfig.phase2PrivateKey.count(); count++) {
							valueList.append(TQT_DBusData::fromByte(connection->eapConfig.phase2PrivateKey[count]));
						}
					}
					if (valueList.count() > 0) settingsMap["phase2-private-key"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("phase2-private-key");
				}
				else {
					settingsMap.remove("phase2-private-key");
				}
				if (connection->eapConfig.valid) {
					settingsMap["phase2-private-key-password-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(connection->eapConfig.phase2PrivateKeyPasswordFlags)));
				}
				else {
					settingsMap.remove("phase2-private-key-password-flags");
				}
				if (connection->eapConfig.valid) {
					settingsMap["system-ca-certs"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->eapConfig.forceSystemCaCertificates));
				}
				else {
					settingsMap.remove("system-ca-certs");
				}
				if (connection->eapConfig.secretsValid) {
					settingsMap["password"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(connection->eapConfig.password));
				}
				else {
					settingsMap.remove("password");
				}
				if (connection->eapConfig.valid) {
					TQT_DBusDataValueList valueList;
					{
						unsigned int count;
						for (count=0; count<connection->eapConfig.binaryPassword.count(); count++) {
							valueList.append(TQT_DBusData::fromByte(connection->eapConfig.binaryPassword[count]));
						}
					}
					if (valueList.count() > 0) settingsMap["password-raw"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("password-raw");
				}
				else {
					settingsMap.remove("password-raw");
				}
				if (connection->eapConfig.secretsValid) {
					settingsMap["private-key-password"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(connection->eapConfig.privateKeyPassword));
					settingsMap["phase2-private-key-password"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(connection->eapConfig.phase2PrivateKeyPassword));
				}
				else {
					settingsMap.remove("private-key-password");
					settingsMap.remove("phase2-private-key-password");
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("802-1x", dbusData, TRUE); else outerMap.remove("802-1x");

		groupValid = false;
		dbusData = outerMap["802-3-ethernet"];
		if (ethernetConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				settingsMap["duplex"] = convertDBUSDataToVariantData(TQT_DBusData::fromString((connection->fullDuplex)?"full":"half"));
				if (connection->lockedHWAddress.isValid()) {
					NetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					NetworkByteList::iterator it;
					for (it = address.begin(); it != address.end(); ++it) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(*it);
						valueList.append(innerDataValue);
					}
					TQT_DBusData nmHWAddress = TQT_DBusData::fromTQValueList(valueList);
					settingsMap["mac-address"] = convertDBUSDataToVariantData(nmHWAddress);
				}
				else {
					settingsMap.remove("mac-address");
				}
				if (connection->manualHWAddress.isValid()) {
					NetworkByteList address = connection->manualHWAddress.address();
					TQT_DBusDataValueList valueList;
					NetworkByteList::iterator it;
					for (it = address.begin(); it != address.end(); ++it) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(*it);
						valueList.append(innerDataValue);
					}
					TQT_DBusData nmHWAddress = TQT_DBusData::fromTQValueList(valueList);
					settingsMap["cloned-mac-address"] = convertDBUSDataToVariantData(nmHWAddress);
				}
				else {
					settingsMap.remove("cloned-mac-address");
				}
				if (connection->mtu > 0) {
					settingsMap["mtu"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->mtu));
				}
				else {
					settingsMap.remove("mtu");
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("802-3-ethernet", dbusData, TRUE); else outerMap.remove("802-3-ethernet");

		groupValid = false;
		dbusData = outerMap["infiniband"];
		if (infinibandConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				if (connection->lockedHWAddress.isValid()) {
					NetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					NetworkByteList::iterator it;
					for (it = address.begin(); it != address.end(); ++it) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(*it);
						valueList.append(innerDataValue);
					}
					TQT_DBusData nmHWAddress = TQT_DBusData::fromTQValueList(valueList);
					settingsMap["mac-address"] = convertDBUSDataToVariantData(nmHWAddress);
				}
				else {
					settingsMap.remove("mac-address");
				}
				if (connection->mtu > 0) {
					settingsMap["mtu"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->mtu));
				}
				else {
					settingsMap.remove("mtu");
				}
				UPDATE_STRING_SETTING_IF_VALID(tdeIBTransportToNMIBTransport(infinibandConnection->transportMode), "transport-mode", settingsMap)
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("infiniband", dbusData, TRUE); else outerMap.remove("infiniband");

		groupValid = false;
		dbusData = outerMap["802-11-wireless"];
		if (wiFiConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				{
					unsigned int i;
					TQT_DBusDataValueList valueList;
					for (i=0; i<wiFiConnection->SSID.count(); i++) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(wiFiConnection->SSID[i]);
						valueList.append(innerDataValue);
					}
					settingsMap["ssid"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				if (connection->lockedHWAddress.isValid()) {
					NetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					NetworkByteList::iterator it;
					for (it = address.begin(); it != address.end(); ++it) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(*it);
						valueList.append(innerDataValue);
					}
					TQT_DBusData nmHWAddress = TQT_DBusData::fromTQValueList(valueList);
					settingsMap["mac-address"] = convertDBUSDataToVariantData(nmHWAddress);
				}
				else {
					settingsMap.remove("mac-address");
				}
				if (connection->manualHWAddress.isValid()) {
					NetworkByteList address = connection->manualHWAddress.address();
					TQT_DBusDataValueList valueList;
					NetworkByteList::iterator it;
					for (it = address.begin(); it != address.end(); ++it) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(*it);
						valueList.append(innerDataValue);
					}
					TQT_DBusData nmHWAddress = TQT_DBusData::fromTQValueList(valueList);
					settingsMap["cloned-mac-address"] = convertDBUSDataToVariantData(nmHWAddress);
				}
				else {
					settingsMap.remove("cloned-mac-address");
				}
				if (connection->mtu > 0) {
					settingsMap["mtu"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->mtu));
				}
				else {
					settingsMap.remove("mtu");
				}
				UPDATE_STRING_SETTING_IF_VALID(tdeWiFiModeToNMWiFiMode(wiFiConnection->operatingMode), "mode", settingsMap)
				UPDATE_STRING_SETTING_IF_VALID(tdeWiFiFrequencyBandToNMWiFiFrequencyBand(wiFiConnection->bandRestriction), "band", settingsMap)
				if (wiFiConnection->channelRestriction > 0) {
					settingsMap["channel"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(wiFiConnection->channelRestriction));
				}
				else {
					settingsMap.remove("channel");
				}
				if (wiFiConnection->bitRateRestriction > 0) {
					settingsMap["rate"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(wiFiConnection->bitRateRestriction/1000));
				}
				else {
					settingsMap.remove("rate");
				}
				if (wiFiConnection->powerRestriction > 0) {
					settingsMap["tx-power"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(wiFiConnection->powerRestriction));
				}
				else {
					settingsMap.remove("tx-power");
				}
				if (wiFiConnection->accessPointRestriction.isValid()) {
					NetworkByteList address = wiFiConnection->accessPointRestriction.address();
					TQT_DBusDataValueList valueList;
					NetworkByteList::iterator it;
					for (it = address.begin(); it != address.end(); ++it) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(*it);
						valueList.append(innerDataValue);
					}
					TQT_DBusData nmHWAddress = TQT_DBusData::fromTQValueList(valueList);
					settingsMap["bssid"] = convertDBUSDataToVariantData(nmHWAddress);
				}
				else {
					settingsMap.remove("bssid");
				}
				{
					TQT_DBusDataValueList valueList;
					MACAddressList::iterator it;
					for (it = wiFiConnection->blacklistedBSSIDs.begin(); it != wiFiConnection->blacklistedBSSIDs.end(); ++it) {
						valueList.append(TQT_DBusData::fromString((*it).toString()));
					}
					if (valueList.count() > 0) settingsMap["mac-address-blacklist"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					TQT_DBusDataValueList valueList;
					MACAddressList::iterator it;
					for (it = wiFiConnection->heardBSSIDs.begin(); it != wiFiConnection->heardBSSIDs.end(); ++it) {
						valueList.append(TQT_DBusData::fromString((*it).toString()));
					}
					if (valueList.count() > 0) settingsMap["seen-bssids"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					if (wiFiConnection->securityRequired) {
						settingsMap["security"] = convertDBUSDataToVariantData(TQT_DBusData::fromString("802-11-wireless-security"));
					}
					else {
						settingsMap.remove("security");
					}
				}
				{
					if (wiFiConnection->isHiddenNetwork) {
						settingsMap["hidden"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(true));
					}
					else {
						settingsMap.remove("hidden");
					}
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("802-11-wireless", dbusData, TRUE); else outerMap.remove("802-11-wireless");

		groupValid = false;
		dbusData = outerMap["802-11-wireless-security"];
		if (wiFiConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				if (wiFiConnection->securityRequired) {
					if (wiFiConnection->securityRequired) {
						settingsMap["key-mgmt"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(tdeWiFiKeyTypeToNMWiFiKeyType(wiFiConnection->securitySettings.keyType)));
					}
					if (wiFiConnection->securitySettings.wepKeyIndex > 0) {
						settingsMap["wep-tx-keyidx"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(wiFiConnection->securitySettings.wepKeyIndex));
					}
					else {
						settingsMap.remove("wep-tx-keyidx");
					}
					UPDATE_STRING_SETTING_IF_VALID(tdeWiFiAuthTypeToNMWiFiAuthType(wiFiConnection->securitySettings.authType), "auth-alg", settingsMap)
					{
						TQT_DBusDataValueList valueList;
						{
							TQStringList strings = tdeWiFiWPAVersionToNMWiFiWPAVersion(wiFiConnection->securitySettings.wpaVersion);
							for (TQStringList::Iterator it = strings.begin(); it != strings.end(); ++it) {
								valueList.append(TQT_DBusData::fromString(*it));
							}
						}
						if (valueList.count() > 0) settingsMap["proto"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
						else settingsMap.remove("proto");
					}
					{
						TQT_DBusDataValueList valueList;
						{
							if (wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::Any)) {
								if (!wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherWEP40)) wiFiConnection->securitySettings.allowedPairWiseCiphers.append(NetworkWiFiConnectionCipher::CipherWEP40);
								if (!wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherWEP104)) wiFiConnection->securitySettings.allowedPairWiseCiphers.append(NetworkWiFiConnectionCipher::CipherWEP104);
								if (!wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherTKIP)) wiFiConnection->securitySettings.allowedPairWiseCiphers.append(NetworkWiFiConnectionCipher::CipherTKIP);
								if (!wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherCCMP)) wiFiConnection->securitySettings.allowedPairWiseCiphers.append(NetworkWiFiConnectionCipher::CipherCCMP);
							}
							for (NetworkWiFiConnectionCipherList::Iterator it = wiFiConnection->securitySettings.allowedPairWiseCiphers.begin(); it != wiFiConnection->securitySettings.allowedPairWiseCiphers.end(); ++it) {
								valueList.append(TQT_DBusData::fromString(tdeWiFiCipherToNMWiFiCipher(*it)));
							}
						}
						if (valueList.count() > 0) settingsMap["pairwise"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
						else settingsMap.remove("pairwise");
					}
					{
						TQT_DBusDataValueList valueList;
						{
							if (wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::Any)) {
								if (!wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherWEP40)) wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(NetworkWiFiConnectionCipher::CipherWEP40);
								if (!wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherWEP104)) wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(NetworkWiFiConnectionCipher::CipherWEP104);
								if (!wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherTKIP)) wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(NetworkWiFiConnectionCipher::CipherTKIP);
								if (!wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(NetworkWiFiConnectionCipher::CipherCCMP)) wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(NetworkWiFiConnectionCipher::CipherCCMP);
							}
							for (NetworkWiFiConnectionCipherList::Iterator it = wiFiConnection->securitySettings.allowedGroupWiseCiphers.begin(); it != wiFiConnection->securitySettings.allowedGroupWiseCiphers.end(); ++it) {
								valueList.append(TQT_DBusData::fromString(tdeWiFiCipherToNMWiFiCipher(*it)));
							}
						}
						if (valueList.count() > 0) settingsMap["group"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
						else settingsMap.remove("group");
					}
					UPDATE_STRING_SETTING_IF_VALID(wiFiConnection->securitySettings.leapUsername, "leap-username", settingsMap)
					settingsMap["wep-key-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(wiFiConnection->securitySettings.wepKeyFlags)));
					settingsMap["wep-key-type"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdeWepKeyTypeToNMWepKeyType(wiFiConnection->securitySettings.wepKeyType)));
					settingsMap["psk-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(wiFiConnection->securitySettings.pskFlags)));
					settingsMap["leap-password-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(wiFiConnection->securitySettings.leapPasswordFlags)));
					if (wiFiConnection->securitySettings.secretsValid) {
						UPDATE_STRING_SETTING_IF_VALID(wiFiConnection->securitySettings.wepKey0, "wep-key0", settingsMap)
						UPDATE_STRING_SETTING_IF_VALID(wiFiConnection->securitySettings.wepKey1, "wep-key1", settingsMap)
						UPDATE_STRING_SETTING_IF_VALID(wiFiConnection->securitySettings.wepKey2, "wep-key2", settingsMap)
						UPDATE_STRING_SETTING_IF_VALID(wiFiConnection->securitySettings.wepKey3, "wep-key3", settingsMap)
						UPDATE_STRING_SETTING_IF_VALID(wiFiConnection->securitySettings.psk, "psk", settingsMap)
						UPDATE_STRING_SETTING_IF_VALID(wiFiConnection->securitySettings.leapPassword, "leap-password", settingsMap)
					}
					else {
						settingsMap.remove("wep-key0");
						settingsMap.remove("wep-key1");
						settingsMap.remove("wep-key2");
						settingsMap.remove("wep-key3");
						settingsMap.remove("psk");
						settingsMap.remove("leap-password");
					}
				}
				else {
					settingsMap.remove("key-mgmt");
					settingsMap.remove("wep-tx-keyidx");
					settingsMap.remove("auth-alg");
					settingsMap.remove("proto");
					settingsMap.remove("pairwise");
					settingsMap.remove("group");
					settingsMap.remove("leap-username");
					settingsMap.remove("wep-key-flags");
					settingsMap.remove("wep-key-type");
					settingsMap.remove("psk-flags");
					settingsMap.remove("leap-password-flags");
					settingsMap.remove("wep-key0");
					settingsMap.remove("wep-key1");
					settingsMap.remove("wep-key2");
					settingsMap.remove("wep-key3");
					settingsMap.remove("psk");
					settingsMap.remove("leap-password");
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("802-11-wireless-security", dbusData, TRUE); else outerMap.remove("802-11-wireless-security");

		groupValid = false;
		dbusData = outerMap["vpn"];
		if (vpnConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				{
					TQString pluginService = vpnConnection->vpnPluginID;
					if (pluginService != "") {
						pluginService = "org.freedesktop.NetworkManager." + pluginService;
					}
					UPDATE_STRING_SETTING_IF_VALID(pluginService, "service-type", settingsMap)
				}
				UPDATE_STRING_SETTING_IF_VALID(vpnConnection->lockedUserName, "user-name", settingsMap)
				{
					TQMap<TQString, TQT_DBusData> nestedConnectionSettingsMap;
					NetworkSettingsMap::const_iterator it;
					for (it = vpnConnection->pluginData.begin(); it != vpnConnection->pluginData.end(); ++it) {
						nestedConnectionSettingsMap[it.key()] = TQT_DBusData::fromString(it.data());
					}
					if (nestedConnectionSettingsMap.count() > 0) settingsMap["data"] = convertDBUSDataToVariantData(TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(nestedConnectionSettingsMap)));
					else settingsMap.remove("data");
				}
				if (vpnConnection->secretsValid) {
					TQMap<TQString, TQT_DBusData> nestedConnectionSettingsMap;
					NetworkSettingsMap::const_iterator it;
					for (it = vpnConnection->pluginSecrets.begin(); it != vpnConnection->pluginSecrets.end(); ++it) {
						nestedConnectionSettingsMap[it.key()] = TQT_DBusData::fromString(it.data());
					}
					if (nestedConnectionSettingsMap.count() > 0) settingsMap["secrets"] = convertDBUSDataToVariantData(TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(nestedConnectionSettingsMap)));
					else settingsMap.remove("secrets");
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("vpn", dbusData, TRUE); else outerMap.remove("vpn");

		groupValid = false;
		dbusData = outerMap["wimax"];
		if (wiMaxConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				if (connection->lockedHWAddress.isValid()) {
					NetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					NetworkByteList::iterator it;
					for (it = address.begin(); it != address.end(); ++it) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(*it);
						valueList.append(innerDataValue);
					}
					TQT_DBusData nmHWAddress = TQT_DBusData::fromTQValueList(valueList);
					settingsMap["mac-address"] = convertDBUSDataToVariantData(nmHWAddress);
				}
				else {
					settingsMap.remove("mac-address");
				}
				UPDATE_STRING_SETTING_IF_VALID(wiMaxConnection->networkServiceProvider, "network-name", settingsMap)
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("wimax", dbusData, TRUE); else outerMap.remove("wimax");

		groupValid = false;
		dbusData = outerMap["vlan"];
		if (vlanConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				UPDATE_STRING_SETTING_IF_VALID(vlanConnection->kernelName, "interface-name", settingsMap)
				UPDATE_STRING_SETTING_IF_VALID(vlanConnection->parentConnectionUUID, "parent", settingsMap)
				settingsMap["id"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(vlanConnection->vlanID));
				settingsMap["flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdeVLANFlagsToNMVLANFlags(vlanConnection->vlanFlags)));
				{
					TQT_DBusDataValueList valueList;
					NetworkPriorityMap::const_iterator it;
					for (it = vlanConnection->ingressPriorityMap.begin(); it != vlanConnection->ingressPriorityMap.end(); ++it) {
						valueList.append(TQT_DBusData::fromString(TQString("%1:%2").arg(it.key()).arg(it.data())));
					}
					if (valueList.count() > 0) settingsMap["ingress-priority-map"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					TQT_DBusDataValueList valueList;
					NetworkPriorityMap::const_iterator it;
					for (it = vlanConnection->egressPriorityMap.begin(); it != vlanConnection->egressPriorityMap.end(); ++it) {
						valueList.append(TQT_DBusData::fromString(TQString("%1:%2").arg(it.key()).arg(it.data())));
					}
					if (valueList.count() > 0) settingsMap["egress-priority-map"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("vlan", dbusData, TRUE); else outerMap.remove("vlan");

		groupValid = false;
		dbusData = outerMap["serial"];
		if (connection->serialConfig.valid) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				settingsMap["baud"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->serialConfig.baudRate));
				settingsMap["bits"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->serialConfig.byteWidth));
				settingsMap["parity"] = convertDBUSDataToVariantData(TQT_DBusData::fromByte(tdeParityToNMParity(connection->serialConfig.parity)));
				settingsMap["stopbits"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->serialConfig.stopBits));
				settingsMap["send-delay"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt64(connection->serialConfig.txDelay));
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("serial", dbusData, TRUE); else outerMap.remove("serial");

		groupValid = false;
		dbusData = outerMap["ppp"];
		if (connection->pppConfig.valid) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				settingsMap["noauth"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->pppConfig.requireServerAuthentication)));
				settingsMap["refuse-eap"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::DisableEAP));
				settingsMap["refuse-pap"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::DisablePAP));
				settingsMap["refuse-chap"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::DisableCHAP));
				settingsMap["refuse-mschap"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::DisableMSCHAP));
				settingsMap["refuse-mschapv2"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::DisableMSCHAPv2));
				settingsMap["nobsdcomp"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->pppConfig.flags & NetworkPPPFlags::AllowBSDCompression)));
				settingsMap["nodeflate"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->pppConfig.flags & NetworkPPPFlags::AllowDeflateCompression)));
				settingsMap["no-vj-comp"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->pppConfig.flags & NetworkPPPFlags::AllowVJCompression)));
				settingsMap["require-mppe"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::RequireMPPE));
				settingsMap["require-mppe-128"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::RequireMPPE128));
				settingsMap["mppe-stateful"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::StatefulMPPE));
				settingsMap["crtscts"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & NetworkPPPFlags::UseHardwareFlowControl));
				settingsMap["baud"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->pppConfig.baudRate));
				if (connection->pppConfig.mru > 0) {
					settingsMap["mru"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->pppConfig.mru));
				}
				else {
					settingsMap.remove("mru");
				}
				if (connection->pppConfig.mtu > 0) {
					settingsMap["mtu"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->pppConfig.mtu));
				}
				else {
					settingsMap.remove("mtu");
				}
				if (connection->pppConfig.mtu > 0) {
					settingsMap["lcp-echo-interval"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->pppConfig.lcpEchoPingInterval));
				}
				else {
					settingsMap.remove("lcp-echo-interval");
				}
				if (connection->pppConfig.mtu > 0) {
					settingsMap["lcp-echo-failure"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(connection->pppConfig.lcpEchoFailureThreshold));
				}
				else {
					settingsMap.remove("lcp-echo-failure");
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("ppp", dbusData, TRUE); else outerMap.remove("ppp");

		groupValid = false;
		dbusData = outerMap["pppoe"];
		if (connection->pppoeConfig.valid) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				UPDATE_STRING_SETTING_IF_VALID(connection->pppoeConfig.networkServiceProvider, "service", settingsMap)
				UPDATE_STRING_SETTING_IF_VALID(connection->pppoeConfig.username, "username", settingsMap)
				if (connection->pppoeConfig.secretsValid) {
					UPDATE_STRING_SETTING_IF_VALID(connection->pppoeConfig.password, "password", settingsMap)
				}
				settingsMap["password-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(connection->pppoeConfig.passwordFlags)));
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("pppoe", dbusData, TRUE); else outerMap.remove("pppoe");

		groupValid = false;
		dbusData = outerMap["802-11-olpc-mesh"];
		if (olpcMeshConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				{
					unsigned int i;
					TQT_DBusDataValueList valueList;
					for (i=0; i<olpcMeshConnection->SSID.count(); i++) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(olpcMeshConnection->SSID[i]);
						valueList.append(innerDataValue);
					}
					settingsMap["ssid"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				settingsMap["channel"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(olpcMeshConnection->channel));
				{
					unsigned int i;
					TQT_DBusDataValueList valueList;
					for (i=0; i<olpcMeshConnection->anycastDHCPHWAddress.count(); i++) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(olpcMeshConnection->anycastDHCPHWAddress[i]);
						valueList.append(innerDataValue);
					}
					settingsMap["dhcp-anycast-address"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("802-11-olpc-mesh", dbusData, TRUE); else outerMap.remove("802-11-olpc-mesh");

		groupValid = false;
		dbusData = outerMap["bluetooth"];
		if (bluetoothConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				if (connection->lockedHWAddress.isValid()) {
					NetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					NetworkByteList::iterator it;
					for (it = address.begin(); it != address.end(); ++it) {
						TQT_DBusData innerDataValue = TQT_DBusData::fromByte(*it);
						valueList.append(innerDataValue);
					}
					TQT_DBusData nmHWAddress = TQT_DBusData::fromTQValueList(valueList);
					settingsMap["bdaddr"] = convertDBUSDataToVariantData(nmHWAddress);
				}
				else {
					settingsMap.remove("bdaddr");
				}
				UPDATE_STRING_SETTING_IF_VALID(tdeBluetoothModeToNMBluetoothMode(bluetoothConnection->type), "type", settingsMap)
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("bluetooth", dbusData, TRUE); else outerMap.remove("bluetooth");

		groupValid = false;
		dbusData = outerMap["cdma"];
		if ((modemConnection) && (modemConnection->type == ModemConnectionType::CDMA)) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				UPDATE_STRING_SETTING_IF_VALID(modemConnection->cdmaConfig.providerDataNumber, "number", settingsMap)
				UPDATE_STRING_SETTING_IF_VALID(modemConnection->cdmaConfig.username, "username", settingsMap)
				if (connection->pppoeConfig.secretsValid) {
					UPDATE_STRING_SETTING_IF_VALID(modemConnection->cdmaConfig.password, "password", settingsMap)
				}
				settingsMap["password-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(modemConnection->cdmaConfig.passwordFlags)));
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("cdma", dbusData, TRUE); else outerMap.remove("cdma");

		groupValid = false;
		dbusData = outerMap["gsm"];
		if ((modemConnection) && (modemConnection->type == ModemConnectionType::GSM)) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				UPDATE_STRING_SETTING_IF_VALID(modemConnection->gsmConfig.providerDataNumber, "number", settingsMap)
				UPDATE_STRING_SETTING_IF_VALID(modemConnection->gsmConfig.username, "username", settingsMap)
				if (connection->pppoeConfig.secretsValid) {
					UPDATE_STRING_SETTING_IF_VALID(modemConnection->gsmConfig.password, "password", settingsMap)
				}
				settingsMap["password-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(modemConnection->gsmConfig.passwordFlags)));
				UPDATE_STRING_SETTING_IF_VALID(modemConnection->gsmConfig.accessPointName, "apn", settingsMap)
				UPDATE_STRING_SETTING_IF_VALID(modemConnection->gsmConfig.networkID, "network-id", settingsMap)
				settingsMap["network-type"] = convertDBUSDataToVariantData(TQT_DBusData::fromInt32(tdeGSMModeToNMGSMMode(modemConnection->gsmConfig.networkType)));
				if (connection->pppoeConfig.secretsValid) {
					UPDATE_STRING_SETTING_IF_VALID(modemConnection->gsmConfig.pin, "pin", settingsMap)
				}
				settingsMap["pin-flags"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(tdePasswordFlagsToNMPasswordFlags(modemConnection->gsmConfig.pinFlags)));
				settingsMap["allowed-bands"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(modemConnection->gsmConfig.allowedFrequencyBands));
				settingsMap["home-only"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(modemConnection->gsmConfig.allowRoaming)));
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("gsm", dbusData, TRUE); else outerMap.remove("gsm");

		groupValid = false;
		dbusData = outerMap["ipv4"];
		{
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				{
					TQT_DBusDataValueList valueList;
					NetworkSingleIPConfigurationList::iterator it;
					for (it = connection->ipConfig.ipConfigurations.begin(); it != connection->ipConfig.ipConfigurations.end(); ++it) {
						if ((*it).isIPv4()) {
							TQT_DBusDataValueList innerValueList;
							// WARNING: The exact order of the data in the list is critical, therefore extreme caution should be exercised when modifying the code below!
							// Address
							innerValueList.append(TQT_DBusData::fromUInt32(reverseIPV4ByteOrder((*it).ipAddress.toIPv4Address())));
							// Netmask
							innerValueList.append(TQT_DBusData::fromUInt32((*it).networkMask.toCIDRMask()));
							// Gateway
							innerValueList.append(TQT_DBusData::fromUInt32(reverseIPV4ByteOrder((*it).gateway.toIPv4Address())));
							valueList.append(TQT_DBusData::fromTQValueList(innerValueList));
						}
					}
					if (valueList.count() <= 0) {
						// Create an empty list with the correct DBUS type structure
						TQT_DBusData valueList;
						valueList = TQT_DBusData::fromList(TQT_DBusDataList(TQT_DBusData::UInt32));
						settingsMap["addresses"] = convertDBUSDataToVariantData(TQT_DBusData::fromList(TQT_DBusDataList(valueList)));
					}
					else {
						settingsMap["addresses"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					}
				}
				{
					if (!connection->ipConfig.dhcpClientIdentifier.isNull()) {
						settingsMap["dhcp-client-id"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(connection->ipConfig.dhcpClientIdentifier));
					}
				}
				{
					TQT_DBusDataValueList valueList;
					NetworkAddressList::iterator it;
					for (it = connection->ipConfig.resolvers.begin(); it != connection->ipConfig.resolvers.end(); ++it) {
						if ((*it).isIPv4Address()) {
							valueList.append(TQT_DBusData::fromUInt32(reverseIPV4ByteOrder((*it).toIPv4Address())));
						}
					}
					if (valueList.count() <= 0) {
						// Create an empty list with the correct DBUS type structure
						settingsMap["dns"] = convertDBUSDataToVariantData(TQT_DBusData::fromList(TQT_DBusDataList(TQT_DBusData::UInt32)));
					}
					else {
						settingsMap["dns"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					}
				}
				{
					TQT_DBusDataValueList valueList;
					NetworkSearchDomainList::iterator it;
					for (it = connection->ipConfig.searchDomains.begin(); it != connection->ipConfig.searchDomains.end(); ++it) {
						if ((*it).isIPv4()) {
							valueList.append(TQT_DBusData::fromString((*it).searchDomain()));
						}
					}
					if (valueList.count() > 0) settingsMap["dns-search"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					settingsMap["ignore-auto-dns"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4DHCPDNS)));
				}
				{
					settingsMap["may-fail"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!connection->requireIPV4));
				}
				{
					TQString method;
					if (connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4DHCPIP) {
						method = "auto";
					}
					else if (connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4LocalOnly) {
						method = "link-local";
					}
					else if (connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4StartConnectionSharingServer) {
						method = "shared";
					}
					else if (connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4Disabled) {
						method = "disabled";
					}
					else {
						method = "manual";
					}
					if (!method.isNull())
						settingsMap["method"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(method));
				}
				{
					settingsMap["ignore-auto-routes"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4DHCPRoutes)));
				}
				{
					settingsMap["never-default"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute)));
				}
				{
					TQT_DBusDataValueList valueList;
					NetworkSingleRouteConfigurationList::iterator it;
					for (it = connection->ipConfig.routeConfigurations.begin(); it != connection->ipConfig.routeConfigurations.end(); ++it) {
						if ((*it).isIPv4()) {
							TQT_DBusDataValueList innerValueList;
							// WARNING: The exact order of the data in the list is critical, therefore extreme caution should be exercised when modifying the code below!
							// Address
							innerValueList.append(TQT_DBusData::fromUInt32(reverseIPV4ByteOrder((*it).ipAddress.toIPv4Address())));
							// Netmask
							innerValueList.append(TQT_DBusData::fromUInt32((*it).networkMask.toCIDRMask()));
							// Gateway
							innerValueList.append(TQT_DBusData::fromUInt32(reverseIPV4ByteOrder((*it).gateway.toIPv4Address())));
							// Metric
							innerValueList.append(TQT_DBusData::fromUInt32((*it).metric));
							valueList.append(TQT_DBusData::fromTQValueList(innerValueList));
						}
					}
					if (valueList.count() <= 0) {
						// Create an empty list with the correct DBUS type structure
						TQT_DBusData valueList;
						valueList = TQT_DBusData::fromList(TQT_DBusDataList(TQT_DBusData::UInt32));
						settingsMap["routes"] = convertDBUSDataToVariantData(TQT_DBusData::fromList(TQT_DBusDataList(valueList)));
					}
					else {
						settingsMap["routes"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					}
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("ipv4", dbusData, TRUE); else outerMap.remove("ipv4");

		groupValid = false;
		dbusData = outerMap["ipv6"];
		{
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				{
					TQT_DBusDataValueList valueList;
					NetworkSingleIPConfigurationList::iterator it;
					for (it = connection->ipConfig.ipConfigurations.begin(); it != connection->ipConfig.ipConfigurations.end(); ++it) {
						if ((*it).isIPv6()) {
							int i;
							Q_IPV6ADDR v6address;
							TQT_DBusDataValueList innerValueList;
							TQT_DBusDataValueList innerMostValueList;
							// WARNING: The exact order of the data in the list is critical, therefore extreme caution should be exercised when modifying the code below!
							// Address
							innerMostValueList.clear();
							v6address = (*it).ipAddress.toIPv6Address();
							for (i=0; i<16; i++) {
								innerMostValueList.append(TQT_DBusData::fromByte(v6address.c[i]));
							}
							innerValueList.append(TQT_DBusData::fromTQValueList(innerMostValueList));
							// Netmask
							innerValueList.append(TQT_DBusData::fromUInt32((*it).networkMask.toCIDRMask()));
							// Gateway
							innerMostValueList.clear();
							v6address = (*it).gateway.toIPv6Address();
							for (i=0; i<16; i++) {
								innerMostValueList.append(TQT_DBusData::fromByte(v6address.c[i]));
							}
							innerValueList.append(TQT_DBusData::fromTQValueList(innerMostValueList));
							valueList.append(TQT_DBusData::fromStruct(innerValueList));
						}
					}
					if (valueList.count() <= 0) {
						// Create an empty list with the correct DBUS type structure
						TQT_DBusDataValueList innerValueList;
						TQT_DBusDataValueList innerMostValueList;
						// WARNING: The exact order of the data in the list is critical, therefore extreme caution should be exercised when modifying the code below!
						// Address
						innerMostValueList.clear();
						innerMostValueList.append(TQT_DBusData::fromByte(0));
						innerValueList.append(TQT_DBusData::fromTQValueList(innerMostValueList));
						// Netmask
						innerValueList.append(TQT_DBusData::fromUInt32(0));
						// Gateway
						innerMostValueList.clear();
						innerMostValueList.append(TQT_DBusData::fromByte(0));
						innerValueList.append(TQT_DBusData::fromTQValueList(innerMostValueList));
						settingsMap["addresses"] = convertDBUSDataToVariantData(TQT_DBusData::fromList(TQT_DBusDataList(TQT_DBusData::fromStruct(innerValueList))));
					}
					else {
						settingsMap["addresses"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					}
				}
				{
					TQT_DBusDataValueList valueList;
					NetworkAddressList::iterator it;
					for (it = connection->ipConfig.resolvers.begin(); it != connection->ipConfig.resolvers.end(); ++it) {
						if ((*it).isIPv6Address()) {
							int i;
							Q_IPV6ADDR v6address;
							TQT_DBusDataValueList innerValueList;
							v6address = (*it).toIPv6Address();
							for (i=0; i<16; i++) {
								innerValueList.append(TQT_DBusData::fromByte(v6address.c[i]));
							}
							valueList.append(TQT_DBusData::fromTQValueList(innerValueList));
						}
					}
					if (valueList.count() <= 0) {
						// Create an empty list with the correct DBUS type structure
						TQT_DBusData valueList;
						valueList = TQT_DBusData::fromList(TQT_DBusDataList(TQT_DBusData::Byte));
						settingsMap["dns"] = convertDBUSDataToVariantData(TQT_DBusData::fromList(TQT_DBusDataList(valueList)));
					}
					else {
						settingsMap["dns"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					}
				}
				{
					TQT_DBusDataValueList valueList;
					NetworkSearchDomainList::iterator it;
					for (it = connection->ipConfig.searchDomains.begin(); it != connection->ipConfig.searchDomains.end(); ++it) {
						if ((*it).isIPv6()) {
							valueList.append(TQT_DBusData::fromString((*it).searchDomain()));
						}
					}
					if (valueList.count() > 0) settingsMap["dns-search"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					settingsMap["ignore-auto-dns"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4DHCPDNS)));
				}
				{
					settingsMap["may-fail"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!connection->requireIPV6));
				}
				{
					TQString method;
					if (connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV6DHCPIP) {
						method = "auto";
					}
					else if (connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV6LocalOnly) {
						method = "link-local";
					}
					else if (connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV6StartConnectionSharingServer) {
						method = "shared";
					}
					else if (connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV6Disabled) {
						method = "ignore";
					}
					else {
						method = "manual";
					}
					if (!method.isNull())
						settingsMap["method"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(method));
				}
				{
					settingsMap["ignore-auto-routes"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV6DHCPRoutes)));
				}
				{
					settingsMap["never-default"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute)));
				}
				{
					TQT_DBusDataValueList valueList;
					NetworkSingleRouteConfigurationList::iterator it;
					for (it = connection->ipConfig.routeConfigurations.begin(); it != connection->ipConfig.routeConfigurations.end(); ++it) {
						if ((*it).isIPv6()) {
							int i;
							Q_IPV6ADDR v6address;
							TQT_DBusDataValueList innerValueList;
							TQT_DBusDataValueList innerMostValueList;
							// WARNING: The exact order of the data in the list is critical, therefore extreme caution should be exercised when modifying the code below!
							// Address
							innerMostValueList.clear();
							v6address = (*it).ipAddress.toIPv6Address();
							for (i=0; i<16; i++) {
								innerMostValueList.append(TQT_DBusData::fromByte(v6address.c[i]));
							}
							innerValueList.append(TQT_DBusData::fromTQValueList(innerMostValueList));
							// Netmask
							innerValueList.append(TQT_DBusData::fromUInt32((*it).networkMask.toCIDRMask()));
							// Gateway
							innerMostValueList.clear();
							v6address = (*it).gateway.toIPv6Address();
							for (i=0; i<16; i++) {
								innerMostValueList.append(TQT_DBusData::fromByte(v6address.c[i]));
							}
							innerValueList.append(TQT_DBusData::fromTQValueList(innerMostValueList));
							// Metric
							innerValueList.append(TQT_DBusData::fromUInt32((*it).metric));
							valueList.append(TQT_DBusData::fromStruct(innerValueList));
						}
					}
					if (valueList.count() <= 0) {
						// Create an empty list with the correct DBUS type structure
						TQT_DBusDataValueList innerValueList;
						TQT_DBusDataValueList innerMostValueList;
						// WARNING: The exact order of the data in the list is critical, therefore extreme caution should be exercised when modifying the code below!
						// Address
						innerMostValueList.clear();
						innerMostValueList.append(TQT_DBusData::fromByte(0));
						innerValueList.append(TQT_DBusData::fromTQValueList(innerMostValueList));
						// Netmask
						innerValueList.append(TQT_DBusData::fromUInt32(0));
						// Gateway
						innerMostValueList.clear();
						innerMostValueList.append(TQT_DBusData::fromByte(0));
						innerValueList.append(TQT_DBusData::fromTQValueList(innerMostValueList));
						// Metric
						innerValueList.append(TQT_DBusData::fromUInt32(0));
						settingsMap["routes"] = convertDBUSDataToVariantData(TQT_DBusData::fromList(TQT_DBusDataList(TQT_DBusData::fromStruct(innerValueList))));
					}
					else {
						settingsMap["routes"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					}
				}
			}
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("ipv6", dbusData, TRUE); else outerMap.remove("ipv6");
	}
	connectionSettingsMap = TQT_DBusDataMap<TQString>(outerMap);

	// If existing==true, a connection already existed and simply needs to be updated
	// If existing==false, a new connection must be created
	// To update: Use 'DBus::ConnectionSettingsInterface' with the connection path 'existingConnection' and call 'virtual bool UpdateAsync(int& asyncCallId, const TQT_DBusDataMap<TQString>& properties, TQT_DBusError& error);'
	// To create new: Use 'd->m_networkManagerSettings' and call 'virtual bool AddConnectionAsync(int& asyncCallId, const TQT_DBusDataMap<TQString>& connection, TQT_DBusError& error);'

#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
	printf("[network-manager comm debug] uploaded DBUS object structure map follows:\n"); fflush(stdout);
	printDBUSObjectStructure(TQT_DBusData::fromStringKeyMap(connectionSettingsMap));
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS

	if (existing) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
		printf("[network-manager comm debug] Updating existing connection\n"); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
		// Save connection settings to the path specified
		DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
		connectionSettings.setConnection(TQT_DBusConnection::systemBus());
		connect(&connectionSettings, SIGNAL(UpdateAsyncReply(int)), d, SLOT(processConnectionSettingsUpdateAsyncReply(int)));
		int asyncCallID;
		ret = connectionSettings.UpdateAsync(asyncCallID, connectionSettingsMap, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR((error.name() + ": " + error.message()))
		}
		if (ret) {
			// Wait for the asynchronous call to return...
			d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
			TQTimer nmCallTimeoutTimer;
			nmCallTimeoutTimer.start(NM_ASYNC_TIMEOUT_MS, TRUE);
			while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
				tqApp->processEvents();
				if (!nmCallTimeoutTimer.isActive()) {
					PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
					timed_out = true;
					break;
				}
			}
			d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
			return !timed_out;
		}
		else {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return FALSE;
		}
	}
	else {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
		printf("[network-manager comm debug] Creating new connection\n"); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
		// Create new connection
		connect(d->m_networkManagerSettings, SIGNAL(AddConnectionAsyncReply(int, const TQT_DBusObjectPath&)), d, SLOT(processAddConnectionAsyncReply(int, const TQT_DBusObjectPath&)));
		connect(d->m_networkManagerSettings, SIGNAL(AsyncErrorResponseDetected(int, const TQT_DBusError)), d, SLOT(processAddConnectionAsyncError(int, const TQT_DBusError)));
		int asyncCallID;
		ret = d->m_networkManagerSettings->AddConnectionAsync(asyncCallID, connectionSettingsMap, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR((error.name() + ": " + error.message()))
		}
		if (ret) {
			// Wait for the asynchronous call to return...
			d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
			TQTimer nmCallTimeoutTimer;
			nmCallTimeoutTimer.start(NM_ASYNC_TIMEOUT_MS, TRUE);
			while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
				if (!nmCallTimeoutTimer.isActive()) {
					PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
					timed_out = true;
					break;
				}
				tqApp->processEvents();
			}
			if (d->nmAddConnectionAsyncErrorResponse.contains(asyncCallID)) {
				PRINT_ERROR((d->nmAddConnectionAsyncErrorResponse[asyncCallID].name() + ": " + d->nmAddConnectionAsyncErrorResponse[asyncCallID].message()));
				d->nmAddConnectionAsyncErrorResponse.remove(asyncCallID);
			}
			if (!d->nmAddConnectionAsyncResponse[asyncCallID].data()) {
				PRINT_ERROR(TQString("NetworkManager did not return a new connection object!"))
				command_failed = true;
			}
			d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
			if (d->nmAddConnectionAsyncResponse.contains(asyncCallID)) {
				d->nmAddConnectionAsyncResponse.remove(asyncCallID);
			}
			return ((!timed_out) && (!command_failed));
		}
		else {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return FALSE;
		}
	}
}

bool NetworkConnectionManager_BackendNM::deleteConnection(TQString uuid) {
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool timed_out = false;
	bool ret;
	if (d->m_networkManagerSettings) {
		ret = d->m_networkManagerSettings->GetConnectionByUuid(uuid, existingConnection, error);
		if (ret) {
			// Obtain connection secrets from the path specified
			DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
			connectionSettings.setConnection(TQT_DBusConnection::systemBus());
			connect(&connectionSettings, SIGNAL(DeleteAsyncReply(int)), d, SLOT(processConnectionSettingsUpdateAsyncReply(int)));
			int asyncCallID;
			ret = connectionSettings.DeleteAsync(asyncCallID, error);
			if (ret && error.isValid()) {
				ret = 0;
				PRINT_ERROR((error.name() + ": " + error.message()))
			}
			if (ret) {
				// Wait for the asynchronous call to return...
				d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
				TQTimer nmCallTimeoutTimer;
				nmCallTimeoutTimer.start(NM_ASYNC_TIMEOUT_MS, TRUE);
				while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
					tqApp->processEvents();
					if (!nmCallTimeoutTimer.isActive()) {
						PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
						timed_out = true;
						break;
					}
				}
				d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
				return !timed_out;
			}
			else {
				PRINT_ERROR(TQString("Unable to remove connection with uuid '%1'").arg(uuid))
				return FALSE;
			}
		}
		else {
			PRINT_WARNING(TQString("connection for provided uuid '%1' was not found").arg(uuid));
			return FALSE;
		}
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object"));
		return FALSE;
	}
}

bool NetworkConnectionManager_BackendNM::verifyConnectionSettings(NetworkConnection* connection, NetworkConnectionErrorFlags::NetworkConnectionErrorFlags* type, NetworkErrorStringMap* reason) {
	// FIXME
	// This should actually attempt to validate all the settings!

	if (!connection) {
		return false;
	}

	if (connection->friendlyName == "") {
		if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidConnectionSetting] = i18n("Connection name is invalid");
		if (type) *type |= NetworkConnectionErrorFlags::InvalidConnectionSetting;
		return false;
	}

	if (connection->ipConfig.valid) {
		// Iterate over all addresses
		NetworkSingleIPConfigurationList::iterator it;
		for (it = connection->ipConfig.ipConfigurations.begin(); it != connection->ipConfig.ipConfigurations.end(); ++it) {
			if ((*it).isIPv4()) {
				if (!(connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV4DHCPIP)) {
					if (!NetworkConnectionManager::validateIPAddress((*it).ipAddress)) {
						if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidIPv4Setting] = i18n("IPv4 address is invalid");
						if (type) *type |= NetworkConnectionErrorFlags::InvalidIPv4Setting;
						return false;
					}
				}
			}
			else if ((*it).isIPv6()) {
				if (!(connection->ipConfig.connectionFlags & NetworkIPConfigurationFlags::IPV6DHCPIP)) {
					if (!NetworkConnectionManager::validateIPAddress((*it).ipAddress)) {
						if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidIPv6Setting] = i18n("IPv6 address is invalid");
						if (type) *type |= NetworkConnectionErrorFlags::InvalidIPv6Setting;
						return false;
					}
				}
			}
		}
	}

	WiFiConnection* wiFiConnection = dynamic_cast<WiFiConnection*>(connection);
	if (wiFiConnection) {
		if (wiFiConnection->SSID.count() < 1) {
			if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessSetting] = i18n("No SSID provided");
			if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessSetting;
			return false;
		}
		if (wiFiConnection->securityRequired) {
			if (wiFiConnection->securitySettings.secretsValid) {
				if ((wiFiConnection->securitySettings.keyType == NetworkWiFiKeyType::WEP) || ((wiFiConnection->securitySettings.keyType == NetworkWiFiKeyType::DynamicWEP) && ((wiFiConnection->securitySettings.authType == NetworkWiFiAuthType::Open) || (wiFiConnection->securitySettings.authType == NetworkWiFiAuthType::Shared)))) {
					if (wiFiConnection->securitySettings.wepKeyType == NetworkWepKeyType::Hexadecimal) {
						if (wiFiConnection->securitySettings.wepKey0 != "") {
							if ((wiFiConnection->securitySettings.wepKey0.length() != 10) && (wiFiConnection->securitySettings.wepKey0.length() != 26)) {
								if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("WEP key 0 has invalid length");
								if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
								return false;
							}
						}
						if (wiFiConnection->securitySettings.wepKey1 != "") {
							if ((wiFiConnection->securitySettings.wepKey1.length() != 10) && (wiFiConnection->securitySettings.wepKey1.length() != 26)) {
								if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("WEP key 1 has invalid length");
								if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
								return false;
							}
						}
						if (wiFiConnection->securitySettings.wepKey2 != "") {
							if ((wiFiConnection->securitySettings.wepKey2.length() != 10) && (wiFiConnection->securitySettings.wepKey2.length() != 26)) {
								if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("WEP key 2 has invalid length");
								if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
								return false;
							}
						}
						if (wiFiConnection->securitySettings.wepKey3 != "") {
							if ((wiFiConnection->securitySettings.wepKey3.length() != 10) && (wiFiConnection->securitySettings.wepKey3.length() != 26)) {
								if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("WEP key 3 has invalid length");
								if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
								return false;
							}
						}
						if ((wiFiConnection->securitySettings.wepKey0 == "") && (wiFiConnection->securitySettings.wepKey1 == "") && (wiFiConnection->securitySettings.wepKey2 == "") && (wiFiConnection->securitySettings.wepKey3 == "")) {
							if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("No WEP key(s) provided");
							if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
							return false;
						}
					}
					else if (wiFiConnection->securitySettings.wepKeyType == NetworkWepKeyType::Ascii) {
						if (wiFiConnection->securitySettings.wepKey0 != "") {
							if ((wiFiConnection->securitySettings.wepKey0.length() != 5) && (wiFiConnection->securitySettings.wepKey0.length() != 13)) {
								if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("WEP key 0 has invalid length");
								if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
								return false;
							}
						}
						if (wiFiConnection->securitySettings.wepKey1 != "") {
							if ((wiFiConnection->securitySettings.wepKey1.length() != 5) && (wiFiConnection->securitySettings.wepKey1.length() != 13)) {
								if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("WEP key 1 has invalid length");
								if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
								return false;
							}
						}
						if (wiFiConnection->securitySettings.wepKey2 != "") {
							if ((wiFiConnection->securitySettings.wepKey2.length() != 5) && (wiFiConnection->securitySettings.wepKey2.length() != 13)) {
								if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("WEP key 2 has invalid length");
								if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
								return false;
							}
						}
						if (wiFiConnection->securitySettings.wepKey3 != "") {
							if ((wiFiConnection->securitySettings.wepKey3.length() != 5) && (wiFiConnection->securitySettings.wepKey3.length() != 13)) {
								if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("WEP key 3 has invalid length");
								if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
								return false;
							}
						}
						if ((wiFiConnection->securitySettings.wepKey0 == "") && (wiFiConnection->securitySettings.wepKey1 == "") && (wiFiConnection->securitySettings.wepKey2 == "") && (wiFiConnection->securitySettings.wepKey3 == "")) {
							if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("No WEP key(s) provided");
							if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
							return false;
						}
					}
					else if (wiFiConnection->securitySettings.wepKeyType == NetworkWepKeyType::Ascii) {
						if ((wiFiConnection->securitySettings.wepKey0 == "") && (wiFiConnection->securitySettings.wepKey1 == "") && (wiFiConnection->securitySettings.wepKey2 == "") && (wiFiConnection->securitySettings.wepKey3 == "")) {
							if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("No WEP key(s) provided");
							if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
							return false;
						}
					}
				}
				else if ((wiFiConnection->securitySettings.keyType == NetworkWiFiKeyType::DynamicWEP) && (wiFiConnection->securitySettings.authType == NetworkWiFiAuthType::LEAP)) {
					if ((wiFiConnection->securitySettings.leapUsername.length() < 1) || (wiFiConnection->securitySettings.leapPassword.length() < 1)) {
						if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("LEAP username and/or password not provided");
						if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
						return false;
					}
				}
				else if ((wiFiConnection->securitySettings.keyType == NetworkWiFiKeyType::WPAAdHoc) || (wiFiConnection->securitySettings.keyType == NetworkWiFiKeyType::WPAInfrastructure) || (wiFiConnection->securitySettings.keyType == NetworkWiFiKeyType::WPAEnterprise)) {
					if (wiFiConnection->securitySettings.psk.length() == 64) {
						// Verify that only hex characters are present in the string
						bool ok;
						wiFiConnection->securitySettings.psk.toULongLong(&ok, 16);
						if (!ok) {
							if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("Hexadecimal length PSK contains non-hexadecimal characters");
							if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
							return false;
						}
					}
					else if ((wiFiConnection->securitySettings.psk.length() < 8) || (wiFiConnection->securitySettings.psk.length() > 63)) {
						if (reason) (*reason)[NetworkConnectionErrorFlags::InvalidWirelessKey] = i18n("No PSK provided");
						if (type) *type |= NetworkConnectionErrorFlags::InvalidWirelessKey;
						return false;
					}
				}
			}
		}
	}

	return TRUE;
}

NetworkConnectionStatus::NetworkConnectionStatus NetworkConnectionManager_BackendNM::initiateConnection(TQString uuid) {
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool ret;
	if ((d->m_networkManagerSettings) && (d->m_networkManagerProxy)) {
		ret = d->m_networkManagerSettings->GetConnectionByUuid(uuid, existingConnection, error);
		if (ret) {
			if (m_macAddress == "") {
				d->m_dbusDeviceString = "/";
			}
			else {
				d->m_dbusDeviceString = deviceInterfaceString(m_macAddress);
			}
#ifndef USE_ASYNC_DBUS_CONNECTION_COMMAND_CALLS
			TQT_DBusObjectPath active_connection;
			ret = d->m_networkManagerProxy->ActivateConnection(existingConnection, TQT_DBusObjectPath(d->m_dbusDeviceString.ascii()), TQT_DBusObjectPath("/"), active_connection, error);
			if (ret && error.isValid()) {
				ret = 0;
				PRINT_ERROR((error.name() + ": " + error.message()))
			}
			return checkConnectionStatus(uuid);
#else // USE_ASYNC_DBUS_CONNECTION_COMMAND_CALLS
#ifdef WAIT_FOR_OPERATION_BEFORE_RETURNING
			connect(d->m_networkManagerProxy, SIGNAL(ActivateConnectionAsyncReply(int, const TQT_DBusObjectPath&)), d, SLOT(processAddConnectionAsyncReply(int, const TQT_DBusObjectPath&)));
			connect(d->m_networkManagerProxy, SIGNAL(AsyncErrorResponseDetected(int, const TQT_DBusError)), d, SLOT(processAddConnectionAsyncError(int, const TQT_DBusError)));
#endif // WAIT_FOR_OPERATION_BEFORE_RETURNING
			int asyncCallID;
			ret = d->m_networkManagerProxy->ActivateConnectionAsync(asyncCallID, existingConnection, TQT_DBusObjectPath(d->m_dbusDeviceString.ascii()), TQT_DBusObjectPath("/"), error);
			if (ret && error.isValid()) {
				ret = 0;
				PRINT_ERROR((error.name() + ": " + error.message()))
			}
#ifdef WAIT_FOR_OPERATION_BEFORE_RETURNING
			if (ret) {
				// Wait for the asynchronous call to return...
				d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
				TQTimer nmCallTimeoutTimer;
				nmCallTimeoutTimer.start(NM_ASYNC_TIMEOUT_MS, TRUE);
				while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
					if (!nmCallTimeoutTimer.isActive()) {
						PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
						break;
					}
					tqApp->processEvents();
				}
				d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
				if (d->nmAddConnectionAsyncErrorResponse.contains(asyncCallID)) {
					PRINT_ERROR((d->nmAddConnectionAsyncErrorResponse[asyncCallID].name() + ": " + d->nmAddConnectionAsyncErrorResponse[asyncCallID].message()));
					d->nmAddConnectionAsyncErrorResponse.remove(asyncCallID);
				}
				if (!d->nmAddConnectionAsyncResponse[asyncCallID].data()) {
					PRINT_ERROR(TQString("NetworkManager did not return a new connection object!"))
				}
				if (d->nmAddConnectionAsyncResponse.contains(asyncCallID)) {
					d->nmAddConnectionAsyncResponse.remove(asyncCallID);
				}
				return checkConnectionStatus(uuid);
			}
			else {
				// Error!
				PRINT_ERROR((error.name() + ": " + error.message()))
				return checkConnectionStatus(uuid);
			}
#else
			return checkConnectionStatus(uuid);
#endif
#endif // USE_ASYNC_DBUS_CONNECTION_COMMAND_CALLS
		}
		else {
			PRINT_WARNING(TQString("connection for provided uuid '%1' was not found").arg(uuid));
			return NetworkConnectionStatus::Invalid;
		}
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object"));
		return NetworkConnectionStatus::Invalid;
	}
}

NetworkConnectionStatus::NetworkConnectionStatus NetworkConnectionManager_BackendNM::checkConnectionStatus(TQString uuid) {
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	if (d->m_networkManagerProxy) {
		TQT_DBusObjectPathList activeConnections = d->m_networkManagerProxy->getActiveConnections(error);
		TQT_DBusObjectPathList::iterator it;
		for (it = activeConnections.begin(); it != activeConnections.end(); ++it) {
			DBus::ActiveConnectionProxy activeConnection(NM_DBUS_SERVICE, (*it));
			activeConnection.setConnection(TQT_DBusConnection::systemBus());
			if (activeConnection.getUuid(error) == uuid) {
				return nmDeviceStateToTDEDeviceState(activeConnection.getState(error));
			}
		}
		PRINT_WARNING(TQString("active connection for provided uuid '%1' was not found").arg(uuid));
		return NetworkConnectionStatus::Invalid;
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object"));
		return NetworkConnectionStatus::Invalid;
	}
}

TQCString NetworkConnectionManager_BackendNM::getActiveConnectionPath(TQString uuid) {
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	if (d->m_networkManagerProxy) {
		TQT_DBusObjectPathList activeConnections = d->m_networkManagerProxy->getActiveConnections(error);
		TQT_DBusObjectPathList::iterator it;
		for (it = activeConnections.begin(); it != activeConnections.end(); ++it) {
			DBus::ActiveConnectionProxy activeConnection(NM_DBUS_SERVICE, (*it));
			activeConnection.setConnection(TQT_DBusConnection::systemBus());
			if (activeConnection.getUuid(error) == uuid) {
				return (*it);
			}
		}
		PRINT_WARNING(TQString("active connection for provided uuid '%1' was not found").arg(uuid));
		return TQT_DBusObjectPath();
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object"));
		return TQT_DBusObjectPath();
	}
}

TQStringList NetworkConnectionManager_BackendNM::connectionPhysicalDeviceUUIDs(TQString uuid) {
	if (deviceType() == NetworkDeviceType::BackendOnly) {
		return TQStringList();
	}

	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	if (d->m_networkManagerProxy) {
		TQT_DBusObjectPathList activeConnections = d->m_networkManagerProxy->getActiveConnections(error);
		TQT_DBusObjectPathList::iterator it;
		TQStringList ret;
		for (it = activeConnections.begin(); it != activeConnections.end(); ++it) {
			DBus::ActiveConnectionProxy activeConnection(NM_DBUS_SERVICE, (*it));
			activeConnection.setConnection(TQT_DBusConnection::systemBus());
			if (activeConnection.getUuid(error) == uuid) {
				TQValueList<TQT_DBusObjectPath> deviceList = activeConnection.getDevices(error);
				TQT_DBusObjectPathList::iterator it2;
				for (it2 = deviceList.begin(); it2 != deviceList.end(); ++it2) {
					TQString macAddress = macAddressForGenericDevice(*it2);
					TQString devUUID = tdeDeviceUUIDForMACAddress(macAddress);
					if (devUUID != "") {
						ret.append(devUUID);
					}
				}
			}
		}
		return ret;
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object"));
		return TQStringList();
	}
}

NetworkVPNTypeList NetworkConnectionManager_BackendNM::availableVPNTypes() {
	NetworkVPNTypeList ret;

	// Query NetworkManager to verify plugin availability before claiming support for a VPN type
	TQDir serviceDir(NM_PLUGIN_SERVICE_DIR, TQString(), TQDir::Name|TQDir::IgnoreCase, TQDir::Files);
	TQStringList services = serviceDir.entryList ().grep (".name", true);

	if (services.count () > 0) {
		// read in all available Services
		for (TQStringList::Iterator i = services.begin (); i != services.end (); ++i) {
			TQString service = NM_PLUGIN_SERVICE_DIR + TQString ("/") + *i;
			TDEConfig* tdeconfig = new TDEConfig (service, true, true, "config");
			tdeconfig->setGroup ("VPN Connection");

			TQString serviceName = tdeconfig->readEntry("name", TQString());
			serviceName = serviceName.lower();

			if (serviceName == "openvpn") {
				ret.append(NetworkVPNType::OpenVPN);
			}
			if (serviceName == "pptp") {
				ret.append(NetworkVPNType::PPTP);
			}
			if (serviceName == "strongswan") {
				ret.append(NetworkVPNType::StrongSwan);
			}
			if (serviceName == "vpnc") {
				ret.append(NetworkVPNType::VPNC);
			}

			delete tdeconfig;
		}
	}

	return ret;
}

NetworkConnectionStatus::NetworkConnectionStatus NetworkConnectionManager_BackendNM::deactivateConnection(TQString uuid) {
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool ret;
	if ((d->m_networkManagerSettings) && (d->m_networkManagerProxy)) {
		existingConnection = getActiveConnectionPath(uuid);
		if (existingConnection.isValid()) {
			if (m_macAddress == "") {
				d->m_dbusDeviceString = "/";
			}
			else {
				d->m_dbusDeviceString = deviceInterfaceString(m_macAddress);
			}
#ifndef USE_ASYNC_DBUS_CONNECTION_COMMAND_CALLS
			ret = d->m_networkManagerProxy->DeactivateConnection(existingConnection, error);
			if (ret && error.isValid()) {
				ret = 0;
				PRINT_ERROR((error.name() + ": " + error.message()))
			}
			return checkConnectionStatus(uuid);
#else // USE_ASYNC_DBUS_CONNECTION_COMMAND_CALLS
#ifdef WAIT_FOR_OPERATION_BEFORE_RETURNING
			connect(d->m_networkManagerProxy, SIGNAL(DeactivateConnectionAsyncReply(int)), d, SLOT(processConnectionSettingsUpdateAsyncReply(int)));
#endif // WAIT_FOR_OPERATION_BEFORE_RETURNING
			int asyncCallID;
			ret = d->m_networkManagerProxy->DeactivateConnectionAsync(asyncCallID, existingConnection, error);
			if (ret && error.isValid()) {
				ret = 0;
				PRINT_ERROR((error.name() + ": " + error.message()))
			}
#ifdef WAIT_FOR_OPERATION_BEFORE_RETURNING
			if (ret) {
				// Wait for the asynchronous call to return...
				d->nmConnectionSettingsAsyncCallWaiting[asyncCallID] = true;
				TQTimer nmCallTimeoutTimer;
				nmCallTimeoutTimer.start(NM_ASYNC_TIMEOUT_MS, TRUE);
				while (d->nmConnectionSettingsAsyncCallWaiting[asyncCallID]) {
					if (!nmCallTimeoutTimer.isActive()) {
						PRINT_ERROR(TQString("DBUS asynchronous call timed out!"))
						break;
					}
					tqApp->processEvents();
				}
				d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
				return checkConnectionStatus(uuid);
			}
			else {
				// Error!
				PRINT_ERROR((error.name() + ": " + error.message()))
				return checkConnectionStatus(uuid);
			}
#else
			return checkConnectionStatus(uuid);
#endif
#endif // USE_ASYNC_DBUS_CONNECTION_COMMAND_CALLS
		}
		else {
			PRINT_WARNING(TQString("connection for provided uuid '%1' was not found").arg(uuid));
			return NetworkConnectionStatus::Invalid;
		}
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object"));
		return NetworkConnectionStatus::Invalid;
	}
}

TQStringList NetworkConnectionManager_BackendNM::validSettings() {
	TQStringList ret;

	ret.append("NetworkSingleIPConfiguration::ipAddress");
	ret.append("NetworkSingleIPConfiguration::networkMask");
	ret.append("NetworkSingleIPConfiguration::gateway");

	ret.append("NetworkSingleRouteConfiguration::ipAddress");
	ret.append("NetworkSingleRouteConfiguration::networkMask");
	ret.append("NetworkSingleRouteConfiguration::gateway");

	ret.append("NetworkIEEE8021xConfiguration::valid");
	ret.append("NetworkIEEE8021xConfiguration::allowedValid");
	ret.append("NetworkIEEE8021xConfiguration::secretsValid");
	ret.append("NetworkIEEE8021xConfiguration::type");
	ret.append("NetworkIEEE8021xConfiguration::userName");
	ret.append("NetworkIEEE8021xConfiguration::anonymousUserName");
	ret.append("NetworkIEEE8021xConfiguration::pacFileName");
	ret.append("NetworkIEEE8021xConfiguration::caCertificate");
	ret.append("NetworkIEEE8021xConfiguration::additionalCAFilesPath");
	ret.append("NetworkIEEE8021xConfiguration::authServerCertSubjectMatch");
	ret.append("NetworkIEEE8021xConfiguration::alternateAuthServerCertSubjectMatch");
	ret.append("NetworkIEEE8021xConfiguration::clientCertificate");
	ret.append("NetworkIEEE8021xConfiguration::forcePEAPVersion");
	ret.append("NetworkIEEE8021xConfiguration::forcePEAPLabel");
	ret.append("NetworkIEEE8021xConfiguration::fastProvisioningFlags");
	ret.append("NetworkIEEE8021xConfiguration::phase2NonEAPAuthMethod");
	ret.append("NetworkIEEE8021xConfiguration::phase2EAPAuthMethod");
	ret.append("NetworkIEEE8021xConfiguration::allowedPhase2NonEAPMethods");
	ret.append("NetworkIEEE8021xConfiguration::allowedPhase2EAPMethods");
	ret.append("NetworkIEEE8021xConfiguration::phase2CaCertificate");
	ret.append("NetworkIEEE8021xConfiguration::phase2CaFilesPath");
	ret.append("NetworkIEEE8021xConfiguration::phase2AuthServerCertSubjectMatch");
	ret.append("NetworkIEEE8021xConfiguration::phase2AlternateAuthServerCertSubjectMatch");
	ret.append("NetworkIEEE8021xConfiguration::phase2ClientCertificate");
	ret.append("NetworkIEEE8021xConfiguration::password");
	ret.append("NetworkIEEE8021xConfiguration::passwordFlags");
	ret.append("NetworkIEEE8021xConfiguration::binaryPassword");
	ret.append("NetworkIEEE8021xConfiguration::binaryPasswordFlags");
	ret.append("NetworkIEEE8021xConfiguration::privateKey");
	ret.append("NetworkIEEE8021xConfiguration::privateKeyPassword");
	ret.append("NetworkIEEE8021xConfiguration::privateKeyPasswordFlags");
	ret.append("NetworkIEEE8021xConfiguration::phase2PrivateKey");
	ret.append("NetworkIEEE8021xConfiguration::phase2PrivateKeyPassword");
	ret.append("NetworkIEEE8021xConfiguration::phase2PrivateKeyPasswordFlags");
	ret.append("NetworkIEEE8021xConfiguration::forceSystemCaCertificates");

	ret.append("NetworkPPPConfiguration::valid");
	ret.append("NetworkPPPConfiguration::requireServerAuthentication");
	ret.append("NetworkPPPConfiguration::flags");
	ret.append("NetworkPPPConfiguration::baudRate");
	ret.append("NetworkPPPConfiguration::mru");
	ret.append("NetworkPPPConfiguration::mtu");
	ret.append("NetworkPPPConfiguration::lcpEchoPingInterval");
	ret.append("NetworkPPPConfiguration::lcpEchoFailureThreshold");

	ret.append("NetworkPPPOEConfiguration::valid");
	ret.append("NetworkPPPOEConfiguration::secretsValid");
	ret.append("NetworkPPPOEConfiguration::networkServiceProvider");
	ret.append("NetworkPPPOEConfiguration::username");
	ret.append("NetworkPPPOEConfiguration::password");
	ret.append("NetworkPPPOEConfiguration::passwordFlags");

	ret.append("NetworkSerialConfiguration::valid");
	ret.append("NetworkSerialConfiguration::baudRate");
	ret.append("NetworkSerialConfiguration::byteWidth");
	ret.append("NetworkSerialConfiguration::parity");
	ret.append("NetworkSerialConfiguration::stopBits");
	ret.append("NetworkSerialConfiguration::txDelay");

	ret.append("NetworkCDMAConfiguration::valid");
	ret.append("NetworkCDMAConfiguration::secretsValid");
	ret.append("NetworkCDMAConfiguration::providerDataNumber");
	ret.append("NetworkCDMAConfiguration::username");
	ret.append("NetworkCDMAConfiguration::password");
	ret.append("NetworkCDMAConfiguration::passwordFlags");

	ret.append("NetworkGSMConfiguration::valid");
	ret.append("NetworkGSMConfiguration::secretsValid");
	ret.append("NetworkGSMConfiguration::providerDataNumber");
	ret.append("NetworkGSMConfiguration::username");
	ret.append("NetworkGSMConfiguration::password");
	ret.append("NetworkGSMConfiguration::passwordFlags");
	ret.append("NetworkGSMConfiguration::accessPointName");
	ret.append("NetworkGSMConfiguration::networkID");
	ret.append("NetworkGSMConfiguration::networkType");
	ret.append("NetworkGSMConfiguration::pin");
	ret.append("NetworkGSMConfiguration::pinFlags");
	ret.append("NetworkGSMConfiguration::allowedFrequencyBands");
	ret.append("NetworkGSMConfiguration::allowRoaming");

	ret.append("TDENetworkWiFiSecurityConfiguration::valid");
	ret.append("TDENetworkWiFiSecurityConfiguration::secretsValid");
	ret.append("TDENetworkWiFiSecurityConfiguration::keyType");
	ret.append("TDENetworkWiFiSecurityConfiguration::authType");
	ret.append("TDENetworkWiFiSecurityConfiguration::wpaVersion");
	ret.append("TDENetworkWiFiSecurityConfiguration::cipher");
	ret.append("TDENetworkWiFiSecurityConfiguration::wepKey0");
	ret.append("TDENetworkWiFiSecurityConfiguration::wepKey1");
	ret.append("TDENetworkWiFiSecurityConfiguration::wepKey2");
	ret.append("TDENetworkWiFiSecurityConfiguration::wepKey3");
	ret.append("TDENetworkWiFiSecurityConfiguration::wepKeyFlags");
	ret.append("TDENetworkWiFiSecurityConfiguration::wepKeyIndex");
	ret.append("TDENetworkWiFiSecurityConfiguration::wepKeyType");
	ret.append("TDENetworkWiFiSecurityConfiguration::allowedPairWiseCiphers");
	ret.append("TDENetworkWiFiSecurityConfiguration::allowedGroupWiseCiphers");
	ret.append("TDENetworkWiFiSecurityConfiguration::psk");
	ret.append("TDENetworkWiFiSecurityConfiguration::pskFlags");
	ret.append("TDENetworkWiFiSecurityConfiguration::leapUsername");
	ret.append("TDENetworkWiFiSecurityConfiguration::leapPassword");
	ret.append("TDENetworkWiFiSecurityConfiguration::leapPasswordFlags");

	ret.append("NetworkIPConfiguration::valid");
	ret.append("NetworkIPConfiguration::connectionFlags");
	ret.append("NetworkIPConfiguration::ipConfigurations");
	ret.append("NetworkIPConfiguration::routeConfigurations");
	ret.append("NetworkIPConfiguration::broadcast");
	ret.append("NetworkIPConfiguration::destination");
	ret.append("NetworkIPConfiguration::resolvers");
	ret.append("NetworkIPConfiguration::searchDomains");
	ret.append("NetworkIPConfiguration::dhcpClientIdentifier");

	ret.append("NetworkConnection::UUID");
	ret.append("NetworkConnection::friendlyName");
	ret.append("NetworkConnection::ipConfig");
	ret.append("NetworkConnection::lockedHWAddress");
	ret.append("NetworkConnection::manualHWAddress");
	ret.append("NetworkConnection::readOnly");
	ret.append("NetworkConnection::autoConnect");
	ret.append("NetworkConnection::fullDuplex");
	ret.append("NetworkConnection::requireIPV4");
	ret.append("NetworkConnection::requireIPV6");
	ret.append("NetworkConnection::mtu");
	ret.append("NetworkConnection::eapConfig");
	ret.append("NetworkConnection::pppConfig");
	ret.append("NetworkConnection::pppoeConfig");
	ret.append("NetworkConnection::serialConfig");
	ret.append("NetworkConnection::authorizedUsers");
	ret.append("NetworkConnection::masterConnectionUUID");
	ret.append("NetworkConnection::slaveType");
	ret.append("NetworkConnection::lastKnownConnection");

	ret.append("WiFiConnection::SSID");
	ret.append("WiFiConnection::operatingMode");
	ret.append("WiFiConnection::bandRestriction");
	ret.append("WiFiConnection::channelRestriction");
	ret.append("WiFiConnection::bitRateRestriction");
	ret.append("WiFiConnection::powerRestriction");
	ret.append("WiFiConnection::accessPointRestriction");
	ret.append("WiFiConnection::blacklistedBSSIDs");
	ret.append("WiFiConnection::heardBSSIDs");
	ret.append("WiFiConnection::isHiddenNetwork");
	ret.append("WiFiConnection::securityRequired");
	ret.append("WiFiConnection::securitySettings");

	ret.append("WiredInfinibandConnection::transportMode");

	ret.append("VPNConnection::vpnPluginID");
	ret.append("VPNConnection::lockedUserName");
	ret.append("VPNConnection::pluginData");
	ret.append("VPNConnection::secretsValid");
	ret.append("VPNConnection::pluginSecrets");

	ret.append("WiMaxConnection::networkServiceProvider");

	ret.append("VLANConnection::kernelName");
	ret.append("VLANConnection::parentConnectionUUID");
	ret.append("VLANConnection::vlanID");
	ret.append("VLANConnection::vlanFlags");
	ret.append("VLANConnection::ingressPriorityMap");
	ret.append("VLANConnection::egressPriorityMap");

	ret.append("OLPCMeshConnection::SSID");
	ret.append("OLPCMeshConnection::channel");
	ret.append("OLPCMeshConnection::anycastDHCPHWAddress");

	ret.append("BluetoothConnection::type");

	ret.append("ModemConnection::type");
	ret.append("ModemConnection::cdmaConfig");
	ret.append("ModemConnection::gsmConfig");

	ret.append("WiredInfinibandConnection::transportMode");

	return ret;
}

NetworkWiFiAPInfo* NetworkConnectionManager_BackendNM::getAccessPointDetails(TQString dbusPath) {
	if (dbusPath == "") {
		return NULL;
	}

	NetworkWiFiAPInfo* apInfo = new NetworkWiFiAPInfo;
	TQT_DBusError error;
	unsigned int index;
	DBus::AccessPointProxy accessPoint(NM_DBUS_SERVICE, dbusPath);
	accessPoint.setConnection(TQT_DBusConnection::systemBus());

	TQValueList<TQ_UINT8> nmSSID = accessPoint.getSsid(error);
	if (error.isValid()) {
		delete apInfo;
		return NULL;
	}
	TQValueList<TQ_UINT8>::iterator it;
	index = 0;
	for (it = nmSSID.begin(); it != nmSSID.end(); ++it) {
		apInfo->SSID.resize(index+1);
		apInfo->SSID[index] = (*it);
		index++;
	}

	apInfo->wpaFlags = nmAPSecFlagsToTDEAPSecFlags(accessPoint.getFlags(error), accessPoint.getWpaFlags(error));
	apInfo->rsnFlags = nmAPSecFlagsToTDEAPSecFlags(accessPoint.getFlags(error), accessPoint.getRsnFlags(error));
	apInfo->frequency = accessPoint.getFrequency(error);
	apInfo->BSSID.fromString(accessPoint.getHwAddress(error));
	apInfo->maxBitrate = accessPoint.getMaxBitrate(error);
	apInfo->signalQuality = (accessPoint.getStrength(error)/100.0);

	apInfo->valid = true;

	return apInfo;
}

NetworkHWNeighborList* NetworkConnectionManager_BackendNM::siteSurvey() {
	TQT_DBusError error;
	bool ret;

	NetworkDeviceType::NetworkDeviceType myDeviceType = deviceType();
	d->m_dbusDeviceString = deviceInterfaceString(m_macAddress);
	clearNetworkHWNeighborList();

	if (myDeviceType == NetworkDeviceType::WiFi) {
		DBus::WiFiDeviceProxy wiFiDevice(NM_DBUS_SERVICE, d->m_dbusDeviceString);
		wiFiDevice.setConnection(TQT_DBusConnection::systemBus());
		// FIXME
		// Should call wiFiDevice.RequestScanAsync first to rescan all access points
		TQT_DBusObjectPathList accessPoints;
		ret = wiFiDevice.GetAccessPoints(accessPoints, error);
		if (ret) {
			TQT_DBusObjectPathList::iterator it;
			for (it = accessPoints.begin(); it != accessPoints.end(); ++it) {
				NetworkWiFiAPInfo* apInfo = getAccessPointDetails(TQString(*it));
				if (apInfo) {
					m_hwNeighborList->append(apInfo);
					// Ensure that this AP is monitored for changes
					d->internalProcessWiFiAccessPointAdded(*it);
				}
			}
		}
	}

	return m_hwNeighborList;
}

bool NetworkConnectionManager_BackendNM::networkingEnabled() {
	if (d->m_networkManagerProxy) {
		bool ret;
		TQT_DBusError error;
		ret = d->m_networkManagerProxy->getNetworkingEnabled(error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return FALSE;
		}
		else {
			return ret;
		}
	}
	else {
		return FALSE;
	}
}

bool NetworkConnectionManager_BackendNM::wiFiHardwareEnabled() {
	if (d->m_networkManagerProxy) {
		bool ret;
		TQT_DBusError error;
		ret = d->m_networkManagerProxy->getWirelessHardwareEnabled(error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return FALSE;
		}
		else {
			return ret;
		}
	}
	else {
		return FALSE;
	}
}

bool NetworkConnectionManager_BackendNM::enableNetworking(bool enable) {
	// FIXME
	// Yes, this abuses the Sleep command
	// Is there a better way to do it?
	if (d->m_networkManagerProxy) {
		int asynccallid;
		TQT_DBusError error;
		d->m_networkManagerProxy->SleepAsync(asynccallid, !enable, error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return FALSE;
		}
		else {
			// FIXME
			// Wait for async reply before returning...
			return TRUE;
		}
	}
	else {
		return FALSE;
	}
}

bool NetworkConnectionManager_BackendNM::enableWiFi(bool enable) {
	if (d->m_networkManagerProxy) {
		TQT_DBusError error;
		d->m_networkManagerProxy->setWirelessEnabled(enable, error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
	else {
		return FALSE;
	}
}

bool NetworkConnectionManager_BackendNM::wiFiEnabled() {
	if (d->m_networkManagerProxy) {
		bool ret;
		TQT_DBusError error;
		ret = d->m_networkManagerProxy->getWirelessEnabled(error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR((error.name() + ": " + error.message()))
			return FALSE;
		}
		else {
			return ret;
		}
	}
	else {
		return FALSE;
	}
}

TQStringList NetworkConnectionManager_BackendNM::defaultNetworkDevices() {
	// Cycle through all available connections and see which one is default, then find all devices for that connection...
	TQStringList ret;

	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	if (d->m_networkManagerProxy) {
		TQT_DBusObjectPathList activeConnections = d->m_networkManagerProxy->getActiveConnections(error);
		TQT_DBusObjectPathList::iterator it;
		for (it = activeConnections.begin(); it != activeConnections.end(); ++it) {
			DBus::ActiveConnectionProxy activeConnection(NM_DBUS_SERVICE, (*it));
			activeConnection.setConnection(TQT_DBusConnection::systemBus());
			if (activeConnection.getDefault(error)) {
				// This is the default ipv4 connection
				TQString uuid = activeConnection.getUuid(error);
				TQStringList devices = connectionPhysicalDeviceUUIDs(uuid);
				for (TQStringList::Iterator it2 = devices.begin(); it2 != devices.end(); ++it2) {
					ret.append(*it);
				}
			}
			else if (activeConnection.getDefault6(error)) {
				// This is the default ipv6 connection
				TQString uuid = activeConnection.getUuid(error);
				TQStringList devices = connectionPhysicalDeviceUUIDs(uuid);
				for (TQStringList::Iterator it2 = devices.begin(); it2 != devices.end(); ++it2) {
					ret.append(*it);
				}
			}
		}
		return ret;
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object"));
		return TQStringList();
	}
}

NetworkConnectionManager_BackendNMPrivate::NetworkConnectionManager_BackendNMPrivate(NetworkConnectionManager_BackendNM* parent) : m_networkManagerProxy(NULL), m_networkManagerSettings(NULL), m_networkDeviceProxy(NULL), m_wiFiDeviceProxy(NULL), m_vpnProxy(NULL), nonReentrantCallActive(false), vpn_service_error_notified(false), device_autoconnect_error_notified(false), m_parent(parent), m_prevDeviceState(-1) {
	// Set up global signal handler
	m_dbusSignalConnection = new TQT_DBusConnection(TQT_DBusConnection::systemBus());
	m_dbusSignalReceiver = new NetworkConnectionManager_BackendNM_DBusSignalReceiver(this);
	m_dbusSignalConnection->connect(m_dbusSignalReceiver, TQT_SLOT(dbusSignal(const TQT_DBusMessage&)));
}

NetworkConnectionManager_BackendNMPrivate::~NetworkConnectionManager_BackendNMPrivate() {
	// Destroy global signal handler
	if (m_dbusSignalConnection) delete m_dbusSignalConnection;
	if (m_dbusSignalReceiver) delete m_dbusSignalReceiver;

	// Destroy proxy objects
	TQMap<TQString, DBus::AccessPointProxy*>::iterator it;
	for (it = m_accessPointProxyList.begin(); it != m_accessPointProxyList.end(); ++it) {
		DBus::AccessPointProxy *apProxy = it.data();
		if (apProxy) {
			delete apProxy;
		}
	}
	m_accessPointProxyList.clear();
}

#include "network-manager.moc"
#include "network-manager_p.moc"
