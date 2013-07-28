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

#include "monitordevice.h"

#include "config.h"

using namespace TDEHW;

MonitorDevice::MonitorDevice(GenericDeviceType::GenericDeviceType dt, TQString dn) : GenericDevice(dt, dn) {
}

MonitorDevice::~MonitorDevice() {
}

bool MonitorDevice::connected() {
	return m_connected;
}

void MonitorDevice::internalSetConnected(bool cn) {
	m_connected = cn;
}

bool MonitorDevice::enabled() {
	return m_enabled;
}

void MonitorDevice::internalSetEnabled(bool en) {
	m_enabled = en;
}

TQByteArray MonitorDevice::edid() {
	return m_edid;
}

void MonitorDevice::internalSetEdid(TQByteArray ed) {
	m_edid = ed;
}

ResolutionList MonitorDevice::resolutions() {
	return m_resolutions;
}

void MonitorDevice::internalSetResolutions(ResolutionList rs) {
	m_resolutions = rs;
}

TQString MonitorDevice::portType() {
	return m_portType;
}

void MonitorDevice::internalSetPortType(TQString pt) {
	m_portType = pt;
}

DisplayPowerLevel::DisplayPowerLevel MonitorDevice::powerLevel() {
	return m_powerLevel;
}

void MonitorDevice::internalSetPowerLevel(DisplayPowerLevel::DisplayPowerLevel pl) {
	m_powerLevel = pl;
}

#include "monitordevice.moc"
