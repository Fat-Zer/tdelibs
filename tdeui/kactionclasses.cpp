/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>

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

#include "kactionclasses.h"

#include <assert.h>
#include <ft2build.h>
#include <fontconfig/fontconfig.h>

#include <tqcursor.h>
#include <tqclipboard.h>
#include <tqfontdatabase.h>
#include <tqobjectlist.h>
#include <tqwhatsthis.h>
#include <tqtimer.h>
#include <tqfile.h>
#include <tqregexp.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kaccel.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfontcombo.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <kpopupmenu.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>

class KToggleAction::KToggleActionPrivate
{
public:
  KToggleActionPrivate()
  {
    m_checked = false;
    m_checkedGuiItem = 0;
  }

  bool m_checked;
  TQString m_exclusiveGroup;
  KGuiItem* m_checkedGuiItem;
};

KToggleAction::KToggleAction( const TQString& text, const KShortcut& cut,
                              TQObject* parent,
                              const char* name )
    : KAction( text, cut, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const TQString& text, const KShortcut& cut,
                              const TQObject* receiver, const char* slot,
                              TQObject* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const TQString& text, const TQIconSet& pix,
                              const KShortcut& cut,
                              TQObject* parent, const char* name )
  : KAction( text, pix, cut, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const TQString& text, const TQString& pix,
                              const KShortcut& cut,
                              TQObject* parent, const char* name )
 : KAction( text, pix, cut, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const TQString& text, const TQIconSet& pix,
                              const KShortcut& cut,
                              const TQObject* receiver,
                              const char* slot, TQObject* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const TQString& text, const TQString& pix,
                              const KShortcut& cut,
                              const TQObject* receiver,
                              const char* slot, TQObject* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( TQObject* parent, const char* name )
    : KAction( parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::~KToggleAction()
{
  delete d->m_checkedGuiItem;
  delete d;
}

int KToggleAction::plug( TQWidget* widget, int index )
{
  if ( !::tqqt_cast<TQPopupMenu *>( widget ) && !::tqqt_cast<KToolBar *>( widget ) )
  {
    kdWarning() << "Can not plug KToggleAction in " << widget->className() << endl;
    return -1;
  }
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;

  int _index = KAction::plug( widget, index );
  if ( _index == -1 )
    return _index;

  if ( ::tqqt_cast<KToolBar *>( widget ) ) {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    bar->setToggle( itemId( _index ), true );
    bar->setButton( itemId( _index ), isChecked() );
  }

  if ( d->m_checked )
    updateChecked( _index );

  return _index;
}

void KToggleAction::setChecked( bool c )
{
  if ( c == d->m_checked )
    return;
  //kdDebug(129) << "KToggleAction::setChecked(" << c << ") " << this << " " << name() << endl;

  d->m_checked = c;

  int len = containerCount();

  for( int i = 0; i < len; ++i )
    updateChecked( i );

  if ( c && parent() && !exclusiveGroup().isEmpty() ) {
    const TQObjectList list = parent()->childrenListObject();
    if ( !list.isEmpty() ) {
      TQObjectListIt it( list );
      for( ; it.current(); ++it ) {
          if ( ::tqqt_cast<KToggleAction *>( it.current() ) && it.current() != this &&
            static_cast<KToggleAction*>(it.current())->exclusiveGroup() == exclusiveGroup() ) {
	  KToggleAction *a = static_cast<KToggleAction*>(it.current());
	  if( a->isChecked() ) {
	    a->setChecked( false );
	    emit a->toggled( false );
	  }
        }
      }
    }
  }
}

void KToggleAction::updateChecked( int id )
{
  TQWidget *w = container( id );

  if ( ::tqqt_cast<TQPopupMenu *>( w ) ) {
    TQPopupMenu* pm = static_cast<TQPopupMenu*>(w);
    int itemId_ = itemId( id );
    if ( !d->m_checkedGuiItem )
      pm->setItemChecked( itemId_, d->m_checked );
    else {
      const KGuiItem* gui = d->m_checked ? d->m_checkedGuiItem : &guiItem();
      if ( d->m_checkedGuiItem->hasIcon() )
          pm->changeItem( itemId_, gui->iconSet( KIcon::Small ), gui->text() );
      else
          pm->changeItem( itemId_, gui->text() );

      // If the text doesn't change, then set the icon to be "pressed", otherwise
      // there is too little difference between checked and unchecked.
      if ( d->m_checkedGuiItem->text() == guiItem().text() )
           pm->setItemChecked( itemId_, d->m_checked );

      if ( !d->m_checkedGuiItem->whatsThis().isEmpty() ) // if empty, we keep the initial one
          pm->TQMenuData::setWhatsThis( itemId_, gui->whatsThis() );
      updateShortcut( pm, itemId_ );
    }
  }
  else if ( ::tqqt_cast<TQMenuBar *>( w ) ) // not handled in plug...
    static_cast<TQMenuBar*>(w)->setItemChecked( itemId( id ), d->m_checked );
  else if ( ::tqqt_cast<KToolBar *>( w ) )
  {
    TQWidget* r = static_cast<KToolBar*>( w )->getButton( itemId( id ) );
    if ( r && ::tqqt_cast<KToolBarButton *>( r ) ) {
      static_cast<KToolBar*>( w )->setButton( itemId( id ), d->m_checked );
      if ( d->m_checkedGuiItem && d->m_checkedGuiItem->hasIcon() ) {
        const KGuiItem* gui = d->m_checked ? d->m_checkedGuiItem : &guiItem();
        static_cast<KToolBar*>( w )->setButtonIconSet( itemId( id ), gui->iconSet( KIcon::Toolbar ) );
      }
    }
  }
}

void KToggleAction::slotActivated()
{
  setChecked( !isChecked() );
  KAction::slotActivated();
  emit toggled( isChecked() );
}

bool KToggleAction::isChecked() const
{
  return d->m_checked;
}

void KToggleAction::setExclusiveGroup( const TQString& name )
{
  d->m_exclusiveGroup = name;
}

TQString KToggleAction::exclusiveGroup() const
{
  return d->m_exclusiveGroup;
}

void KToggleAction::setCheckedState( const KGuiItem& checkedItem )
{
  delete d->m_checkedGuiItem;
  d->m_checkedGuiItem = new KGuiItem( checkedItem );
}

TQString KToggleAction::toolTip() const
{
  if ( d->m_checkedGuiItem && d->m_checked )
      return d->m_checkedGuiItem->toolTip();
  else
      return KAction::toolTip();
}

KRadioAction::KRadioAction( const TQString& text, const KShortcut& cut,
                            TQObject* parent, const char* name )
: KToggleAction( text, cut, parent, name )
{
}

KRadioAction::KRadioAction( const TQString& text, const KShortcut& cut,
                            const TQObject* receiver, const char* slot,
                            TQObject* parent, const char* name )
: KToggleAction( text, cut, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( const TQString& text, const TQIconSet& pix,
                            const KShortcut& cut,
                            TQObject* parent, const char* name )
: KToggleAction( text, pix, cut, parent, name )
{
}

KRadioAction::KRadioAction( const TQString& text, const TQString& pix,
                            const KShortcut& cut,
                            TQObject* parent, const char* name )
: KToggleAction( text, pix, cut, parent, name )
{
}

KRadioAction::KRadioAction( const TQString& text, const TQIconSet& pix,
                            const KShortcut& cut,
                            const TQObject* receiver, const char* slot,
                            TQObject* parent, const char* name )
: KToggleAction( text, pix, cut, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( const TQString& text, const TQString& pix,
                            const KShortcut& cut,
                            const TQObject* receiver, const char* slot,
                            TQObject* parent, const char* name )
: KToggleAction( text, pix, cut, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( TQObject* parent, const char* name )
: KToggleAction( parent, name )
{
}

void KRadioAction::slotActivated()
{
  if ( isChecked() )
  {
    const TQObject *senderObj = TQT_TQOBJECT_CONST(sender());

    if ( !senderObj || !::tqqt_cast<const KToolBarButton *>( senderObj ) )
      return;

    const_cast<KToolBarButton *>( static_cast<const KToolBarButton *>( TQT_TQWIDGET_CONST(senderObj) ) )->on( true );

    return;
  }

  KToggleAction::slotActivated();
}

class KSelectAction::KSelectActionPrivate
{
public:
  KSelectActionPrivate()
  {
    m_edit = false;
    m_menuAccelsEnabled = true;
    m_menu = 0;
    m_current = -1;
    m_comboWidth = -1;
    m_maxComboViewCount = -1;
  }
  bool m_edit;
  bool m_menuAccelsEnabled;
  TQPopupMenu *m_menu;
  int m_current;
  int m_comboWidth;
  TQStringList m_list;
  int m_maxComboViewCount;

  TQString makeMenuText( const TQString &_text )
  {
      if ( m_menuAccelsEnabled )
        return _text;
      TQString text = _text;
      uint i = 0;
      while ( i < text.length() ) {
          if ( text[ i ] == '&' ) {
              text.insert( i, '&' );
              i += 2;
          }
          else
              ++i;
      }
      return text;
  }
};

KSelectAction::KSelectAction( const TQString& text, const KShortcut& cut,
                              TQObject* parent, const char* name )
  : KAction( text, cut, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const TQString& text, const KShortcut& cut,
                              const TQObject* receiver, const char* slot,
                              TQObject* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const TQString& text, const TQIconSet& pix,
                              const KShortcut& cut,
                              TQObject* parent, const char* name )
  : KAction( text, pix, cut, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const TQString& text, const TQString& pix,
                              const KShortcut& cut,
                              TQObject* parent, const char* name )
  : KAction( text, pix, cut, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const TQString& text, const TQIconSet& pix,
                              const KShortcut& cut,
                              const TQObject* receiver,
                              const char* slot, TQObject* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const TQString& text, const TQString& pix,
                              const KShortcut& cut,
                              const TQObject* receiver,
                              const char* slot, TQObject* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( TQObject* parent, const char* name )
  : KAction( parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::~KSelectAction()
{
  assert(d);
  delete d->m_menu;
  delete d; d = 0;
}

void KSelectAction::setCurrentItem( int id )
{
    if ( id >= (int)d->m_list.count() ) {
        Q_ASSERT(id < (int)d->m_list.count());
        return;
    }

    if ( d->m_menu )
    {
        if ( d->m_current >= 0 )
            d->m_menu->setItemChecked( d->m_current, false );
        if ( id >= 0 )
            d->m_menu->setItemChecked( id, true );
    }

    d->m_current = id;

    int len = containerCount();

    for( int i = 0; i < len; ++i )
        updateCurrentItem( i );

    //    emit KAction::activated();
    //    emit activated( currentItem() );
    //    emit activated( currentText() );
}

void KSelectAction::setComboWidth( int width )
{
  if ( width < 0 )
    return;

  d->m_comboWidth=width;

  int len = containerCount();

  for( int i = 0; i < len; ++i )
    updateComboWidth( i );

}

void KSelectAction::setMaxComboViewCount( int n )
{
  d->m_maxComboViewCount = n;
}

TQPopupMenu* KSelectAction::popupMenu() const
{
	kdDebug(129) << "KAction::popupMenu()" << endl; // remove -- ellis
  if ( !d->m_menu )
  {
    d->m_menu = new KPopupMenu(0L, "KSelectAction::popupMenu()");
    setupMenu();
    if ( d->m_current >= 0 )
      d->m_menu->setItemChecked( d->m_current, true );
  }

  return d->m_menu;
}

void KSelectAction::setupMenu() const
{
    if ( !d->m_menu )
        return;
    d->m_menu->clear();

    TQStringList::ConstIterator it = d->m_list.begin();
    for( uint id = 0; it != d->m_list.end(); ++it, ++id ) {
        TQString text = *it;
        if ( !text.isEmpty() )
            d->m_menu->insertItem( d->makeMenuText( text ), this, TQT_SLOT( slotActivated( int ) ), 0, id );
        else
            d->m_menu->insertSeparator();
    }
}

void KSelectAction::changeItem( int index, const TQString& text )
{
  if ( index < 0 || index >= (int)d->m_list.count() )
  {
    kdWarning() << "KSelectAction::changeItem Index out of scope" << endl;
    return;
  }

  d->m_list[ index ] = text;

  if ( d->m_menu )
    d->m_menu->changeItem( index, d->makeMenuText( text ) );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    changeItem( i, index, text );
}

void KSelectAction::changeItem( int id, int index, const TQString& text)
{
  if ( index < 0 )
        return;

  TQWidget* w = container( id );
  if ( ::tqqt_cast<KToolBar *>( w ) )
  {
     TQWidget* r = (static_cast<KToolBar*>( w ))->getWidget( itemId( id ) );
     if ( ::tqqt_cast<TQComboBox *>( r ) )
     {
        TQComboBox *b = static_cast<TQComboBox*>( r );
        b->changeItem(text, index );
     }
  }
}

void KSelectAction::setItems( const TQStringList &lst )
{
  d->m_list = lst;
  d->m_current = -1;

  setupMenu();

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateItems( i );

  // Disable if empty and not editable
  setEnabled ( lst.count() > 0 || d->m_edit );
}

TQStringList KSelectAction::items() const
{
  return d->m_list;
}

TQString KSelectAction::currentText() const
{
  if ( currentItem() < 0 )
    return TQString::null;

  return d->m_list[ currentItem() ];
}

int KSelectAction::currentItem() const
{
  return d->m_current;
}

void KSelectAction::updateCurrentItem( int id )
{
  if ( d->m_current < 0 )
        return;

  TQWidget* w = container( id );
  if ( ::tqqt_cast<KToolBar *>( w ) ) {
    TQWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( ::tqqt_cast<TQComboBox *>( r ) ) {
      TQComboBox *b = static_cast<TQComboBox*>( r );
      b->setCurrentItem( d->m_current );
    }
  }
}

int KSelectAction::comboWidth() const
{
  return d->m_comboWidth;
}

void KSelectAction::updateComboWidth( int id )
{
  TQWidget* w = container( id );
  if ( ::tqqt_cast<KToolBar *>( w ) ) {
    TQWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( ::tqqt_cast<TQComboBox *>( r ) ) {
      TQComboBox *cb = static_cast<TQComboBox*>( r );
      cb->setMinimumWidth( d->m_comboWidth );
      cb->setMaximumWidth( d->m_comboWidth );
    }
  }
}

void KSelectAction::updateItems( int id )
{
  kdDebug(129) << "KAction::updateItems( " << id << ", lst )" << endl; // remove -- ellis
  TQWidget* w = container( id );
  if ( ::tqqt_cast<KToolBar *>( w ) ) {
    TQWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( ::tqqt_cast<TQComboBox *>( r ) ) {
      TQComboBox *cb = static_cast<TQComboBox*>( r );
      cb->clear();
      TQStringList lst = comboItems();
      TQStringList::ConstIterator it = lst.begin();
      for( ; it != lst.end(); ++it )
        cb->insertItem( *it );
      // qt caches and never recalculates the sizeHint()
      // qcombobox.cpp recommends calling setFont to invalidate the sizeHint
      // setFont sets own_font = True, so we're a bit mean and calll
      // unsetFont which calls setFont and then overwrites the own_font
      cb->unsetFont();
    }
   }
}

int KSelectAction::plug( TQWidget *widget, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  kdDebug(129) << "KSelectAction::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( ::tqqt_cast<TQPopupMenu *>( widget) )
  {
    // Create the PopupMenu and store it in m_menu
    (void)popupMenu();

    TQPopupMenu* menu = static_cast<TQPopupMenu*>( widget );
    int id;
    if ( hasIcon() )
      id = menu->insertItem( iconSet(), text(), d->m_menu, -1, index );
    else
      id = menu->insertItem( text(), d->m_menu, -1, index );

    if ( !isEnabled() )
        menu->setItemEnabled( id, false );

    TQString wth = whatsThis();
    if ( !wth.isEmpty() )
        menu->TQMenuData::setWhatsThis( id, wth );

    addContainer( menu, id );
    connect( menu, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( ::tqqt_cast<KToolBar *>( widget ) )
  {
    KToolBar* bar = static_cast<KToolBar*>( widget );
    int id_ = KAction::getToolButtonID();
    bar->insertCombo( comboItems(), id_, isEditable(),
                      TQT_SIGNAL( activated( const TQString & ) ), this,
                      TQT_SLOT( slotActivated( const TQString & ) ), isEnabled(),
                      toolTip(), -1, index );

    TQComboBox *cb = bar->getCombo( id_ );
    if ( cb )
    {
      if (!isEditable()) cb->setFocusPolicy(TQ_NoFocus);
      cb->setMinimumWidth( cb->sizeHint().width() );
      if ( d->m_comboWidth > 0 )
      {
        cb->setMinimumWidth( d->m_comboWidth );
        cb->setMaximumWidth( d->m_comboWidth );
      }
      cb->setInsertionPolicy( TQComboBox::NoInsertion );
      TQWhatsThis::add( cb, whatsThis() );
      if ( d->m_maxComboViewCount != -1 ) cb->setSizeLimit( d->m_maxComboViewCount );
    }

    addContainer( bar, id_ );

    connect( bar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    updateCurrentItem( containerCount() - 1 );

    return containerCount() - 1;
  }
  else if ( ::tqqt_cast<TQMenuBar *>( widget ) )
  {
    // Create the PopupMenu and store it in m_menu
    (void)popupMenu();

    TQMenuBar* menu = static_cast<TQMenuBar*>( widget );
    int id = menu->insertItem( text(), d->m_menu, -1, index );

    if ( !isEnabled() )
        menu->setItemEnabled( id, false );

    TQString wth = whatsThis();
    if ( !wth.isEmpty() )
        menu->TQMenuData::setWhatsThis( id, wth );

    addContainer( menu, id );
    connect( menu, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  kdWarning() << "Can not plug KAction in " << widget->className() << endl;
  return -1;
}

TQStringList KSelectAction::comboItems() const
{
  if( d->m_menuAccelsEnabled ) {
    TQStringList lst;
    TQStringList::ConstIterator it = d->m_list.begin();
    for( ; it != d->m_list.end(); ++it )
    {
      TQString item = *it;
      int i = item.find( '&' );
      if ( i > -1 )
        item = item.remove( i, 1 );
      lst.append( item );
    }
    return lst;
  }
  else
    return d->m_list;
}

void KSelectAction::clear()
{
  if ( d->m_menu )
    d->m_menu->clear();

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateClear( i );
}

void KSelectAction::updateClear( int id )
{
  TQWidget* w = container( id );
  if ( ::tqqt_cast<KToolBar *>( w ) ) {
    TQWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( ::tqqt_cast<TQComboBox *>( r ) ) {
      TQComboBox *b = static_cast<TQComboBox*>( r );
      b->clear();
    }
  }
}

void KSelectAction::slotActivated( int id )
{
  if ( d->m_current == id )
    return;

  setCurrentItem( id );
  // Delay this. Especially useful when the slot connected to activated() will re-create
  // the menu, e.g. in the recent files action. This prevents a crash.
  TQTimer::singleShot( 0, this, TQT_SLOT( slotActivated() ) );
}

void KSelectAction::slotActivated( const TQString &text )
{
  if ( isEditable() )
  {
    TQStringList lst = d->m_list;
    if(!lst.contains(text))
    {
      lst.append( text );
      setItems( lst );
    }
  }

  int i = d->m_list.findIndex( text );
  if ( i > -1 )
      setCurrentItem( i );
  else
      setCurrentItem( comboItems().findIndex( text ) );
  // Delay this. Especially useful when the slot connected to activated() will re-create
  // the menu, e.g. in the recent files action. This prevents a crash.
  TQTimer::singleShot( 0, this, TQT_SLOT( slotActivated() ) );
}

void KSelectAction::slotActivated()
{
  KAction::slotActivated();
  kdDebug(129) << "KSelectAction::slotActivated currentItem=" << currentItem() << " currentText=" << currentText() << endl;
  emit activated( currentItem() );
  emit activated( currentText() );
}

void KSelectAction::setEditable( bool edit )
{
  d->m_edit = edit;
}

bool KSelectAction::isEditable() const
{
  return d->m_edit;
}

void KSelectAction::setRemoveAmpersandsInCombo( bool b )
{
  setMenuAccelsEnabled( b );
}

bool KSelectAction::removeAmpersandsInCombo() const
{
  return menuAccelsEnabled( );
}

void KSelectAction::setMenuAccelsEnabled( bool b )
{
  d->m_menuAccelsEnabled = b;
}

bool KSelectAction::menuAccelsEnabled() const
{
  return d->m_menuAccelsEnabled;
}

class KListAction::KListActionPrivate
{
public:
  KListActionPrivate()
  {
    m_current = 0;
  }
  int m_current;
};

KListAction::KListAction( const TQString& text, const KShortcut& cut,
                          TQObject* parent, const char* name )
  : KSelectAction( text, cut, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const TQString& text, const KShortcut& cut,
                          const TQObject* receiver, const char* slot,
                          TQObject* parent, const char* name )
  : KSelectAction( text, cut, parent, name )
{
  d = new KListActionPrivate;
  if ( receiver )
    connect( this, TQT_SIGNAL( activated( int ) ), receiver, slot );
}

KListAction::KListAction( const TQString& text, const TQIconSet& pix,
                          const KShortcut& cut,
                          TQObject* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const TQString& text, const TQString& pix,
                          const KShortcut& cut,
                          TQObject* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const TQString& text, const TQIconSet& pix,
                          const KShortcut& cut, const TQObject* receiver,
                          const char* slot, TQObject* parent,
                          const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  d = new KListActionPrivate;
  if ( receiver )
    connect( this, TQT_SIGNAL( activated( int ) ), receiver, slot );
}

KListAction::KListAction( const TQString& text, const TQString& pix,
                          const KShortcut& cut, const TQObject* receiver,
                          const char* slot, TQObject* parent,
                          const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  d = new KListActionPrivate;
  if ( receiver )
    connect( this, TQT_SIGNAL( activated( int ) ), receiver, slot );
}

KListAction::KListAction( TQObject* parent, const char* name )
  : KSelectAction( parent, name )
{
  d = new KListActionPrivate;
}

KListAction::~KListAction()
{
  delete d; d = 0;
}

void KListAction::setCurrentItem( int index )
{
  KSelectAction::setCurrentItem( index );
  d->m_current = index;

  //  emit KAction::activated();
  //  emit activated( currentItem() );
  // emit activated( currentText() );
}

TQString KListAction::currentText() const
{
  return KSelectAction::currentText();
}

int KListAction::currentItem() const
{
  return d->m_current;
}

class KRecentFilesAction::KRecentFilesActionPrivate
{
public:
  KRecentFilesActionPrivate()
  {
    m_maxItems = 0;
    m_popup = 0;
  }
  uint m_maxItems;
  KPopupMenu *m_popup;
  TQMap<TQString, TQString> m_shortNames;
  TQMap<TQString, KURL> m_urls;
};

KRecentFilesAction::KRecentFilesAction( const TQString& text,
                                        const KShortcut& cut,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

KRecentFilesAction::KRecentFilesAction( const TQString& text,
                                        const KShortcut& cut,
                                        const TQObject* receiver,
                                        const char* slot,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  if ( receiver )
    connect( this,     TQT_SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const TQString& text,
                                        const TQIconSet& pix,
                                        const KShortcut& cut,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

KRecentFilesAction::KRecentFilesAction( const TQString& text,
                                        const TQString& pix,
                                        const KShortcut& cut,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

KRecentFilesAction::KRecentFilesAction( const TQString& text,
                                        const TQIconSet& pix,
                                        const KShortcut& cut,
                                        const TQObject* receiver,
                                        const char* slot,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  if ( receiver )
    connect( this,     TQT_SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const TQString& text,
                                        const TQString& pix,
                                        const KShortcut& cut,
                                        const TQObject* receiver,
                                        const char* slot,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  if ( receiver )
    connect( this,     TQT_SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( TQObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( parent, name )
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

void KRecentFilesAction::init()
{
  KRecentFilesAction *that = const_cast<KRecentFilesAction*>(this);
  that->d->m_popup = new KPopupMenu;
  connect(d->m_popup, TQT_SIGNAL(aboutToShow()), this, TQT_SLOT(menuAboutToShow()));
  connect(d->m_popup, TQT_SIGNAL(activated(int)), this, TQT_SLOT(menuItemActivated(int)));
  connect( this, TQT_SIGNAL( activated( const TQString& ) ),
           this, TQT_SLOT( itemSelected( const TQString& ) ) );

  setMenuAccelsEnabled( false );
}

KRecentFilesAction::~KRecentFilesAction()
{
  delete d->m_popup;
  delete d; d = 0;
}

uint KRecentFilesAction::maxItems() const
{
    return d->m_maxItems;
}

void KRecentFilesAction::setMaxItems( uint maxItems )
{
    TQStringList lst = KSelectAction::items();
    uint oldCount   = lst.count();

    // set new maxItems
    d->m_maxItems = maxItems;

    // remove all items that are too much
    while( lst.count() > maxItems )
    {
        // remove last item
        TQString lastItem = lst.last();
        d->m_shortNames.erase( lastItem );
        d->m_urls.erase( lastItem );
        lst.remove( lastItem );
    }

    // set new list if changed
    if( lst.count() != oldCount )
        setItems( lst );
}

void KRecentFilesAction::addURL( const KURL& url )
{
    addURL( url, url.fileName() );
}

void KRecentFilesAction::addURL( const KURL& url, const TQString& name )
{
    if ( url.isLocalFile() && !KGlobal::dirs()->relativeLocation("tmp", url.path()).startsWith("/"))
       return;
    const TQString file = url.pathOrURL();
    TQStringList lst = KSelectAction::items();

    // remove file if already in list
    const TQStringList::Iterator end = lst.end();
    for ( TQStringList::Iterator it = lst.begin(); it != end; ++it )
    {
      TQString title = (*it);
      if ( title.endsWith( file + "]" ) )
      {
        lst.remove( it );
        d->m_urls.erase( title );
        d->m_shortNames.erase( title );
        break;
      }
    }
    // remove last item if already maxitems in list
    if( lst.count() == d->m_maxItems )
    {
        // remove last item
        const TQString lastItem = lst.last();
        d->m_shortNames.erase( lastItem );
        d->m_urls.erase( lastItem );
        lst.remove( lastItem );
    }

    // add file to list
    const TQString title = name + " [" + file + "]";
    d->m_shortNames.insert( title, name );
    d->m_urls.insert( title, url );
    lst.prepend( title );
    setItems( lst );
}

void KRecentFilesAction::removeURL( const KURL& url )
{
    TQStringList lst = KSelectAction::items();
    TQString     file = url.pathOrURL();

    // remove url
    TQStringList::Iterator end = lst.end();
    for ( TQStringList::Iterator it = lst.begin(); it != end; ++it )
    {
      if ( (*it).endsWith( file + "]" ))
      {
        d->m_shortNames.erase( (*it) );
        d->m_urls.erase( (*it) );
        lst.remove( it );
        setItems( lst );
        break;
      }
    }
}

void KRecentFilesAction::clearURLList()
{
    clear();
    d->m_shortNames.clear();
    d->m_urls.clear();
}

void KRecentFilesAction::loadEntries( KConfig* config, TQString groupname)
{
    TQString     key;
    TQString     value;
    TQString     nameKey;
    TQString     nameValue;
    TQString      title;
    TQString     oldGroup;
    TQStringList lst;
    KURL        url;

    oldGroup = config->group();

    if (groupname.isEmpty())
      groupname = "RecentFiles";
    config->setGroup( groupname );

    // read file list
    for( unsigned int i = 1 ; i <= d->m_maxItems ; i++ )
    {
        key = TQString( "File%1" ).arg( i );
        value = config->readPathEntry( key );
        url = KURL::fromPathOrURL( value );

        // Don't restore if file doesn't exist anymore
        if (url.isLocalFile() && !TQFile::exists(url.path()))
          continue;

        nameKey = TQString( "Name%1" ).arg( i );
        nameValue = config->readPathEntry( nameKey, url.fileName() );
        title = nameValue + " [" + value + "]";
        if (!value.isNull())
        {
          lst.append( title );
          d->m_shortNames.insert( title, nameValue );
          d->m_urls.insert( title, url );
        }
    }

    // set file
    setItems( lst );

    config->setGroup( oldGroup );
}

void KRecentFilesAction::saveEntries( KConfig* config, TQString groupname )
{
    TQString     key;
    TQString     value;
    TQString     oldGroup;
    TQStringList lst = KSelectAction::items();

    oldGroup = config->group();

    if (groupname.isEmpty())
      groupname = "RecentFiles";
    config->deleteGroup( groupname, true );
    config->setGroup( groupname );

    // write file list
    for( unsigned int i = 1 ; i <= lst.count() ; i++ )
    {
        //kdDebug(129) << "Entry for " << lst[i-1] << d->m_urls[ lst[ i - 1 ] ] << endl;
        key = TQString( "File%1" ).arg( i );
        value = d->m_urls[ lst[ i - 1 ] ].pathOrURL();
        config->writePathEntry( key, value );
        key = TQString( "Name%1" ).arg( i );
        value = d->m_shortNames[ lst[ i - 1 ] ];
        config->writePathEntry( key, value );
    }

    config->setGroup( oldGroup );
}

void KRecentFilesAction::itemSelected( const TQString& text )
{
    //return a copy of the URL since the slot where it is connected might call
    //addURL or removeURL where the d->m_urls.erase( title ) could destroy the
    //d->m_urls[ text ] and the emitted URL will be invalid in the rest of the slot
    emit urlSelected( KURL(d->m_urls[ text ]) );
}

void KRecentFilesAction::menuItemActivated( int id )
{
    TQString text = d->m_popup->text(id);
    //return a copy of the URL since the slot where it is connected might call
    //addURL or removeURL where the d->m_urls.erase( title ) could destroy the
    //d->m_urls[ text ] and the emitted URL will be invalid in the rest of the slot
    emit urlSelected( KURL(d->m_urls[ text ]) );
}

void KRecentFilesAction::menuAboutToShow()
{
    KPopupMenu *menu = d->m_popup;
    menu->clear();
    TQStringList list = KSelectAction::items();
    for ( TQStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
       menu->insertItem(*it);
    }
}

int KRecentFilesAction::plug( TQWidget *widget, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  // This is very related to KActionMenu::plug.
  // In fact this class could be an interesting base class for KActionMenu
  if ( ::tqqt_cast<KToolBar *>( widget ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int id_ = KAction::getToolButtonID();

    KInstance * instance;
    if ( m_parentCollection )
        instance = m_parentCollection->instance();
    else
        instance = KGlobal::instance();

    bar->insertButton( icon(), id_, TQT_SIGNAL( clicked() ), this,
                       TQT_SLOT( slotClicked() ), isEnabled(), plainText(),
                       index, instance );

    addContainer( bar, id_ );

    connect( bar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    bar->setDelayedPopup( id_, d->m_popup, true);

    if ( !whatsThis().isEmpty() )
        TQWhatsThis::add( bar->getButton( id_ ), whatsThisWithIcon() );

    return containerCount() - 1;
  }

  return KListAction::plug( widget, index );
}

void KRecentFilesAction::slotClicked()
{
  KAction::slotActivated();
}

void KRecentFilesAction::slotActivated(const TQString& text)
{
  KListAction::slotActivated(text);
}


void KRecentFilesAction::slotActivated(int id)
{
  KListAction::slotActivated(id);
}


void KRecentFilesAction::slotActivated()
{
  emit activated( currentItem() );
  emit activated( currentText() );
}

//KDE4: rename to urls() and return a KURL::List
TQStringList KRecentFilesAction::items() const
{
    TQStringList lst = KSelectAction::items();
    TQStringList result;

    for( unsigned int i = 1 ; i <= lst.count() ; i++ )
    {
        result += d->m_urls[ lst[ i - 1 ] ].prettyURL(0, KURL::StripFileProtocol);
    }

    return result;
}

//KDE4: remove
TQStringList KRecentFilesAction::completeItems() const
{
    return KSelectAction::items();
}


class KFontAction::KFontActionPrivate
{
public:
  KFontActionPrivate()
  {
  }
  TQStringList m_fonts;
};

KFontAction::KFontAction( const TQString& text,
                          const KShortcut& cut, TQObject* parent,
                          const char* name )
  : KSelectAction( text, cut, parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, 0 );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::KFontAction( const TQString& text, const KShortcut& cut,
                          const TQObject* receiver, const char* slot,
                          TQObject* parent, const char* name )
    : KSelectAction( text, cut, receiver, slot, parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, 0 );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::KFontAction( const TQString& text, const TQIconSet& pix,
                          const KShortcut& cut,
                          TQObject* parent, const char* name )
    : KSelectAction( text, pix, cut, parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, 0 );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::KFontAction( const TQString& text, const TQString& pix,
                          const KShortcut& cut,
                          TQObject* parent, const char* name )
    : KSelectAction( text, pix, cut, parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, 0 );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::KFontAction( const TQString& text, const TQIconSet& pix,
                          const KShortcut& cut,
                          const TQObject* receiver, const char* slot,
                          TQObject* parent, const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, 0 );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::KFontAction( const TQString& text, const TQString& pix,
                          const KShortcut& cut,
                          const TQObject* receiver, const char* slot,
                          TQObject* parent, const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, 0 );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::KFontAction( uint fontListCriteria, const TQString& text,
                          const KShortcut& cut, TQObject* parent,
                          const char* name )
    : KSelectAction( text, cut, parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, fontListCriteria );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::KFontAction( uint fontListCriteria, const TQString& text, const TQString& pix,
                          const KShortcut& cut,
                          TQObject* parent, const char* name )
    : KSelectAction( text, pix, cut, parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, fontListCriteria );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::KFontAction( TQObject* parent, const char* name )
  : KSelectAction( parent, name )
{
    d = new KFontActionPrivate;
    KFontChooser::getFontList( d->m_fonts, 0 );
    KSelectAction::setItems( d->m_fonts );
    setEditable( true );
}

KFontAction::~KFontAction()
{
    delete d;
    d = 0;
}

/*
 * Maintenance note: Keep in sync with KFontCombo::setCurrentFont()
 */
void KFontAction::setFont( const TQString &family )
{
    TQString lowerName = family.lower();
    int i = 0;
    for ( TQStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
    {
       if ((*it).lower() == lowerName)
       {
          setCurrentItem(i);
          return;
       }
    }
    i = lowerName.find(" [");
    if (i>-1)
    {
       lowerName = lowerName.left(i);
       i = 0;
       for ( TQStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
       {
          if ((*it).lower() == lowerName)
          {
             setCurrentItem(i);
             return;
          }
       }
    }

    lowerName += " [";
    i = 0;
    for ( TQStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
    {
       if ((*it).lower().startsWith(lowerName))
       {
          setCurrentItem(i);
          return;
       }
    }

    // nothing matched yet, try a fontconfig reverse lookup and
    // check again to solve an alias
    FcPattern *pattern = NULL;
    FcConfig *config = NULL;
    TQString realFamily;
    TQRegExp regExp("[-:]");
    pattern = FcNameParse( (unsigned char*) family.ascii() );
    FcDefaultSubstitute(pattern);
    FcConfigSubstitute (config, pattern, FcMatchPattern);
    pattern = FcFontMatch(NULL, pattern, NULL);
    realFamily = (char*)FcNameUnparse(pattern);
    realFamily.remove(realFamily.find(regExp), realFamily.length());

    if ( !realFamily.isEmpty() && realFamily != family )
       setFont( realFamily );
    else
       kdDebug(129) << "Font not found " << family.lower() << endl;
}

int KFontAction::plug( TQWidget *w, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  if ( ::tqqt_cast<KToolBar *>( w ) )
  {
    KToolBar* bar = static_cast<KToolBar*>( w );
    int id_ = KAction::getToolButtonID();
    KFontCombo *cb = new KFontCombo( items(), bar );
    connect( cb, TQT_SIGNAL( activated( const TQString & ) ),
             TQT_SLOT( slotActivated( const TQString & ) ) );
    cb->setEnabled( isEnabled() );
    bar->insertWidget( id_, comboWidth(), cb, index );
    cb->setMinimumWidth( cb->sizeHint().width() );

    addContainer( bar, id_ );

    connect( bar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    updateCurrentItem( containerCount() - 1 );

    return containerCount() - 1;
  }
  else return KSelectAction::plug( w, index );
}

class KFontSizeAction::KFontSizeActionPrivate
{
public:
  KFontSizeActionPrivate()
  {
  }
};

KFontSizeAction::KFontSizeAction( const TQString& text,
                                  const KShortcut& cut,
                                  TQObject* parent, const char* name )
  : KSelectAction( text, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const TQString& text,
                                  const KShortcut& cut,
                                  const TQObject* receiver, const char* slot,
                                  TQObject* parent, const char* name )
  : KSelectAction( text, cut, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const TQString& text, const TQIconSet& pix,
                                  const KShortcut& cut,
                                  TQObject* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const TQString& text, const TQString& pix,
                                  const KShortcut& cut,
                                  TQObject* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const TQString& text, const TQIconSet& pix,
                                  const KShortcut& cut,
                                  const TQObject* receiver,
                                  const char* slot, TQObject* parent,
                                  const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const TQString& text, const TQString& pix,
                                  const KShortcut& cut,
                                  const TQObject* receiver,
                                  const char* slot, TQObject* parent,
                                  const char* name )
  : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( TQObject* parent, const char* name )
  : KSelectAction( parent, name )
{
  init();
}

KFontSizeAction::~KFontSizeAction()
{
    delete d;
    d = 0;
}

void KFontSizeAction::init()
{
    d = new KFontSizeActionPrivate;

    setEditable( true );
    TQFontDatabase fontDB;
    TQValueList<int> sizes = fontDB.standardSizes();
    TQStringList lst;
    for ( TQValueList<int>::Iterator it = sizes.begin(); it != sizes.end(); ++it )
        lst.append( TQString::number( *it ) );

    setItems( lst );
}

void KFontSizeAction::setFontSize( int size )
{
    if ( size == fontSize() ) {
        setCurrentItem( items().findIndex( TQString::number( size ) ) );
        return;
    }

    if ( size < 1 ) {
        kdWarning() << "KFontSizeAction: Size " << size << " is out of range" << endl;
        return;
    }

    int index = items().findIndex( TQString::number( size ) );
    if ( index == -1 ) {
        // Insert at the correct position in the list (to keep sorting)
        TQValueList<int> lst;
        // Convert to list of ints
        TQStringList itemsList = items();
        for (TQStringList::Iterator it = itemsList.begin() ; it != itemsList.end() ; ++it)
            lst.append( (*it).toInt() );
        // New size
        lst.append( size );
        // Sort the list
        qHeapSort( lst );
        // Convert back to string list
        TQStringList strLst;
        for (TQValueList<int>::Iterator it = lst.begin() ; it != lst.end() ; ++it)
            strLst.append( TQString::number(*it) );
        KSelectAction::setItems( strLst );
        // Find new current item
        index = lst.findIndex( size );
        setCurrentItem( index );
    }
    else
        setCurrentItem( index );


    //emit KAction::activated();
    //emit activated( index );
    //emit activated( TQString::number( size ) );
    //emit fontSizeChanged( size );
}

int KFontSizeAction::fontSize() const
{
  return currentText().toInt();
}

void KFontSizeAction::slotActivated( int index )
{
  KSelectAction::slotActivated( index );

  emit fontSizeChanged( items()[ index ].toInt() );
}

void KFontSizeAction::slotActivated( const TQString& size )
{
  setFontSize( size.toInt() ); // insert sorted first
  KSelectAction::slotActivated( size );
  emit fontSizeChanged( size.toInt() );
}

class KActionMenu::KActionMenuPrivate
{
public:
  KActionMenuPrivate()
  {
    m_popup = new KPopupMenu(0L,"KActionMenu::KActionMenuPrivate");
    m_delayed = true;
    m_stickyMenu = true;
  }
  ~KActionMenuPrivate()
  {
    delete m_popup; m_popup = 0;
  }
  KPopupMenu *m_popup;
  bool m_delayed;
  bool m_stickyMenu;
};

KActionMenu::KActionMenu( TQObject* parent, const char* name )
  : KAction( parent, name )
{
  d = new KActionMenuPrivate;
  setShortcutConfigurable( false );
}

KActionMenu::KActionMenu( const TQString& text, TQObject* parent,
                          const char* name )
  : KAction( text, 0, parent, name )
{
  d = new KActionMenuPrivate;
  setShortcutConfigurable( false );
}

KActionMenu::KActionMenu( const TQString& text, const TQIconSet& icon,
                          TQObject* parent, const char* name )
  : KAction( text, icon, 0, parent, name )
{
  d = new KActionMenuPrivate;
  setShortcutConfigurable( false );
}

KActionMenu::KActionMenu( const TQString& text, const TQString& icon,
                          TQObject* parent, const char* name )
  : KAction( text, icon, 0, parent, name )
{
  d = new KActionMenuPrivate;
  setShortcutConfigurable( false );
}

KActionMenu::~KActionMenu()
{
    unplugAll();
    kdDebug(129) << "KActionMenu::~KActionMenu()" << endl; // ellis
    delete d; d = 0;
}

void KActionMenu::popup( const TQPoint& global )
{
  popupMenu()->popup( global );
}

KPopupMenu* KActionMenu::popupMenu() const
{
  return d->m_popup;
}

void KActionMenu::insert( KAction* cmd, int index )
{
  if ( cmd )
    cmd->plug( d->m_popup, index );
}

void KActionMenu::remove( KAction* cmd )
{
  if ( cmd )
    cmd->unplug( d->m_popup );
}

bool KActionMenu::delayed() const {
    return d->m_delayed;
}

void KActionMenu::setDelayed(bool _delayed) {
    d->m_delayed = _delayed;
}

bool KActionMenu::stickyMenu() const {
    return d->m_stickyMenu;
}

void KActionMenu::setStickyMenu(bool sticky) {
    d->m_stickyMenu = sticky;
}

int KActionMenu::plug( TQWidget* widget, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  kdDebug(129) << "KActionMenu::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( ::tqqt_cast<TQPopupMenu *>( widget ) )
  {
    TQPopupMenu* menu = static_cast<TQPopupMenu*>( widget );
    int id;
    if ( hasIcon() )
      id = menu->insertItem( iconSet(), text(), d->m_popup, -1, index );
    else
      id = menu->insertItem( text(), d->m_popup, -1, index );

    if ( !isEnabled() )
      menu->setItemEnabled( id, false );

    addContainer( menu, id );
    connect( menu, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( menu, this );

    return containerCount() - 1;
  }
  else if ( ::tqqt_cast<KToolBar *>( widget ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    int id_ = KAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() )
      bar->insertButton( iconSet().pixmap(), id_, TQT_SIGNAL( clicked() ), this,
                         TQT_SLOT( slotActivated() ), isEnabled(), plainText(),
                         index );
    else
    {
      KInstance *instance;

      if ( m_parentCollection )
        instance = m_parentCollection->instance();
      else
        instance = KGlobal::instance();

      bar->insertButton( icon(), id_, TQT_SIGNAL( clicked() ), this,
                         TQT_SLOT( slotActivated() ), isEnabled(), plainText(),
                         index, instance );
    }

    addContainer( bar, id_ );

    if (!whatsThis().isEmpty())
      TQWhatsThis::add( bar->getButton(id_), whatsThis() );

    connect( bar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    if (delayed()) {
        bar->setDelayedPopup( id_, popupMenu(), stickyMenu() );
    } else {
        bar->getButton(id_)->setPopup(popupMenu(), stickyMenu() );
    }

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( bar, this );

    return containerCount() - 1;
  }
  else if ( ::tqqt_cast<TQMenuBar *>( widget ) )
  {
    TQMenuBar *bar = static_cast<TQMenuBar *>( widget );

    int id;

    id = bar->insertItem( text(), popupMenu(), -1, index );

    if ( !isEnabled() )
        bar->setItemEnabled( id, false );

    addContainer( bar, id );
    connect( bar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

////////

KToolBarPopupAction::KToolBarPopupAction( const TQString& text,
                                          const TQString& icon,
                                          const KShortcut& cut,
                                          TQObject* parent, const char* name )
  : KAction( text, icon, cut, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

KToolBarPopupAction::KToolBarPopupAction( const TQString& text,
                                          const TQString& icon,
                                          const KShortcut& cut,
                                          const TQObject* receiver,
                                          const char* slot, TQObject* parent,
                                          const char* name )
  : KAction( text, icon, cut, receiver, slot, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

KToolBarPopupAction::KToolBarPopupAction( const KGuiItem& item,
                                          const KShortcut& cut,
                                          const TQObject* receiver,
                                          const char* slot, KActionCollection* parent,
                                          const char* name )
  : KAction( item, cut, receiver, slot, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

KToolBarPopupAction::~KToolBarPopupAction()
{
    delete m_popup;
}

bool KToolBarPopupAction::delayed() const {
    return m_delayed;
}

void KToolBarPopupAction::setDelayed(bool delayed) {
    m_delayed = delayed;
}

bool KToolBarPopupAction::stickyMenu() const {
    return m_stickyMenu;
}

void KToolBarPopupAction::setStickyMenu(bool sticky) {
    m_stickyMenu = sticky;
}

int KToolBarPopupAction::plug( TQWidget *widget, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  // This is very related to KActionMenu::plug.
  // In fact this class could be an interesting base class for KActionMenu
  if ( ::tqqt_cast<KToolBar *>( widget ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int id_ = KAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() ) {
        bar->insertButton( iconSet().pixmap(), id_, TQT_SIGNAL( buttonClicked(int, TQt::ButtonState) ), this,
                           TQT_SLOT( slotButtonClicked(int, TQt::ButtonState) ),
                           isEnabled(), plainText(),
                           index );
    } else {
        KInstance * instance;
        if ( m_parentCollection )
            instance = m_parentCollection->instance();
        else
            instance = KGlobal::instance();

        bar->insertButton( icon(), id_, TQT_SIGNAL( buttonClicked(int, TQt::ButtonState) ), this,
                           TQT_SLOT( slotButtonClicked(int, TQt::ButtonState) ),
                           isEnabled(), plainText(),
                           index, instance );
    }

    addContainer( bar, id_ );

    connect( bar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    if (delayed()) {
        bar->setDelayedPopup( id_, popupMenu(), stickyMenu() );
    } else {
        bar->getButton(id_)->setPopup(popupMenu(), stickyMenu());
    }

    if ( !whatsThis().isEmpty() )
        TQWhatsThis::add( bar->getButton( id_ ), whatsThisWithIcon() );

    return containerCount() - 1;
  }

  return KAction::plug( widget, index );
}

KPopupMenu *KToolBarPopupAction::popupMenu() const
{
    if ( !m_popup ) {
        KToolBarPopupAction *that = const_cast<KToolBarPopupAction*>(this);
        that->m_popup = new KPopupMenu;
    }
    return m_popup;
}

////////

KToggleToolBarAction::KToggleToolBarAction( const char* toolBarName,
         const TQString& text, KActionCollection* parent, const char* name )
  : KToggleAction( text, KShortcut(), parent, name )
  , m_toolBarName( toolBarName )
  , m_toolBar( 0L )
{
}

KToggleToolBarAction::KToggleToolBarAction( KToolBar *toolBar, const TQString &text,
                                            KActionCollection *parent, const char *name )
  : KToggleAction( text, KShortcut(), parent, name )
  , m_toolBarName( 0 ), m_toolBar( toolBar )
{
}

KToggleToolBarAction::~KToggleToolBarAction()
{
}

int KToggleToolBarAction::plug( TQWidget* w, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
      return -1;

  if ( !m_toolBar ) {
    // Note: topLevelWidget() stops too early, we can't use it.
    TQWidget * tl = w;
    TQWidget * n;
    while ( !tl->isDialog() && ( n = tl->parentWidget() ) ) // lookup parent and store
      tl = n;

    KMainWindow * mw = tqt_dynamic_cast<KMainWindow *>(tl); // try to see if it's a kmainwindow

    if ( mw )
        m_toolBar = mw->toolBar( m_toolBarName );
  }

  if( m_toolBar ) {
    setChecked( m_toolBar->isVisible() );
    connect( m_toolBar, TQT_SIGNAL(visibilityChanged(bool)), this, TQT_SLOT(setChecked(bool)) );
    // Also emit toggled when the toolbar's visibility changes (see comment in header)
    connect( m_toolBar, TQT_SIGNAL(visibilityChanged(bool)), this, TQT_SIGNAL(toggled(bool)) );
  } else {
    setEnabled( false );
  }

  return KToggleAction::plug( w, index );
}

void KToggleToolBarAction::setChecked( bool c )
{
  if( m_toolBar && c != m_toolBar->isVisible() ) {
    if( c ) {
      m_toolBar->show();
    } else {
      m_toolBar->hide();
    }
    TQMainWindow* mw = m_toolBar->mainWindow();
    if ( mw && ::tqqt_cast<KMainWindow *>( mw ) )
      static_cast<KMainWindow *>( mw )->setSettingsDirty();
  }
  KToggleAction::setChecked( c );
}

////////

KToggleFullScreenAction::KToggleFullScreenAction( const KShortcut &cut,
                             const TQObject* receiver, const char* slot,
                             TQObject* parent, TQWidget* window,
                             const char* name )
  : KToggleAction( TQString::null, cut, receiver, slot, parent, name ),
    window( NULL )
{
  setWindow( window );
}

KToggleFullScreenAction::~KToggleFullScreenAction()
{
}

void KToggleFullScreenAction::setWindow( TQWidget* w )
{
  if( window )
    window->removeEventFilter( this );
  window = w;
  if( window )
    window->installEventFilter( this );
}

void KToggleFullScreenAction::setChecked( bool c )
{
  if (c)
  {
     setText(i18n("Exit F&ull Screen Mode"));
     setIcon("window_nofullscreen");
  }
  else
  {
     setText(i18n("F&ull Screen Mode"));
     setIcon("window_fullscreen");
  }
  KToggleAction::setChecked( c );
}

bool KToggleFullScreenAction::eventFilter( TQObject* o, TQEvent* e )
{
    if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(window) )
        if( e->type() == TQEvent::WindowStateChange )
            {
            if( window->isFullScreen() != isChecked())
                slotActivated(); // setChecked( window->isFullScreen()) wouldn't emit signals
            }
    return false;
}

////////

KWidgetAction::KWidgetAction( TQWidget* widget,
    const TQString& text, const KShortcut& cut,
    const TQObject* receiver, const char* slot,
    KActionCollection* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
  , m_widget( widget )
  , m_autoSized( false )
{
  connect( this, TQT_SIGNAL(enabled(bool)), widget, TQT_SLOT(setEnabled(bool)) );
}

KWidgetAction::~KWidgetAction()
{
}

void KWidgetAction::setAutoSized( bool autoSized )
{
  if( m_autoSized == autoSized )
    return;

  m_autoSized = autoSized;

  if( !m_widget || !isPlugged() )
    return;

  KToolBar* toolBar = (KToolBar*)m_widget->parent();
  int i = findContainer( toolBar );
  if ( i == -1 )
    return;
  int id = itemId( i );

  toolBar->setItemAutoSized( id, m_autoSized );
}

int KWidgetAction::plug( TQWidget* w, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
      return -1;

  if ( !::tqqt_cast<KToolBar *>( w ) ) {
    kdError() << "KWidgetAction::plug: KWidgetAction must be plugged into KToolBar." << endl;
    return -1;
  }
  if ( !m_widget ) {
    kdError() << "KWidgetAction::plug: Widget was deleted or null!" << endl;
    return -1;
  }

  KToolBar* toolBar = static_cast<KToolBar*>( w );

  int id = KAction::getToolButtonID();

  m_widget->reparent( toolBar, TQPoint() );
  toolBar->insertWidget( id, 0, m_widget, index );
  toolBar->setItemAutoSized( id, m_autoSized );

  TQWhatsThis::add( m_widget, whatsThis() );
  addContainer( toolBar, id );

  connect( toolBar, TQT_SIGNAL( toolbarDestroyed() ), this, TQT_SLOT( slotToolbarDestroyed() ) );
  connect( toolBar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

  return containerCount() - 1;
}

void KWidgetAction::unplug( TQWidget *w )
{
  if( !m_widget || !isPlugged() )
    return;

  KToolBar* toolBar = (KToolBar*)m_widget->parent();
  if ( toolBar == w )
  {
      disconnect( toolBar, TQT_SIGNAL( toolbarDestroyed() ), this, TQT_SLOT( slotToolbarDestroyed() ) );
      m_widget->reparent( 0L, TQPoint(), false /*showIt*/ );
  }
  KAction::unplug( w );
}

void KWidgetAction::slotToolbarDestroyed()
{
  //Q_ASSERT( m_widget ); // When exiting the app the widget could be destroyed before the toolbar.
  Q_ASSERT( isPlugged() );
  if( !m_widget || !isPlugged() )
    return;

  // Don't let a toolbar being destroyed, delete my widget.
  m_widget->reparent( 0L, TQPoint(), false /*showIt*/ );
}

////////

KActionSeparator::KActionSeparator( TQObject *parent, const char *name )
  : KAction( parent, name )
{
}

KActionSeparator::~KActionSeparator()
{
}

int KActionSeparator::plug( TQWidget *widget, int index )
{
  if ( ::tqqt_cast<TQPopupMenu *>( widget) )
  {
    TQPopupMenu* menu = static_cast<TQPopupMenu*>( widget );

    int id = menu->insertSeparator( index );

    addContainer( menu, id );
    connect( menu, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( ::tqqt_cast<TQMenuBar *>( widget ) )
  {
    TQMenuBar *menuBar = static_cast<TQMenuBar *>( widget );

    int id = menuBar->insertSeparator( index );

    addContainer( menuBar, id );

    connect( menuBar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( ::tqqt_cast<KToolBar *>( widget ) )
  {
    KToolBar *toolBar = static_cast<KToolBar *>( widget );

    int id = toolBar->insertSeparator( index );

    addContainer( toolBar, id );

    connect( toolBar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

KPasteTextAction::KPasteTextAction( const TQString& text,
                            const TQString& icon,
                            const KShortcut& cut,
                            const TQObject* receiver,
                            const char* slot, TQObject* parent,
                            const char* name)
  : KAction( text, icon, cut, receiver, slot, parent, name )
{
  m_popup = new KPopupMenu;
  connect(m_popup, TQT_SIGNAL(aboutToShow()), this, TQT_SLOT(menuAboutToShow()));
  connect(m_popup, TQT_SIGNAL(activated(int)), this, TQT_SLOT(menuItemActivated(int)));
  m_popup->setCheckable(true);
  m_mixedMode = true;
}

KPasteTextAction::~KPasteTextAction()
{
  delete m_popup;
}

void KPasteTextAction::setMixedMode(bool mode)
{
  m_mixedMode = mode;
}

int KPasteTextAction::plug( TQWidget *widget, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  if ( ::tqqt_cast<KToolBar *>( widget ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int id_ = KAction::getToolButtonID();

    KInstance * instance;
    if ( m_parentCollection )
        instance = m_parentCollection->instance();
    else
        instance = KGlobal::instance();

    bar->insertButton( icon(), id_, TQT_SIGNAL( clicked() ), this,
                       TQT_SLOT( slotActivated() ), isEnabled(), plainText(),
                       index, instance );

    addContainer( bar, id_ );

    connect( bar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    bar->setDelayedPopup( id_, m_popup, true );

    if ( !whatsThis().isEmpty() )
        TQWhatsThis::add( bar->getButton( id_ ), whatsThisWithIcon() );

    return containerCount() - 1;
  }

  return KAction::plug( widget, index );
}

void KPasteTextAction::menuAboutToShow()
{
    m_popup->clear();
    TQStringList list;
    DCOPClient *client = kapp->dcopClient();
    if (client->isAttached() && client->isApplicationRegistered("klipper")) {
      DCOPRef klipper("klipper","klipper");
      DCOPReply reply = klipper.call("getClipboardHistoryMenu");
      if (reply.isValid())
        list = reply;
    }
    TQString clipboardText = tqApp->tqclipboard()->text(TQClipboard::Clipboard);
    if (list.isEmpty())
        list << clipboardText;
    bool found = false;
    for ( TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
    {
      TQString text = KStringHandler::cEmSqueeze((*it).simplifyWhiteSpace(), m_popup->fontMetrics(), 20);
      text.replace("&", "&&");
      int id = m_popup->insertItem(text);
      if (!found && *it == clipboardText)
      {
        m_popup->setItemChecked(id, true);
        found = true;
      }
    }
}

void KPasteTextAction::menuItemActivated( int id)
{
    DCOPClient *client = kapp->dcopClient();
    if (client->isAttached() && client->isApplicationRegistered("klipper")) {
      DCOPRef klipper("klipper","klipper");
      DCOPReply reply = klipper.call("getClipboardHistoryItem(int)", m_popup->indexOf(id));
      if (!reply.isValid())
        return;
      TQString clipboardText = reply;
      reply = klipper.call("setClipboardContents(TQString)", clipboardText);
      if (reply.isValid())
        kdDebug(129) << "Clipboard: " << TQString(tqApp->tqclipboard()->text(TQClipboard::Clipboard)) << endl;
    }
    TQTimer::singleShot(20, this, TQT_SLOT(slotActivated()));
}

void KPasteTextAction::slotActivated()
{
  if (!m_mixedMode) {
    TQWidget *w = tqApp->widgetAt(TQCursor::pos(), true);
    TQMimeSource *data = TQApplication::tqclipboard()->data();
    if (!data->provides("text/plain") && w) {
      m_popup->popup(w->mapToGlobal(TQPoint(0, w->height())));
    } else
      KAction::slotActivated();
  } else
    KAction::slotActivated();
}


void KToggleAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KRadioAction::virtual_hook( int id, void* data )
{ KToggleAction::virtual_hook( id, data ); }

void KSelectAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KListAction::virtual_hook( int id, void* data )
{ KSelectAction::virtual_hook( id, data ); }

void KRecentFilesAction::virtual_hook( int id, void* data )
{ KListAction::virtual_hook( id, data ); }

void KFontAction::virtual_hook( int id, void* data )
{ KSelectAction::virtual_hook( id, data ); }

void KFontSizeAction::virtual_hook( int id, void* data )
{ KSelectAction::virtual_hook( id, data ); }

void KActionMenu::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KToolBarPopupAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KToggleToolBarAction::virtual_hook( int id, void* data )
{ KToggleAction::virtual_hook( id, data ); }

void KToggleFullScreenAction::virtual_hook( int id, void* data )
{ KToggleAction::virtual_hook( id, data ); }

void KWidgetAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KActionSeparator::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KPasteTextAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

/* vim: et sw=2 ts=2
 */

#include "kactionclasses.moc"
