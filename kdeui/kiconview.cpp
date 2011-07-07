/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

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

#include "config.h"

#include <tqtimer.h>
#include <tqpainter.h>
#include <tqpixmapcache.h>
#include <tqcleanuphandler.h>

#include "kiconview.h"
#include "kwordwrap.h"
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <kipc.h> 

#include <kcursor.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>

class KIconView::KIconViewPrivate
{
public:
    KIconViewPrivate() {
        mode = KIconView::Execute;
        fm = 0L;
        doAutoSelect = true;
        textHeight = 0;
        dragHoldItem = 0L;
    }
    KIconView::Mode mode;
    bool doAutoSelect;
    TQFontMetrics *fm;
    TQPixmapCache maskCache;
    int textHeight;
    TQIconViewItem *dragHoldItem;
    TQTimer dragHoldTimer;
    TQTimer doubleClickIgnoreTimer;
};

KIconView::KIconView( TQWidget *parent, const char *name, WFlags f )
    : TQIconView( parent, name, f )
{
    d = new KIconViewPrivate;

    connect( this, TQT_SIGNAL( onViewport() ),
             this, TQT_SLOT( slotOnViewport() ) );
    connect( this, TQT_SIGNAL( onItem( TQIconViewItem * ) ),
             this, TQT_SLOT( slotOnItem( TQIconViewItem * ) ) );
    slotSettingsChanged( KApplication::SETTINGS_MOUSE );
    if ( kapp ) { // maybe null when used inside designer
        connect( kapp, TQT_SIGNAL( settingsChanged(int) ), TQT_SLOT( slotSettingsChanged(int) ) );
        kapp->addKipcEventMask( KIPC::SettingsChanged );
    }

    m_pCurrentItem = 0L;

    m_pAutoSelect = new TQTimer( this );
    connect( m_pAutoSelect, TQT_SIGNAL( timeout() ),
             this, TQT_SLOT( slotAutoSelect() ) );

    connect( &d->dragHoldTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(slotDragHoldTimeout()) );
}

KIconView::~KIconView()
{
    delete d->fm;
    delete d;
}


void KIconView::setMode( KIconView::Mode mode )
{
    d->mode = mode;
}

KIconView::Mode KIconView::mode() const
{
    return d->mode;
}

void KIconView::slotOnItem( TQIconViewItem *item )
{
    if ( item ) {
        if ( m_bUseSingle ) {
            if ( m_bChangeCursorOverItem )
                viewport()->setCursor( KCursor().handCursor() );

            if ( (m_autoSelectDelay > -1) ) {
                m_pAutoSelect->start( m_autoSelectDelay, true );
            }
        }
        m_pCurrentItem = item;
    }
}

void KIconView::slotOnViewport()
{
    if ( m_bUseSingle && m_bChangeCursorOverItem )
        viewport()->unsetCursor();

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}

void KIconView::slotSettingsChanged(int category)
{
    if ( category != KApplication::SETTINGS_MOUSE )
      return;
    m_bUseSingle = KGlobalSettings::singleClick();
    //kdDebug() << "KIconView::slotSettingsChanged for mouse, usesingle=" << m_bUseSingle << endl;

    disconnect( this, TQT_SIGNAL( mouseButtonClicked( int, TQIconViewItem *,
						  const TQPoint & ) ),
		this, TQT_SLOT( slotMouseButtonClicked( int, TQIconViewItem *,
						    const TQPoint & ) ) );
//         disconnect( this, TQT_SIGNAL( doubleClicked( TQIconViewItem *,
// 						 const TQPoint & ) ),
// 		    this, TQT_SLOT( slotExecute( TQIconViewItem *,
// 					     const TQPoint & ) ) );

    if( m_bUseSingle ) {
      connect( this, TQT_SIGNAL( mouseButtonClicked( int, TQIconViewItem *,
						 const TQPoint & ) ),
	       this, TQT_SLOT( slotMouseButtonClicked( int, TQIconViewItem *,
						   const TQPoint & ) ) );
    }
    else {
//         connect( this, TQT_SIGNAL( doubleClicked( TQIconViewItem *,
// 					      const TQPoint & ) ),
//                  this, TQT_SLOT( slotExecute( TQIconViewItem *,
// 					  const TQPoint & ) ) );
    }

    m_bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();
    m_autoSelectDelay = m_bUseSingle ? KGlobalSettings::autoSelectDelay() : -1;

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
        viewport()->unsetCursor();
}

void KIconView::slotAutoSelect()
{
  // check that the item still exists
  if( index( m_pCurrentItem ) == -1 || !d->doAutoSelect )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  ButtonState keybstate = KApplication::keyboardMouseState();
  TQIconViewItem* previousItem = currentItem();
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & ShiftButton) ) {
      //Temporary implementation of the selection until TQIconView supports it
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & ControlButton) )
	clearSelection();

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->tqsetUpdatesEnabled( false );

      //Calculate the smallest rectangle that contains the current Item
      //and the one that got the autoselect event
      TQRect r;
      TQRect redraw;
      if ( previousItem )
	r = TQRect( QMIN( previousItem->x(), m_pCurrentItem->x() ),
		   QMIN( previousItem->y(), m_pCurrentItem->y() ),
		   0, 0 );
      else
	r = TQRect( 0, 0, 0, 0 );
      if ( previousItem->x() < m_pCurrentItem->x() )
	r.setWidth( m_pCurrentItem->x() - previousItem->x() + m_pCurrentItem->width() );
      else
	r.setWidth( previousItem->x() - m_pCurrentItem->x() + previousItem->width() );
      if ( previousItem->y() < m_pCurrentItem->y() )
	r.setHeight( m_pCurrentItem->y() - previousItem->y() + m_pCurrentItem->height() );
      else
	r.setHeight( previousItem->y() - m_pCurrentItem->y() + previousItem->height() );
      r = r.normalize();

      //Check for each item whether it is within the rectangle.
      //If yes, select it
      for( TQIconViewItem* i = firstItem(); i; i = i->nextItem() ) {
	if( i->intersects( r ) ) {
	  redraw = redraw.unite( i->rect() );
	  setSelected( i, select, true );
	}
      }

      blockSignals( block );
      viewport()->tqsetUpdatesEnabled( update );
      repaintContents( redraw, false );

      emit selectionChanged();

      if( selectionMode() == TQIconView::Single )
	emit selectionChanged( m_pCurrentItem );

      //setSelected( m_pCurrentItem, true, (keybstate & ControlButton), (keybstate & ShiftButton) );
    }
    else if( (keybstate & ControlButton) )
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected(), true );
    else
      setSelected( m_pCurrentItem, true );
  }
  else
    kdDebug() << "KIconView: That's not supposed to happen!!!!" << endl;
}

void KIconView::emitExecute( TQIconViewItem *item, const TQPoint &pos )
{
  if ( d->mode != Execute )
  {
    // kdDebug() << "KIconView::emitExecute : not in execute mode !" << endl;
    return;
  }

  ButtonState keybstate = KApplication::keyboardMouseState();

  m_pAutoSelect->stop();

  //Don�t emit executed if in SC mode and Shift or Ctrl are pressed
  if( !( m_bUseSingle && ((keybstate & ShiftButton) || (keybstate & ControlButton)) ) ) {
    setSelected( item, false );
    viewport()->unsetCursor();
    emit executed( item );
    emit executed( item, pos );
  }
}

void KIconView::updateDragHoldItem( TQDropEvent *e )
{
  TQIconViewItem *item = tqfindItem( e->pos() );

  if ( d->dragHoldItem != item)
  {
    d->dragHoldItem = item;
    if( item  )
    {
      d->dragHoldTimer.start( 1000, true );
    }
    else
    {
      d->dragHoldTimer.stop();
    }
  }
}

void KIconView::focusOutEvent( TQFocusEvent *fe )
{
  m_pAutoSelect->stop();

  TQIconView::focusOutEvent( fe );
}

void KIconView::leaveEvent( TQEvent *e )
{
  m_pAutoSelect->stop();

  TQIconView::leaveEvent( e );
}

void KIconView::contentsMousePressEvent( TQMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  TQIconView::contentsMousePressEvent( e );
  d->doAutoSelect = false;
}

void KIconView::contentsMouseDoubleClickEvent ( TQMouseEvent * e )
{
  TQIconView::contentsMouseDoubleClickEvent( e );

  TQIconViewItem* item = tqfindItem( e->pos() );

  if( item ) {
    if( (e->button() == Qt::LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );

    emit doubleClicked( item, e->globalPos() );
  }
  d->doubleClickIgnoreTimer.start(0, true);
}

void KIconView::slotMouseButtonClicked( int btn, TQIconViewItem *item, const TQPoint &pos )
{
  //kdDebug() << " KIconView::slotMouseButtonClicked() item=" << item << endl;
  if( d->doubleClickIgnoreTimer.isActive() )
    return; // Ignore double click
    
  if( (btn == Qt::LeftButton) && item )
    emitExecute( item, pos );
}

void KIconView::contentsMouseReleaseEvent( TQMouseEvent *e )
{
    d->doAutoSelect = true;
    TQIconView::contentsMouseReleaseEvent( e );
}

void KIconView::contentsDragEnterEvent( TQDragEnterEvent *e )
{
    updateDragHoldItem( e );
    TQIconView::contentsDragEnterEvent( e );
}

void KIconView::contentsDragLeaveEvent( TQDragLeaveEvent *e )
{
    d->dragHoldTimer.stop();
    d->dragHoldItem = 0L;
    TQIconView::contentsDragLeaveEvent( e );
}


void KIconView::contentsDragMoveEvent( TQDragMoveEvent *e )
{
    updateDragHoldItem( e );
    TQIconView::contentsDragMoveEvent( e );
}

void KIconView::contentsDropEvent( TQDropEvent* e )
{
    d->dragHoldTimer.stop();
    TQIconView::contentsDropEvent( e );
}

void KIconView::slotDragHoldTimeout()
{
    TQIconViewItem *tmp = d->dragHoldItem;
    d->dragHoldItem = 0L;

    emit held( tmp );
}

void KIconView::takeItem( TQIconViewItem * item )
{
    if ( item == d->dragHoldItem )
    {
        d->dragHoldTimer.stop();
        d->dragHoldItem = 0L;
    }

    TQIconView::takeItem( item );
}

void KIconView::cancelPendingHeldSignal()
{
    d->dragHoldTimer.stop();
    d->dragHoldItem = 0L;
}

void KIconView::wheelEvent( TQWheelEvent *e )
{
    if (horizontalScrollBar() && (arrangement() == TQIconView::TopToBottom)) {
        TQWheelEvent ce(e->pos(), e->delta(), e->state(), Qt::Horizontal);
        TQApplication::sendEvent( horizontalScrollBar(), &ce);
	if (ce.isAccepted()) {
            e->accept();
	    return;
	}
    }
    TQIconView::wheelEvent(e);
}

void KIconView::setFont( const TQFont &font )
{
    delete d->fm;
    d->fm = 0L;
    TQIconView::setFont( font );
}

TQFontMetrics *KIconView::itemFontMetrics() const
{
    if (!d->fm) {
        // TQIconView creates one too, but we can't access it
        d->fm = new TQFontMetrics( font() );
    }
    return d->fm;
}

TQPixmap KIconView::selectedIconPixmap( TQPixmap *pix, const TQColor &col ) const
{
    TQPixmap m;
    if ( d->maskCache.tqfind( TQString::number( pix->serialNumber() ), m ) )
	return m;
    m = KPixmapEffect::selectedPixmap( KPixmap(*pix), col );
    d->maskCache.insert( TQString::number( pix->serialNumber() ), m );
    return m;
}

int KIconView::iconTextHeight() const
{
    return d->textHeight > 0 ? d->textHeight : ( wordWrapIconText() ? 99 : 1 );
}

void KIconView::setIconTextHeight( int n )
{
    int oldHeight = iconTextHeight();
    if ( n > 1 )
        d->textHeight = n;
    else
        d->textHeight = 1;

    // so that Qt still shows the tooltip when even a wrapped text is too long
    setWordWrapIconText( false );

    // update view if needed
    if ( iconTextHeight() != oldHeight )
        setFont( font() );  // hack to recalc items
}

/////////////

struct KIconViewItem::KIconViewItemPrivate
{
    TQSize m_pixmapSize;
};

void KIconViewItem::init()
{
    m_wordWrap = 0L;
    d = 0L;
    calcRect();
}

KIconViewItem::~KIconViewItem()
{
    delete m_wordWrap;
    delete d;
}

void KIconViewItem::calcRect( const TQString& text_ )
{
    bool drawRoundedRect = KGlobalSettings::iconUseRoundedRect();

    Q_ASSERT( iconView() );
    if ( !iconView() )
        return;
    delete m_wordWrap;
    m_wordWrap = 0L;
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !iconView()->inherits("KIconView") )
    {
        kdWarning() << "KIconViewItem used in a " << iconView()->className() << " !!" << endl;
        return;
    }
#endif
    //kdDebug() << "KIconViewItem::calcRect - " << text() << endl;
    KIconView *view = static_cast<KIconView *>(iconView());
    TQRect itemIconRect = pixmapRect();
    TQRect itemTextRect = textRect();
    TQRect tqitemRect = rect();

    int pw = 0;
    int ph = 0;

#ifndef QT_NO_PICTURE
    if ( picture() ) {
        TQRect br = picture()->boundingRect();
        pw = br.width() + 2;
        ph = br.height() + 2;
    } else
#endif
    {
        // Qt uses unknown_icon if no pixmap. Let's see if we need that - I doubt it
        if (!pixmap())
            return;
        pw = pixmap()->width() + 2;
        ph = pixmap()->height() + 2;
    }
    itemIconRect.setWidth( pw );
#if 1 // FIXME 
    // There is a bug in Qt which prevents the item from being placed
    // properly when the pixmapRect is not at the top of the tqitemRect, so we
    // have to increase the height of the pixmapRect and leave it at the top
    // of the tqitemRect...
    if ( d && !d->m_pixmapSize.isNull() )
        itemIconRect.setHeight( d->m_pixmapSize.height() + 2 );
    else
#endif
    itemIconRect.setHeight( ph );

    int tw = 0;
    if ( d && !d->m_pixmapSize.isNull() )
        tw = view->maxItemWidth() - ( view->itemTextPos() == TQIconView::Bottom ? 0 :
                                      d->m_pixmapSize.width() + 2 );
    else
        tw = view->maxItemWidth() - ( view->itemTextPos() == TQIconView::Bottom ? 0 :
                                      itemIconRect.width() );
    
    TQFontMetrics *fm = view->itemFontMetrics();
    TQString t;
    TQRect r;
    
    // When is text_ set ? Doesn't look like it's ever set.
    t = text_.isEmpty() ? text() : text_;
    
    // Max text height
    int nbLines = static_cast<KIconView*>( iconView() )->iconTextHeight();
    int height = nbLines > 0 ? fm->height() * nbLines : 0xFFFFFFFF;
    
    // Should not be higher than pixmap if text is alongside icons
    if ( view->itemTextPos() != TQIconView::Bottom ) {
        if ( d && !d->m_pixmapSize.isNull() )
            height = QMIN( d->m_pixmapSize.height() + 2, height );
        else
            height = QMIN( itemIconRect.height(), height );
        height = QMAX( height, fm->height() );
    }
    
    // Calculate the word-wrap
    TQRect outerRect( 0, 0, tw - 6, height );
    m_wordWrap = KWordWrap::formatText( *fm, outerRect, 0, t );
    r = m_wordWrap->boundingRect();

    int realWidth = QMAX( QMIN( r.width() + 4, tw ), fm->width( "X" ) );
    if (drawRoundedRect == true)
      itemTextRect.setWidth( realWidth + 2);
    else
      itemTextRect.setWidth( realWidth );
    itemTextRect.setHeight( r.height() );

    int w = 0;    int h = 0;    int y = 0;
    if ( view->itemTextPos() == TQIconView::Bottom ) {
        // If the pixmap size has been specified, use it
        if ( d && !d->m_pixmapSize.isNull() )
        {
            w = QMAX( itemTextRect.width(), d->m_pixmapSize.width() + 2 );
            h = itemTextRect.height() + d->m_pixmapSize.height() + 2 + 1;
#if 0 // FIXME 
            // Waiting for the qt bug to be solved, the pixmapRect must
            // stay on the top...
            y = d->m_pixmapSize.height() + 2 - itemIconRect.height();
#endif
        }
        else {
            w = QMAX( itemTextRect.width(), itemIconRect.width() );
            h = itemTextRect.height() + itemIconRect.height() + 1;
        }

        tqitemRect.setWidth( w );
        tqitemRect.setHeight( h );
        int width = QMAX( w, TQApplication::globalStrut().width() ); // see TQIconViewItem::width()
        int height = QMAX( h, TQApplication::globalStrut().height() ); // see TQIconViewItem::height()
        itemTextRect = TQRect( ( width - itemTextRect.width() ) / 2, height - itemTextRect.height(),
                              itemTextRect.width(), itemTextRect.height() );
        itemIconRect = TQRect( ( width - itemIconRect.width() ) / 2, y,
                              itemIconRect.width(), itemIconRect.height() );
    } else {
        // If the pixmap size has been specified, use it
        if ( d && !d->m_pixmapSize.isNull() )
        {
            h = QMAX( itemTextRect.height(), d->m_pixmapSize.height() + 2 );
#if 0 // FIXME 
            // Waiting for the qt bug to be solved, the pixmapRect must
            // stay on the top...
            y = ( d->m_pixmapSize.height() + 2 - itemIconRect.height() ) / 2;
#endif
        }
        else
            h = QMAX( itemTextRect.height(), itemIconRect.height() );
        w = itemTextRect.width() + itemIconRect.width() + 1;

        tqitemRect.setWidth( w );
        tqitemRect.setHeight( h );
        int width = QMAX( w, TQApplication::globalStrut().width() ); // see TQIconViewItem::width()
        int height = QMAX( h, TQApplication::globalStrut().height() ); // see TQIconViewItem::height()

        itemTextRect = TQRect( width - itemTextRect.width(), ( height - itemTextRect.height() ) / 2,
                              itemTextRect.width(), itemTextRect.height() );
        if ( itemIconRect.height() > itemTextRect.height() ) // icon bigger than text -> center vertically
            itemIconRect = TQRect( 0, ( height - itemIconRect.height() ) / 2,
                                  itemIconRect.width(), itemIconRect.height() );
        else // icon smaller than text -> place in top or center with first line
	    itemIconRect = TQRect( 0, QMAX(( fm->height() - itemIconRect.height() ) / 2 + y, 0),
                                  itemIconRect.width(), itemIconRect.height() );
        if ( ( itemIconRect.height() <= 20 ) && ( itemTextRect.height() < itemIconRect.height() ) )
        {
            itemTextRect.setHeight( itemIconRect.height() - 2 );
            itemTextRect.setY( itemIconRect.y() );
        }
    }

    if ( itemIconRect != pixmapRect() )
        setPixmapRect( itemIconRect );
    if ( itemTextRect != textRect() )
        setTextRect( itemTextRect );
    if ( tqitemRect != rect() )
        setItemRect( tqitemRect );

    // Done by setPixmapRect, setTextRect and setItemRect !  [and useless if no rect changed]
    //view->updateItemContainer( this );

}

void KIconViewItem::paintItem( TQPainter *p, const TQColorGroup &cg )
{
    TQIconView* view = iconView();
    Q_ASSERT( view );
    if ( !view )
        return;
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !view->inherits("KIconView") )
    {
        kdWarning() << "KIconViewItem used in a " << view->className() << " !!" << endl;
        return;
    }
#endif

    p->save();

    paintPixmap(p, cg);
    paintText(p, cg);

    p->restore();
}

KWordWrap * KIconViewItem::wordWrap()
{
    return m_wordWrap;
}

void KIconViewItem::paintPixmap( TQPainter *p, const TQColorGroup &cg )
{
    KIconView *kview = static_cast<KIconView *>(iconView());

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	TQPicture *pic = picture();
	if ( isSelected() ) {
            // TODO something as nice as selectedIconPixmap if possible ;)
	    p->fillRect( pixmapRect( false ), TQBrush( cg.highlight(), TQBrush::Dense4Pattern) );
	}
	p->drawPicture( x()-pic->boundingRect().x(), y()-pic->boundingRect().y(), *pic );
    } else
#endif
    {
        int iconX = pixmapRect( false ).x();
        int iconY = pixmapRect( false ).y();

        TQPixmap *pix = pixmap();
        if ( !pix || pix->isNull() )
            return;

#if 1 // FIXME 
        // Move the pixmap manually because the pixmapRect is at the
        // top of the tqitemRect
        // (won't be needed anymore in future versions of qt)
        if ( d && !d->m_pixmapSize.isNull() )
        {
            int offset = 0;
            if ( kview->itemTextPos() == TQIconView::Bottom )
                offset = d->m_pixmapSize.height() - pix->height();
            else
                offset = ( d->m_pixmapSize.height() - pix->height() ) / 2;
            if ( offset > 0 )
                iconY += offset;
        }
#endif
        if ( isSelected() ) {
            TQPixmap selectedPix = kview->selectedIconPixmap( pix, cg.highlight() );
            p->drawPixmap( iconX, iconY, selectedPix );
        } else {
            p->drawPixmap( iconX, iconY, *pix );
        }
    }
}

void KIconViewItem::paintText( TQPainter *p, const TQColorGroup &cg )
{
    bool drawRoundedRect = KGlobalSettings::iconUseRoundedRect();
    int textX;
    if (drawRoundedRect == true)
      textX = textRect( false ).x() + 4;
    else
      textX = textRect( false ).x() + 2;
    int textY = textRect( false ).y();

    if ( isSelected() ) {
      if (drawRoundedRect == true) {
	p->setBrush(TQBrush(cg.highlight()));
	p->setPen(TQPen(cg.highlight()));
	p->drawRoundRect( textRect( false ) ,1000/textRect(false).width(),1000/textRect(false).height() );
      }
      else {
        p->fillRect( textRect( false ), cg.highlight() );
      }
        p->setPen( TQPen( cg.highlightedText() ) );
    } else {
        if ( iconView()->itemTextBackground() != Qt::NoBrush )
            p->fillRect( textRect( false ), iconView()->itemTextBackground() );
        p->setPen( cg.text() );
    }

    int align = iconView()->itemTextPos() == TQIconView::Bottom ? AlignHCenter : AlignAuto;
    m_wordWrap->drawText( p, textX, textY, align | KWordWrap::Truncate );
}

TQSize KIconViewItem::pixmapSize() const
{
    return d ? d->m_pixmapSize : TQSize( 0, 0 );
}

void KIconViewItem::setPixmapSize( const TQSize& size )
{
    if ( !d )
        d = new KIconViewItemPrivate;

    d->m_pixmapSize = size;
}

void KIconView::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kiconview.moc"
