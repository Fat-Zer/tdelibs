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

#include "tdeeventdevice.h"

#include <unistd.h>
#include <linux/input.h>

#include <tqsocketnotifier.h>

#include "tdelocale.h"

#include "tdehardwaredevices.h"

#include "config.h"

using namespace TDEHW;

TDEEventDevice::TDEEventDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
	m_fd = -1;
	m_fdMonitorActive = false;
}

TDEEventDevice::~TDEEventDevice() {
	if (m_fd >= 0) {
		close(m_fd);
	}
}

TDEEventDeviceType::TDEEventDeviceType TDEEventDevice::eventType() {
	return m_eventType;
}

void TDEEventDevice::internalSetEventType(TDEEventDeviceType::TDEEventDeviceType et) {
	m_eventType = et;
}

TDESwitchType::TDESwitchType TDEEventDevice::providedSwitches() {
	return m_providedSwitches;
}

void TDEEventDevice::internalSetProvidedSwitches(TDESwitchType::TDESwitchType sl) {
	m_providedSwitches = sl;
}

TDESwitchType::TDESwitchType TDEEventDevice::activeSwitches() {
	return m_switchActive;
}

void TDEEventDevice::internalSetActiveSwitches(TDESwitchType::TDESwitchType sl) {
	m_switchActive = sl;
}

// Keep this in sync with the TDESwitchType definition in the header
TQStringList TDEEventDevice::friendlySwitchList(TDESwitchType::TDESwitchType switches) {
	TQStringList ret;

	if (switches & TDESwitchType::Lid) {
		ret.append(i18n("Lid Switch"));
	}
	if (switches & TDESwitchType::TabletMode) {
		ret.append(i18n("Tablet Mode"));
	}
	if (switches & TDESwitchType::HeadphoneInsert) {
		ret.append(i18n("Headphone Inserted"));
	}
	if (switches & TDESwitchType::RFKill) {
		ret.append(i18n("Radio Frequency Device Kill Switch"));
	}
	if (switches & TDESwitchType::Radio) {
		ret.append(i18n("Enable Radio"));
	}
	if (switches & TDESwitchType::MicrophoneInsert) {
		ret.append(i18n("Microphone Inserted"));
	}
	if (switches & TDESwitchType::Dock) {
		ret.append(i18n("Docked"));
	}
	if (switches & TDESwitchType::LineOutInsert) {
		ret.append(i18n("Line Out Inserted"));
	}
	if (switches & TDESwitchType::JackPhysicalInsert) {
		ret.append(i18n("Physical Jack Inserted"));
	}
	if (switches & TDESwitchType::VideoOutInsert) {
		ret.append(i18n("Video Out Inserted"));
	}
	if (switches & TDESwitchType::CameraLensCover) {
		ret.append(i18n("Camera Lens Cover"));
	}
	if (switches & TDESwitchType::KeypadSlide) {
		ret.append(i18n("Keypad Slide"));
	}
	if (switches & TDESwitchType::FrontProximity) {
		ret.append(i18n("Front Proximity"));
	}
	if (switches & TDESwitchType::RotateLock) {
		ret.append(i18n("Rotate Lock"));
	}
	if (switches & TDESwitchType::LineInInsert) {
		ret.append(i18n("Line In Inserted"));
	}
	if (switches & TDESwitchType::PowerButton) {
		ret.append(i18n("Power Button"));
	}
	if (switches & TDESwitchType::SleepButton) {
		ret.append(i18n("Sleep Button"));
	}

	return ret;
}

void TDEEventDevice::internalStartFdMonitoring(TDEHardwareDevices* hwmanager) {
	if (!m_fdMonitorActive) {
		// For security and performance reasons, only monitor known ACPI buttons
		if (eventType() != TDEEventDeviceType::Unknown) {
			if (m_fd >= 0) {
				m_eventNotifier = new TQSocketNotifier(m_fd, TQSocketNotifier::Read, this);
				connect( m_eventNotifier, TQT_SIGNAL(activated(int)), this, TQT_SLOT(eventReceived()) );
			}
			connect( this, TQT_SIGNAL(keyPressed(unsigned int, TDEEventDevice*)), hwmanager, TQT_SLOT(processEventDeviceKeyPressed(unsigned int, TDEEventDevice*)) );
		}
		m_fdMonitorActive = true;
	}
}

void TDEEventDevice::eventReceived() {
	struct input_event ev;
	int r;
	r = read(m_fd, &ev, sizeof(struct input_event));
	if (r > 0) {
		if ((ev.type == EV_KEY) && (ev.value == 1)) {	// Only detect keypress events (value == 1)
			emit keyPressed(ev.code, this);
		}
	}
}

#include "tdeeventdevice.moc"
