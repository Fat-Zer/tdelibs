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

#include "batterydevice.h"

#include "config.h"

using namespace TDEHW;

BatteryDevice::BatteryDevice(GenericDeviceType::GenericDeviceType dt, TQString dn) : GenericDevice(dt, dn) {
}

BatteryDevice::~BatteryDevice() {
}

double BatteryDevice::voltage() {
	return m_currentVoltage;
}

void BatteryDevice::internalSetVoltage(double vt) {
	m_currentVoltage = vt;
}

double BatteryDevice::maximumVoltage() {
	return m_maximumVoltage;
}

void BatteryDevice::internalSetMaximumVoltage(double vt) {
	m_maximumVoltage = vt;
}

double BatteryDevice::minimumVoltage() {
	return m_minimumVoltage;
}

void BatteryDevice::internalSetMinimumVoltage(double vt) {
	m_minimumVoltage = vt;
}

double BatteryDevice::maximumDesignVoltage() {
	return m_maximumDesignVoltage;
}

void BatteryDevice::internalSetMaximumDesignVoltage(double vt) {
	m_maximumDesignVoltage = vt;
}

double BatteryDevice::energy() {
	return m_currentEnergy;
}

void BatteryDevice::internalSetEnergy(double vt) {
	m_currentEnergy = vt;
}

double BatteryDevice::alarmEnergy() {
	return m_alarmEnergy;
}

void BatteryDevice::internalSetAlarmEnergy(double vt) {
	m_alarmEnergy = vt;
}

double BatteryDevice::maximumEnergy() {
	return m_maximumEnergy;
}

void BatteryDevice::internalSetMaximumEnergy(double vt) {
	m_maximumEnergy = vt;
}

double BatteryDevice::maximumDesignEnergy() {
	return m_maximumDesignEnergy;
}

void BatteryDevice::internalSetMaximumDesignEnergy(double vt) {
	m_maximumDesignEnergy = vt;
}

double BatteryDevice::dischargeRate() {
	return m_dischargeRate;
}

void BatteryDevice::internalSetDischargeRate(double vt) {
	m_dischargeRate = vt;
}

double BatteryDevice::timeRemaining() {
	return m_timeRemaining;
}

void BatteryDevice::internalSetTimeRemaining(double tr) {
	m_timeRemaining = tr;
}

TQString BatteryDevice::technology() {
	return m_technology;
}

void BatteryDevice::internalSetTechnology(TQString tc) {
	m_technology = tc;
}

BatteryStatus::BatteryStatus BatteryDevice::status() {
	return m_status;
}

void BatteryDevice::internalSetStatus(TQString tc) {
	tc = tc.lower();

	if (tc == "charging") {
		m_status = BatteryStatus::Charging;
	}
	else if (tc == "discharging") {
		m_status = BatteryStatus::Discharging;
	}
	else if (tc == "full") {
		m_status = BatteryStatus::Full;
	}
	else {
		m_status = BatteryStatus::Unknown;
	}
}

bool BatteryDevice::installed() {
	return m_installed;
}

void BatteryDevice::internalSetInstalled(bool tc) {
	m_installed = tc;
}

double BatteryDevice::chargePercent() {
	return (m_currentEnergy/m_maximumEnergy)*100.0;
}

#include "batterydevice.moc"
