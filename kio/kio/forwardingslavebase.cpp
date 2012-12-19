/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

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

#include <kdebug.h>
#include <kio/job.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>

#include <tqapplication.h>
#include <tqeventloop.h>

#include "forwardingslavebase.h"

namespace KIO
{

class ForwardingSlaveBasePrivate
{
};

ForwardingSlaveBase::ForwardingSlaveBase(const TQCString &protocol,
                                         const TQCString &poolSocket,
                                         const TQCString &appSocket)
    : TQObject(), SlaveBase(protocol, poolSocket, appSocket)
{
}

ForwardingSlaveBase::~ForwardingSlaveBase()
{
}

bool ForwardingSlaveBase::internalRewriteURL(const KURL &url, KURL &newURL)
{
    bool result = true;

    if ( url.protocol().ascii()==mProtocol )
    {
        result = rewriteURL(url, newURL);
    }
    else
    {
        newURL = url;
    }

    m_processedURL = newURL;
    m_requestedURL = url;
    return result;
}

void ForwardingSlaveBase::prepareUDSEntry(KIO::UDSEntry &entry,
                                          bool listing) const
{
    kdDebug() << "ForwardingSlaveBase::prepareUDSEntry: listing=="
              << listing << endl;

    bool url_found = false;
    TQString name;
    KURL url;

    KIO::UDSEntry::iterator it = entry.begin();
    KIO::UDSEntry::iterator end = entry.end();

    for(; it!=end; ++it)
    {
        KURL new_url = m_requestedURL;

        switch( (*it).m_uds )
        {
        case KIO::UDS_NAME:
            name = (*it).m_str;
            kdDebug() << "Name = " << name << endl;
	    break;
        case KIO::UDS_URL:
            url_found = true;
            url = (*it).m_str;
	    if (listing)
            {
                new_url.addPath(url.fileName());
            }
            (*it).m_str = new_url.url();
            kdDebug() << "URL = " << url << endl;
            kdDebug() << "New URL = " << (*it).m_str << endl;
            break;
        }
    }

    if ( m_processedURL.isLocalFile() )
    {
        KURL new_url = m_processedURL;
        if (listing)
        {
            new_url.addPath( name );
        }

        KIO::UDSAtom atom;
        atom.m_uds = KIO::UDS_LOCAL_PATH;
        atom.m_long = 0;
        atom.m_str = new_url.path();
        entry.append(atom);
    }
}

void ForwardingSlaveBase::get(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::get: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::TransferJob *job = KIO::get(new_url, false, false);
        connectTransferJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::put(const KURL &url, int permissions,
                              bool overwrite, bool resume )
{
    kdDebug() << "ForwardingSlaveBase::put: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::TransferJob *job = KIO::put(new_url, permissions, overwrite,
                                         resume, false);
        connectTransferJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::stat(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::stat: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::SimpleJob *job = KIO::stat(new_url, false);
        connectSimpleJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::mimetype(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::mimetype: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::TransferJob *job = KIO::mimetype(new_url, false);
        connectTransferJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::listDir(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::listDir: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::ListJob *job = KIO::listDir(new_url, false);
        connectListJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::mkdir(const KURL &url, int permissions)
{
    kdDebug() << "ForwardingSlaveBase::mkdir: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::SimpleJob *job = KIO::mkdir(new_url, permissions);
        connectSimpleJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::rename(const KURL &src, const KURL &dest,
                                 bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::rename: " << src << ", " << dest << endl;

    KURL new_src, new_dest;
    if ( internalRewriteURL(src, new_src) && internalRewriteURL(dest, new_dest) )
    {
        KIO::Job *job = KIO::rename(new_src, new_dest, overwrite);
        connectJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::symlink(const TQString &target, const KURL &dest,
                                  bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::symlink: " << target << ", " << dest << endl;

    KURL new_dest;
    if ( internalRewriteURL(dest, new_dest) )
    {
        KIO::SimpleJob *job = KIO::symlink(target, new_dest, overwrite, false);
        connectSimpleJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::chmod(const KURL &url, int permissions)
{
    kdDebug() << "ForwardingSlaveBase::chmod: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::SimpleJob *job = KIO::chmod(new_url, permissions);
        connectSimpleJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::copy(const KURL &src, const KURL &dest,
                               int permissions, bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::copy: " << src << ", " << dest << endl;

    KURL new_src, new_dest;
    if ( internalRewriteURL(src, new_src) && internalRewriteURL(dest, new_dest) )
    {
        KIO::Job *job = KIO::file_copy(new_src, new_dest, permissions,
                                       overwrite, false);
        connectJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::del(const KURL &url, bool isfile)
{
    kdDebug() << "ForwardingSlaveBase::del: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        if (isfile)
        {
            KIO::DeleteJob *job = KIO::del(new_url, false, false);
            connectJob(job);
        }
        else
        {
            KIO::SimpleJob *job = KIO::rmdir(new_url);
            connectSimpleJob(job);
        }

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::localURL(const KURL& remoteURL)
{
    kdDebug() << "ForwardingSlaveBase::localURL: " << remoteURL << endl;

    KURL new_url;
    if ( internalRewriteURL(remoteURL, new_url) )
    {
        KIO::LocalURLJob *job = KIO::localURL(new_url);
        connectLocalURLJob(job);

        tqApp->eventLoop()->enterLoop();
    }
    else
    {
        // Let the slave base emit the required signals
        SlaveBase::localURL(remoteURL);
    }
}

//////////////////////////////////////////////////////////////////////////////

void ForwardingSlaveBase::connectJob(KIO::Job *job)
{
    // We will forward the warning message, no need to let the job
    // display it itself
    job->setInteractive(false);

    // Forward metadata (e.g. modification time for put())
    job->setMetaData( allMetaData() );
#if 0 // debug code
    kdDebug() << k_funcinfo << "transferring metadata:" << endl;
    const MetaData md = allMetaData();
    for ( MetaData::const_iterator it = md.begin(); it != md.end(); ++it )
        kdDebug() << it.key() << " = " << it.data() << endl;
#endif

    connect( job, TQT_SIGNAL( result(KIO::Job *) ),
             this, TQT_SLOT( slotResult(KIO::Job *) ) );
    connect( job, TQT_SIGNAL( warning(KIO::Job *, const TQString &) ),
             this, TQT_SLOT( slotWarning(KIO::Job *, const TQString &) ) );
    connect( job, TQT_SIGNAL( infoMessage(KIO::Job *, const TQString &) ),
             this, TQT_SLOT( slotInfoMessage(KIO::Job *, const TQString &) ) );
    connect( job, TQT_SIGNAL( totalSize(KIO::Job *, KIO::filesize_t) ),
             this, TQT_SLOT( slotTotalSize(KIO::Job *, KIO::filesize_t) ) );
    connect( job, TQT_SIGNAL( processedSize(KIO::Job *, KIO::filesize_t) ),
             this, TQT_SLOT( slotProcessedSize(KIO::Job *, KIO::filesize_t) ) );
    connect( job, TQT_SIGNAL( speed(KIO::Job *, unsigned long) ),
             this, TQT_SLOT( slotSpeed(KIO::Job *, unsigned long) ) );
}

void ForwardingSlaveBase::connectSimpleJob(KIO::SimpleJob *job)
{
    connectJob(job);
    connect( job, TQT_SIGNAL( redirection(KIO::Job *, const KURL &) ),
             this, TQT_SLOT( slotRedirection(KIO::Job *, const KURL &) ) );
}

void ForwardingSlaveBase::connectListJob(KIO::ListJob *job)
{
    connectSimpleJob(job);
    connect( job, TQT_SIGNAL( entries(KIO::Job *, const KIO::UDSEntryList &) ),
             this, TQT_SLOT( slotEntries(KIO::Job *, const KIO::UDSEntryList &) ) );
}

void ForwardingSlaveBase::connectTransferJob(KIO::TransferJob *job)
{
    connectSimpleJob(job);
    connect( job, TQT_SIGNAL( data(KIO::Job *, const TQByteArray &) ),
             this, TQT_SLOT( slotData(KIO::Job *, const TQByteArray &) ) );
    connect( job, TQT_SIGNAL( dataReq(KIO::Job *, TQByteArray &) ),
             this, TQT_SLOT( slotDataReq(KIO::Job *, TQByteArray &) ) );
    connect( job, TQT_SIGNAL( mimetype(KIO::Job *, const TQString &) ),
             this, TQT_SLOT( slotMimetype(KIO::Job *, const TQString &) ) );
    connect( job, TQT_SIGNAL( canResume(KIO::Job *, KIO::filesize_t) ),
             this, TQT_SLOT( slotCanResume(KIO::Job *, KIO::filesize_t) ) );
}

void ForwardingSlaveBase::connectLocalURLJob(KIO::LocalURLJob *job)
{
    connectJob(job);
    connect( job, TQT_SIGNAL( localURL(KIO::Job *, const KURL&, bool) ),
             this, TQT_SLOT( slotLocalURL(KIO::Job *, const KURL&, bool) ) );
}

//////////////////////////////////////////////////////////////////////////////

void ForwardingSlaveBase::slotResult(KIO::Job *job)
{
    if ( job->error() != 0)
    {
        error( job->error(), job->errorText() );
    }
    else
    {
        KIO::StatJob *stat_job = dynamic_cast<KIO::StatJob *>(job);
        if ( stat_job!=0L )
        {
            KIO::UDSEntry entry = stat_job->statResult();
	    prepareUDSEntry(entry);
            statEntry( entry );
        }
        finished();
    }

    tqApp->eventLoop()->exitLoop();
}

void ForwardingSlaveBase::slotWarning(KIO::Job* /*job*/, const TQString &msg)
{
    warning(msg);
}

void ForwardingSlaveBase::slotInfoMessage(KIO::Job* /*job*/, const TQString &msg)
{
    infoMessage(msg);
}

void ForwardingSlaveBase::slotTotalSize(KIO::Job* /*job*/, KIO::filesize_t size)
{
    totalSize(size);
}

void ForwardingSlaveBase::slotProcessedSize(KIO::Job* /*job*/, KIO::filesize_t size)
{
    processedSize(size);
}

void ForwardingSlaveBase::slotSpeed(KIO::Job* /*job*/, unsigned long bytesPerSecond)
{
    speed(bytesPerSecond);
}

void ForwardingSlaveBase::slotRedirection(KIO::Job *job, const KURL &url)
{
    redirection(url);

    // We've been redirected stop everything.
    job->kill( true );
    finished();

    tqApp->eventLoop()->exitLoop();
}

void ForwardingSlaveBase::slotEntries(KIO::Job* /*job*/,
                                      const KIO::UDSEntryList &entries)
{
    KIO::UDSEntryList final_entries = entries;

    KIO::UDSEntryList::iterator it = final_entries.begin();
    KIO::UDSEntryList::iterator end = final_entries.end();

    for(; it!=end; ++it)
    {
        prepareUDSEntry(*it, true);
    }

    listEntries( final_entries );
}

void ForwardingSlaveBase::slotData(KIO::Job* /*job*/, const TQByteArray &d)
{
    data(d);
}

void ForwardingSlaveBase::slotDataReq(KIO::Job* /*job*/, TQByteArray &data)
{
    dataReq();
    readData(data);
}

void ForwardingSlaveBase::slotMimetype (KIO::Job* /*job*/, const TQString &type)
{
    mimeType(type);
}

void ForwardingSlaveBase::slotCanResume (KIO::Job* /*job*/, KIO::filesize_t offset)
{
    canResume(offset);
}

void ForwardingSlaveBase::slotLocalURL(KIO::Job *, const KURL& url, bool)
{
    SlaveBase::localURL(url);
}

}

#include "forwardingslavebase.moc"

