/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

#include "kaction.h"

#include <assert.h>

#include <tqtooltip.h>
#include <tqwhatsthis.h>

#include <kaccel.h>
#include <kaccelbase.h>
#include <kaccelprivate.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kguiitem.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <kpopupmenu.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>

#include <ft2build.h>
#include <X11/Xdefs.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xft/Xft.h>

/**
* How it works.
* KActionCollection is an organizing container for KActions.
* KActionCollection keeps track of the information necessary to handle
* configuration and shortcuts.
*
* Focus Widget pointer:
* This is the widget which is the focus for action shortcuts.
* It is set either by passing a TQWidget* to the KActionCollection constructor
* or by calling setWidget() if the widget wasn't known when the object was
* initially constructed (as in KXMLGUIClient and KParts::PartBase)
*
* Shortcuts:
* An action's shortcut will not not be connected unless a focus widget has
* been specified in KActionCollection.
*
* XML Filename:
* This is used to save user-modified settings back to the *ui.rc file.
* It is set by KXMLGUIFactory.
*/

int KAction::getToolButtonID()
{
    static int toolbutton_no = -2;
    return toolbutton_no--;
}

//---------------------------------------------------------------------
// KAction::KActionPrivate
//---------------------------------------------------------------------

class KAction::KActionPrivate : public KGuiItem
{
public:
  KActionPrivate() : KGuiItem()
  {
    m_kaccel = 0;
    m_configurable = true;
  }

  KAccel *m_kaccel;
  TQValueList<KAccel*> m_kaccelList;

  TQString m_groupText;
  TQString m_group;

  KShortcut m_cut;
  KShortcut m_cutDefault;

  bool m_configurable;

  struct Container
  {
    Container() { m_container = 0; m_representative = 0; m_id = 0; }
    Container( const Container& s ) { m_container = s.m_container;
                                      m_id = s.m_id; m_representative = s.m_representative; }
    TQWidget* m_container;
    int m_id;
    TQWidget* m_representative;
  };

  TQValueList<Container> m_containers;
};

//---------------------------------------------------------------------
// KAction
//---------------------------------------------------------------------

KAction::KAction( const TQString& text, const KShortcut& cut,
             const TQObject* receiver, const char* slot,
             KActionCollection* parent, const char* name )
: TQObject( parent, name ), d(new KActionPrivate)
{
	initPrivate( text, cut, receiver, slot );
}

KAction::KAction( const TQString& text, const TQString& sIconName, const KShortcut& cut,
	const TQObject* receiver, const char* slot,
	KActionCollection* parent, const char* name )
: TQObject( parent, name ), d(new KActionPrivate)
{
	initPrivate( text, cut, receiver, slot );
	d->setIconName( sIconName );
}

KAction::KAction( const TQString& text, const TQIconSet& pix, const KShortcut& cut,
	const TQObject* receiver, const char* slot,
	KActionCollection* parent, const char* name )
: TQObject( parent, name ), d(new KActionPrivate)
{
	initPrivate( text, cut, receiver, slot );
	d->setIconSet( pix );
}

KAction::KAction( const KGuiItem& item, const KShortcut& cut,
	const TQObject* receiver, const char* slot,
	KActionCollection* parent, const char* name )
: TQObject( parent, name ), d(new KActionPrivate)
{
	initPrivate( item.text(), cut, receiver, slot );
	if( item.hasIcon() )
		setIcon( item.iconName() );
	setToolTip( item.toolTip() );
	setWhatsThis( item.whatsThis() );
}

#ifndef KDE_NO_COMPAT // KDE 4: remove
KAction::KAction( const TQString& text, const KShortcut& cut,
                  TQObject* parent, const char* name )
 : TQObject( parent, name ), d(new KActionPrivate)
{
    initPrivate( text, cut, 0, 0 );
}

KAction::KAction( const TQString& text, const KShortcut& cut,
                  const TQObject* receiver,
                  const char* slot, TQObject* parent, const char* name )
 : TQObject( parent, name ), d(new KActionPrivate)
{
    initPrivate( text, cut, receiver, slot );
}

KAction::KAction( const TQString& text, const TQIconSet& pix,
                  const KShortcut& cut,
                  TQObject* parent, const char* name )
 : TQObject( parent, name ), d(new KActionPrivate)
{
    initPrivate( text, cut, 0, 0 );
    setIconSet( pix );
}

KAction::KAction( const TQString& text, const TQString& pix,
                  const KShortcut& cut,
                  TQObject* parent, const char* name )
: TQObject( parent, name ), d(new KActionPrivate)
{
    initPrivate( text, cut, 0, 0 );
    d->setIconName( pix );
}

KAction::KAction( const TQString& text, const TQIconSet& pix,
                  const KShortcut& cut,
                  const TQObject* receiver, const char* slot, TQObject* parent,
                  const char* name )
 : TQObject( parent, name ), d(new KActionPrivate)
{
    initPrivate( text, cut, receiver, slot );
    setIconSet( pix );
}

KAction::KAction( const TQString& text, const TQString& pix,
                  const KShortcut& cut,
                  const TQObject* receiver, const char* slot, TQObject* parent,
                  const char* name )
  : TQObject( parent, name ), d(new KActionPrivate)
{
    initPrivate( text, cut, receiver, slot );
    d->setIconName(pix);
}

KAction::KAction( TQObject* parent, const char* name )
 : TQObject( parent, name ), d(new KActionPrivate)
{
    initPrivate( TQString::null, KShortcut(), 0, 0 );
}
#endif // KDE 4: remove end

KAction::~KAction()
{
    kdDebug(129) << "KAction::~KAction( this = \"" << name() << "\" )" << endl; // -- ellis
#ifndef KDE_NO_COMPAT
     if (d->m_kaccel)
       unplugAccel();
#endif

    // If actionCollection hasn't already been destructed,
    if ( m_parentCollection ) {
        m_parentCollection->take( this );

        const TQValueList<KAccel*> & accelList = d->m_kaccelList;
        TQValueList<KAccel*>::const_iterator itr = accelList.constBegin();
        const TQValueList<KAccel*>::const_iterator itrEnd = accelList.constEnd();

        const char * const namePtr = name();
        for (; itr != itrEnd; ++itr )
            (*itr)->remove(namePtr);

    }

    // Do not call unplugAll from here, as tempting as it sounds.
    // KAction is designed around the idea that you need to plug
    // _and_ to unplug it "manually". Unplugging leads to an important
    // slowdown when e.g. closing the window, in which case we simply
    // want to destroy everything asap, not to remove actions one by one
    // from the GUI.

    delete d;
}

void KAction::initPrivate( const TQString& text, const KShortcut& cut,
                  const TQObject* receiver, const char* slot )
{
    d->m_cutDefault = cut;

    m_parentCollection = dynamic_cast<KActionCollection *>( parent() );
    kdDebug(129) << "KAction::initPrivate(): this = " << this << " name = \"" << name() << "\" cut = " << cut.toStringInternal() << " m_parentCollection = " << m_parentCollection << endl;
    if ( m_parentCollection )
        m_parentCollection->insert( this );

    if ( receiver && slot )
        connect( this, TQT_SIGNAL( activated() ), receiver, slot );

    if( !cut.isNull() && !qstrcmp( name(), "unnamed" ) )
        kdWarning(129) << "KAction::initPrivate(): trying to assign a shortcut (" << cut.toStringInternal() << ") to an unnamed action." << endl;
    d->setText( text );
    initShortcut( cut );
}

bool KAction::isPlugged() const
{
  return (!d->m_containers.empty()) || d->m_kaccel;
}

bool KAction::isPlugged( const TQWidget *container ) const
{
  return findContainer( container ) > -1;
}

bool KAction::isPlugged( const TQWidget *container, int id ) const
{
  int i = findContainer( container );
  return ( i > -1 && itemId( i ) == id );
}

bool KAction::isPlugged( const TQWidget *container, const TQWidget *_representative ) const
{
  int i = findContainer( container );
  return ( i > -1 && representative( i ) == _representative );
}


/*
Three actionCollection conditions:
	1) Scope is known on creation and KAccel object is created (e.g. KMainWindow)
	2) Scope is unknown and no KAccel object is available (e.g. KXMLGUIClient)
		a) addClient() will be called on object
		b) we just want to add the actions to another KXMLGUIClient object

The question is how to do we incorporate #2b into the XMLGUI framework?


We have a KCommandHistory object with undo and redo actions in a passed actionCollection
We have a KoDoc object which holds a KCommandHistory object and the actionCollection
We have two KoView objects which both point to the same KoDoc object
Undo and Redo should be available in both KoView objects, and
	calling the undo->setEnabled() should affect both KoViews

When addClient is called, it needs to be able to find the undo and redo actions
When it calls plug() on them, they need to be inserted into the KAccel object of the appropriate KoView

In this case, the actionCollection belongs to KoDoc and we need to let it know that its shortcuts
have the same scope as the KoView actionCollection

KXMLGUIClient::addSubActionCollection

Document:
	create document actions

View
	create view actions
	add document actionCollection as sub-collection

A parentCollection is created
Scenario 1: parentCollection has a focus widget set (e.g. via KMainWindow)
	A KAccel object is created in the parentCollection
	A KAction is created with parent=parentCollection
	The shortcut is inserted into this actionCollection
	Scenario 1a: xml isn't used
		done
	Scenario 1b: KXMLGUIBuilder::addClient() called
		setWidget is called -- ignore
		shortcuts are set
Scenario 2: parentCollection has no focus widget (e.g., KParts)
	A KAction is created with parent=parentCollection
	Scenario 2a: xml isn't used
		no shortcuts
	Scenario 2b: KXMLGUIBuilder::addClient() called
		setWidget is called
		shortcuts are inserted into current KAccel
		shortcuts are set in all other KAccels, if the action is present in the other KAccels
*/

/*
shortcut may be set:
	- on construction
	- on plug
	- on reading XML
	- on plugAccel (deprecated)

On Construction: [via initShortcut()]
	insert into KAccel of m_parentCollection,
		if kaccel() && isAutoConnectShortcuts() exists

On Plug: [via plug() -> plugShortcut()]
	insert into KAccel of m_parentCollection, if exists and not already inserted into

On Read XML: [via setShortcut()]
	set in all current KAccels
	insert into KAccel of m_parentCollection, if exists and not already inserted into
*/

KAccel* KAction::kaccelCurrent()
{
  if( m_parentCollection && m_parentCollection->builderKAccel() )
    return m_parentCollection->builderKAccel();
  else if( m_parentCollection && m_parentCollection->kaccel() )
    return m_parentCollection->kaccel();
  else
    return 0L;
}

// Only to be called from initPrivate()
bool KAction::initShortcut( const KShortcut& cut )
{
    d->m_cut = cut;

    // Only insert action into KAccel if it has a valid name,
    if( qstrcmp( name(), "unnamed" ) &&
        m_parentCollection &&
        m_parentCollection->isAutoConnectShortcuts() &&
        m_parentCollection->kaccel() )
    {
        insertKAccel( m_parentCollection->kaccel() );
        return true;
    }
    return false;
 }

// Only to be called from plug()
void KAction::plugShortcut()
{
  KAccel* const kaccel = kaccelCurrent();

  //kdDebug(129) << "KAction::plugShortcut(): this = " << this << " kaccel() = " << (m_parentCollection ? m_parentCollection->kaccel() : 0) << endl;
  if( kaccel && qstrcmp( name(), "unnamed" ) ) {
    // Check if already plugged into current KAccel object
    const TQValueList<KAccel*> & accelList = d->m_kaccelList;
    TQValueList<KAccel*>::const_iterator itr = accelList.constBegin();
    const TQValueList<KAccel*>::const_iterator itrEnd = accelList.constEnd();

    for( ; itr != itrEnd; ++itr) {
      if( (*itr) == kaccel )
        return;
    }

    insertKAccel( kaccel );
  }
}

bool KAction::setShortcut( const KShortcut& cut )
{
  bool bChanged = (d->m_cut != cut);
  d->m_cut = cut;

  KAccel* const kaccel = kaccelCurrent();
  bool bInsertRequired = true;
  // Apply new shortcut to all existing KAccel objects

  const TQValueList<KAccel*> & accelList = d->m_kaccelList;
  TQValueList<KAccel*>::const_iterator itr = accelList.constBegin();
  const TQValueList<KAccel*>::const_iterator itrEnd = accelList.constEnd();

  for( ; itr != itrEnd; ++itr) {
    // Check whether shortcut has already been plugged into
    //  the current kaccel object.
    if( (*itr) == kaccel )
      bInsertRequired = false;
    if( bChanged )
      updateKAccelShortcut( *itr );
  }

  // Only insert action into KAccel if it has a valid name,
  if( kaccel && bInsertRequired && qstrcmp( name(), "unnamed" ) )
    insertKAccel( kaccel );

  if( bChanged ) {
#ifndef KDE_NO_COMPAT    // KDE 4: remove
    if ( d->m_kaccel )
      d->m_kaccel->setShortcut( name(), cut );
#endif    // KDE 4: remove end
      int len = containerCount();
      for( int i = 0; i < len; ++i )
          updateShortcut( i );
  }
  return true;
}

bool KAction::updateKAccelShortcut( KAccel* kaccel )
{
  // Check if action is permitted
  if (kapp && !kapp->authorizeKAction(name()))
    return false;

  bool b = true;

  if ( !kaccel->actions().actionPtr( name() ) ) {
    if(!d->m_cut.isNull() ) {
      kdDebug(129) << "Inserting " << name() << ", " << d->text() << ", " << d->plainText() << endl;
      b = kaccel->insert( name(), d->plainText(), TQString::null,
          d->m_cut,
          this, TQT_SLOT(slotActivated()),
          isShortcutConfigurable(), isEnabled() );
    }
  }
  else
    b = kaccel->setShortcut( name(), d->m_cut );

  return b;
}

void KAction::insertKAccel( KAccel* kaccel )
{
  //kdDebug(129) << "KAction::insertKAccel( " << kaccel << " ): this = " << this << endl;
  if ( !kaccel->actions().actionPtr( name() ) ) {
    if( updateKAccelShortcut( kaccel ) ) {
      d->m_kaccelList.append( kaccel );
      connect( kaccel, TQT_SIGNAL(destroyed()), this, TQT_SLOT(slotDestroyed()) );
    }
  }
  else
    kdWarning(129) << "KAction::insertKAccel( kaccel = " << kaccel << " ): KAccel object already contains an action name \"" << name() << "\"" << endl; // -- ellis
}

void KAction::removeKAccel( KAccel* kaccel )
{
  //kdDebug(129) << "KAction::removeKAccel( " << i << " ): this = " << this << endl;
  TQValueList<KAccel*> & accelList = d->m_kaccelList;
  TQValueList<KAccel*>::iterator itr = accelList.begin();
  const TQValueList<KAccel*>::iterator itrEnd = accelList.end();

  for( ; itr != itrEnd; ++itr) {
    if( (*itr) == kaccel ) {
      kaccel->remove( name() );
      accelList.remove( itr );
      disconnect( kaccel, TQT_SIGNAL(destroyed()), this, TQT_SLOT(slotDestroyed()) );
      break;
    }
  }
}

#ifndef KDE_NO_COMPAT
// KDE 4: remove
void KAction::setAccel( int keyQt )
{
  setShortcut( KShortcut(keyQt) );
}
#endif // KDE 4: remove end

void KAction::updateShortcut( int i )
{
  int id = itemId( i );

  TQWidget* w = container( i );
  if ( ::tqqt_cast<TQPopupMenu *>( w ) ) {
    TQPopupMenu* menu = static_cast<TQPopupMenu*>(w);
    updateShortcut( menu, id );
  }
  else if ( ::tqqt_cast<TQMenuBar *>( w ) )
    static_cast<TQMenuBar*>(w)->setAccel( d->m_cut.keyCodeQt(), id );
}

void KAction::updateShortcut( TQPopupMenu* menu, int id )
{
  //kdDebug(129) << "KAction::updateShortcut(): this = " << this << " d->m_kaccelList.count() = " << d->m_kaccelList.count() << endl;
  // If the action has a KAccel object,
  //  show the string representation of its shortcut.
  if ( d->m_kaccel || d->m_kaccelList.count() ) {
    TQString s = menu->text( id );
    int i = s.tqfind( '\t' );
    if ( i >= 0 )
      s.replace( i+1, s.length()-i, d->m_cut.seq(0).toString() );
    else
      s += "\t" + d->m_cut.seq(0).toString();

    menu->changeItem( id, s );
  }
  // Otherwise insert the shortcut itself into the popup menu.
  else {
    // This is a fall-hack in case the KAction is missing a proper parent collection.
    //  It should be removed eventually. --ellis
    menu->setAccel( d->m_cut.keyCodeQt(), id );
    kdDebug(129) << "KAction::updateShortcut(): name = \"" << name() << "\", cut = " << d->m_cut.toStringInternal() << "; No KAccel, probably missing a parent collection." << endl;
  }
}

const KShortcut& KAction::shortcut() const
{
  return d->m_cut;
}

const KShortcut& KAction::shortcutDefault() const
{
  return d->m_cutDefault;
}

TQString KAction::shortcutText() const
{
  return d->m_cut.toStringInternal();
}

void KAction::setShortcutText( const TQString& s )
{
  setShortcut( KShortcut(s) );
}

#ifndef KDE_NO_COMPAT // Remove in KDE 4
int KAction::accel() const
{
  return d->m_cut.keyCodeQt();
}
#endif

void KAction::setGroup( const TQString& grp )
{
  d->m_group = grp;

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateGroup( i );
}

void KAction::updateGroup( int )
{
  // DO SOMETHING
}

TQString KAction::group() const
{
  return d->m_group;
}

bool KAction::isEnabled() const
{
  return d->isEnabled();
}

bool KAction::isShortcutConfigurable() const
{
  return d->m_configurable;
}

void KAction::setToolTip( const TQString& tt )
{
  d->setToolTip( tt );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateToolTip( i );
}

void KAction::updateToolTip( int i )
{
  TQWidget *w = container( i );

  if ( ::tqqt_cast<KToolBar *>( w ) )
    TQToolTip::add( static_cast<KToolBar*>(w)->getWidget( itemId( i ) ), d->toolTip() );
}

TQString KAction::toolTip() const
{
  return d->toolTip();
}

int KAction::plug( TQWidget *w, int index )
{
  //kdDebug(129) << "KAction::plug( " << w << ", " << index << " )" << endl;
  if (!w ) {
	kdWarning(129) << "KAction::plug called with 0 argument\n";
 	return -1;
  }

  // Ellis: print warning if there is a shortcut, but no KAccel available (often due to no widget available in the actioncollection)
  // David: Well, it doesn't matter much, things still work (e.g. Undo in koffice) via TQAccel.
  // We should probably re-enable the warning for things that only KAccel can do, though - e.g. WIN key (mapped to Meta).
#if 0 //ndef NDEBUG
  KAccel* kaccel = kaccelCurrent();
  if( !d->m_cut.isNull() && !kaccel ) {
    kdDebug(129) << "KAction::plug(): has no KAccel object; this = " << this << " name = " << name() << " parentCollection = " << m_parentCollection << endl; // ellis
  }
#endif

  // Check if action is permitted
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;

  plugShortcut();

  if ( ::tqqt_cast<TQPopupMenu *>( w ) )
  {
    TQPopupMenu* menu = static_cast<TQPopupMenu*>( w );
    int id;
    // Don't insert shortcut into menu if it's already in a KAccel object.
    int keyQt = (d->m_kaccelList.count() || d->m_kaccel) ? 0 : d->m_cut.keyCodeQt();

    if ( d->hasIcon() )
    {
        KInstance *instance;
        if ( m_parentCollection )
          instance = m_parentCollection->instance();
        else
          instance = KGlobal::instance();
        id = menu->insertItem( d->iconSet( KIcon::Small, 0, instance ), d->text(), this,//dsweet
                                 TQT_SLOT( slotPopupActivated() ), keyQt,
                                 -1, index );
    }
    else
        id = menu->insertItem( d->text(), this,
                               TQT_SLOT( slotPopupActivated() ),
                               keyQt, -1, index );

    // If the shortcut is already in a KAccel object, then
    //  we need to set the menu item's shortcut text.
    if ( d->m_kaccelList.count() || d->m_kaccel )
        updateShortcut( menu, id );

    // call setItemEnabled only if the item really should be disabled,
    // because that method is slow and the item is per default enabled
    if ( !d->isEnabled() )
        menu->setItemEnabled( id, false );

    if ( !d->whatsThis().isEmpty() )
        menu->TQMenuData::setWhatsThis( id, whatsThisWithIcon() );

    addContainer( menu, id );
    connect( menu, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( menu, this );

    return d->m_containers.count() - 1;
  }
  else if ( ::tqqt_cast<KToolBar *>( w ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( w );

    int id_ = getToolButtonID();
    KInstance *instance;
    if ( m_parentCollection )
      instance = m_parentCollection->instance();
    else
      instance = KGlobal::instance();

    if ( icon().isEmpty() && !iconSet().pixmap().isNull() ) // old code using TQIconSet directly
    {
        bar->insertButton( iconSet().pixmap(), id_, TQT_SIGNAL( buttonClicked(int, TQt::ButtonState) ), this,
                           TQT_SLOT( slotButtonClicked(int, TQt::ButtonState) ),
                           d->isEnabled(), d->plainText(), index );
    }
    else
    {
        TQString icon = d->iconName();
        if ( icon.isEmpty() )
            icon = "unknown";
        bar->insertButton( icon, id_, TQT_SIGNAL( buttonClicked(int, TQt::ButtonState) ), this,
                           TQT_SLOT( slotButtonClicked(int, TQt::ButtonState) ),
                           d->isEnabled(), d->plainText(), index, instance );
    }

    KToolBarButton* ktb = bar->getButton(id_);
    ktb->setName( TQCString("toolbutton_")+name() );

    if ( !d->whatsThis().isEmpty() )
        TQWhatsThis::add( bar->getButton(id_), whatsThisWithIcon() );

    if ( !d->toolTip().isEmpty() )
      TQToolTip::add( bar->getButton(id_), d->toolTip() );

    addContainer( bar, id_ );

    connect( bar, TQT_SIGNAL( destroyed() ), this, TQT_SLOT( slotDestroyed() ) );

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( bar, this );

    return containerCount() - 1;
  }

  return -1;
}

void KAction::unplug( TQWidget *w )
{
  int i = findContainer( w );
  if ( i == -1 )
    return;
  int id = itemId( i );

  if ( ::tqqt_cast<TQPopupMenu *>( w ) )
  {
    TQPopupMenu *menu = static_cast<TQPopupMenu *>( w );
    menu->removeItem( id );
  }
  else if ( ::tqqt_cast<KToolBar *>( w ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( w );
    bar->removeItemDelayed( id );
  }
  else if ( ::tqqt_cast<TQMenuBar *>( w ) )
  {
    TQMenuBar *bar = static_cast<TQMenuBar *>( w );
    bar->removeItem( id );
  }

  removeContainer( i );
  if ( m_parentCollection )
    m_parentCollection->disconnectHighlight( w, this );
}

void KAction::plugAccel(KAccel *kacc, bool configurable)
{
  kdWarning(129) << "KAction::plugAccel(): call to deprecated action." << endl;
  kdDebug(129) << kdBacktrace() << endl;
  //kdDebug(129) << "KAction::plugAccel( kacc = " << kacc << " ): name \"" << name() << "\"" << endl;
  if ( d->m_kaccel )
    unplugAccel();

  // If the parent collection's accel ptr isn't set yet
  //if ( m_parentCollection && !m_parentCollection->accel() )
  //  m_parentCollection->setAccel( kacc );

  // We can only plug this action into the given KAccel object
  //  if it does not already contain an action with the same name.
  if ( !kacc->actions().actionPtr(name()) )
  {
    d->m_kaccel = kacc;
    d->m_kaccel->insert(name(), d->plainText(), TQString::null,
        KShortcut(d->m_cut),
        this, TQT_SLOT(slotActivated()),
        configurable, isEnabled());
    connect(d->m_kaccel, TQT_SIGNAL(destroyed()), this, TQT_SLOT(slotDestroyed()));
    //connect(d->m_kaccel, TQT_SIGNAL(keycodeChanged()), this, TQT_SLOT(slotKeycodeChanged()));
  }
  else
    kdWarning(129) << "KAction::plugAccel( kacc = " << kacc << " ): KAccel object already contains an action name \"" << name() << "\"" << endl; // -- ellis
}

void KAction::unplugAccel()
{
  //kdDebug(129) << "KAction::unplugAccel() " << this << " " << name() << endl;
  if ( d->m_kaccel )
  {
    d->m_kaccel->remove(name());
    d->m_kaccel = 0;
  }
}

void KAction::plugMainWindowAccel( TQWidget *w )
{
  // Note: tqtopLevelWidget() stops too early, we can't use it.
  TQWidget * tl = w;
  TQWidget * n;
  while ( !tl->isDialog() && ( n = tl->tqparentWidget() ) ) // lookup parent and store
    tl = n;

  KMainWindow * mw = dynamic_cast<KMainWindow *>(tl); // try to see if it's a kmainwindow
  if (mw)
    plugAccel( mw->accel() );
  else
    kdDebug(129) << "KAction::plugMainWindowAccel: Toplevel widget isn't a KMainWindow, can't plug accel. " << tl << endl;
}

void KAction::setEnabled(bool enable)
{
  //kdDebug(129) << "KAction::setEnabled( " << enable << " ): this = " << this << " d->m_kaccelList.count() = " << d->m_kaccelList.count() << endl;
  if ( enable == d->isEnabled() )
    return;

#ifndef KDE_NO_COMPAT
  // KDE 4: remove
  if (d->m_kaccel)
    d->m_kaccel->setEnabled(name(), enable);
#endif  // KDE 4: remove end

  const TQValueList<KAccel*> & accelList = d->m_kaccelList;
  TQValueList<KAccel*>::const_iterator itr = accelList.constBegin();
  const TQValueList<KAccel*>::const_iterator itrEnd = accelList.constEnd();

  const char * const namePtr = name();

  for ( ; itr != itrEnd; ++itr )
    (*itr)->setEnabled( namePtr, enable );

  d->setEnabled( enable );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateEnabled( i );

  emit enabled( d->isEnabled() );
}

void KAction::updateEnabled( int i )
{
    TQWidget *w = container( i );

    if ( ::tqqt_cast<TQPopupMenu *>( w ) )
      static_cast<TQPopupMenu*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
    else if ( ::tqqt_cast<TQMenuBar *>( w ) )
      static_cast<TQMenuBar*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
    else if ( ::tqqt_cast<KToolBar *>( w ) )
      static_cast<KToolBar*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
}

void KAction::setShortcutConfigurable( bool b )
{
    d->m_configurable = b;
}

void KAction::setText( const TQString& text )
{
#ifndef KDE_NO_COMPAT
  // KDE 4: remove
  if (d->m_kaccel) {
    KAccelAction* pAction = d->m_kaccel->actions().actionPtr(name());
    if (pAction)
      pAction->setLabel( text );
  }
#endif  // KDE 4: remove end
  const TQValueList<KAccel*> & accelList = d->m_kaccelList;
  TQValueList<KAccel*>::const_iterator itr = accelList.constBegin();
  const TQValueList<KAccel*>::const_iterator itrEnd = accelList.constEnd();

  const char * const namePtr = name();

  for( ; itr != itrEnd; ++itr ) {
    KAccelAction* const pAction = (*itr)->actions().actionPtr(namePtr);
    if (pAction)
      pAction->setLabel( text );
  }

  d->setText( text );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateText( i );
}

void KAction::updateText( int i )
{
  TQWidget *w = container( i );

  if ( ::tqqt_cast<TQPopupMenu *>( w ) ) {
    int id = itemId( i );
    static_cast<TQPopupMenu*>(w)->changeItem( id, d->text() );
    if (!d->m_cut.isNull())
      updateShortcut( static_cast<TQPopupMenu*>(w), id );
  }
  else if ( ::tqqt_cast<TQMenuBar *>( w ) )
    static_cast<TQMenuBar*>(w)->changeItem( itemId( i ), d->text() );
  else if ( ::tqqt_cast<KToolBar *>( w ) )
  {
    TQWidget *button = static_cast<KToolBar *>(w)->getWidget( itemId( i ) );
    if ( ::tqqt_cast<KToolBarButton *>( button ) )
      static_cast<KToolBarButton *>(button)->setText( d->plainText() );
  }
}

TQString KAction::text() const
{
  return d->text();
}

TQString KAction::plainText() const
{
  return d->plainText( );
}

void KAction::setIcon( const TQString &icon )
{
  d->setIconName( icon );

  // now handle any toolbars
  int len = containerCount();
  for ( int i = 0; i < len; ++i )
    updateIcon( i );
}

void KAction::updateIcon( int id )
{
  TQWidget* w = container( id );

  if ( ::tqqt_cast<TQPopupMenu *>( w ) ) {
    int itemId_ = itemId( id );
    static_cast<TQPopupMenu*>(w)->changeItem( itemId_, d->iconSet( KIcon::Small ), d->text() );
    if (!d->m_cut.isNull())
      updateShortcut( static_cast<TQPopupMenu*>(w), itemId_ );
  }
  else if ( ::tqqt_cast<TQMenuBar *>( w ) )
    static_cast<TQMenuBar*>(w)->changeItem( itemId( id ), d->iconSet( KIcon::Small ), d->text() );
  else if ( ::tqqt_cast<KToolBar *>( w ) )
    static_cast<KToolBar *>(w)->setButtonIcon( itemId( id ), d->iconName() );
}

TQString KAction::icon() const
{
  return d->iconName( );
}

void KAction::setIconSet( const TQIconSet &iconset )
{
  d->setIconSet( iconset );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateIconSet( i );
}


void KAction::updateIconSet( int id )
{
  TQWidget *w = container( id );

  if ( ::tqqt_cast<TQPopupMenu *>( w ) )
  {
    int itemId_ = itemId( id );
    static_cast<TQPopupMenu*>(w)->changeItem( itemId_, d->iconSet(), d->text() );
    if (!d->m_cut.isNull())
      updateShortcut( static_cast<TQPopupMenu*>(w), itemId_ );
  }
  else if ( ::tqqt_cast<TQMenuBar *>( w ) )
    static_cast<TQMenuBar*>(w)->changeItem( itemId( id ), d->iconSet(), d->text() );
  else if ( ::tqqt_cast<KToolBar *>( w ) )
  {
    if ( icon().isEmpty() && d->hasIcon() ) // only if there is no named icon ( scales better )
      static_cast<KToolBar *>(w)->setButtonIconSet( itemId( id ), d->iconSet() );
    else
      static_cast<KToolBar *>(w)->setButtonIconSet( itemId( id ), d->iconSet( KIcon::Small ) );
  }
}

TQIconSet KAction::iconSet( KIcon::Group group, int size ) const
{
    return d->iconSet( group, size );
}

bool KAction::hasIcon() const
{
  return d->hasIcon();
}

void KAction::setWhatsThis( const TQString& text )
{
  d->setWhatsThis(  text );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateWhatsThis( i );
}

void KAction::updateWhatsThis( int i )
{
  TQPopupMenu* pm = popupMenu( i );
  if ( pm )
  {
    pm->TQMenuData::setWhatsThis( itemId( i ), d->whatsThis() );
    return;
  }

  KToolBar *tb = toolBar( i );
  if ( tb )
  {
    TQWidget *w = tb->getButton( itemId( i ) );
    TQWhatsThis::remove( w );
    TQWhatsThis::add( w, d->whatsThis() );
    return;
  }
}

TQString KAction::whatsThis() const
{
  return d->whatsThis();
}

TQString KAction::whatsThisWithIcon() const
{
    TQString text = whatsThis();
    if (!d->iconName().isEmpty())
      return TQString::tqfromLatin1("<img source=\"small|%1\"> %2").arg(d->iconName() ).arg(text);
    return text;
}

TQWidget* KAction::container( int index ) const
{
  assert( index < containerCount() );
  return d->m_containers[ index ].m_container;
}

KToolBar* KAction::toolBar( int index ) const
{
    return dynamic_cast<KToolBar *>( d->m_containers[ index ].m_container );
}

TQPopupMenu* KAction::popupMenu( int index ) const
{
    return dynamic_cast<TQPopupMenu *>( d->m_containers[ index ].m_container );
}

TQWidget* KAction::representative( int index ) const
{
  return d->m_containers[ index ].m_representative;
}

int KAction::itemId( int index ) const
{
  return d->m_containers[ index ].m_id;
}

int KAction::containerCount() const
{
  return d->m_containers.count();
}

uint KAction::kaccelCount() const
{
  return d->m_kaccelList.count();
}

void KAction::addContainer( TQWidget* c, int id )
{
  KActionPrivate::Container p;
  p.m_container = c;
  p.m_id = id;
  d->m_containers.append( p );
}

void KAction::addContainer( TQWidget* c, TQWidget* w )
{
  KActionPrivate::Container p;
  p.m_container = c;
  p.m_representative = w;
  d->m_containers.append( p );
}

void KAction::activate()
{
  emit activated( KAction::EmulatedActivation, Qt::NoButton );
  slotActivated();
}

void KAction::slotActivated()
{
  const TQObject *senderObj = TQT_TQOBJECT_CONST(sender());
  if ( senderObj )
  {
    if ( ::tqqt_cast<KAccelPrivate *>( senderObj ) )
        emit activated( KAction::AccelActivation, Qt::NoButton );
  }
  emit activated();
}

// This catches signals emitted by KActions inserted into QPopupMenu
// We do crude things inside it, because we need to know which
// TQPopupMenu emitted the signal. We need to be sure that it is
// only called by QPopupMenus, we plugged us in.
void KAction::slotPopupActivated()
{
  if( ::tqqt_cast<TQSignal *>(sender()))
  {
    int id = dynamic_cast<const TQSignal *>(sender())->value().toInt();
    int pos = findContainer(id);
    if(pos != -1)
    {
      TQPopupMenu* qpm = dynamic_cast<TQPopupMenu *>( container(pos) );
      if(qpm)
      {
        KPopupMenu* kpm = dynamic_cast<KPopupMenu *>( qpm );
        TQt::ButtonState state;
        if ( kpm ) // KPopupMenu? Nice, it stores the state.
            state = kpm->state();
        else { // just a QPopupMenu? We'll ask for the state now then (small race condition?)
            kdDebug(129) << "KAction::slotPopupActivated not a KPopupMenu -> using keyboardMouseState()" << endl;
            state = KApplication::keyboardMouseState();
        }
        emit activated( KAction::PopupMenuActivation, state );
        slotActivated();
        return;
      }
    }
  }

  kdWarning(129)<<"Don't connect KAction::slotPopupActivated() to anything, expect into QPopupMenus which are in containers. Use slotActivated instead."<<endl;
  emit activated( KAction::PopupMenuActivation, Qt::NoButton );
  slotActivated();
}

void KAction::slotButtonClicked( int, TQt::ButtonState state )
{
  kdDebug(129) << "slotButtonClicked() state=" << state << endl;
  emit activated( KAction::ToolBarActivation, state );

  // RightButton isn't really an activation
  if ( ( state & Qt::LeftButton ) || ( state & Qt::MidButton ) )
    slotActivated();
}


void KAction::slotDestroyed()
{
  kdDebug(129) << "KAction::slotDestroyed(): this = " << this << ", name = \"" << name() << "\", sender = " << sender() << endl;
  const TQObject* const o = TQT_TQOBJECT_CONST(sender());

#ifndef KDE_NO_COMPAT  // KDE 4: remove
  if ( o == d->m_kaccel )
  {
    d->m_kaccel = 0;
    return;
  }
#endif  // KDE 4: remove end
  TQValueList<KAccel*> & accelList = d->m_kaccelList;
  TQValueList<KAccel*>::iterator itr = accelList.begin();
  const TQValueList<KAccel*>::iterator itrEnd = accelList.end();

  for( ; itr != itrEnd; ++itr)
  {
    if ( o == *itr )
    {
      disconnect( *itr, TQT_SIGNAL(destroyed()), this, TQT_SLOT(slotDestroyed()) );
      accelList.remove(itr);
      return;
    }
  }

  int i;
  do
  {
    i = findContainer( TQT_TQWIDGET_CONST( static_cast<const QObject*>(o) ) );
    if ( i != -1 )
      removeContainer( i );
  } while ( i != -1 );
}

int KAction::findContainer( const TQWidget* widget ) const
{
  int pos = 0;

  const TQValueList<KActionPrivate::Container> & containers = d->m_containers;

  TQValueList<KActionPrivate::Container>::ConstIterator it = containers.constBegin();
  const TQValueList<KActionPrivate::Container>::ConstIterator itEnd = containers.constEnd();

  while( it != itEnd )
  {
    if ( (*it).m_representative == widget || (*it).m_container == widget )
      return pos;
    ++it;
    ++pos;
  }

  return -1;
}

int KAction::findContainer( const int id ) const
{
  int pos = 0;

  const TQValueList<KActionPrivate::Container> & containers = d->m_containers;

  TQValueList<KActionPrivate::Container>::ConstIterator it = containers.constBegin();
  const TQValueList<KActionPrivate::Container>::ConstIterator itEnd = containers.constEnd();

  while( it != itEnd )
  {
    if ( (*it).m_id == id )
      return pos;
    ++it;
    ++pos;
  }

  return -1;
}

void KAction::removeContainer( int index )
{
  int i = 0;

  TQValueList<KActionPrivate::Container> & containers = d->m_containers;

  TQValueList<KActionPrivate::Container>::Iterator it = containers.begin();
  const TQValueList<KActionPrivate::Container>::Iterator itEnd = containers.end();

  while( it != itEnd )
  {
    if ( i == index )
    {
      containers.remove( it );
      return;
    }
    ++it;
    ++i;
  }
}

// FIXME: Remove this (ellis)
void KAction::slotKeycodeChanged()
{
  kdDebug(129) << "KAction::slotKeycodeChanged()" << endl; // -- ellis
  KAccelAction* pAction = d->m_kaccel->actions().actionPtr(name());
  if( pAction )
    setShortcut(pAction->shortcut());
}

KActionCollection *KAction::parentCollection() const
{
    return m_parentCollection;
}

void KAction::unplugAll()
{
  while ( containerCount() != 0 )
    unplug( container( 0 ) );
}

const KGuiItem& KAction::guiItem() const
{
    return *d;
}

void KAction::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

/* vim: et sw=2 ts=2
 */

#include "kaction.moc"
