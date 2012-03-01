/* kldapclient.cpp - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 *      Ported to KABC by Daniel Molkentin <molkentin@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */



#include <tqfile.h>
#include <tqimage.h>
#include <tqlabel.h>
#include <tqpixmap.h>
#include <tqtextstream.h>
#include <tqurl.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kmdcodec.h>
#include <kprotocolinfo.h>

#include "ldapclient.h"
#include "ldif.h"
#include "ldapurl.h"

using namespace KABC;

class LdapClient::LdapClientPrivate{
public:
  TQString bindDN;
  TQString pwdBindDN;
  LDIF ldif;
};

TQString LdapObject::toString() const
{
  TQString result = TQString::fromLatin1( "\ndn: %1\n" ).arg( dn );
  for ( LdapAttrMap::ConstIterator it = attrs.begin(); it != attrs.end(); ++it ) {
    TQString attr = it.key();
    for ( LdapAttrValue::ConstIterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 ) {
      result += TQString::fromUtf8( LDIF::assembleLine( attr, *it2, 76 ) ) + "\n";
    }
  }

  return result;
}

void LdapObject::clear()
{
  dn = TQString::null;
  attrs.clear();
}

void LdapObject::assign( const LdapObject& that )
{
  if ( &that != this ) {
    dn = that.dn;
    attrs = that.attrs;
    client = that.client;
  }
}

LdapClient::LdapClient( TQObject* parent, const char* name )
  : TQObject( parent, name ), mJob( 0 ), mActive( false )
{
  d = new LdapClientPrivate;
}

LdapClient::~LdapClient()
{
  cancelQuery();
  delete d; d = 0;
}

void LdapClient::setHost( const TQString& host )
{
  mHost = host;
}

void LdapClient::setPort( const TQString& port )
{
  mPort = port;
}

void LdapClient::setBase( const TQString& base )
{
  mBase = base;
}

void LdapClient::setBindDN( const TQString& bindDN )
{
  d->bindDN = bindDN;
}

void LdapClient::setPwdBindDN( const TQString& pwdBindDN )
{
  d->pwdBindDN = pwdBindDN;
}

void LdapClient::setAttrs( const TQStringList& attrs )
{
  mAttrs = attrs;
}

void LdapClient::startQuery( const TQString& filter )
{
  cancelQuery();
  LDAPUrl url;

  url.setProtocol( "ldap" );
  url.setUser( d->bindDN );
  url.setPass( d->pwdBindDN );
  url.setHost( mHost );
  url.setPort( mPort.toUInt() );
  url.setDn( mBase );
  url.setAttributes( mAttrs );
  url.setScope( mScope == "one" ? LDAPUrl::One : LDAPUrl::Sub );
  url.setFilter( "("+filter+")" );

  kdDebug(5700) << "Doing query: " << url.prettyURL() << endl;

  startParseLDIF();
  mActive = true;
  mJob = KIO::get( url, false, false );
  connect( mJob, TQT_SIGNAL( data( KIO::Job*, const TQByteArray& ) ),
           this, TQT_SLOT( slotData( KIO::Job*, const TQByteArray& ) ) );
  connect( mJob, TQT_SIGNAL( infoMessage( KIO::Job*, const TQString& ) ),
           this, TQT_SLOT( slotInfoMessage( KIO::Job*, const TQString& ) ) );
  connect( mJob, TQT_SIGNAL( result( KIO::Job* ) ),
           this, TQT_SLOT( slotDone() ) );
}

void LdapClient::cancelQuery()
{
  if ( mJob ) {
    mJob->kill();
    mJob = 0;
  }

  mActive = false;
}

void LdapClient::slotData( KIO::Job*, const TQByteArray& data )
{
#ifndef NDEBUG // don't create the QString
//  TQString str( data );
//  kdDebug(5700) << "LdapClient: Got \"" << str << "\"\n";
#endif
  parseLDIF( data );
}

void LdapClient::slotInfoMessage( KIO::Job*, const TQString & )
{
  //tqDebug("Job said \"%s\"", info.latin1());
}

void LdapClient::slotDone()
{
  endParseLDIF();
  mActive = false;
#if 0
  for ( TQValueList<LdapObject>::Iterator it = mObjects.begin(); it != mObjects.end(); ++it ) {
    tqDebug( (*it).toString().latin1() );
  }
#endif
  int err = mJob->error();
  if ( err && err != KIO::ERR_USER_CANCELED ) {
    emit error( KIO::buildErrorString( err, TQString("%1:%2").arg( mHost ).arg( mPort ) ) );
  }
  emit done();
}

void LdapClient::startParseLDIF()
{
  mCurrentObject.clear();
  mLastAttrName  = 0;
  mLastAttrValue = 0;
  mIsBase64 = false;
  d->ldif.startParsing();
}

void LdapClient::endParseLDIF()
{
}

void LdapClient::parseLDIF( const TQByteArray& data )
{
  if ( data.size() ) {
    d->ldif.setLDIF( data );
  } else {
    d->ldif.endLDIF();
  }

  LDIF::ParseVal ret;
  TQString name;
  do {
    ret = d->ldif.nextItem();
    switch ( ret ) {
      case LDIF::Item:
      {
        name = d->ldif.attr();
        // Must make a copy! TQByteArray is explicitely shared
        TQByteArray value = d->ldif.val().copy();
        mCurrentObject.attrs[ name ].append( value );
        break;
      }
     case LDIF::EndEntry:
        mCurrentObject.dn = d->ldif.dn();
        mCurrentObject.client = this;
        emit result( mCurrentObject );
        mCurrentObject.clear();
        break;
      default:
        break;
    }
  } while ( ret != LDIF::MoreData );
}

TQString LdapClient::bindDN() const
{
  return d->bindDN;
}

TQString LdapClient::pwdBindDN() const
{
  return d->pwdBindDN;
}

LdapSearch::LdapSearch()
    : mActiveClients( 0 ), mNoLDAPLookup( false )
{
  if ( !KProtocolInfo::isKnownProtocol( KURL("ldap://localhost") ) ) {
    mNoLDAPLookup = true;
    return;
  }

  // stolen from KAddressBook
  KConfig config( "kabldaprc", true );
  config.setGroup( "LDAP" );
  int numHosts = config.readUnsignedNumEntry( "NumSelectedHosts");
  if ( !numHosts ) {
    mNoLDAPLookup = true;
    return;
  } else {
    for ( int j = 0; j < numHosts; j++ ) {
      LdapClient* ldapClient = new LdapClient( this );

      TQString host =  config.readEntry( TQString( "SelectedHost%1" ).arg( j ), "" ).stripWhiteSpace();
      if ( !host.isEmpty() )
        ldapClient->setHost( host );

      TQString port = TQString::number( config.readUnsignedNumEntry( TQString( "SelectedPort%1" ).arg( j ) ) );
      if ( !port.isEmpty() )
        ldapClient->setPort( port );

      TQString base = config.readEntry( TQString( "SelectedBase%1" ).arg( j ), "" ).stripWhiteSpace();
      if ( !base.isEmpty() )
        ldapClient->setBase( base );

      TQString bindDN = config.readEntry( TQString( "SelectedBind%1" ).arg( j ) ).stripWhiteSpace();
      if ( !bindDN.isEmpty() )
        ldapClient->setBindDN( bindDN );

      TQString pwdBindDN = config.readEntry( TQString( "SelectedPwdBind%1" ).arg( j ) );
      if ( !pwdBindDN.isEmpty() )
        ldapClient->setPwdBindDN( pwdBindDN );

      TQStringList attrs;
      attrs << "cn" << "mail" << "givenname" << "sn";
      ldapClient->setAttrs( attrs );

      connect( ldapClient, TQT_SIGNAL( result( const KABC::LdapObject& ) ),
               this, TQT_SLOT( slotLDAPResult( const KABC::LdapObject& ) ) );
      connect( ldapClient, TQT_SIGNAL( done() ),
               this, TQT_SLOT( slotLDAPDone() ) );
      connect( ldapClient, TQT_SIGNAL( error( const TQString& ) ),
               this, TQT_SLOT( slotLDAPError( const TQString& ) ) );

      mClients.append( ldapClient );
    }
  }

  connect( &mDataTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( slotDataTimer() ) );
}

void LdapSearch::startSearch( const TQString& txt )
{
  if ( mNoLDAPLookup )
    return;

  cancelSearch();

  int pos = txt.find( '\"' );
  if( pos >= 0 )
  {
    ++pos;
    int pos2 = txt.find( '\"', pos );
    if( pos2 >= 0 )
        mSearchText = txt.mid( pos , pos2 - pos );
    else
        mSearchText = txt.mid( pos );
  } else
    mSearchText = txt;

  TQString filter = TQString( "|(cn=%1*)(mail=%2*)(givenName=%3*)(sn=%4*)" )
      .arg( mSearchText ).arg( mSearchText ).arg( mSearchText ).arg( mSearchText );

  TQValueList< LdapClient* >::Iterator it;
  for ( it = mClients.begin(); it != mClients.end(); ++it ) {
    (*it)->startQuery( filter );
    ++mActiveClients;
  }
}

void LdapSearch::cancelSearch()
{
  TQValueList< LdapClient* >::Iterator it;
  for ( it = mClients.begin(); it != mClients.end(); ++it )
    (*it)->cancelQuery();

  mActiveClients = 0;
  mResults.clear();
}

void LdapSearch::slotLDAPResult( const KABC::LdapObject& obj )
{
  mResults.append( obj );
  if ( !mDataTimer.isActive() )
    mDataTimer.start( 500, true );
}

void LdapSearch::slotLDAPError( const TQString& )
{
  slotLDAPDone();
}

void LdapSearch::slotLDAPDone()
{
  if ( --mActiveClients > 0 )
    return;

  finish();
}

void LdapSearch::slotDataTimer()
{
  TQStringList lst;
  LdapResultList reslist;
  makeSearchData( lst, reslist );
  if ( !lst.isEmpty() )
    emit searchData( lst );
  if ( !reslist.isEmpty() )
    emit searchData( reslist );
}

void LdapSearch::finish()
{
  mDataTimer.stop();

  slotDataTimer(); // emit final bunch of data
  emit searchDone();
}

void LdapSearch::makeSearchData( TQStringList& ret, LdapResultList& resList )
{
  TQString search_text_upper = mSearchText.upper();

  TQValueList< KABC::LdapObject >::ConstIterator it1;
  for ( it1 = mResults.begin(); it1 != mResults.end(); ++it1 ) {
    TQString name, mail, givenname, sn;

    LdapAttrMap::ConstIterator it2;
    for ( it2 = (*it1).attrs.begin(); it2 != (*it1).attrs.end(); ++it2 ) {
      TQString tmp = TQString::fromUtf8( (*it2).first(), (*it2).first().size() );
      if ( it2.key() == "cn" )
        name = tmp; // TODO loop?
      else if( it2.key() == "mail" )
        mail = tmp;
      else if( it2.key() == "givenName" )
        givenname = tmp;
      else if( it2.key() == "sn" )
        sn = tmp;
    }

    if( mail.isEmpty())
      continue; // nothing, bad entry
    else if ( name.isEmpty() )
      ret.append( mail );
    else {
      kdDebug(5700) << "<" << name << "><" << mail << ">" << endl;
      ret.append( TQString( "%1 <%2>" ).arg( name ).arg( mail ) );
    }

    LdapResult sr;
    sr.clientNumber = mClients.findIndex( (*it1).client );
    sr.name = name;
    sr.email = mail;
    resList.append( sr );
  }

  mResults.clear();
}

bool LdapSearch::isAvailable() const
{
  return !mNoLDAPLookup;
}



#include "ldapclient.moc"
