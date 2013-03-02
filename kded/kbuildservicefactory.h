/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
                 1999 Waldo Bastian <bastian@kde.org>

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

#ifndef __k_build_service_factory_h__
#define __k_build_service_factory_h__

#include <tqptrdict.h>
#include <tqstringlist.h>

#include <kservicefactory.h>
// We export the services to the service group factory!
#include <kbuildservicegroupfactory.h>

/**
 * Service factory for building tdesycoca
 * @internal
 */
class KBuildServiceFactory : public KServiceFactory
{
public:
  /**
   * Create factory
   */
  KBuildServiceFactory( KSycocaFactory *serviceTypeFactory,
                        KBuildServiceGroupFactory *serviceGroupFactory );

  virtual ~KBuildServiceFactory();

  KService *findServiceByName(const TQString &_name);

  /**
   * Construct a KService from a config file.
   */
  virtual KSycocaEntry * createEntry(const TQString &file, const char *resource);

  virtual KService * createEntry( int ) { assert(0); return 0L; }

  /**
   * Add a new entry.
   */
  void addEntry(KSycocaEntry *newEntry, const char *resource);

  /**
   * Write out service specific index files.
   */
  virtual void save(TQDataStream &str);

  /**
   * Write out header information
   *
   * Don't forget to call the parent first when you override
   * this function.
   */
  virtual void saveHeader(TQDataStream &str);

  /**
   * Returns all resource types for this service factory
   */  
  static TQStringList resourceTypes();
private:
  void saveOfferList(TQDataStream &str);
  void saveInitList(TQDataStream &str);

  TQDict<KService> m_serviceDict;
  TQPtrDict<KService> m_dupeDict;
  KSycocaFactory *m_serviceTypeFactory;
  KBuildServiceGroupFactory *m_serviceGroupFactory;
};

#endif
