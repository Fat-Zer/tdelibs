/* This file is part of the KDE libraries
   Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>
   Copyright (c) 2004 Gustavo Sverzut Barbieri <gsbarbieri@users.sourceforge.net>

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

/**
 * \todo
 *     Maybe we should have a common interface for SearchLines, this file
 * is so close (it's actually based on) klistviewsearchline! Only few methods
 * would be reimplemented.
 */

#include "kiconviewsearchline.h"

#include <tqiconview.h>
#include <klocale.h>
#include <tqtimer.h>
#include <kdebug.h>

#define DEFAULT_CASESENSITIVE false

typedef TQValueList <TQIconViewItem *> QIconViewItemList;

class KIconViewSearchLine::KIconViewSearchLinePrivate
{
public:
  KIconViewSearchLinePrivate() :
    iconView( 0 ),
    caseSensitive( DEFAULT_CASESENSITIVE ),
    activeSearch( false ),
    hiddenListChanged( 0 ),
    queuedSearches( 0 ) {}

  TQIconView *iconView;
  bool caseSensitive;
  bool activeSearch;
  TQString search;
  int queuedSearches;
  int hiddenListChanged;
  QIconViewItemList hiddenItems;
};

/******************************************************************************
 * Public Methods                                                             *
 *****************************************************************************/
KIconViewSearchLine::KIconViewSearchLine( TQWidget *parent,
					  TQIconView *iconView,
					  const char *name ) :
  KLineEdit( parent, name )
{
  d = NULL;
  init( iconView );
}

KIconViewSearchLine::KIconViewSearchLine( TQWidget *parent, const char *name ) :
  KLineEdit( parent, name )
{
  d = NULL;
  init( NULL );
}

KIconViewSearchLine::~KIconViewSearchLine()
{
  clear(); // empty hiddenItems, returning items back to iconView
  delete d;
}

bool KIconViewSearchLine::caseSensitive() const
{
  return d->caseSensitive;
}

TQIconView *KIconViewSearchLine::iconView() const
{
  return d->iconView;
}

/******************************************************************************
 * Public Slots                                                               *
 *****************************************************************************/
void KIconViewSearchLine::updateSearch( const TQString &s )
{
	long original_count;
	int original_hiddenListChanged;

	if( ! d->iconView )
		return; // disabled

	TQString search = d->search = s.isNull() ? text() : s;
	TQIconViewItem *currentItem = d->iconView->currentItem();

	TQIconViewItem *item = NULL;

	// Remove Non-Matching items, add them them to hidden list
	TQIconViewItem *i = d->iconView->firstItem();
	while ( i != NULL ) {
		item = i;
		i = i->nextItem(); // Point to next, otherwise will loose it.
		if ( ! itemMatches( item, search ) ) {
			hideItem( item );

			if ( item == currentItem )
				currentItem = NULL; // It's not in iconView anymore.
		}
	}

	// Add Matching items, remove from hidden list
	original_count = d->hiddenItems.count();
	original_hiddenListChanged = d->hiddenListChanged;
	for (QIconViewItemList::iterator it=d->hiddenItems.begin();it!=d->hiddenItems.end();++it) {
		item = *it;
		if ((original_count != d->hiddenItems.count()) || (original_hiddenListChanged != d->hiddenListChanged)) {
			// The list has changed; pointers are now most likely invalid
			// ABORT, but restart the search at the beginning
			original_count = d->hiddenItems.count();
			original_hiddenListChanged = d->hiddenListChanged;
			it=d->hiddenItems.begin();
		}
		else {
			if ( itemMatches( item, search ) )
				showItem( item );
		}
	}
	d->iconView->sort();

	if ( currentItem != NULL )
	d->iconView->ensureItemVisible( currentItem );
}

void KIconViewSearchLine::clear()
{
  // Clear hidden list, give items back to TQIconView, if it still exists
  TQIconViewItem *item = NULL;
  QIconViewItemList::iterator it = d->hiddenItems.begin();
  while ( it != d->hiddenItems.end() )
    {
      item = *it;
      ++it;
      if ( item != NULL )
	{
	  if ( d->iconView != NULL )
	    showItem( item );
	  else
	    delete item;
	}
    }
  if ( ! d->hiddenItems.isEmpty() )
    kdDebug() << __FILE__ << ":" << __LINE__ <<
      "hiddenItems is not empty as it should be. " <<
      d->hiddenItems.count() << " items are still there.\n" << endl;

  d->search = "";
  d->queuedSearches = 0;
  KLineEdit::clear();
}

void KIconViewSearchLine::iconDeleted(const TQString &filename) {
	// Clear hidden list, give items back to TQIconView, if it still exists
	TQIconViewItem *item = NULL;
	QIconViewItemList::iterator it = d->hiddenItems.begin();
	while ( it != d->hiddenItems.end() )
	{
		item = *it;
		++it;
		if ( item != NULL )
		{
			if (item->text() == filename) {
				if (d->iconView != NULL)
					showItem( item );
				else
					delete item;
			}
		}
	}
}

void KIconViewSearchLine::setCaseSensitive( bool cs )
{
  d->caseSensitive = cs;
}

void KIconViewSearchLine::setIconView( TQIconView *iv )
{
  if ( d->iconView != NULL )
    disconnect( d->iconView, TQT_SIGNAL( destroyed() ),
		this,        TQT_SLOT(   iconViewDeleted() ) );

  d->iconView = iv;

  if ( iv != NULL )
    {
      connect( d->iconView, TQT_SIGNAL( destroyed() ),
	       this,        TQT_SLOT(   iconViewDeleted() ) );
      setEnabled( true );
    }
  else
    setEnabled( false );
}

/******************************************************************************
 * Protected Methods                                                          *
 *****************************************************************************/
bool KIconViewSearchLine::itemMatches( const TQIconViewItem *item,
				       const TQString &s ) const
{
  if ( s.isEmpty() )
    return true;

  if ( item == NULL )
    return false;

  TQString itemtext = item->text();
  return ( itemtext.find( s, 0, caseSensitive() ) >= 0 );
}

void KIconViewSearchLine::init( TQIconView *iconView )
{
  delete d;
  d = new KIconViewSearchLinePrivate;

  d->iconView = iconView;

  connect( this, TQT_SIGNAL( textChanged( const TQString & ) ),
	   this, TQT_SLOT(   queueSearch( const TQString & ) ) );

  if ( iconView != NULL )
    {
      connect( iconView, TQT_SIGNAL( destroyed() ),
	       this,     TQT_SLOT(   iconViewDeleted() ) );
      setEnabled( true );
    }
  else
    setEnabled( false );
}

void KIconViewSearchLine::hideItem( TQIconViewItem *item )
{
  if ( ( item == NULL ) || ( d->iconView == NULL ) )
    return;

  d->hiddenListChanged++;
  d->hiddenItems.append( item );
  d->iconView->takeItem( item );
}

void KIconViewSearchLine::showItem( TQIconViewItem *item )
{
  if ( d->iconView == NULL )
    {
      kdDebug() << __FILE__ << ":" << __LINE__ <<
	"showItem() could not be called while there's no iconView set." <<
	endl;
      return;
    }
  d->hiddenListChanged++;
  d->iconView->insertItem( item );
  d->hiddenItems.remove( item );
  item->setText(item->text());
}

/******************************************************************************
 * Protected Slots                                                            *
 *****************************************************************************/
void KIconViewSearchLine::queueSearch( const TQString &s )
{
  d->queuedSearches++;
  d->search = s;
  TQTimer::singleShot( 200, this, TQT_SLOT( activateSearch() ) );
}

void KIconViewSearchLine::activateSearch()
{
  d->queuedSearches--;

  if ( d->queuedSearches <= 0 )
  {
      updateSearch( d->search );
      d->queuedSearches = 0;
  }
  else {
      TQTimer::singleShot( 200, this, TQT_SLOT( activateSearch() ) );
  }
}

/******************************************************************************
 * Private Slots                                                              *
 *****************************************************************************/
void KIconViewSearchLine::iconViewDeleted()
{
  d->iconView = NULL;
  setEnabled( false );
}

#include "kiconviewsearchline.moc"
