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

#include "networkdevice.h"

// Network connection manager
#include "networkconnections.h"

#include "config.h"

#ifdef WITH_NETWORK_MANAGER_BACKEND
	#include "network-manager.h"
#endif // WITH_NETWORK_MANAGER_BACKEND

using namespace TDEHW;

NetworkDevice::NetworkDevice(GenericDeviceType::GenericDeviceType dt, TQString dn) : GenericDevice(dt, dn) {
	m_rxbytes = -1;
	m_txbytes = -1;
	m_rxpackets = -1;
	m_txpackets = -1;
	m_connectionManager = NULL;
}

NetworkDevice::~NetworkDevice() {
	if (m_connectionManager) {
		delete m_connectionManager;
	}
}

TQString NetworkDevice::macAddress() {
	return m_macAddress;
}

void NetworkDevice::internalSetMacAddress(TQString ma) {
	m_macAddress = ma;
}

TQString NetworkDevice::state() {
	return m_state;
}

void NetworkDevice::internalSetState(TQString st) {
	m_state = st;
}

bool NetworkDevice::carrierPresent() {
	return m_carrier;
}

void NetworkDevice::internalSetCarrierPresent(bool cp) {
	m_carrier = cp;
}

bool NetworkDevice::dormant() {
	return m_dormant;
}

void NetworkDevice::internalSetDormant(bool dm) {
	m_dormant = dm;
}

TQString NetworkDevice::ipV4Address() {
	return m_ipV4Address;
}

void NetworkDevice::internalSetIpV4Address(TQString ad) {
	m_ipV4Address = ad;
}

TQString NetworkDevice::ipV6Address() {
	return m_ipV6Address;
}

void NetworkDevice::internalSetIpV6Address(TQString ad) {
	m_ipV6Address = ad;
}

TQString NetworkDevice::ipV4Netmask() {
	return m_ipV4Netmask;
}

void NetworkDevice::internalSetIpV4Netmask(TQString nm) {
	m_ipV4Netmask = nm;
}

TQString NetworkDevice::ipV6Netmask() {
	return m_ipV6Netmask;
}

void NetworkDevice::internalSetIpV6Netmask(TQString nm) {
	m_ipV6Netmask = nm;
}

TQString NetworkDevice::ipV4Broadcast() {
	return m_ipV4Broadcast;
}

void NetworkDevice::internalSetIpV4Broadcast(TQString br) {
	m_ipV4Broadcast = br;
}

TQString NetworkDevice::ipV6Broadcast() {
	return m_ipV6Broadcast;
}

void NetworkDevice::internalSetIpV6Broadcast(TQString br) {
	m_ipV6Broadcast = br;
}

TQString NetworkDevice::ipV4Destination() {
	return m_ipV4Destination;
}

void NetworkDevice::internalSetIpV4Destination(TQString ds) {
	m_ipV4Destination = ds;
}

TQString NetworkDevice::ipV6Destination() {
	return m_ipV6Destination;
}

void NetworkDevice::internalSetIpV6Destination(TQString ds) {
	m_ipV6Destination = ds;
}

double NetworkDevice::rxBytes() {
	return m_rxbytes;
}

void NetworkDevice::internalSetRxBytes(double rx) {
	m_rxbytes = rx;
}

double NetworkDevice::txBytes() {
	return m_txbytes;
}

void NetworkDevice::internalSetTxBytes(double tx) {
	m_txbytes = tx;
}

double NetworkDevice::rxPackets() {
	return m_rxpackets;
}

void NetworkDevice::internalSetRxPackets(double rx) {
	m_rxpackets = rx;
}

double NetworkDevice::txPackets() {
	return m_txpackets;
}

void NetworkDevice::internalSetTxPackets(double tx) {
	m_txpackets = tx;
}

NetworkConnectionManager* NetworkDevice::connectionManager() {
#ifdef WITH_NETWORK_MANAGER_BACKEND
	if (!m_connectionManager) {
		m_connectionManager = new NetworkConnectionManager_BackendNM(m_macAddress);
	}
#endif // WITH_NETWORK_MANAGER_BACKEND

	return m_connectionManager;
}

void NetworkDevice::internalSetConnectionManager(NetworkConnectionManager* mgr) {
	m_connectionManager = mgr;
}

#include "networkdevice.moc"
