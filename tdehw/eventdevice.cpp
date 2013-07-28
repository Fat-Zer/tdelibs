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

#include "eventdevice.h"

#include <unistd.h>
#include <linux/input.h>

#include <tqsocketnotifier.h>

#include "tdelocale.h"

#include "hardwaredevices.h"

#include "config.h"

using namespace TDEHW;

EventDevice::EventDevice(GenericDeviceType::GenericDeviceType dt, TQString dn) : GenericDevice(dt, dn) {
	m_fd = -1;
	m_fdMonitorActive = false;
}

EventDevice::~EventDevice() {
	if (m_fd >= 0) {
		close(m_fd);
	}
}

EventDeviceType::EventDeviceType EventDevice::eventType() {
	return m_eventType;
}

void EventDevice::internalSetEventType(EventDeviceType::EventDeviceType et) {
	m_eventType = et;
}

SwitchType::SwitchType EventDevice::providedSwitches() {
	return m_providedSwitches;
}

void EventDevice::internalSetProvidedSwitches(SwitchType::SwitchType sl) {
	m_providedSwitches = sl;
}

SwitchType::SwitchType EventDevice::activeSwitches() {
	return m_switchActive;
}

void EventDevice::internalSetActiveSwitches(SwitchType::SwitchType sl) {
	m_switchActive = sl;
}

// Keep this in sync with the SwitchType definition in the header
TQStringList EventDevice::friendlySwitchList(SwitchType::SwitchType switches) {
	TQStringList ret;

	if (switches & SwitchType::Lid) {
		ret.append(i18n("Lid Switch"));
	}
	if (switches & SwitchType::TabletMode) {
		ret.append(i18n("Tablet Mode"));
	}
	if (switches & SwitchType::HeadphoneInsert) {
		ret.append(i18n("Headphone Inserted"));
	}
	if (switches & SwitchType::RFKill) {
		ret.append(i18n("Radio Frequency Device Kill Switch"));
	}
	if (switches & SwitchType::Radio) {
		ret.append(i18n("Enable Radio"));
	}
	if (switches & SwitchType::MicrophoneInsert) {
		ret.append(i18n("Microphone Inserted"));
	}
	if (switches & SwitchType::Dock) {
		ret.append(i18n("Docked"));
	}
	if (switches & SwitchType::LineOutInsert) {
		ret.append(i18n("Line Out Inserted"));
	}
	if (switches & SwitchType::JackPhysicalInsert) {
		ret.append(i18n("Physical Jack Inserted"));
	}
	if (switches & SwitchType::VideoOutInsert) {
		ret.append(i18n("Video Out Inserted"));
	}
	if (switches & SwitchType::CameraLensCover) {
		ret.append(i18n("Camera Lens Cover"));
	}
	if (switches & SwitchType::KeypadSlide) {
		ret.append(i18n("Keypad Slide"));
	}
	if (switches & SwitchType::FrontProximity) {
		ret.append(i18n("Front Proximity"));
	}
	if (switches & SwitchType::RotateLock) {
		ret.append(i18n("Rotate Lock"));
	}
	if (switches & SwitchType::LineInInsert) {
		ret.append(i18n("Line In Inserted"));
	}
	if (switches & SwitchType::PowerButton) {
		ret.append(i18n("Power Button"));
	}
	if (switches & SwitchType::SleepButton) {
		ret.append(i18n("Sleep Button"));
	}

	return ret;
}

void EventDevice::internalStartFdMonitoring(HardwareDevices* hwmanager) {
	if (!m_fdMonitorActive) {
		// For security and performance reasons, only monitor known ACPI buttons
		if (eventType() != EventDeviceType::Unknown) {
			if (m_fd >= 0) {
				m_eventNotifier = new TQSocketNotifier(m_fd, TQSocketNotifier::Read, this);
				connect( m_eventNotifier, TQT_SIGNAL(activated(int)), this, TQT_SLOT(eventReceived()) );
			}
			connect( this, TQT_SIGNAL(keyPressed(unsigned int, EventDevice*)), hwmanager, TQT_SLOT(processEventDeviceKeyPressed(unsigned int, EventDevice*)) );
		}
		m_fdMonitorActive = true;
	}
}

void EventDevice::eventReceived() {
	struct input_event ev;
	int r;
	r = read(m_fd, &ev, sizeof(struct input_event));
	if (r > 0) {
		if ((ev.type == EV_KEY) && (ev.value == 1)) {	// Only detect keypress events (value == 1)
			emit keyPressed(ev.code, this);
		}
	}
}

#include "eventdevice.moc"
