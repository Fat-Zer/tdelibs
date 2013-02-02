/* This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <assert.h>
#include <stdlib.h>

#include <tdeaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "config-tdefile.h"
#include "tdefileview.h"

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

TQDir::SortSpec KFileView::defaultSortSpec = static_cast<TQDir::SortSpec>(TQDir::Name | TQDir::IgnoreCase | TQDir::DirsFirst);

class KFileView::KFileViewPrivate
{
public:
    KFileViewPrivate()
    {
        actions = 0;
        dropOptions = 0;
    }

    ~KFileViewPrivate()
    {
        if( actions ) {
            actions->clear(); // so that the removed() signal is emitted!
            delete actions;
        }
    }

    TQGuardedPtr<TDEActionCollection> actions;
    int dropOptions;
};


KFileView::KFileView()
{
    d = new KFileViewPrivate();
    m_sorting  = KFileView::defaultSortSpec;

    sig = new KFileViewSignaler();
    sig->setName("view-signaller");

    m_selectedList = 0L;
    filesNumber = 0;
    dirsNumber = 0;

    view_mode = All;
    selection_mode = KFile::Single;
    m_viewName = i18n("Unknown View");

    myOnlyDoubleClickSelectsFiles = false;
    m_itemList.setAutoDelete( false ); // just references
}

KFileView::~KFileView()
{
    delete d;
    delete sig;
    delete m_selectedList;
}

void KFileView::setParentView(KFileView *parent)
{
    if ( parent ) { // pass all signals right to our parent
        TQObject::connect(sig, TQT_SIGNAL( activatedMenu(const KFileItem *,
                                                        const TQPoint& ) ),
                parent->sig, TQT_SIGNAL( activatedMenu(const KFileItem *,
                                                        const TQPoint& )));
        TQObject::connect(sig, TQT_SIGNAL( dirActivated(const KFileItem *)),
                parent->sig, TQT_SIGNAL( dirActivated(const KFileItem*)));
        TQObject::connect(sig, TQT_SIGNAL( fileSelected(const KFileItem *)),
                parent->sig, TQT_SIGNAL( fileSelected(const KFileItem*)));
        TQObject::connect(sig, TQT_SIGNAL( fileHighlighted(const KFileItem *) ),
                            parent->sig,TQT_SIGNAL(fileHighlighted(const KFileItem*)));
        TQObject::connect(sig, TQT_SIGNAL( sortingChanged( TQDir::SortSpec ) ),
                            parent->sig, TQT_SIGNAL(sortingChanged( TQDir::SortSpec)));
        TQObject::connect(sig, TQT_SIGNAL( dropped(const KFileItem *, TQDropEvent*, const KURL::List&) ),
                            parent->sig, TQT_SIGNAL(dropped(const KFileItem *, TQDropEvent*, const KURL::List&)));
    }
}

bool KFileView::updateNumbers(const KFileItem *i)
{
    if (!( viewMode() & Files ) && i->isFile())
        return false;

    if (!( viewMode() & Directories ) && i->isDir())
        return false;

    if (i->isDir())
        dirsNumber++;
    else
        filesNumber++;

    return true;
}

void qt_qstring_stats();

// filter out files if we're in directory mode and count files/directories
// and insert into the view
void KFileView::addItemList(const KFileItemList& list)
{
    KFileItem *tmp;

    for (KFileItemListIterator it(list); (tmp = it.current()); ++it)
    {
        if (!updateNumbers(tmp))
            continue;

        insertItem( tmp );
    }

#ifdef Q2HELPER
    qt_qstring_stats();
#endif
}

void KFileView::insertItem( KFileItem * )
{
}

void KFileView::setSorting(TQDir::SortSpec new_sort)
{
    m_sorting = new_sort;
}

void KFileView::clear()
{
    m_itemList.clear();
    filesNumber = 0;
    dirsNumber = 0;
    clearView();
}

void KFileView::sortReversed()
{
    int spec = sorting();

    setSorting( static_cast<TQDir::SortSpec>( spec ^ TQDir::Reversed ) );
}

#if 0
int KFileView::compareItems(const KFileItem *fi1, const KFileItem *fi2) const
{
    static const TQString &dirup = TDEGlobal::staticQString("..");
    bool bigger = true;
    bool keepFirst = false;
    bool dirsFirst = ((m_sorting & TQDir::DirsFirst) == TQDir::DirsFirst);

    if (fi1 == fi2)
	return 0;

    // .. is always bigger, independent of the sort criteria
    if ( fi1->name() == dirup ) {
	bigger = false;
	keepFirst = dirsFirst;
    }
    else if ( fi2->name() == dirup ) {
	bigger = true;
	keepFirst = dirsFirst;
    }

    else {
	if ( fi1->isDir() != fi2->isDir() && dirsFirst ) {
	    bigger = fi2->isDir();
	    keepFirst = true;
	}
	else {

	    TQDir::SortSpec sort = static_cast<TQDir::SortSpec>(m_sorting & TQDir::SortByMask);

	    //if (fi1->isDir() || fi2->isDir())
            // sort = static_cast<TQDir::SortSpec>(KFileView::defaultSortSpec & TQDir::SortByMask);

            switch (sort) {
                case TQDir::Name:
                default:
sort_by_name:
                    if ( (m_sorting & TQDir::IgnoreCase) == TQDir::IgnoreCase )
                        bigger = (fi1->name( true ) > fi2->name( true ));
                    else
                        bigger = (fi1->name() > fi2->name());
                    break;
                case TQDir::Time:
                {
                    time_t t1 = fi1->time( TDEIO::UDS_MODIFICATION_TIME );
                    time_t t2 = fi2->time( TDEIO::UDS_MODIFICATION_TIME );
                    if ( t1 != t2 ) {
                        bigger = (t1 > t2);
                        break;
                    }

                    // Sort by name if both items have the same timestamp.
                    // Don't honor the reverse flag tho.
                    else {
                        keepFirst = true;
                        goto sort_by_name;
                    }
                }
                case TQDir::Size:
                {
                    TDEIO::filesize_t s1 = fi1->size();
                    TDEIO::filesize_t s2 = fi2->size();
                    if ( s1 != s2 ) {
                        bigger = (s1 > s2);
                        break;
                    }

                    // Sort by name if both items have the same size.
                    // Don't honor the reverse flag tho.
                    else {
                        keepFirst = true;
                        goto sort_by_name;
                    }
                }
                case TQDir::Unsorted:
                    bigger = true;  // nothing
                    break;
	    }
	}
    }

    if (reversed && !keepFirst ) // don't reverse dirs to the end!
      bigger = !bigger;

    return (bigger ? 1 : -1);
}
#endif

void  KFileView::updateView(bool f)
{
    widget()->repaint(f);
}

void KFileView::updateView(const KFileItem *)
{
}

void KFileView::setCurrentItem(const TQString &filename )
{
    if (!filename.isNull()) {
        KFileItem *item;
        for ( (item = firstFileItem()); item; item = nextItem( item ) ) {
            if (item->name() == filename) {
                setCurrentItem( item );
                return;
            }
        }
    }

    kdDebug(tdefile_area) << "setCurrentItem: no match found: " << filename << endl;
}

const KFileItemList * KFileView::items() const
{
    KFileItem *item = 0L;

    // only ever use m_itemList in this method!
    m_itemList.clear();
    for ( (item = firstFileItem()); item; item = nextItem( item ) )
        m_itemList.append( item );

    return &m_itemList;
}


const KFileItemList * KFileView::selectedItems() const
{
    if ( !m_selectedList )
	m_selectedList = new KFileItemList;

    m_selectedList->clear();

    KFileItem *item;
    for ( (item = firstFileItem()); item; item = nextItem( item ) ) {
        if ( isSelected( item ) )
            m_selectedList->append( item );
    }

    return m_selectedList;
}

void KFileView::selectAll()
{
    if (selection_mode == KFile::NoSelection || selection_mode== KFile::Single)
        return;

    KFileItem *item = 0L;
    for ( (item = firstFileItem()); item; item = nextItem( item ) )
        setSelected( item, true );
}


void KFileView::invertSelection()
{
    KFileItem *item = 0L;
    for ( (item = firstFileItem()); item; item = nextItem( item ) )
        setSelected( item, !isSelected( item ) );
}


void KFileView::setSelectionMode( KFile::SelectionMode sm )
{
    selection_mode = sm;
}

KFile::SelectionMode KFileView::selectionMode() const
{
    return selection_mode;
}

void KFileView::setViewMode( ViewMode vm )
{
    view_mode = vm;
}

void KFileView::removeItem( const KFileItem *item )
{
    if ( !item )
	return;

    if ( item->isDir() )
        dirsNumber--;
    else
        filesNumber--;

    if ( m_selectedList )
        m_selectedList->removeRef( item );
}

void KFileView::listingCompleted()
{
    // empty default impl.
}

TDEActionCollection * KFileView::actionCollection() const
{
    if ( !d->actions )
        d->actions = new TDEActionCollection( widget(), "KFileView::d->actions" );
    return d->actions;
}

void KFileView::readConfig( TDEConfig *, const TQString&  )
{
}

void KFileView::writeConfig( TDEConfig *, const TQString& )
{
}

TQString KFileView::sortingKey( const TQString& value, bool isDir, int sortSpec )
{
    bool reverse   = sortSpec & TQDir::Reversed;
    bool dirsFirst = sortSpec & TQDir::DirsFirst;
    char start = (isDir && dirsFirst) ? (reverse ? '2' : '0') : '1';
    TQString result = (sortSpec & TQDir::IgnoreCase) ? value.lower() : value;
    return result.prepend( start );
}

TQString KFileView::sortingKey( TDEIO::filesize_t value, bool isDir, int sortSpec)
{
    bool reverse = sortSpec & TQDir::Reversed;
    bool dirsFirst = sortSpec & TQDir::DirsFirst;
    char start = (isDir && dirsFirst) ? (reverse ? '2' : '0') : '1';
    return TDEIO::number( value ).rightJustify( 24, '0' ).prepend( start );
}

void KFileView::setDropOptions(int options)
{
    virtual_hook(VIRTUAL_SET_DROP_OPTIONS, &options); // Virtual call
}

void KFileView::setDropOptions_impl(int options)
{
    d->dropOptions = options;
}

int KFileView::dropOptions()
{
    return d->dropOptions;
}

int KFileView::autoOpenDelay()
{
    return (TQApplication::startDragTime() * 3) / 2;
}

void KFileView::virtual_hook( int id, void* data)
{ 
    switch(id) {
      case VIRTUAL_SET_DROP_OPTIONS: 
         setDropOptions_impl(*(int *)data);
         break;
      default:
         /*BASE::virtual_hook( id, data );*/
         break;
    }
}

#include "tdefileview.moc"
