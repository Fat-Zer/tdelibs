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

TDENetworkConnectionManager_BackendNM::TDENetworkConnectionManager_BackendNM(TQString macAddress) : TDENetworkConnectionManager(macAddress) {
	//
}

TDENetworkConnectionManager_BackendNM::~TDENetworkConnectionManager_BackendNM() {
	//
}

TDENetworkConnectionType::TDENetworkConnectionType TDENetworkConnectionManager_BackendNM::connectionType() {
	//
}

TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags TDENetworkConnectionManager_BackendNM::backendStatus() {
	//
}

TDENetworkDeviceInformation TDENetworkConnectionManager_BackendNM::deviceInformation() {
	//
}

void TDENetworkConnectionManager_BackendNM::loadConnectionInformation() {
	//
}

bool TDENetworkConnectionManager_BackendNM::saveConnection(TDENetworkConnection connection) {
	//
}

bool TDENetworkConnectionManager_BackendNM::deleteConnection(TQString uuid) {
	//
}

TDENetworkConnectionList* TDENetworkConnectionManager_BackendNM::connections() {
	//
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDENetworkConnectionManager_BackendNM::initiateConnection(TQString uuid) {
	//
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDENetworkConnectionManager_BackendNM::checkConnectionStatus(TQString uuid) {
	//
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDENetworkConnectionManager_BackendNM::deactivateConnection(TQString uuid) {
	//
}

TDENetworkHWNeighborList* TDENetworkConnectionManager_BackendNM::siteSurvey() {
	//
}

#include "network-manager.moc"