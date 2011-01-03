/*
 * This file is part of the KDE project.
 * Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include "kfilemetapreview.h"

#include <tqlayout.h>

#include <kio/previewjob.h>
#include <klibloader.h>
#include <kimagefilepreview.h>
#include <kmimetype.h>

bool KFileMetaPreview::s_tryAudioPreview = true;

KFileMetaPreview::KFileMetaPreview( TQWidget *parent, const char *name )
    : KPreviewWidgetBase( parent, name ),
      haveAudioPreview( false )
{
    TQHBoxLayout *tqlayout = new TQHBoxLayout( this, 0, 0 );
    m_stack = new TQWidgetStack( this );
    tqlayout->addWidget( m_stack );

    // ###
//     m_previewProviders.setAutoDelete( true );
    initPreviewProviders();
}

KFileMetaPreview::~KFileMetaPreview()
{
}

void KFileMetaPreview::initPreviewProviders()
{
    m_previewProviders.clear();
    // hardcoded so far

    // image previews
    KImageFilePreview *imagePreview = new KImageFilePreview( m_stack );
    (void) m_stack->addWidget( imagePreview );
    m_stack->raiseWidget( imagePreview );
    resize( imagePreview->tqsizeHint() );

    TQStringList mimeTypes = imagePreview->supportedMimeTypes();
    TQStringList::ConstIterator it = mimeTypes.begin();
    for ( ; it != mimeTypes.end(); ++it )
    {
//         qDebug(".... %s", (*it).latin1());
        m_previewProviders.insert( *it, imagePreview );
    }
}

KPreviewWidgetBase * KFileMetaPreview::previewProviderFor( const TQString& mimeType )
{
//     qDebug("### looking for: %s", mimeType.latin1());
    // often the first highlighted item, where we can be sure, there is no plugin
    // (this "folders reflect icons" is a konq-specific thing, right?)
    if ( mimeType == "inode/directory" ) 
        return 0L;

    KPreviewWidgetBase *provider = m_previewProviders.tqfind( mimeType );
    if ( provider )
        return provider;

//qDebug("#### didn't tqfind anything for: %s", mimeType.latin1());

    if ( s_tryAudioPreview && 
         !mimeType.startsWith("text/") && !mimeType.startsWith("image/") )
    {
        if ( !haveAudioPreview )
        {
            KPreviewWidgetBase *audioPreview = createAudioPreview( m_stack );
            if ( audioPreview )
            {
                haveAudioPreview = true;
                (void) m_stack->addWidget( audioPreview );
                TQStringList mimeTypes = audioPreview->supportedMimeTypes();
                TQStringList::ConstIterator it = mimeTypes.begin();
                for ( ; it != mimeTypes.end(); ++it )
                    m_previewProviders.insert( *it, audioPreview );
            }
        }
    }

    // with the new mimetypes from the audio-preview, try again
    provider = m_previewProviders.tqfind( mimeType );
    if ( provider )
        return provider;

    // ### mimetype may be image/* for example, try that
    int index = mimeType.tqfind( '/' );
    if ( index > 0 )
    {
        provider = m_previewProviders.tqfind( mimeType.left( index + 1 ) + "*" );
        if ( provider )
            return provider;
    }

    KMimeType::Ptr mimeInfo = KMimeType::mimeType( mimeType );
    if ( mimeInfo )
    {
        // check mime type inheritance
        TQString parentMimeType = mimeInfo->parentMimeType();
        while ( !parentMimeType.isEmpty() )
        {
            provider = m_previewProviders.tqfind( parentMimeType );
            if ( provider )
                return provider;

            KMimeType::Ptr parentMimeInfo = KMimeType::mimeType( parentMimeType );
            if ( !parentMimeInfo ) break;

            parentMimeType = parentMimeInfo->parentMimeType();
        }

        // check X-KDE-Text property
        TQVariant textProperty = mimeInfo->property( "X-KDE-text" );
        if ( textProperty.isValid() && textProperty.type() == TQVariant::Bool )
        {
            if ( textProperty.toBool() )
            {
                provider = m_previewProviders.tqfind( "text/plain" );
                if ( provider )
                    return provider;

                provider = m_previewProviders.tqfind( "text/*" );
                if ( provider )
                    return provider;
            }
        }
    }

    return 0L;
}

void KFileMetaPreview::showPreview(const KURL &url)
{
    KMimeType::Ptr mt = KMimeType::findByURL( url );
    KPreviewWidgetBase *provider = previewProviderFor( mt->name() );
    if ( provider )
    {
        if ( provider != m_stack->visibleWidget() ) // stop the previous preview
            clearPreview();

        m_stack->setEnabled( true );
        m_stack->raiseWidget( provider );
        provider->showPreview( url );
    }
    else
    {
        clearPreview();
        m_stack->setEnabled( false );
    }
}

void KFileMetaPreview::clearPreview()
{
    if ( m_stack->visibleWidget() )
        static_cast<KPreviewWidgetBase*>( m_stack->visibleWidget() )->clearPreview();
}

void KFileMetaPreview::addPreviewProvider( const TQString& mimeType,
                                           KPreviewWidgetBase *provider )
{
    m_previewProviders.insert( mimeType, provider );
}

void KFileMetaPreview::clearPreviewProviders()
{
    TQDictIterator<KPreviewWidgetBase> it( m_previewProviders );
    for ( ; it.current(); ++it )
        m_stack->removeWidget( it.current() );

    m_previewProviders.clear();
}

// static
KPreviewWidgetBase * KFileMetaPreview::createAudioPreview( TQWidget *parent )
{
    KLibFactory *factory = KLibLoader::self()->factory( "kfileaudiopreview" );
    if ( !factory )
    {
        s_tryAudioPreview = false;
        return 0L;
    }

    return dynamic_cast<KPreviewWidgetBase*>( factory->create( parent, "kfileaudiopreview" ));
}

void KFileMetaPreview::virtual_hook( int, void* ) {}

#include "kfilemetapreview.moc"
