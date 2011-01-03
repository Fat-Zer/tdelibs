/* This file is part of the KDE libraries
   Copyright (c) 2004 Szombathelyi Gy�gy <gyurco@freemail.hu>

   The implementation is based on the documentation and sample code
   at http://davenport.sourceforge.net/ntlm.html
   The DES encryption functions are from libntlm 
   at http://josefsson.org/libntlm/

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

#include <string.h>

#include <tqdatetime.h>
#include <kapplication.h>
#include <kswap.h>
#include <kmdcodec.h>
#include <kdebug.h>

#include "des.h"
#include "kntlm.h"

TQString KNTLM::getString( const TQByteArray &buf, const SecBuf &secbuf, bool tqunicode )
{
  //watch for buffer overflows
  TQ_UINT32 offset;
  TQ_UINT16 len;
  offset = KFromToLittleEndian((TQ_UINT32)secbuf.offset);
  len = KFromToLittleEndian(secbuf.len);
  if ( offset > buf.size() ||
       offset + len > buf.size() ) return TQString::null;

  TQString str;
  const char *c = buf.data() + offset;
  
  if ( tqunicode ) {
    str = UnicodeLE2TQString( (TQChar*) c, len >> 1 );
  } else {
    str = TQString::tqfromLatin1( c, len );
  }
  return str;
}

TQByteArray KNTLM::getBuf( const TQByteArray &buf, const SecBuf &secbuf )
{
  TQByteArray ret;
  TQ_UINT32 offset;
  TQ_UINT16 len;
  offset = KFromToLittleEndian((TQ_UINT32)secbuf.offset);
  len = KFromToLittleEndian(secbuf.len);
  //watch for buffer overflows
  if ( offset > buf.size() ||
       offset + len > buf.size() ) return ret;
  ret.duplicate( buf.data() + offset, buf.size() );
  return ret;
}

void KNTLM::addString( TQByteArray &buf, SecBuf &secbuf, const TQString &str, bool tqunicode )
{
  TQByteArray tmp;

  if ( tqunicode ) {
    tmp = QString2UnicodeLE( str );
    addBuf( buf, secbuf, tmp );
  } else {
    const char *c;
    c = str.latin1();
    tmp.setRawData( c, str.length() );
    addBuf( buf, secbuf, tmp );
    tmp.resetRawData( c, str.length() );
  }
}

void KNTLM::addBuf( TQByteArray &buf, SecBuf &secbuf, TQByteArray &data )
{
  TQ_UINT32 offset;
  TQ_UINT16 len, maxlen;
  offset = (buf.size() + 1) & 0xfffffffe;
  len = data.size();
  maxlen = data.size();
  
  secbuf.offset = KFromToLittleEndian((TQ_UINT32)offset);
  secbuf.len = KFromToLittleEndian(len);
  secbuf.maxlen = KFromToLittleEndian(maxlen);
  buf.resize( offset + len );
  memcpy( buf.data() + offset, data.data(), data.size() );
}

bool KNTLM::getNegotiate( TQByteArray &negotiate, const TQString &domain, const TQString &workstation, TQ_UINT32 flags )
{
  TQByteArray rbuf( sizeof(Negotiate) );
  
  rbuf.fill( 0 );
  memcpy( rbuf.data(), "NTLMSSP", 8 );
  ((Negotiate*) rbuf.data())->msgType = KFromToLittleEndian( (TQ_UINT32)1 );
  if ( !domain.isEmpty() ) {
    flags |= Negotiate_Domain_Supplied;
    addString( rbuf, ((Negotiate*) rbuf.data())->domain, domain );
  }
  if ( !workstation.isEmpty() ) {
    flags |= Negotiate_WS_Supplied;
    addString( rbuf, ((Negotiate*) rbuf.data())->domain, workstation );
  }
  ((Negotiate*) rbuf.data())->flags = KFromToLittleEndian( flags );
  negotiate = rbuf;
  return true;
}

bool KNTLM::getAuth( TQByteArray &auth, const TQByteArray &challenge, const TQString &user, 
  const TQString &password, const TQString &domain, const TQString &workstation, 
  bool forceNTLM, bool forceNTLMv2 )
{
  TQByteArray rbuf( sizeof(Auth) );
  Challenge *ch = (Challenge *) challenge.data();
  TQByteArray response;
  uint chsize = challenge.size();
  bool tqunicode = false;
  TQString dom;

  //challenge structure too small
  if ( chsize < 32 ) return false;

  tqunicode = KFromToLittleEndian(ch->flags) & Negotiate_Unicode;
  if ( domain.isEmpty() )
    dom = getString( challenge, ch->targetName, tqunicode );
  else
    dom = domain;
    
  rbuf.fill( 0 );
  memcpy( rbuf.data(), "NTLMSSP", 8 );
  ((Auth*) rbuf.data())->msgType = KFromToLittleEndian( (TQ_UINT32)3 );
  ((Auth*) rbuf.data())->flags = ch->flags;
  TQByteArray targetInfo = getBuf( challenge, ch->targetInfo );

//  if ( forceNTLMv2 || (!targetInfo.isEmpty() && (KFromToLittleEndian(ch->flags) & Negotiate_Target_Info)) /* may support NTLMv2 */ ) {
//    if ( KFromToLittleEndian(ch->flags) & Negotiate_NTLM ) {
//      if ( targetInfo.isEmpty() ) return false;
//      response = getNTLMv2Response( dom, user, password, targetInfo, ch->challengeData );
//      addBuf( rbuf, ((Auth*) rbuf.data())->ntResponse, response );
//    } else {
//      if ( !forceNTLM ) {
//        response = getLMv2Response( dom, user, password, ch->challengeData );
//        addBuf( rbuf, ((Auth*) rbuf.data())->lmResponse, response );
//      } else 
//        return false;
//    }
//  } else { //if no targetinfo structure and NTLMv2 or LMv2 not forced, try the older methods

    response = getNTLMResponse( password, ch->challengeData );
    addBuf( rbuf, ((Auth*) rbuf.data())->ntResponse, response );
    response = getLMResponse( password, ch->challengeData );
    addBuf( rbuf, ((Auth*) rbuf.data())->lmResponse, response );
//  }
  if ( !dom.isEmpty() )
    addString( rbuf, ((Auth*) rbuf.data())->domain, dom, tqunicode );
  addString( rbuf, ((Auth*) rbuf.data())->user, user, tqunicode );
  if ( !workstation.isEmpty() )
    addString( rbuf, ((Auth*) rbuf.data())->workstation, workstation, tqunicode );

  auth = rbuf;

  return true;
}

TQByteArray KNTLM::getLMResponse( const TQString &password, const unsigned char *challenge )
{
  TQByteArray hash, answer;

  hash = lmHash( password );
  hash.resize( 21 );
  memset( hash.data() + 16, 0, 5 );
  answer = lmResponse( hash, challenge );
  hash.fill( 0 );
  return answer;
}

TQByteArray KNTLM::lmHash( const TQString &password )
{
  TQByteArray keyBytes( 14 );
  TQByteArray hash( 16 );
  DES_KEY ks;
  const char *magic = "KGS!@#$%";

  keyBytes.fill( 0 );
  strncpy( keyBytes.data(), password.upper().latin1(), 14 );

  convertKey( (unsigned char*) keyBytes.data(), &ks );
  ntlm_des_ecb_encrypt( magic, 8, &ks, (unsigned char*) hash.data() );

  convertKey( (unsigned char*) keyBytes.data() + 7, &ks );
  ntlm_des_ecb_encrypt( magic, 8, &ks, (unsigned char*) hash.data() + 8 );

  keyBytes.fill( 0 );
  memset( &ks, 0, sizeof (ks) );

  return hash;
}

TQByteArray KNTLM::lmResponse( const TQByteArray &hash, const unsigned char *challenge )
{
  DES_KEY ks;
  TQByteArray answer( 24 );

  convertKey( (unsigned char*) hash.data(), &ks );
  ntlm_des_ecb_encrypt( challenge, 8, &ks, (unsigned char*) answer.data() );

  convertKey( (unsigned char*) hash.data() + 7, &ks );
  ntlm_des_ecb_encrypt( challenge, 8, &ks, (unsigned char*) answer.data() + 8 );

  convertKey( (unsigned char*) hash.data() + 14, &ks );
  ntlm_des_ecb_encrypt( challenge, 8, &ks, (unsigned char*) answer.data() + 16 );

  memset( &ks, 0, sizeof (ks) );
  return answer;
}

TQByteArray KNTLM::getNTLMResponse( const TQString &password, const unsigned char *challenge )
{
  TQByteArray hash, answer;

  hash = ntlmHash( password );
  hash.resize( 21 );
  memset( hash.data() + 16, 0, 5 );
  answer = lmResponse( hash, challenge );
  hash.fill( 0 );
  return answer;
}

TQByteArray KNTLM::ntlmHash( const TQString &password )
{
  KMD4::Digest digest;
  TQByteArray ret, tqunicode;
  tqunicode = QString2UnicodeLE( password );

  KMD4 md4( tqunicode );
  md4.rawDigest( digest );
  ret.duplicate( (const char*) digest, sizeof( digest ) );
  return ret;
}

TQByteArray KNTLM::getNTLMv2Response( const TQString &target, const TQString &user,
  const TQString &password, const TQByteArray &targetInformation,
  const unsigned char *challenge )
{
  TQByteArray hash = ntlmv2Hash( target, user, password );
  TQByteArray blob = createBlob( targetInformation );
  return lmv2Response( hash, blob, challenge );
}

TQByteArray KNTLM::getLMv2Response( const TQString &target, const TQString &user,
  const TQString &password, const unsigned char *challenge )
{
  TQByteArray hash = ntlmv2Hash( target, user, password );
  TQByteArray clientChallenge( 8 );
  for ( uint i = 0; i<8; i++ ) {
    clientChallenge.data()[i] = KApplication::random() % 0xff;
  }
  return lmv2Response( hash, clientChallenge, challenge );
}

TQByteArray KNTLM::ntlmv2Hash( const TQString &target, const TQString &user, const TQString &password )
{
  TQByteArray hash1 = ntlmHash( password );
  TQByteArray key, ret;
  TQString id = user.upper() + target.upper();
  key = QString2UnicodeLE( id );
  ret = hmacMD5( key, hash1 );
  return ret;  
}

TQByteArray KNTLM::lmv2Response( const TQByteArray &hash, 
  const TQByteArray &clientData, const unsigned char *challenge )
{
  TQByteArray data( 8 + clientData.size() );
  memcpy( data.data(), challenge, 8 );
  memcpy( data.data() + 8, clientData.data(), clientData.size() );
  TQByteArray mac = hmacMD5( data, hash );
  mac.resize( 16 + clientData.size() );
  memcpy( mac.data() + 16, clientData.data(), clientData.size() );
  return mac;
}

TQByteArray KNTLM::createBlob( const TQByteArray &targetinfo )
{
  TQByteArray blob( sizeof(Blob) + 4 + targetinfo.size() );
  blob.fill( 0 );
  
  Blob *bl = (Blob *) blob.data();
  bl->signature = KFromToBigEndian( (TQ_UINT32) 0x01010000 );
  TQ_UINT64 now = TQDateTime::tqcurrentDateTime().toTime_t();
  now += (TQ_UINT64)3600*(TQ_UINT64)24*(TQ_UINT64)134774;
  now *= (TQ_UINT64)10000000;
  bl->timestamp = KFromToLittleEndian( now );
  for ( uint i = 0; i<8; i++ ) {
    bl->challenge[i] = KApplication::random() % 0xff;
  }
  memcpy( blob.data() + sizeof(Blob), targetinfo.data(), targetinfo.size() );
  return blob;
}

TQByteArray KNTLM::hmacMD5( const TQByteArray &data, const TQByteArray &key )
{
  TQ_UINT8 ipad[64], opad[64];
  KMD5::Digest digest;
  TQByteArray ret;
  
  memset( ipad, 0x36, sizeof(ipad) );
  memset( opad, 0x5c, sizeof(opad) );
  for ( int i = key.size()-1; i >= 0; i-- ) {
    ipad[i] ^= key[i];
    opad[i] ^= key[i];
  }

  TQByteArray content( data.size()+64 );
  memcpy( content.data(), ipad, 64 );
  memcpy( content.data() + 64, data.data(), data.size() );
  KMD5 md5( content );
  md5.rawDigest( digest );
  content.resize( sizeof(digest) + 64 );
  memcpy( content.data(), opad, 64 );
  memcpy( content.data() + 64, digest, sizeof(digest) );
  md5.reset();
  md5.update( content );
  md5.rawDigest( digest );

  ret.duplicate( (const char*) digest, sizeof( digest ) );
  return ret;
}

/*
* turns a 56 bit key into the 64 bit, odd parity key and sets the key.
* The key schedule ks is also set.
*/
void KNTLM::convertKey( unsigned char *key_56, void* ks )
{
  unsigned char key[8];

  key[0] = key_56[0];
  key[1] = ((key_56[0] << 7) & 0xFF) | (key_56[1] >> 1);
  key[2] = ((key_56[1] << 6) & 0xFF) | (key_56[2] >> 2);
  key[3] = ((key_56[2] << 5) & 0xFF) | (key_56[3] >> 3);
  key[4] = ((key_56[3] << 4) & 0xFF) | (key_56[4] >> 4);
  key[5] = ((key_56[4] << 3) & 0xFF) | (key_56[5] >> 5);
  key[6] = ((key_56[5] << 2) & 0xFF) | (key_56[6] >> 6);
  key[7] = (key_56[6] << 1) & 0xFF;

  for ( uint i=0; i<8; i++ ) {
    unsigned char b = key[i];
    bool needsParity = (((b>>7) ^ (b>>6) ^ (b>>5) ^ (b>>4) ^ (b>>3) ^ (b>>2) ^ (b>>1)) & 0x01) == 0;
    if ( needsParity ) 
      key[i] |= 0x01;
    else
      key[i] &= 0xfe;
  }

  ntlm_des_set_key ( (DES_KEY*) ks, (char*) &key, sizeof (key));

  memset (&key, 0, sizeof (key));
}

TQByteArray KNTLM::QString2UnicodeLE( const TQString &target )
{
  TQByteArray tqunicode( target.length() * 2 );
  for ( uint i = 0; i < target.length(); i++ ) {
    ((TQ_UINT16*)tqunicode.data())[ i ] = KFromToLittleEndian( target[i].tqunicode() );
  }
  return tqunicode;
}

TQString KNTLM::UnicodeLE2TQString( const TQChar* data, uint len )
{
  TQString ret;
  for ( uint i = 0; i < len; i++ ) {
    ret += KFromToLittleEndian( data[ i ].tqunicode() );
  }
  return ret;
}
