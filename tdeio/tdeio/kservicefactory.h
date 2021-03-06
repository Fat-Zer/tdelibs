/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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

#ifndef __kservicefactory_h__
#define __kservicefactory_h__

#include <tqstringlist.h>

#include "kservice.h"
#include "tdesycocafactory.h"
#include <assert.h>

class KSycoca;
class KSycocaDict;

/**
 * @internal
 * A sycoca factory for services (e.g. applications)
 * It loads the services from parsing directories (e.g. applnk/)
 * but can also create service from data streams or single config files
 */
class TDEIO_EXPORT KServiceFactory : public KSycocaFactory
{
  K_SYCOCAFACTORY( KST_KServiceFactory )
public:
  /**
   * Create factory
   */
  KServiceFactory();
  virtual ~KServiceFactory();
  
  /**
   * Construct a KService from a config file.
   */
  virtual KSycocaEntry *createEntry(const TQString &, const char *) 
    { assert(0); return 0; }

  /**
   * Find a service (by name, e.g. "Terminal")
   */
  KService * findServiceByName( const TQString &_name );

  /**
   * Find a service (by desktop file name, e.g. "konsole")
   */
  KService * findServiceByDesktopName( const TQString &_name );

  /**
   * Find a service ( by desktop path, e.g. "System/konsole.desktop")
   */
  KService * findServiceByDesktopPath( const TQString &_name );

  /**
   * Find a service ( by menu id, e.g. "tde-konsole.desktop")
   */
  KService * findServiceByMenuId( const TQString &_menuId );

  /**
   * @return the services supporting the given service type
   */
  KService::List offers( int serviceTypeOffset );

  /**
   * @return all services. Very memory consuming, avoid using.
   */
  KService::List allServices();

  /**
   * @return all services which have a "X-TDE-Init" line.
   */
  KService::List allInitServices();

  /**
   * @return the unique service factory, creating it if necessary
   */
  static KServiceFactory * self();

protected:
  virtual KService * createEntry(int offset);
  int m_offerListOffset;
  int m_initListOffset;  
  KSycocaDict *m_nameDict;
  int m_nameDictOffset;
  KSycocaDict *m_relNameDict;
  int m_relNameDictOffset;
  KSycocaDict *m_menuIdDict;
  int m_menuIdDictOffset;

private:
  static KServiceFactory *_self;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KServiceFactoryPrivate* d;
};

#endif
