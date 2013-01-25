//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __kbookmarkmenu_h__
#define __kbookmarkmenu_h__

#include <sys/types.h>

#include <tqptrlist.h>
#include <tqptrstack.h>
#include <tqobject.h>
#include <tqlistview.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kaction.h>

#include "kbookmark.h"
#include "kbookmarkmanager.h"

class TQString;
class TQPopupMenu;
class TQPushButton;
class TQListView;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KPopupMenu;

namespace TDEIO { class Job; }

/**
 * This class provides a bookmark menu.  It is typically used in
 * cooperation with KActionMenu but doesn't have to be.
 *
 * If you use this class by itself, then it will use KDE defaults for
 * everything -- the bookmark path, bookmark editor, bookmark launcher..
 * everything.  These defaults reside in the classes
 * KBookmarkOwner (editing bookmarks) and KBookmarkManager
 * (almost everything else).  If you wish to change the defaults in
 * any way, you must reimplement and instantiate those classes
 * <em>before</em> this class is ever called.
 *
 * Using this class is very simple:
 *
 * 1) Create a popup menu (either KActionMenu or KPopupMenu will do)
 * 2) Instantiate a new KBookmarkMenu object using the above popup
 *    menu as a parameter
 * 3) Insert your (now full) popup menu wherever you wish
 *
 * Again, if you wish to modify any defaults, the procedure is:
 *
 * 1a) Reimplement your own KBookmarkOwner
 * 1b) Reimplement and instantiate your own KBookmarkManager
 */
class KIO_EXPORT KBookmarkMenu : public TQObject
{
  Q_OBJECT
  friend class KBookmarkMenuNSImporter;
  friend class RMB;
public:
  /**
   * Fills a bookmark menu
   * (one instance of KBookmarkMenu is created for the toplevel menu,
   *  but also one per submenu).
   *
   * @param mgr The bookmark manager to use (i.e. for reading and writing)
   * @param owner implementation of the KBookmarkOwner callback interface.
   * @param parentMenu menu to be filled
   * @param collec parent collection for the KActions. 
   *  Only used for other menus than the toplevel one.
   * @param root true for the toplevel menu
   * @param add true to show the "Add Bookmark" and "New Folder" entries
   * @param parentAddress the address of the group containing the items
   *  that we want to show.
   * @see KBookmark::address.
   * Be careful :
   * A _null_ parentAddress denotes a NS-bookmark menu.
   * An _empty_ parentAddress denotes the toplevel bookmark menu
   */
  KBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KPopupMenu * parentMenu,
                 KActionCollection * collec, bool root, bool add = true,
                 const TQString & parentAddress = "" );

  ~KBookmarkMenu();

  /**
   * Even if you think you need to use this, you are probably wrong.
   * It fills a bookmark menu starting a given KBookmark.
   * This is public for KBookmarkBar.
   */
  void fillBookmarkMenu();

  /**
   * Call ensureUpToDate() if you need KBookmarkMenu to adjust to its 
   * final size before it is executed.
   **/
  void ensureUpToDate();

  /**
   * Structure used for storing information about 
   * the dynamic menu setting
   * @since 3.2
   */
  // TODO - transform into class
  struct DynMenuInfo {
     bool show;
     TQString location;
     TQString type;
     TQString name;
     class DynMenuInfoPrivate *d;
  };

  /**
   * @return dynmenu info block for the given dynmenu name
   * @since 3.2
   */
  static DynMenuInfo showDynamicBookmarks( const TQString &id );

  /**
   * Shows an extra menu for the given bookmarks file and type. 
   * Upgrades from option inside XBEL to option in rc file
   * on first call of this function.
   * @param id the unique identification for the dynamic menu
   * @param info a DynMenuInfo struct containing the to be added/modified data
   * @since 3.2
   */
  static void setDynamicBookmarks( const TQString &id, const DynMenuInfo &info );

  /**
   * @return list of dynamic menu ids
   * @since 3.2
   */
  static TQStringList dynamicBookmarksList();

signals:
  void aboutToShowContextMenu( const KBookmark &, TQPopupMenu * );
  /**
   * @since 3.4
   */
  void openBookmark( const TQString& url, TQt::ButtonState state );

public slots: // public for bookmark bar
  void slotBookmarksChanged( const TQString & );

protected slots:
  void slotAboutToShow();
  void slotAboutToShowContextMenu( KPopupMenu *, int, TQPopupMenu * );
  void slotActionHighlighted( KAction * );

  void slotRMBActionRemove( int );
  void slotRMBActionInsert( int );
  void slotRMBActionCopyLocation( int );
  void slotRMBActionEditAt( int );
  void slotRMBActionProperties( int );

  void slotBookmarkSelected();
  /**
   * @ since 3.4
   */
  void slotBookmarkSelected( KAction::ActivationReason reason, TQt::ButtonState state );
  void slotAddBookmarksList();
  void slotAddBookmark();
  void slotNewFolder();

  /**
   * load Netscape's bookmarks
   */
  void slotNSLoad();

protected:
  KExtendedBookmarkOwner* extOwner();
  void refill();
  void addAddBookmark();
  void addAddBookmarksList();
  void addEditBookmarks();
  void addNewFolder();

  void fillContextMenu( TQPopupMenu *, const TQString &, int );

  bool m_bIsRoot:1;
  bool m_bAddBookmark:1;
  bool m_bDirty:1;
  bool m_bNSBookmark:1;
  bool m_bAddShortcuts:1;

  KBookmarkManager * m_pManager;
  KBookmarkOwner *m_pOwner;
  /**
   * The menu in which we plug our actions.
   * Supplied in the constructor.
   */
  KPopupMenu * m_parentMenu;
  /**
   * List of our sub menus
   */
  TQPtrList<KBookmarkMenu> m_lstSubMenus;
  KActionCollection * m_actionCollection;
  /**
   * List of our actions.
   */
  TQPtrList<KAction> m_actions;
  /**
   * Parent bookmark for this menu.
   */
  TQString m_parentAddress;

  // TODO make non static!
  static TQString s_highlightedAddress;
  static TQString s_highlightedImportLocation;
  static TQString s_highlightedImportType;
};

/**
 * A class connected to KNSBookmarkImporter, to fill KActionMenus.
 */
class KIO_EXPORT KBookmarkMenuNSImporter : public TQObject
{
  Q_OBJECT
public:
  KBookmarkMenuNSImporter( KBookmarkManager* mgr, KBookmarkMenu * menu, KActionCollection * act ) :
     m_menu(menu), m_actionCollection(act), m_pManager(mgr) {}

  void openNSBookmarks();
  void openBookmarks( const TQString &location, const TQString &type );
  void connectToImporter( const TQObject &importer );

protected slots:
  void newBookmark( const TQString & text, const TQCString & url, const TQString & );
  void newFolder( const TQString & text, bool, const TQString & );
  void newSeparator();
  void endFolder();

protected:
  TQPtrStack<KBookmarkMenu> mstack;
  KBookmarkMenu * m_menu;
  KActionCollection * m_actionCollection;
  KBookmarkManager* m_pManager;
};

#endif
