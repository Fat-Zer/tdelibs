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

#include "cpudevice.h"

#include <unistd.h>

#include <tqfile.h>

#include "hardwaredevices.h"

#include "config.h"

// uPower
#if defined(WITH_TDEHWLIB_DAEMONS) || defined(WITH_HAL)
 	#include <tqdbusdata.h>
 	#include <tqdbusmessage.h>
 	#include <tqdbusproxy.h>
 	#include <tqdbusvariant.h>
 	#include <tqdbusconnection.h>
#endif // defined(WITH_TDEHWLIB_DAEMONS) || defined(WITH_HAL)

using namespace TDEHW;

CPUDevice::CPUDevice(GenericDeviceType::GenericDeviceType dt, TQString dn) : GenericDevice(dt, dn) {
	m_frequency = -1;
	m_minfrequency = -1;
	m_maxfrequency = -1;
	m_corenumber = -1;
	m_transitionlatency = -1;
}

CPUDevice::~CPUDevice() {
}

double CPUDevice::frequency() {
	return m_frequency;
}

void CPUDevice::internalSetFrequency(double fr) {
	m_frequency = fr;
}

double CPUDevice::minFrequency() {
	return m_minfrequency;
}

void CPUDevice::internalSetMinFrequency(double fr) {
	m_minfrequency = fr;
}

double CPUDevice::maxFrequency() {
	return m_maxfrequency;
}

void CPUDevice::internalSetMaxFrequency(double fr) {
	m_maxfrequency = fr;
}

double CPUDevice::transitionLatency() {
	return m_transitionlatency;
}

void CPUDevice::internalSetTransitionLatency(double tl) {
	m_transitionlatency = tl;
}

TQString CPUDevice::governor() {
	return m_governor;
}

void CPUDevice::internalSetGovernor(TQString gr) {
	m_governor = gr;
}

TQString CPUDevice::scalingDriver() {
	return m_scalingdriver;
}

void CPUDevice::internalSetScalingDriver(TQString dr) {
	m_scalingdriver = dr;
}

TQStringList CPUDevice::dependentProcessors() {
	return m_tiedprocs;
}

void CPUDevice::internalSetDependentProcessors(TQStringList dp) {
	m_tiedprocs = dp;
}

TQStringList CPUDevice::availableFrequencies() {
	return m_frequencies;
}

void CPUDevice::internalSetAvailableFrequencies(TQStringList af) {
	m_frequencies = af;
}

TQStringList CPUDevice::availableGovernors() {
	return m_governers;
}

void CPUDevice::internalSetAvailableGovernors(TQStringList gp) {
	m_governers = gp;
}

void CPUDevice::internalSetCoreNumber(int cn) {
	m_corenumber = cn;
}

bool CPUDevice::canSetGovernor() {
	TQString governornode = systemPath() + "/cpufreq/scaling_governor";
	int rval = access (governornode.ascii(), W_OK);
	if (rval == 0) {
		return TRUE;
	}

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy hardwareControl("org.trinitydesktop.hardwarecontrol", "/org/trinitydesktop/hardwarecontrol", "org.trinitydesktop.hardwarecontrol.CPUGovernor", dbusConn);
			if (hardwareControl.canSend()) {
				// can set CPU governor?
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromInt32(coreNumber());
				TQT_DBusMessage reply = hardwareControl.sendWithReply("CanSetCPUGovernor", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toVariant().value.toBool();
				}
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

#ifdef WITH_HAL
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusMessage msg = TQT_DBusMessage::methodCall(
						"org.freedesktop.Hal",
						"/org/freedesktop/Hal/devices/computer",
						"org.freedesktop.Hal.Device.CPUFreq",
						"GetCPUFreqGovernor");
			TQT_DBusMessage reply = dbusConn.sendWithReply(msg);
			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				return true;
			}
		}
	}
#endif // WITH_HAL

	return FALSE;
}

void CPUDevice::setGovernor(TQString gv) {
	bool setGovernorDone = FALSE;

	TQString governornode = systemPath() + "/cpufreq/scaling_governor";
	TQFile file( governornode );
	if ( file.open( IO_WriteOnly ) ) {
		TQTextStream stream( &file );
		stream << gv.lower();
		file.close();
		setGovernorDone = TRUE;
	}

#ifdef WITH_TDEHWLIB_DAEMONS
	if ( !setGovernorDone ) {
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy hardwareControl("org.trinitydesktop.hardwarecontrol", "/org/trinitydesktop/hardwarecontrol", "org.trinitydesktop.hardwarecontrol.CPUGovernor", dbusConn);
			if (hardwareControl.canSend()) {
				// set CPU governor
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromInt32(coreNumber()) << TQT_DBusData::fromString(gv.lower());
				TQT_DBusMessage reply = hardwareControl.sendWithReply("SetCPUGovernor", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage) {
					setGovernorDone = TRUE;
				}
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

#ifdef WITH_HAL
	if ( !setGovernorDone ) {
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy cpuFreqControl("org.freedesktop.Hal", "/org/freedesktop/Hal/devices/computer", "org.freedesktop.Hal.Device.CPUFreq", dbusConn);
			if (cpuFreqControl.canSend()) {
				// set CPU governor
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(gv.lower());
				TQT_DBusMessage reply = cpuFreqControl.sendWithReply("SetCPUFreqGovernor", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					setGovernorDone = TRUE;
				}
			}
		}
	}
#endif // WITH_HAL

	// Force update of the device information object
	if ( setGovernorDone ) {
		HardwareDevices::instance()->processModifiedCPUs();
	}
}

bool CPUDevice::canSetMaximumScalingFrequency() {
	TQString freqnode = systemPath() + "/cpufreq/scaling_max_freq";
	int rval = access (freqnode.ascii(), W_OK);
	if (rval == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void CPUDevice::setMaximumScalingFrequency(double fr) {
	TQString freqnode = systemPath() + "/cpufreq/scaling_max_freq";
	TQFile file( freqnode );
	if ( file.open( IO_WriteOnly ) ) {
		TQTextStream stream( &file );
		stream << TQString("%1").arg(fr*1000000.0, 0, 'f', 0);
		file.close();
	}

	// Force update of the device information object
	HardwareDevices::instance()->processModifiedCPUs();
}

int CPUDevice::coreNumber() {
	return m_corenumber;
}

#include "cpudevice.moc"
