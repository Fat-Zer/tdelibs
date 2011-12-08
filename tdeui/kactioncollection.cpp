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

#include "kactioncollection.h"
#include "kactionshortcutlist.h"
#include "ktoolbar.h"
#include "kxmlguifactory.h"
#include "kxmlguiclient.h"

#include <kaccel.h>
#include <kaccelbase.h>
#include <kapplication.h>
#include <kdebug.h>

#include <tqpopupmenu.h>
#include <tqptrdict.h>
#include <tqvariant.h>

class KActionCollection::KActionCollectionPrivate
{
public:
  KActionCollectionPrivate()
  {
    m_instance = 0;
    //m_bOneKAccelOnly = false;
    //m_iWidgetCurrent = 0;
    m_bAutoConnectShortcuts = true;
    m_widget = 0;
    m_kaccel = m_builderKAccel = 0;
    m_dctHighlightContainers.setAutoDelete( true );
    m_highlight = false;
    m_currentHighlightAction = 0;
    m_statusCleared = true;
    m_parentGUIClient = 0L;
  }

  KInstance *m_instance;
  TQString m_sXMLFile;
  bool m_bAutoConnectShortcuts;
  //bool m_bOneKAccelOnly;
  //int m_iWidgetCurrent;
  //TQValueList<TQWidget*> m_widgetList;
  //TQValueList<KAccel*> m_kaccelList;
  TQValueList<KActionCollection*> m_docList;
  TQWidget *m_widget;
  KAccel *m_kaccel;
  KAccel *m_builderKAccel;

  TQAsciiDict<KAction> m_actionDict;
  TQPtrDict< TQPtrList<KAction> > m_dctHighlightContainers;
  bool m_highlight;
  KAction *m_currentHighlightAction;
  bool m_statusCleared;
  const KXMLGUIClient *m_parentGUIClient;
};

KActionCollection::KActionCollection( TQWidget *parent, const char *name,
                                      KInstance *instance )
  : TQObject( parent, name )
{
  kdDebug(129) << "KActionCollection::KActionCollection( " << parent << ", " << name << " ): this = " << this << endl; // ellis
  d = new KActionCollectionPrivate;
  if( parent )
    setWidget( parent );
  //d->m_bOneKAccelOnly = (d->m_kaccelList.count() > 0);
  setInstance( instance );
}


KActionCollection::KActionCollection( TQWidget *watch, TQObject* parent, const char *name,
                                      KInstance *instance )
  : TQObject( parent, name )
{
  kdDebug(129) << "KActionCollection::KActionCollection( " << watch << ", " << parent << ", " << name << " ): this = " << this << endl; //ellis
  d = new KActionCollectionPrivate;
  if( watch )
    setWidget( watch );
  //d->m_bOneKAccelOnly = (d->m_kaccelList.count() > 0);
  setInstance( instance );
}

#ifndef KDE_NO_COMPAT
// KDE 4: remove
KActionCollection::KActionCollection( TQObject *parent, const char *name,
                                      KInstance *instance )
  : TQObject( parent, name )
{
  kdWarning(129) << "KActionCollection::KActionCollection( TQObject *parent, const char *name, KInstance *instance )" << endl; //ellis
  kdDebug(129) << kdBacktrace() << endl;
  d = new KActionCollectionPrivate;
  TQWidget* w = tqt_dynamic_cast<TQWidget*>( parent );
  if( w )
    setWidget( w );
  //d->m_bOneKAccelOnly = (d->m_kaccelList.count() > 0);
  setInstance( instance );
}

KActionCollection::KActionCollection( const KActionCollection &copy )
    : TQObject()
{
  kdWarning(129) << "KActionCollection::KActionCollection( const KActionCollection & ): function is severely deprecated." << endl;
  d = new KActionCollectionPrivate;
  *this = copy;
}
#endif // KDE 4: remove end

KActionCollection::KActionCollection( const char *name, const KXMLGUIClient *parent )
    : TQObject( 0L, name )
{
  d = new KActionCollectionPrivate;
  d->m_parentGUIClient=parent;
  d->m_instance=parent->instance();
}


KActionCollection::~KActionCollection()
{
  kdDebug(129) << "KActionCollection::~KActionCollection(): this = " << this << endl;
  for ( TQAsciiDictIterator<KAction> it( d->m_actionDict ); it.current(); ++it ) {
    KAction* pAction = it.current();
    if ( pAction->m_parentCollection == this )
      pAction->m_parentCollection = 0L;
  }

  delete d->m_kaccel;
  delete d->m_builderKAccel;
  delete d; d = 0;
}

void KActionCollection::setWidget( TQWidget* w )
{
  //if ( d->m_actionDict.count() > 0 ) {
  //  kdError(129) << "KActionCollection::setWidget(): must be called before any actions are added to collection!" << endl;
  //  kdDebug(129) << kdBacktrace() << endl;
  //}
  //else
  if ( !d->m_widget ) {
    d->m_widget = w;
    d->m_kaccel = new KAccel( w, this, "KActionCollection-KAccel" );
  }
  else if ( d->m_widget != w )
    kdWarning(129) << "KActionCollection::setWidget(): tried to change widget from " << d->m_widget << " to " << w << endl;
}

void KActionCollection::setAutoConnectShortcuts( bool b )
{
  d->m_bAutoConnectShortcuts = b;
}

bool KActionCollection::isAutoConnectShortcuts()
{
  return d->m_bAutoConnectShortcuts;
}

bool KActionCollection::addDocCollection( KActionCollection* pDoc )
{
	d->m_docList.append( pDoc );
	return true;
}

void KActionCollection::beginXMLPlug( TQWidget *widget )
{
	kdDebug(129) << "KActionCollection::beginXMLPlug( buildWidget = " << widget << " ): this = " <<  this << " d->m_builderKAccel = " << d->m_builderKAccel << endl;

	if( widget && !d->m_builderKAccel ) {
            d->m_builderKAccel = new KAccel( widget, this, "KActionCollection-BuilderKAccel" );
	}
}

void KActionCollection::endXMLPlug()
{
	kdDebug(129) << "KActionCollection::endXMLPlug(): this = " <<  this << endl;
	//s_kaccelXML = 0;
}

void KActionCollection::prepareXMLUnplug()
{
	kdDebug(129) << "KActionCollection::prepareXMLUnplug(): this = " <<  this << endl;
	unplugShortcuts( d->m_kaccel );

	if( d->m_builderKAccel ) {
		unplugShortcuts( d->m_builderKAccel );
		delete d->m_builderKAccel;
		d->m_builderKAccel = 0;
	}
}

void KActionCollection::unplugShortcuts( KAccel* kaccel )
{
  for ( TQAsciiDictIterator<KAction> it( d->m_actionDict ); it.current(); ++it ) {
    KAction* pAction = it.current();
    pAction->removeKAccel( kaccel );
  }

  for( uint i = 0; i < d->m_docList.count(); i++ )
    d->m_docList[i]->unplugShortcuts( kaccel );
}

/*void KActionCollection::addWidget( TQWidget* w )
{
  if( !d->m_bOneKAccelOnly ) {
    kdDebug(129) << "KActionCollection::addWidget( " << w << " ): this = " << this << endl;
    for( uint i = 0; i < d->m_widgetList.count(); i++ ) {
      if( d->m_widgetList[i] == w ) {
        d->m_iWidgetCurrent = i;
        return;
      }
  }
    d->m_iWidgetCurrent = d->m_widgetList.count();
    d->m_widgetList.append( w );
    d->m_kaccelList.append( new KAccel( w, this, "KActionCollection-KAccel" ) );
  }
}

void KActionCollection::removeWidget( TQWidget* w )
{
  if( !d->m_bOneKAccelOnly ) {
    kdDebug(129) << "KActionCollection::removeWidget( " << w << " ): this = " << this << endl;
    for( uint i = 0; i < d->m_widgetList.count(); i++ ) {
      if( d->m_widgetList[i] == w ) {
        // Remove KAccel object from children.
        KAccel* pKAccel = d->m_kaccelList[i];
        for ( TQAsciiDictIterator<KAction> it( d->m_actionDict ); it.current(); ++it ) {
          KAction* pAction = it.current();
          if ( pAction->m_parentCollection == this ) {
            pAction->removeKAccel( pKAccel );
          }
        }
        delete pKAccel;

        d->m_widgetList.remove( d->m_widgetList.tqat( i ) );
        d->m_kaccelList.remove( d->m_kaccelList.tqat( i ) );

        if( d->m_iWidgetCurrent == (int)i )
          d->m_iWidgetCurrent = -1;
        else if( d->m_iWidgetCurrent > (int)i )
          d->m_iWidgetCurrent--;
        return;
      }
    }
    kdWarning(129) << "KActionCollection::removeWidget( " << w << " ): widget not in list." << endl;
  }
}

bool KActionCollection::ownsKAccel() const
{
  return d->m_bOneKAccelOnly;
}

uint KActionCollection::widgetCount() const
{
  return d->m_widgetList.count();
}

const KAccel* KActionCollection::widgetKAccel( uint i ) const
{
  return d->m_kaccelList[i];
}*/

KAccel* KActionCollection::kaccel()
{
  //if( d->m_kaccelList.count() > 0 )
  //  return d->m_kaccelList[d->m_iWidgetCurrent];
  //else
  //  return 0;
  return d->m_kaccel;
}

const KAccel* KActionCollection::kaccel() const
{
  //if( d->m_kaccelList.count() > 0 )
  //  return d->m_kaccelList[d->m_iWidgetCurrent];
  //else
  //  return 0;
  return d->m_kaccel;
}

// Return the key to use in d->m_actionDict for the given action.
// Usually name(), except when unnamed.
static const char* actionDictKey( KAction* action, char* buffer )
{
  const char* name = action->name();
  if( !qstrcmp( name, "unnamed" ) )
  {
     sprintf(buffer, "unnamed-%p", (void *)action);
     return buffer;
  }
  return name;
}

void KActionCollection::_insert( KAction* action )
{
  char unnamed_name[100];
  const char *name = actionDictKey( action, unnamed_name );
  KAction *a = d->m_actionDict[ name ];
  if ( a == action )
      return;

  d->m_actionDict.insert( name, action );

  emit inserted( action );
}

void KActionCollection::_remove( KAction* action )
{
  char unnamed_name[100];
  const char *name = actionDictKey( action, unnamed_name );

  KAction *a = d->m_actionDict.take( name );
  if ( !a || a != action )
      return;

  emit removed( action );
  // note that we delete the action without its parent collection set to 0.
  // This triggers kaccel::remove, to remove any shortcut.
  delete a;
}

KAction* KActionCollection::_take( KAction* action )
{
  char unnamed_name[100];
  const char *name = actionDictKey( action, unnamed_name );

  KAction *a = d->m_actionDict.take( name );
  if ( !a || a != action )
      return 0;

  if ( a->m_parentCollection == this )
      a->m_parentCollection = 0;

  emit removed( action );

  return a;
}

void KActionCollection::_clear()
{
  TQAsciiDictIterator<KAction> it( d->m_actionDict );
  while ( it.current() )
    _remove( it.current() );
}

void KActionCollection::insert( KAction* action )   { _insert( action ); }
void KActionCollection::remove( KAction* action )   { _remove( action ); }
KAction* KActionCollection::take( KAction* action ) { return _take( action ); }
void KActionCollection::clear()                     { _clear(); }
KAccel* KActionCollection::accel()                  { return kaccel(); }
const KAccel* KActionCollection::accel() const      { return kaccel(); }
KAccel* KActionCollection::builderKAccel() const    { return d->m_builderKAccel; }

KAction* KActionCollection::action( const char* name, const char* classname ) const
{
  KAction* pAction = 0;

  if ( !classname && name )
    pAction = d->m_actionDict[ name ];

  else {
    TQAsciiDictIterator<KAction> it( d->m_actionDict );
    for( ; it.current(); ++it )
    {
      if ( ( !name || !strcmp( it.current()->name(), name ) ) &&
          ( !classname || !strcmp( it.current()->className(), classname ) ) ) {
        pAction = it.current();
        break;
      }
    }
  }

  if( !pAction ) {
    for( uint i = 0; i < d->m_docList.count() && !pAction; i++ )
      pAction = d->m_docList[i]->action( name, classname );
  }

  return pAction;
}

KAction* KActionCollection::action( int index ) const
{
  TQAsciiDictIterator<KAction> it( d->m_actionDict );
  it += index;
  return it.current();
//  return d->m_actions.tqat( index );
}

bool KActionCollection::readShortcutSettings( const TQString& sConfigGroup, KConfigBase* pConfig )
{
  return KActionShortcutList(this).readSettings( sConfigGroup, pConfig );
}

bool KActionCollection::writeShortcutSettings( const TQString& sConfigGroup, KConfigBase* pConfig ) const
{
  return KActionShortcutList((KActionCollection*)this).writeSettings( sConfigGroup, pConfig );
}

uint KActionCollection::count() const
{
  return d->m_actionDict.count();
}

TQStringList KActionCollection::groups() const
{
  TQStringList lst;

  TQAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    if ( !it.current()->group().isEmpty() && !lst.contains( it.current()->group() ) )
      lst.append( it.current()->group() );

  return lst;
}

KActionPtrList KActionCollection::actions( const TQString& group ) const
{
  KActionPtrList lst;

  TQAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    if ( it.current()->group() == group )
      lst.append( it.current() );
    else if ( it.current()->group().isEmpty() && group.isEmpty() )
      lst.append( it.current() );

  return lst;
}

KActionPtrList KActionCollection::actions() const
{
  KActionPtrList lst;

  TQAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    lst.append( it.current() );

  return lst;
}

void KActionCollection::setInstance( KInstance *instance )
{
  if ( instance )
    d->m_instance = instance;
  else
    d->m_instance = KGlobal::instance();
}

KInstance *KActionCollection::instance() const
{
  return d->m_instance;
}

void KActionCollection::setXMLFile( const TQString& sXMLFile )
{
  d->m_sXMLFile = sXMLFile;
}

const TQString& KActionCollection::xmlFile() const
{
  return d->m_sXMLFile;
}

void KActionCollection::setHighlightingEnabled( bool enable )
{
  d->m_highlight = enable;
}

bool KActionCollection::highlightingEnabled() const
{
  return d->m_highlight;
}

void KActionCollection::connectHighlight( TQWidget *container, KAction *action )
{
  if ( !d->m_highlight )
    return;

  TQPtrList<KAction> *actionList = d->m_dctHighlightContainers[ container ];

  if ( !actionList )
  {
    actionList = new TQPtrList<KAction>;

    if ( ::tqqt_cast<TQPopupMenu *>( container ) )
    {
      connect( container, TQT_SIGNAL( highlighted( int ) ),
               this, TQT_SLOT( slotMenuItemHighlighted( int ) ) );
      connect( container, TQT_SIGNAL( aboutToHide() ),
               this, TQT_SLOT( slotMenuAboutToHide() ) );
    }
    else if ( ::tqqt_cast<KToolBar *>( container ) )
    {
      connect( container, TQT_SIGNAL( highlighted( int, bool ) ),
               this, TQT_SLOT( slotToolBarButtonHighlighted( int, bool ) ) );
    }

    connect( container, TQT_SIGNAL( destroyed() ),
             this, TQT_SLOT( slotDestroyed() ) );

    d->m_dctHighlightContainers.insert( container, actionList );
  }

  actionList->append( action );
}

void KActionCollection::disconnectHighlight( TQWidget *container, KAction *action )
{
  if ( !d->m_highlight )
    return;

  TQPtrList<KAction> *actionList = d->m_dctHighlightContainers[ container ];

  if ( !actionList )
    return;

  actionList->removeRef( action );

  if ( actionList->isEmpty() )
    d->m_dctHighlightContainers.remove( container );
}

void KActionCollection::slotMenuItemHighlighted( int id )
{
  if ( !d->m_highlight )
    return;

  if ( d->m_currentHighlightAction )
    emit actionHighlighted( d->m_currentHighlightAction, false );

  TQWidget *container = const_cast<TQWidget*>(TQT_TQWIDGET_CONST( sender() ));

  d->m_currentHighlightAction = findAction( container, id );

  if ( !d->m_currentHighlightAction )
  {
      if ( !d->m_statusCleared )
          emit clearStatusText();
      d->m_statusCleared = true;
      return;
  }

  d->m_statusCleared = false;
  emit actionHighlighted( d->m_currentHighlightAction );
  emit actionHighlighted( d->m_currentHighlightAction, true );
  emit actionStatusText( d->m_currentHighlightAction->toolTip() );
}

void KActionCollection::slotMenuAboutToHide()
{
    if ( d->m_currentHighlightAction )
        emit actionHighlighted( d->m_currentHighlightAction, false );
    d->m_currentHighlightAction = 0;

    if ( !d->m_statusCleared )
        emit clearStatusText();
    d->m_statusCleared = true;
}

void KActionCollection::slotToolBarButtonHighlighted( int id, bool highlight )
{
  if ( !d->m_highlight )
    return;

  TQWidget *container = const_cast<TQWidget*>(TQT_TQWIDGET_CONST( sender() ));

  KAction *action = findAction( container, id );

  if ( !action )
  {
      d->m_currentHighlightAction = 0;
      // use tooltip groups for toolbar status text stuff instead (Simon)
//      emit clearStatusText();
      return;
  }

  emit actionHighlighted( action, highlight );

  if ( highlight )
    d->m_currentHighlightAction = action;
  else
  {
    d->m_currentHighlightAction = 0;
//    emit clearStatusText();
  }
}

void KActionCollection::slotDestroyed()
{
    d->m_dctHighlightContainers.remove( reinterpret_cast<void *>( const_cast<TQObject*>(TQT_TQOBJECT_CONST(sender())) ) );
}

KAction *KActionCollection::findAction( TQWidget *container, int id )
{
  TQPtrList<KAction> *actionList = d->m_dctHighlightContainers[ reinterpret_cast<void *>( container ) ];

  if ( !actionList )
    return 0;

  TQPtrListIterator<KAction> it( *actionList );
  for (; it.current(); ++it )
    if ( it.current()->isPlugged( container, id ) )
      return it.current();

  return 0;
}

const KXMLGUIClient *KActionCollection::parentGUIClient() const
{
	return d->m_parentGUIClient;
}

#ifndef KDE_NO_COMPAT
// KDE 4: remove
KActionCollection KActionCollection::operator+(const KActionCollection &c ) const
{
  kdWarning(129) << "KActionCollection::operator+(): function is severely deprecated." << endl;
  KActionCollection ret( *this );

  TQValueList<KAction *> actions = c.actions();
  TQValueList<KAction *>::ConstIterator it = actions.begin();
  TQValueList<KAction *>::ConstIterator end = actions.end();
  for (; it != end; ++it )
    ret.insert( *it );

  return ret;
}

KActionCollection &KActionCollection::operator=( const KActionCollection &copy )
{
  kdWarning(129) << "KActionCollection::operator=(): function is severely deprecated." << endl;
  //d->m_bOneKAccelOnly = copy.d->m_bOneKAccelOnly;
  //d->m_iWidgetCurrent = copy.d->m_iWidgetCurrent;
  //d->m_widgetList = copy.d->m_widgetList;
  //d->m_kaccelList = copy.d->m_kaccelList;
  d->m_widget = copy.d->m_widget;
  d->m_kaccel = copy.d->m_kaccel;
  d->m_actionDict = copy.d->m_actionDict;
  setInstance( copy.instance() );
  return *this;
}

KActionCollection &KActionCollection::operator+=( const KActionCollection &c )
{
  kdWarning(129) << "KActionCollection::operator+=(): function is severely deprecated." << endl;
  TQAsciiDictIterator<KAction> it(c.d->m_actionDict);
  for ( ; it.current(); ++it )
    insert( it.current() );

  return *this;
}
#endif // KDE 4: remove end

//---------------------------------------------------------------------
// KActionShortcutList
//---------------------------------------------------------------------

KActionShortcutList::KActionShortcutList( KActionCollection* pColl )
: m_actions( *pColl )
	{ }
KActionShortcutList::~KActionShortcutList()
	{ }
uint KActionShortcutList::count() const
	{ return m_actions.count(); }
TQString KActionShortcutList::name( uint i ) const
	{ return m_actions.action(i)->name(); }
TQString KActionShortcutList::label( uint i ) const
	{ return m_actions.action(i)->text(); }
TQString KActionShortcutList::whatsThis( uint i ) const
	{ return m_actions.action(i)->whatsThis(); }
const KShortcut& KActionShortcutList::shortcut( uint i ) const
	{ return m_actions.action(i)->shortcut(); }
const KShortcut& KActionShortcutList::shortcutDefault( uint i ) const
	{ return m_actions.action(i)->shortcutDefault(); }
bool KActionShortcutList::isConfigurable( uint i ) const
	{ return m_actions.action(i)->isShortcutConfigurable(); }
bool KActionShortcutList::setShortcut( uint i, const KShortcut& cut )
	{ return m_actions.action(i)->setShortcut( cut ); }
const KInstance* KActionShortcutList::instance() const
	{ return m_actions.instance(); }
TQVariant KActionShortcutList::getOther( Other, uint ) const
	{ return TQVariant(); }
bool KActionShortcutList::setOther( Other, uint, TQVariant )
	{ return false; }
const KAction *KActionShortcutList::action( uint i) const
	{ return m_actions.action(i); }

bool KActionShortcutList::save() const
{
	const KXMLGUIClient* guiClient=m_actions.parentGUIClient();
	const TQString xmlFile=guiClient ? guiClient->xmlFile() : m_actions.xmlFile();
	kdDebug(129) << "KActionShortcutList::save(): xmlFile = " << xmlFile << endl;

	if( m_actions.xmlFile().isEmpty() )
		return writeSettings();

	TQString attrShortcut  = TQString::tqfromLatin1("shortcut");
	TQString attrAccel     = TQString::tqfromLatin1("accel"); // Depricated attribute

	// Read XML file
	TQString sXml( KXMLGUIFactory::readConfigFile( xmlFile, false, instance() ) );
	TQDomDocument doc;
	doc.setContent( sXml );

	// Process XML data

        // Get hold of ActionProperties tag
        TQDomElement elem = KXMLGUIFactory::actionPropertiesElement( doc );

	// now, iterate through our actions
	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		const TQString& sName = name(i);

		bool bSameAsDefault = (shortcut(i) == shortcutDefault(i));
		//kdDebug(129) << "name = " << sName << " shortcut = " << shortcut(i).toStringInternal() << " def = " << shortcutDefault(i).toStringInternal() << endl;

		// now see if this element already exists
                // and create it if necessary (unless bSameAsDefault)
		TQDomElement act_elem = KXMLGUIFactory::findActionByName( elem, sName, !bSameAsDefault );
                if ( act_elem.isNull() )
                    continue;

		act_elem.removeAttribute( attrAccel );
		if( bSameAsDefault ) {
			act_elem.removeAttribute( attrShortcut );
			//kdDebug(129) << "act_elem.attributes().count() = " << act_elem.attributes().count() << endl;
			if( act_elem.attributes().count() == 1 )
				elem.removeChild( act_elem );
		} else {
			act_elem.setAttribute( attrShortcut, shortcut(i).toStringInternal() );
		}
	}

	// Write back to XML file
	return KXMLGUIFactory::saveConfigFile( doc, guiClient ? guiClient->localXMLFile() : m_actions.xmlFile(), instance() );
}

//---------------------------------------------------------------------
// KActionPtrShortcutList
//---------------------------------------------------------------------

KActionPtrShortcutList::KActionPtrShortcutList( KActionPtrList& list )
: m_actions( list )
	{ }
KActionPtrShortcutList::~KActionPtrShortcutList()
	{ }
uint KActionPtrShortcutList::count() const
	{ return m_actions.count(); }
TQString KActionPtrShortcutList::name( uint i ) const
	{ return m_actions[i]->name(); }
TQString KActionPtrShortcutList::label( uint i ) const
	{ return m_actions[i]->text(); }
TQString KActionPtrShortcutList::whatsThis( uint i ) const
	{ return m_actions[i]->whatsThis(); }
const KShortcut& KActionPtrShortcutList::shortcut( uint i ) const
	{ return m_actions[i]->shortcut(); }
const KShortcut& KActionPtrShortcutList::shortcutDefault( uint i ) const
	{ return m_actions[i]->shortcutDefault(); }
bool KActionPtrShortcutList::isConfigurable( uint i ) const
	{ return m_actions[i]->isShortcutConfigurable(); }
bool KActionPtrShortcutList::setShortcut( uint i, const KShortcut& cut )
	{ return m_actions[i]->setShortcut( cut ); }
TQVariant KActionPtrShortcutList::getOther( Other, uint ) const
	{ return TQVariant(); }
bool KActionPtrShortcutList::setOther( Other, uint, TQVariant )
	{ return false; }
bool KActionPtrShortcutList::save() const
	{ return false; }

void KActionShortcutList::virtual_hook( int id, void* data )
{ KShortcutList::virtual_hook( id, data ); }

void KActionPtrShortcutList::virtual_hook( int id, void* data )
{ KShortcutList::virtual_hook( id, data ); }

void KActionCollection::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

/* vim: et sw=2 ts=2
 */

#include "kactioncollection.moc"