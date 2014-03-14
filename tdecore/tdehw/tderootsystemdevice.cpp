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

#include "tderootsystemdevice.h"
#include "tdestoragedevice.h"

#include <unistd.h>

#include <tqfile.h>

#include <dcopclient.h>
#include "tdeglobal.h"
#include "tdeconfig.h"
#include "tdeapplication.h"
#include "kstandarddirs.h"

#include "config.h"

#if defined(WITH_TDEHWLIB_DAEMONS) || defined(WITH_UPOWER) || defined(WITH_DEVKITPOWER) || defined(WITH_HAL) || defined(WITH_CONSOLEKIT)
	#include <tqdbusdata.h>
	#include <tqdbusmessage.h>
	#include <tqdbusproxy.h>
	#include <tqdbusvariant.h>
	#include <tqdbusconnection.h>
#endif // defined(WITH_TDEHWLIB_DAEMONS) || defined(WITH_UPOWER) || defined(WITH_DEVKITPOWER) || defined(WITH_HAL) || defined(WITH_CONSOLEKIT)

bool isNetworkFileSystem(TQString fileSystemType) {
	if ((fileSystemType.startsWith("nfs"))
		|| (fileSystemType == "cifs")
		) {
		return TRUE;
	}

	return FALSE;
}

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

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can set hibernation method?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.trinitydesktop.hardwarecontrol",
						"/org/trinitydesktop/hardwarecontrol",
						"org.trinitydesktop.hardwarecontrol.Power",
						"CanSetHibernationMethod");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return reply[0].toBool();
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

	return FALSE;
}

bool TDERootSystemDevice::canStandby() {
	// Network file systems mounted on $HOME typically cause nasty suspend/resume failures
	// See Bug 1615 for details
	if (isNetworkFileSystem(TDEStorageDevice::determineFileSystemType(TDEGlobal::dirs()->localtdedir()))) {
		return FALSE;
	}

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

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can standby?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.trinitydesktop.hardwarecontrol",
						"/org/trinitydesktop/hardwarecontrol",
						"org.trinitydesktop.hardwarecontrol.Power",
						"CanStandby");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return reply[0].toBool();
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

	return FALSE;
}

bool TDERootSystemDevice::canFreeze() {
	// Network file systems mounted on $HOME typically cause nasty suspend/resume failures
	// See Bug 1615 for details
	if (isNetworkFileSystem(TDEStorageDevice::determineFileSystemType(TDEGlobal::dirs()->localtdedir()))) {
		return FALSE;
	}

	TQString statenode = "/sys/power/state";
	int rval = access (statenode.ascii(), W_OK);
	if (rval == 0) {
		if (powerStates().contains(TDESystemPowerState::Freeze)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can freeze?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.trinitydesktop.hardwarecontrol",
						"/org/trinitydesktop/hardwarecontrol",
						"org.trinitydesktop.hardwarecontrol.Power",
						"CanFreeze");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return reply[0].toBool();
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

	return FALSE;
}

bool TDERootSystemDevice::canSuspend() {
	// Network file systems mounted on $HOME typically cause nasty suspend/resume failures
	// See Bug 1615 for details
	if (isNetworkFileSystem(TDEStorageDevice::determineFileSystemType(TDEGlobal::dirs()->localtdedir()))) {
		return FALSE;
	}

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

#ifdef WITH_LOGINDPOWER
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can suspend?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.freedesktop.login1",
						"/org/freedesktop/login1",
						"org.freedesktop.login1.Manager",
						"CanSuspend");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return (reply[0].toString() == "yes");
			}
		}
	}
#endif // WITH_LOGINDPOWER

#ifdef WITH_UPOWER
	{
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
			}
		}
	}
#endif// WITH_UPOWER

#ifdef WITH_DEVKITPOWER
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy devkitpowerProperties("org.freedesktop.DeviceKit.Power", "/org/freedesktop/DeviceKit/Power", "org.freedesktop.DBus.Properties", dbusConn);
			if (devkitpowerProperties.canSend()) {
				// can suspend?
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(devkitpowerProperties.interface()) << TQT_DBusData::fromString("CanSuspend");
				TQT_DBusMessage reply = devkitpowerProperties.sendWithReply("Get", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toVariant().value.toBool();
				}
			}
		}
	}
#endif// WITH_DEVKITPOWER

#ifdef WITH_HAL
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy halProperties("org.freedesktop.Hal", "/org/freedesktop/Hal/devices/computer", "org.freedesktop.Hal.Device", dbusConn);
			if (halProperties.canSend()) {
				// can suspend?
				TQValueList<TQT_DBusData> params;
				TQT_DBusMessage reply;
				params.clear();
				params << TQT_DBusData::fromString("power_management.can_suspend");
				reply = halProperties.sendWithReply("GetPropertyBoolean", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toBool();
				}
				params.clear();
				params << TQT_DBusData::fromString("power_management.can_suspend_to_ram");
				reply = halProperties.sendWithReply("GetPropertyBoolean", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toBool();
				}
			}
		}
	}
#endif // WITH_HAL

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can suspend?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.trinitydesktop.hardwarecontrol",
						"/org/trinitydesktop/hardwarecontrol",
						"org.trinitydesktop.hardwarecontrol.Power",
						"CanSuspend");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return reply[0].toBool();
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

	return FALSE;
}

bool TDERootSystemDevice::canHibernate() {
	// Network file systems mounted on $HOME typically cause nasty suspend/resume failures
	// See Bug 1615 for details
	if (isNetworkFileSystem(TDEStorageDevice::determineFileSystemType(TDEGlobal::dirs()->localtdedir()))) {
		return FALSE;
	}

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

#ifdef WITH_LOGINDPOWER
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can hibernate?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.freedesktop.login1",
						"/org/freedesktop/login1",
						"org.freedesktop.login1.Manager",
						"CanHibernate");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return (reply[0].toString() == "yes");
			}
		}
	}
#endif // WITH_LOGINDPOWER

#ifdef WITH_UPOWER
	{
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
			}
		}
	}
#endif// WITH_UPOWER

#ifdef WITH_DEVKITPOWER
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy devkitpowerProperties("org.freedesktop.DeviceKit.Power", "/org/freedesktop/DeviceKit/Power", "org.freedesktop.DBus.Properties", dbusConn);
			if (devkitpowerProperties.canSend()) {
				// can hibernate?
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(devkitpowerProperties.interface()) << TQT_DBusData::fromString("CanHibernate");
				TQT_DBusMessage reply = devkitpowerProperties.sendWithReply("Get", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toVariant().value.toBool();
				}
			}
		}
	}
#endif// WITH_DEVKITPOWER

#ifdef WITH_HAL
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy halProperties("org.freedesktop.Hal", "/org/freedesktop/Hal/devices/computer", "org.freedesktop.Hal.Device", dbusConn);
			if (halProperties.canSend()) {
				// can hibernate?
				TQValueList<TQT_DBusData> params;
				TQT_DBusMessage reply;
				params.clear();
				params << TQT_DBusData::fromString("power_management.can_hibernate");
				reply = halProperties.sendWithReply("GetPropertyBoolean", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toBool();
				}
				params.clear();
				params << TQT_DBusData::fromString("power_management.can_suspend_to_disk");
				reply = halProperties.sendWithReply("GetPropertyBoolean", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toBool();
				}
			}
		}
	}
#endif // WITH_HAL

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can hibernate?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.trinitydesktop.hardwarecontrol",
						"/org/trinitydesktop/hardwarecontrol",
						"org.trinitydesktop.hardwarecontrol.Power",
						"CanHibernate");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return reply[0].toBool();
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

	return FALSE;
}

bool TDERootSystemDevice::canPowerOff() {
	TDEConfig config("ksmserverrc", true);
	config.setGroup("General" );
	if (!config.readBoolEntry( "offerShutdown", true )) {
		return FALSE;
	}

#ifdef WITH_LOGINDPOWER
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can power off?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.freedesktop.login1",
						"/org/freedesktop/login1",
						"org.freedesktop.login1.Manager",
						"CanPowerOff");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return (reply[0].toString() == "yes");
			}
		}
	}
#endif // WITH_LOGINDPOWER

#ifdef WITH_CONSOLEKIT
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can power off?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.freedesktop.ConsoleKit",
						"/org/freedesktop/ConsoleKit/Manager",
						"org.freedesktop.ConsoleKit.Manager",
						"CanStop");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return reply[0].toBool();
			}
		}
	}
#endif // WITH_CONSOLEKIT

	// FIXME
	// Can we power down this system?
	// This should probably be checked via DCOP and therefore interface with TDM
	// if ( DM().canShutdown() ) {
	// 	return TRUE;
	// }
	return TRUE;
}

bool TDERootSystemDevice::canReboot() {
	TDEConfig config("ksmserverrc", true);
	config.setGroup("General" );
	if (!config.readBoolEntry( "offerShutdown", true )) {
		return FALSE;
	}

#ifdef WITH_LOGINDPOWER
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can reboot?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.freedesktop.login1",
						"/org/freedesktop/login1",
						"org.freedesktop.login1.Manager",
						"CanReboot");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return (reply[0].toString() == "yes");
			}
		}
	}
#endif // WITH_LOGINDPOWER

#ifdef WITH_CONSOLEKIT
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			// can reboot?
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.freedesktop.ConsoleKit",
						"/org/freedesktop/ConsoleKit/Manager",
						"org.freedesktop.ConsoleKit.Manager",
						"CanRestart");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return reply[0].toBool();
			}
		}
	}
#endif // WITH_CONSOLEKIT

	// FIXME
	// Can we power down this system?
	// This should probably be checked via DCOP and therefore interface with TDM
	// if ( DM().canShutdown() ) {
	// 	return TRUE;
	// }
	return TRUE;
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
		return;
	}

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy hardwareControl("org.trinitydesktop.hardwarecontrol", "/org/trinitydesktop/hardwarecontrol", "org.trinitydesktop.hardwarecontrol.Power", dbusConn);
			if (hardwareControl.canSend()) {
				// set hibernation method
				TQValueList<TQT_DBusData> params;
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
				params << TQT_DBusData::fromString(hibernationCommand);
				TQT_DBusMessage reply = hardwareControl.sendWithReply("SetHibernationMethod", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage) {
					return;
				}
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

}

bool TDERootSystemDevice::setPowerState(TDESystemPowerState::TDESystemPowerState ps) {
	if ((ps == TDESystemPowerState::Standby) || (ps == TDESystemPowerState::Freeze) || (ps == TDESystemPowerState::Suspend) || (ps == TDESystemPowerState::Hibernate)) {
		TQString statenode = "/sys/power/state";
		TQFile file( statenode );
		if ( file.open( IO_WriteOnly ) ) {
			TQString powerCommand;
			if (ps == TDESystemPowerState::Standby) {
				powerCommand = "standby";
			}
			if (ps == TDESystemPowerState::Freeze) {
				powerCommand = "freeze";
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

#ifdef WITH_LOGINDPOWER
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				if (ps == TDESystemPowerState::Suspend) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.login1",
								"/org/freedesktop/login1",
								"org.freedesktop.login1.Manager",
								"Suspend");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
				else if (ps == TDESystemPowerState::Hibernate) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.login1",
								"/org/freedesktop/login1",
								"org.freedesktop.login1.Manager",
								"Hibernate");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
			}
		}
#endif // WITH_LOGINDPOWER

#ifdef WITH_UPOWER
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				if (ps == TDESystemPowerState::Suspend) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.UPower",
								"/org/freedesktop/UPower",
								"org.freedesktop.UPower",
								"Suspend");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
				else if (ps == TDESystemPowerState::Hibernate) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.UPower",
								"/org/freedesktop/UPower",
								"org.freedesktop.UPower",
								"Hibernate");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
			}
		}
#endif // WITH_UPOWER

#ifdef WITH_DEVKITPOWER
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				if (ps == TDESystemPowerState::Suspend) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.DeviceKit.Power",
								"/org/freedesktop/DeviceKit/Power",
								"org.freedesktop.DeviceKit.Power",
								"Suspend");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
				else if (ps == TDESystemPowerState::Hibernate) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.DeviceKit.Power",
								"/org/freedesktop/DeviceKit/Power",
								"org.freedesktop.DeviceKit.Power",
								"Hibernate");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
			}
		}
#endif // WITH_DEVKITPOWER

#ifdef WITH_HAL
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				if (ps == TDESystemPowerState::Suspend) {
					TQT_DBusProxy halPowerManagement(
							"org.freedesktop.Hal",
							"/org/freedesktop/Hal/devices/computer",
							"org.freedesktop.Hal.Device.SystemPowerManagement",
							dbusConn);
					TQValueList<TQT_DBusData> params;
					params << TQT_DBusData::fromInt32(0);
					TQT_DBusMessage reply = halPowerManagement.sendWithReply("Suspend", params);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
				else if (ps == TDESystemPowerState::Hibernate) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.freedesktop.Hal",
								"/org/freedesktop/Hal/devices/computer",
								"org.freedesktop.Hal.Device.SystemPowerManagement",
								"Hibernate");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
			}
		}
#endif // WITH_HAL

#ifdef WITH_TDEHWLIB_DAEMONS
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				if (ps == TDESystemPowerState::Standby) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.trinitydesktop.hardwarecontrol",
								"/org/trinitydesktop/hardwarecontrol",
								"org.trinitydesktop.hardwarecontrol.Power",
								"Standby");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
				else if (ps == TDESystemPowerState::Freeze) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.trinitydesktop.hardwarecontrol",
								"/org/trinitydesktop/hardwarecontrol",
								"org.trinitydesktop.hardwarecontrol.Power",
								"Freeze");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
				else if (ps == TDESystemPowerState::Suspend) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.trinitydesktop.hardwarecontrol",
								"/org/trinitydesktop/hardwarecontrol",
								"org.trinitydesktop.hardwarecontrol.Power",
								"Suspend");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
				else if (ps == TDESystemPowerState::Hibernate) {
					TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
								"org.trinitydesktop.hardwarecontrol",
								"/org/trinitydesktop/hardwarecontrol",
								"org.trinitydesktop.hardwarecontrol.Power",
								"Hibernate");
					TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
					if (reply.type() == TQT_DBusMessage::ReplyMessage) {
						return true;
					}
				}
			}
		}
#endif // WITH_TDEHWLIB_DAEMONS

		return false;
	}
	else if (ps == TDESystemPowerState::PowerOff) {
		TDEConfig config("ksmserverrc", true);
		config.setGroup("General" );
		if (!config.readBoolEntry( "offerShutdown", true )) {
			return false;
		}
#ifdef WITH_LOGINDPOWER
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
							"org.trinitydesktop.login1",
							"/org/freedesktop/login1",
							"org.freedesktop.login1.Manager",
							"PowerOff");
				TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
				if (reply.type() == TQT_DBusMessage::ReplyMessage) {
					return true;
				}
			}
		}
#endif // WITH_LOGINDPOWER
#ifdef WITH_CONSOLEKIT
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
							"org.freedesktop.ConsoleKit",
							"/org/freedesktop/ConsoleKit/Manager",
							"org.freedesktop.ConsoleKit.Manager",
							"Stop");
				TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
				if (reply.type() == TQT_DBusMessage::ReplyMessage) {
					return true;
				}
			}
		}
#endif // WITH_CONSOLEKIT
		// Power down the system using a DCOP command
		// Values are explained at http://lists.kde.org/?l=kde-linux&m=115770988603387
		TQByteArray data;
		TQDataStream arg(data, IO_WriteOnly);
		arg << (int)0 << (int)2 << (int)2;
		if ( kapp->dcopClient()->send("ksmserver", "default", "logout(int,int,int)", data) ) {
			return true;
		}
		return false;
	}
	else if (ps == TDESystemPowerState::Reboot) {
		TDEConfig config("ksmserverrc", true);
		config.setGroup("General" );
		if (!config.readBoolEntry( "offerShutdown", true )) {
			return false;
		}
#ifdef WITH_LOGINDPOWER
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
							"org.trinitydesktop.login1",
							"/org/freedesktop/login1",
							"org.freedesktop.login1.Manager",
							"Reboot");
				TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
				if (reply.type() == TQT_DBusMessage::ReplyMessage) {
					return true;
				}
			}
		}
#endif // WITH_LOGINDPOWER
#ifdef WITH_CONSOLEKIT
		{
			TQT_DBusConnection dbusConn;
			dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
			if ( dbusConn.isConnected() ) {
				TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
							"org.freedesktop.ConsoleKit",
							"/org/freedesktop/ConsoleKit/Manager",
							"org.freedesktop.ConsoleKit.Manager",
							"Restart");
				TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
				if (reply.type() == TQT_DBusMessage::ReplyMessage) {
					return true;
				}
			}
		}
#endif // WITH_CONSOLEKIT
		// Power down the system using a DCOP command
		// Values are explained at http://lists.kde.org/?l=kde-linux&m=115770988603387
		TQByteArray data;
		TQDataStream arg(data, IO_WriteOnly);
		arg << (int)0 << (int)1 << (int)2;
		if ( kapp->dcopClient()->send("ksmserver", "default", "logout(int,int,int)", data) ) {
			return true;
		}
		return false;
	}
	else if (ps == TDESystemPowerState::Active) {
		// Ummm...we're already active...
		return true;
	}

	return false;
}

#include "tderootsystemdevice.moc"
