/*
    This file is part of libtderesources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef TDERESOURCES_FACTORY_H
#define TDERESOURCES_FACTORY_H

#include <tqdict.h>
#include <tqstring.h>

#include <tdeconfig.h>
#include <klibloader.h>
#include <ktrader.h>

#include "resource.h"
#include "configwidget.h"

namespace KRES {

/**
 * Class for loading resource plugins.
 * Use this class if you need resources with special
 * settings, otherwise use KRES::Manager::createResource()
 * to get resources with the default settings.
 *
 * Example:
 *
 * \code
 * TDEABC::Factory *factory = TDEABC::Factory::self( "contact" );
 *
 * // to allow a transparent configuration of resources, we have
 * // to use a tdeconfig object.
 * TDEConfig config( "tst" );
 * config.writePathEntry( "FileName", "/home/foobar/test.vcf" );// resource dependend
 * config.writeEntry( "FileFormat", "vcard" );                  // resource dependend
 *
 * TDEABC::Resource *res = factory->resource( "file", &config );
 *
 * // do something with resource
 *
 * \endcode
 */
class TDERESOURCES_EXPORT Factory
{
  public:

    /**
     * Returns the global resource factory.
     */
    static Factory *self( const TQString& resourceFamily );

    ~Factory();

    /**
     * Returns the config widget for the given resource type,
     * or a null pointer if resource type doesn't exist.
     *
     * @param type   The type of the resource, returned by typeNames()
     * @param parent The parent widget
     */
    ConfigWidget *configWidget( const TQString& type, TQWidget *parent = 0 );

    /**
     * Returns a pointer to a resource object or a null pointer
     * if resource type doesn't exist.
     *
     * @param type   The type of the resource, returned by typeNames()
     * @param config The config object where the resource get it settings from,
     *               or 0 if a resource with default values should be created.
     */
    Resource *resource( const TQString& type, const TDEConfig *config );

    /**
     * Returns a list of all available resource types.
     */
    TQStringList typeNames() const;

    /**
     * Returns the name for a special type.
     */
    TQString typeName( const TQString &type ) const;

    /**
     * Returns the description for a special type.
     */
    TQString typeDescription( const TQString &type ) const;

  protected:
    Factory( const TQString& resourceFamily );

  private:
    static TQDict<Factory> *mSelves;

    TQString mResourceFamily;
    TQMap<TQString, KService::Ptr> mTypeMap;
};

}
#endif
