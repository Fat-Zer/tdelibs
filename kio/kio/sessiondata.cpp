/* This file is part of the KDE project
   Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street,
   Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <tqptrlist.h>
#include <tqtextcodec.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <dcopclient.h>
#include <kprotocolmanager.h>
#include <kstandarddirs.h>

#include <kdesu/client.h>
#include <kio/slaveconfig.h>
#include <kio/http_slave_defaults.h>

#include "sessiondata.h"
#include "sessiondata.moc"

namespace KIO {

/***************************** SessionData::AuthData ************************/
struct SessionData::AuthData
{

public:
  AuthData() {}

  AuthData(const TQCString& k, const TQCString& g, bool p) {
    key = k;
    group = g;
    persist = p;
  }

  bool isKeyMatch( const TQCString& val ) const {
    return (val==key);
  }

  bool isGroupMatch( const TQCString& val ) const {
    return (val==group);
  }

  TQCString key;
  TQCString group;
  bool persist;
};

/************************* SessionData::AuthDataList ****************************/
class SessionData::AuthDataList : public TQPtrList<SessionData::AuthData>
{
public:
  AuthDataList();
  ~AuthDataList();

  void addData( SessionData::AuthData* );
  void removeData( const TQCString& );

  bool pingCacheDaemon();
  void registerAuthData( SessionData::AuthData* );
  void unregisterAuthData( SessionData::AuthData* );
  void purgeCachedData();

private:
#ifdef Q_OS_UNIX
  KDEsuClient * m_kdesuClient;
#endif
};

SessionData::AuthDataList::AuthDataList()
{
#ifdef Q_OS_UNIX
  m_kdesuClient = new KDEsuClient;
#endif
  setAutoDelete(true);
}

SessionData::AuthDataList::~AuthDataList()
{
  purgeCachedData();
#ifdef Q_OS_UNIX
  delete m_kdesuClient;
  m_kdesuClient = 0;
#endif
}

void SessionData::AuthDataList::addData( SessionData::AuthData* d )
{
  TQPtrListIterator<SessionData::AuthData> it ( *this );
  for ( ; it.current(); ++it )
  {
    if ( it.current()->isKeyMatch( d->key ) )
        return;
  }
  registerAuthData( d );
  append( d );
}

void SessionData::AuthDataList::removeData( const TQCString& gkey )
{
  TQPtrListIterator<SessionData::AuthData> it( *this );
  for( ; it.current(); ++it )
  {
    if ( it.current()->isGroupMatch(gkey) &&  pingCacheDaemon() )
    {
        unregisterAuthData( it.current() );
        remove( it.current() );
    }
  }
}

bool SessionData::AuthDataList::pingCacheDaemon()
{
#ifdef Q_OS_UNIX
  Q_ASSERT(m_kdesuClient);

  int success = m_kdesuClient->ping();
  if( success == -1 )
  {
    success = m_kdesuClient->startServer();
    if( success == -1 )
        return false;
  }
  return true;
#else
  return false;
#endif
}

void SessionData::AuthDataList::registerAuthData( SessionData::AuthData* d )
{
  if( !pingCacheDaemon() )
    return;

#ifdef Q_OS_UNIX
  bool ok;
  TQCString ref_key = d->key + "-refcount";
  int count = m_kdesuClient->getVar(ref_key).toInt( &ok );
  if( ok )
  {
    TQCString val;
    val.setNum( count+1 );
    m_kdesuClient->setVar( ref_key, val, 0, d->group );
  }
  else
    m_kdesuClient->setVar( ref_key, "1", 0, d->group );
#endif
}

void SessionData::AuthDataList::unregisterAuthData( SessionData::AuthData* d )
{
  if ( !d  || d->persist )
    return;

  bool ok;
  int count;
  TQCString ref_key = d->key + "-refcount";

#ifdef Q_OS_UNIX
  count = m_kdesuClient->getVar( ref_key ).toInt( &ok );
  if ( ok )
  {
    if ( count > 1 )
    {
        TQCString val;
        val.setNum(count-1);
        m_kdesuClient->setVar( ref_key, val, 0, d->group );
    }
    else
    {
        m_kdesuClient->delVars(d->key);
    }
  }
#endif
}

void SessionData::AuthDataList::purgeCachedData()
{
  if ( !isEmpty() && pingCacheDaemon() )
  {
    TQPtrListIterator<SessionData::AuthData> it( *this );
    for ( ; it.current(); ++it )
        unregisterAuthData( it.current() );
  }
}

/********************************* SessionData ****************************/

class SessionData::SessionDataPrivate
{
public:
  SessionDataPrivate() {
    useCookie = true;
    initDone = false;
  }

  bool initDone;
  bool useCookie;
  TQString charsets;
  TQString language;
};

SessionData::SessionData()
{
  authData = 0;
  d = new SessionDataPrivate;
}

SessionData::~SessionData()
{
  delete d;
  delete authData;
  d = 0L;
  authData = 0L;
}

void SessionData::configDataFor( MetaData &configData, const TQString &proto,
                             const TQString & )
{
  if ( (proto.tqfind("http", 0, false) == 0 ) ||
     (proto.tqfind("webdav", 0, false) == 0) )
  {
    if (!d->initDone)
        reset();

    // These might have already been set so check first
    // to make sure that we do not trumpt settings sent
    // by apps or end-user.
    if ( configData["Cookies"].isEmpty() )
        configData["Cookies"] = d->useCookie ? "true" : "false";
    if ( configData["Languages"].isEmpty() )
        configData["Languages"] = d->language;
    if ( configData["Charsets"].isEmpty() )
        configData["Charsets"] = d->charsets;
    if ( configData["CacheDir"].isEmpty() )
        configData["CacheDir"] = KGlobal::dirs()->saveLocation("cache", "http");
    if ( configData["UserAgent"].isEmpty() )
    {
      configData["UserAgent"] = KProtocolManager::defaultUserAgent();
    }
  }
}

void SessionData::reset()
{
    d->initDone = true;
    // Get Cookie settings...
    KConfig* cfg = new KConfig("kcookiejarrc", true, false);
    cfg->setGroup( "Cookie Policy" );
    d->useCookie = cfg->readBoolEntry( "Cookies", true );
    delete cfg;

    static const TQString & english = KGlobal::staticQString( "en" );

    // Get language settings...
    TQStringList languageList = KGlobal::locale()->languagesTwoAlpha();
    TQStringList::Iterator it = languageList.tqfind( TQString::tqfromLatin1("C") );
    if ( it != languageList.end() )
    {
        if ( languageList.tqcontains( english ) > 0 )
          languageList.remove( it );
        else
          (*it) = english;
    }
    if ( !languageList.tqcontains( english ) )
       languageList.append( english );

    d->language = languageList.join( ", " );

    d->charsets = TQString::tqfromLatin1(TQTextCodec::codecForLocale()->mimeName()).lower();
    KProtocolManager::reparseConfiguration();
}

void SessionData::slotAuthData( const TQCString& key, const TQCString& gkey,
                                 bool keep )
{
  if (!authData)
    authData = new AuthDataList;
  authData->addData( new SessionData::AuthData(key, gkey, keep) );
}

void SessionData::slotDelAuthData( const TQCString& gkey )
{
  if (!authData)
     return;
  authData->removeData( gkey );
}

void SessionData::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

}
