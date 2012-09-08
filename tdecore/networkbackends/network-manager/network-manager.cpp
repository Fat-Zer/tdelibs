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

#include "network-manager.h"
#include "network-manager_p.h"

#define PRINT_ERROR(x) printf("[TDE NM Backend ERROR] %s\n\r", x.ascii());
#define UPDATE_STRING_SETTING_IF_VALID(string, key, settingsMap)	if (!string.isNull()) settingsMap[key] = convertDBUSDataToVariantData(TQT_DBusData::fromString(string));	\
									else settingsMap.remove(key);

#define NM_ASYNC_TIMEOUT_MS 1000

#define DEBUG_NETWORK_MANAGER_COMMUNICATIONS

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

void printDBUSObjectStructure(TQT_DBusData object, int level=0) {
	int i;
	TQString levelIndent = "";
	for (i=0; i<level; i++) {
		levelIndent = levelIndent + " ";
	}
	TQCString signature = object.buildDBusSignature();
	printf("%s%s\n\r", levelIndent.ascii(), signature.data()); fflush(stdout);

	if (object.type() == TQT_DBusData::Map) {
		// HACK
		// NetworkManager currently uses string key maps exclusively as far as I can tell, so this should be adequate for the time being
		TQMap<TQString, TQT_DBusData> outerMap = object.toStringKeyMap().toTQMap();
		TQMap<TQString, TQT_DBusData>::const_iterator it;
        	for (it = outerMap.begin(); it != outerMap.end(); ++it) {
			printDBUSObjectStructure(*it, level+1);
		}
	}
	if (object.type() == TQT_DBusData::List) {
		TQT_DBusDataValueList valueList = object.toTQValueList();
		TQT_DBusDataValueList::const_iterator it;
		for (it = valueList.begin(); it != valueList.end(); ++it) {
			printDBUSObjectStructure(*it, level+1);
		}
	}
	else if (object.type() == TQT_DBusData::Variant) {
		TQT_DBusVariant dataValueVariant = object.toVariant();
		TQT_DBusData dataValue = dataValueVariant.value;
		printDBUSObjectStructure(dataValue, level+1);
	}
}

TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags nmGlobalStateToTDEGlobalState(TQ_UINT32 nmType) {
	TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags ret = TDENetworkGlobalManagerFlags::Unknown;

	if (nmType == NM_STATE_UNKNOWN) {
		ret |= TDENetworkGlobalManagerFlags::Unknown;
	}
	else if (nmType == NM_STATE_ASLEEP) {
		ret |= TDENetworkGlobalManagerFlags::Disconnected;
		ret |= TDENetworkGlobalManagerFlags::Sleeping;
	}
	else if (nmType == NM_STATE_DISCONNECTED) {
		ret |= TDENetworkGlobalManagerFlags::Disconnected;
	}
	else if (nmType == NM_STATE_DISCONNECTING) {
		ret |= TDENetworkGlobalManagerFlags::Connected;
		ret |= TDENetworkGlobalManagerFlags::DeactivatingLink;
	}
	else if (nmType == NM_STATE_CONNECTING) {
		ret |= TDENetworkGlobalManagerFlags::Disconnected;
		ret |= TDENetworkGlobalManagerFlags::EstablishingLink;
	}
	else if (nmType == NM_STATE_CONNECTED_LOCAL) {
		ret |= TDENetworkGlobalManagerFlags::Connected;
		ret |= TDENetworkGlobalManagerFlags::LinkLocalAccess;
	}
	else if (nmType == NM_STATE_CONNECTED_SITE) {
		ret |= TDENetworkGlobalManagerFlags::Connected;
		ret |= TDENetworkGlobalManagerFlags::SiteLocalAccess;
	}
	else if (nmType == NM_STATE_CONNECTED_GLOBAL) {
		ret |= TDENetworkGlobalManagerFlags::Connected;
		ret |= TDENetworkGlobalManagerFlags::GlobalAccess;
	}

	return ret;
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus nmDeviceStateToTDEDeviceState(TQ_UINT32 nmType) {
	TDENetworkConnectionStatus::TDENetworkConnectionStatus ret = TDENetworkConnectionStatus::Invalid;

	if (nmType == NM_DEVICE_STATE_UNKNOWN) {
		ret |= TDENetworkConnectionStatus::Invalid;
	}
	else if (nmType == NM_DEVICE_STATE_UNMANAGED) {
		ret |= TDENetworkConnectionStatus::UnManaged;
	}
	else if (nmType == NM_DEVICE_STATE_UNAVAILABLE) {
		ret |= TDENetworkConnectionStatus::Disconnected;
		ret |= TDENetworkConnectionStatus::LinkUnavailable;
	}
	else if (nmType == NM_DEVICE_STATE_DISCONNECTED) {
		ret |= TDENetworkConnectionStatus::Disconnected;
	}
	else if (nmType == NM_DEVICE_STATE_PREPARE) {
		ret |= TDENetworkConnectionStatus::Disconnected;
		ret |= TDENetworkConnectionStatus::EstablishingLink;
	}
	else if (nmType == NM_DEVICE_STATE_CONFIG) {
		ret |= TDENetworkConnectionStatus::Disconnected;
		ret |= TDENetworkConnectionStatus::EstablishingLink;
	}
	else if (nmType == NM_DEVICE_STATE_NEED_AUTH) {
		ret |= TDENetworkConnectionStatus::Disconnected;
		ret |= TDENetworkConnectionStatus::NeedAuthorization;
	}
	else if (nmType == NM_DEVICE_STATE_IP_CONFIG) {
		ret |= TDENetworkConnectionStatus::Disconnected;
		ret |= TDENetworkConnectionStatus::ConfiguringProtocols;
	}
	else if (nmType == NM_DEVICE_STATE_IP_CHECK) {
		ret |= TDENetworkConnectionStatus::Disconnected;
		ret |= TDENetworkConnectionStatus::VerifyingProtocols;
	}
	else if (nmType == NM_DEVICE_STATE_SECONDARIES) {
		ret |= TDENetworkConnectionStatus::Disconnected;
		ret |= TDENetworkConnectionStatus::DependencyWait;
	}
	else if (nmType == NM_DEVICE_STATE_ACTIVATED) {
		ret |= TDENetworkConnectionStatus::Connected;
	}
	else if (nmType == NM_DEVICE_STATE_DEACTIVATING) {
		ret |= TDENetworkConnectionStatus::Connected;
		ret |= TDENetworkConnectionStatus::DeactivatingLink;
	}
	else if (nmType == NM_DEVICE_STATE_FAILED) {
		ret |= TDENetworkConnectionStatus::Disconnected;
		ret |= TDENetworkConnectionStatus::Failed;
	}

	return ret;
}

TDENetworkDeviceType::TDENetworkDeviceType TDENetworkConnectionManager_BackendNM::nmDeviceTypeToTDEDeviceType(TQ_UINT32 nmType) {
	TDENetworkDeviceType::TDENetworkDeviceType ret = TDENetworkDeviceType::Other;

	if (nmType == NM_DEVICE_TYPE_UNKNOWN) {
		ret = TDENetworkDeviceType::Other;
	}
	else if (nmType == NM_DEVICE_TYPE_ETHERNET) {
		ret = TDENetworkDeviceType::WiredEthernet;
	}
	else if (nmType == NM_DEVICE_TYPE_WIFI) {
		ret = TDENetworkDeviceType::WiFi;
	}
	else if (nmType == NM_DEVICE_TYPE_UNUSED1) {
	}
	else if (nmType == NM_DEVICE_TYPE_UNUSED2) {
	}
	else if (nmType == NM_DEVICE_TYPE_BT) {
		ret = TDENetworkDeviceType::Bluetooth;
	}
	else if (nmType == NM_DEVICE_TYPE_OLPC_MESH) {
		ret = TDENetworkDeviceType::OLPCMesh;
	}
	else if (nmType == NM_DEVICE_TYPE_WIMAX) {
		ret = TDENetworkDeviceType::WiMax;
	}
	else if (nmType == NM_DEVICE_TYPE_MODEM) {
		ret = TDENetworkDeviceType::Modem;
	}
	else if (nmType == NM_DEVICE_TYPE_INFINIBAND) {
		ret = TDENetworkDeviceType::Infiniband;
	}
	else if (nmType == NM_DEVICE_TYPE_BOND) {
		ret = TDENetworkDeviceType::Bond;
	}
	else if (nmType == NM_DEVICE_TYPE_VLAN) {
		ret = TDENetworkDeviceType::VLAN;
	}
	else if (nmType == NM_DEVICE_TYPE_ADSL) {
		ret = TDENetworkDeviceType::ADSL;
	}

	return ret;
}

TDENetworkConnectionType::TDENetworkConnectionType nmConnectionTypeToTDEConnectionType(TQString nm) {
	TDENetworkConnectionType::TDENetworkConnectionType ret = TDENetworkConnectionType::Other;

	if (nm.lower() == "802-3-ethernet") {
		ret = TDENetworkConnectionType::WiredEthernet;
	}
	else if (nm.lower() == "infiniband") {
		ret = TDENetworkConnectionType::Infiniband;
	}
	else if (nm.lower() == "802-11-wireless") {
		ret = TDENetworkConnectionType::WiFi;
	}
	else if (nm.lower() == "vpn") {
		ret = TDENetworkConnectionType::VPN;
	}
	else if (nm.lower() == "wimax") {
		ret = TDENetworkConnectionType::WiMax;
	}
	else if (nm.lower() == "vlan") {
		ret = TDENetworkConnectionType::VLAN;
	}
	else if (nm.lower() == "802-11-olpc-mesh") {
		ret = TDENetworkConnectionType::OLPCMesh;
	}
	else if (nm.lower() == "bluetooth") {
		ret = TDENetworkConnectionType::Bluetooth;
	}
	else if (nm.lower() == "cdma") {
		ret = TDENetworkConnectionType::Modem;
	}
	else if (nm.lower() == "gsm") {
		ret = TDENetworkConnectionType::Modem;
	}

	return ret;
}

TQString tdeConnectionTypeToNMConnectionType(TDENetworkConnectionType::TDENetworkConnectionType type, TDEModemConnectionType::TDEModemConnectionType modemType=TDEModemConnectionType::Other) {
	TQString ret;

	if (type == TDENetworkConnectionType::WiredEthernet) {
		ret = "802-3-ethernet";
	}
	else if (type == TDENetworkConnectionType::Infiniband) {
		ret = "infiniband";
	}
	else if (type == TDENetworkConnectionType::WiFi) {
		ret = "802-11-wireless";
	}
	else if (type == TDENetworkConnectionType::VPN) {
		ret = "vpn";
	}
	else if (type == TDENetworkConnectionType::WiMax) {
		ret = "wimax";
	}
	else if (type == TDENetworkConnectionType::VLAN) {
		ret = "vlan";
	}
	else if (type == TDENetworkConnectionType::OLPCMesh) {
		ret = "802-11-olpc-mesh";
	}
	else if (type == TDENetworkConnectionType::Bluetooth) {
		ret = "bluetooth";
	}
	else if (type == TDENetworkConnectionType::Modem) {
		if (modemType == TDEModemConnectionType::CDMA) {
			ret = "cdma";
		}
		else if (modemType == TDEModemConnectionType::GSM) {
			ret = "gsm";
		}
	}

	return ret;
}

TDENetworkIEEE8021xType::TDENetworkIEEE8021xType nmEAPTypeToTDEEAPType(TQString nm) {
	TDENetworkIEEE8021xType::TDENetworkIEEE8021xType ret = TDENetworkIEEE8021xType::None;

	if (nm.lower() == "") {
		ret = TDENetworkIEEE8021xType::None;
	}
	else if (nm.lower() == "leap") {
		ret = TDENetworkIEEE8021xType::LEAP;
	}
	else if (nm.lower() == "md5") {
		ret = TDENetworkIEEE8021xType::MD5;
	}
	else if (nm.lower() == "pap") {
		ret = TDENetworkIEEE8021xType::PAP;
	}
	else if (nm.lower() == "chap") {
		ret = TDENetworkIEEE8021xType::CHAP;
	}
	else if (nm.lower() == "mschap") {
		ret = TDENetworkIEEE8021xType::MSCHAP;
	}
	else if (nm.lower() == "mschapv2") {
		ret = TDENetworkIEEE8021xType::MSCHAPV2;
	}
	else if (nm.lower() == "fast") {
		ret = TDENetworkIEEE8021xType::Fast;
	}
	else if (nm.lower() == "psk") {
		ret = TDENetworkIEEE8021xType::PSK;
	}
	else if (nm.lower() == "pax") {
		ret = TDENetworkIEEE8021xType::PAX;
	}
	else if (nm.lower() == "sake") {
		ret = TDENetworkIEEE8021xType::SAKE;
	}
	else if (nm.lower() == "gpsk") {
		ret = TDENetworkIEEE8021xType::GPSK;
	}
	else if (nm.lower() == "tls") {
		ret = TDENetworkIEEE8021xType::TLS;
	}
	else if (nm.lower() == "peap") {
		ret = TDENetworkIEEE8021xType::PEAP;
	}
	else if (nm.lower() == "ttls") {
		ret = TDENetworkIEEE8021xType::TTLS;
	}
	else if (nm.lower() == "sim") {
		ret = TDENetworkIEEE8021xType::SIM;
	}
	else if (nm.lower() == "gtc") {
		ret = TDENetworkIEEE8021xType::GTC;
	}
	else if (nm.lower() == "otp") {
		ret = TDENetworkIEEE8021xType::OTP;
	}
	else {
		PRINT_ERROR(TQString("unknown EAP type %s requested in existing connection").arg(nm.lower()))
	}

	return ret;
}

TQString tdeEAPTypeToNMEAPType(TDENetworkIEEE8021xType::TDENetworkIEEE8021xType eaptype) {
	TQString ret = "";

	if (eaptype == TDENetworkIEEE8021xType::None) {
		ret = "";
	}
	else if (eaptype == TDENetworkIEEE8021xType::LEAP) {
		ret = "leap";
	}
	else if (eaptype == TDENetworkIEEE8021xType::MD5) {
		ret = "md5";
	}
	else if (eaptype == TDENetworkIEEE8021xType::PAP) {
		ret = "pap";
	}
	else if (eaptype == TDENetworkIEEE8021xType::CHAP) {
		ret = "chap";
	}
	else if (eaptype == TDENetworkIEEE8021xType::MSCHAP) {
		ret = "mschap";
	}
	else if (eaptype == TDENetworkIEEE8021xType::MSCHAPV2) {
		ret = "mschapv2";
	}
	else if (eaptype == TDENetworkIEEE8021xType::Fast) {
		ret = "fast";
	}
	else if (eaptype == TDENetworkIEEE8021xType::PSK) {
		ret = "psk";
	}
	else if (eaptype == TDENetworkIEEE8021xType::PAX) {
		ret = "pax";
	}
	else if (eaptype == TDENetworkIEEE8021xType::SAKE) {
		ret = "sake";
	}
	else if (eaptype == TDENetworkIEEE8021xType::GPSK) {
		ret = "gpsk";
	}
	else if (eaptype == TDENetworkIEEE8021xType::TLS) {
		ret = "tls";
	}
	else if (eaptype == TDENetworkIEEE8021xType::PEAP) {
		ret = "peap";
	}
	else if (eaptype == TDENetworkIEEE8021xType::TTLS) {
		ret = "ttls";
	}
	else if (eaptype == TDENetworkIEEE8021xType::SIM) {
		ret = "sim";
	}
	else if (eaptype == TDENetworkIEEE8021xType::GTC) {
		ret = "gtc";
	}
	else if (eaptype == TDENetworkIEEE8021xType::OTP) {
		ret = "otp";
	}
	else {
		PRINT_ERROR(TQString("unknown TDE EAP type %d requested in new or updated connection").arg(eaptype))
	}

	return ret;
}

TDENetworkIEEE8021xFastFlags::TDENetworkIEEE8021xFastFlags nmEAPFastFlagsToTDEEAPFastFlags(TQString nm) {
	TDENetworkIEEE8021xFastFlags::TDENetworkIEEE8021xFastFlags ret = TDENetworkIEEE8021xFastFlags::AllowUnauthenticated | TDENetworkIEEE8021xFastFlags::AllowAuthenticated;

	unsigned int nm_int = nm.toUInt();
	if (nm_int == NM_EAP_FAST_PROVISIONING_DISABLED) {
		ret = TDENetworkIEEE8021xFastFlags::None;
	}
	else if (nm_int == NM_EAP_FAST_PROVISIONING_UNAUTHONLY) {
		ret = TDENetworkIEEE8021xFastFlags::AllowUnauthenticated;
	}
	else if (nm_int == NM_EAP_FAST_PROVISIONING_AUTHONLY) {
		ret = TDENetworkIEEE8021xFastFlags::AllowAuthenticated;
	}
	else if (nm_int == NM_EAP_FAST_PROVISIONING_BOTH) {
		ret = TDENetworkIEEE8021xFastFlags::AllowUnauthenticated | TDENetworkIEEE8021xFastFlags::AllowAuthenticated;
	}
	else {
		PRINT_ERROR(TQString("unknown EAP fast flag %s requested in existing connection").arg(nm.lower()))
	}

	return ret;
}

TQString tdeEAPFastFlagsToNMEAPFastFlags(TDENetworkIEEE8021xFastFlags::TDENetworkIEEE8021xFastFlags eaptype) {
	TQString ret = "";

	if ((eaptype & TDENetworkIEEE8021xFastFlags::AllowUnauthenticated) && (eaptype & TDENetworkIEEE8021xFastFlags::AllowAuthenticated)) {
		ret = TQString("%1").arg(NM_EAP_FAST_PROVISIONING_BOTH);
	}
	else if (eaptype & TDENetworkIEEE8021xFastFlags::AllowAuthenticated) {
		ret = TQString("%1").arg(NM_EAP_FAST_PROVISIONING_AUTHONLY);
	}
	else if (eaptype & TDENetworkIEEE8021xFastFlags::AllowUnauthenticated) {
		ret = TQString("%1").arg(NM_EAP_FAST_PROVISIONING_UNAUTHONLY);
	}
	else {
		ret = TQString("%1").arg(NM_EAP_FAST_PROVISIONING_DISABLED);
	}

	return ret;
}

TDEWiFiMode::TDEWiFiMode nmWiFiModeToTDEWiFiMode(TQString nm) {
	TDEWiFiMode::TDEWiFiMode ret = TDEWiFiMode::Infrastructure;

	if (nm.lower() == "infrastructure") {
		ret = TDEWiFiMode::Infrastructure;
	}
	else if (nm.lower() == "adhoc") {
		ret = TDEWiFiMode::AdHoc;
	}

	return ret;
}

TQString tdeWiFiModeToNMWiFiMode(TDEWiFiMode::TDEWiFiMode mode) {
	TQString ret;

	if (mode == TDEWiFiMode::Infrastructure) {
		ret = "infrastructure";
	}
	else if (mode == TDEWiFiMode::AdHoc) {
		ret = "adhoc";
	}

	return ret;
}

TDEWiFiMode::TDEWiFiMode nmWiFiModeToTDEWiFiMode(TQ_UINT32 nm) {
	TDEWiFiMode::TDEWiFiMode ret = TDEWiFiMode::Infrastructure;

	if (nm == NM_802_11_MODE_INFRASTRUCTURE) {
		ret = TDEWiFiMode::Infrastructure;
	}
	else if (nm == NM_802_11_MODE_ADHOC) {
		ret = TDEWiFiMode::AdHoc;
	}

	return ret;
}

TDENetworkWiFiClientFlags::TDENetworkWiFiClientFlags tdeWiFiFlagsToNMWiFiFlags(TQ_UINT32 nm) {
	TDENetworkWiFiClientFlags::TDENetworkWiFiClientFlags ret = TDENetworkWiFiClientFlags::None;

	if (nm & NM_802_11_DEVICE_CAP_CIPHER_WEP40) {
		ret | TDENetworkWiFiClientFlags::CipherWEP40;
	}
	if (nm & NM_802_11_DEVICE_CAP_CIPHER_WEP104) {
		ret | TDENetworkWiFiClientFlags::CipherWEP104;
	}
	if (nm & NM_802_11_DEVICE_CAP_CIPHER_TKIP) {
		ret | TDENetworkWiFiClientFlags::CipherTKIP;
	}
	if (nm & NM_802_11_DEVICE_CAP_CIPHER_CCMP) {
		ret | TDENetworkWiFiClientFlags::CipherCCMP;
	}
	if (nm & NM_802_11_DEVICE_CAP_WPA) {
		ret | TDENetworkWiFiClientFlags::CipherWPA;
	}
	if (nm & NM_802_11_DEVICE_CAP_RSN) {
		ret | TDENetworkWiFiClientFlags::CipherRSN;
	}

	return ret;
}

TDEBluetoothConnectionType::TDEBluetoothConnectionType nmBluetoothModeToTDEBluetoothMode(TQString nm) {
	TDEBluetoothConnectionType::TDEBluetoothConnectionType ret = TDEBluetoothConnectionType::PAN;

	if (nm.lower() == "dun") {
		ret = TDEBluetoothConnectionType::DUN;
	}
	else if (nm.lower() == "panu") {
		ret = TDEBluetoothConnectionType::PAN;
	}

	return ret;
}

TQString tdeBluetoothModeToNMBluetoothMode(TDEBluetoothConnectionType::TDEBluetoothConnectionType type) {
	TQString ret;

	if (type == TDEBluetoothConnectionType::DUN) {
		ret = "dun";
	}
	else if (type == TDEBluetoothConnectionType::PAN) {
		ret = "panu";
	}

	return ret;
}

TDEGSMNetworkType::TDEGSMNetworkType nmGSMModeToTDEGSMMode(TQ_INT32 nm) {
	TDEGSMNetworkType::TDEGSMNetworkType ret = TDEGSMNetworkType::Any;

	if (nm == NM_GSM_3G_ONLY) {
		ret = TDEGSMNetworkType::Only3G;
	}
	else if (nm == NM_GSM_GPRS_EDGE_ONLY) {
		ret = TDEGSMNetworkType::GPRSEdge;
	}
	else if (nm == NM_GSM_PREFER_3G) {
		ret = TDEGSMNetworkType::Prefer3G;
	}
	else if (nm == NM_GSM_PREFER_2G) {
		ret = TDEGSMNetworkType::Prefer2G;
	}

	return ret;
}

TQ_INT32 tdeGSMModeToNMGSMMode(TDEGSMNetworkType::TDEGSMNetworkType type) {
	TQ_INT32 ret = -1;

	if (type == TDEGSMNetworkType::Only3G) {
		ret = NM_GSM_3G_ONLY;
	}
	else if (type == TDEGSMNetworkType::GPRSEdge) {
		ret = NM_GSM_GPRS_EDGE_ONLY;
	}
	else if (type == TDEGSMNetworkType::Prefer3G) {
		ret = NM_GSM_PREFER_3G;
	}
	else if (type == TDEGSMNetworkType::Prefer2G) {
		ret = NM_GSM_PREFER_2G;
	}

	return ret;
}

TDEWiFiFrequencyBand::TDEWiFiFrequencyBand nmWiFiFrequencyBandToTDEWiFiFrequencyBand(TQString nm) {
	TDEWiFiFrequencyBand::TDEWiFiFrequencyBand ret = TDEWiFiFrequencyBand::Other;

	if (nm.lower() == "") {
		ret = TDEWiFiFrequencyBand::None;
	}
	else if (nm.lower() == "bg") {
		ret = TDEWiFiFrequencyBand::Band2_4GHz;
	}
	else if (nm.lower() == "a") {
		ret = TDEWiFiFrequencyBand::Band5GHz;
	}

	return ret;
}

TQString tdeWiFiFrequencyBandToNMWiFiFrequencyBand(TDEWiFiFrequencyBand::TDEWiFiFrequencyBand mode) {
	TQString ret;

	if (mode == TDEWiFiFrequencyBand::None) {
		ret = TQString::null;
	}
	else if (mode == TDEWiFiFrequencyBand::Band2_4GHz) {
		ret = "bg";
	}
	else if (mode == TDEWiFiFrequencyBand::Band5GHz) {
		ret = "a";
	}

	return ret;
}

TDENetworkWiFiKeyType::TDENetworkWiFiKeyType nmWiFiKeyTypeToTDEWiFiKeyType(TQString nm) {
	TDENetworkWiFiKeyType::TDENetworkWiFiKeyType ret = TDENetworkWiFiKeyType::Other;

	if (nm.lower() == "none") {
		ret = TDENetworkWiFiKeyType::WEP;
	}
	else if (nm.lower() == "ieee8021x") {
		ret = TDENetworkWiFiKeyType::DynamicWEP;
	}
	else if (nm.lower() == "wpa-none") {
		ret = TDENetworkWiFiKeyType::WPAAdHoc;
	}
	else if (nm.lower() == "wpa-psk") {
		ret = TDENetworkWiFiKeyType::WPAInfrastructure;
	}
	else if (nm.lower() == "wpa-eap") {
		ret = TDENetworkWiFiKeyType::WPAEnterprise;
	}

	return ret;
}

TQString tdeWiFiKeyTypeToNMWiFiKeyType(TDENetworkWiFiKeyType::TDENetworkWiFiKeyType type) {
	TQString ret;

	if (type == TDENetworkWiFiKeyType::WEP) {
		return "none";
	}
	else if (type == TDENetworkWiFiKeyType::DynamicWEP) {
		return "ieee8021x";
	}
	else if (type == TDENetworkWiFiKeyType::WPAAdHoc) {
		return "wpa-none";
	}
	else if (type == TDENetworkWiFiKeyType::WPAInfrastructure) {
		return "wpa-psk";
	}
	else if (type == TDENetworkWiFiKeyType::WPAEnterprise) {
		return "wpa-eap";
	}

	return ret;
}

TDENetworkWiFiAuthType::TDENetworkWiFiAuthType nmWiFiAuthTypeToTDEWiFiAuthType(TQString nm) {
	TDENetworkWiFiAuthType::TDENetworkWiFiAuthType ret = TDENetworkWiFiAuthType::Other;

	if (nm.lower() == "open") {
		ret = TDENetworkWiFiAuthType::Open;
	}
	else if (nm.lower() == "shared") {
		ret = TDENetworkWiFiAuthType::Shared;
	}
	else if (nm.lower() == "leap") {
		ret = TDENetworkWiFiAuthType::LEAP;
	}

	return ret;
}

TQString tdeWiFiAuthTypeToNMWiFiAuthType(TDENetworkWiFiAuthType::TDENetworkWiFiAuthType type) {
	TQString ret;

	if (type == TDENetworkWiFiAuthType::Open) {
		return "open";
	}
	else if (type == TDENetworkWiFiAuthType::Shared) {
		return "shared";
	}
	else if (type == TDENetworkWiFiAuthType::LEAP) {
		return "leap";
	}

	return ret;
}

TDENetworkWiFiWPAVersionFlags::TDENetworkWiFiWPAVersionFlags nmWiFiWPAVersionToTDEWiFiWPAVersion(TQStringList nm) {
	TDENetworkWiFiWPAVersionFlags::TDENetworkWiFiWPAVersionFlags ret = TDENetworkWiFiWPAVersionFlags::None;

	if ((nm.contains("wpa") && nm.contains("rsn")) || (nm.count() < 1)) {
		ret |= TDENetworkWiFiWPAVersionFlags::Any;
	}
	else if (nm.contains("wpa")) {
		ret |= TDENetworkWiFiWPAVersionFlags::WPA;
	}
	else if (nm.contains("rsn")) {
		ret |= TDENetworkWiFiWPAVersionFlags::RSN;
	}

	return ret;
}

TQStringList tdeWiFiWPAVersionToNMWiFiWPAVersion(TDENetworkWiFiWPAVersionFlags::TDENetworkWiFiWPAVersionFlags type) {
	TQStringList ret;

	if (type & TDENetworkWiFiWPAVersionFlags::WPA) {
		ret.append("wpa");
	}
	if (type & TDENetworkWiFiWPAVersionFlags::RSN) {
		ret.append("rsn");
	}

	return ret;
}

TDENetworkWiFiConnectionCipher::TDENetworkWiFiConnectionCipher nmWiFiCipherToTDEWiFiCipher(TQString nm) {
	TDENetworkWiFiConnectionCipher::TDENetworkWiFiConnectionCipher ret = TDENetworkWiFiConnectionCipher::None;

	if (nm.lower() == "wep40") {
		ret = TDENetworkWiFiConnectionCipher::CipherWEP40;
	}
	else if (nm.lower() == "wep104") {
		ret = TDENetworkWiFiConnectionCipher::CipherWEP104;
	}
	else if (nm.lower() == "tkip") {
		ret = TDENetworkWiFiConnectionCipher::CipherTKIP;
	}
	else if (nm.lower() == "ccmp") {
		ret = TDENetworkWiFiConnectionCipher::CipherCCMP;
	}

	return ret;
}

TQString tdeWiFiCipherToNMWiFiCipher(TDENetworkWiFiConnectionCipher::TDENetworkWiFiConnectionCipher cipher) {
	TQString ret;

	if (cipher == TDENetworkWiFiConnectionCipher::CipherWEP40) {
		ret = "wep40";
	}
	else if (cipher == TDENetworkWiFiConnectionCipher::CipherWEP104) {
		ret = "wep104";
	}
	else if (cipher == TDENetworkWiFiConnectionCipher::CipherTKIP) {
		ret = "tkip";
	}
	else if (cipher == TDENetworkWiFiConnectionCipher::CipherCCMP) {
		ret = "ccmp";
	}

	return ret;
}

TDENetworkSlaveDeviceType::TDENetworkSlaveDeviceType nmSlaveTypeToTDESlaveType(TQString nm) {
	TDENetworkSlaveDeviceType::TDENetworkSlaveDeviceType ret = TDENetworkSlaveDeviceType::None;

	if (nm.lower() == "bond") {
		ret = TDENetworkSlaveDeviceType::Bond;
	}

	return ret;
}

TQString tdeSlaveTypeToNMSlaveType(TDENetworkSlaveDeviceType::TDENetworkSlaveDeviceType slavetype) {
	TQString ret;

	if (slavetype == TDENetworkSlaveDeviceType::Bond) {
		ret = "bond";
	}

	return ret;
}

TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags nmPasswordFlagsToTDEPasswordFlags(unsigned int nm) {
	TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags ret = TDENetworkPasswordHandlingFlags::None;

	if (nm & NM_PASSWORD_SECRET_AGENTOWNED) {
		ret |= TDENetworkPasswordHandlingFlags::ExternalStorage;
	}
	if (nm & NM_PASSWORD_SECRET_NOTSAVED) {
		ret |= TDENetworkPasswordHandlingFlags::NoSave;
	}
	if (nm & NM_PASSWORD_SECRET_NOTREQUIRED) {
		ret |= TDENetworkPasswordHandlingFlags::NoPrompt;
	}

	return ret;
}

unsigned int tdePasswordFlagsToNMPasswordFlags(TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags flags) {
	unsigned int ret = 0;

	if (flags & TDENetworkPasswordHandlingFlags::ExternalStorage) {
		ret |= NM_PASSWORD_SECRET_AGENTOWNED;
	}
	if (flags & TDENetworkPasswordHandlingFlags::NoSave) {
		ret |= NM_PASSWORD_SECRET_NOTSAVED;
	}
	if (flags & TDENetworkPasswordHandlingFlags::NoPrompt) {
		ret |= NM_PASSWORD_SECRET_NOTREQUIRED;
	}

	return ret;
}

TDENetworkVLANFlags::TDENetworkVLANFlags nmVLANFlagsToTDEVLANFlags(unsigned int nm) {
	TDENetworkVLANFlags::TDENetworkVLANFlags ret = TDENetworkVLANFlags::None;

	if (nm & NM_VLAN_REORDER_PACKET_HEADERS) {
		ret |= TDENetworkVLANFlags::ReorderPacketHeaders;
	}
	if (nm & NM_VLAN_USE_GVRP) {
		ret |= TDENetworkVLANFlags::UseGVRP;
	}
	if (nm & NM_VLAN_LOOSE_BINDING) {
		ret |= TDENetworkVLANFlags::LooseBinding;
	}

	return ret;
}

unsigned int tdeVLANFlagsToNMVLANFlags(TDENetworkVLANFlags::TDENetworkVLANFlags flags) {
	unsigned int ret = 0;

	if (flags & TDENetworkVLANFlags::ReorderPacketHeaders) {
		ret |= NM_VLAN_REORDER_PACKET_HEADERS;
	}
	if (flags & TDENetworkVLANFlags::UseGVRP) {
		ret |= NM_VLAN_USE_GVRP;
	}
	if (flags & TDENetworkVLANFlags::LooseBinding) {
		ret |= NM_VLAN_LOOSE_BINDING;
	}

	return ret;
}

TDENetworkParity::TDENetworkParity nmParityToTDEParity(char nm) {
	TDENetworkParity::TDENetworkParity ret = TDENetworkParity::None;

	if (nm == 'E') {
		ret = TDENetworkParity::Even;
	}
	else if (nm == 'o') {
		ret = TDENetworkParity::Odd;
	}

	return ret;
}

char tdeParityToNMParity(TDENetworkParity::TDENetworkParity parity) {
	char ret = 'n';

	if (parity == TDENetworkParity::Even) {
		ret = 'E';
	}
	else if (parity == TDENetworkParity::Odd) {
		ret = 'o';
	}

	return ret;
}

TDENetworkWepKeyType::TDENetworkWepKeyType nmWepKeyTypeToTDEWepKeyType(unsigned int nm) {
	TDENetworkWepKeyType::TDENetworkWepKeyType ret = TDENetworkWepKeyType::Hexadecimal;

	if (nm == NM_WEP_TYPE_HEXADECIMAL) {
		ret = TDENetworkWepKeyType::Hexadecimal;
	}
	else if (nm == NM_WEP_TYPE_PASSPHRASE) {
		ret = TDENetworkWepKeyType::Passphrase;
	}

	return ret;
}

unsigned int tdeWepKeyTypeToNMWepKeyType(TDENetworkWepKeyType::TDENetworkWepKeyType type) {
	unsigned int ret = 0;

	if (type == TDENetworkWepKeyType::Hexadecimal) {
		ret = NM_WEP_TYPE_HEXADECIMAL;
	}
	else if (type == TDENetworkWepKeyType::Passphrase) {
		ret = NM_WEP_TYPE_PASSPHRASE;
	}

	return ret;
}

TDENetworkDeviceCapabilityFlags::TDENetworkDeviceCapabilityFlags nmCapabilityFlagsToTDECapabilityFlags(unsigned int nm) {
	TDENetworkDeviceCapabilityFlags::TDENetworkDeviceCapabilityFlags ret = TDENetworkDeviceCapabilityFlags::None;

	if (nm & NM_DEVICE_CAP_NM_SUPPORTED) {
		ret |= TDENetworkDeviceCapabilityFlags::Supported;
	}
	if (nm & NM_DEVICE_CAP_CARRIER_DETECT) {
		ret |= TDENetworkDeviceCapabilityFlags::CanDetectLink;
	}

	return ret;
}

unsigned int tdeCapabilityFlagsToNMCapabilityFlags(TDENetworkDeviceCapabilityFlags::TDENetworkDeviceCapabilityFlags flags) {
	unsigned int ret = 0;

	if (flags & TDENetworkDeviceCapabilityFlags::Supported) {
		ret |= NM_DEVICE_CAP_NM_SUPPORTED;
	}
	if (flags & TDENetworkDeviceCapabilityFlags::CanDetectLink) {
		ret |= NM_DEVICE_CAP_CARRIER_DETECT;
	}

	return ret;
}

TDENetworkWiFiAPFlags::TDENetworkWiFiAPFlags nmAPSecFlagsToTDEAPSecFlags(unsigned int genflags, unsigned int nm) {
	TDENetworkWiFiAPFlags::TDENetworkWiFiAPFlags ret = TDENetworkWiFiAPFlags::None;

	if (genflags & NM_ACCESS_POINT_CAP_PRIVACY) {
		ret |= TDENetworkWiFiAPFlags::PrivacySupport;
	}

	if (nm & NM_ACCESS_POINT_SEC_PAIR_WEP40) {
		ret |= TDENetworkWiFiAPFlags::PairWEP40;
	}
	if (nm & NM_ACCESS_POINT_SEC_PAIR_WEP104) {
		ret |= TDENetworkWiFiAPFlags::PairWEP104;
	}
	if (nm & NM_ACCESS_POINT_SEC_PAIR_TKIP) {
		ret |= TDENetworkWiFiAPFlags::PairTKIP;
	}
	if (nm & NM_ACCESS_POINT_SEC_PAIR_CCMP) {
		ret |= TDENetworkWiFiAPFlags::PairCCMP;
	}
	if (nm & NM_ACCESS_POINT_SEC_GROUP_WEP40) {
		ret |= TDENetworkWiFiAPFlags::GroupWEP40;
	}
	if (nm & NM_ACCESS_POINT_SEC_GROUP_WEP104) {
		ret |= TDENetworkWiFiAPFlags::GroupWEP104;
	}
	if (nm & NM_ACCESS_POINT_SEC_GROUP_TKIP) {
		ret |= TDENetworkWiFiAPFlags::GroupTKIP;
	}
	if (nm & NM_ACCESS_POINT_SEC_GROUP_CCMP) {
		ret |= TDENetworkWiFiAPFlags::GroupCCMP;
	}
	if (nm & NM_ACCESS_POINT_SEC_KEY_MGMT_PSK) {
		ret |= TDENetworkWiFiAPFlags::KeyManagementPSK;
	}
	if (nm & NM_ACCESS_POINT_SEC_KEY_MGMT_802_1X) {
		ret |= TDENetworkWiFiAPFlags::KeyManagement80211;
	}

	return ret;
}

unsigned int tdeAPSecFlagsToNMAPGenSecFlags(TDENetworkWiFiAPFlags::TDENetworkWiFiAPFlags flags) {
	unsigned int ret = 0;

	if (flags & TDENetworkWiFiAPFlags::PrivacySupport) {
		ret |= NM_ACCESS_POINT_CAP_PRIVACY;
	}

	return ret;
}

unsigned int tdeAPSecFlagsToNMAPSecFlags(TDENetworkWiFiAPFlags::TDENetworkWiFiAPFlags flags) {
	unsigned int ret = 0;

	if (flags & TDENetworkWiFiAPFlags::PairWEP40) {
		ret |= NM_ACCESS_POINT_SEC_PAIR_WEP40;
	}
	if (flags & TDENetworkWiFiAPFlags::PairWEP104) {
		ret |= NM_ACCESS_POINT_SEC_PAIR_WEP104;
	}
	if (flags & TDENetworkWiFiAPFlags::PairTKIP) {
		ret |= NM_ACCESS_POINT_SEC_PAIR_TKIP;
	}
	if (flags & TDENetworkWiFiAPFlags::PairCCMP) {
		ret |= NM_ACCESS_POINT_SEC_PAIR_CCMP;
	}
	if (flags & TDENetworkWiFiAPFlags::GroupWEP40) {
		ret |= NM_ACCESS_POINT_SEC_GROUP_WEP40;
	}
	if (flags & TDENetworkWiFiAPFlags::GroupWEP104) {
		ret |= NM_ACCESS_POINT_SEC_GROUP_WEP104;
	}
	if (flags & TDENetworkWiFiAPFlags::GroupTKIP) {
		ret |= NM_ACCESS_POINT_SEC_GROUP_TKIP;
	}
	if (flags & TDENetworkWiFiAPFlags::GroupCCMP) {
		ret |= NM_ACCESS_POINT_SEC_GROUP_CCMP;
	}
	if (flags & TDENetworkWiFiAPFlags::KeyManagementPSK) {
		ret |= NM_ACCESS_POINT_SEC_KEY_MGMT_PSK;
	}
	if (flags & TDENetworkWiFiAPFlags::KeyManagement80211) {
		ret |= NM_ACCESS_POINT_SEC_KEY_MGMT_802_1X;
	}

	return ret;
}

TDENetworkInfinibandTransportMode::TDENetworkInfinibandTransportMode nmIBTransportToTDEIBTransport(TQString nm) {
	TDENetworkInfinibandTransportMode::TDENetworkInfinibandTransportMode ret = TDENetworkInfinibandTransportMode::Other;

	if (nm.lower() == "datagram") {
		ret = TDENetworkInfinibandTransportMode::Datagram;
	}
	else if (nm.lower() == "connected") {
		ret = TDENetworkInfinibandTransportMode::Connected;
	}

	return ret;
}

TQString tdeIBTransportToNMIBTransport(TDENetworkInfinibandTransportMode::TDENetworkInfinibandTransportMode mode) {
	TQString ret;

	if (mode == TDENetworkInfinibandTransportMode::Datagram) {
		ret = "datagram";
	}
	else if (mode == TDENetworkInfinibandTransportMode::Connected) {
		ret = "connected";
	}

	return ret;
}

TQString TDENetworkConnectionManager_BackendNM::deviceInterfaceString(TQString macAddress) {
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
				TDENetworkDeviceType::TDENetworkDeviceType deviceType = nmDeviceTypeToTDEDeviceType(genericDevice.getDeviceType(error));
				if (error.isValid()) {
					// Error!
					PRINT_ERROR(error.name())
					break;
				}
				else if (deviceType == TDENetworkDeviceType::WiredEthernet) {
					DBus::EthernetDeviceProxy ethernetDevice(NM_DBUS_SERVICE, (*it));
					ethernetDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = ethernetDevice.getPermHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == TDENetworkDeviceType::Infiniband) {
					DBus::InfinibandDeviceProxy infinibandDevice(NM_DBUS_SERVICE, (*it));
					infinibandDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = infinibandDevice.getHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == TDENetworkDeviceType::WiFi) {
					DBus::WiFiDeviceProxy wiFiDevice(NM_DBUS_SERVICE, (*it));
					wiFiDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = wiFiDevice.getPermHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == TDENetworkDeviceType::WiMax) {
					DBus::WiMaxDeviceProxy wiMaxDevice(NM_DBUS_SERVICE, (*it));
					wiMaxDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = wiMaxDevice.getHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == TDENetworkDeviceType::OLPCMesh) {
					DBus::OlpcMeshDeviceProxy olpcMeshDevice(NM_DBUS_SERVICE, (*it));
					olpcMeshDevice.setConnection(TQT_DBusConnection::systemBus());
					TQString candidateMACAddress = olpcMeshDevice.getHwAddress(error);
					if (!error.isValid()) {
						if (candidateMACAddress.lower() == macAddress.lower()) {
							return (*it);
						}
					}
				}
				else if (deviceType == TDENetworkDeviceType::Bluetooth) {
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
			PRINT_ERROR(error.name())
			return "";
		}
	}
	else {
		return "";
	}
}

TDENetworkConnectionManager_BackendNM::TDENetworkConnectionManager_BackendNM(TQString macAddress) : TDENetworkConnectionManager(macAddress) {
	d = new TDENetworkConnectionManager_BackendNMPrivate(this);

	// Set up proxy interfaces
	d->m_networkManagerProxy = new DBus::NetworkManagerProxy(NM_DBUS_SERVICE, NM_DBUS_PATH);
	d->m_networkManagerProxy->setConnection(TQT_DBusConnection::systemBus());
	d->m_networkManagerSettings = new DBus::SettingsInterface(NM_DBUS_SERVICE, NM_DBUS_PATH_SETTINGS);
	d->m_networkManagerSettings->setConnection(TQT_DBusConnection::systemBus());

	TQString dbusDeviceString = deviceInterfaceString(macAddress);
	if (dbusDeviceString != "") {
		d->m_networkDeviceProxy = new DBus::DeviceProxy(NM_DBUS_SERVICE, dbusDeviceString);
		d->m_networkDeviceProxy->setConnection(TQT_DBusConnection::systemBus());
		if (deviceType() == TDENetworkDeviceType::WiFi) {
			d->m_wiFiDeviceProxy = new DBus::WiFiDeviceProxy(NM_DBUS_SERVICE, dbusDeviceString);
			d->m_wiFiDeviceProxy->setConnection(TQT_DBusConnection::systemBus());
		}
	}

	// Connect global signals
	connect(d->m_networkManagerProxy, SIGNAL(StateChanged(TQ_UINT32)), d, SLOT(internalProcessGlobalStateChanged(TQ_UINT32)));

	// Connect local signals
	if (d->m_networkDeviceProxy) {
		connect(d->m_networkDeviceProxy, SIGNAL(StateChanged(TQ_UINT32, TQ_UINT32, TQ_UINT32)), d, SLOT(internalProcessDeviceStateChanged(TQ_UINT32, TQ_UINT32, TQ_UINT32)));
	}
	if (d->m_wiFiDeviceProxy) {
		connect(d->m_wiFiDeviceProxy, SIGNAL(AccessPointAdded(const TQT_DBusObjectPath&)), d, SLOT(internalProcessWiFiAccessPointAdded(const TQT_DBusObjectPath&)));
		connect(d->m_wiFiDeviceProxy, SIGNAL(AccessPointRemoved(const TQT_DBusObjectPath&)), d, SLOT(internalProcessWiFiAccessPointRemoved(const TQT_DBusObjectPath&)));
	}

	// Create public lists
	m_connectionList = new TDENetworkConnectionList;
	m_hwNeighborList = new TDENetworkHWNeighborList;
}

TDENetworkConnectionManager_BackendNM::~TDENetworkConnectionManager_BackendNM() {
	// Destroy public lists
	clearTDENetworkConnectionList();
	delete m_connectionList;
	clearTDENetworkHWNeighborList();
	delete m_hwNeighborList;

	// Tear down proxy interfaces
	if (d->m_networkManagerProxy) delete d->m_networkManagerProxy;
	if (d->m_networkManagerSettings) delete d->m_networkManagerSettings;
	if (d->m_networkDeviceProxy) delete d->m_networkDeviceProxy;

	delete d;
}

void TDENetworkConnectionManager_BackendNMPrivate::internalProcessGlobalStateChanged(TQ_UINT32 state) {
	m_parent->internalNetworkConnectionStateChanged(nmGlobalStateToTDEGlobalState(state));
}

void TDENetworkConnectionManager_BackendNMPrivate::internalProcessDeviceStateChanged(TQ_UINT32 newState, TQ_UINT32 oldState, TQ_UINT32 reason) {
	Q_UNUSED(oldState)
	Q_UNUSED(reason)
	m_parent->internalNetworkDeviceStateChanged(nmDeviceStateToTDEDeviceState(newState), m_parent->m_macAddress);
}

void TDENetworkConnectionManager_BackendNMPrivate::internalProcessWiFiAccessPointAdded(const TQT_DBusObjectPath& dbuspath) {
	TDENetworkWiFiAPInfo* apInfo = m_parent->getAccessPointDetails(dbuspath);
	if (apInfo) {
		m_parent->internalAccessPointStatusChanged(apInfo->BSSID, TDENetworkAPEventType::Discovered);
		delete apInfo;
	}
}

void TDENetworkConnectionManager_BackendNMPrivate::internalProcessWiFiAccessPointRemoved(const TQT_DBusObjectPath& dbuspath) {
	TDENetworkWiFiAPInfo* apInfo = m_parent->getAccessPointDetails(dbuspath);
	if (apInfo) {
		m_parent->internalAccessPointStatusChanged(apInfo->BSSID, TDENetworkAPEventType::Lost);
		delete apInfo;
	}
}

// FIXME
// If access point strength changes, this must be called:
// m_parent->internalAccessPointStatusChanged(apInfo->BSSID, TDENetworkAPEventType::Lost);
// How do I get NetworkManager to notify me when an access point changes strength?  Do I have to poll it for this information?

TDENetworkDeviceType::TDENetworkDeviceType TDENetworkConnectionManager_BackendNM::deviceType() {
	if (m_macAddress == "") {
		return TDENetworkDeviceType::BackendOnly;
	}
	else {
		// Query NM for the device type
		TQT_DBusError error;
		TQString dbusDeviceString = deviceInterfaceString(m_macAddress);
		DBus::DeviceProxy genericDevice(NM_DBUS_SERVICE, dbusDeviceString);
		genericDevice.setConnection(TQT_DBusConnection::systemBus());
		TDENetworkDeviceType::TDENetworkDeviceType ret = nmDeviceTypeToTDEDeviceType(genericDevice.getDeviceType(error));
		if (error.isValid()) {
			// Error!
			PRINT_ERROR(error.name())
			return TDENetworkDeviceType::Other;
		}
		else {
			return ret;
		}
	}
}

TDENetworkConnectionType::TDENetworkConnectionType TDENetworkConnectionManager_BackendNM::connectionType(TQString dbusPath) {
	TDENetworkConnectionType::TDENetworkConnectionType connType = TDENetworkConnectionType::Other;
	TQ_UINT32 ret;
	TQT_DBusError error;

	// Obtain connection settings from the path specified
	DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, dbusPath);
	connectionSettings.setConnection(TQT_DBusConnection::systemBus());
	connect(&connectionSettings, SIGNAL(GetSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)), d, SLOT(processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)));
	int asyncCallID;
	ret = connectionSettings.GetSettingsAsync(asyncCallID, error);
	if (ret && error.isValid()) {
		ret = 0;
		PRINT_ERROR(error.name())
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
		d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
		d->nmConnectionSettingsAsyncSettingsResponse.remove(asyncCallID);

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

TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags TDENetworkConnectionManager_BackendNM::backendStatus() {
	if (d->m_networkManagerProxy) {
		TQ_UINT32 ret;
		TQT_DBusError error;
		ret = d->m_networkManagerProxy->getState(error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR(error.name())
			return TDENetworkGlobalManagerFlags::BackendUnavailable;
		}
		else {
			return nmGlobalStateToTDEGlobalState(ret);
		}
	}
	else {
		return TDENetworkGlobalManagerFlags::BackendUnavailable;
	}
}

TDENetworkDeviceInformation TDENetworkConnectionManager_BackendNM::deviceInformation() {
	TQT_DBusError error;
	TDENetworkDeviceInformation ret;

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

		// Populate wiFiInfo
		if ((deviceType() == TDENetworkDeviceType::WiFi) && (d->m_wiFiDeviceProxy)) {
			ret.wiFiInfo.valid = true;
			ret.wiFiInfo.hwAddress.fromString(d->m_wiFiDeviceProxy->getHwAddress(error));
			ret.wiFiInfo.permanentHWAddress.fromString(d->m_wiFiDeviceProxy->getPermHwAddress(error));
			ret.wiFiInfo.operatingMode = nmWiFiModeToTDEWiFiMode(d->m_wiFiDeviceProxy->getMode(error));
			ret.wiFiInfo.bitrate = d->m_wiFiDeviceProxy->getBitrate(error);
			TDENetworkWiFiAPInfo* apInfo = getAccessPointDetails(d->m_wiFiDeviceProxy->getActiveAccessPoint(error));
			if (apInfo) {
				ret.wiFiInfo.activeAccessPointBSSID = apInfo->BSSID;
				delete apInfo;
			}
			else {
				ret.wiFiInfo.activeAccessPointBSSID = TDEMACAddress();
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
			if (!error.isValid()) {
				ret.activeConnectionUUID = TQString::null;
			}
		}

		ret.valid = true;
	}

	return ret;
}

void TDENetworkConnectionManager_BackendNMPrivate::processConnectionSettingsAsyncReply(int asyncCallId, const TQT_DBusDataMap<TQString>& settings) {
	nmConnectionSettingsAsyncCallWaiting[asyncCallId] = false;
	nmConnectionSettingsAsyncSettingsResponse[asyncCallId] = settings;
}

void TDENetworkConnectionManager_BackendNMPrivate::processConnectionSettingsUpdateAsyncReply(int asyncCallId) {
	nmConnectionSettingsAsyncCallWaiting[asyncCallId] = false;
}

void TDENetworkConnectionManager_BackendNMPrivate::processAddConnectionAsyncReply(int asyncCallId, const TQT_DBusObjectPath& path) {
	nmConnectionSettingsAsyncCallWaiting[asyncCallId] = false;
	nmAddConnectionAsyncResponse[asyncCallId] = path;
}

void TDENetworkConnectionManager_BackendNM::loadConnectionInformation() {
	TDEMACAddress deviceMACAddress;
	deviceMACAddress.fromString(m_macAddress);

	if (d->m_networkManagerSettings) {
		clearTDENetworkConnectionList();
		TQT_DBusObjectPathList connections;
		TQT_DBusError error;
		bool ret;
		int state;
		ret = d->m_networkManagerSettings->ListConnections(connections, error);
		if (ret) {
			TQT_DBusObjectPathList::iterator it;
			for (it = connections.begin(); it != connections.end(); ++it) {
				TDENetworkConnection* connection;
				TDEWiredEthernetConnection* ethernetConnection = NULL;
				TDEWiredInfinibandConnection* infinibandConnection = NULL;
				TDEWiFiConnection* wiFiConnection = NULL;
				TDEVPNConnection* vpnConnection = NULL;
				TDEWiMaxConnection* wiMaxConnection = NULL;
				TDEVLANConnection* vlanConnection = NULL;
				TDEOLPCMeshConnection* olpcMeshConnection = NULL;
				TDEBluetoothConnection* bluetoothConnection = NULL;
				TDEModemConnection* modemConnection = NULL;
				TDENetworkConnectionType::TDENetworkConnectionType connType = connectionType((*it));
				if (connType == TDENetworkConnectionType::WiredEthernet) {
					connection = ethernetConnection = new TDEWiredEthernetConnection;
				}
				else if (connType == TDENetworkConnectionType::Infiniband) {
					connection = infinibandConnection = new TDEWiredInfinibandConnection;
				}
				else if (connType == TDENetworkConnectionType::WiFi) {
					connection = wiFiConnection = new TDEWiFiConnection;
				}
				else if (connType == TDENetworkConnectionType::VPN) {
					connection = vpnConnection = new TDEVPNConnection;
				}
				else if (connType == TDENetworkConnectionType::WiMax) {
					connection = wiMaxConnection = new TDEWiMaxConnection;
				}
				else if (connType == TDENetworkConnectionType::VLAN) {
					connection = vlanConnection = new TDEVLANConnection;
				}
				else if (connType == TDENetworkConnectionType::OLPCMesh) {
					connection = olpcMeshConnection = new TDEOLPCMeshConnection;
				}
				else if (connType == TDENetworkConnectionType::Bluetooth) {
					connection = bluetoothConnection = new TDEBluetoothConnection;
				}
				else if (connType == TDENetworkConnectionType::Modem) {
					connection = modemConnection = new TDEModemConnection;
				}
				else {
					connection = new TDENetworkConnection;
				}
				// Set up defaults
				connection->ipConfig.connectionFlags =	TDENetworkIPConfigurationFlags::IPV4DHCPIP			| \
									TDENetworkIPConfigurationFlags::IPV4DHCPDNS			| \
									TDENetworkIPConfigurationFlags::IPV4DHCPRoutes			| \
									TDENetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute	| \
									TDENetworkIPConfigurationFlags::IPV6DHCPIP			| \
									TDENetworkIPConfigurationFlags::IPV6DHCPDNS			| \
									TDENetworkIPConfigurationFlags::IPV6DHCPRoutes			| \
									TDENetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute;

#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
				printf("[network-manager comm debug] %s\n\r", (*it).data()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS

				// Obtain connection settings from the path specified
				DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, (*it));
				connectionSettings.setConnection(TQT_DBusConnection::systemBus());
				connect(&connectionSettings, SIGNAL(GetSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)), d, SLOT(processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)));
				int asyncCallID;
				ret = connectionSettings.GetSettingsAsync(asyncCallID, error);
				if (ret && error.isValid()) {
					ret = 0;
					PRINT_ERROR(error.name())
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
					d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
					d->nmConnectionSettingsAsyncSettingsResponse.remove(asyncCallID);

#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
					printf("[network-manager comm debug] received DBUS object structure map follows:\n\r"); fflush(stdout);
					printDBUSObjectStructure(TQT_DBusData::fromStringKeyMap(connectionSettingsMap));
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS

					// Parse settings
					TQT_DBusTQStringDataMap::const_iterator it2;
					for (it2 = connectionSettingsMap.begin(); it2 != connectionSettingsMap.end(); ++it2) {
						TQString outerKeyValue = it2.key();
						TQT_DBusData dataValue = it2.data();
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
						printf("[network-manager comm debug] [%s]\n\r", outerKeyValue.ascii()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
						TQT_DBusTQStringDataMap nestedConnectionSettingsMap = dataValue.toStringKeyMap();
						TQT_DBusTQStringDataMap::const_iterator it3;
						for (it3 = nestedConnectionSettingsMap.begin(); it3 != nestedConnectionSettingsMap.end(); ++it3) {
							TQString keyValue = it3.key();
							TQT_DBusData dataValue = it3.data();
							if (dataValue.type() != TQT_DBusData::Variant) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
								printf("[network-manager comm debug] %s = %s (type %d(%s))\n\r", keyValue.ascii(), dataValue.toString().ascii(), dataValue.type(), dataValue.typeName()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
								// No NM settings are known which use this style
							}
							else {
								TQT_DBusVariant dataValueVariant = dataValue.toVariant();
								TQT_DBusData dataValue2 = dataValueVariant.value;
								if (dataValue2.type() != TQT_DBusData::Variant) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
									printf("[network-manager comm debug] %s = %s (type %d(%s), signature %s)\n\r", keyValue.ascii(), dataValue2.toString().ascii(), dataValue2.type(), dataValue2.typeName(), dataValueVariant.signature.ascii()); fflush(stdout);
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
											TDENetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->lockedHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "cloned-mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											TDENetworkByteList macAddress;
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
											TDENetworkByteList macAddress;
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
											TDENetworkByteList macAddress;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												macAddress.append(innerDataValue.toByte());
											}
											connection->lockedHWAddress.setAddress(macAddress);
										}
										else if (keyValue.lower() == "cloned-mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											TDENetworkByteList macAddress;
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
											wiFiConnection->operatingMode = nmWiFiModeToTDEWiFiMode(dataValue2.toString());
										}
										else if (keyValue.lower() == "band") {
											wiFiConnection->bandRestriction = nmWiFiFrequencyBandToTDEWiFiFrequencyBand(dataValue2.toString());
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
											TDENetworkByteList macAddress;
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
												TDEMACAddress hwAddress;
												hwAddress.fromString(innerDataValue.toString());
												wiFiConnection->blacklistedBSSIDs.append(hwAddress);
											}
										}
										else if (keyValue.lower() == "seen-bssids") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TDEMACAddress hwAddress;
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
											if ((wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherWEP40))
												|| (wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherWEP104))
												|| (wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherTKIP))
												|| (wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherCCMP))) {
												wiFiConnection->securitySettings.allowedPairWiseCiphers.append(TDENetworkWiFiConnectionCipher::Any);
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
											if ((wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherWEP40))
												|| (wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherWEP104))
												|| (wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherTKIP))
												|| (wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherCCMP))) {
												wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(TDENetworkWiFiConnectionCipher::Any);
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
											vpnConnection->vpnPluginID = dataValue2.toString();
										}
										else if (keyValue.lower() == "user-name") {
											vpnConnection->lockedUserName = dataValue2.toString();
										}
										else if (keyValue.lower() == "data") {
											TQT_DBusTQStringDataMap nestedConnectionSettingsMap = dataValue2.toStringKeyMap();
											TQT_DBusTQStringDataMap::const_iterator it4;
											for (it3 = nestedConnectionSettingsMap.begin(); it4 != nestedConnectionSettingsMap.end(); ++it4) {
												TQString keyValue4 = it4.key();
												TQT_DBusData dataValue4 = it4.data();
												if (dataValue4.type() == TQT_DBusData::String) {
													vpnConnection->pluginData[keyValue] = dataValue4.toString();
												}
											}
										}
									}
									else if (outerKeyValue.lower() == "wimax") {
										if (keyValue.lower() == "mac-address") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											TDENetworkByteList macAddress;
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
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::DisableEAP;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::DisableEAP;
										}
										else if (keyValue.lower() == "refuse-pap") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::DisablePAP;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::DisablePAP;
										}
										else if (keyValue.lower() == "refuse-chap") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::DisableCHAP;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::DisableCHAP;
										}
										else if (keyValue.lower() == "refuse-mschap") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::DisableMSCHAP;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::DisableMSCHAP;
										}
										else if (keyValue.lower() == "refuse-mschapv2") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::DisableMSCHAPv2;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::DisableMSCHAPv2;
										}
										else if (keyValue.lower() == "nobsdcomp") {
											if (dataValue2.toBool())	connection->pppConfig.flags &= ~TDENetworkPPPFlags::AllowBSDCompression;
											else				connection->pppConfig.flags |=  TDENetworkPPPFlags::AllowBSDCompression;
										}
										else if (keyValue.lower() == "nodeflate") {
											if (dataValue2.toBool())	connection->pppConfig.flags &= ~TDENetworkPPPFlags::AllowDeflateCompression;
											else				connection->pppConfig.flags |=  TDENetworkPPPFlags::AllowDeflateCompression;
										}
										else if (keyValue.lower() == "no-vj-comp") {
											if (dataValue2.toBool())	connection->pppConfig.flags &= ~TDENetworkPPPFlags::AllowVJCompression;
											else				connection->pppConfig.flags |=  TDENetworkPPPFlags::AllowVJCompression;
										}
										else if (keyValue.lower() == "require-mppe") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::RequireMPPE;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::RequireMPPE;
										}
										else if (keyValue.lower() == "require-mppe-128") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::RequireMPPE128;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::RequireMPPE128;
										}
										else if (keyValue.lower() == "mppe-stateful") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::StatefulMPPE;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::StatefulMPPE;
										}
										else if (keyValue.lower() == "crtscts") {
											if (dataValue2.toBool())	connection->pppConfig.flags |=  TDENetworkPPPFlags::UseHardwareFlowControl;
											else				connection->pppConfig.flags &= ~TDENetworkPPPFlags::UseHardwareFlowControl;
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
											TDENetworkByteList macAddress;
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
										modemConnection->type = TDEModemConnectionType::CDMA;
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
										modemConnection->type = TDEModemConnectionType::GSM;
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
												TDENetworkSingleIPConfiguration ipConfig;
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
												connection->ipConfig.searchDomains.append(TDENetworkSearchDomain(innerDataValue.toString(), false));
											}
										}
										else if (keyValue.lower() == "ignore-auto-dns") {
											bool nm_static_dns = dataValue2.toBool();
											if (nm_static_dns) {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV4DHCPDNS;
											}
											else {
												connection->ipConfig.connectionFlags |= TDENetworkIPConfigurationFlags::IPV4DHCPDNS;
											}
										}
										else if (keyValue.lower() == "may-fail") {
											bool nm_may_fail = dataValue2.toBool();
											connection->requireIPV4 = !nm_may_fail;
										}
										else if (keyValue.lower() == "method") {
											TQString nm_method = dataValue2.toString().lower();
											if (nm_method == "auto") {
												connection->ipConfig.connectionFlags |= TDENetworkIPConfigurationFlags::IPV4DHCPIP;
											}
											else if (nm_method == "manual") {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV4DHCPIP;
											}
											else if (nm_method == "link-local") {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV4LocalOnly;
											}
											else if (nm_method == "shared") {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV4StartConnectionSharingServer;
											}
											else if (nm_method == "disabled") {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV4Disabled;
											}
										}
										else if (keyValue.lower() == "ignore-auto-routes") {
											bool nm_static_routes = dataValue2.toBool();
											if (nm_static_routes) {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV4DHCPRoutes;
											}
											else {
												connection->ipConfig.connectionFlags |= TDENetworkIPConfigurationFlags::IPV4DHCPRoutes;
											}
										}
										else if (keyValue.lower() == "never-default") {
											bool nm_can_default_route = !dataValue2.toBool();
											if (nm_can_default_route) {
												connection->ipConfig.connectionFlags |= TDENetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute;
											}
											else {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute;
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
												TDENetworkSingleRouteConfiguration routeConfig;
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
												TDENetworkSingleIPConfiguration ipConfig;
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
												connection->ipConfig.searchDomains.append(TDENetworkSearchDomain(innerDataValue.toString(), true));
											}
										}
										else if (keyValue.lower() == "ignore-auto-dns") {
											bool nm_static_dns = dataValue2.toBool();
											if (nm_static_dns) {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV4DHCPDNS;
											}
											else {
												connection->ipConfig.connectionFlags |= TDENetworkIPConfigurationFlags::IPV4DHCPDNS;
											}
										}
										else if (keyValue.lower() == "may-fail") {
											bool nm_may_fail = dataValue2.toBool();
											connection->requireIPV6 = !nm_may_fail;
										}
										else if (keyValue.lower() == "method") {
											TQString nm_method = dataValue2.toString().lower();
											if (nm_method == "auto") {
												connection->ipConfig.connectionFlags |= TDENetworkIPConfigurationFlags::IPV6DHCPIP;
											}
											else if (nm_method == "manual") {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV6DHCPIP;
											}
											else if (nm_method == "link-local") {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV6LocalOnly;
											}
											else if (nm_method == "shared") {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV6StartConnectionSharingServer;
											}
											else if (nm_method == "ignore") {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV6Disabled;
											}
										}
										else if (keyValue.lower() == "ignore-auto-routes") {
											bool nm_static_routes = dataValue2.toBool();
											if (nm_static_routes) {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV6DHCPRoutes;
											}
											else {
												connection->ipConfig.connectionFlags |= TDENetworkIPConfigurationFlags::IPV6DHCPRoutes;
											}
										}
										else if (keyValue.lower() == "never-default") {
											bool nm_can_default_route = !dataValue2.toBool();
											if (nm_can_default_route) {
												connection->ipConfig.connectionFlags |= TDENetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute;
											}
											else {
												connection->ipConfig.connectionFlags &= ~TDENetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute;
											}
										}
										else if (keyValue.lower() == "routes") {
											TQT_DBusDataValueList valueList = dataValue2.toTQValueList();
											TQT_DBusDataValueList::const_iterator it4;
											for (it4 = valueList.begin(); it4 != valueList.end(); ++it4) {
												TQT_DBusData innerDataValue = *it4;
												TQT_DBusDataValueList innerValueList = innerDataValue.toStruct();
												TQT_DBusDataValueList::const_iterator it5;
												TDENetworkSingleRouteConfiguration routeConfig;
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
					PRINT_ERROR(error.name())
				}
			}
		}
		else {
			// Error!
			PRINT_ERROR(error.name())
		}
		internalNetworkManagementEvent(TDENetworkGlobalEventType::ConnectionListChanged);
	}
}

void TDENetworkConnectionManager_BackendNM::loadConnectionAllowedValues(TDENetworkConnection* connection) {
	if (connection) {
		// Insert all allowed EAP phase 2 methods
		connection->eapConfig.allowedPhase2NonEAPMethods.clear();
		connection->eapConfig.allowedPhase2NonEAPMethods.append(TDENetworkIEEE8021xType::MD5);
		connection->eapConfig.allowedPhase2NonEAPMethods.append(TDENetworkIEEE8021xType::MSCHAPV2);
		connection->eapConfig.allowedPhase2NonEAPMethods.append(TDENetworkIEEE8021xType::OTP);
		connection->eapConfig.allowedPhase2NonEAPMethods.append(TDENetworkIEEE8021xType::GTC);
		connection->eapConfig.allowedPhase2NonEAPMethods.append(TDENetworkIEEE8021xType::TLS);

		connection->eapConfig.allowedPhase2EAPMethods.clear();
		connection->eapConfig.allowedPhase2EAPMethods.append(TDENetworkIEEE8021xType::PAP);
		connection->eapConfig.allowedPhase2EAPMethods.append(TDENetworkIEEE8021xType::CHAP);
		connection->eapConfig.allowedPhase2EAPMethods.append(TDENetworkIEEE8021xType::MSCHAP);
		connection->eapConfig.allowedPhase2EAPMethods.append(TDENetworkIEEE8021xType::MSCHAPV2);
		connection->eapConfig.allowedPhase2EAPMethods.append(TDENetworkIEEE8021xType::OTP);
		connection->eapConfig.allowedPhase2EAPMethods.append(TDENetworkIEEE8021xType::GTC);
		connection->eapConfig.allowedPhase2EAPMethods.append(TDENetworkIEEE8021xType::TLS);

		connection->eapConfig.allowedValid = true;
	}
}

// NOTE
// While this separate separate routine is needed to get the secrets, note that secrets must
// be saved using the same connection map save routine that all other settings use above.
bool TDENetworkConnectionManager_BackendNM::loadConnectionSecrets(TQString uuid) {
	bool ret = TRUE;
	ret = ret && loadConnectionSecretsForGroup(uuid, "802-1x");
	return ret;
}

bool TDENetworkConnectionManager_BackendNM::loadConnectionSecretsForGroup(TQString uuid, TQString group) {
	TDENetworkConnection* connection = findConnectionByUUID(uuid);
	if (!connection) {
		PRINT_ERROR(TQString("Unable to locate connection with uuid '%1' in local database.  Did you run loadConnectionInformation() first?"));
		return FALSE;
	}
	//TDEWiredEthernetConnection* ethernetConnection = dynamic_cast<TDEWiredEthernetConnection*>(connection);
	//TDEWiredInfinibandConnection* infinibandConnection = dynamic_cast<TDEWiredInfinibandConnection*>(connection);
	TDEWiFiConnection* wiFiConnection = dynamic_cast<TDEWiFiConnection*>(connection);
	TDEVPNConnection* vpnConnection = dynamic_cast<TDEVPNConnection*>(connection);
	//TDEWiMaxConnection* wiMaxConnection = dynamic_cast<TDEWiMaxConnection*>(connection);
	//TDEVLANConnection* vlanConnection = dynamic_cast<TDEVLANConnection*>(connection);
	//TDEOLPCMeshConnection* olpcMeshConnection = dynamic_cast<TDEVLANConnection*>(connection);
	//TDEBluetoothConnection* bluetoothConnection = dynamic_cast<TDEBluetoothConnection*>(connection);
	TDEModemConnection* modemConnection = dynamic_cast<TDEModemConnection*>(connection);
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool ret;
	TQT_DBusTQStringDataMap connectionSecretsMap(TQT_DBusData::String);
	ret = d->m_networkManagerSettings->GetConnectionByUuid(uuid, existingConnection, error);
	if (ret) {
		// Obtain connection secrets from the path specified
		DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
		connectionSettings.setConnection(TQT_DBusConnection::systemBus());
		connect(&connectionSettings, SIGNAL(GetSecretsAsyncReply(int, const TQT_DBusDataMap<TQString>&)), d, SLOT(processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)));
		int asyncCallID;
		ret = connectionSettings.GetSecretsAsync(asyncCallID, group, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR(error.name())
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
			connectionSecretsMap = d->nmConnectionSettingsAsyncSettingsResponse[asyncCallID];
			d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
			d->nmConnectionSettingsAsyncSettingsResponse.remove(asyncCallID);

#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
			printf("[network-manager comm debug] received DBUS object structure map follows:\n\r"); fflush(stdout);
			printDBUSObjectStructure(TQT_DBusData::fromStringKeyMap(connectionSecretsMap));
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS

			// Parse settings
			TQT_DBusTQStringDataMap::const_iterator it2;
			for (it2 = connectionSecretsMap.begin(); it2 != connectionSecretsMap.end(); ++it2) {
				TQString outerKeyValue = it2.key();
				TQT_DBusData dataValue = it2.data();
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
				printf("[network-manager comm debug] [%s]\n\r", outerKeyValue.ascii()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
				TQT_DBusTQStringDataMap nestedConnectionSettingsMap = dataValue.toStringKeyMap();
				TQT_DBusTQStringDataMap::const_iterator it3;
				for (it3 = nestedConnectionSettingsMap.begin(); it3 != nestedConnectionSettingsMap.end(); ++it3) {
					TQString keyValue = it3.key();
					TQT_DBusData dataValue = it3.data();
					if (dataValue.type() != TQT_DBusData::Variant) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
						printf("[network-manager comm debug] %s = %s (type %d(%s))\n\r", keyValue.ascii(), dataValue.toString().ascii(), dataValue.type(), dataValue.typeName()); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
						// No NM settings are known which use this style
					}
					else {
						TQT_DBusVariant dataValueVariant = dataValue.toVariant();
						TQT_DBusData dataValue2 = dataValueVariant.value;
						if (dataValue2.type() != TQT_DBusData::Variant) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
							printf("[network-manager comm debug] %s = %s (type %d(%s), signature %s)\n\r", keyValue.ascii(), dataValue2.toString().ascii(), dataValue2.type(), dataValue2.typeName(), dataValueVariant.signature.ascii()); fflush(stdout);
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
								}
								else if (keyValue.lower() == "wep-key1") {
									wiFiConnection->securitySettings.wepKey1 = dataValue2.toString();
								}
								else if (keyValue.lower() == "wep-key2") {
									wiFiConnection->securitySettings.wepKey2 = dataValue2.toString();
								}
								else if (keyValue.lower() == "wep-key3") {
									wiFiConnection->securitySettings.wepKey3 = dataValue2.toString();
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
									for (it3 = nestedConnectionSettingsMap.begin(); it4 != nestedConnectionSettingsMap.end(); ++it4) {
										TQString keyValue4 = it4.key();
										TQT_DBusData dataValue4 = it4.data();
										if (dataValue4.type() == TQT_DBusData::String) {
											vpnConnection->pluginSecrets[keyValue] = dataValue4.toString();
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
		PRINT_ERROR(TQString("connection for provided uuid '%1' was not found").arg(uuid));
		return FALSE;
	}
}

bool TDENetworkConnectionManager_BackendNM::saveConnection(TDENetworkConnection* connection) {
	// Find path for connection with specified UUID, if it exists
	// This is so that any settings that we are not aware of can be loaded now and preserved through the update operation
	if (!connection) {
		PRINT_ERROR(TQString("connection cannot be NULL!"));
		return FALSE;
	}
	TDEWiredEthernetConnection* ethernetConnection = dynamic_cast<TDEWiredEthernetConnection*>(connection);
	TDEWiredInfinibandConnection* infinibandConnection = dynamic_cast<TDEWiredInfinibandConnection*>(connection);
	TDEWiFiConnection* wiFiConnection = dynamic_cast<TDEWiFiConnection*>(connection);
	TDEVPNConnection* vpnConnection = dynamic_cast<TDEVPNConnection*>(connection);
	TDEWiMaxConnection* wiMaxConnection = dynamic_cast<TDEWiMaxConnection*>(connection);
	TDEVLANConnection* vlanConnection = dynamic_cast<TDEVLANConnection*>(connection);
	TDEOLPCMeshConnection* olpcMeshConnection = dynamic_cast<TDEOLPCMeshConnection*>(connection);
	TDEBluetoothConnection* bluetoothConnection = dynamic_cast<TDEBluetoothConnection*>(connection);
	TDEModemConnection* modemConnection = dynamic_cast<TDEModemConnection*>(connection);
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool ret;
	bool existing;
	TQT_DBusTQStringDataMap connectionSettingsMap(TQT_DBusData::String);
	existing = false;
	ret = d->m_networkManagerSettings->GetConnectionByUuid(connection->UUID, existingConnection, error);
	if (ret) {
		// Obtain connection settings from the path specified
		DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
		connectionSettings.setConnection(TQT_DBusConnection::systemBus());
		connect(&connectionSettings, SIGNAL(GetSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)), d, SLOT(processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&)));
		int asyncCallID;
		ret = connectionSettings.GetSettingsAsync(asyncCallID, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR(error.name())
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
			connectionSettingsMap = d->nmConnectionSettingsAsyncSettingsResponse[asyncCallID];
			existing = true;
			d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
			d->nmConnectionSettingsAsyncSettingsResponse.remove(asyncCallID);
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
						if (modemConnection->type == TDEModemConnectionType::CDMA) {
							type = "cdma";
						}
						else if (modemConnection->type == TDEModemConnectionType::GSM) {
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
				settingsMap["master"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(connection->masterConnectionUUID));
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
					TDENetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					TDENetworkByteList::iterator it;
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
					TDENetworkByteList address = connection->manualHWAddress.address();
					TQT_DBusDataValueList valueList;
					TDENetworkByteList::iterator it;
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
					TDENetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					TDENetworkByteList::iterator it;
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
					TDENetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					TDENetworkByteList::iterator it;
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
					TDENetworkByteList address = connection->manualHWAddress.address();
					TQT_DBusDataValueList valueList;
					TDENetworkByteList::iterator it;
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
					TDENetworkByteList address = wiFiConnection->accessPointRestriction.address();
					TQT_DBusDataValueList valueList;
					TDENetworkByteList::iterator it;
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
					TDEMACAddressList::iterator it;
					for (it = wiFiConnection->blacklistedBSSIDs.begin(); it != wiFiConnection->blacklistedBSSIDs.end(); ++it) {
						valueList.append(TQT_DBusData::fromString((*it).toString()));
					}
					if (valueList.count() > 0) settingsMap["mac-address-blacklist"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					TQT_DBusDataValueList valueList;
					TDEMACAddressList::iterator it;
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
					settingsMap["hidden"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(wiFiConnection->isHiddenNetwork));
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
				{
					if (wiFiConnection->securityRequired) {
						settingsMap["key-mgmt"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(tdeWiFiKeyTypeToNMWiFiKeyType(wiFiConnection->securitySettings.keyType)));
					}
					else {
						settingsMap.remove("key-mgmt");
					}
				}
				if (wiFiConnection->securitySettings.wepKeyIndex > 0) {
					settingsMap["wep-tx-keyidx"] = convertDBUSDataToVariantData(TQT_DBusData::fromUInt32(wiFiConnection->securitySettings.wepKeyIndex));
				}
				else {
					settingsMap.remove("wep-tx-keyidx");
				}
				{
					if (wiFiConnection->securityRequired) {
						settingsMap["auth-alg"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(tdeWiFiAuthTypeToNMWiFiAuthType(wiFiConnection->securitySettings.authType)));
					}
					else {
						settingsMap.remove("auth-alg");
					}
				}
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
						if (wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::Any)) {
							if (!wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherWEP40)) wiFiConnection->securitySettings.allowedPairWiseCiphers.append(TDENetworkWiFiConnectionCipher::CipherWEP40);
							if (!wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherWEP104)) wiFiConnection->securitySettings.allowedPairWiseCiphers.append(TDENetworkWiFiConnectionCipher::CipherWEP104);
							if (!wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherTKIP)) wiFiConnection->securitySettings.allowedPairWiseCiphers.append(TDENetworkWiFiConnectionCipher::CipherTKIP);
							if (!wiFiConnection->securitySettings.allowedPairWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherCCMP)) wiFiConnection->securitySettings.allowedPairWiseCiphers.append(TDENetworkWiFiConnectionCipher::CipherCCMP);
						}
						for (TDENetworkWiFiConnectionCipherList::Iterator it = wiFiConnection->securitySettings.allowedPairWiseCiphers.begin(); it != wiFiConnection->securitySettings.allowedPairWiseCiphers.end(); ++it) {
							valueList.append(TQT_DBusData::fromString(tdeWiFiCipherToNMWiFiCipher(*it)));
						}
					}
					if (valueList.count() > 0) settingsMap["pairwise"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
					else settingsMap.remove("pairwise");
				}
				{
					TQT_DBusDataValueList valueList;
					{
						if (wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::Any)) {
							if (!wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherWEP40)) wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(TDENetworkWiFiConnectionCipher::CipherWEP40);
							if (!wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherWEP104)) wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(TDENetworkWiFiConnectionCipher::CipherWEP104);
							if (!wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherTKIP)) wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(TDENetworkWiFiConnectionCipher::CipherTKIP);
							if (!wiFiConnection->securitySettings.allowedGroupWiseCiphers.contains(TDENetworkWiFiConnectionCipher::CipherCCMP)) wiFiConnection->securitySettings.allowedGroupWiseCiphers.append(TDENetworkWiFiConnectionCipher::CipherCCMP);
						}
						for (TDENetworkWiFiConnectionCipherList::Iterator it = wiFiConnection->securitySettings.allowedGroupWiseCiphers.begin(); it != wiFiConnection->securitySettings.allowedGroupWiseCiphers.end(); ++it) {
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
				if (connection->eapConfig.secretsValid) {
					settingsMap["wep-key0"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(wiFiConnection->securitySettings.wepKey0));
					settingsMap["wep-key1"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(wiFiConnection->securitySettings.wepKey1));
					settingsMap["wep-key2"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(wiFiConnection->securitySettings.wepKey2));
					settingsMap["wep-key3"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(wiFiConnection->securitySettings.wepKey3));
					settingsMap["psk"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(wiFiConnection->securitySettings.psk));
					settingsMap["leap-password"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(wiFiConnection->securitySettings.leapPassword));
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
			dbusData = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(settingsMap));
			groupValid = (settingsMap.count() > 0);
		}
		if (groupValid) outerMap.insert("802-11-wireless-security", dbusData, TRUE); else outerMap.remove("802-11-wireless-security");

		groupValid = false;
		dbusData = outerMap["vpn"];
		if (vpnConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				UPDATE_STRING_SETTING_IF_VALID(vpnConnection->vpnPluginID, "service-type", settingsMap)
				UPDATE_STRING_SETTING_IF_VALID(vpnConnection->lockedUserName, "user-name", settingsMap)
				{
					TQMap<TQString, TQT_DBusData> nestedConnectionSettingsMap;
					TDENetworkSettingsMap::const_iterator it;
					for (it = vpnConnection->pluginData.begin(); it != vpnConnection->pluginData.end(); ++it) {
						nestedConnectionSettingsMap[it.key()] = TQT_DBusData::fromString(it.data());
					}
					if (nestedConnectionSettingsMap.count() > 0) settingsMap["data"] = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(nestedConnectionSettingsMap));
					else settingsMap.remove("data");
				}
				if (vpnConnection->secretsValid) {
					TQMap<TQString, TQT_DBusData> nestedConnectionSettingsMap;
					TDENetworkSettingsMap::const_iterator it;
					for (it = vpnConnection->pluginSecrets.begin(); it != vpnConnection->pluginSecrets.end(); ++it) {
						nestedConnectionSettingsMap[it.key()] = TQT_DBusData::fromString(it.data());
					}
					if (nestedConnectionSettingsMap.count() > 0) settingsMap["secrets"] = TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(nestedConnectionSettingsMap));
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
					TDENetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					TDENetworkByteList::iterator it;
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
					TDENetworkPriorityMap::const_iterator it;
					for (it = vlanConnection->ingressPriorityMap.begin(); it != vlanConnection->ingressPriorityMap.end(); ++it) {
						valueList.append(TQT_DBusData::fromString(TQString("%1:%2").arg(it.key()).arg(it.data())));
					}
					if (valueList.count() > 0) settingsMap["ingress-priority-map"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					TQT_DBusDataValueList valueList;
					TDENetworkPriorityMap::const_iterator it;
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
				settingsMap["refuse-eap"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::DisableEAP));
				settingsMap["refuse-pap"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::DisablePAP));
				settingsMap["refuse-chap"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::DisableCHAP));
				settingsMap["refuse-mschap"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::DisableMSCHAP));
				settingsMap["refuse-mschapv2"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::DisableMSCHAPv2));
				settingsMap["nobsdcomp"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->pppConfig.flags & TDENetworkPPPFlags::AllowBSDCompression)));
				settingsMap["nodeflate"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->pppConfig.flags & TDENetworkPPPFlags::AllowDeflateCompression)));
				settingsMap["no-vj-comp"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->pppConfig.flags & TDENetworkPPPFlags::AllowVJCompression)));
				settingsMap["require-mppe"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::RequireMPPE));
				settingsMap["require-mppe-128"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::RequireMPPE128));
				settingsMap["mppe-stateful"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::StatefulMPPE));
				settingsMap["crtscts"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(connection->pppConfig.flags & TDENetworkPPPFlags::UseHardwareFlowControl));
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
		if (olpcMeshConnection) {
			TQMap<TQString, TQT_DBusData> settingsMap = dbusData.toStringKeyMap().toTQMap();
			{
				if (connection->lockedHWAddress.isValid()) {
					TDENetworkByteList address = connection->lockedHWAddress.address();
					TQT_DBusDataValueList valueList;
					TDENetworkByteList::iterator it;
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
		if ((modemConnection) && (modemConnection->type == TDEModemConnectionType::CDMA)) {
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
		if ((modemConnection) && (modemConnection->type == TDEModemConnectionType::GSM)) {
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
					TDENetworkSingleIPConfigurationList::iterator it;
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
					TDENetworkAddressList::iterator it;
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
					TDENetworkSearchDomainList::iterator it;
					for (it = connection->ipConfig.searchDomains.begin(); it != connection->ipConfig.searchDomains.end(); ++it) {
						if ((*it).isIPv4()) {
							valueList.append(TQT_DBusData::fromString((*it).searchDomain()));
						}
					}
					if (valueList.count() > 0) settingsMap["dns-search"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					settingsMap["ignore-auto-dns"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV4DHCPDNS)));
				}
				{
					settingsMap["may-fail"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!connection->requireIPV4));
				}
				{
					TQString method;
					if (connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV4DHCPIP) {
						method = "auto";
					}
					else if (connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV4LocalOnly) {
						method = "link-local";
					}
					else if (connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV4StartConnectionSharingServer) {
						method = "shared";
					}
					else if (connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV4Disabled) {
						method = "disabled";
					}
					else {
						method = "manual";
					}
					if (!method.isNull())
						settingsMap["method"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(method));
				}
				{
					settingsMap["ignore-auto-routes"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV4DHCPRoutes)));
				}
				{
					settingsMap["never-default"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute)));
				}
				{
					TQT_DBusDataValueList valueList;
					TDENetworkSingleRouteConfigurationList::iterator it;
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
					TDENetworkSingleIPConfigurationList::iterator it;
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
					TDENetworkAddressList::iterator it;
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
					TDENetworkSearchDomainList::iterator it;
					for (it = connection->ipConfig.searchDomains.begin(); it != connection->ipConfig.searchDomains.end(); ++it) {
						if ((*it).isIPv6()) {
							valueList.append(TQT_DBusData::fromString((*it).searchDomain()));
						}
					}
					if (valueList.count() > 0) settingsMap["dns-search"] = convertDBUSDataToVariantData(TQT_DBusData::fromTQValueList(valueList));
				}
				{
					settingsMap["ignore-auto-dns"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV4DHCPDNS)));
				}
				{
					settingsMap["may-fail"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!connection->requireIPV6));
				}
				{
					TQString method;
					if (connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV6DHCPIP) {
						method = "auto";
					}
					else if (connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV6LocalOnly) {
						method = "link-local";
					}
					else if (connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV6StartConnectionSharingServer) {
						method = "shared";
					}
					else if (connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV6Disabled) {
						method = "ignore";
					}
					else {
						method = "manual";
					}
					if (!method.isNull())
						settingsMap["method"] = convertDBUSDataToVariantData(TQT_DBusData::fromString(method));
				}
				{
					settingsMap["ignore-auto-routes"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV6DHCPRoutes)));
				}
				{
					settingsMap["never-default"] = convertDBUSDataToVariantData(TQT_DBusData::fromBool(!(connection->ipConfig.connectionFlags & TDENetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute)));
				}
				{
					TQT_DBusDataValueList valueList;
					TDENetworkSingleRouteConfigurationList::iterator it;
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
	printf("[network-manager comm debug] uploaded DBUS object structure map follows:\n\r"); fflush(stdout);
	printDBUSObjectStructure(TQT_DBusData::fromStringKeyMap(connectionSettingsMap));
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS

	if (existing) {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
		printf("[network-manager comm debug] Updating existing connection\n\r"); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
		// Save connection settings to the path specified
		DBus::ConnectionSettingsInterface connectionSettings(NM_DBUS_SERVICE, existingConnection);
		connectionSettings.setConnection(TQT_DBusConnection::systemBus());
		connect(&connectionSettings, SIGNAL(UpdateAsyncReply(int)), d, SLOT(processConnectionSettingsUpdateAsyncReply(int)));
		int asyncCallID;
		ret = connectionSettings.UpdateAsync(asyncCallID, connectionSettingsMap, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR(error.name())
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
			d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
			return TRUE;
		}
		else {
			// Error!
			PRINT_ERROR(error.name())
			return FALSE;
		}
	}
	else {
#ifdef DEBUG_NETWORK_MANAGER_COMMUNICATIONS
		printf("[network-manager comm debug] Creating new connection\n\r"); fflush(stdout);
#endif // DEBUG_NETWORK_MANAGER_COMMUNICATIONS
		// Create new connection
		connect(d->m_networkManagerSettings, SIGNAL(AddConnectionAsyncReply(int, const TQT_DBusObjectPath&)), d, SLOT(processAddConnectionAsyncReply(int, const TQT_DBusObjectPath&)));
		int asyncCallID;
		ret = d->m_networkManagerSettings->AddConnectionAsync(asyncCallID, connectionSettingsMap, error);
		if (ret && error.isValid()) {
			ret = 0;
			PRINT_ERROR(error.name())
		}
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
			d->nmAddConnectionAsyncResponse.remove(asyncCallID);
			return TRUE;
		}
		else {
			// Error!
			PRINT_ERROR(error.name())
			return FALSE;
		}
	}
}

bool TDENetworkConnectionManager_BackendNM::deleteConnection(TQString uuid) {
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
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
				PRINT_ERROR(error.name())
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
				d->nmConnectionSettingsAsyncCallWaiting.remove(asyncCallID);
				return TRUE;
			}
			else {
				PRINT_ERROR(TQString("Unable to remove connection with uuid '%1'").arg(uuid))
				return FALSE;
			}
		}
		else {
			PRINT_ERROR(TQString("connection for provided uuid '%1' was not found").arg(uuid));
			return FALSE;
		}
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object").arg(uuid));
		return FALSE;
	}
}

bool TDENetworkConnectionManager_BackendNM::verifyConnectionSettings(TDENetworkConnection* connection) {
	// FIXME
	// This should actually attempt to validate the settings!
	return TRUE;
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDENetworkConnectionManager_BackendNM::initiateConnection(TQString uuid) {
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool ret;
	if ((d->m_networkManagerSettings) && (d->m_networkManagerProxy)) {
		ret = d->m_networkManagerSettings->GetConnectionByUuid(uuid, existingConnection, error);
		if (ret) {
			TQString dbusDeviceString;
			if (m_macAddress == "") {
				dbusDeviceString = "/";
			}
			else {
				dbusDeviceString = deviceInterfaceString(m_macAddress);
			}
			connect(d->m_networkManagerProxy, SIGNAL(ActivateConnectionAsyncReply(int, const TQT_DBusObjectPath&)), d, SLOT(processAddConnectionAsyncReply(int, const TQT_DBusObjectPath&)));
			int asyncCallID;
			ret = d->m_networkManagerProxy->ActivateConnectionAsync(asyncCallID, existingConnection, TQT_DBusObjectPath(dbusDeviceString.ascii()), TQT_DBusObjectPath("/"), error);
			if (ret && error.isValid()) {
				ret = 0;
				PRINT_ERROR(error.name())
			}
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
				d->nmAddConnectionAsyncResponse.remove(asyncCallID);
				return checkConnectionStatus(uuid);
			}
			else {
				// Error!
				PRINT_ERROR(error.name())
				return checkConnectionStatus(uuid);
			}
		}
		else {
			PRINT_ERROR(TQString("connection for provided uuid '%1' was not found").arg(uuid));
			return TDENetworkConnectionStatus::Invalid;
		}
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object").arg(uuid));
		return TDENetworkConnectionStatus::Invalid;
	}
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDENetworkConnectionManager_BackendNM::checkConnectionStatus(TQString uuid) {
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
		PRINT_ERROR(TQString("active connection for provided uuid '%1' was not found").arg(uuid));
		return TDENetworkConnectionStatus::Invalid;
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object").arg(uuid));
		return TDENetworkConnectionStatus::Invalid;
	}
}

TQStringList TDENetworkConnectionManager_BackendNM::connectionPhysicalDeviceUUIDs(TQString uuid) {
	if (deviceType() == TDENetworkDeviceType::BackendOnly) {
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
					DBus::DeviceProxy underlyingNetworkDeviceProxy(NM_DBUS_SERVICE, *it2);
					underlyingNetworkDeviceProxy.setConnection(TQT_DBusConnection::systemBus());
					TQString devUUID = underlyingNetworkDeviceProxy.getUdi(error);
					if (devUUID != "") {
						ret.append(devUUID);
					}
				}
			}
		}
		return ret;
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object").arg(uuid));
		return TQStringList();
	}
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDENetworkConnectionManager_BackendNM::deactivateConnection(TQString uuid) {
	TQT_DBusObjectPath existingConnection;
	TQT_DBusError error;
	bool ret;
	if ((d->m_networkManagerSettings) && (d->m_networkManagerProxy)) {
		ret = d->m_networkManagerSettings->GetConnectionByUuid(uuid, existingConnection, error);
		if (ret) {
			TQString dbusDeviceString;
			if (m_macAddress == "") {
				dbusDeviceString = "/";
			}
			else {
				dbusDeviceString = deviceInterfaceString(m_macAddress);
			}
			connect(d->m_networkManagerProxy, SIGNAL(DeactivateConnectionAsyncReply(int)), d, SLOT(processConnectionSettingsUpdateAsyncReply(int)));
			int asyncCallID;
			ret = d->m_networkManagerProxy->DeactivateConnectionAsync(asyncCallID, existingConnection, error);
			if (ret && error.isValid()) {
				ret = 0;
				PRINT_ERROR(error.name())
			}
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
				PRINT_ERROR(error.name())
				return checkConnectionStatus(uuid);
			}
		}
		else {
			PRINT_ERROR(TQString("connection for provided uuid '%1' was not found").arg(uuid));
			return TDENetworkConnectionStatus::Invalid;
		}
	}
	else {
		PRINT_ERROR(TQString("invalid internal network-manager settings proxy object").arg(uuid));
		return TDENetworkConnectionStatus::Invalid;
	}
}

TDENetworkWiFiAPInfo* TDENetworkConnectionManager_BackendNM::getAccessPointDetails(TQString dbusPath) {
	TDENetworkWiFiAPInfo* apInfo = new TDENetworkWiFiAPInfo;
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

TDENetworkHWNeighborList* TDENetworkConnectionManager_BackendNM::siteSurvey() {
	TQT_DBusError error;
	bool ret;

	TDENetworkDeviceType::TDENetworkDeviceType myDeviceType = deviceType();
	TQString dbusDeviceString = deviceInterfaceString(m_macAddress);
	clearTDENetworkHWNeighborList();

	if (myDeviceType == TDENetworkDeviceType::WiFi) {
		DBus::WiFiDeviceProxy wiFiDevice(NM_DBUS_SERVICE, dbusDeviceString);
		wiFiDevice.setConnection(TQT_DBusConnection::systemBus());
		// FIXME
		// Should call wiFiDevice.RequestScanAsync first to rescan all access points
		TQT_DBusObjectPathList accessPoints;
		ret = wiFiDevice.GetAccessPoints(accessPoints, error);
		if (ret) {
			TQT_DBusObjectPathList::iterator it;
			for (it = accessPoints.begin(); it != accessPoints.end(); ++it) {
				TDENetworkWiFiAPInfo* apInfo = getAccessPointDetails(TQString(*it));
				if (apInfo) {
					m_hwNeighborList->append(apInfo);
				}
			}
		}
	}

	return m_hwNeighborList;
}

bool TDENetworkConnectionManager_BackendNM::networkingEnabled() {
	if (d->m_networkManagerProxy) {
		bool ret;
		TQT_DBusError error;
		ret = d->m_networkManagerProxy->getNetworkingEnabled(error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR(error.name())
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

bool TDENetworkConnectionManager_BackendNM::wiFiHardwareEnabled() {
	if (d->m_networkManagerProxy) {
		bool ret;
		TQT_DBusError error;
		ret = d->m_networkManagerProxy->getWirelessHardwareEnabled(error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR(error.name())
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

bool TDENetworkConnectionManager_BackendNM::enableNetworking(bool enable) {
	// FIXME
	// Yes, this abuses the Sleep command
	// Is there a better way to do it?
	if (d->m_networkManagerProxy) {
		int asynccallid;
		TQT_DBusError error;
		d->m_networkManagerProxy->SleepAsync(asynccallid, !enable, error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR(error.name())
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

bool TDENetworkConnectionManager_BackendNM::enableWiFi(bool enable) {
	if (d->m_networkManagerProxy) {
		TQT_DBusError error;
		d->m_networkManagerProxy->setWirelessEnabled(enable, error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR(error.name())
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

bool TDENetworkConnectionManager_BackendNM::wiFiEnabled() {
	if (d->m_networkManagerProxy) {
		bool ret;
		TQT_DBusError error;
		ret = d->m_networkManagerProxy->getWirelessEnabled(error);
		if (error.isValid()) {
			// Error!
			PRINT_ERROR(error.name())
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

#include "network-manager.moc"
#include "network-manager_p.moc"