
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

#ifndef _TDECRYPTOGRAPHICCARDDEVICE_H
#define _TDECRYPTOGRAPHICCARDDEVICE_H

#include "tdegenericdevice.h"

#ifndef _TDECRYPTOGRAPHICCARDDEVICE_INTERNAL
	#ifdef KSSL_HAVE_SSL
		typedef struct x509_st X509;
	#else
		class X509;
	#endif
#endif

class TQEventLoopThread;
class CryptoCardDeviceWatcher;

typedef TQValueList<X509*> X509CertificatePtrList;
typedef TQValueListIterator<X509*> X509CertificatePtrListIterator;

class TDECORE_EXPORT TDECryptographicCardDevice : public TDEGenericDevice
{
	Q_OBJECT

	public:
		/**
		 *  Constructor.
		 *  @param Device type
		 */
		TDECryptographicCardDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		 * Destructor.
		 */
		~TDECryptographicCardDevice();

		/**
		 * Enable / disable monitoring of insert / remove events.
		 * @param enable true to enable, false to disable.
		 */
		void enableCardMonitoring(bool enable);

		/**
		 * If monitoring of insert / remove events is enabled,
		 * return whether or not a card is present.
		 * @return -1 if status unknown, 0 if card not present,
		 * 1 if card is present.
		 */
		int cardPresent();

		/**
		 * If monitoring of insert / remove events is enabled,
		 * and a card has been inserted, @return the card's ATR.
		 * @return TQString::null if no card or card status unknown.
		 */
		TQString cardATR();

		/**
		 * If monitoring of insert / remove events is enabled,
		 * and a card has been inserted, @return a list of all
		 * X509 certificates on the card.
		 * @return an empty list if no card or card contents unknown.
		 *
		 * @example KSSLCertificate* tdeCert = KSSLCertificate::fromX509(cardX509Certificates().first());
		 */
		X509CertificatePtrList cardX509Certificates();

	public slots:
		void cardStatusChanged(TQString status, TQString atr);

	signals:
		void cardInserted();
		void cardRemoved();

	private:
		TQEventLoopThread *m_watcherThread;
		CryptoCardDeviceWatcher *m_watcherObject;

		bool m_cardPresent;
		TQString m_cardATR;
		X509CertificatePtrList m_cardCertificates;

	friend class TDEHardwareDevices;
	friend class CryptoCardDeviceWatcher;
};

#endif // _TDECRYPTOGRAPHICCARDDEVICE_H
