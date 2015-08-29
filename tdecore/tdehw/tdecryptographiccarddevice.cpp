/* This file is part of the TDE libraries
   Copyright (C) 2015 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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

#include "tdecryptographiccarddevice_private.h"
#include "tdecryptographiccarddevice.h"

#include <tqpixmap.h>
#include <tqtimer.h>
#include <ntqthread.h>
#include <ntqeventloop.h>
#include <ntqapplication.h>

#include "tdeglobal.h"
#include "tdelocale.h"

#include "tdehardwaredevices.h"

#include "config.h"

// 1 second
#define PCSC_POLL_TIMEOUT_S 1000

/* FIXME
 * This is incomplete
 */
#ifdef WITH_PCSC
static TQString pcsc_error_code_to_string(long errcode) {
	if (errcode == SCARD_W_UNPOWERED_CARD) {
		return i18n("card not powered on");
	}
	else if (errcode == SCARD_E_PROTO_MISMATCH) {
		return i18n("protocol mismatch");
	}
	else {
		return TQString::null;
	}
}
#endif

CryptoCardDeviceWatcher::CryptoCardDeviceWatcher() {
	m_readerStates = NULL;
}

CryptoCardDeviceWatcher::~CryptoCardDeviceWatcher() {
	free(m_readerStates);
}

void CryptoCardDeviceWatcher::run() {
#ifdef WITH_PCSC
	bool first_loop;
	unsigned int i;
	long ret;

	DWORD dword_readers;
	LPSTR lpstring_readers = NULL;

	TQStringList readers;

	first_loop = true;
	m_terminationRequested = false;

	TQEventLoop* eventLoop = TQApplication::eventLoop();
	if (!eventLoop) return;

	ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &m_cardContext);
	if (ret != SCARD_S_SUCCESS) {
		printf("TDECryptographicCardDevice: PCSC SCardEstablishContext cannot connect to resource manager (%lX)", ret);
		eventLoop->exit(0);
		return;
	}

	ret = SCardListReaders(m_cardContext, NULL, NULL, &dword_readers);
	if (ret == SCARD_S_SUCCESS) {
		lpstring_readers = (LPSTR)malloc(sizeof(char)*dword_readers);
		if (lpstring_readers == NULL) {
			printf("TDECryptographicCardDevice: insufficient memory, aborting");
			eventLoop->exit(0);
			return;
		}

		ret = SCardListReaders(m_cardContext, NULL, lpstring_readers, &dword_readers);
		if (ret == SCARD_S_SUCCESS) {
			/* Extract reader names from the null separated string */
			char *ptr = lpstring_readers;
			while (*ptr != '\0') {
				readers.append(ptr);
				ptr += strlen(ptr)+1;
			}

			free(lpstring_readers);

			m_readerStates = (SCARD_READERSTATE*)calloc(readers.count(), sizeof(*m_readerStates));
			if (m_readerStates == NULL) {
				printf("TDECryptographicCardDevice: insufficient memory, aborting");
				free(lpstring_readers);
				eventLoop->exit(0);
				return;
			}

			for (i=0; i<readers.count(); i++) {
				m_readerStates[i].szReader = strdup(readers[i].ascii());
				m_readerStates[i].dwCurrentState = SCARD_STATE_UNAWARE;
			}

			ret = SCardGetStatusChange(m_cardContext, PCSC_POLL_TIMEOUT_S, m_readerStates, readers.count());
			while ((ret == SCARD_S_SUCCESS) || (ret == SCARD_E_TIMEOUT)) {
				if (m_terminationRequested) {
					for (i=0; i<readers.count(); i++) {
						free((char*)m_readerStates[i].szReader);
					}
					eventLoop->exit(0);
					return;
				}

				for (i=0; i<readers.count(); i++) {
					/* FIXME
					 * Find a better / more reliable way to match the card low level device to the PCSC name
					 */
					if (!readers[i].contains(cardDevice->friendlyName())) {
						continue;
					}

					if (first_loop) {
						if (m_readerStates[i].dwEventState & SCARD_STATE_PRESENT) {
							// sleep(1);	// Allow the card to settle
							statusChanged("PRESENT", getCardATR(readers[i]));
						}
						first_loop = false;
					}
					if (m_readerStates[i].dwEventState & SCARD_STATE_CHANGED) {
						if ((m_readerStates[i].dwCurrentState & SCARD_STATE_PRESENT)
							&& (m_readerStates[i].dwEventState & SCARD_STATE_EMPTY)) {
							statusChanged("REMOVED", TQString::null);
						}
						else if ((m_readerStates[i].dwCurrentState & SCARD_STATE_EMPTY)
							&& (m_readerStates[i].dwEventState & SCARD_STATE_PRESENT)) {
							// sleep(1);	// Allow the card to settle
							statusChanged("INSERTED", getCardATR(readers[i]));
						}
						m_readerStates[i].dwCurrentState = m_readerStates[i].dwEventState;
					}
					else {
						continue;
					}
				}
				ret = SCardGetStatusChange(m_cardContext, PCSC_POLL_TIMEOUT_S, m_readerStates, readers.count());
			}
		}
	}

	eventLoop->exit(0);
#endif
}

void CryptoCardDeviceWatcher::requestTermination() {
	m_terminationRequested = true;
}

TQString CryptoCardDeviceWatcher::getCardATR(TQString readerName) {
#ifdef WITH_PCSC
	unsigned int i;
	long ret;
	TQString atr_formatted;
	SCARDHANDLE hCard = 0;
	DWORD dwActiveProtocol = 0;
	DWORD cByte = 0;

	ret = SCardConnect(m_cardContext, readerName.ascii(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
	if (ret == SCARD_S_SUCCESS) {
		ret = SCardGetAttrib(hCard, SCARD_ATTR_ATR_STRING, NULL, &cByte);
		if (ret == SCARD_S_SUCCESS) {
			char* data = new char[cByte];
			ret = SCardGetAttrib(hCard, SCARD_ATTR_ATR_STRING, (LPBYTE)data, &cByte);
			atr_formatted = TQString::null;
			for (i=0; i<cByte; i++) {
				TQString formatted;
				formatted.sprintf("%02x ", ((uint8_t)(*(data+i))));
				atr_formatted.append(formatted.upper());
			}
			atr_formatted = atr_formatted.stripWhiteSpace();
			free(data);
			SCardDisconnect(hCard, SCARD_LEAVE_CARD);
		}
	}
	else {
		TQString errstring = pcsc_error_code_to_string(ret);
		if (errstring != "") {
			atr_formatted = i18n("Unknown (%1)").arg(errstring);
		}
		else {
			atr_formatted = TQString("CARD_CONNECT_FAIL (%1)").arg(ret, 0, 16);
		}
	}

	return atr_formatted;
#endif
}

TDECryptographicCardDevice::TDECryptographicCardDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn),
	m_watcherThread(NULL),
	m_watcherObject(NULL),
	m_cardPresent(false) {
	//
}

TDECryptographicCardDevice::~TDECryptographicCardDevice() {
	enableCardMonitoring(false);
}

void TDECryptographicCardDevice::enableCardMonitoring(bool enable) {
#ifdef WITH_PCSC
	if (enable) {
		if (m_watcherObject && m_watcherThread) {
			// Monitoring thread already active; abort!
			return;
		}

		m_watcherThread = new TQEventLoopThread();
		m_watcherObject = new CryptoCardDeviceWatcher();

		m_watcherObject->cardDevice = this;
		m_watcherObject->moveToThread(m_watcherThread);
		TQObject::connect(m_watcherObject, SIGNAL(statusChanged(TQString,TQString)), this, SLOT(cardStatusChanged(TQString,TQString)));
		TQTimer::singleShot(0, m_watcherObject, SLOT(run()));

		m_watcherThread->start();
	}
	else {
		if (m_watcherObject) {
			m_watcherObject->requestTermination();
			delete m_watcherObject;
			m_watcherObject = NULL;
		}
		if (m_watcherThread) {
			m_watcherThread->wait();
			delete m_watcherThread;
			m_watcherThread = NULL;
		}
	}
#endif
}

int TDECryptographicCardDevice::cardPresent() {
	if (m_watcherObject && m_watcherThread) {
		if (m_cardPresent)
			return 1;
		else
			return 0;
	}
	else {
		return -1;
	}
}

TQString TDECryptographicCardDevice::cardATR() {
	if (m_watcherObject && m_watcherThread) {
		if (m_cardPresent)
			return m_cardATR;
		else
			return TQString::null;
	}
	else {
		return TQString::null;
	}
}

void TDECryptographicCardDevice::cardStatusChanged(TQString status, TQString atr) {
	if (status == "INSERTED") {
		m_cardPresent = true;
		m_cardATR = atr;
		emit(cardInserted());
	}
	else if (status == "REMOVED") {
		m_cardPresent = false;
		m_cardATR = atr;
		emit(cardRemoved());
	}
	else if (status == "PRESENT") {
		m_cardATR = atr;
		m_cardPresent = true;
	}
}

#include "tdecryptographiccarddevice.moc"
#include "tdecryptographiccarddevice_private.moc"
