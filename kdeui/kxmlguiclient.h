/* This file is part of the KDE libraries
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

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
#ifndef _KXMLGUICLIENT_H
#define _KXMLGUICLIENT_H

#include <tqdom.h>
#include <tqptrlist.h>
#include <tqmap.h>
#include <tqstringlist.h>

#include <kdelibs_export.h>

class TQWidget;
class KAction;
class KActionCollection;
class KInstance;
class KXMLGUIClientPrivate;
class KXMLGUIFactory;
class KXMLGUIBuilder;

/**
 *
 * A KXMLGUIClient can be used with KXMLGUIFactory to create a
 * GUI from actions and an XML document, and can be dynamically merged
 * with other KXMLGUIClients.
 */
class KDEUI_EXPORT KXMLGUIClient
{
    friend class KEditToolbarWidget; // for setXMLFile(3 args)
public:
  /**
   * Constructs a KXMLGUIClient which can be used with a
   * KXMLGUIFactory to create a GUI from actions and an XML document, and
   * which can be dynamically merged with other KXMLGUIClients.
   */
  KXMLGUIClient();

  /**
   * Constructs a KXMLGUIClient which can be used with a KXMLGUIFactory
   * to create a GUI from actions and an XML document,
   * and which can be dynamically merged with other KXMLGUIClients.
   *
   * This constructor takes an additional @p parent argument, which makes
   * the client a child client of the parent.
   *
   * Child clients are automatically added to the GUI if the parent is added.
   *
   */
  KXMLGUIClient( KXMLGUIClient *parent );

  /**
   * Destructs the KXMLGUIClient.
   */
  virtual ~KXMLGUIClient();

  /**
   * Retrieves an action of the client by name.  If not found, it looks in its child clients.
   * This method is provided for convenience, as it uses actionCollection()
   * to get the action object.
   */
  KAction* action( const char* name ) const;

  /**
   * Retrieves an action for a given TQDomElement. The default
   * implementation uses the "name" attribute to query the action
   * object via the other action() method.
   */
  virtual KAction *action( const TQDomElement &element ) const;

  /**
   * Retrieves the entire action collection for the GUI client. If
   * you subclass KXMLGUIClient you should call
   * KActionCollection::setWidget( TQWidget* ) with this object, or
   * you will encounter subtle bugs with KAction keyboard shortcuts.
   * This is not necessary if your KXMLGUIClient is a KMainWindow.
   *
   * @see KActionCollection::setWidget( TQWidget* )
   */
  virtual KActionCollection* actionCollection() const;

  /**
   * @return The instance ( KInstance ) for this GUI client.
   */
  virtual KInstance *instance() const;

  /**
   * @return The parsed XML in a TQDomDocument, set by
   * setXMLFile() or setXML().
   * This document describes the tqlayout of the GUI.
   */
  virtual TQDomDocument domDocument() const;

  /**
   * This will return the name of the XML file as set by setXMLFile().
   * If setXML() is used directly, then this will return NULL.
   *
   * The filename that this returns is obvious for components as each
   * component has exactly one XML file.  In non-components, however,
   * there are usually two: the global file and the local file.  This
   * function doesn't really care about that, though.  It will always
   * return the last XML file set.  This, in almost all cases, will
   * be the local XML file.
   *
   * @return The name of the XML file or TQString::null
   */
  virtual TQString xmlFile() const;

  virtual TQString localXMLFile() const;

  /**
   * @internal
   */
  void setXMLGUIBuildDocument( const TQDomDocument &doc );
  /**
   * @internal
   */
  TQDomDocument xmlguiBuildDocument() const;

  /**
   * This method is called by the KXMLGUIFactory as soon as the client
   * is added to the KXMLGUIFactory's GUI.
   */
  void setFactory( KXMLGUIFactory *factory );
  /**
   * Retrieves a pointer to the KXMLGUIFactory this client is
   * associated with (will return 0L if the client's GUI has not been built
   * by a KXMLGUIFactory.
   */
  KXMLGUIFactory *factory() const;

  /**
   * KXMLGUIClients can form a simple child/parent object tree. This
   * method returns a pointer to the parent client or 0L if it has no
   * parent client assigned.
   */
  KXMLGUIClient *parentClient() const;

  /**
   * Use this method to make a client a child client of another client.
   * Usually you don't need to call this method, as it is called
   * automatically when using the second constructor, which takes a
   * parent argument.
   */
  void insertChildClient( KXMLGUIClient *child );

  /**
   * Removes the given @p child from the client's children list.
   */
  void removeChildClient( KXMLGUIClient *child );

  /**
   * Retrieves a list of all child clients.
   */
  const TQPtrList<KXMLGUIClient> *childClients();

  /**
   * A client can have an own KXMLGUIBuilder.
   * Use this method to assign your builder instance to the client (so that the
   * KXMLGUIFactory can use it when building the client's GUI)
   *
   * Client specific guibuilders are useful if you want to create
   * custom container widgets for your GUI.
   */
  void setClientBuilder( KXMLGUIBuilder *builder );

  /**
   * Retrieves the client's GUI builder or 0L if no client specific
   * builder has been assigned via setClientBuilder()
   */
  KXMLGUIBuilder *clientBuilder() const;

  /**
   * Forces this client to re-read its XML resource file.  This is
   * intended to be used when you know that the resource file has
   * changed and you will soon be rebuilding the GUI.  It has no
   * useful effect with non-KParts GUIs, so don't bother using it
   * unless your app is component based.
   */
  void reloadXML();

  /**
   * ActionLists are a way for XMLGUI to support dynamic lists of
   * actions.  E.g. if you are writing a file manager, and there is a
   * menu file whose contents depend on the mimetype of the file that
   * is selected, then you can achieve this using ActionLists. It
   * works as follows:
   * In your xxxui.rc file ( the one that you set in setXMLFile()
   * ), you put an <p>\<ActionList name="xxx"\></p> tag.  E.g.
   * \verbatim
   * <kpartgui name="xxx_part" version="1">
   * <MenuBar>
   *   <Menu name="file">
   *     ...  <!-- some useful actions-->
   *     <ActionList name="xxx_file_actionlist" />
   *     ...  <!-- even more useful actions-->
   *   </Menu>
   *   ...
   * </MenuBar>
   * </kpartgui>
   * \endverbatim
   *
   * This tag will get expanded to a list of actions.  In the example
   * above ( a file manager with a dynamic file menu ), you would call
   * \code
   * TQPtrList<KAction> file_actions;
   * for( ... )
   *   if( ... )
   *     file_actions.append( cool_action );
   * unplugActionList( "xxx_file_actionlist" );
   * plugActionList( "xxx_file_actionlist", file_actions );
   * \endcode
   * every time a file is selected, unselected or ...
   *
   * \note You should not call createGUI() after calling this
   *       function.  In fact, that would remove the newly added
   *       actionlists again...
   * \note Forgetting to call unplugActionList() before
   *       plugActionList() would leave the previous actions in the
   *       menu too..
   */
  void plugActionList( const TQString &name, const TQPtrList<KAction> &actionList );

  /**
   * The complement of plugActionList() ...
   */
  void unplugActionList( const TQString &name );

  static TQString findMostRecentXMLFile( const TQStringList &files, TQString &doc );

  void addStateActionEnabled(const TQString& state, const TQString& action);

  void addStateActionDisabled(const TQString& state, const TQString& action);

  enum ReverseStateChange { StateNoReverse, StateReverse };
  struct StateChange
  {
    TQStringList actionsToEnable;
    TQStringList actionsToDisable;
  };

  StateChange getActionsToChangeForState(const TQString& state);

  /// @since 3.1
  void beginXMLPlug( TQWidget * );
  /// @since 3.1
  void endXMLPlug();
  /// @since 3.1
  void prepareXMLUnplug( TQWidget * );

protected:
  /**
   * Returns true if client was added to super client list.
   * Returns false if client was already in list.
   */
  //bool addSuperClient( KXMLGUIClient * );

  /**
   * Sets the instance ( KInstance) for this part.
   *
   * Call this first in the inherited class constructor.
   * (At least before setXMLFile().)
   */
  virtual void setInstance( KInstance *instance );

  /**
   * Sets the name of the rc file containing the XML for the part.
   *
   * Call this in the Part-inherited class constructor.
   *
   * @param file Either an absolute path for the file, or simply the
   *             filename, which will then be assumed to be installed
   *             in the "data" resource, under a directory named like
   *             the instance.
   * @param merge Whether to merge with the global document.
   * @param setXMLDoc Specify whether to call setXML. Default is true.
   *               and the DOM document at once.
   **/
  virtual void setXMLFile( const TQString& file, bool merge = false, bool setXMLDoc = true );

  virtual void setLocalXMLFile( const TQString &file );

  /**
   * Sets the XML for the part.
   *
   * Call this in the Part-inherited class constructor if you
   *  don't call setXMLFile().
   **/
  virtual void setXML( const TQString &document, bool merge = false );

  /**
   * Sets the Document for the part, describing the tqlayout of the GUI.
   *
   * Call this in the Part-inherited class constructor if you don't call
   * setXMLFile or setXML .
   */
  virtual void setDOMDocument( const TQDomDocument &document, bool merge = false );

  /**
   * This function will attempt to give up some memory after the GUI
   * is built.  It should never be used in apps where the GUI may be
   * rebuilt at some later time (components, for instance).
   */
  virtual void conserveMemory();

  /**
   * Actions can collectively be assigned a "State". To accomplish this
   * the respective actions are tagged as \<enable\> or \<disable\> in
   * a \<State\> \</State\> group of the XMLfile. During program execution the
   * programmer can call stateChanged() to set actions to a defined state.
   *
   * @param newstate Name of a State in the XMLfile.
   * @param reverse If the flag reverse is set to StateReverse, the State is reversed.
   * (actions to be enabled will be disabled and action to be disabled will be enabled)
   * Default is reverse=false.
   */
   virtual void stateChanged(const TQString &newstate, ReverseStateChange reverse = StateNoReverse);

   // Use this one for KDE 4.0
   //virtual void stateChanged(const TQString &newstate, bool reverse = false);

private:
  struct DocStruct
  {
    TQString file;
    TQString data;
  };

  bool mergeXML( TQDomElement &base, const TQDomElement &additive,
                 KActionCollection *actionCollection );

  TQDomElement tqfindMatchingElement( const TQDomElement &base,
                                   const TQDomElement &additive );

  typedef TQMap<TQString, TQMap<TQString, TQString> > ActionPropertiesMap;

  static ActionPropertiesMap extractActionProperties( const TQDomDocument &doc );

  static void storeActionProperties( TQDomDocument &doc, const ActionPropertiesMap &properties );

  static TQString tqfindVersionNumber( const TQString &_xml );

  // Actions to enable/disable on a state change
  TQMap<TQString,StateChange> m_actionsStateMap;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KXMLGUIClientPrivate *d;
};

#endif
