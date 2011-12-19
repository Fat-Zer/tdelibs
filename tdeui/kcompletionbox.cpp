/* This file is part of the KDE libraries

   Copyright (c) 2000,2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2000 Stefan Schimanski <1Stein@gmx.de>
   Copyright (c) 2000,2001,2002,2003,2004 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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


#include <tqapplication.h>
#include <tqcombobox.h>
#include <tqevent.h>
#include <tqstyle.h>

#include <kdebug.h>
#include <kconfig.h>
#include <knotifyclient.h>
#include <kglobalsettings.h>

#include "kcompletionbox.h"

class KCompletionBox::KCompletionBoxPrivate
{
public:
    TQWidget *m_parent; // necessary to set the focus back
    TQString cancelText;
    bool tabHandling;
    bool down_workaround;
    bool upwardBox;
    bool emitSelected;
};

KCompletionBox::KCompletionBox( TQWidget *parent, const char *name )
 :KListBox( parent, name, (WFlags)WType_Popup ), d(new KCompletionBoxPrivate)
{

    d->m_parent        = parent;
    d->tabHandling     = true;
    d->down_workaround = false;
    d->upwardBox       = false;
    d->emitSelected    = true;

    setColumnMode( 1 );
    setLineWidth( 1 );
    setFrameStyle( TQFrame::Box | TQFrame::Plain );

    if ( parent )
        setFocusProxy( parent );
    else
        setFocusPolicy( TQ_NoFocus );

    setVScrollBarMode( Auto );
    setHScrollBarMode( AlwaysOff );

    connect( this, TQT_SIGNAL( doubleClicked( TQListBoxItem * )),
             TQT_SLOT( slotActivated( TQListBoxItem * )) );

    // grmbl, just TQListBox workarounds :[ Thanks Volker.
    connect( this, TQT_SIGNAL( currentChanged( TQListBoxItem * )),
             TQT_SLOT( slotCurrentChanged() ));
    connect( this, TQT_SIGNAL( clicked( TQListBoxItem * )),
             TQT_SLOT( slotItemClicked( TQListBoxItem * )) );
}

KCompletionBox::~KCompletionBox()
{
    d->m_parent = 0L;
    delete d;
}

TQStringList KCompletionBox::items() const
{
    TQStringList list;

    const TQListBoxItem* currItem = firstItem();

    while (currItem) {
        list.append(currItem->text());
        currItem = currItem->next();
    }

    return list;
}

void KCompletionBox::slotActivated( TQListBoxItem *item )
{
    if ( !item )
        return;

    hide();
    emit activated( item->text() );
}

bool KCompletionBox::eventFilter( TQObject *o, TQEvent *e )
{
    int type = e->type();

    if ( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->m_parent) ) {
        if ( isVisible() ) {
            if ( type == TQEvent::KeyPress ) {
                TQKeyEvent *ev = TQT_TQKEYEVENT( e );
                switch ( ev->key() ) {
                    case Key_BackTab:
                        if ( d->tabHandling && (ev->state() == Qt::NoButton ||
                             (ev->state() & ShiftButton)) ) {
                            up();
                            ev->accept();
                            return true;
                        }
                        break;
                    case Key_Tab:
                        if ( d->tabHandling && (ev->state() == Qt::NoButton) ) {
                            down(); // Only on TAB!!
                            ev->accept();
                            return true;
                        }
                        break;
                    case Key_Down:
                        down();
                        ev->accept();
                        return true;
                    case Key_Up:
                        // If there is no selected item and we've popped up above
                        // our parent, select the first item when they press up.
                        if ( selectedItem() ||
                             mapToGlobal( TQPoint( 0, 0 ) ).y() >
                             d->m_parent->mapToGlobal( TQPoint( 0, 0 ) ).y() )
                            up();
                        else
                            down();
                        ev->accept();
                        return true;
                    case Key_Prior:
                        pageUp();
                        ev->accept();
                        return true;
                    case Key_Next:
                        pageDown();
                        ev->accept();
                        return true;
                    case Key_Escape:
                        canceled();
                        ev->accept();
                        return true;
                    case Key_Enter:
                    case Key_Return:
                        if ( ev->state() & ShiftButton ) {
                            hide();
                            ev->accept();  // Consume the Enter event
                            return true;
                        }
                        break;
                    case Key_End:
                        if ( ev->state() & ControlButton )
                        {
                            end();
                            ev->accept();
                            return true;
                        }
                    case Key_Home:
                        if ( ev->state() & ControlButton )
                        {
                            home();
                            ev->accept();
                            return true;
                        }
                    default:
                        break;
                }
            }
            else if ( type == TQEvent::AccelOverride ) {
                // Override any acceleartors that match
                // the key sequences we use here...
                TQKeyEvent *ev = TQT_TQKEYEVENT( e );
                switch ( ev->key() ) {
                    case Key_Down:
                    case Key_Up:
                    case Key_Prior:
                    case Key_Next:
                    case Key_Escape:
                    case Key_Enter:
                    case Key_Return:
                      ev->accept();
                      return true;
                      break;
                    case Key_Tab:
                    case Key_BackTab:
                        if ( ev->state() == Qt::NoButton ||
                            (ev->state() & ShiftButton))
                        {
                            ev->accept();
                            return true;
                        }
                        break;
                    case Key_Home:
                    case Key_End:
                        if ( ev->state() & ControlButton )
                        {
                            ev->accept();
                            return true;
                        }
                        break;
                    default:
                        break;
                }
            }

            // parent loses focus or gets a click -> we hide
            else if ( type == TQEvent::FocusOut || type == TQEvent::Resize ||
                      type == TQEvent::Close || type == TQEvent::Hide ||
                      type == TQEvent::Move ) {
                hide();
            }
        }
    }

    // any mouse-click on something else than "this" makes us hide
    else if ( type == TQEvent::MouseButtonPress ) {
        TQMouseEvent *ev = TQT_TQMOUSEEVENT( e );
        if ( !rect().contains( ev->pos() )) // this widget
            hide();

        if ( !d->emitSelected && currentItem() && !::tqqt_cast<TQScrollBar*>(o) )
        {
          emit highlighted( currentText() );
          hide();
          ev->accept();  // Consume the mouse click event...
          return true;
        }
    }

    return KListBox::eventFilter( o, e );
}


void KCompletionBox::popup()
{
    if ( count() == 0 )
        hide();
    else {
        ensureCurrentVisible();
        bool block = signalsBlocked();
        blockSignals( true );
        setCurrentItem( 0 );
        blockSignals( block );
        clearSelection();
        if ( !isVisible() )
            show();
        else if ( size().height() != sizeHint().height() )
            sizeAndPosition();
    }
}

void KCompletionBox::sizeAndPosition()
{
    int currentGeom = height();
    TQPoint currentPos = pos();
    TQRect geom = calculateGeometry();
    resize( geom.size() );

    int x = currentPos.x(), y = currentPos.y();
    if ( d->m_parent ) {
      if ( !isVisible() ) {
        TQRect screenSize = KGlobalSettings::desktopGeometry(d->m_parent);

        TQPoint orig = d->m_parent->mapToGlobal( TQPoint(0, d->m_parent->height()) );
        x = orig.x() + geom.x();
        y = orig.y() + geom.y();

        if ( x + width() > screenSize.right() )
            x = screenSize.right() - width();
        if (y + height() > screenSize.bottom() ) {
            y = y - height() - d->m_parent->height();
            d->upwardBox = true;
        }
      }
      else {
        // Are we above our parent? If so we must keep bottom edge anchored.
        if (d->upwardBox)
          y += (currentGeom-height());
      }
      move( x, y);
    }
}

void KCompletionBox::show()
{
    d->upwardBox = false;
    if ( d->m_parent ) {
        sizeAndPosition();
        tqApp->installEventFilter( this );
    }

    // ### we shouldn't need to call this, but without this, the scrollbars
    // are pretty b0rked.
    //triggerUpdate( true );

    // Workaround for I'm not sure whose bug - if this KCompletionBox' parent
    // is in a layout, that layout will detect inserting new child (posted
    // ChildInserted event), and will trigger relayout (post LayoutHint event).
    // TQWidget::show() sends also posted ChildInserted events for the parent,
    // and later all LayoutHint events, which causes layout updating.
    // The problem is, KCompletionBox::eventFilter() detects resizing
    // of the parent, and calls hide() - and this hide() happen in the middle
    // of show(), causing inconsistent state. I'll try to submit a Qt patch too.
    tqApp->sendPostedEvents();
    KListBox::show();
}

void KCompletionBox::hide()
{
    if ( d->m_parent )
        tqApp->removeEventFilter( this );
    d->cancelText = TQString::null;
    KListBox::hide();
}

TQRect KCompletionBox::calculateGeometry() const
{
    int x = 0, y = 0;
    int ih = itemHeight();
    int h = QMIN( 15 * ih, (int) count() * ih ) + 2*frameWidth();

    int w = (d->m_parent) ? d->m_parent->width() : KListBox::minimumSizeHint().width();
    w = QMAX( KListBox::minimumSizeHint().width(), w );

    //If we're inside a combox, Qt by default makes the dropdown
    // as wide as the combo, and gives the style a chance
    // to adjust it. Do that here as well, for consistency
    const TQObject* combo;
    if ( d->m_parent && (combo = d->m_parent->parent() ) &&
        combo->inherits(TQCOMBOBOX_OBJECT_NAME_STRING) )
    {
        const TQComboBox* cb = static_cast<const TQComboBox*>(TQT_TQWIDGET_CONST(combo));

        //Expand to the combo width
        w = QMAX( w, cb->width() );

        TQPoint parentCorner = d->m_parent->mapToGlobal(TQPoint(0, 0));
        TQPoint comboCorner  = cb->mapToGlobal(TQPoint(0, 0));

        //We need to adjust our horizontal position to also be WRT to the combo
        x += comboCorner.x() -  parentCorner.x();

        //The same with vertical one
        y += cb->height() - d->m_parent->height() +
             comboCorner.y() - parentCorner.y();

        //Ask the style to refine this a bit
        TQRect styleAdj = tqstyle().querySubControlMetrics(TQStyle::CC_ComboBox,
                                    cb, TQStyle::SC_ComboBoxListBoxPopup,
                                    TQStyleOption(x, y, w, h));
        //TQCommonStyle returns TQRect() by default, so this is what we get if the
        //style doesn't implement this
        if (!styleAdj.isNull())
            return styleAdj;

    }
    return TQRect(x, y, w, h);
}

TQSize KCompletionBox::sizeHint() const
{
    return calculateGeometry().size();
}

void KCompletionBox::down()
{
    int i = currentItem();

    if ( i == 0 && d->down_workaround ) {
        d->down_workaround = false;
        setCurrentItem( 0 );
        setSelected( 0, true );
        emit highlighted( currentText() );
    }

    else if ( i < (int) count() - 1 )
        setCurrentItem( i + 1 );
}

void KCompletionBox::up()
{
    if ( currentItem() > 0 )
        setCurrentItem( currentItem() - 1 );
}

void KCompletionBox::pageDown()
{
    int i = currentItem() + numItemsVisible();
    i = i > (int)count() - 1 ? (int)count() - 1 : i;
    setCurrentItem( i );
}

void KCompletionBox::pageUp()
{
    int i = currentItem() - numItemsVisible();
    i = i < 0 ? 0 : i;
    setCurrentItem( i );
}

void KCompletionBox::home()
{
    setCurrentItem( 0 );
}

void KCompletionBox::end()
{
    setCurrentItem( count() -1 );
}

void KCompletionBox::setTabHandling( bool enable )
{
    d->tabHandling = enable;
}

bool KCompletionBox::isTabHandling() const
{
    return d->tabHandling;
}

void KCompletionBox::setCancelledText( const TQString& text )
{
    d->cancelText = text;
}

TQString KCompletionBox::cancelledText() const
{
    return d->cancelText;
}

void KCompletionBox::canceled()
{
    if ( !d->cancelText.isNull() )
        emit userCancelled( d->cancelText );
    if ( isVisible() )
        hide();
}

class KCompletionBoxItem : public TQListBoxItem
{
public:
    //Returns true if dirty.
    bool reuse( const TQString& newText )
    {
        if ( text() == newText )
            return false;
        setText( newText );
        return true;
    }
};


void KCompletionBox::insertItems( const TQStringList& items, int index )
{
    bool block = signalsBlocked();
    blockSignals( true );
    insertStringList( items, index );
    blockSignals( block );
    d->down_workaround = true;
}

void KCompletionBox::setItems( const TQStringList& items )
{
    bool block = signalsBlocked();
    blockSignals( true );

    TQListBoxItem* item = firstItem();
    if ( !item ) {
        insertStringList( items );
    }
    else {
        //Keep track of whether we need to change anything,
        //so we can avoid a repaint for identical updates,
        //to reduce flicker
        bool dirty = false;

        TQStringList::ConstIterator it = items.constBegin();
        const TQStringList::ConstIterator itEnd = items.constEnd();

        for ( ; it != itEnd; ++it) {
            if ( item ) {
                const bool changed = ((KCompletionBoxItem*)item)->reuse( *it );
                dirty = dirty || changed;
                item = item->next();
            }
            else {
                dirty = true;
                //Inserting an item is a way of making this dirty
                insertItem( new TQListBoxText( *it ) );
            }
        }

        //If there is an unused item, mark as dirty -> less items now
        if ( item ) {
            dirty = true;
        }

        TQListBoxItem* tmp = item;
        while ( (item = tmp ) ) {
            tmp = item->next();
            delete item;
        }

        if (dirty)
            triggerUpdate( false );
    }

    if ( isVisible() && size().height() != sizeHint().height() )
        sizeAndPosition();

    blockSignals( block );
    d->down_workaround = true;
}

void KCompletionBox::slotCurrentChanged()
{
    d->down_workaround = false;
}

void KCompletionBox::slotItemClicked( TQListBoxItem *item )
{
    if ( item )
    {
        if ( d->down_workaround ) {
            d->down_workaround = false;
            emit highlighted( item->text() );
        }

        hide();
        emit activated( item->text() );
    }
}

void KCompletionBox::setActivateOnSelect(bool state)
{
    d->emitSelected = state;
}

bool KCompletionBox::activateOnSelect() const
{
    return d->emitSelected;
}

void KCompletionBox::virtual_hook( int id, void* data )
{ KListBox::virtual_hook( id, data ); }

#include "kcompletionbox.moc"
