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
#ifndef __kfilterdev_h
#define __kfilterdev_h

#include <tqiodevice.h>
#include <tqstring.h>
#include <tdelibs_export.h>

class TQFile;
class KFilterBase;

/**
 * A class for reading and writing compressed data onto a device
 * (e.g. file, but other usages are possible, like a buffer or a socket).
 *
 * To simply read/write compressed files, see deviceForFile.
 *
 * @author David Faure <faure@kde.org>
 */
class KIO_EXPORT KFilterDev : public TQIODevice
{
public:
    /**
     * Constructs a KFilterDev for a given filter (e.g. gzip, bzip2 etc.).
     * @param filter the KFilterBase to use
     * @param autoDeleteFilterBase when true this object will become the
     * owner of @p filter.
     */
    KFilterDev( KFilterBase * filter, bool autoDeleteFilterBase = false );
    /**
     * Destructs the KFilterDev.
     * Calls close() if the filter device is still open.
     */
    virtual ~KFilterDev();

    /**
     * Open for reading or writing.
     * If the KFilterBase's device is not opened, it will be opened.
     */
#ifdef qdoc
#else
    virtual bool open( TQ_OpenMode mode );
#endif
    /**
     * Close after reading or writing.
     * If the KFilterBase's device was opened by open(), it will be closed.
     */
    virtual void close();
    virtual void flush();

    /**
     * For writing gzip compressed files only:
     * set the name of the original file, to be used in the gzip header.
     * @param fileName the name of the original file
     */
    void setOrigFileName( const TQCString & fileName );

    /**
     * Call this let this device skip the gzip headers when reading/writing.
     * This way KFilterDev (with gzip filter) can be used as a direct wrapper
     * around zlib - this is used by KZip.
     * @since 3.1
     */
    void setSkipHeaders();

    // Not implemented
#ifdef qdoc
#else
#ifdef USE_QT4
    virtual qint64 size() const;
#else // USE_QT4
    virtual TQIODevice::Offset size() const;
#endif // USE_QT4
#endif

    virtual TQIODevice::Offset at() const;
    /**
     * That one can be quite slow, when going back. Use with care.
     */
    virtual bool at( TQIODevice::Offset );

    virtual bool atEnd() const;

#ifdef qdoc
#else
    virtual TQT_TQIO_LONG tqreadBlock( char *data, TQT_TQIO_ULONG maxlen );
    virtual TQT_TQIO_LONG tqwriteBlock( const char *data, TQT_TQIO_ULONG len );
#endif
    //int readLine( char *data, uint maxlen );

    virtual int getch();
    virtual int putch( int );
    virtual int ungetch( int );

#ifdef KDE_NO_COMPAT
private:
#endif
    /**
     * Call this to create the appropriate filter device for @p base
     * working on @p file . The returned TQIODevice has to be deleted
     * after using.
     * @deprecated. Use deviceForFile instead.
     * To be removed in KDE 3.0
     */
    static TQIODevice* createFilterDevice(KFilterBase* base, TQFile* file) KDE_DEPRECATED;
public:

    /**
     * Creates an i/o device that is able to read from @p fileName,
     * whether it's compressed or not. Available compression filters
     * (gzip/bzip2 etc.) will automatically be used.
     *
     * The compression filter to be used is determined from the @p fileName
     * if @p mimetype is empty. Pass "application/x-gzip" or "application/x-bzip2"
     * to force the corresponding decompression filter, if available.
     *
     * Warning: application/x-bzip2 may not be available.
     * In that case a TQFile opened on the compressed data will be returned !
     * Use KFilterBase::findFilterByMimeType and code similar to what
     * deviceForFile is doing, to better control what's happening.
     *
     * The returned TQIODevice has to be deleted after using.
     *
     * @param fileName the name of the file to filter
     * @param mimetype the mime type of the file to filter, or TQString::null if unknown
     * @param forceFilter if true, the function will either find a compression filter, or return 0.
     *                    If false, it will always return a TQIODevice. If no
     *                    filter is available it will return a simple TQFile.
     *                    This can be useful if the file is usable without a filter.
     * @return if a filter has been found, the TQIODevice for the filter. If the
     *         filter does not exist, the return value depends on @p forceFilter.
     *         The returned TQIODevice has to be deleted after using.
     */
    static TQIODevice * deviceForFile( const TQString & fileName, const TQString & mimetype = TQString::null,
                                      bool forceFilter = false );

    /**
     * Creates an i/o device that is able to read from the TQIODevice @p inDevice,
     * whether the data is compressed or not. Available compression filters
     * (gzip/bzip2 etc.) will automatically be used.
     *
     * The compression filter to be used is determined @p mimetype .
     * Pass "application/x-gzip" or "application/x-bzip2"
     * to use the corresponding decompression filter.
     *
     * Warning: application/x-bzip2 may not be available.
     * In that case 0 will be returned !
     *
     * The returned TQIODevice has to be deleted after using.
     * @param inDevice input device, becomes owned by this device! Automatically deleted!
     * @param mimetype the mime type for the filter
     * @return a TQIODevice that filters the original stream. Must be deleted after
     *         using
     */
    static TQIODevice * device( TQIODevice* inDevice, const TQString & mimetype);
    // BIC: merge with device() method below, using default value for autoDeleteInDevice

    /**
     * Creates an i/o device that is able to read from the TQIODevice @p inDevice,
     * whether the data is compressed or not. Available compression filters
     * (gzip/bzip2 etc.) will automatically be used.
     *
     * The compression filter to be used is determined @p mimetype .
     * Pass "application/x-gzip" or "application/x-bzip2"
     * to use the corresponding decompression filter.
     *
     * Warning: application/x-bzip2 may not be available.
     * In that case 0 will be returned !
     *
     * The returned TQIODevice has to be deleted after using.
     * @param inDevice input device. Won't be deleted if @p autoDeleteInDevice = false
     * @param mimetype the mime type for the filter
     * @param autoDeleteInDevice if true, @p inDevice will be deleted automatically
     * @return a TQIODevice that filters the original stream. Must be deleted after
     *         using
     * @since 3.1
     */
    static TQIODevice * device( TQIODevice* inDevice, const TQString & mimetype, bool autoDeleteInDevice );

private:
    KFilterBase *filter;
    class KFilterDevPrivate;
    KFilterDevPrivate * d;
};


#endif

