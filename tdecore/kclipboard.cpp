/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>

#include "kclipboard.h"

/*
 * This class provides an automatic synchronization of the X11 Clipboard and Selection
 * buffers. There are two configuration options in the kdeglobals configuration file,
 * in the [General] section:
 * - SynchronizeClipboardAndSelection - whenever the Selection changes, Clipboard is
 *   set to the same value. This can be also enabled in Klipper.
 * - ClipboardSetSelection - whenever the Clipboard changes, Selection is set
 *   to the same value. This setting is only for die-hard fans of the old broken
 *   KDE1/2 behavior, which can potentionally lead to unexpected problems,
 *   and this setting therefore can be enabled only in the configuration file.
 *
 *  Whenever reporting any bug only remotely related to clipboard, first make
 *  sure you can reproduce it when both these two options are turned off,
 *  especially the second one.
 */

class KClipboardSynchronizer::MimeSource : public TQMimeSource
{
public:
    MimeSource( const TQMimeSource * src )
        : TQMimeSource(),
          m_formats( true ) // deep copies!
    {
        m_formats.setAutoDelete( true );
        m_data.setAutoDelete( true );

        if ( src )
        {
            TQByteArray *byteArray;
            const char *format;
            int i = 0;
            while ( (format = src->format( i++ )) )
            {
                byteArray = new TQByteArray();
                *byteArray = src->encodedData( format ).copy();
                m_data.append( byteArray );
                m_formats.append( format );
            }
        }
    }

    ~MimeSource() {}

    virtual const char *format( int i ) const {
        if ( i < (int) m_formats.count() )
            return m_formats.tqat( i );
        else
            return 0L;
    }
    virtual bool provides( const char *mimeType ) const {
        return ( m_formats.find( mimeType ) > -1 );
    }
    virtual TQByteArray encodedData( const char *format ) const
    {
        int index = m_formats.find( format );
        if ( index > -1 )
            return *(m_data.tqat( index ));

        return TQByteArray();
    }

private:
    mutable TQStrList m_formats;
    mutable TQPtrList<TQByteArray> m_data;
};


KClipboardSynchronizer * KClipboardSynchronizer::s_self = 0L;
bool KClipboardSynchronizer::s_sync = false;
bool KClipboardSynchronizer::s_reverse_sync = false;
bool KClipboardSynchronizer::s_blocked = false;

KClipboardSynchronizer * KClipboardSynchronizer::self()
{
    if ( !s_self )
        s_self = new KClipboardSynchronizer( TQT_TQOBJECT(kapp), "KDE Clipboard" );

    return s_self;
}

KClipboardSynchronizer::KClipboardSynchronizer( TQObject *parent, const char *name )
    : TQObject( parent, name )
{
    s_self = this;

    KConfigGroup config( KGlobal::config(), "General" );
    s_sync = config.readBoolEntry( "SynchronizeClipboardAndSelection", s_sync);
    s_reverse_sync = config.readBoolEntry( "ClipboardSetSelection",
                                                s_reverse_sync );

    setupSignals();
}

KClipboardSynchronizer::~KClipboardSynchronizer()
{
    if ( s_self == this )
        s_self = 0L;
}

void KClipboardSynchronizer::setupSignals()
{
    TQClipboard *clip = TQApplication::tqclipboard();
    disconnect( clip, NULL, this, NULL );
    if( s_sync )
        connect( clip, TQT_SIGNAL( selectionChanged() ),
                 TQT_SLOT( slotSelectionChanged() ));
    if( s_reverse_sync )
        connect( clip, TQT_SIGNAL( dataChanged() ),
                 TQT_SLOT( slotClipboardChanged() ));
}

void KClipboardSynchronizer::slotSelectionChanged()
{
    TQClipboard *clip = TQApplication::tqclipboard();

//     qDebug("*** sel changed: %i", s_blocked);
    if ( s_blocked || !clip->ownsSelection() )
        return;

    setClipboard( new MimeSource( clip->data( TQClipboard::Selection) ),
                  TQClipboard::Clipboard );
}

void KClipboardSynchronizer::slotClipboardChanged()
{
    TQClipboard *clip = TQApplication::tqclipboard();

//     qDebug("*** clip changed : %i (implicit: %i, ownz: clip: %i, selection: %i)", s_blocked, s_implicitSelection, clip->ownsClipboard(), clip->ownsSelection());
    if ( s_blocked || !clip->ownsClipboard() )
        return;

    setClipboard( new MimeSource( clip->data( TQClipboard::Clipboard ) ),
                  TQClipboard::Selection );
}

void KClipboardSynchronizer::setClipboard( TQMimeSource *data, TQClipboard::Mode mode )
{
//     qDebug("---> setting clipboard: %p", data);

    TQClipboard *clip = TQApplication::tqclipboard();

    s_blocked = true;

    if ( mode == TQClipboard::Clipboard )
    {
        clip->setData( data, TQClipboard::Clipboard );
    }
    else if ( mode == TQClipboard::Selection )
    {
        clip->setData( data, TQClipboard::Selection );
    }

    s_blocked = false;
}

void KClipboardSynchronizer::setSynchronizing( bool sync )
{
    s_sync = sync;
    self()->setupSignals();
}

void KClipboardSynchronizer::setReverseSynchronizing( bool enable )
{
    s_reverse_sync = enable;
    self()->setupSignals();
}

// private, called by KApplication
void KClipboardSynchronizer::newConfiguration( int config )
{
    s_sync = (config & Synchronize);
    self()->setupSignals();
}

#include "kclipboard.moc"