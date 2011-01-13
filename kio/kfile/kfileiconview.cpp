// -*- c++ -*-
/* This file is part of the KDE libraries
   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
                 2000,2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.	If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <tqt.h>

#include <tqfontmetrics.h>
#include <tqkeycode.h>
#include <tqlabel.h>
#include <tqpainter.h>
#include <tqpixmap.h>
#include <tqregexp.h>
#include <tqtimer.h>
#include <tqtooltip.h>

#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kiconeffect.h>
#include <kglobalsettings.h>
#include <kurldrag.h>
#include <kio/previewjob.h>

#include "kfileiconview.h"
#include "config-kfile.h"

#define DEFAULT_PREVIEW_SIZE 60
#define DEFAULT_SHOW_PREVIEWS false
#define DEFAULT_VIEW_MODE "SmallColumns"

KFileIconViewItem::~KFileIconViewItem()
{
    fileInfo()->removeExtraData( iconView() );
}

class KFileIconView::KFileIconViewPrivate
{
public:
    KFileIconViewPrivate( KFileIconView *parent ) {
        previewIconSize = 60;
        job = 0;
        dropItem = 0;

        noArrangement = false;
	ignoreMaximumSize = false;
	smallColumns = new KRadioAction( i18n("Small Icons"), 0, TQT_TQOBJECT(parent),
					 TQT_SLOT( slotSmallColumns() ),
					 parent->actionCollection(),
					 "small columns" );

	largeRows = new KRadioAction( i18n("Large Icons"), 0, TQT_TQOBJECT(parent),
				      TQT_SLOT( slotLargeRows() ),
				      parent->actionCollection(),
				      "large rows" );

	smallColumns->setExclusiveGroup(TQString::tqfromLatin1("IconView mode"));
	largeRows->setExclusiveGroup(TQString::tqfromLatin1("IconView mode"));

        previews = new KToggleAction( i18n("Thumbnail Previews"), 0,
                                      parent->actionCollection(),
                                      "show previews" );
        zoomIn = KStdAction::zoomIn( TQT_TQOBJECT(parent), TQT_SLOT( zoomIn() ),
                                     parent->actionCollection(), "zoomIn" );
        zoomOut = KStdAction::zoomOut( TQT_TQOBJECT(parent), TQT_SLOT( zoomOut() ),
                                     parent->actionCollection(), "zoomOut" );

        previews->setGroup("previews");
        zoomIn->setGroup("previews");
        zoomOut->setGroup("previews");

        connect( previews, TQT_SIGNAL( toggled( bool )),
                 parent, TQT_SLOT( slotPreviewsToggled( bool )));

        connect( &previewTimer, TQT_SIGNAL( timeout() ),
                 parent, TQT_SLOT( showPreviews() ));
        connect( &autoOpenTimer, TQT_SIGNAL( timeout() ),
                 parent, TQT_SLOT( slotAutoOpen() ));
    }

    ~KFileIconViewPrivate() {
        if ( job )
            job->kill();
    }

    KRadioAction *smallColumns, *largeRows;
    KAction *zoomIn, *zoomOut;
    KToggleAction *previews;
    KIO::PreviewJob *job;
    KFileIconViewItem *dropItem;
    TQTimer previewTimer;
    TQTimer autoOpenTimer;
    TQStringList previewMimeTypes;
    int previewIconSize;
    bool noArrangement :1;
    bool ignoreMaximumSize :1;
};

KFileIconView::KFileIconView(TQWidget *parent, const char *name)
    : KIconView(parent, name), KFileView()
{
    d = new KFileIconViewPrivate( this );

    setViewName( i18n("Icon View") );

    toolTip = 0;
    setResizeMode( Adjust );
    setMaxItemWidth( 300 );
    setWordWrapIconText( false );
    setArrangement( TopToBottom );
    setAutoArrange( true );
    setItemsMovable( false );
    setMode( KIconView::Select );
    KIconView::setSorting( true );
    // as long as TQIconView only shows tooltips when the cursor is over the
    // icon (and not the text), we have to create our own tooltips
    setShowToolTips( false );
    slotSmallColumns();
    d->smallColumns->setChecked( true );

    connect( this, TQT_SIGNAL( returnPressed(TQIconViewItem *) ),
	     TQT_SLOT( slotActivate( TQIconViewItem *) ) );

    // we want single click _and_ double click (as convenience)
    connect( this, TQT_SIGNAL( clicked(TQIconViewItem *, const TQPoint&) ),
	     TQT_SLOT( selected( TQIconViewItem *) ) );
    connect( this, TQT_SIGNAL( doubleClicked(TQIconViewItem *, const TQPoint&) ),
	     TQT_SLOT( slotActivate( TQIconViewItem *) ) );

    connect( this, TQT_SIGNAL( onItem( TQIconViewItem * ) ),
	     TQT_SLOT( showToolTip( TQIconViewItem * ) ) );
    connect( this, TQT_SIGNAL( onViewport() ),
	     TQT_SLOT( removeToolTip() ) );
    connect( this, TQT_SIGNAL( contextMenuRequested(TQIconViewItem*,const TQPoint&)),
	     TQT_SLOT( slotActivateMenu( TQIconViewItem*, const TQPoint& ) ) );

    KFile::SelectionMode sm = KFileView::selectionMode();
    switch ( sm ) {
    case KFile::Multi:
	TQIconView::setSelectionMode( TQIconView::Multi );
	break;
    case KFile::Extended:
	TQIconView::setSelectionMode( TQIconView::Extended );
	break;
    case KFile::NoSelection:
	TQIconView::setSelectionMode( TQIconView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	TQIconView::setSelectionMode( TQIconView::Single );
	break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, TQT_SIGNAL( selectionChanged() ),
		 TQT_SLOT( slotSelectionChanged() ));
    else
	connect( this, TQT_SIGNAL( selectionChanged( TQIconViewItem * )),
		 TQT_SLOT( highlighted( TQIconViewItem * )));

    viewport()->installEventFilter( this );

    // for mimetype resolving
    m_resolver = new KMimeTypeResolver<KFileIconViewItem,KFileIconView>(this);
}

KFileIconView::~KFileIconView()
{
    delete m_resolver;
    removeToolTip();
    delete d;
}

void KFileIconView::readConfig( KConfig *kc, const TQString& group )
{
    TQString gr = group.isEmpty() ? TQString("KFileIconView") : group;
    KConfigGroupSaver cs( kc, gr );
    TQString small = TQString::tqfromLatin1("SmallColumns");
    d->previewIconSize = kc->readNumEntry( "Preview Size", DEFAULT_PREVIEW_SIZE );
    d->previews->setChecked( kc->readBoolEntry( "ShowPreviews", DEFAULT_SHOW_PREVIEWS ) );

    if ( kc->readEntry("ViewMode", DEFAULT_VIEW_MODE ) == small ) {
	d->smallColumns->setChecked( true );
	slotSmallColumns();
    }
    else {
	d->largeRows->setChecked( true );
	slotLargeRows();
    }

    if ( d->previews->isChecked() )
        showPreviews();
}

void KFileIconView::writeConfig( KConfig *kc, const TQString& group )
{
    TQString gr = group.isEmpty() ? TQString("KFileIconView") : group;
    KConfigGroupSaver cs( kc, gr );

    TQString viewMode =  d->smallColumns->isChecked() ?
        TQString::tqfromLatin1("SmallColumns") :
        TQString::tqfromLatin1("LargeRows");
    if(!kc->hasDefault( "ViewMode" ) && viewMode == DEFAULT_VIEW_MODE )
        kc->revertToDefault( "ViewMode" );
    else
        kc->writeEntry( "ViewMode", viewMode );

    int previewsIconSize = d->previewIconSize;
    if(!kc->hasDefault( "Preview Size" ) && previewsIconSize == DEFAULT_PREVIEW_SIZE )
        kc->revertToDefault( "Preview Size" );
    else
        kc->writeEntry( "Preview Size", previewsIconSize );

    bool showPreviews = d->previews->isChecked();
    if(!kc->hasDefault( "ShowPreviews" ) && showPreviews == DEFAULT_SHOW_PREVIEWS )
        kc->revertToDefault( "ShowPreviews" );
    else
        kc->writeEntry( "ShowPreviews", showPreviews );
}

void KFileIconView::removeToolTip()
{
    delete toolTip;
    toolTip = 0;
}

void KFileIconView::showToolTip( TQIconViewItem *item )
{
    delete toolTip;
    toolTip = 0;

    if ( !item )
	return;

    int w = maxItemWidth() - ( itemTextPos() == TQIconView::Bottom ? 0 :
			       item->pixmapRect().width() ) - 4;
    if ( fontMetrics().width( item->text() ) >= w ) {
	toolTip = new TQLabel( TQString::tqfromLatin1(" %1 ").arg(item->text()), 0,
			      "myToolTip",
			      (WFlags)(WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM) );
	toolTip->setFrameStyle( TQFrame::Plain | TQFrame::Box );
	toolTip->setLineWidth( 1 );
	toolTip->tqsetAlignment( AlignLeft | AlignTop );
	toolTip->move( TQCursor::pos() + TQPoint( 14, 14 ) );
	toolTip->adjustSize();
	TQRect screen = TQApplication::desktop()->screenGeometry(
			TQApplication::desktop()->screenNumber(TQCursor::pos()));
	if (toolTip->x()+toolTip->width() > screen.right()) {
		toolTip->move(toolTip->x()+screen.right()-toolTip->x()-toolTip->width(), toolTip->y());
	}
	if (toolTip->y()+toolTip->height() > screen.bottom()) {
		toolTip->move(toolTip->x(), screen.bottom()-toolTip->y()-toolTip->height()+toolTip->y());
	}
	toolTip->setFont( TQToolTip::font() );
	toolTip->tqsetPalette( TQToolTip::palette(), true );
	toolTip->show();
    }
}

void KFileIconView::slotActivateMenu( TQIconViewItem* item, const TQPoint& pos )
{
    if ( !item ) {
	sig->activateMenu( 0, pos );
	return;
    }
    KFileIconViewItem *i = (KFileIconViewItem*) item;
    sig->activateMenu( i->fileInfo(), pos );
}

void KFileIconView::hideEvent( TQHideEvent *e )
{
    removeToolTip();
    KIconView::hideEvent( e );
}

void KFileIconView::keyPressEvent( TQKeyEvent *e )
{
    KIconView::keyPressEvent( e );

    // ignore Ctrl-Return so that the dialog can catch it.
    if ( (e->state() & ControlButton) &&
         (e->key() == Key_Return || e->key() == Key_Enter) )
        e->ignore();
}

void KFileIconView::setSelected( const KFileItem *info, bool enable )
{
    KFileIconViewItem *item = viewItem( info );
    if ( item )
        KIconView::setSelected( item, enable, true );
}

void KFileIconView::selectAll()
{
    if (KFileView::selectionMode() == KFile::NoSelection ||
        KFileView::selectionMode() == KFile::Single)
	return;

    KIconView::selectAll( true );
}

void KFileIconView::clearSelection()
{
    KIconView::clearSelection();
}

void KFileIconView::invertSelection()
{
    KIconView::invertSelection();
}

void KFileIconView::clearView()
{
    m_resolver->m_lstPendingMimeIconItems.clear();

    KIconView::clear();
    stopPreview();
}

void KFileIconView::insertItem( KFileItem *i )
{
    KFileView::insertItem( i );

    TQIconView* qview = static_cast<TQIconView*>( this );
    // Since creating and initializing an item leads to a tqrepaint,
    // we disable updates on the IconView for a while.
    qview->setUpdatesEnabled( false );
    KFileIconViewItem *item = new KFileIconViewItem( qview, i );
    initItem( item, i, true );
    qview->setUpdatesEnabled( true );

    if ( !i->isMimeTypeKnown() )
        m_resolver->m_lstPendingMimeIconItems.append( item );

    i->setExtraData( this, item );
}

void KFileIconView::slotActivate( TQIconViewItem *item )
{
    if ( !item )
	return;
    const KFileItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    if ( fi )
	sig->activate( fi );
}

void KFileIconView::selected( TQIconViewItem *item )
{
    if ( !item || (KApplication::keyboardMouseState() & (ShiftButton | ControlButton)) != 0 )
	return;

    if ( KGlobalSettings::singleClick() ) {
	const KFileItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
	if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
	    sig->activate( fi );
    }
}

void KFileIconView::setCurrentItem( const KFileItem *item )
{
    KFileIconViewItem *it = viewItem( item );
    if ( it )
        KIconView::setCurrentItem( it );
}

KFileItem * KFileIconView::currentFileItem() const
{
    KFileIconViewItem *current = static_cast<KFileIconViewItem*>( currentItem() );
    if ( current )
        return current->fileInfo();

    return 0L;
}

void KFileIconView::highlighted( TQIconViewItem *item )
{
    if ( !item )
	return;
    const KFileItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    if ( fi )
	sig->highlightFile( fi );
}

void KFileIconView::setSelectionMode( KFile::SelectionMode sm )
{
    disconnect( TQT_SIGNAL( selectionChanged() ), this );
    disconnect( TQT_SIGNAL( selectionChanged( TQIconViewItem * )), this );

    KFileView::setSelectionMode( sm );
    switch ( KFileView::selectionMode() ) {
    case KFile::Multi:
	TQIconView::setSelectionMode( TQIconView::Multi );
	break;
    case KFile::Extended:
	TQIconView::setSelectionMode( TQIconView::Extended );
	break;
    case KFile::NoSelection:
	TQIconView::setSelectionMode( TQIconView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	TQIconView::setSelectionMode( TQIconView::Single );
	break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, TQT_SIGNAL( selectionChanged() ),
		 TQT_SLOT( slotSelectionChanged() ));
    else
	connect( this, TQT_SIGNAL( selectionChanged( TQIconViewItem * )),
		 TQT_SLOT( highlighted( TQIconViewItem * )));
}

bool KFileIconView::isSelected( const KFileItem *i ) const
{
    KFileIconViewItem *item = viewItem( i );
    return (item && item->isSelected());
}

void KFileIconView::updateView( bool b )
{
    if ( !b )
        return; // eh?

    KFileIconViewItem *item = static_cast<KFileIconViewItem*>(TQIconView::firstItem());
    if ( item ) {
        do {
            if ( d->previews->isChecked() ) {
                if ( canPreview( item->fileInfo() ) )
                    item->setPixmapSize( TQSize( d->previewIconSize, d->previewIconSize ) );
            }
            else {
                // unset pixmap size (used for previews)
                if ( !item->pixmapSize().isNull() )
                    item->setPixmapSize( TQSize( 0, 0 ) );
            }
            // recalculate item parameters but avoid an in-place tqrepaint
            item->setPixmap( (item->fileInfo())->pixmap( myIconSize ), true, false );
            item = static_cast<KFileIconViewItem *>(item->nextItem());
        } while ( item != 0L );
    }
}

void KFileIconView::updateView( const KFileItem *i )
{
    KFileIconViewItem *item = viewItem( i );
    if ( item )
        initItem( item, i, true );
}

void KFileIconView::removeItem( const KFileItem *i )
{
    if ( !i )
	return;

    if ( d->job )
        d->job->removeItem( i );

    KFileIconViewItem *item = viewItem( i );
    m_resolver->m_lstPendingMimeIconItems.remove( item );
    delete item;

    KFileView::removeItem( i );
}

void KFileIconView::setIconSize( int size )
{
    myIconSize = size;
    updateIcons();
}

void KFileIconView::setPreviewSize( int size )
{
    if ( size < 30 )
        size = 30; // minimum

    d->previewIconSize = size;
    if ( d->previews->isChecked() )
        showPreviews();
}

void KFileIconView::setIgnoreMaximumSize(bool ignoreSize)
{
    d->ignoreMaximumSize = ignoreSize;
}

void KFileIconView::updateIcons()
{
    updateView( true );
    arrangeItemsInGrid();
}

void KFileIconView::ensureItemVisible( const KFileItem *i )
{
    KFileIconViewItem *item = viewItem( i );
    if ( item )
	KIconView::ensureItemVisible( item );
}

void KFileIconView::slotSelectionChanged()
{
    sig->highlightFile( 0L );
}

void KFileIconView::slotSmallColumns()
{
    // setItemTextPos(), setArrangement(), setWordWrapIconText() and
    // setIconSize() all call arrangeItemsInGrid() :( Prevent this.
    d->noArrangement = true; // stop arrangeItemsInGrid()!

    // Make sure to uncheck previews if selected
    if ( d->previews->isChecked() )
    {
        stopPreview();
        d->previews->setChecked( false );
    }
    setGridX( -1 );
    setMaxItemWidth( 300 );
    setItemTextPos( Right );
    setArrangement( TopToBottom );
    setWordWrapIconText( false );
    setSpacing( 0 );

    d->noArrangement = false; // now we can arrange
    setIconSize( KIcon::SizeSmall );
}

void KFileIconView::slotLargeRows()
{
    // setItemTextPos(), setArrangement(), setWordWrapIconText() and
    // setIconSize() all call arrangeItemsInGrid() :( Prevent this.
    d->noArrangement = true; // stop arrangeItemsInGrid()!

    setGridX( KGlobal::iconLoader()->currentSize( KIcon::Desktop ) + 50 );
    setItemTextPos( Bottom );
    setArrangement( LeftToRight );
    setWordWrapIconText( true );
    setSpacing( 5 ); // default in QIconView

    d->noArrangement = false; // now we can arrange
    setIconSize( KIcon::SizeMedium );
}

void KFileIconView::stopPreview()
{
    if ( d->job ) {
        d->job->kill();
        d->job = 0L;
    }
}

void KFileIconView::slotPreviewsToggled( bool on )
{
    if ( on )
        showPreviews();
    else {
        stopPreview();
        slotLargeRows();
    }
}

void KFileIconView::showPreviews()
{
    if ( d->previewMimeTypes.isEmpty() )
        d->previewMimeTypes = KIO::PreviewJob::supportedMimeTypes();

    stopPreview();
    d->previews->setChecked( true );

    if ( !d->largeRows->isChecked() ) {
        d->largeRows->setChecked( true );
        slotLargeRows(); // also sets the icon size and updates the grid
    }
    else {
        updateIcons();
    }

    d->job = KIO::filePreview(*items(), d->previewIconSize,d->previewIconSize);
    d->job->setIgnoreMaximumSize(d->ignoreMaximumSize);

    connect( d->job, TQT_SIGNAL( result( KIO::Job * )),
             this, TQT_SLOT( slotPreviewResult( KIO::Job * )));
    connect( d->job, TQT_SIGNAL( gotPreview( const KFileItem*, const TQPixmap& )),
             TQT_SLOT( gotPreview( const KFileItem*, const TQPixmap& ) ));
//     connect( d->job, TQT_SIGNAL( failed( const KFileItem* )),
//              this, TQT_SLOT( slotFailed( const KFileItem* ) ));
}

void KFileIconView::slotPreviewResult( KIO::Job *job )
{
    if ( job == d->job )
        d->job = 0L;
}

void KFileIconView::gotPreview( const KFileItem *item, const TQPixmap& pix )
{
    KFileIconViewItem *it = viewItem( item );
    if ( it )
        if( item->overlays() & KIcon::HiddenOverlay )
        {
            TQPixmap p( pix );

            KIconEffect::semiTransparent( p );
            it->setPixmap( p );
        }
        else
            it->setPixmap( pix );
}

bool KFileIconView::canPreview( const KFileItem *item ) const
{
    TQStringList::Iterator it = d->previewMimeTypes.begin();
    TQRegExp r;
    r.setWildcard( true );

    for ( ; it != d->previewMimeTypes.end(); ++it ) {
        TQString type = *it;
        // the "mimetype" can be "image/*"
        if ( type.at( type.length() - 1 ) == '*' ) {
            r.setPattern( type );
            if ( r.search( item->mimetype() ) != -1 )
                return true;
        }
        else
            if ( item->mimetype() == type )
                return true;
    }

    return false;
}

KFileItem * KFileIconView::firstFileItem() const
{
    KFileIconViewItem *item = static_cast<KFileIconViewItem*>( firstItem() );
    if ( item )
        return item->fileInfo();
    return 0L;
}

KFileItem * KFileIconView::nextItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        KFileIconViewItem *item = viewItem( fileItem );
        if ( item && item->nextItem() )
            return ((KFileIconViewItem*) item->nextItem())->fileInfo();
    }
    return 0L;
}

KFileItem * KFileIconView::prevItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        KFileIconViewItem *item = viewItem( fileItem );
        if ( item && item->prevItem() )
            return ((KFileIconViewItem*) item->prevItem())->fileInfo();
    }
    return 0L;
}

void KFileIconView::setSorting( TQDir::SortSpec spec )
{
    KFileView::setSorting( spec );
    KFileItemListIterator it( *items() );

    KFileItem *item;

    if ( spec & TQDir::Time ) {
        for ( ; (item = it.current()); ++it )
            // warning, time_t is often signed -> cast it
            viewItem(item)->setKey( sortingKey( (unsigned long)item->time( KIO::UDS_MODIFICATION_TIME ), item->isDir(), spec ));
    }

    else if ( spec & TQDir::Size ) {
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->size(), item->isDir(),
                                                spec ));
    }
    else { // Name or Unsorted
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->text(), item->isDir(),
                                                spec ));
    }

    KIconView::setSorting( true, !isReversed() );
    sort( !isReversed() );
}

//
// mimetype determination on demand
//
void KFileIconView::mimeTypeDeterminationFinished()
{
    // anything to do?
}

void KFileIconView::determineIcon( KFileIconViewItem *item )
{
    (void) item->fileInfo()->determineMimeType();
    updateView( item->fileInfo() );
}

void KFileIconView::listingCompleted()
{
    arrangeItemsInGrid();

    // TQIconView doesn't set the current item automatically, so we have to do
    // that. We don't want to emit selectionChanged() tho.
    if ( !currentItem() ) {
        bool block = signalsBlocked();
        blockSignals( true );
        TQIconViewItem *item = viewItem( firstFileItem() );
        KIconView::setCurrentItem( item );
        KIconView::setSelected( item, false );
        blockSignals( block );
    }

    m_resolver->start( d->previews->isChecked() ? 0 : 10 );
}

// need to remove our tooltip, eventually
bool KFileIconView::eventFilter( TQObject *o, TQEvent *e )
{
    if ( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(viewport()) || TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(this) ) {
        int type = e->type();
        if ( type == TQEvent::Leave ||
             type == TQEvent::FocusOut )
            removeToolTip();
    }

    return KIconView::eventFilter( o, e );
}

/////////////////////////////////////////////////////////////////

// ### workaround for Qt3 Bug
void KFileIconView::showEvent( TQShowEvent *e )
{
    KIconView::showEvent( e );
}


void KFileIconView::initItem( KFileIconViewItem *item, const KFileItem *i,
                              bool updateTextAndPixmap )
{
    if ( d->previews->isChecked() && canPreview( i ) )
        item->setPixmapSize( TQSize( d->previewIconSize, d->previewIconSize ) );

    if ( updateTextAndPixmap )
    {
        // this causes a tqrepaint of the item, which we want to avoid during
        // directory listing, when all items are created. We want to paint all
        // items at once, not every single item in that case.
        item->setText( i->text() , false, false );
        item->setPixmap( i->pixmap( myIconSize ) );
    }

    // see also setSorting()
    TQDir::SortSpec spec = KFileView::sorting();

    if ( spec & TQDir::Time )
        // warning, time_t is often signed -> cast it
        item->setKey( sortingKey( (unsigned long) i->time( KIO::UDS_MODIFICATION_TIME ),
                                  i->isDir(), spec ));
    else if ( spec & TQDir::Size )
        item->setKey( sortingKey( i->size(), i->isDir(), spec ));

    else // Name or Unsorted
        item->setKey( sortingKey( i->text(), i->isDir(), spec ));

    //qDebug("** key for: %s: %s", i->text().latin1(), item->key().latin1());

    if ( d->previews->isChecked() )
        d->previewTimer.start( 10, true );
}

void KFileIconView::arrangeItemsInGrid( bool update )
{
    if ( d->noArrangement )
        return;

    KIconView::arrangeItemsInGrid( update );
}

void KFileIconView::zoomIn()
{
    setPreviewSize( d->previewIconSize + 30 );
}

void KFileIconView::zoomOut()
{
    setPreviewSize( d->previewIconSize - 30 );
}

TQDragObject *KFileIconView::dragObject()
{
    // create a list of the URL:s that we want to drag
    KURL::List urls;
    KFileItemListIterator it( * KFileView::selectedItems() );
    for ( ; it.current(); ++it ){
        urls.append( (*it)->url() );
    }
    TQPixmap pixmap;
    if( urls.count() > 1 )
        pixmap = DesktopIcon( "kmultiple", iconSize() );
    if( pixmap.isNull() )
        pixmap = currentFileItem()->pixmap( iconSize() );

    TQPoint hotspot;
    hotspot.setX( pixmap.width() / 2 );
    hotspot.setY( pixmap.height() / 2 );
    TQDragObject* myDragObject = new KURLDrag( urls, widget() );
    myDragObject->setPixmap( pixmap, hotspot );
    return myDragObject;
}

void KFileIconView::slotAutoOpen()
{
    d->autoOpenTimer.stop();
    if( !d->dropItem )
        return;

    KFileItem *fileItem = d->dropItem->fileInfo();
    if (!fileItem)
        return;

    if( fileItem->isFile() )
        return;

    if ( fileItem->isDir() || fileItem->isLink())
        sig->activate( fileItem );
}

bool KFileIconView::acceptDrag(TQDropEvent* e) const
{
   return KURLDrag::canDecode( e ) &&
       (e->source()!=const_cast<KFileIconView*>(this)) &&
       ( e->action() == TQDropEvent::Copy
      || e->action() == TQDropEvent::Move
      || e->action() == TQDropEvent::Link );
}

void KFileIconView::contentsDragEnterEvent( TQDragEnterEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    KFileIconViewItem *item = dynamic_cast<KFileIconViewItem*>(tqfindItem( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       d->dropItem = item;
       d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void KFileIconView::contentsDragMoveEvent( TQDragMoveEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    KFileIconViewItem *item = dynamic_cast<KFileIconViewItem*>(tqfindItem( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       if (d->dropItem != item)
       {
           d->dropItem = item;
           d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
       }
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void KFileIconView::contentsDragLeaveEvent( TQDragLeaveEvent * )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();
}

void KFileIconView::contentsDropEvent( TQDropEvent *e )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();

    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    KFileIconViewItem *item = dynamic_cast<KFileIconViewItem*>(tqfindItem( contentsToViewport( e->pos() ) ));
    KFileItem * fileItem = 0;
    if (item)
        fileItem = item->fileInfo();

    emit dropped(e, fileItem);

    KURL::List urls;
    if (KURLDrag::decode( e, urls ) && !urls.isEmpty())
    {
        emit dropped(e, urls, fileItem ? fileItem->url() : KURL());
        sig->dropURLs(fileItem, e, urls);
    }
}

void KFileIconView::virtual_hook( int id, void* data )
{ KIconView::virtual_hook( id, data );
  KFileView::virtual_hook( id, data ); }

#include "kfileiconview.moc"
