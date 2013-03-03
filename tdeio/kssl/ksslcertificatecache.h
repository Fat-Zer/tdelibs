/* This file is part of the KDE project
 *
 * Copyright (C) 2000, 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _INCLUDE_KSSLCCACHE_H
#define _INCLUDE_KSSLCCACHE_H

class KSSLCertificate;
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdatetime.h>

#include <tdelibs_export.h>

class TDEIO_EXPORT KSSLCertificateCache {
public:

enum KSSLCertificatePolicy { Unknown, Reject, Accept, Prompt, Ambiguous };
// Unknown: no policy has been set for this record
// Reject: user has requested to not accept data from this site
// Accept: user has requested to always accept data from this site
// Prompt: user wishes to be prompted before accepting this certificate
//         You may need to set a [non-]permanent policy on this record after
//         the user is prompted.
// Ambiguous: The state cannot be uniquely determined.  Hopefully this
//            doesn't happen.

  KSSLCertificateCache();
  ~KSSLCertificateCache();

  void addCertificate(KSSLCertificate& cert, KSSLCertificatePolicy policy, 
                                                     bool permanent = true);

  // WARNING!  This is not a "secure" method.  You need to actually
  //           do a getPolicyByCertificate to be cryptographically sure
  //           that this is an accepted certificate/site pair.
  //           (note that the site (CN) is encoded in the certificate
  //            so you should only accept certificates whose CN matches
  //            the exact FQDN of the site presenting it)
  //           If you're just doing an OpenSSL connection, I believe it
  //           tests this for you, but don't take my word for it.
  KSSLCertificatePolicy getPolicyByCN(TQString& cn);

  KSSLCertificatePolicy getPolicyByCertificate(KSSLCertificate& cert);

  bool seenCN(TQString& cn);
  bool seenCertificate(KSSLCertificate& cert);

  bool removeByCN(TQString& cn);
  bool removeByCertificate(KSSLCertificate& cert);

  bool isPermanent(KSSLCertificate& cert);

  bool modifyByCN(TQString& cn,
                  KSSLCertificateCache::KSSLCertificatePolicy policy,
                  bool permanent,
                  TQDateTime& expires);

  bool modifyByCertificate(KSSLCertificate& cert,
                           KSSLCertificateCache::KSSLCertificatePolicy policy,
                           bool permanent,
                           TQDateTime& expires);

  TQStringList getHostList(KSSLCertificate& cert);
  bool addHost(KSSLCertificate& cert, TQString& host);
  bool removeHost(KSSLCertificate& cert, TQString& host);

  // SMIME
  TQStringList getKDEKeyByEmail(const TQString &email);
  KSSLCertificate *getCertByMD5Digest(const TQString &key);

  void reload();

  // You shouldn't need to call this but in some weird circumstances
  // it might be necessary.
  void saveToDisk();

private:
  class KSSLCertificateCachePrivate;
  KSSLCertificateCachePrivate *d;

  void loadDefaultPolicies();
  void clearList();

};


TDEIO_EXPORT TQDataStream& operator<<(TQDataStream& s, const KSSLCertificateCache::KSSLCertificatePolicy& p);
TDEIO_EXPORT TQDataStream& operator>>(TQDataStream& s, KSSLCertificateCache::KSSLCertificatePolicy& p);

#endif