/* This file is part of the KDE libraries
   Copyright (C) 2001-2002 Michael H�ckel <haeckel@kde.org>
   $Id$
 
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

#ifndef TDESASL_H
#define TDESASL_H

#include <tqstring.h>

#include <tdelibs_export.h>

class KURL;
class TQStrIList;

/**
 * This library can create responses for SASL authentication for a given
 * challenge and a given secret. This way of authentication is common for
 * SMTP, POP3, IMAP and LDAP.
 *
 * SASL is one way strong encryption and therefore useful for authentication,
 * but not for secret information transfer.
 * It is possibly to prove with SASL to know a shared secret like a password.
 * It is not possible with SASL to transfer any other information in an
 * encrypted way. For that purpose OpenPGP or SSL are useful.
 *
 * Currently PLAIN (RFC 2595), LOGIN (not really a SASL mechanism, but
 * used like that in IMAP and SMTP), CRAM-MD5 (RFC 2195) and
 * DIGEST-MD5 (RFC 2831) authentication are supported.  PLAIN and
 * LOGIN transmit the credentials in the clear (apart from a possible
 * base64 encoding).
 *
 * For KDE 3.2, the API has been extended to allow transparent use of
 * all currently supported SASL mechanisms. Example:
 * \code
 * KDESasl sasl( myUser, myPass, myProtocol );
 * if ( !sasl.chooseMethod( myMechanismsSupportedByServer ) )
 *   return false; // couldn't agree on a method
 *
 * int numResponses = 0;
 * if ( sasl.clientStarts() ) { // check whether we're supposed to start the dialog
 *   ++numResponses;
 *   mySendAuthCommand( sasl.method(), sasl.getResponse() );
 * } else {
 *   mySendAuthCommand( sasl.method() );
 * }
 * for ( ; !sasl.dialogComplete( numResponses ) ; ++numResponses ) {
 *   TQByteArray challenge = myRecvChallenge();
 *   mySendResponse( sasl.getResponse( challenge ) );
 * }
 * return myCheckSuccess();
 * \endcode
 *
 * @author Michael H�ckel <haeckel@kde.org>
 * @version $Id$
 */

class TDEIO_EXPORT KDESasl
{

public:
  /**
   * Construct a sasl object and initialize it with the username and password
   * passed via the url.
   */
  KDESasl(const KURL &aUrl);
  /**
   * This is a conveniece function and differs from the above function only by
   * what arguments it accepts.
   */
  KDESasl(const TQString &aUser, const TQString &aPass, const TQString &aProtocol);
  /*
   * You need to have a virtual destructor!
   */
  virtual ~KDESasl();
  /**
   * @returns the most secure method from the given methods and use it for
   * further operations.
   */
  virtual TQCString chooseMethod(const TQStrIList aMethods);
  /**
   * Explicitely set the SASL method used.
   */
  virtual void setMethod(const TQCString &aMethod);
  /**
   * @return the SASL method used.
   * @since 3.2
   */
  TQCString method() const;
  /**
   * @param numCalls number of times getResponse() has been called.
   * @return whether the challenge/response dialog has completed
   *
   * @since 3.2
   */
  bool dialogComplete( int numCalls ) const;
  /**
   * @return whether the currently selected mechanism results in
   * cleartext passwords being sent over the network and thus should
   * be used only under TLS/SSL cover or for legacy servers.
   *
   * @since 3.2
   */
  bool isClearTextMethod() const;
  /**
   * Creates a response using the formerly chosen SASL method.
   * For LOGIN authentication you have to call this function twice. KDESasl
   * realizes on its own, if you are calling it for the first or for the
   * second time.
   * @param aChallenge is the challenge sent to create a response for
   * @param aBase64 specifies, whether the authentication protocol uses base64
   * encoding. The challenge is decoded from base64 and the response is
   * encoded base64 if set to true.
   */
   TQCString getResponse(const TQByteArray &aChallenge=TQByteArray(), bool aBase64 = true);
  /**
   * Create a response as above but place it in a QByteArray
   */
  TQByteArray getBinaryResponse(const TQByteArray &aChallenge=TQByteArray(), bool aBase64=true); 
  /**
   * Returns true if the client is supposed to initiate the
   * challenge-respinse dialog with an initial response (which most
   * protocols can transfer alongside the authentication command as an
   * optional second parameter). This method relieves the sasl user
   * from knowing details about the mechanism. If true, use 
   * #getResponse() with a null challenge.
   *
   * @since 3.2
   */
  bool clientStarts() const;
protected:
  /**
   * PLAIN authentication as described in RFC 2595
   */
  virtual TQByteArray getPlainResponse();
  /**
   * LOGIN authentication
   */
  virtual TQByteArray getLoginResponse();
  /**
   * CRAM-MD5 authentication as described in RFC 2195
   */
  virtual TQByteArray getCramMd5Response(const TQByteArray &aChallenge);
  /**
   * DIGEST-MD5 authentication as described in RFC 2831
   */
  virtual TQByteArray getDigestMd5Response(const TQByteArray &aChallenge);

private:
  TQString mProtocol, mUser, mPass;
  TQCString mMethod;
  bool mFirst;
};

#endif
