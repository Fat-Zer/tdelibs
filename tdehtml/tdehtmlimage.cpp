/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

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

#include "tdehtmlimage.h"
#include "tdehtmlview.h"
#include "tdehtml_ext.h"
#include "xml/dom_docimpl.h"
#include "html/html_documentimpl.h"
#include "html/html_elementimpl.h"
#include "rendering/render_image.h"
#include "misc/loader.h"

#include <tqvbox.h>
#include <tqtimer.h>

#include <tdeio/job.h>
#include <kinstance.h>
#include <kmimetype.h>
#include <klocale.h>

K_EXPORT_COMPONENT_FACTORY( tdehtmlimagefactory /*NOT the part name, see Makefile.am*/, KHTMLImageFactory )

TDEInstance *KHTMLImageFactory::s_instance = 0;

KHTMLImageFactory::KHTMLImageFactory()
{
    s_instance = new TDEInstance( "tdehtmlimage" );
}

KHTMLImageFactory::~KHTMLImageFactory()
{
    delete s_instance;
}

KParts::Part *KHTMLImageFactory::createPartObject( TQWidget *parentWidget, const char *widgetName,
                                                   TQObject *parent, const char *name,
                                                   const char *className, const TQStringList & )
{
  KHTMLPart::GUIProfile prof = KHTMLPart::DefaultGUI;
  if ( strcmp( className, "Browser/View" ) == 0 )
    prof = KHTMLPart::BrowserViewGUI;
  return new KHTMLImage( parentWidget, widgetName, parent, name, prof );
}

KHTMLImage::KHTMLImage( TQWidget *parentWidget, const char *widgetName,
                        TQObject *parent, const char *name, KHTMLPart::GUIProfile prof )
    : KParts::ReadOnlyPart( parent, name ), m_image( 0 )
{
    KHTMLPart* parentPart = ::tqqt_cast<KHTMLPart *>( parent );
    setInstance( KHTMLImageFactory::instance(), prof == KHTMLPart::BrowserViewGUI && !parentPart );

    TQVBox *box = new TQVBox( parentWidget, widgetName );

    m_tdehtml = new KHTMLPart( box, widgetName, this, "htmlimagepart", prof );
    m_tdehtml->setAutoloadImages( true );
    m_tdehtml->widget()->installEventFilter(this);
    connect( m_tdehtml->view(), TQT_SIGNAL( finishedLayout() ), this, TQT_SLOT( restoreScrollPosition() ) );

    setWidget( box );

    // VBox can't take focus, so pass it on to sub-widget
    box->setFocusProxy( m_tdehtml->widget() );

    m_ext = new KHTMLImageBrowserExtension( this, "be" );

    // Remove unnecessary actions.
    KAction *encodingAction = actionCollection()->action( "setEncoding" );
    if ( encodingAction )
    {
        encodingAction->unplugAll();
        delete encodingAction;
    }
    KAction *viewSourceAction= actionCollection()->action( "viewDocumentSource" );
    if ( viewSourceAction )
    {
        viewSourceAction->unplugAll();
        delete viewSourceAction;
    }

    KAction *selectAllAction= actionCollection()->action( "selectAll" );
    if ( selectAllAction )
    {
        selectAllAction->unplugAll();
        delete selectAllAction;
    }

    // forward important signals from the tdehtml part

    // forward opening requests to parent frame (if existing)
    KHTMLPart *p = ::tqqt_cast<KHTMLPart *>(parent);
    KParts::BrowserExtension *be = p ? p->browserExtension() : m_ext;
    connect(m_tdehtml->browserExtension(), TQT_SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)),
    		be, TQT_SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)));

    connect( m_tdehtml->browserExtension(), TQT_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &,
             const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t) ), m_ext, TQT_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &,
             const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t) ) );

    connect( m_tdehtml->browserExtension(), TQT_SIGNAL( enableAction( const char *, bool ) ),
             m_ext, TQT_SIGNAL( enableAction( const char *, bool ) ) );

    m_ext->setURLDropHandlingEnabled( true );
}

KHTMLImage::~KHTMLImage()
{
    disposeImage();

    // important: delete the html part before the part or qobject destructor runs.
    // we now delete the htmlpart which deletes the part's widget which makes
    // _OUR_ m_widget 0 which in turn avoids our part destructor to delete the
    // widget ;-)
    // ### additional note: it _can_ be that the part has been deleted before:
    // when we're in a html frameset and the view dies first, then it will also
    // kill the htmlpart
    if ( m_tdehtml )
        delete static_cast<KHTMLPart *>( m_tdehtml );
}

bool KHTMLImage::openURL( const KURL &url )
{
    static const TQString &html = TDEGlobal::staticQString( "<html><body><img src=\"%1\"></body></html>" );

    disposeImage();

    m_url = url;

    emit started( 0 );

    KParts::URLArgs args = m_ext->urlArgs();
    m_mimeType = args.serviceType;

    emit setWindowCaption( url.prettyURL() );

    // Need to keep a copy of the offsets since they are cleared when emitting completed
    m_xOffset = args.xOffset;
    m_yOffset = args.yOffset;

    m_tdehtml->begin( m_url );
    m_tdehtml->setAutoloadImages( true );

    DOM::DocumentImpl *impl = dynamic_cast<DOM::DocumentImpl *>( m_tdehtml->document().handle() ); // ### hack ;-)
    if (!impl) return false;
    if ( m_ext->urlArgs().reload )
        impl->docLoader()->setCachePolicy( TDEIO::CC_Reload );

    tdehtml::DocLoader *dl = impl->docLoader();
    m_image = dl->requestImage( m_url.url() );
    if ( m_image )
        m_image->ref( this );

    m_tdehtml->write( html.arg( m_url.url() ) );
    m_tdehtml->end();

    /*
    connect( tdehtml::Cache::loader(), TQT_SIGNAL( requestDone( tdehtml::DocLoader*, tdehtml::CachedObject *) ),
            this, TQT_SLOT( updateWindowCaption() ) );
            */
    return true;
}

bool KHTMLImage::closeURL()
{
    disposeImage();
    return m_tdehtml->closeURL();
}

// This can happen after openURL returns, or directly from m_image->ref()
void KHTMLImage::notifyFinished( tdehtml::CachedObject *o )
{
    if ( !m_image || o != m_image )
        return;

    const TQPixmap &pix = m_image->pixmap();
    TQString caption;

    KMimeType::Ptr mimeType;
    if ( !m_mimeType.isEmpty() )
        mimeType = KMimeType::mimeType( m_mimeType );

    if ( mimeType ) {
        if (m_image && !m_image->suggestedTitle().isEmpty()) {
            caption = i18n( "%1 (%2 - %3x%4 Pixels)" ).arg( m_image->suggestedTitle(), mimeType->comment() ).arg( pix.width() ).arg( pix.height() );
        } else {
            caption = i18n( "%1 - %2x%3 Pixels" ).arg( mimeType->comment() )
                .arg( pix.width() ).arg( pix.height() );
        }
    } else {
        if (m_image && !m_image->suggestedTitle().isEmpty()) {
            caption = i18n( "%1 (%2x%3 Pixels)" ).arg(m_image->suggestedTitle()).arg( pix.width() ).arg( pix.height() );
        } else {
            caption = i18n( "Image - %1x%2 Pixels" ).arg( pix.width() ).arg( pix.height() );
        }
    }

    emit setWindowCaption( caption );
    emit completed();
    emit setStatusBarText(i18n("Done."));
}

void KHTMLImage::restoreScrollPosition()
{
    if ( m_tdehtml->view()->contentsY() == 0 ) {
        m_tdehtml->view()->setContentsPos( m_xOffset, m_yOffset );
    }
}

void KHTMLImage::guiActivateEvent( KParts::GUIActivateEvent *e )
{
    // prevent the base implementation from emitting setWindowCaption with
    // our url. It destroys our pretty, previously caption. Konq saves/restores
    // the caption for us anyway.
    if ( e->activated() )
        return;
    KParts::ReadOnlyPart::guiActivateEvent(e);
}

/*
void KHTMLImage::slotImageJobFinished( TDEIO::Job *job )
{
    if ( job->error() )
    {
        job->showErrorDialog();
        emit canceled( job->errorString() );
    }
    else
    {
        emit completed();
        TQTimer::singleShot( 0, this, TQT_SLOT( updateWindowCaption() ) );
    }
}

void KHTMLImage::updateWindowCaption()
{
    if ( !m_tdehtml )
        return;

    DOM::HTMLDocumentImpl *impl = dynamic_cast<DOM::HTMLDocumentImpl *>( m_tdehtml->document().handle() );
    if ( !impl )
        return;

    DOM::HTMLElementImpl *body = impl->body();
    if ( !body )
        return;

    DOM::NodeImpl *image = body->firstChild();
    if ( !image )
        return;

    tdehtml::RenderImage *renderImage = dynamic_cast<tdehtml::RenderImage *>( image->renderer() );
    if ( !renderImage )
        return;

    TQPixmap pix = renderImage->pixmap();

    TQString caption;

    KMimeType::Ptr mimeType;
    if ( !m_mimeType.isEmpty() )
        mimeType = KMimeType::mimeType( m_mimeType );

    if ( mimeType )
        caption = i18n( "%1 - %2x%3 Pixels" ).arg( mimeType->comment() )
                  .arg( pix.width() ).arg( pix.height() );
    else
        caption = i18n( "Image - %1x%2 Pixels" ).arg( pix.width() ).arg( pix.height() );

    emit setWindowCaption( caption );
    emit completed();
    emit setStatusBarText(i18n("Done."));
}
*/

void KHTMLImage::disposeImage()
{
    if ( !m_image )
        return;

    m_image->deref( this );
    m_image = 0;
}

bool KHTMLImage::eventFilter(TQObject *, TQEvent *e) {
    switch (e->type()) {
      case TQEvent::DragEnter:
      case TQEvent::DragMove:
      case TQEvent::DragLeave:
      case TQEvent::Drop: {
        // find out if this part is embedded in a frame, and send the
	// event to its outside widget
	KHTMLPart *p = ::tqqt_cast<KHTMLPart *>(parent());
	if (p)
	    return TQApplication::sendEvent(p->widget(), e);
        // otherwise simply forward all dnd events to the part widget,
	// konqueror will handle them properly there
        return TQApplication::sendEvent(widget(), e);
      }
      default: ;
    }
    return false;
}

KHTMLImageBrowserExtension::KHTMLImageBrowserExtension( KHTMLImage *parent, const char *name )
    : KParts::BrowserExtension( parent, name )
{
    m_imgPart = parent;
}

int KHTMLImageBrowserExtension::xOffset()
{
    return m_imgPart->doc()->view()->contentsX();
}

int KHTMLImageBrowserExtension::yOffset()
{
    return m_imgPart->doc()->view()->contentsY();
}

void KHTMLImageBrowserExtension::print()
{
    static_cast<KHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->print();
}

void KHTMLImageBrowserExtension::reparseConfiguration()
{
    static_cast<KHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->reparseConfiguration();
    m_imgPart->doc()->setAutoloadImages( true );
}


void KHTMLImageBrowserExtension::disableScrolling()
{
    static_cast<KHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->disableScrolling();
}

using namespace KParts;

/* vim: et sw=4 ts=4
 */

#include "tdehtmlimage.moc"
