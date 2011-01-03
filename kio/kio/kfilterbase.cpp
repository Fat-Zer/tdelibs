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

#include "kfilterbase.h"
#include <klibloader.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <kdebug.h>

KFilterBase::KFilterBase()
    : m_dev( 0L ), m_bAutoDel( false )
{
}

KFilterBase::~KFilterBase()
{
    if ( m_bAutoDel )
        delete m_dev;
}

void KFilterBase::setDevice( TQIODevice * dev, bool autodelete )
{
    m_dev = dev;
    m_bAutoDel = autodelete;
}

KFilterBase * KFilterBase::tqfindFilterByFileName( const TQString & fileName )
{
    KMimeType::Ptr mime = KMimeType::tqfindByPath( fileName );
    kdDebug(7005) << "KFilterBase::tqfindFilterByFileName mime=" << mime->name() << endl;
    return tqfindFilterByMimeType(mime->name());
}

KFilterBase * KFilterBase::tqfindFilterByMimeType( const TQString & mimeType )
{
    KTrader::OfferList offers = KTrader::self()->query( "KDECompressionFilter",
                                                        TQString("'") + mimeType + "' in ServiceTypes" );
    KTrader::OfferList::ConstIterator it = offers.begin();
    KTrader::OfferList::ConstIterator end = offers.end();

    kdDebug(7005) << "KFilterBase::tqfindFilterByMimeType(" << mimeType << ") got " << offers.count() << " offers" << endl;
    for (; it != end; ++it )
    {
        if ((*it)->library().isEmpty()) { continue; }
        KLibFactory *factory = KLibLoader::self()->factory((*it)->library().latin1());
        if (!factory) { continue; }
        KFilterBase *filter = static_cast<KFilterBase*>( factory->create(0, (*it)->desktopEntryName().latin1() ) );
        if ( filter )
            return filter;
    }

    if ( mimeType == "application/x-bzip2" || mimeType == "application/x-gzip" ) // #88574
        kdWarning(7005) << "KFilterBase::tqfindFilterByMimeType : no filter found for " << mimeType << endl;

    return 0L;
}

void KFilterBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kfilterbase.moc"
