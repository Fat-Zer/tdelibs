/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kservice.h"
#include "ksycoca.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"
#include "kuserprofile.h"
#include <assert.h>
#include <kdebug.h>
#include <kdesktopfile.h>

template TQDataStream& operator>> <TQString, TQVariant>(TQDataStream&, TQMap<TQString, TQVariant>&);
template TQDataStream& operator<< <TQString, TQVariant>(TQDataStream&, const TQMap<TQString, TQVariant>&);

class KServiceType::KServiceTypePrivate
{
public:
  KServiceTypePrivate() : parentTypeLoaded(false) { }

  KServiceType::Ptr parentType;
  KService::List services;
  bool parentTypeLoaded;
};

KServiceType::KServiceType( const TQString & _fullpath)
 : KSycocaEntry(_fullpath), d(0)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KServiceType::KServiceType( KDesktopFile *config )
 : KSycocaEntry(config->fileName()), d(0)
{
  init(config);
}

void
KServiceType::init( KDesktopFile *config)
{
  // Is it a mimetype ?
  m_strName = config->readEntry( "MimeType" );

  // Or is it a servicetype ?
  if ( m_strName.isEmpty() )
  {
    m_strName = config->readEntry( "X-KDE-ServiceType" );
  }

  m_strComment = config->readComment();
  m_bDeleted = config->readBoolEntry( "Hidden", false );
  m_strIcon = config->readIcon();

  // We store this as property to preserve BC, we can't change that
  // because KSycoca needs to remain BC between KDE 2.x and KDE 3.x
  TQString sDerived = config->readEntry( "X-KDE-Derived" );
  m_bDerived = !sDerived.isEmpty();
  if ( m_bDerived )
    m_mapProps.insert( "X-KDE-Derived", sDerived );

  TQStringList tmpList = config->groupList();
  TQStringList::Iterator gIt = tmpList.begin();

  for( ; gIt != tmpList.end(); ++gIt )
  {
    if ( (*gIt).tqfind( "Property::" ) == 0 )
    {
      config->setGroup( *gIt );
      TQVariant v = config->readPropertyEntry( "Value",
                   TQVariant::nameToType( config->readEntry( "Type" ).ascii() ) );
      if ( v.isValid() )
          m_mapProps.insert( (*gIt).mid( 10 ), v );
    }
  }

  gIt = tmpList.begin();
  for( ; gIt != tmpList.end(); ++gIt )
  {
    if( (*gIt).tqfind( "PropertyDef::" ) == 0 )
    {
      config->setGroup( *gIt );
      m_mapPropDefs.insert( (*gIt).mid( 13 ),
			    TQVariant::nameToType( config->readEntry( "Type" ).ascii() ) );
    }
  }

  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( const TQString & _fullpath, const TQString& _type,
                            const TQString& _icon, const TQString& _comment )
 : KSycocaEntry(_fullpath), d(0)
{
  m_strName = _type;
  m_strIcon = _icon;
  m_strComment = _comment;
  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( TQDataStream& _str, int offset )
 : KSycocaEntry( _str, offset ), d(0)
{
  load( _str);
}

void
KServiceType::load( TQDataStream& _str )
{
  TQ_INT8 b;
  _str >> m_strName >> m_strIcon >> m_strComment >> m_mapProps >> m_mapPropDefs
       >> b;
  m_bValid = b;
  m_bDerived = m_mapProps.tqcontains("X-KDE-Derived");
}

void
KServiceType::save( TQDataStream& _str )
{
  KSycocaEntry::save( _str );
  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in ksycoca.h
  _str << m_strName << m_strIcon << m_strComment << m_mapProps << m_mapPropDefs
       << (TQ_INT8)m_bValid;
}

KServiceType::~KServiceType()
{
  delete d;
}

TQString KServiceType::parentServiceType() const
{
  TQVariant v = property("X-KDE-Derived");
  return v.toString();
}

bool KServiceType::inherits( const TQString& servTypeName ) const
{
  if ( name() == servTypeName )
      return true;
  TQString st = parentServiceType();
  while ( !st.isEmpty() )
  {
      KServiceType::Ptr ptr = KServiceType::serviceType( st );
      if (!ptr) return false; //error
      if ( ptr->name() == servTypeName )
          return true;
      st = ptr->parentServiceType();
  }
  return false;
}

QVariant
KServiceType::property( const TQString& _name ) const
{
  TQVariant v;

  if ( _name == "Name" )
    v = TQVariant( m_strName );
  else if ( _name == "Icon" )
    v = TQVariant( m_strIcon );
  else if ( _name == "Comment" )
    v = TQVariant( m_strComment );
  else {
    TQMap<TQString,TQVariant>::ConstIterator it = m_mapProps.tqfind( _name );
    if ( it != m_mapProps.end() )
      v = it.data();
  }

  return v;
}

QStringList
KServiceType::propertyNames() const
{
  TQStringList res;

  TQMap<TQString,TQVariant>::ConstIterator it = m_mapProps.begin();
  for( ; it != m_mapProps.end(); ++it )
    res.append( it.key() );

  res.append( "Name" );
  res.append( "Comment" );
  res.append( "Icon" );

  return res;
}

TQVariant::Type
KServiceType::propertyDef( const TQString& _name ) const
{
  TQMap<TQString,TQVariant::Type>::ConstIterator it = m_mapPropDefs.tqfind( _name );
  if ( it == m_mapPropDefs.end() )
    return TQVariant::Invalid;
  return it.data();
}

QStringList
KServiceType::propertyDefNames() const
{
  TQStringList l;

  TQMap<TQString,TQVariant::Type>::ConstIterator it = m_mapPropDefs.begin();
  for( ; it != m_mapPropDefs.end(); ++it )
    l.append( it.key() );

  return l;
}

KServiceType::Ptr KServiceType::serviceType( const TQString& _name )
{
  KServiceType * p = KServiceTypeFactory::self()->tqfindServiceTypeByName( _name );
  return KServiceType::Ptr( p );
}

static void addUnique(KService::List &lst, TQDict<KService> &dict, const KService::List &newLst, bool lowPrio)
{
  TQValueListConstIterator<KService::Ptr> it = newLst.begin();
  for( ; it != newLst.end(); ++it )
  {
     KService *service = static_cast<KService*>(*it);
     if (dict.tqfind(service->desktopEntryPath()))
        continue;
     dict.insert(service->desktopEntryPath(), service);
     lst.append(service);
     if (lowPrio)
        service->setInitialPreference( 0 );
  }
}

KService::List KServiceType::offers( const TQString& _servicetype )
{
  TQDict<KService> dict(53);
  KService::List lst;

  // Services associated directly with this servicetype (the normal case)
  KServiceType::Ptr serv = KServiceTypeFactory::self()->tqfindServiceTypeByName( _servicetype );
  if ( serv )
    addUnique(lst, dict, KServiceFactory::self()->offers( serv->offset() ), false);
  else
    kdWarning(7009) << "KServiceType::offers : servicetype " << _servicetype << " not found" << endl;

  // Find services associated with any mimetype parents. e.g. text/x-java -> text/plain    
  KMimeType::Ptr mime = dynamic_cast<KMimeType*>(static_cast<KServiceType *>(serv));
  bool isAMimeType = (mime != 0);
  if (mime)
  {
     while(true)
     {
        TQString parent = mime->parentMimeType();
        if (parent.isEmpty())
           break;
        mime = dynamic_cast<KMimeType *>(KServiceTypeFactory::self()->tqfindServiceTypeByName( parent ));
        if (!mime)
           break;
        
        addUnique(lst, dict, KServiceFactory::self()->offers( mime->offset() ), false);
     }
  }
  serv = mime = 0;

  //TQValueListIterator<KService::Ptr> it = lst.begin();
  //for( ; it != lst.end(); ++it )
  //    kdDebug() << (*it).data() << " " << (*it)->name() << endl;

  // Support for all/* is deactivated by KServiceTypeProfile::configurationMode()
  // (and makes no sense when querying for an "all" servicetype itself
  // nor for non-mimetypes service types)
  if ( !KServiceTypeProfile::configurationMode()
       && isAMimeType
       && _servicetype.left(4) != "all/" )
  {
    // Support for services associated with "all"
    KServiceType * servAll = KServiceTypeFactory::self()->tqfindServiceTypeByName( "all/all" );
    if ( servAll )
    {
        addUnique(lst, dict, KServiceFactory::self()->offers( servAll->offset() ), true);
    }
    else
      kdWarning(7009) << "KServiceType::offers : servicetype all/all not found" << endl;
    delete servAll;

    // Support for services associated with "allfiles"
    if ( _servicetype != "inode/directory" && _servicetype != "inode/directory-locked" )
    {
      KServiceType * servAllFiles = KServiceTypeFactory::self()->tqfindServiceTypeByName( "all/allfiles" );
      if ( servAllFiles )
      {
        addUnique(lst, dict, KServiceFactory::self()->offers( servAllFiles->offset() ), true);
      }
      else
        kdWarning(7009) << "KServiceType::offers : servicetype all/allfiles not found" << endl;
      delete servAllFiles;
    }
  }

  return lst;
}

KServiceType::List KServiceType::allServiceTypes()
{
  return KServiceTypeFactory::self()->allServiceTypes();
}

KServiceType::Ptr KServiceType::parentType()
{
  if (d && d->parentTypeLoaded)
     return d->parentType;
  
  if (!d)
     d = new KServiceTypePrivate;
     
  TQString parentSt = parentServiceType();
  if (!parentSt.isEmpty())
  {
    d->parentType = KServiceTypeFactory::self()->tqfindServiceTypeByName( parentSt );
    if (!d->parentType)
      kdWarning(7009) << "'" << desktopEntryPath() << "' specifies undefined mimetype/servicetype '"<< parentSt << "'" << endl;
  }
  
  d->parentTypeLoaded = true;

  return d->parentType;
}

void KServiceType::addService(KService::Ptr service)
{
  if (!d)
     d = new KServiceTypePrivate;
  
  if (d->services.count() && d->services.last() == service)
     return;
     
  d->services.append(service);
}

KService::List KServiceType::services()
{
  if (d)
     return d->services;

  return KService::List();
}

void KServiceType::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }
