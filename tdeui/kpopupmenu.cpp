/* This file is part of the KDE libraries
   Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>
   Copyright (C) 2002 Hamish Rodda <rodda@kde.org>

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
#include <tqcursor.h>
#include <tqpainter.h>
#include <tqtimer.h>
#include <tqfontmetrics.h>

#ifdef USE_QT4
#undef None
#endif // USE_QT4

#include <tqstyle.h>

#include "kpopupmenu.h"

#include <kdebug.h>
#include <kapplication.h>

KPopupTitle::KPopupTitle(TQWidget *parent, const char *name)
    : TQWidget(parent, name)
{
    setMinimumSize(16, fontMetrics().height()+8);
}

KPopupTitle::KPopupTitle(KPixmapEffect::GradientType /* gradient */,
                         const TQColor &/* color */, const TQColor &/* textColor */,
                         TQWidget *parent, const char *name)
   : TQWidget(parent, name)
{
    calcSize();
}

KPopupTitle::KPopupTitle(const KPixmap & /* background */, const TQColor &/* color */,
                         const TQColor &/* textColor */, TQWidget *parent,
                         const char *name)
    : TQWidget(parent, name)
{
    calcSize();
}

void KPopupTitle::setTitle(const TQString &text, const TQPixmap *icon)
{
    titleStr = text;
    if (icon)
        miniicon = *icon;
    else
        miniicon.resize(0, 0);

    calcSize();
}

void KPopupTitle::setText( const TQString &text )
{
    titleStr = text;
    calcSize();
}

void KPopupTitle::setIcon( const TQPixmap &pix )
{
    miniicon = pix;
    calcSize();
}

void KPopupTitle::calcSize()
{
    TQFont f = font();
    f.setBold(true);
    int w = miniicon.width()+TQFontMetrics(f).width(titleStr);
    int h = QMAX( fontMetrics().height(), miniicon.height() );
    setMinimumSize( w+16, h+8 );
}

void KPopupTitle::paintEvent(TQPaintEvent *)
{
    TQRect r(rect());
    TQPainter p(this);
    kapp->style().tqdrawPrimitive(TQStyle::PE_HeaderSectionMenu, &p, r, palette().active());

    if (!miniicon.isNull())
        p.drawPixmap(4, (r.height()-miniicon.height())/2, miniicon);

    if (!titleStr.isNull())
    {
        p.setPen(palette().active().text());
        TQFont f = p.font();
        f.setBold(true);
        p.setFont(f);
        if(!miniicon.isNull())
        {
            p.drawText(miniicon.width()+8, 0, width()-(miniicon.width()+8),
                       height(), AlignLeft | AlignVCenter | SingleLine,
                       titleStr);
        }
        else
        {
            p.drawText(0, 0, width(), height(),
                       AlignCenter | SingleLine, titleStr);
        }
    }
}

TQSize KPopupTitle::sizeHint() const
{
    return minimumSize();
}

class KPopupMenu::KPopupMenuPrivate
{
public:
    KPopupMenuPrivate ()
        : noMatches(false)
        , shortcuts(false)
        , autoExec(false)
        , lastHitIndex(-1)
        , state(Qt::NoButton)
        , m_ctxMenu(0)
    {}

    ~KPopupMenuPrivate ()
    {
        delete m_ctxMenu;
    }

    TQString m_lastTitle;

    // variables for keyboard navigation
    TQTimer clearTimer;

    bool noMatches : 1;
    bool shortcuts : 1;
    bool autoExec : 1;

    TQString keySeq;
    TQString originalText;

    int lastHitIndex;
    TQt::ButtonState state;

    // support for RMB menus on menus
    TQPopupMenu* m_ctxMenu;
    static bool s_continueCtxMenuShow;
    static int s_highlightedItem;
    static KPopupMenu* s_contextedMenu;
};

int KPopupMenu::KPopupMenuPrivate::s_highlightedItem(-1);
KPopupMenu* KPopupMenu::KPopupMenuPrivate::s_contextedMenu(0);
bool KPopupMenu::KPopupMenuPrivate::s_continueCtxMenuShow(true);

KPopupMenu::KPopupMenu(TQWidget *parent, const char *name)
    : TQPopupMenu(parent, name)
{
    d = new KPopupMenuPrivate;
    resetKeyboardVars();
    connect(&(d->clearTimer), TQT_SIGNAL(timeout()), TQT_SLOT(resetKeyboardVars()));
}

KPopupMenu::~KPopupMenu()
{
    if (KPopupMenuPrivate::s_contextedMenu == this)
    {
        KPopupMenuPrivate::s_contextedMenu = 0;
        KPopupMenuPrivate::s_highlightedItem = -1;
    }

    delete d;
}

int KPopupMenu::insertTitle(const TQString &text, int id, int index)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(text);
    int ret = insertItem(titleItem, id, index);
    setItemEnabled(ret, false);
    return ret;
}

int KPopupMenu::insertTitle(const TQPixmap &icon, const TQString &text, int id,
                            int index)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(text, &icon);
    int ret = insertItem(titleItem, id, index);
    setItemEnabled(ret, false);
    return ret;
}

void KPopupMenu::changeTitle(int id, const TQString &text)
{
    TQMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            ((KPopupTitle *)item->widget())->setTitle(text);
#ifndef NDEBUG
        else
            kdWarning() << "KPopupMenu: changeTitle() called with non-title id "<< id << endl;
#endif
    }
#ifndef NDEBUG
    else
        kdWarning() << "KPopupMenu: changeTitle() called with invalid id " << id << endl;
#endif
}

void KPopupMenu::changeTitle(int id, const TQPixmap &icon, const TQString &text)
{
    TQMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            ((KPopupTitle *)item->widget())->setTitle(text, &icon);
#ifndef NDEBUG
        else
            kdWarning() << "KPopupMenu: changeTitle() called with non-title id "<< id << endl;
#endif
    }
#ifndef NDEBUG
    else
        kdWarning() << "KPopupMenu: changeTitle() called with invalid id " << id << endl;
#endif
}

TQString KPopupMenu::title(int id) const
{
    if(id == -1) // obsolete
        return d->m_lastTitle;
    TQMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            return ((KPopupTitle *)item->widget())->title();
        else
            qWarning("KPopupMenu: title() called with non-title id %d.", id);
    }
    else
        qWarning("KPopupMenu: title() called with invalid id %d.", id);
    return TQString::null;
}

TQPixmap KPopupMenu::titlePixmap(int id) const
{
    TQMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            return ((KPopupTitle *)item->widget())->icon();
        else
            qWarning("KPopupMenu: titlePixmap() called with non-title id %d.", id);
    }
    else
        qWarning("KPopupMenu: titlePixmap() called with invalid id %d.", id);
    TQPixmap tmp;
    return tmp;
}

/**
 * This is re-implemented for keyboard navigation.
 */
void KPopupMenu::closeEvent(TQCloseEvent*e)
{
    if (d->shortcuts)
        resetKeyboardVars();
    TQPopupMenu::closeEvent(e);
}

void KPopupMenu::activateItemAt(int index)
{
    d->state = Qt::NoButton;
    TQPopupMenu::activateItemAt(index);
}

TQt::ButtonState KPopupMenu::state() const
{
    return d->state;
}

void KPopupMenu::keyPressEvent(TQKeyEvent* e)
{
    d->state = Qt::NoButton;
    if (!d->shortcuts) {
        // continue event processing by Qpopup
        //e->ignore();
        d->state = e->state();
        TQPopupMenu::keyPressEvent(e);
        return;
    }

    int i = 0;
    bool firstpass = true;
    TQString keyString = e->text();

    // check for common commands dealt with by QPopup
    int key = e->key();
    if (key == Key_Escape || key == Key_Return || key == Key_Enter
            || key == Key_Up || key == Key_Down || key == Key_Left
            || key == Key_Right || key == Key_F1) {

        resetKeyboardVars();
        // continue event processing by Qpopup
        //e->ignore();
        d->state = e->state();
        TQPopupMenu::keyPressEvent(e);
        return;
    } else if ( key == Key_Shift || key == Key_Control || key == Key_Alt || key == Key_Meta )
	return TQPopupMenu::keyPressEvent(e);

    // check to see if the user wants to remove a key from the sequence (backspace)
    // or clear the sequence (delete)
    if (!d->keySeq.isNull()) {

        if (key == Key_Backspace) {

            if (d->keySeq.length() == 1) {
                resetKeyboardVars();
                return;
            }

            // keep the last sequence in keyString
            keyString = d->keySeq.left(d->keySeq.length() - 1);

            // allow sequence matching to be tried again
            resetKeyboardVars();

        } else if (key == Key_Delete) {
            resetKeyboardVars();

            // clear active item
            setActiveItem(0);
            return;

        } else if (d->noMatches) {
            // clear if there are no matches
            resetKeyboardVars();

            // clear active item
            setActiveItem(0);

        } else {
            // the key sequence is not a null string
            // therefore the lastHitIndex is valid
            i = d->lastHitIndex;
        }
    } else if (key == Key_Backspace && parentMenu) {
        // backspace with no chars in the buffer... go back a menu.
        hide();
        resetKeyboardVars();
        return;
    }

    d->keySeq += keyString;
    int seqLen = d->keySeq.length();

    for (; i < (int)count(); i++) {
        // compare typed text with text of this entry
        int j = idAt(i);

        // don't search disabled entries
        if (!isItemEnabled(j))
            continue;

        TQString thisText;

        // retrieve the right text
        // (the last selected item one may have additional ampersands)
        if (i == d->lastHitIndex)
            thisText = d->originalText;
        else
            thisText = text(j);

        // if there is an accelerator present, remove it
        if ((int)accel(j) != 0)
            thisText = thisText.replace("&", TQString());

        // chop text to the search length
        thisText = thisText.left(seqLen);

        // do the search
        if (!thisText.find(d->keySeq, 0, false)) {

            if (firstpass) {
                // match
                setActiveItem(i);

                // check to see if we're underlining a different item
                if (d->lastHitIndex != i)
                    // yes; revert the underlining
                    changeItem(idAt(d->lastHitIndex), d->originalText);

                // set the original text if it's a different item
                if (d->lastHitIndex != i || d->lastHitIndex == -1)
                    d->originalText = text(j);

                // underline the currently selected item
                changeItem(j, underlineText(d->originalText, d->keySeq.length()));

                // remember what's going on
                d->lastHitIndex = i;

                // start/restart the clear timer
                d->clearTimer.start(5000, true);

                // go around for another try, to see if we can execute
                firstpass = false;
            } else {
                // don't allow execution
                return;
            }
        }

        // fall through to allow execution
    }

    if (!firstpass) {
        if (d->autoExec) {
            // activate anything
            activateItemAt(d->lastHitIndex);
            resetKeyboardVars();

        } else if (findItem(idAt(d->lastHitIndex)) &&
                 findItem(idAt(d->lastHitIndex))->popup()) {
            // only activate sub-menus
            activateItemAt(d->lastHitIndex);
            resetKeyboardVars();
        }

        return;
    }

    // no matches whatsoever, clean up
    resetKeyboardVars(true);
    //e->ignore();
    TQPopupMenu::keyPressEvent(e);
}

bool KPopupMenu::focusNextPrevChild( bool next )
{
    resetKeyboardVars();
    return TQPopupMenu::focusNextPrevChild( next );
}

TQString KPopupMenu::underlineText(const TQString& text, uint length)
{
    TQString ret = text;
    for (uint i = 0; i < length; i++) {
        if (ret[2*i] != '&')
            ret.insert(2*i, "&");
    }
    return ret;
}

void KPopupMenu::resetKeyboardVars(bool noMatches /* = false */)
{
    // Clean up keyboard variables
    if (d->lastHitIndex != -1) {
        changeItem(idAt(d->lastHitIndex), d->originalText);
        d->lastHitIndex = -1;
    }

    if (!noMatches) {
        d->keySeq = TQString::null;
    }

    d->noMatches = noMatches;
}

void KPopupMenu::setKeyboardShortcutsEnabled(bool enable)
{
    d->shortcuts = enable;
}

void KPopupMenu::setKeyboardShortcutsExecute(bool enable)
{
    d->autoExec = enable;
}
/**
 * End keyboard navigation.
 */

/**
 * RMB menus on menus
 */

void KPopupMenu::mousePressEvent(TQMouseEvent* e)
{
    if (d->m_ctxMenu && d->m_ctxMenu->isVisible())
    {
        // hide on a second context menu event
        d->m_ctxMenu->hide();
    }

    TQPopupMenu::mousePressEvent(e);
}

void KPopupMenu::mouseReleaseEvent(TQMouseEvent* e)
{
    // Save the button, and the modifiers from state()
    d->state = TQt::ButtonState(e->button() | (e->state() & KeyButtonMask));
    
    if ( !d->m_ctxMenu || !d->m_ctxMenu->isVisible() )
	TQPopupMenu::mouseReleaseEvent(e);
}

TQPopupMenu* KPopupMenu::contextMenu()
{
    if (!d->m_ctxMenu)
    {
        d->m_ctxMenu = new TQPopupMenu(this);
        connect(d->m_ctxMenu, TQT_SIGNAL(aboutToHide()), this, TQT_SLOT(ctxMenuHiding()));
    }

    return d->m_ctxMenu;
}

const TQPopupMenu* KPopupMenu::contextMenu() const
{
    return const_cast< KPopupMenu* >( this )->contextMenu();
}

void KPopupMenu::hideContextMenu()
{
    KPopupMenuPrivate::s_continueCtxMenuShow = false;
}

int KPopupMenu::contextMenuFocusItem()
{
    return KPopupMenuPrivate::s_highlightedItem;
}

KPopupMenu* KPopupMenu::contextMenuFocus()
{
    return KPopupMenuPrivate::s_contextedMenu;
}

void KPopupMenu::itemHighlighted(int /* whichItem */)
{
    if (!d->m_ctxMenu || !d->m_ctxMenu->isVisible())
    {
        return;
    }

    d->m_ctxMenu->hide();
    showCtxMenu(mapFromGlobal(TQCursor::pos()));
}

void KPopupMenu::showCtxMenu(TQPoint pos)
{
    TQMenuItem* item = findItem(KPopupMenuPrivate::s_highlightedItem);
    if (item)
    {
        TQPopupMenu* subMenu = item->popup();
        if (subMenu)
        {
            disconnect(subMenu, TQT_SIGNAL(aboutToShow()), this, TQT_SLOT(ctxMenuHideShowingMenu()));
        }
    }

    KPopupMenuPrivate::s_highlightedItem = idAt(pos);

    if (KPopupMenuPrivate::s_highlightedItem == -1)
    {
        KPopupMenuPrivate::s_contextedMenu = 0;
        return;
    }

    emit aboutToShowContextMenu(this, KPopupMenuPrivate::s_highlightedItem, d->m_ctxMenu);

    TQPopupMenu* subMenu = findItem(KPopupMenuPrivate::s_highlightedItem)->popup();
    if (subMenu)
    {
        connect(subMenu, TQT_SIGNAL(aboutToShow()), TQT_SLOT(ctxMenuHideShowingMenu()));
        TQTimer::singleShot(100, subMenu, TQT_SLOT(hide()));
    }

    if (!KPopupMenuPrivate::s_continueCtxMenuShow)
    {
        KPopupMenuPrivate::s_continueCtxMenuShow = true;
        return;
    }

    KPopupMenuPrivate::s_contextedMenu = this;
    d->m_ctxMenu->popup(this->mapToGlobal(pos));
    connect(this, TQT_SIGNAL(highlighted(int)), this, TQT_SLOT(itemHighlighted(int)));
}

/*
 * this method helps prevent submenus popping up while we have a context menu
 * showing
 */
void KPopupMenu::ctxMenuHideShowingMenu()
{
    TQMenuItem* item = findItem(KPopupMenuPrivate::s_highlightedItem);
    if (item)
    {
        TQPopupMenu* subMenu = item->popup();
        if (subMenu)
        {
            TQTimer::singleShot(0, subMenu, TQT_SLOT(hide()));
        }
    }
}

void KPopupMenu::ctxMenuHiding()
{
    if (KPopupMenuPrivate::s_highlightedItem)
    {
        TQPopupMenu* subMenu = findItem(KPopupMenuPrivate::s_highlightedItem)->popup();
        if (subMenu)
        {
            disconnect(subMenu, TQT_SIGNAL(aboutToShow()), this, TQT_SLOT(ctxMenuHideShowingMenu()));
        }
    }

    disconnect(this, TQT_SIGNAL(highlighted(int)), this, TQT_SLOT(itemHighlighted(int)));
    KPopupMenuPrivate::s_continueCtxMenuShow = true;
}

void KPopupMenu::contextMenuEvent(TQContextMenuEvent* e)
{
    if (d->m_ctxMenu)
    {
        if (e->reason() == TQContextMenuEvent::Mouse)
        {
            showCtxMenu(e->pos());
        }
        else if (actItem != -1)
        {
            showCtxMenu(itemGeometry(actItem).center());
        }

        e->accept();
        return;
    }

    TQPopupMenu::contextMenuEvent(e);
}

void KPopupMenu::hideEvent(TQHideEvent*)
{
    if (d->m_ctxMenu && d->m_ctxMenu->isVisible())
    {
        // we need to block signals here when the ctxMenu is showing
        // to prevent the TQPopupMenu::activated(int) signal from emitting
        // when hiding with a context menu, the user doesn't expect the
        // menu to actually do anything.
        // since hideEvent gets called very late in the process of hiding
        // (deep within TQWidget::hide) the activated(int) signal is the
        // last signal to be emitted, even after things like aboutToHide()
        // AJS
        blockSignals(true);
        d->m_ctxMenu->hide();
        blockSignals(false);
    }
}
/**
 * end of RMB menus on menus support
 */

// Obsolete
KPopupMenu::KPopupMenu(const TQString& title, TQWidget *parent, const char *name)
    : TQPopupMenu(parent, name)
{
    d = new KPopupMenuPrivate;
    insertTitle(title);
}

// Obsolete
void KPopupMenu::setTitle(const TQString &title)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(title);
    insertItem(titleItem);
    d->m_lastTitle = title;
}

void KPopupTitle::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KPopupMenu::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kpopupmenu.moc"
