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

#ifndef KABC_RESOURCELDAP_H
#define KABC_RESOURCELDAP_H

#include <kabc/resource.h>
#include <kabc/ldif.h>
#include <kio/job.h>

class KConfig;

namespace KABC {

class KABC_EXPORT ResourceLDAPKIO : public Resource
{
  Q_OBJECT

  public:
    enum CachePolicy{ Cache_No, Cache_NoConnection, Cache_Always };

    ResourceLDAPKIO( const KConfig* );
    virtual ~ResourceLDAPKIO();
    /**
     *  Call this after you used one of the set... methods 
     */
    virtual void init();
    
    virtual void writeConfig( KConfig* );

    virtual bool doOpen();
    virtual void doClose();

    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    virtual bool readOnly() const { return Resource::readOnly(); }
    virtual void setReadOnly( bool value );

    virtual bool load();
    virtual bool asyncLoad();
    virtual bool save( Ticket * ticket );
    virtual bool asyncSave( Ticket * ticket );

    virtual void removeAddressee( const Addressee& addr );

    void setUser( const TQString &user );
    TQString user() const;

    void setPassword( const TQString &password );
    TQString password() const;
    
    void setRealm( const TQString &realm );
    TQString realm() const;
    
    void setBindDN( const TQString &binddn );
    TQString bindDN() const;

    void setDn( const TQString &dn );
    TQString dn() const;

    void setHost( const TQString &host );
    TQString host() const;

    void setPort( int port );
    int port() const;

    void setVer( int ver );
    int ver() const;
    
    void setSizeLimit( int sizelimit );
    int sizeLimit();
    
    void setTimeLimit( int timelimit );
    int timeLimit();

    void setFilter( const TQString &filter );
    TQString filter() const;

    void setIsAnonymous( bool value );
    bool isAnonymous() const;

    void setAttributes( const TQMap<TQString, TQString> &attributes );
    TQMap<TQString, TQString> attributes() const;
    
    void setRDNPrefix( int value );
    int RDNPrefix() const;

    void setIsTLS( bool value );
    bool isTLS() const ;
    
    void setIsSSL( bool value );
    bool isSSL() const;
    
    void setIsSubTree( bool value );
    bool isSubTree() const ;

    void setIsSASL( bool value );
    bool isSASL() const ;

    void setMech( const TQString &mech );
    TQString mech() const;

    void setCachePolicy( int pol );
    int cachePolicy() const;

    void setAutoCache( bool value );
    bool autoCache();
    
    TQString cacheDst() const;
    
protected slots:
    void entries( KIO::Job*, const KIO::UDSEntryList& );
    void data( KIO::Job*, const TQByteArray& );
    void result( KIO::Job* );
    void listResult( KIO::Job* );
    void syncLoadSaveResult( KIO::Job* );
    void saveResult( KIO::Job* );
    void saveData( KIO::Job*, TQByteArray& );
    void loadCacheResult( KIO::Job* );
  
  private:
    TQString mUser;
    TQString mPassword;
    TQString mDn;
    TQString mHost;
    TQString mFilter;
    int mPort;
    bool mAnonymous;
    TQMap<TQString, TQString> mAttributes;

    KURL mLDAPUrl;
    int mGetCounter; //KDE 4: remove
    bool mErrorOccured; //KDE 4: remove
    TQString mErrorMsg;
    TQMap<KIO::Job*, TQByteArray> mJobMap; //KDE 4: remove

    KIO::Job *loadFromCache();
    void createCache();
    void activateCache();
    void enter_loop();
    TQCString addEntry( const TQString &attr, const TQString &value, bool mod );
    TQString tqfindUid( const TQString &uid );
    bool AddresseeToLDIF( TQByteArray &ldif, const Addressee &addr, 
      const TQString &olddn );
    
    class ResourceLDAPKIOPrivate;
    ResourceLDAPKIOPrivate *d;
};

}

#endif
