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

#include "hardwaredevices.h"
#include "networkconnections.h"

#include <tqtimer.h>

#include <tdelocale.h>

#include "networkdevice.h"
// #define DEBUG_SIGNAL_QUEUE 1

#include "config.h"

#ifdef WITH_NETWORK_MANAGER_BACKEND
#include "network-manager.h"
#endif // WITH_NETWORK_MANAGER_BACKEND

#define SET_BIT(x, y) (x |= 1 << y)
#define TEST_BIT(x, y) ((x & (1 << y)) >> y)

using namespace TDEHW;

/*================================================================================================*/
/* NetworkSearchDomain                                                                         */
/*================================================================================================*/

NetworkSearchDomain::NetworkSearchDomain() {
	m_isIPV6 = false;
}

NetworkSearchDomain::NetworkSearchDomain(TQString domain, bool ipv6) {
	m_isIPV6 = ipv6;
	m_domain = domain;
}

NetworkSearchDomain::~NetworkSearchDomain() {
	//
}

TQString NetworkSearchDomain::searchDomain() {
	return m_domain;
}

void NetworkSearchDomain::setSearchDomain(TQString domain, bool ipv6) {
	m_isIPV6 = ipv6;
	m_domain = domain;
}

bool NetworkSearchDomain::isIPv4() {
	return !m_isIPV6;
}

bool NetworkSearchDomain::isIPv6() {
	return m_isIPV6;
}

/*================================================================================================*/
/* NetMask                                                                                     */
/*================================================================================================*/

NetMask::NetMask() {
	m_ipv4NetMask = 0;
	m_isIPV6 = false;
}

NetMask::NetMask(TQ_UINT32 netmask) {
	m_ipv4NetMask = netmask;
	m_isIPV6 = false;
}

NetMask::NetMask(TQ_UINT8* netmask) {
	m_ipv6NetMask = TQHostAddress(netmask);
	m_isIPV6 = true;
}

NetMask::~NetMask() {
	//
}

void NetMask::fromCIDRMask(unsigned char mask, bool ipv6) {
	unsigned int i;
	unsigned int j;
	unsigned int internalMask = mask;
	if (!ipv6) {
		m_ipv4NetMask = 0;
		for (i=31;i>=(32-internalMask);i--) {
			SET_BIT(m_ipv4NetMask, i);
		}
		m_isIPV6 = false;
	}
	else {
		Q_IPV6ADDR maskarray;
		j=0;
		unsigned int byteno=0;
		memset(maskarray.c, 0, 16);
		for (i=127;i>=(128-internalMask);i--) {
			SET_BIT(maskarray.c[byteno], (i-((15-byteno)*8)));
			j++;
			if (j>7) {
				j=0;
				byteno++;
			}
		}
		m_ipv6NetMask = TQHostAddress(maskarray);
		m_isIPV6 = true;
	}
}

unsigned char NetMask::toCIDRMask() {
	unsigned int i;
	unsigned int j;
	if (!m_isIPV6) {
		for (i=0; i<32; i++) {
			if (TEST_BIT(m_ipv4NetMask, i)) {
				break;
			}
		}
		return 32-i;
	}
	else {
		Q_IPV6ADDR mask = m_ipv6NetMask.toIPv6Address();
		bool found = false;
		for (j=0; j<16; ++j) {
			for (i=0; i<8; i++) {
				if (!TEST_BIT(mask.c[j], i)) {
					found = true;
					break;
				}
			}
			if (found) break;
		}
		return ((j*8)+i);
	}
}

void NetMask::fromString(TQString mask) {
	if (mask.contains(".")) {
		m_isIPV6 = false;
		m_ipv4NetMask = 0;
		TQStringList pieces = TQStringList::split(".", mask);
		TQ_UINT8 chunk;
		chunk = pieces[0].toUShort();
		m_ipv4NetMask = m_ipv4NetMask | (chunk << 24);
		chunk = pieces[1].toUShort();
		m_ipv4NetMask = m_ipv4NetMask | (chunk << 16);
		chunk = pieces[2].toUShort();
		m_ipv4NetMask = m_ipv4NetMask | (chunk << 8);
		chunk = pieces[3].toUShort();
		m_ipv4NetMask = m_ipv4NetMask | (chunk << 0);
	}
	else if (mask.contains(":")) {
		m_isIPV6 = true;
		m_ipv6NetMask.setAddress(mask);
	}
}

TQString NetMask::toString() {
	if (!m_isIPV6) {
		return TQString("%1.%2.%3.%4").arg((m_ipv4NetMask & 0xff000000) >> 24).arg((m_ipv4NetMask & 0x00ff0000) >> 16).arg((m_ipv4NetMask & 0x0000ff00) >> 8).arg((m_ipv4NetMask & 0x000000ff) >> 0);
	}
	else {
		return m_ipv6NetMask.toString();
	}
}

bool NetMask::isIPv4() {
	return !m_isIPV6;
}

bool NetMask::isIPv6() {
	return m_isIPV6;
}

/*================================================================================================*/
/* MACAddress                                                                      */
/*================================================================================================*/

MACAddress::MACAddress() {
	m_macAddress.clear();
	m_isValid = false;
}

MACAddress::MACAddress(NetworkByteList address) {
	m_macAddress = address;
	m_isValid = true;
}

MACAddress::~MACAddress() {
	//
}

NetworkByteList MACAddress::address() {
	return m_macAddress;
}

void MACAddress::setAddress(NetworkByteList address) {
	m_macAddress = address;
	m_isValid = true;
}

bool MACAddress::isValid() {
	if (!m_isValid) {
		return false;
	}

	if (m_macAddress.count() > 0) {
		return true;
	}
	else {
		return false;
	}
}

void MACAddress::fromString(TQString address) {
	TQStringList pieces = TQStringList::split(":", address);
	m_macAddress.clear();
	for (TQStringList::Iterator it = pieces.begin(); it != pieces.end(); ++it) {
		m_macAddress.append((*it).toUShort(0, 16));
	}
	m_isValid = true;
}

TQString MACAddress::toString() {
	TQString ret;
	NetworkByteList::iterator it;
	for (it = m_macAddress.begin(); it != m_macAddress.end(); ++it) {
		if (ret != "") {
			ret.append(":");
		}
		ret.append(TQString().sprintf("%02x", *it));
	}
	return ret.lower();
}

namespace TDEHW {

bool operator==(const MACAddress &a1, const MACAddress &a2) {
	if (a1.m_macAddress.count() != a2.m_macAddress.count()) {
		return false;
	}
	else {
		unsigned int i;
		for (i=0; i<a1.m_macAddress.count(); i++) {
			if (a1.m_macAddress[i] != a2.m_macAddress[i]) {
				return false;
			}
		}
		return true;
	}
}

bool operator<(const MACAddress &a1, const MACAddress &a2) {
	if (a1.m_macAddress.count() < a2.m_macAddress.count()) {
		return true;
	}
	else {
		unsigned int i;
		for (i=0; i<a1.m_macAddress.count(); i++) {
			if (a1.m_macAddress[i] < a2.m_macAddress[i]) {
				return true;
			}
			if (a1.m_macAddress[i] > a2.m_macAddress[i]) {
				return false;
			}
		}
		return false;
	}
}

}

/*================================================================================================*/
/* NetworkSingleIPConfiguration                                                                */
/*================================================================================================*/

NetworkSingleIPConfiguration::NetworkSingleIPConfiguration() {
	valid = false;
}

NetworkSingleIPConfiguration::~NetworkSingleIPConfiguration() {
	//
}

bool NetworkSingleIPConfiguration::isIPv4() {
	return ipAddress.isIPv4Address() & valid;
}

bool NetworkSingleIPConfiguration::isIPv6() {
	return ipAddress.isIPv6Address() & valid;
}

/*================================================================================================*/
/* NetworkSingleRouteConfiguration                                                                */
/*================================================================================================*/

NetworkSingleRouteConfiguration::NetworkSingleRouteConfiguration() {
	valid = false;
}

NetworkSingleRouteConfiguration::~NetworkSingleRouteConfiguration() {
	//
}

bool NetworkSingleRouteConfiguration::isIPv4() {
	return ipAddress.isIPv4Address() & valid;
}

bool NetworkSingleRouteConfiguration::isIPv6() {
	return ipAddress.isIPv6Address() & valid;
}

/*================================================================================================*/
/* NetworkIEEE8021xConfiguration                                                               */
/*================================================================================================*/

NetworkIEEE8021xConfiguration::NetworkIEEE8021xConfiguration() {
	valid = false;
	allowedValid = false;
	secretsValid = false;
	type = NetworkIEEE8021xType::None;
	fastProvisioningFlags = NetworkIEEE8021xFastFlags::None;
	phase2NonEAPAuthMethod = NetworkIEEE8021xType::None;
	phase2EAPAuthMethod = NetworkIEEE8021xType::None;
	passwordFlags = NetworkPasswordHandlingFlags::None;
	binaryPasswordFlags = NetworkPasswordHandlingFlags::None;
	privateKeyPasswordFlags = NetworkPasswordHandlingFlags::None;
	phase2PrivateKeyPasswordFlags = NetworkPasswordHandlingFlags::None;
	forceSystemCaCertificates = false;
}

NetworkIEEE8021xConfiguration::~NetworkIEEE8021xConfiguration() {
	//
}

/*================================================================================================*/
/* NetworkPPPConfiguration                                                                     */
/*================================================================================================*/

NetworkPPPConfiguration::NetworkPPPConfiguration() {
	valid = false;
	requireServerAuthentication = false;
	flags = NetworkPPPFlags::None;
	baudRate = 115200;
	mru = 0;
	mtu = 0;
	lcpEchoPingInterval = 0;
	lcpEchoFailureThreshold = 0;
}

NetworkPPPConfiguration::~NetworkPPPConfiguration() {
	//
}

/*================================================================================================*/
/* NetworkPPPOEConfiguration                                                                   */
/*================================================================================================*/

NetworkPPPOEConfiguration::NetworkPPPOEConfiguration() {
	valid = false;
	secretsValid = false;
	passwordFlags = NetworkPasswordHandlingFlags::None;
}

NetworkPPPOEConfiguration::~NetworkPPPOEConfiguration() {
	//
}

/*================================================================================================*/
/* NetworkSerialConfiguration                                                                  */
/*================================================================================================*/

NetworkSerialConfiguration::NetworkSerialConfiguration() {
	valid = false;
	baudRate = 115200;
	byteWidth = 8;
	parity = NetworkParity::None;
	stopBits = 1;
	txDelay = 0;
}

NetworkSerialConfiguration::~NetworkSerialConfiguration() {
	//
}

/*================================================================================================*/
/* NetworkCDMAConfiguration                                                                    */
/*================================================================================================*/

NetworkCDMAConfiguration::NetworkCDMAConfiguration() {
	valid = false;
	secretsValid = false;
	passwordFlags = NetworkPasswordHandlingFlags::None;
}

NetworkCDMAConfiguration::~NetworkCDMAConfiguration() {
	//
}

/*================================================================================================*/
/* NetworkGSMConfiguration                                                                     */
/*================================================================================================*/

NetworkGSMConfiguration::NetworkGSMConfiguration() {
	valid = false;
	secretsValid = false;
	passwordFlags = NetworkPasswordHandlingFlags::None;
	networkType = GSMNetworkType::Any;
	pinFlags = NetworkPasswordHandlingFlags::None;
	allowRoaming = false;
}

NetworkGSMConfiguration::~NetworkGSMConfiguration() {
	//
}

/*================================================================================================*/
/* NetworkIPConfiguration                                                                      */
/*================================================================================================*/

NetworkIPConfiguration::NetworkIPConfiguration() {
	valid = false;
	connectionFlags =	NetworkIPConfigurationFlags::IPV4DHCPIP			| \
				NetworkIPConfigurationFlags::IPV4DHCPDNS			| \
				NetworkIPConfigurationFlags::IPV4DHCPRoutes			| \
				NetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute	| \
				NetworkIPConfigurationFlags::IPV6DHCPIP			| \
				NetworkIPConfigurationFlags::IPV6DHCPDNS			| \
				NetworkIPConfigurationFlags::IPV6DHCPRoutes			| \
				NetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute;
}

NetworkIPConfiguration::~NetworkIPConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkWiFiSecurityConfiguration                                                            */
/*================================================================================================*/

TDENetworkWiFiSecurityConfiguration::TDENetworkWiFiSecurityConfiguration() {
	valid = false;
	secretsValid = false;
	keyType = NetworkWiFiKeyType::Other;
	authType = NetworkWiFiAuthType::Other;
	wpaVersion = NetworkWiFiWPAVersionFlags::Any;
	cipher = NetworkWiFiConnectionCipher::None;
	wepKeyFlags = NetworkPasswordHandlingFlags::None;
	wepKeyIndex = 0;
	wepKeyType = NetworkWepKeyType::Hexadecimal;
	pskFlags = NetworkPasswordHandlingFlags::None;
	leapPasswordFlags = NetworkPasswordHandlingFlags::None;
}

TDENetworkWiFiSecurityConfiguration::~TDENetworkWiFiSecurityConfiguration() {
	//
}

/*================================================================================================*/
/* NetworkWiFiDeviceInfo                                                                      */
/*================================================================================================*/

NetworkWiFiDeviceInfo::NetworkWiFiDeviceInfo() {
	valid = false;
}

NetworkWiFiDeviceInfo::~NetworkWiFiDeviceInfo() {
	//
}

/*================================================================================================*/
/* NetworkDeviceInformation                                                                      */
/*================================================================================================*/

NetworkDeviceInformation::NetworkDeviceInformation() {
	valid = false;
	statusFlags = NetworkConnectionStatus::Invalid;
	capabilityFlags = NetworkDeviceCapabilityFlags::None;
}

NetworkDeviceInformation::~NetworkDeviceInformation() {
	//
}

/*================================================================================================*/
/* NetworkHWNeighbor                                                                      */
/*================================================================================================*/

NetworkHWNeighbor::NetworkHWNeighbor() {
	valid = false;
}

NetworkHWNeighbor::~NetworkHWNeighbor() {
	//
}

/*================================================================================================*/
/* NetworkWiFiAPInfo                                                                      */
/*================================================================================================*/

NetworkWiFiAPInfo::NetworkWiFiAPInfo() : NetworkHWNeighbor() {
	//
}

NetworkWiFiAPInfo::~NetworkWiFiAPInfo() {
	//
}

TQString NetworkWiFiAPInfo::friendlySSID() const {
	TQString ret;

	ret = TQString(SSID);
	if (ret == "") {
		ret = "<" + i18n("hidden") + ">";
	}

	return ret;
}

/*================================================================================================*/
/* NetworkConnection                                                                           */
/*================================================================================================*/

NetworkConnection::NetworkConnection() {
	readOnly = false;
	autoConnect = false;
	fullDuplex = true;
	requireIPV4 = false;
	requireIPV6 = false;
	mtu = 0;
}

NetworkConnection::~NetworkConnection() {
	//
}

NetworkConnectionType::NetworkConnectionType NetworkConnection::type() {
	if (dynamic_cast<WiredEthernetConnection*>(this)) {
		return NetworkConnectionType::WiredEthernet;
	}
	else if (dynamic_cast<WiredInfinibandConnection*>(this)) {
		return NetworkConnectionType::Infiniband;
	}
	else if (dynamic_cast<VPNConnection*>(this)) {
		return NetworkConnectionType::VPN;
	}
	else if (dynamic_cast<WiMaxConnection*>(this)) {
		return NetworkConnectionType::WiMax;
	}
	else if (dynamic_cast<VLANConnection*>(this)) {
		return NetworkConnectionType::VLAN;
	}
	else if (dynamic_cast<OLPCMeshConnection*>(this)) {
		return NetworkConnectionType::OLPCMesh;
	}
	else if (dynamic_cast<BluetoothConnection*>(this)) {
		return NetworkConnectionType::Bluetooth;
	}
	else if (dynamic_cast<ModemConnection*>(this)) {
		return NetworkConnectionType::Modem;
	}
	else if (dynamic_cast<WiFiConnection*>(this)) {
		return NetworkConnectionType::WiFi;
	}
	else {
		return NetworkConnectionType::Other;
	}
}

/*================================================================================================*/
/* WiredEthernetConnection                                                                     */
/*================================================================================================*/

WiredEthernetConnection::WiredEthernetConnection() : NetworkConnection() {
	//
}

WiredEthernetConnection::~WiredEthernetConnection() {
	//
}

/*================================================================================================*/
/* WiredInfinibandConnection                                                                   */
/*================================================================================================*/

WiredInfinibandConnection::WiredInfinibandConnection() : NetworkConnection() {
	//
}

WiredInfinibandConnection::~WiredInfinibandConnection() {
	//
}

/*================================================================================================*/
/* VPNConnection                                                                               */
/*================================================================================================*/

VPNConnection::VPNConnection() : NetworkConnection() {
	secretsValid = false;
}

VPNConnection::~VPNConnection() {
	//
}

/*================================================================================================*/
/* WiMaxConnection                                                                             */
/*================================================================================================*/

WiMaxConnection::WiMaxConnection() : NetworkConnection() {
	//
}

WiMaxConnection::~WiMaxConnection() {
	//
}

/*================================================================================================*/
/* VLANConnection                                                                              */
/*================================================================================================*/

VLANConnection::VLANConnection() : NetworkConnection() {
	//
}

VLANConnection::~VLANConnection() {
	//
}

/*================================================================================================*/
/* OLPCMeshConnection                                                                          */
/*================================================================================================*/

OLPCMeshConnection::OLPCMeshConnection() : NetworkConnection() {
	channel = 0;
}

OLPCMeshConnection::~OLPCMeshConnection() {
	//
}

/*================================================================================================*/
/* BluetoothConnection                                                                         */
/*================================================================================================*/

BluetoothConnection::BluetoothConnection() : NetworkConnection() {
	type = BluetoothConnectionType::Other;
}

BluetoothConnection::~BluetoothConnection() {
	//
}

/*================================================================================================*/
/* ModemConnection                                                                             */
/*================================================================================================*/

ModemConnection::ModemConnection() : NetworkConnection() {
	type = ModemConnectionType::Other;
}

ModemConnection::~ModemConnection() {
	//
}

/*================================================================================================*/
/* WiFiConnection                                                                              */
/*================================================================================================*/

WiFiConnection::WiFiConnection() : NetworkConnection() {
	operatingMode = WiFiMode::Other;
	bandRestriction = WiFiFrequencyBand::None;
	channelRestriction = -1;
	bitRateRestriction = -1;
	powerRestriction = -1;
	isHiddenNetwork = false;
	securityRequired = false;
}

WiFiConnection::~WiFiConnection() {
	//
}

/*================================================================================================*/
/* NetworkConnectionManager                                                                    */
/*================================================================================================*/

NetworkConnectionManager::NetworkConnectionManager(TQString macAddress) : TQObject(), m_connectionList(NULL), m_hwNeighborList(NULL), m_macAddress(macAddress), m_prevConnectionStatus(NetworkGlobalManagerFlags::Unknown) {
	m_emissionTimer = new TQTimer();
	connect(m_emissionTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(emitQueuedSignals()));
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, TRUE);
}

NetworkConnectionManager::~NetworkConnectionManager() {
	m_emissionTimer->stop();
	delete m_emissionTimer;
}

TQString NetworkConnectionManager::deviceMACAddress() {
	return m_macAddress;
}

NetworkConnectionList* NetworkConnectionManager::connections() {
	return m_connectionList;
}

NetworkConnection* NetworkConnectionManager::findConnectionByUUID(TQString uuid) {
	NetworkConnection *connection;
	for (connection = m_connectionList->first(); connection; connection = m_connectionList->next()) {
		if (connection->UUID == uuid) {
			return connection;
		}
	}
	return NULL;
}

NetworkDevice* NetworkConnectionManager::findDeviceByUUID(TQString uuid) {
	HardwareDevices *hwdevices = HardwareDevices::instance();
	if (!hwdevices) return NULL;

	GenericHardwareList devices = hwdevices->listByDeviceClass(GenericDeviceType::Network);
	for (GenericHardwareList::iterator it = devices.begin(); it != devices.end(); ++it)
	{
		NetworkDevice* dev = dynamic_cast<NetworkDevice*>(*it);
		if (dev) {
			NetworkConnectionManager* deviceConnectionManager = dev->connectionManager();
			if (deviceConnectionManager) {
				if (deviceConnectionManager->deviceInformation().UUID == uuid) {
					return dev;
				}
			}
		}
	}

	return NULL;
}

NetworkWiFiAPInfo* NetworkConnectionManager::findAccessPointByBSSID(MACAddress bssid) {
	if (!bssid.isValid()) {
		return NULL;
	}

	NetworkHWNeighbor *neighbor;
	for (neighbor = m_hwNeighborList->first(); neighbor; neighbor = m_hwNeighborList->next()) {
		NetworkWiFiAPInfo* apInfo = dynamic_cast<NetworkWiFiAPInfo*>(neighbor);
		if (!apInfo) {
			continue;
		}
		if (apInfo->BSSID == bssid) {
			return apInfo;
		}
	}
	return NULL;
}

TQString NetworkConnectionManager::friendlyConnectionTypeName(NetworkConnectionType::NetworkConnectionType type) {
	if (type == NetworkConnectionType::WiredEthernet) {
		return i18n("Wired Ethernet");
	}
	else if (type == NetworkConnectionType::WiFi) {
		return i18n("802.11 WiFi");
	}
	else if (type == NetworkConnectionType::Bluetooth) {
		return i18n("Bluetooth");
	}
	else if (type == NetworkConnectionType::OLPCMesh) {
		return i18n("OLPC Mesh");
	}
	else if (type == NetworkConnectionType::WiMax) {
		return i18n("WiMax");
	}
	else if (type == NetworkConnectionType::Modem) {
		return i18n("Cellular Modem");
	}
	else if (type == NetworkConnectionType::Infiniband) {
		return i18n("Infiniband");
	}
	else if (type == NetworkConnectionType::Bond) {
		return i18n("Bond");
	}
	else if (type == NetworkConnectionType::VLAN) {
		return i18n("Virtual LAN");
	}
	else if (type == NetworkConnectionType::ADSL) {
		return i18n("ADSL");
	}
	else if (type == NetworkConnectionType::VPN) {
		return i18n("Virtual Private Network");
	}
	else if (type == NetworkConnectionType::Other) {
		return i18n("Other");
	}
	else {
		return TQString::null;
	}
}

bool NetworkConnectionManager::validateIPAddress(TQHostAddress address) {
	if (address.isIPv4Address()) {
		TQ_UINT32 rawaddress = address.toIPv4Address();
		if ((((rawaddress & 0xff000000) >> 24) == 0) || ((rawaddress & 0x000000ff) == 0) || ((rawaddress & 0x000000ff) == 255)) {
			return false;
		}
	}
	else if (address.isIPv6Address()) {
		Q_IPV6ADDR rawaddress = address.toIPv6Address();
		if (rawaddress.c[0] == 0xff) {
			return false;
		}
	}
	return true;
}

bool NetworkConnectionManager::validateIPNeworkMask(TQHostAddress netmask) {
	Q_UNUSED(netmask);
	return TRUE;
}

void NetworkConnectionManager::clearNetworkConnectionList() {
	NetworkConnection *connection;
	for (connection = m_connectionList->first(); connection; connection = m_connectionList->next()) {
		delete connection;
	}
	m_connectionList->clear();
}

void NetworkConnectionManager::clearNetworkHWNeighborList() {
	NetworkHWNeighbor *neighbor;
	for (neighbor = m_hwNeighborList->first(); neighbor; neighbor = m_hwNeighborList->next()) {
		delete neighbor;
	}
	m_hwNeighborList->clear();
}

void NetworkConnectionManager::internalNetworkConnectionStateChanged(NetworkGlobalManagerFlags::NetworkGlobalManagerFlags newState) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 0;
	queuedEvent.newState = newState;
	queuedEvent.previousState = m_prevConnectionStatus;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, TRUE);

	m_prevConnectionStatus = newState;
}

void NetworkConnectionManager::internalNetworkDeviceStateChanged(NetworkConnectionStatus::NetworkConnectionStatus newState, TQString hwAddress) {
	if (!m_prevDeviceStatus.contains(hwAddress)) {
		m_prevDeviceStatus[hwAddress] = NetworkConnectionStatus::Invalid;
	}

	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 1;
	queuedEvent.newConnStatus = newState;
	queuedEvent.previousConnStatus = m_prevDeviceStatus[hwAddress];
	queuedEvent.hwAddress = hwAddress;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, TRUE);

	m_prevDeviceStatus[hwAddress] = newState;
}

void NetworkConnectionManager::internalAccessPointStatusChanged(MACAddress BSSID, NetworkAPEventType::NetworkAPEventType event) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 2;
	queuedEvent.BSSID = BSSID;
	queuedEvent.apevent = event;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, TRUE);
}

void NetworkConnectionManager::internalNetworkDeviceEvent(NetworkDeviceEventType::NetworkDeviceEventType event, TQString message) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 3;
	queuedEvent.ndevent = event;
	queuedEvent.message = message;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, TRUE);
}

void NetworkConnectionManager::internalVpnEvent(NetworkVPNEventType::NetworkVPNEventType event, TQString message) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 4;
	queuedEvent.vpnevent = event;
	queuedEvent.message = message;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, TRUE);
}

void NetworkConnectionManager::internalNetworkManagementEvent(NetworkGlobalEventType::NetworkGlobalEventType event) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 5;
	queuedEvent.globalevent = event;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, TRUE);
}

void NetworkConnectionManager::emitQueuedSignals() {
	if (!m_globalEventQueueEventList.isEmpty()) {
#ifdef DEBUG_SIGNAL_QUEUE
		kdDebug() << "NetworkConnectionManager::emitQueuedSignals: Going to dequeue " << m_globalEventQueueEventList.count() << " events..." << endl;
#endif // DEBUG_SIGNAL_QUEUE
		NetworkEventQueueEvent_PrivateList::Iterator it;
		it = m_globalEventQueueEventList.begin();
		while (it != m_globalEventQueueEventList.end()) {
			TDENetworkEventQueueEvent_Private event = (*it);
			it = m_globalEventQueueEventList.remove(it);
			if (event.eventType == 0) {
				emit(networkConnectionStateChanged(event.newState, event.previousState));
			}
			else if (event.eventType == 1) {
				emit(networkDeviceStateChanged(event.newConnStatus, event.previousConnStatus, event.hwAddress));
			}
			else if (event.eventType == 2) {
				emit(accessPointStatusChanged(event.BSSID, event.apevent));
			}
			else if (event.eventType == 3) {
				emit(networkDeviceEvent(event.ndevent, event.message));
			}
			else if (event.eventType == 4) {
				emit(vpnEvent(event.vpnevent, event.message));
			}
			else if (event.eventType == 5) {
				emit(networkManagementEvent(event.globalevent));
			}
		}
#ifdef DEBUG_SIGNAL_QUEUE
		kdDebug() << "NetworkConnectionManager::emitQueuedSignals: " << m_globalEventQueueEventList.count() << " events remain in queue" << endl;
#endif // DEBUG_SIGNAL_QUEUE
	}
}

/*================================================================================================*/
/* GlobalNetworkManager                                                                        */
/*================================================================================================*/

GlobalNetworkManager::GlobalNetworkManager() : m_internalConnectionManager(NULL) {
#ifdef WITH_NETWORK_MANAGER_BACKEND
	m_internalConnectionManager = new NetworkConnectionManager_BackendNM(TQString::null);
#endif // WITH_NETWORK_MANAGER_BACKEND
	if (m_internalConnectionManager) {
		connect(m_internalConnectionManager, SIGNAL(networkConnectionStateChanged(NetworkGlobalManagerFlags::NetworkGlobalManagerFlags, NetworkGlobalManagerFlags::NetworkGlobalManagerFlags)), this, SIGNAL(networkConnectionStateChanged(NetworkGlobalManagerFlags::NetworkGlobalManagerFlags, NetworkGlobalManagerFlags::NetworkGlobalManagerFlags)));
		connect(m_internalConnectionManager, SIGNAL(vpnEvent(NetworkVPNEventType::NetworkVPNEventType, TQString)), this, SIGNAL(vpnEvent(NetworkVPNEventType::NetworkVPNEventType, TQString)));
		connect(m_internalConnectionManager, SIGNAL(networkManagementEvent(NetworkGlobalEventType::NetworkGlobalEventType)), this, SIGNAL(networkManagementEvent(NetworkGlobalEventType::NetworkGlobalEventType)));
	}
}

GlobalNetworkManager::~GlobalNetworkManager() {
	delete m_internalConnectionManager;
}

TQString GlobalNetworkManager::backendName() {
	if (!m_internalConnectionManager) return TQString::null;
	return m_internalConnectionManager->backendName();
}

NetworkGlobalManagerFlags::NetworkGlobalManagerFlags GlobalNetworkManager::backendStatus() {
	if (!m_internalConnectionManager) return NetworkGlobalManagerFlags::BackendUnavailable;
	return m_internalConnectionManager->backendStatus();
}

void GlobalNetworkManager::loadConnectionInformation() {
	if (!m_internalConnectionManager) return;
	return m_internalConnectionManager->loadConnectionInformation();
}

void GlobalNetworkManager::loadConnectionAllowedValues(NetworkConnection* connection) {
	if (!m_internalConnectionManager) return;
	return m_internalConnectionManager->loadConnectionAllowedValues(connection);
}

bool GlobalNetworkManager::loadConnectionSecrets(TQString uuid) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->loadConnectionSecrets(uuid);
}

bool GlobalNetworkManager::saveConnection(NetworkConnection* connection) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->saveConnection(connection);
}

bool GlobalNetworkManager::deleteConnection(TQString uuid) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->deleteConnection(uuid);
}

bool GlobalNetworkManager::verifyConnectionSettings(NetworkConnection* connection, NetworkConnectionErrorFlags::NetworkConnectionErrorFlags* type, NetworkErrorStringMap* reason) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->verifyConnectionSettings(connection, type, reason);
}

NetworkConnectionStatus::NetworkConnectionStatus GlobalNetworkManager::initiateConnection(TQString uuid) {
	if (!m_internalConnectionManager) return NetworkConnectionStatus::Invalid;
	return m_internalConnectionManager->initiateConnection(uuid);
}

NetworkConnectionStatus::NetworkConnectionStatus GlobalNetworkManager::checkConnectionStatus(TQString uuid) {
	if (!m_internalConnectionManager) return NetworkConnectionStatus::Invalid;
	return m_internalConnectionManager->checkConnectionStatus(uuid);
}

NetworkConnectionStatus::NetworkConnectionStatus GlobalNetworkManager::deactivateConnection(TQString uuid) {
	if (!m_internalConnectionManager) return NetworkConnectionStatus::Invalid;
	return m_internalConnectionManager->deactivateConnection(uuid);
}

TQStringList GlobalNetworkManager::validSettings() {
	if (!m_internalConnectionManager) return TQStringList();
	return m_internalConnectionManager->validSettings();
}

NetworkHWNeighborList* GlobalNetworkManager::siteSurvey() {
	if (!m_internalConnectionManager) return NULL;
	return m_internalConnectionManager->siteSurvey();
}

TQStringList GlobalNetworkManager::connectionPhysicalDeviceUUIDs(TQString uuid) {
	if (!m_internalConnectionManager) return TQStringList();
	return m_internalConnectionManager->connectionPhysicalDeviceUUIDs(uuid);
}

NetworkVPNTypeList GlobalNetworkManager::availableVPNTypes() {
	if (!m_internalConnectionManager) return NetworkVPNTypeList();
	return m_internalConnectionManager->availableVPNTypes();
}

bool GlobalNetworkManager::networkingEnabled() {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->networkingEnabled();
}

bool GlobalNetworkManager::enableNetworking(bool enable) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->enableNetworking(enable);
}

bool GlobalNetworkManager::wiFiHardwareEnabled() {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->wiFiHardwareEnabled();
}

bool GlobalNetworkManager::enableWiFi(bool enable) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->enableWiFi(enable);
}

bool GlobalNetworkManager::wiFiEnabled() {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->wiFiEnabled();
}

TQStringList GlobalNetworkManager::defaultNetworkDevices() {
	if (!m_internalConnectionManager) return TQStringList();
	return m_internalConnectionManager->defaultNetworkDevices();
}

NetworkConnectionList* GlobalNetworkManager::connections() {
	if (!m_internalConnectionManager) return NULL;
	return m_internalConnectionManager->connections();
}

NetworkConnection* GlobalNetworkManager::findConnectionByUUID(TQString uuid) {
	if (!m_internalConnectionManager) return NULL;
	return m_internalConnectionManager->findConnectionByUUID(uuid);
}

NetworkDevice* GlobalNetworkManager::findDeviceByUUID(TQString uuid) {
	if (!m_internalConnectionManager) return NULL;
	return m_internalConnectionManager->findDeviceByUUID(uuid);
}

NetworkWiFiAPInfo* GlobalNetworkManager::findAccessPointByBSSID(MACAddress bssid) {
	HardwareDevices *hwdevices = HardwareDevices::instance();
	if (!hwdevices) {
		return NULL;
	}

	NetworkWiFiAPInfo* ret = NULL;
	GenericHardwareList devices = hwdevices->listByDeviceClass(GenericDeviceType::Network);
	for (GenericHardwareList::iterator it = devices.begin(); it != devices.end(); ++it) {
		NetworkDevice* dev = dynamic_cast<NetworkDevice*>(*it);
		if (dev) {
			NetworkConnectionManager* deviceConnMan = dev->connectionManager();
			NetworkWiFiAPInfo* candidate = deviceConnMan->findAccessPointByBSSID(bssid);
			if (candidate) {
				ret = candidate;
			}
		}
	}

	return ret;
}

/*================================================================================================*/
/* End                                                                                            */
/*================================================================================================*/

#include "networkconnections.moc"
