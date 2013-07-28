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

#include "rootsystemdevice.h"

#include <unistd.h>

#include <tqfile.h>

#include <dcopclient.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <tdeapplication.h>

#include "config.h"

#if defined(WITH_UPOWER) || defined(WITH_CONSOLEKIT)
	#include <tqdbusdata.h>
	#include <tqdbusmessage.h>
	#include <tqdbusproxy.h>
	#include <tqdbusvariant.h>
	#include <tqdbusconnection.h>
#endif // defined(WITH_UPOWER) || defined(WITH_CONSOLEKIT)

using namespace TDEHW;

TDERootSystemDevice::TDERootSystemDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
	m_hibernationSpace = -1;
}

TDERootSystemDevice::~TDERootSystemDevice() {
}

TDESystemFormFactor::TDESystemFormFactor TDERootSystemDevice::formFactor() {
	return m_formFactor;
}

void TDERootSystemDevice::internalSetFormFactor(TDESystemFormFactor::TDESystemFormFactor ff) {
	m_formFactor = ff;
}

TDESystemPowerStateList TDERootSystemDevice::powerStates() {
	return m_powerStates;
}

void TDERootSystemDevice::internalSetPowerStates(TDESystemPowerStateList ps) {
	m_powerStates = ps;
}

TDESystemHibernationMethodList TDERootSystemDevice::hibernationMethods() {
	return m_hibernationMethods;
}

void TDERootSystemDevice::internalSetHibernationMethods(TDESystemHibernationMethodList hm) {
	m_hibernationMethods = hm;
}

TDESystemHibernationMethod::TDESystemHibernationMethod TDERootSystemDevice::hibernationMethod() {
	return m_hibernationMethod;
}

void TDERootSystemDevice::internalSetHibernationMethod(TDESystemHibernationMethod::TDESystemHibernationMethod hm) {
	m_hibernationMethod = hm;
}

unsigned long TDERootSystemDevice::diskSpaceNeededForHibernation() {
	return m_hibernationSpace;
}

void TDERootSystemDevice::internalSetDiskSpaceNeededForHibernation(unsigned long sz) {
	m_hibernationSpace = sz;
}

bool TDERootSystemDevice::canSetHibernationMethod() {
	TQString hibernationnode = "/sys/power/disk";
	int rval = access (hibernationnode.ascii(), W_OK);
	if (rval == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

bool TDERootSystemDevice::canStandby() {
	TQString statenode = "/sys/power/state";
	int rval = access (statenode.ascii(), W_OK);
	if (rval == 0) {
		if (powerStates().contains(TDESystemPowerState::Standby)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

bool TDERootSystemDevice::canSuspend() {
	TQString statenode = "/sys/power/state";
	int rval = access (statenode.ascii(), W_OK);
	if (rval == 0) {
		if (powerStates().contains(TDESystemPowerState::Suspend)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
#ifdef WITH_UPOWER
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy upowerProperties("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", dbusConn);
			if (upowerProperties.canSend()) {
				// can suspend?
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(upowerProperties.interface()) << TQT_DBusData::fromString("CanSuspend");
				TQT_DBusMessage reply = upowerProperties.sendWithReply("Get", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toVariant().value.toBool();
				}
				else {
					return FALSE;
				}
			}
			else {
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
#else // WITH_UPOWER
		return FALSE;
#endif// WITH_UPOWER
	}
}

bool TDERootSystemDevice::canHibernate() {
	TQString statenode = "/sys/power/state";
	int rval = access (statenode.ascii(), W_OK);
	if (rval == 0) {
		if (powerStates().contains(TDESystemPowerState::Hibernate)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
#ifdef WITH_UPOWER
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy upowerProperties("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", dbusConn);
			if (upowerProperties.canSend()) {
				// can hibernate?
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(upowerProperties.interface()) << TQT_DBusData::fromString("CanHibernate");
				TQT_DBusMessage reply = upowerProperties.sendWithReply("Get", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toVariant().value.toBool();
				}
				else {
					return FALSE;
				}
			}
			else {
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
#else // WITH_UPOWER
		return FALSE;
#endif// WITH_UPOWER
	}
}

bool TDERootSystemDevice::canPowerOff() {
	TDEConfig *config = TDEGlobal::config();
	config->reparseConfiguration(); // config may have changed in the KControl module
	
	config->setGroup("General" );
	bool maysd = false;
#ifdef WITH_CONSOLEKIT
	if (config->readBoolEntry( "offerShutdown", true )) {
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy consoleKitManager("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", dbusConn);
			if (consoleKitManager.canSend()) {
				// can power off?
				TQValueList<TQT_DBusData> params;
				TQT_DBusMessage reply = consoleKitManager.sendWithReply("CanStop", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					maysd = reply[0].toBool();
				}
				else {
					maysd = false;
				}
			}
			else {
				maysd = false;
			}
		}
		else {
			maysd = false;
		}
	}
#else // WITH_CONSOLEKIT
	// FIXME
	// Can we power down this system?
	// This should probably be checked via DCOP and therefore interface with KDM
	if (config->readBoolEntry( "offerShutdown", true )/* && DM().canShutdown()*/) {	// FIXME
		maysd = true;
	}
#endif // WITH_CONSOLEKIT

	return maysd;
}

bool TDERootSystemDevice::canReboot() {
	TDEConfig *config = TDEGlobal::config();
	config->reparseConfiguration(); // config may have changed in the KControl module
	
	config->setGroup("General" );
	bool mayrb = false;
#ifdef WITH_CONSOLEKIT
	if (config->readBoolEntry( "offerShutdown", true )) {
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy consoleKitManager("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", dbusConn);
			if (consoleKitManager.canSend()) {
				// can reboot?
				TQValueList<TQT_DBusData> params;
				TQT_DBusMessage reply = consoleKitManager.sendWithReply("CanRestart", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					mayrb = reply[0].toBool();
				}
				else {
					mayrb = false;
				}
			}
			else {
				mayrb = false;
			}
		}
		else {
			mayrb = false;
		}
	}
#else // WITH_CONSOLEKIT
	// FIXME
	// Can we power down this system?
	// This should probably be checked via DCOP and therefore interface with KDM
	if (config->readBoolEntry( "offerShutdown", true )/* && DM().canShutdown()*/) {	// FIXME
		mayrb = true;
	}
#endif // WITH_CONSOLEKIT

	return mayrb;
}

void TDERootSystemDevice::setHibernationMethod(TDESystemHibernationMethod::TDESystemHibernationMethod hm) {
	TQString hibernationnode = "/sys/power/disk";
	TQFile file( hibernationnode );
	if ( file.open( IO_WriteOnly ) ) {
		TQString hibernationCommand;
		if (hm == TDESystemHibernationMethod::Platform) {
			hibernationCommand = "platform";
		}
		if (hm == TDESystemHibernationMethod::Shutdown) {
			hibernationCommand = "shutdown";
		}
		if (hm == TDESystemHibernationMethod::Reboot) {
			hibernationCommand = "reboot";
		}
		if (hm == TDESystemHibernationMethod::TestProc) {
			hibernationCommand = "testproc";
		}
		if (hm == TDESystemHibernationMethod::Test) {
			hibernationCommand = "test";
		}
		TQTextStream stream( &file );
		stream << hibernationCommand;
		file.close();
	}
}

bool TDERootSystemDevice::setPowerState(TDESystemPowerState::TDESystemPowerState ps) {
	if ((ps == TDESystemPowerState::Standby) || (ps == TDESystemPowerState::Suspend) || (ps == TDESystemPowerState::Hibernate)) {
		TQString statenode = "/sys/power/state";
		TQFile file( statenode );
		if ( file.open( IO_WriteOnly ) ) {
			TQString powerCommand;
			if (ps == TDESystemPowerState::Standby) {
				powerCommand = "standby";
			}
			if (ps == TDESystemPowerState::Suspend) {
				powerCommand = "mem";
			}
			if (ps == TDESystemPowerState::Hibernate) {
				powerCommand = "disk";
			}
			TQTextStream stream( &file );
			stream << powerCommand;
			file.close();
			return true;
		}
		else {
#ifdef WITH_UPOWER
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				if (ps == TDESystemPowerState::Suspend) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.UPower",
								"/org/freedesktop/UPower",
								"org.freedesktop.UPower",
								"Suspend");
					dbusConn.sendWithReply(msg);
					return true;
				}
				else if (ps == TDESystemPowerState::Hibernate) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.UPower",
								"/org/freedesktop/UPower",
								"org.freedesktop.UPower",
								"Hibernate");
					dbusConn.sendWithReply(msg);
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
#else // WITH_UPOWER
			return false;
#endif // WITH_UPOWER
		}
	}
	else if (ps == TDESystemPowerState::PowerOff) {
#ifdef WITH_CONSOLEKIT
		TDEConfig *config = TDEGlobal::config();
		config->reparseConfiguration(); // config may have changed in the KControl module
		config->setGroup("General" );
		if (config->readBoolEntry( "offerShutdown", true )) {
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
							"org.freedesktop.ConsoleKit",
							"/org/freedesktop/ConsoleKit/Manager",
							"org.freedesktop.ConsoleKit.Manager",
							"Stop");
				dbusConn.sendWithReply(msg);
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
#else // WITH_CONSOLEKIT
		// Power down the system using a DCOP command
		// Values are explained at http://lists.kde.org/?l=kde-linux&m=115770988603387
		TQByteArray data;
		TQDataStream arg(data, IO_WriteOnly);
		arg << (int)0 << (int)2 << (int)2;
		if ( kapp->dcopClient()->send("ksmserver", "default", "logout(int,int,int)", data) ) {
			return true;
		}
		return false;
#endif // WITH_CONSOLEKIT
	}
	else if (ps == TDESystemPowerState::Reboot) {
#ifdef WITH_CONSOLEKIT
		TDEConfig *config = TDEGlobal::config();
		config->reparseConfiguration(); // config may have changed in the KControl module
		config->setGroup("General" );
		if (config->readBoolEntry( "offerShutdown", true )) {
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
							"org.freedesktop.ConsoleKit",
							"/org/freedesktop/ConsoleKit/Manager",
							"org.freedesktop.ConsoleKit.Manager",
							"Restart");
				dbusConn.sendWithReply(msg);
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
#else // WITH_CONSOLEKIT
		// Power down the system using a DCOP command
		// Values are explained at http://lists.kde.org/?l=kde-linux&m=115770988603387
		TQByteArray data;
		TQDataStream arg(data, IO_WriteOnly);
		arg << (int)0 << (int)1 << (int)2;
		if ( kapp->dcopClient()->send("ksmserver", "default", "logout(int,int,int)", data) ) {
			return true;
		}
		return false;
#endif // WITH_CONSOLEKIT
	}
	else if (ps == TDESystemPowerState::Active) {
		// Ummm...we're already active...
		return true;
	}

	return false;
}

#include "rootsystemdevice.moc"
