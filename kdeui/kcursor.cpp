/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth (granroth@kde.org)

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

#ifdef KDE_USE_FINAL
#ifdef KeyRelease
#undef KeyRelease
#endif
#endif

#include <kcursor.h>

#include <tqbitmap.h>
#include <tqcursor.h>
#include <tqevent.h>
#include <tqtimer.h>
#include <tqwidget.h>

#include <kglobal.h>
#include <kconfig.h>
#include <tqscrollview.h>

#include "kcursor_private.h"

KCursor::KCursor()
{
}

TQCursor KCursor::handCursor()
{
        static TQCursor *hand_cursor = 0;

        if (!hand_cursor)
        {
                KConfig *config = KGlobal::config();
                KConfigGroupSaver saver( config, "General" );

#ifndef Q_WS_WIN // this tqmask doesn't work too well on win32
                if ( config->readEntry("handCursorStyle", "Windows") == "Windows" )
                {
                        static const unsigned char HAND_BITS[] = {
                                0x80, 0x01, 0x00, 0x40, 0x02, 0x00, 0x40, 0x02, 0x00, 0x40, 0x02,
                                0x00, 0x40, 0x02, 0x00, 0x40, 0x02, 0x00, 0x40, 0x1e, 0x00, 0x40,
                                0xf2, 0x00, 0x40, 0x92, 0x01, 0x70, 0x92, 0x02, 0x50, 0x92, 0x04,
                                0x48, 0x80, 0x04, 0x48, 0x00, 0x04, 0x48, 0x00, 0x04, 0x08, 0x00,
                                0x04, 0x08, 0x00, 0x04, 0x10, 0x00, 0x04, 0x10, 0x00, 0x04, 0x20,
                                0x00, 0x02, 0x40, 0x00, 0x02, 0x40, 0x00, 0x01, 0xc0, 0xff, 0x01};
                        static const unsigned char HAND_MASK_BITS[] = {
                                0x80, 0x01, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03,
                                0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x1f, 0x00, 0xc0,
                                0xff, 0x00, 0xc0, 0xff, 0x01, 0xf0, 0xff, 0x03, 0xf0, 0xff, 0x07,
                                0xf8, 0xff, 0x07, 0xf8, 0xff, 0x07, 0xf8, 0xff, 0x07, 0xf8, 0xff,
                                0x07, 0xf8, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xe0,
                                0xff, 0x03, 0xc0, 0xff, 0x03, 0xc0, 0xff, 0x01, 0xc0, 0xff, 0x01};
                        TQBitmap hand_bitmap(22, 22, HAND_BITS, true);
                        TQBitmap hand_tqmask(22, 22, HAND_MASK_BITS, true);
                        hand_cursor = new TQCursor(hand_bitmap, hand_tqmask, 7, 0);
                        // Hack to force TQCursor to call XCreatePixmapCursor() immediately
                        // so the bitmaps don't get pushed out of the Xcursor LRU cache.
                        hand_cursor->handle();
                }
                else
#endif //! Q_WS_WIN
                        hand_cursor = new TQCursor(PointingHandCursor);
        }

        Q_CHECK_PTR(hand_cursor);
        return *hand_cursor;
}

/* XPM */
static const char * const working_cursor_xpm[]={
"32 32 3 1",
"# c None",
"a c #000000",
". c #ffffff",
"..##############################",
".a.##########.aaaa.#############",
".aa.#########.aaaa.#############",
".aaa.#######.aaaaaa.############",
".aaaa.#####.a...a..a..##########",
".aaaaa.####a....a...aa##########",
".aaaaaa.###a...aa...aa##########",
".aaaaaaa.##a..a.....aa##########",
".aaaaaaaa.#.aa.....a..##########",
".aaaaa....##.aaaaaa.############",
".aa.aa.######.aaaa.#############",
".a.#.aa.#####.aaaa.#############",
"..##.aa.########################",
"#####.aa.#######################",
"#####.aa.#######################",
"######..########################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################"};


TQCursor KCursor::workingCursor()
{
        static TQCursor *working_cursor = 0;

        if (!working_cursor)
        {
            TQPixmap pm( const_cast< const char** >( working_cursor_xpm ));
            working_cursor = new TQCursor( pm, 1, 1 );
            // Hack to force TQCursor to call XCreatePixmapCursor() immediately
            // so the bitmaps don't get pushed out of the Xcursor LRU cache.
            working_cursor->handle();
        }

        Q_CHECK_PTR(working_cursor);
        return *working_cursor;
}

/**
 * All of the follow functions will return the Qt default for now regardless
 * of the style.  This will change at some later date
 */
TQCursor KCursor::arrowCursor()
{
    return Qt::arrowCursor;
}


TQCursor KCursor::upArrowCursor()
{
    return Qt::upArrowCursor;
}


TQCursor KCursor::crossCursor()
{
    return Qt::crossCursor;
}


TQCursor KCursor::waitCursor()
{
    return Qt::waitCursor;
}


TQCursor KCursor::ibeamCursor()
{
    return Qt::ibeamCursor;
}


TQCursor KCursor::sizeVerCursor()
{
    return Qt::sizeVerCursor;
}


TQCursor KCursor::sizeHorCursor()
{
    return Qt::sizeHorCursor;
}


TQCursor KCursor::sizeBDiagCursor()
{
    return Qt::sizeBDiagCursor;
}


TQCursor KCursor::sizeFDiagCursor()
{
    return Qt::sizeFDiagCursor;
}


TQCursor KCursor::sizeAllCursor()
{
    return Qt::sizeAllCursor;
}


TQCursor KCursor::blankCursor()
{
    return Qt::blankCursor;
}

TQCursor KCursor::whatsThisCursor()
{
    return Qt::whatsThisCursor;
}

// auto-hide cursor stuff

void KCursor::setAutoHideCursor( TQWidget *w, bool enable )
{
    setAutoHideCursor( w, enable, false );
}

void KCursor::setAutoHideCursor( TQWidget *w, bool enable,
				 bool customEventFilter )
{
    KCursorPrivate::self()->setAutoHideCursor( w, enable, customEventFilter );
}

void KCursor::autoHideEventFilter( TQObject *o, TQEvent *e )
{
    KCursorPrivate::self()->eventFilter( o, e );
}

void KCursor::setHideCursorDelay( int ms )
{
    KCursorPrivate::self()->hideCursorDelay = ms;
}

int KCursor::hideCursorDelay()
{
    return KCursorPrivate::self()->hideCursorDelay;
}

// **************************************************************************

KCursorPrivateAutoHideEventFilter::KCursorPrivateAutoHideEventFilter( TQWidget* widget )
    : m_widget( widget )
    , m_wasMouseTracking( m_widget->hasMouseTracking() )
    , m_isCursorHidden( false )
    , m_isOwnCursor( false )
{
    m_widget->setMouseTracking( true );
    connect( &m_autoHideTimer, TQT_SIGNAL( timeout() ),
             this, TQT_SLOT( hideCursor() ) );
}

KCursorPrivateAutoHideEventFilter::~KCursorPrivateAutoHideEventFilter()
{
    if( m_widget != NULL )
        m_widget->setMouseTracking( m_wasMouseTracking );
}

void KCursorPrivateAutoHideEventFilter::resetWidget()
{
    m_widget = NULL;
}

void KCursorPrivateAutoHideEventFilter::hideCursor()
{
    m_autoHideTimer.stop();

    if ( m_isCursorHidden )
        return;

    m_isCursorHidden = true;

    TQWidget* w = actualWidget();

    m_isOwnCursor = w->ownCursor();
    if ( m_isOwnCursor )
        m_oldCursor = w->cursor();

    w->setCursor( KCursor::blankCursor() );
}

void KCursorPrivateAutoHideEventFilter::unhideCursor()
{
    m_autoHideTimer.stop();

    if ( !m_isCursorHidden )
        return;

    m_isCursorHidden = false;

    TQWidget* w = actualWidget();

    if ( w->cursor().tqshape() != Qt::BlankCursor ) // someone messed with the cursor already
	return;

    if ( m_isOwnCursor )
        w->setCursor( m_oldCursor );
    else
        w->unsetCursor();
}

TQWidget* KCursorPrivateAutoHideEventFilter::actualWidget() const
{
    TQWidget* w = m_widget;

    // Is w a scrollview ? Call setCursor on the viewport in that case.
    TQScrollView * sv = dynamic_cast<TQScrollView *>( w );
    if ( sv )
        w = sv->viewport();

    return w;
}

bool KCursorPrivateAutoHideEventFilter::eventFilter( TQObject *o, TQEvent *e )
{
    Q_ASSERT( o == m_widget );

    switch ( e->type() )
    {
    case TQEvent::Create:
        // Qt steals mouseTracking on create()
        m_widget->setMouseTracking( true );
        break;
    case TQEvent::Leave:
    case TQEvent::FocusOut:
    case TQEvent::WindowDeactivate:
        unhideCursor();
        break;
    case TQEvent::KeyPress:
    case TQEvent::AccelOverride:
        hideCursor();
        break;
    case TQEvent::Enter:
    case TQEvent::FocusIn:
    case TQEvent::MouseButtonPress:
    case TQEvent::MouseButtonRelease:
    case TQEvent::MouseButtonDblClick:
    case TQEvent::MouseMove:
    case TQEvent::Show:
    case TQEvent::Hide:
    case TQEvent::Wheel:
        unhideCursor();
        if ( m_widget->hasFocus() )
            m_autoHideTimer.start( KCursorPrivate::self()->hideCursorDelay, true );
        break;
    default:
        break;
    }

    return false;
}

KCursorPrivate * KCursorPrivate::s_self = 0L;

KCursorPrivate * KCursorPrivate::self()
{
    if ( !s_self )
        s_self = new KCursorPrivate;
    // WABA: We never delete KCursorPrivate. Don't change.

    return s_self;
}

KCursorPrivate::KCursorPrivate()
{
    hideCursorDelay = 5000; // 5s default value

    KConfig *kc = KGlobal::config();
    KConfigGroupSaver ks( kc, TQString::tqfromLatin1("KDE") );
    enabled = kc->readBoolEntry(
		  TQString::tqfromLatin1("Autohiding cursor enabled"), true );
}

KCursorPrivate::~KCursorPrivate()
{
}

void KCursorPrivate::setAutoHideCursor( TQWidget *w, bool enable, bool customEventFilter )
{
    if ( !w || !enabled )
        return;

    if ( enable )
    {
        if ( m_eventFilters.tqfind( w ) != NULL )
            return;
        KCursorPrivateAutoHideEventFilter* filter = new KCursorPrivateAutoHideEventFilter( w );
        m_eventFilters.insert( w, filter );
        if ( !customEventFilter )
            w->installEventFilter( filter );
        connect( w, TQT_SIGNAL( destroyed(TQObject*) ),
                 this, TQT_SLOT( slotWidgetDestroyed(TQObject*) ) );
    }
    else
    {
        KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.take( w );
        if ( filter == NULL )
            return;
        w->removeEventFilter( filter );
        delete filter;
        disconnect( w, TQT_SIGNAL( destroyed(TQObject*) ),
                    this, TQT_SLOT( slotWidgetDestroyed(TQObject*) ) );
    }
}

bool KCursorPrivate::eventFilter( TQObject *o, TQEvent *e )
{
    if ( !enabled )
        return false;

    KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.tqfind( o );

    Q_ASSERT( filter != NULL );
    if ( filter == NULL )
        return false;

    return filter->eventFilter( o, e );
}

void KCursorPrivate::slotWidgetDestroyed( TQObject* o )
{
    KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.take( o );

    Q_ASSERT( filter != NULL );

    filter->resetWidget(); // so that dtor doesn't access it
    delete filter;
}

#include "kcursor_private.moc"
