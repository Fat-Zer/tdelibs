/* This file is part of the KDE libraries
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 2001,2001 Ellis Whitehead <ellis@kde.org>

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

#ifndef __KKEYDIALOG_H__
#define __KKEYDIALOG_H__

#include <tqdict.h>
#include <kdialogbase.h>
#include <klistview.h>

class TQButtonGroup;
class TQCheckBox;
class TQGroupBox;
class TQLabel;
class TQLineEdit;
class TQRadioButton;
class KAccel;
class KAccelActions;
class KActionCollection;
class KConfigBase;
class TDEGlobalAccel;
class KKeySequence;
class KShortcut;
class KShortcutList;
class KKeyChooserItem;

/**
 * @short Widget for configuration of KAccel and TDEGlobalAccel.
 *
 * Configure dictionaries of key/action associations for KAccel and
 * TDEGlobalAccel.
 *
 * The class takes care of all aspects of configuration, including
 * handling key conflicts internally. Connect to the allDefault()
 * slot if you want to set all configurable shortcuts to their
 * default values.
 *
 * @see KKeyDialog
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 */
class TDEUI_EXPORT KKeyChooser : public TQWidget
{
	Q_OBJECT
 public:
	enum ActionType { Application, ApplicationGlobal, Standard, Global };

	/**
	 * Constructor.
	 *
	 * @param parent the parent widget for this widget
	 * @param type the ActionType for this KKeyChooser
	 * @param bAllowLetterShortcuts Set to false if unmodified alphanumeric
	 *  keys ('A', '1', etc.) are not permissible shortcuts.
	 **/
	KKeyChooser( TQWidget* parent, ActionType type = Application, bool bAllowLetterShortcuts = true );
	/**
	 * \overload
	 * @param parent parent widget
	 * @param coll the KActionCollection to configure
	 * @param bAllowLetterShortcuts Set to false if unmodified alphanumeric
     *  keys ('A', '1', etc.) are not permissible shortcuts.
     */
	KKeyChooser( KActionCollection* coll, TQWidget* parent, bool bAllowLetterShortcuts = true );

    KKeyChooser( KAccel* actions, TQWidget* parent, bool bAllowLetterShortcuts = true );
	KKeyChooser( TDEGlobalAccel* actions, TQWidget* parent );
	KKeyChooser( KShortcutList*, TQWidget* parent, ActionType type = Application, bool bAllowLetterShortcuts = true );

	virtual ~KKeyChooser();

	/**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * already associated with the KKeyChooser object.
	 */
	bool insert( KActionCollection* ); // #### KDE4 : remove me
    /**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * already associated with the KKeyChooser object.
	 * @param title subtree title of this collection of shortcut.
	 * @since 3.1
	 */
    bool insert( KActionCollection *, const TQString &title);

	void syncToConfig( const TQString& sConfigGroup, KConfigBase* pConfig, bool bClearUnset );

	/**
	 * This function writes any shortcut changes back to the original
	 * action set(s).
	 */
	void commitChanges();

	/**
	 * This commits and then saves the actions to disk.
	 * Any KActionCollection objects with the xmlFile() value set
	 * will be written to an XML file.  All other will be written
	 * to the application's rc file.
	 */
	void save();

        /**
         * Checks whether the given shortcut conflicts with global keyboard shortcuts.
         * If yes, and the warnUser argument is true, warns the user and gives them a chance
         * to reassign the shortcut from the global shortcut.
         *
         * @return true if there was conflict (and the user didn't reassign the shortcut)
         * @param cut the shortcut that will be checked for conflicts
         * @param warnUser if true, the user will be warned about a conflict and given a chance
         *        to reassign the shortcut
         * @param parent parent widget for the warning dialog
         *
         * @since 3.2
         */
        static bool checkGlobalShortcutsConflict( const KShortcut& cut, bool warnUser, TQWidget* parent );

        /**
         * Checks whether the given shortcut conflicts with standard keyboard shortcuts.
         * If yes, and the warnUser argument is true, warns the user and gives them a chance
         * to reassign the shortcut from the standard shortcut.
         *
         * @return true if there was conflict (and the user didn't reassign the shortcut)
         * @param cut the shortcut that will be checked for conflicts
         * @param warnUser if true, the user will be warned about a conflict and given a chance
         *        to reassign the shortcut
         * @param parent parent widget for the warning dialog
         *
         * @since 3.2
         */
        static bool checkStandardShortcutsConflict( const KShortcut& cut, bool warnUser, TQWidget* parent );

 signals:
	/**
	 * Emitted when an action's shortcut has been changed.
	 **/
	void keyChange();

 public slots:
	/**
	 * Set all keys to their default values (bindings).
	 **/
	void allDefault();

	/**
	 * Specifies whether to use the 3 or 4 modifier key scheme.
	 * This determines which default is used when the 'Default' button is
	 * clicked.
	 */
	void setPreferFourModifierKeys( bool preferFourModifierKeys );

 // KDE4 a lot of stuff in this class should be probably private:
 protected:
	enum { NoKey = 1, DefaultKey, CustomKey };

	void initGUI( ActionType type, bool bAllowLetterShortcuts );
	bool insert( KAccel* );
	bool insert( TDEGlobalAccel* );
	bool insert( KShortcutList* );
	/// @since 3.1
	void buildListView( uint iList, const TQString &title = TQString::null );

	void readGlobalKeys();

	void updateButtons();
	void fontChange( const TQFont& _font );
	void setShortcut( const KShortcut& cut );
	bool isKeyPresent( const KShortcut& cut, bool warnuser = true );
        bool isKeyPresentLocally( const KShortcut& cut, KKeyChooserItem* ignoreItem, const TQString& warnText );
	void _warning( const KKeySequence& seq, TQString sAction, TQString sTitle );

 protected slots:
	void slotNoKey();
	void slotDefaultKey();
	void slotCustomKey();
	void slotListItemSelected( TQListViewItem *item );
	void capturedShortcut( const KShortcut& cut );
        void slotSettingsChanged( int );
        void slotListItemDoubleClicked ( TQListViewItem * ipoQListViewItem, const TQPoint & ipoQPoint, int c );

 protected:
	ActionType m_type;
	bool m_bAllowLetterShortcuts;
	bool m_bAllowWinKey; // unused KDE4 remove
	// When set, pressing the 'Default' button will select the aDefaultKeycode4,
	//  otherwise aDefaultKeycode.
	bool m_bPreferFourModifierKeys;

	TQRadioButton* m_prbNone;
	TQRadioButton* m_prbDef;
	TQRadioButton* m_prbCustom;

 private:
        bool isKeyPresentLocally( const KShortcut& cut, KKeyChooserItem* ignoreItem, bool bWarnUser );
        static bool promptForReassign( const KKeySequence& cut, const TQString& sAction, ActionType action, TQWidget* parent );
        // Remove the key sequences contained in cut from the standard shortcut @p name
        // which currently has @p origCut as shortcut.
        static void removeStandardShortcut( const TQString& name, KKeyChooser* chooser, const KShortcut &origCut, const KShortcut &cut );
        // Remove the key sequences contained in cut from the global shortcut @p name
        // which currently has @p origCut as shortcut.
        static void removeGlobalShortcut( const TQString& name, KKeyChooser* chooser, const KShortcut &origCut, const KShortcut &cut );
        static void readGlobalKeys( TQMap< TQString, KShortcut >& map );
        static bool checkGlobalShortcutsConflict( const KShortcut& cut, bool bWarnUser, TQWidget* parent,
            const TQMap< TQString, KShortcut >& map, const TQString& ignoreAction );
        // Remove the key sequences contained in cut from this item
        bool removeShortcut( const TQString& name, const KShortcut &cut );

private slots:
        void captureCurrentItem();

#ifndef KDE_NO_COMPAT
 public:
	/**
	 * @obsolete
	 */
	KKeyChooser( KAccel* actions, TQWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool bAllowWinKey = false );
	/**
	 * @obsolete
	 */
	KKeyChooser( TDEGlobalAccel* actions, TQWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool bAllowWinKey = false );

 public slots:
	/**
	 * Rebuild list entries based on underlying map.
	 * Use this if you changed the underlying map.
	 */
	void listSync();

#endif
 protected:
	virtual void virtual_hook( int id, void* data );
 private:
	class KKeyChooserPrivate *d;
	friend class KKeyDialog;
};
typedef KKeyChooser KKeyChooser;

/**
 * @short Dialog for configuration of KActionCollection, KAccel, and TDEGlobalAccel.
 *
 * The KKeyDialog class is used for configuring dictionaries of key/action
 * associations for KActionCollection, KAccel, and TDEGlobalAccel. It uses the KKeyChooser widget
 * and offers buttons to set all keys to defaults and invoke on-line help.
 *
 * Several static methods are supplied which provide the most convenient interface
 * to the dialog. The most common and most encouraged use is with KActionCollection.
 *
 * \code
 * KKeyDialog::configure( actionCollection() );
 * \endcode
 *
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 */
class TDEUI_EXPORT KKeyDialog : public KDialogBase
{
	Q_OBJECT

public:
	/**
	 * Constructs a KKeyDialog called @p name as a child of @p parent.
	 * Set @p bAllowLetterShortcuts to false if unmodified alphanumeric
	 * keys ('A', '1', etc.) are not permissible shortcuts.
	 */
	KKeyDialog( bool bAllowLetterShortcuts = true, TQWidget* parent = 0, const char* name = 0 );

	/**
	 * Destructor. Deletes all resources used by a KKeyDialog object.
	 */
	virtual ~KKeyDialog();

	/**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * displayed by the dialog.
	 * This method can be useful in applications following the document/view
	 * design, with actions in both the document and the view.
	 * Simply call insert with the action collections of each one in turn.
	 * @return true :)
	 */
	bool insert( KActionCollection* ); // #### KDE4: remove me

        /**
         * Insert an action collection, i.e. add all its actions to the ones
	 * displayed by the dialog.
	 * This method can be useful in applications following the document/view
	 * design, with actions in both the document and the view.
	 * Simply call insert with the action collections of each one in turn.
         *
         * @param title the title associated with the collection (if null, the
         * TDEAboutData::progName() of the collection's instance is used)
         * @return true :)
	 * @since 3.1
	 */
        bool insert(KActionCollection *, const TQString &title);

	/**
	 * Run the dialog and call commitChanges() if @p bSaveSettings
	 * is true.
	 */
	bool configure( bool bSaveSettings = true );

	/**
	 * Commit key setting changes so that changed settings actually become active.
	 * This method is implicitly called from configure(bool) if
	 * @p bSaveSettings is true.
	 */
	void commitChanges();

	/**
	 * Pops up a modal dialog for configuring key settings. The new
	 * shortcut settings will be active if the user presses OK.  If
	 * @p bSaveSettings if true, the settings will also be saved back to
	 * the *uirc file which they were intially read from.
	 * @return Accept if the dialog was closed with OK, Reject otherwise.
	 */
	static int configure( KActionCollection* coll, TQWidget* parent = 0, bool bSaveSettings = true );

	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function, except that settings
	 * are saved to a *.rc file using KConfig.
	 */
	static int configure( KAccel* keys, TQWidget* parent = 0, bool bSaveSettings = true );

	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 */
	static int configure( TDEGlobalAccel* keys, TQWidget* parent = 0, bool bSaveSettings = true );


	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 *
	 * @param coll the KActionCollection to configure
	 * @param bAllowLetterShortcuts Set to false if unmodified alphanumeric
	 *  keys ('A', '1', etc.) are not permissible shortcuts.
	 * @param parent the parent widget to attach to
	 * @param bSaveSettings if true, the settings will also be saved back to
	 * the *uirc file which they were intially read from.
	 */
	static int configure( KActionCollection* coll, bool bAllowLetterShortcuts, TQWidget* parent = 0, bool bSaveSettings = true ); // BCI: merge with bAllowLetterShortcuts = true

	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 **/
	static int configure( KAccel* keys, bool bAllowLetterShortcuts, TQWidget* parent = 0, bool bSaveSettings = true ); // BCI: merge with bAllowLetterShortcuts = true

	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 **/
	static int configure( TDEGlobalAccel* keys, bool bAllowLetterShortcuts, TQWidget* parent = 0, bool bSaveSettings = true ); // BCI: merge with bAllowLetterShortcuts = true

	/**
	 * @deprecated Obsolete.
	 * Please use KKeyDialog::configure instead
	 */
	static KDE_DEPRECATED int configureKeys( KAccel* keys, bool save_settings = true, TQWidget* parent = 0 )
		{ return configure( keys, parent, save_settings ); }
	/**
	 * @deprecated Obsolete.
	 * Please use KKeyDialog::configure instead
	 */
	static KDE_DEPRECATED int configureKeys( TDEGlobalAccel* keys, bool save_settings = true, TQWidget* parent = 0 )
		{ return configure( keys, parent, save_settings ); }
	/**
	 * @deprecated Obsolete.
	 * Please use KKeyDialog::configure instead
	 */
	static KDE_DEPRECATED int configureKeys( KActionCollection* coll, const TQString& /*xmlfile*/,
		bool save_settings = true, TQWidget* parent = 0 )
		{ return configure( coll, parent, save_settings ); }

private:
	KKeyDialog( KKeyChooser::ActionType, bool bAllowLetterShortcuts = true, TQWidget* parent = 0, const char* name = 0 );

 protected:
	virtual void virtual_hook( int id, void* data );

 private:
	class KKeyDialogPrivate* d;
	KKeyChooser* m_pKeyChooser;
};

#endif // __KKEYDIALOG_H__
