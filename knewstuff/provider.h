/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KNEWSTUFF_PROVIDER_H
#define KNEWSTUFF_PROVIDER_H

#include <tqcstring.h>
#include <tqdom.h>
#include <tqobject.h>
#include <tqptrlist.h>
#include <tqstring.h>

#include <kurl.h>

namespace KIO { class Job; }

namespace KNS {

/**
 * @short KNewStuff provider container.
 *
 * This class provides accessors for the provider object.
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class KDE_EXPORT Provider
{
  public:
    typedef TQPtrList<Provider> List;

    /**
     * Constructor.
     */
    Provider();

    /**
     * Constructor with XML feed.
     */
    Provider( const TQDomElement & );

    /**
     * Destructor.
     */
    ~Provider();

    /**
     * Sets the common name of the provider.
     */
    void setName( const TQString & );

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    TQString name() const;

    /**
     * Sets the download URL.
     */
    void setDownloadUrl( const KURL & );

    /**
     * Retrieves the download URL.
     *
     * @return download URL
     */
    KURL downloadUrl() const;

    /**
     * Variant to retrieve 'tagged' download URLs.
     * Variant can be one of 'score', 'downloads', 'latest'.
     *
     * @return download specific URL
     */
    KURL downloadUrlVariant( TQString variant ) const;

    /**
     * Sets the upload URL.
     */
    void setUploadUrl( const KURL & );

    /**
     * Retrieves the upload URL.
     *
     * @return upload URL
     */
    KURL uploadUrl() const;

    /**
     * Sets the URL where a user is led if the provider does not support
     * uploads.
     *
     * @see setNoUpload
     */
    void setNoUploadUrl( const KURL & );

    /**
     * Retrieves the URL where a user is led if the provider does not
     * support uploads.
     *
     * @return website URL
     */
    KURL noUploadUrl() const;

    /**
     * Indicate whether provider supports uploads.
     */
    void setNoUpload( bool );

    /**
     * Query whether provider supports uploads.
     *
     * @return upload support status
     */
    bool noUpload() const;

    /**
     * Sets the URL for an icon for this provider.
     * The icon should be in 32x32 format. If not set, the default icon
     * of KDialogBase is used.
     */
    void setIcon( const KURL & );

    /**
     * Retrieves the icon URL for this provider.
     *
     * @return icon URL
     */
    KURL icon() const;

  protected:
    void parseDomElement( const TQDomElement & );

    TQDomElement createDomElement( TQDomDocument &, TQDomElement &parent );

  private:
    TQString mName;
    KURL mDownloadUrl;
    KURL mUploadUrl;
    KURL mNoUploadUrl;
    KURL mIcon;
    bool mNoUpload;
};

/**
 * KNewStuff provider loader.
 * This class sets up a list of all possible providers by querying
 * the main provider database for this specific application.
 * It should probably not be used directly by the application.
 */
class KDE_EXPORT ProviderLoader : public TQObject
{
    Q_OBJECT
  public:
    /**
     * Constructor.
     *
     * @param tqparentWidget the parent widget
     */
    ProviderLoader( TQWidget *tqparentWidget );

    /**
     * Starts asynchronously loading the list of providers of the
     * specified type.
     *
     * @param type data type such as 'kdesktop/wallpaper'.
     * @param providerList the URl to the list of providers; if empty
     *    we first try the ProvidersUrl from KGlobal::config, then we
     *    fall back to a hardcoded value.
     */
    void load( const TQString &type, const TQString &providerList = TQString::null );

  signals:
    /**
     * Indicates that the list of providers has been successfully loaded.
     */
    void providersLoaded( Provider::List * );

  protected slots:
    void slotJobData( KIO::Job *, const TQByteArray & );
    void slotJobResult( KIO::Job * );

  private:
    TQWidget *mParentWidget;

    TQString mJobData;

    Provider::List mProviders;
};

}

#endif
