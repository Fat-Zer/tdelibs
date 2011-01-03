/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <tqapplication.h>
#include <tqbuffer.h>
#include <tqfile.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstringhandler.h>
#include <ktempfile.h>

#include <stdlib.h>
#include <kio/netaccess.h>
#include <kabc/ldif.h>
#include <kabc/ldapurl.h>

#include "resourceldapkio.h"
#include "resourceldapkioconfig.h"

using namespace KABC;

// Hack from Netaccess
void qt_enter_modal( TQWidget *widget );
void qt_leave_modal( TQWidget *widget );

class ResourceLDAPKIO::ResourceLDAPKIOPrivate 
{
  public:
    LDIF mLdif;
    bool mTLS,mSSL,mSubTree;
    TQString mResultDn;
    Addressee mAddr;
    Address mAd;
    Resource::Iterator mSaveIt;
    bool mSASL;
    TQString mMech;
    TQString mRealm, mBindDN;
    LDAPUrl mLDAPUrl;
    int mVer, mSizeLimit, mTimeLimit, mRDNPrefix;
    int mError;
    int mCachePolicy;
    bool mReadOnly;
    bool mAutoCache;
    TQString mCacheDst;
    KTempFile *mTmp;
};

ResourceLDAPKIO::ResourceLDAPKIO( const KConfig *config )
  : Resource( config )
{
  d = new ResourceLDAPKIOPrivate;
  if ( config ) {
    TQMap<TQString, TQString> attrList;
    TQStringList attributes = config->readListEntry( "LdapAttributes" );
    for ( uint pos = 0; pos < attributes.count(); pos += 2 )
      mAttributes.insert( attributes[ pos ], attributes[ pos + 1 ] );

    mUser = config->readEntry( "LdapUser" );
    mPassword = KStringHandler::obscure( config->readEntry( "LdapPassword" ) );
    mDn = config->readEntry( "LdapDn" );
    mHost = config->readEntry( "LdapHost" );
    mPort = config->readNumEntry( "LdapPort", 389 );
    mFilter = config->readEntry( "LdapFilter" );
    mAnonymous = config->readBoolEntry( "LdapAnonymous" );
    d->mTLS = config->readBoolEntry( "LdapTLS" );
    d->mSSL = config->readBoolEntry( "LdapSSL" );
    d->mSubTree = config->readBoolEntry( "LdapSubTree" );
    d->mSASL = config->readBoolEntry( "LdapSASL" );
    d->mMech = config->readEntry( "LdapMech" );
    d->mRealm = config->readEntry( "LdapRealm" );
    d->mBindDN = config->readEntry( "LdapBindDN" );
    d->mVer = config->readNumEntry( "LdapVer", 3 );
    d->mTimeLimit = config->readNumEntry( "LdapTimeLimit", 0 );
    d->mSizeLimit = config->readNumEntry( "LdapSizeLimit", 0 );
    d->mRDNPrefix = config->readNumEntry( "LdapRDNPrefix", 0 );
    d->mCachePolicy = config->readNumEntry( "LdapCachePolicy", 0 );
    d->mAutoCache = config->readBoolEntry( "LdapAutoCache", true );
  } else {
    mPort = 389;
    mAnonymous = true;
    mUser = mPassword = mHost =  mFilter =  mDn = "";
    d->mMech = d->mRealm = d->mBindDN = "";
    d->mTLS = d->mSSL = d->mSubTree = d->mSASL = false;
    d->mVer = 3; d->mRDNPrefix = 0;
    d->mTimeLimit = d->mSizeLimit = 0;
    d->mCachePolicy = Cache_No;
    d->mAutoCache = true;
  }
  d->mCacheDst = KGlobal::dirs()->saveLocation("cache", "ldapkio") + "/" +
    type() + "_" + identifier();
  init(); 
}

ResourceLDAPKIO::~ResourceLDAPKIO() 
{
  delete d;
}

void ResourceLDAPKIO::enter_loop()
{
  TQWidget dummy(0,0,WType_Dialog | WShowModal);
  dummy.setFocusPolicy( TQWidget::NoFocus );
  qt_enter_modal(&dummy);
  tqApp->enter_loop();
  qt_leave_modal(&dummy);
}

void ResourceLDAPKIO::entries( KIO::Job*, const KIO::UDSEntryList & list )
{
  KIO::UDSEntryListConstIterator it = list.begin();
  KIO::UDSEntryListConstIterator end = list.end();
  for (; it != end; ++it) {
    KIO::UDSEntry::ConstIterator it2 = (*it).begin();
    for( ; it2 != (*it).end(); it2++ ) {
      if ( (*it2).m_uds == KIO::UDS_URL ) {
        KURL tmpurl( (*it2).m_str );
        d->mResultDn = tmpurl.path();
        kdDebug(7125) << "tqfindUid(): " << d->mResultDn << endl;
        if ( d->mResultDn.startsWith("/") ) d->mResultDn.remove(0,1);
        return;
      }
    }
  }
}

void ResourceLDAPKIO::listResult( KIO::Job *job)
{
  d->mError = job->error();  
  if ( d->mError && d->mError != KIO::ERR_USER_CANCELED )
    mErrorMsg = job->errorString();
  else 
    mErrorMsg = "";
  tqApp->exit_loop();
}

TQString ResourceLDAPKIO::tqfindUid( const TQString &uid ) 
{
  LDAPUrl url( d->mLDAPUrl );
  KIO::UDSEntry entry;
  
  mErrorMsg = d->mResultDn = "";

  url.setAttributes("dn");
  url.setFilter( "(" + mAttributes[ "uid" ] + "=" + uid + ")" + mFilter );
  url.setExtension( "x-dir", "one" );

  kdDebug(7125) << "ResourceLDAPKIO::tqfindUid() uid: " << uid << " url " << 
    url.prettyURL() << endl;
  
  KIO::ListJob * listJob = KIO::listDir( url, false /* no GUI */ );
  connect( listJob, 
    TQT_SIGNAL( entries( KIO::Job *, const KIO::UDSEntryList& ) ),
    TQT_SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );
  connect( listJob, TQT_SIGNAL( result( KIO::Job* ) ), 
    this, TQT_SLOT( listResult( KIO::Job* ) ) );

  enter_loop();
  return d->mResultDn;
}

TQCString ResourceLDAPKIO::addEntry( const TQString &attr, const TQString &value, bool mod )
{
  TQCString tmp;
  if ( !attr.isEmpty() ) {
    if ( mod ) tmp += LDIF::assembleLine( "tqreplace", attr ) + "\n";
    tmp += LDIF::assembleLine( attr, value ) + "\n";
    if ( mod ) tmp += "-\n"; 
  }
  return ( tmp );
}

bool ResourceLDAPKIO::AddresseeToLDIF( TQByteArray &ldif, const Addressee &addr, 
  const TQString &olddn )
{
  TQCString tmp;
  TQString dn;
  TQByteArray data;
  bool mod = false;
  
  if ( olddn.isEmpty() ) {
    //insert new entry
    switch ( d->mRDNPrefix ) {
      case 1:
        dn = mAttributes[ "uid" ] + "=" + addr.uid() + "," +mDn;
        break;
      case 0:
      default:  
        dn = mAttributes[ "commonName" ] + "=" + addr.assembledName() + "," +mDn;
        break;
    }
  } else {
    //modify existing entry
    mod = true;
    if ( olddn.startsWith( mAttributes[ "uid" ] ) ) {
      dn = mAttributes[ "uid" ] + "=" + addr.uid() + "," + olddn.section( ',', 1 );
    } else if ( olddn.startsWith( mAttributes[ "commonName" ] ) ) {
      dn = mAttributes[ "commonName" ] + "=" + addr.assembledName() + "," + 
        olddn.section( ',', 1 );
    } else {
      dn = olddn;
    }
    
    if ( olddn.lower() != dn.lower() ) {
      tmp = LDIF::assembleLine( "dn", olddn ) + "\n";
      tmp += "changetype: modrdn\n";
      tmp += LDIF::assembleLine( "newrdn", dn.section( ',', 0, 0 ) ) + "\n";
      tmp += "deleteoldrdn: 1\n\n";
    }
  }
  
  
  tmp += LDIF::assembleLine( "dn", dn ) + "\n";
  if ( mod ) tmp += "changetype: modify\n";
  if ( !mod ) {
    tmp += "objectClass: top\n";
    TQStringList obclass = TQStringList::split( ',', mAttributes[ "objectClass" ] );
    for ( TQStringList::iterator it = obclass.begin(); it != obclass.end(); it++ ) {
      tmp += LDIF::assembleLine( "objectClass", *it ) + "\n";
    }
  }
  
  tmp += addEntry( mAttributes[ "commonName" ], addr.assembledName(), mod );
  tmp += addEntry( mAttributes[ "formattedName" ], addr.formattedName(), mod );
  tmp += addEntry( mAttributes[ "givenName" ], addr.givenName(), mod );
  tmp += addEntry( mAttributes[ "familyName" ], addr.familyName(), mod );
  tmp += addEntry( mAttributes[ "uid" ], addr.uid(), mod );

  PhoneNumber number;
  number = addr.phoneNumber( PhoneNumber::Home );
  tmp += addEntry( mAttributes[ "phoneNumber" ], number.number().utf8(), mod );
  number = addr.phoneNumber( PhoneNumber::Work );
  tmp += addEntry( mAttributes[ "telephoneNumber" ], number.number().utf8(), mod );
  number = addr.phoneNumber( PhoneNumber::Fax );
  tmp += addEntry( mAttributes[ "facsimileTelephoneNumber" ], number.number().utf8(), mod );
  number = addr.phoneNumber( PhoneNumber::Cell );
  tmp += addEntry( mAttributes[ "mobile" ], number.number().utf8(), mod );
  number = addr.phoneNumber( PhoneNumber::Pager );
  tmp += addEntry( mAttributes[ "pager" ], number.number().utf8(), mod );

  tmp += addEntry( mAttributes[ "description" ], addr.note(), mod );
  tmp += addEntry( mAttributes[ "title" ], addr.title(), mod );
  tmp += addEntry( mAttributes[ "organization" ], addr.organization(), mod );

  Address ad = addr.address( Address::Home );
  if ( !ad.isEmpty() ) {
    tmp += addEntry( mAttributes[ "street" ], ad.street(), mod );
    tmp += addEntry( mAttributes[ "state" ], ad.region(), mod );
    tmp += addEntry( mAttributes[ "city" ], ad.locality(), mod );
    tmp += addEntry( mAttributes[ "postalcode" ], ad.postalCode(), mod );
  }
  
  TQStringList emails = addr.emails();
  TQStringList::ConstIterator mailIt = emails.begin();
  
  if ( !mAttributes[ "mail" ].isEmpty() ) {
    if ( mod ) tmp += 
      LDIF::assembleLine( "tqreplace", mAttributes[ "mail" ] ) + "\n";
    if ( mailIt != emails.end() ) {
      tmp += LDIF::assembleLine( mAttributes[ "mail" ], *mailIt ) + "\n";
      mailIt ++;
    }
    if ( mod && mAttributes[ "mail" ] != mAttributes[ "mailAlias" ] ) tmp += "-\n"; 
  }
    
  if ( !mAttributes[ "mailAlias" ].isEmpty() ) {
    if ( mod && mAttributes[ "mail" ] != mAttributes[ "mailAlias" ] ) tmp += 
      LDIF::assembleLine( "tqreplace", mAttributes[ "mailAlias" ] ) + "\n";
    for ( ; mailIt != emails.end(); ++mailIt ) {
      tmp += LDIF::assembleLine( mAttributes[ "mailAlias" ], *mailIt ) + "\n" ;
    }
    if ( mod ) tmp += "-\n";
  }
  
  if ( !mAttributes[ "jpegPhoto" ].isEmpty() ) {
    TQByteArray pic;
    TQBuffer buffer( pic );
    buffer.open( IO_WriteOnly );
    addr.photo().data().save( &buffer, "JPEG" );
    
    if ( mod ) tmp += 
      LDIF::assembleLine( "tqreplace", mAttributes[ "jpegPhoto" ] ) + "\n";
    tmp += LDIF::assembleLine( mAttributes[ "jpegPhoto" ], pic, 76 ) + "\n";
    if ( mod ) tmp += "-\n";
  }
  
  tmp += "\n";  
  kdDebug(7125) << "ldif: " << TQString::fromUtf8(tmp) << endl;
  ldif = tmp;
  return true;
}

void ResourceLDAPKIO::setReadOnly( bool value )
{
  //save the original readonly flag, because offline using disables writing
  d->mReadOnly = true;
  Resource::setReadOnly( value );
}

void ResourceLDAPKIO::init()
{
  if ( mPort == 0 ) mPort = 389;

  /**
    If you want to add new attributes, append them here, add a
    translation string in the ctor of AttributesDialog and
    handle them in the load() method below.
    These are the default values
   */
  if ( !mAttributes.tqcontains("objectClass") )
    mAttributes.insert( "objectClass", "inetOrgPerson" );
  if ( !mAttributes.tqcontains("commonName") )
    mAttributes.insert( "commonName", "cn" );
  if ( !mAttributes.tqcontains("formattedName") )
    mAttributes.insert( "formattedName", "displayName" );
  if ( !mAttributes.tqcontains("familyName") )
    mAttributes.insert( "familyName", "sn" );
  if ( !mAttributes.tqcontains("givenName") )
    mAttributes.insert( "givenName", "givenName" );
  if ( !mAttributes.tqcontains("mail") )
    mAttributes.insert( "mail", "mail" );
  if ( !mAttributes.tqcontains("mailAlias") )
    mAttributes.insert( "mailAlias", "" );
  if ( !mAttributes.tqcontains("phoneNumber") )
    mAttributes.insert( "phoneNumber", "homePhone" );
  if ( !mAttributes.tqcontains("telephoneNumber") )
    mAttributes.insert( "telephoneNumber", "telephoneNumber" );
  if ( !mAttributes.tqcontains("facsimileTelephoneNumber") )
    mAttributes.insert( "facsimileTelephoneNumber", "facsimileTelephoneNumber" );
  if ( !mAttributes.tqcontains("mobile") )
    mAttributes.insert( "mobile", "mobile" );
  if ( !mAttributes.tqcontains("pager") )
    mAttributes.insert( "pager", "pager" );
  if ( !mAttributes.tqcontains("description") )
    mAttributes.insert( "description", "description" );

  if ( !mAttributes.tqcontains("title") )
    mAttributes.insert( "title", "title" );
  if ( !mAttributes.tqcontains("street") )
    mAttributes.insert( "street", "street" );
  if ( !mAttributes.tqcontains("state") )
    mAttributes.insert( "state", "st" );
  if ( !mAttributes.tqcontains("city") )
    mAttributes.insert( "city", "l" );
  if ( !mAttributes.tqcontains("organization") )
    mAttributes.insert( "organization", "o" );
  if ( !mAttributes.tqcontains("postalcode") )
    mAttributes.insert( "postalcode", "postalCode" );

  if ( !mAttributes.tqcontains("uid") )
    mAttributes.insert( "uid", "uid" );
  if ( !mAttributes.tqcontains("jpegPhoto") )
    mAttributes.insert( "jpegPhoto", "jpegPhoto" );

  d->mLDAPUrl = KURL();
  if ( !mAnonymous ) {
    d->mLDAPUrl.setUser( mUser );
    d->mLDAPUrl.setPass( mPassword );
  }
  d->mLDAPUrl.setProtocol( d->mSSL ? "ldaps" : "ldap");
  d->mLDAPUrl.setHost( mHost );
  d->mLDAPUrl.setPort( mPort );
  d->mLDAPUrl.setDn( mDn );

  if (!mAttributes.empty()) {
    TQMap<TQString,TQString>::Iterator it;
    TQStringList attr;
    for ( it = mAttributes.begin(); it != mAttributes.end(); ++it ) {
      if ( !it.data().isEmpty() && it.key() != "objectClass" ) 
        attr.append( it.data() );
    }
    d->mLDAPUrl.setAttributes( attr );
  }

  d->mLDAPUrl.setScope( d->mSubTree ? LDAPUrl::Sub : LDAPUrl::One );
  if ( !mFilter.isEmpty() && mFilter != "(objectClass=*)" ) 
    d->mLDAPUrl.setFilter( mFilter );
  d->mLDAPUrl.setExtension( "x-dir", "base" );
  if ( d->mTLS ) d->mLDAPUrl.setExtension( "x-tls", "" );
  d->mLDAPUrl.setExtension( "x-ver", TQString::number( d->mVer ) );
  if ( d->mSizeLimit ) 
    d->mLDAPUrl.setExtension( "x-sizelimit", TQString::number( d->mSizeLimit ) );
  if ( d->mTimeLimit ) 
    d->mLDAPUrl.setExtension( "x-timelimit", TQString::number( d->mTimeLimit ) );
  if ( d->mSASL ) {
    d->mLDAPUrl.setExtension( "x-sasl", "" );
    if ( !d->mBindDN.isEmpty() ) d->mLDAPUrl.setExtension( "bindname", d->mBindDN );
    if ( !d->mMech.isEmpty() ) d->mLDAPUrl.setExtension( "x-mech", d->mMech );
    if ( !d->mRealm.isEmpty() ) d->mLDAPUrl.setExtension( "x-realm", d->mRealm );
  }

  d->mReadOnly = readOnly();

  kdDebug(7125) << "resource_ldapkio url: " << d->mLDAPUrl.prettyURL() << endl;
}

void ResourceLDAPKIO::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writeEntry( "LdapUser", mUser );
  config->writeEntry( "LdapPassword", KStringHandler::obscure( mPassword ) );
  config->writeEntry( "LdapDn", mDn );
  config->writeEntry( "LdapHost", mHost );
  config->writeEntry( "LdapPort", mPort );
  config->writeEntry( "LdapFilter", mFilter );
  config->writeEntry( "LdapAnonymous", mAnonymous );
  config->writeEntry( "LdapTLS", d->mTLS );
  config->writeEntry( "LdapSSL", d->mSSL );
  config->writeEntry( "LdapSubTree", d->mSubTree );
  config->writeEntry( "LdapSASL", d->mSASL );
  config->writeEntry( "LdapMech", d->mMech );
  config->writeEntry( "LdapVer", d->mVer );
  config->writeEntry( "LdapTimeLimit", d->mTimeLimit );
  config->writeEntry( "LdapSizeLimit", d->mSizeLimit );
  config->writeEntry( "LdapRDNPrefix", d->mRDNPrefix );
  config->writeEntry( "LdapRealm", d->mRealm );
  config->writeEntry( "LdapBindDN", d->mBindDN );
  config->writeEntry( "LdapCachePolicy", d->mCachePolicy );
  config->writeEntry( "LdapAutoCache", d->mAutoCache );

  TQStringList attributes;
  TQMap<TQString, TQString>::Iterator it;
  for ( it = mAttributes.begin(); it != mAttributes.end(); ++it )
    attributes << it.key() << it.data();

  config->writeEntry( "LdapAttributes", attributes );
}

Ticket *ResourceLDAPKIO::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(7125) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceLDAPKIO::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceLDAPKIO::doOpen()
{
  return true;
}

void ResourceLDAPKIO::doClose()
{
}

void ResourceLDAPKIO::createCache()
{
  d->mTmp = NULL;
  if ( d->mCachePolicy == Cache_NoConnection && d->mAutoCache ) {
    d->mTmp = new KTempFile( d->mCacheDst, "tmp" );
    d->mTmp->setAutoDelete( true );
  }
}

void ResourceLDAPKIO::activateCache()
{
  if ( d->mTmp && d->mError == 0 ) {
    d->mTmp->close();
    rename( TQFile::encodeName( d->mTmp->name() ), TQFile::encodeName( d->mCacheDst ) );
  }
  if ( d->mTmp ) {
    delete d->mTmp;
    d->mTmp = 0;
  }
}

KIO::Job *ResourceLDAPKIO::loadFromCache()
{
  KIO::Job *job = NULL;
  if ( d->mCachePolicy == Cache_Always || 
     ( d->mCachePolicy == Cache_NoConnection && 
      d->mError == KIO::ERR_COULD_NOT_CONNECT ) ) {

    d->mAddr = Addressee();
    d->mAd = Address( Address::Home );
    //initialize ldif parser
    d->mLdif.startParsing();

    Resource::setReadOnly( true );
  
    KURL url( d->mCacheDst );
    job = KIO::get( url, true, false );
    connect( job, TQT_SIGNAL( data( KIO::Job*, const TQByteArray& ) ),
      this, TQT_SLOT( data( KIO::Job*, const TQByteArray& ) ) );
  }
  return job;
}

bool ResourceLDAPKIO::load()
{
  kdDebug(7125) << "ResourceLDAPKIO::load()" << endl;
  KIO::Job *job;

  clear();
  //clear the addressee
  d->mAddr = Addressee();
  d->mAd = Address( Address::Home );
  //initialize ldif parser
  d->mLdif.startParsing();

  //set to original settings, offline use will disable writing
  Resource::setReadOnly( d->mReadOnly );

  createCache();
  if ( d->mCachePolicy != Cache_Always ) {
    job = KIO::get( d->mLDAPUrl, true, false );
    connect( job, TQT_SIGNAL( data( KIO::Job*, const TQByteArray& ) ),
      this, TQT_SLOT( data( KIO::Job*, const TQByteArray& ) ) );
    connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
      this, TQT_SLOT( syncLoadSaveResult( KIO::Job* ) ) );
    enter_loop();
  }

  job = loadFromCache();    
  if ( job ) {
    connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
      this, TQT_SLOT( syncLoadSaveResult( KIO::Job* ) ) );
    enter_loop();
  }
  if ( mErrorMsg.isEmpty() ) {
    kdDebug(7125) << "ResourceLDAPKIO load ok!" << endl; 
    return true;
  } else {
    kdDebug(7125) << "ResourceLDAPKIO load finished with error: " << mErrorMsg << endl; 
    addressBook()->error( mErrorMsg );
    return false;
  }
}

bool ResourceLDAPKIO::asyncLoad()
{
  clear();
  //clear the addressee
  d->mAddr = Addressee();
  d->mAd = Address( Address::Home );
  //initialize ldif parser
  d->mLdif.startParsing();

  Resource::setReadOnly( d->mReadOnly );

  createCache();
  if ( d->mCachePolicy != Cache_Always ) {
    KIO::Job *job = KIO::get( d->mLDAPUrl, true, false );
    connect( job, TQT_SIGNAL( data( KIO::Job*, const TQByteArray& ) ),
      this, TQT_SLOT( data( KIO::Job*, const TQByteArray& ) ) );
    connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
      this, TQT_SLOT( result( KIO::Job* ) ) );
  } else {
    result( NULL );
  }
  return true;
}

void ResourceLDAPKIO::data( KIO::Job *, const TQByteArray &data )
{
  if ( data.size() ) {
    d->mLdif.setLDIF( data );
    if ( d->mTmp ) {
      d->mTmp->file()->writeBlock( data );
    }
  } else {
    d->mLdif.endLDIF();
  }
  
  LDIF::ParseVal ret;
  TQString name;
  TQByteArray value;
  do {
    ret = d->mLdif.nextItem();
    switch ( ret ) {
      case LDIF::NewEntry:
        kdDebug(7125) << "new entry: " << d->mLdif.dn() << endl;
        break;
      case LDIF::Item:
        name = d->mLdif.attr().lower();  
        value = d->mLdif.val();      
        if ( name == mAttributes[ "commonName" ].lower() ) {
          if ( !d->mAddr.formattedName().isEmpty() ) {
            TQString fn = d->mAddr.formattedName();
            d->mAddr.setNameFromString( TQString::fromUtf8( value, value.size() ) );
            d->mAddr.setFormattedName( fn );
          } else
            d->mAddr.setNameFromString( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "formattedName" ].lower() ) {
          d->mAddr.setFormattedName( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "givenName" ].lower() ) {
          d->mAddr.setGivenName( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "mail" ].lower() ) {
          d->mAddr.insertEmail( TQString::fromUtf8( value, value.size() ), true );
        } else if ( name == mAttributes[ "mailAlias" ].lower() ) {
          d->mAddr.insertEmail( TQString::fromUtf8( value, value.size() ), false );
        } else if ( name == mAttributes[ "phoneNumber" ].lower() ) {
          PhoneNumber phone;
          phone.setNumber( TQString::fromUtf8( value, value.size() ) );
          d->mAddr.insertPhoneNumber( phone );
        } else if ( name == mAttributes[ "telephoneNumber" ].lower() ) {
          PhoneNumber phone( TQString::fromUtf8( value, value.size() ), 
            PhoneNumber::Work );
          d->mAddr.insertPhoneNumber( phone );
        } else if ( name == mAttributes[ "facsimileTelephoneNumber" ].lower() ) {
          PhoneNumber phone( TQString::fromUtf8( value, value.size() ), 
            PhoneNumber::Fax );
          d->mAddr.insertPhoneNumber( phone );
        } else if ( name == mAttributes[ "mobile" ].lower() ) {
          PhoneNumber phone( TQString::fromUtf8( value, value.size() ), 
            PhoneNumber::Cell );
          d->mAddr.insertPhoneNumber( phone );
        } else if ( name == mAttributes[ "pager" ].lower() ) {
          PhoneNumber phone( TQString::fromUtf8( value, value.size() ), 
            PhoneNumber::Pager );
          d->mAddr.insertPhoneNumber( phone );
        } else if ( name == mAttributes[ "description" ].lower() ) {
          d->mAddr.setNote( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "title" ].lower() ) {
          d->mAddr.setTitle( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "street" ].lower() ) {
          d->mAd.setStreet( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "state" ].lower() ) {
          d->mAd.setRegion( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "city" ].lower() ) {
          d->mAd.setLocality( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "postalcode" ].lower() ) {
          d->mAd.setPostalCode( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "organization" ].lower() ) {
          d->mAddr.setOrganization( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "familyName" ].lower() ) {
          d->mAddr.setFamilyName( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "uid" ].lower() ) {
          d->mAddr.setUid( TQString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "jpegPhoto" ].lower() ) {
          KABC::Picture photo;
          TQImage img( value );
          if ( !img.isNull() ) {
            photo.setData( img );
            photo.setType( "image/jpeg" );
            d->mAddr.setPhoto( photo );
          }
        }

        break;
      case LDIF::EndEntry: {
        d->mAddr.setResource( this );
        d->mAddr.insertAddress( d->mAd );
        d->mAddr.setChanged( false );
        insertAddressee( d->mAddr );
        //clear the addressee
        d->mAddr = Addressee();
        d->mAd = Address( Address::Home );
        }
        break;
      default:
        break;
    }
  } while ( ret != LDIF::MoreData );
}

void ResourceLDAPKIO::loadCacheResult( KIO::Job *job )
{
  mErrorMsg = "";
  d->mError = job->error();
  if ( d->mError && d->mError != KIO::ERR_USER_CANCELED ) {
    mErrorMsg = job->errorString();
  }
  if ( !mErrorMsg.isEmpty() )
    emit loadingError( this, mErrorMsg );
  else
    emit loadingFinished( this );
}

void ResourceLDAPKIO::result( KIO::Job *job )
{
  mErrorMsg = "";
  if ( job ) {
    d->mError = job->error();
    if ( d->mError && d->mError != KIO::ERR_USER_CANCELED ) {
      mErrorMsg = job->errorString();
    }
  } else {
    d->mError = 0;
  }
  activateCache();

  KIO::Job *cjob;
  cjob = loadFromCache();
  if ( cjob ) {
    connect( cjob, TQT_SIGNAL( result( KIO::Job* ) ),
      this, TQT_SLOT( loadCacheResult( KIO::Job* ) ) );
  } else {
    if ( !mErrorMsg.isEmpty() )
      emit loadingError( this, mErrorMsg );
    else
      emit loadingFinished( this );
  }
}

bool ResourceLDAPKIO::save( Ticket* )
{
  kdDebug(7125) << "ResourceLDAPKIO save" << endl;
  
  d->mSaveIt = begin();
  KIO::Job *job = KIO::put( d->mLDAPUrl, -1, true, false, false );
  connect( job, TQT_SIGNAL( dataReq( KIO::Job*, TQByteArray& ) ),
    this, TQT_SLOT( saveData( KIO::Job*, TQByteArray& ) ) );
  connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
    this, TQT_SLOT( syncLoadSaveResult( KIO::Job* ) ) );
  enter_loop();
  if ( mErrorMsg.isEmpty() ) {
    kdDebug(7125) << "ResourceLDAPKIO save ok!" << endl; 
    return true;
  } else {
    kdDebug(7125) << "ResourceLDAPKIO finished with error: " << mErrorMsg << endl; 
    addressBook()->error( mErrorMsg );
    return false;
  }
}

bool ResourceLDAPKIO::asyncSave( Ticket* )
{
  kdDebug(7125) << "ResourceLDAPKIO asyncSave" << endl;
  d->mSaveIt = begin();
  KIO::Job *job = KIO::put( d->mLDAPUrl, -1, true, false, false );
  connect( job, TQT_SIGNAL( dataReq( KIO::Job*, TQByteArray& ) ),
    this, TQT_SLOT( saveData( KIO::Job*, TQByteArray& ) ) );
  connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
    this, TQT_SLOT( saveResult( KIO::Job* ) ) );
  return true;
}

void ResourceLDAPKIO::syncLoadSaveResult( KIO::Job *job )
{
  d->mError = job->error();
  if ( d->mError && d->mError != KIO::ERR_USER_CANCELED )
    mErrorMsg = job->errorString();
  else
    mErrorMsg = "";
  activateCache();
  
  tqApp->exit_loop();
}

void ResourceLDAPKIO::saveResult( KIO::Job *job )
{
  d->mError = job->error();
  if ( d->mError && d->mError != KIO::ERR_USER_CANCELED )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );
}

void ResourceLDAPKIO::saveData( KIO::Job*, TQByteArray& data )
{
  while ( d->mSaveIt != end() &&
       !(*d->mSaveIt).changed() ) d->mSaveIt++;

  if ( d->mSaveIt == end() ) {
    kdDebug(7125) << "ResourceLDAPKIO endData" << endl;
    data.resize(0);
    return;
  }
  
  kdDebug(7125) << "ResourceLDAPKIO saveData: " << (*d->mSaveIt).assembledName() << endl;
  
  AddresseeToLDIF( data, *d->mSaveIt, tqfindUid( (*d->mSaveIt).uid() ) );  
//  kdDebug(7125) << "ResourceLDAPKIO save LDIF: " << TQString::fromUtf8(data) << endl;
  // mark as unchanged
  (*d->mSaveIt).setChanged( false );

  d->mSaveIt++;  
}

void ResourceLDAPKIO::removeAddressee( const Addressee& addr )
{
  TQString dn = tqfindUid( addr.uid() );
  
  kdDebug(7125) << "ResourceLDAPKIO: removeAddressee: " << dn << endl;

  if ( !mErrorMsg.isEmpty() ) {
    addressBook()->error( mErrorMsg );
    return;
  }
  if ( !dn.isEmpty() ) {
    kdDebug(7125) << "ResourceLDAPKIO: found uid: " << dn << endl;
    LDAPUrl url( d->mLDAPUrl );
    url.setPath( "/" + dn );
    url.setExtension( "x-dir", "base" );
    url.setScope( LDAPUrl::Base );
    if ( KIO::NetAccess::del( url, NULL ) ) mAddrMap.erase( addr.uid() );
  } else {
    //maybe it's not saved yet
    mAddrMap.erase( addr.uid() );
  }
}


void ResourceLDAPKIO::setUser( const TQString &user )
{
  mUser = user;
}

TQString ResourceLDAPKIO::user() const
{
  return mUser;
}

void ResourceLDAPKIO::setPassword( const TQString &password )
{
  mPassword = password;
}

TQString ResourceLDAPKIO::password() const
{
  return mPassword;
}

void ResourceLDAPKIO::setDn( const TQString &dn )
{
  mDn = dn;
}

TQString ResourceLDAPKIO::dn() const
{
  return mDn;
}

void ResourceLDAPKIO::setHost( const TQString &host )
{
  mHost = host;
}

TQString ResourceLDAPKIO::host() const
{
  return mHost;
}

void ResourceLDAPKIO::setPort( int port )
{
  mPort = port;
}

int ResourceLDAPKIO::port() const
{
  return mPort;
}

void ResourceLDAPKIO::setVer( int ver )
{
  d->mVer = ver;
}

int ResourceLDAPKIO::ver() const
{
  return d->mVer;
}
    
void ResourceLDAPKIO::setSizeLimit( int sizelimit )
{
  d->mSizeLimit = sizelimit;
}

int ResourceLDAPKIO::sizeLimit()
{
  return d->mSizeLimit;
}
    
void ResourceLDAPKIO::setTimeLimit( int timelimit )
{
  d->mTimeLimit = timelimit;
}

int ResourceLDAPKIO::timeLimit()
{
  return d->mTimeLimit;
}

void ResourceLDAPKIO::setFilter( const TQString &filter )
{
  mFilter = filter;
}

TQString ResourceLDAPKIO::filter() const
{
  return mFilter;
}

void ResourceLDAPKIO::setIsAnonymous( bool value )
{
  mAnonymous = value;
}

bool ResourceLDAPKIO::isAnonymous() const
{
  return mAnonymous;
}

void ResourceLDAPKIO::setIsTLS( bool value )
{
  d->mTLS = value;
}

bool ResourceLDAPKIO::isTLS() const
{
  return d->mTLS;
}
void ResourceLDAPKIO::setIsSSL( bool value )
{
  d->mSSL = value;
}

bool ResourceLDAPKIO::isSSL() const
{
  return d->mSSL;
}

void ResourceLDAPKIO::setIsSubTree( bool value )
{
  d->mSubTree = value;
}

bool ResourceLDAPKIO::isSubTree() const
{
  return d->mSubTree;
}

void ResourceLDAPKIO::setAttributes( const TQMap<TQString, TQString> &attributes )
{
  mAttributes = attributes;
}

TQMap<TQString, TQString> ResourceLDAPKIO::attributes() const
{
  return mAttributes;
}

void ResourceLDAPKIO::setRDNPrefix( int value )
{
  d->mRDNPrefix = value;
}

int ResourceLDAPKIO::RDNPrefix() const
{
  return d->mRDNPrefix;
}

void ResourceLDAPKIO::setIsSASL( bool value )
{
  d->mSASL = value;
}

bool ResourceLDAPKIO::isSASL() const
{
  return d->mSASL;
}

void ResourceLDAPKIO::setMech( const TQString &mech )
{
  d->mMech = mech;
}

TQString ResourceLDAPKIO::mech() const
{
  return d->mMech;
}

void ResourceLDAPKIO::setRealm( const TQString &realm )
{
  d->mRealm = realm;
}

TQString ResourceLDAPKIO::realm() const
{
  return d->mRealm;
}
    
void ResourceLDAPKIO::setBindDN( const TQString &binddn )
{
  d->mBindDN = binddn;
}

TQString ResourceLDAPKIO::bindDN() const
{
  return d->mBindDN;
}

void ResourceLDAPKIO::setCachePolicy( int pol )
{
  d->mCachePolicy = pol;
}

int ResourceLDAPKIO::cachePolicy() const
{
  return d->mCachePolicy;
}

void ResourceLDAPKIO::setAutoCache( bool value )
{
  d->mAutoCache = value;
}

bool ResourceLDAPKIO::autoCache()
{
  return d->mAutoCache;
}

TQString ResourceLDAPKIO::cacheDst() const
{
  return d->mCacheDst;
}    


#include "resourceldapkio.moc"
