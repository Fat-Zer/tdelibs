//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef __kbookmarkimporter_ns_h
#define __kbookmarkimporter_ns_h

#include <tqdom.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <ksimpleconfig.h>
#include <kdemacros.h>

#include "kbookmarkimporter.h"
#include "kbookmarkexporter.h"

/**
 * A class for importing NS bookmarks
 * @deprecated
 */
class KIO_EXPORT_DEPRECATED KNSBookmarkImporter : public TQObject
{
    Q_OBJECT
public:
    KNSBookmarkImporter( const TQString & fileName ) : m_fileName(fileName) {}
    ~KNSBookmarkImporter() {}

    // for compat reasons only
    void parseNSBookmarks() { parseNSBookmarks(false); }
    // go for it. Set utf8 to true for Mozilla, false for Netscape.
    void parseNSBookmarks( bool utf8 );

    static TQString netscapeBookmarksFile( bool forSaving=false );
    static TQString mozillaBookmarksFile( bool forSaving=false );

signals:
    void newBookmark( const TQString & text, const TQCString & url, const TQString & additionalInfo );
    void newFolder( const TQString & text, bool open, const TQString & additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    TQString m_fileName;
};

/**
 * A class for importing NS bookmarks
 * utf8 defaults to off
 * @since 3.2
 */
class KIO_EXPORT KNSBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KNSBookmarkImporterImpl() : m_utf8(false) { }
    void setUtf8(bool utf8) { m_utf8 = utf8; }
    virtual void parse();
    virtual TQString tqfindDefaultLocation(bool forSaving = false) const;
private:
    bool m_utf8;
    class KNSBookmarkImporterImplPrivate *d;
};

/**
 * A class for importing Mozilla bookmarks
 * utf8 defaults to on
 * @since 3.2
 */
class KIO_EXPORT KMozillaBookmarkImporterImpl : public KNSBookmarkImporterImpl
{
public:
    KMozillaBookmarkImporterImpl() { setUtf8(true); }
private:
    class KMozillaBookmarkImporterImplPrivate *d;
};

/**
 * A class that exports all the current bookmarks to Netscape/Mozilla bookmarks
 * Warning, it overwrites the existing bookmarks.html file !
 * @deprecated
 */
class KIO_EXPORT_DEPRECATED KNSBookmarkExporter
{
public:
    KNSBookmarkExporter(KBookmarkManager* mgr, const TQString & fileName)
      : m_fileName(fileName), m_pManager(mgr) { }
    ~KNSBookmarkExporter() {}

    void write() { write(false); } // deprecated
    void write( bool utf8 );

protected:
    void writeFolder(TQTextStream &stream, KBookmarkGroup parent);
    TQString m_fileName;
    KBookmarkManager* m_pManager;
};

/**
 * @since 3.2
 */
class KIO_EXPORT KNSBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KNSBookmarkExporterImpl(KBookmarkManager* mgr, const TQString & fileName)
      : KBookmarkExporterBase(mgr, fileName) 
    { ; }
    virtual ~KNSBookmarkExporterImpl() {}
    virtual void write(KBookmarkGroup);
    void setUtf8(bool);
protected:
    TQString folderAsString(KBookmarkGroup) const;
private:
    bool m_utf8;
    class KNSBookmarkExporterImplPrivate *d;
};

#endif
