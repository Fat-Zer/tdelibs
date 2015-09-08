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

#ifdef WITH_PKCS
	#define _TDECRYPTOGRAPHICCARDDEVICE_INTERNAL 1
#endif

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
#ifdef WITH_PCSC
	m_readerStates = NULL;
#endif
}

CryptoCardDeviceWatcher::~CryptoCardDeviceWatcher() {
#ifdef WITH_PCSC
	free(m_readerStates);
#endif
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
							TQString atr = getCardATR(readers[i]);
							retrieveCardCertificates(readers[i]);
							statusChanged("PRESENT", atr);
						}
						else {
							deleteAllCertificatesFromCache();
						}
						first_loop = false;
					}
					if (m_readerStates[i].dwEventState & SCARD_STATE_CHANGED) {
						if ((m_readerStates[i].dwCurrentState & SCARD_STATE_PRESENT)
							&& (m_readerStates[i].dwEventState & SCARD_STATE_EMPTY)) {
							deleteAllCertificatesFromCache();
							statusChanged("REMOVED", TQString::null);
						}
						else if ((m_readerStates[i].dwCurrentState & SCARD_STATE_EMPTY)
							&& (m_readerStates[i].dwEventState & SCARD_STATE_PRESENT)) {
							// sleep(1);	// Allow the card to settle
							TQString atr = getCardATR(readers[i]);
							retrieveCardCertificates(readers[i]);
							statusChanged("INSERTED", atr);
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
#else
	return TQString::null;
#endif
}

#ifdef WITH_PKCS
static void pkcs_log_hook(IN void * const global_data, IN unsigned flags, IN const char * const format, IN va_list args) {
	vprintf(format, args);
	printf("\n");
}
#endif

int CryptoCardDeviceWatcher::retrieveCardCertificates(TQString readerName) {
#if WITH_PKCS
	int ret = -1;

	CK_RV rv;
	pkcs11h_certificate_id_list_t issuers;
	pkcs11h_certificate_id_list_t certs;
	pkcs11h_certificate_id_t find = NULL;

	printf("Initializing pkcs11-helper\n");
	if ((rv = pkcs11h_initialize()) != CKR_OK) {
		printf("pkcs11h_initialize failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}

	printf("Registering pkcs11-helper hooks\n");
	if ((rv = pkcs11h_setLogHook(pkcs_log_hook, NULL)) != CKR_OK) {
		printf("pkcs11h_setLogHook failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}
	pkcs11h_setLogLevel(PKCS11H_LOG_WARN);

#if 0
	if ((rv = pkcs11h_setTokenPromptHook(_pkcs11h_hooks_token_prompt, NULL)) != CKR_OK) {
		printf("pkcs11h_setTokenPromptHook failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}
	if ((rv = pkcs11h_setPINPromptHook(_pkcs11h_hooks_pin_prompt, NULL)) != CKR_OK) {
		printf("pkcs11h_setPINPromptHook failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}
#endif
	printf("Adding provider '%s'\n", OPENSC_PKCS11_PROVIDER_LIBRARY);
		if ((rv = pkcs11h_addProvider (OPENSC_PKCS11_PROVIDER_LIBRARY, OPENSC_PKCS11_PROVIDER_LIBRARY, FALSE, PKCS11H_PRIVATEMODE_MASK_AUTO, PKCS11H_SLOTEVENT_METHOD_AUTO, 0, FALSE)) != CKR_OK) {
		printf("pkcs11h_addProvider failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}

	rv = pkcs11h_certificate_enumCertificateIds(PKCS11H_ENUM_METHOD_CACHE, NULL, PKCS11H_PROMPT_MASK_ALLOW_NONE, &issuers, &certs);
	if ((rv != CKR_OK) || (certs == NULL)) {
		printf("Cannot enumerate certificates: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}
	printf("Successfully enumerated certificates\n");

	int i = 0;
	for (pkcs11h_certificate_id_list_t cert = certs; cert != NULL; cert = cert->next) {
		TQString label = cert->certificate_id->displayName;
		printf("The name of the %d certficate is %s\n", i, label.ascii());

		pkcs11h_certificate_t certificate;
		rv = pkcs11h_certificate_create(find, NULL, PKCS11H_PROMPT_MASK_ALLOW_NONE, PKCS11H_PIN_CACHE_INFINITE, &certificate);
		if (rv != CKR_OK) {
			printf("Can not read certificate: %s\n", pkcs11h_getMessage(rv));
			pkcs11h_certificate_freeCertificateId(find);
			ret = -1;
			break;
		}
		pkcs11h_certificate_freeCertificateId(find);

		pkcs11h_openssl_session_t openssl_session = NULL;
		if ((openssl_session = pkcs11h_openssl_createSession(certificate)) == NULL) {
			printf("Cannot initialize openssl session to retrieve cryptographic objects\n");
			pkcs11h_certificate_freeCertificate(certificate);
			ret = -1;
			break;
		}
		certificate = NULL;	// the certificate object is managed by openssl_session

		X509* x509_local;
		x509_local = pkcs11h_openssl_session_getX509(openssl_session);
		if (x509_local) {
			printf("Successfully retrieved X509 certificate\n");
		}
		else {
			printf("Cannot get X509 object\n");
			ret = -1;
		}
#if 0
		RSA* rsa_local;
		rsa_local = pkcs11h_openssl_session_getRSA(openssl_session);
		if (rsa_local) {
			printf("Successfully retrieved RSA public key\n");
		}
		else {
			printf("Cannot get RSA object\n");
			ret = -1;
		}
#endif

		X509* x509_copy = X509_dup(x509_local);
		if (x509_copy) {
			cardDevice->m_cardCertificates.append(x509_copy);
		}
		else {
			printf("Unable to copy X509 certificate\n");
		}

		pkcs11h_certificate_freeCertificateIdList(issuers);
		pkcs11h_certificate_freeCertificateIdList(certs);

		pkcs11h_openssl_freeSession(openssl_session);

		i++;
	}

	return ret;
#else
	return -1;
#endif
}

void CryptoCardDeviceWatcher::deleteAllCertificatesFromCache() {
#ifdef WITH_PKCS
	X509 *x509_cert;

	X509CertificatePtrListIterator it;
	for (it = cardDevice->m_cardCertificates.begin(); it != cardDevice->m_cardCertificates.end(); ++it) {
		x509_cert = *it;
		X509_free(x509_cert);
	}

	cardDevice->m_cardCertificates.clear();
#endif
}

TDECryptographicCardDevice::TDECryptographicCardDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn),
	m_watcherThread(NULL),
	m_watcherObject(NULL),
	m_cardPresent(false) {
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

X509CertificatePtrList TDECryptographicCardDevice::cardX509Certificates() {
	if (m_watcherObject && m_watcherThread) {
		if (m_cardPresent)
			return m_cardCertificates;
		else
			return X509CertificatePtrList();
	}
	else {
		return X509CertificatePtrList();
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
