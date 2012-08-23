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
		virtual bool saveConnection(TDENetworkConnection connection);
		virtual bool deleteConnection(TQString uuid);

		virtual TDENetworkConnectionList* connections();

		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus initiateConnection(TQString uuid);
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus checkConnectionStatus(TQString uuid);
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus deactivateConnection(TQString uuid);

		virtual TDENetworkHWNeighborList* siteSurvey();
};

#endif // _TDENETWORKBACKEND_NETWORKMANAGER_H