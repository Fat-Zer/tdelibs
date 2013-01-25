/*  This file is part of the KDE libraries
    Copyright (C) 2002 Ellis Whitehead <ellis@kde.org>

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

#ifndef __KSHORTCUTLIST_H
#define __KSHORTCUTLIST_H

#include <tqglobal.h>	// For uint
#include <tqstring.h>
#include "tdelibs_export.h"

class TQVariant;
class TDEConfigBase;
class TDEInstance;
class KKeySequence;
class KShortcut;

/**********************************************************************
* This is a wrapper class which allows a function to use one interface
* to KActionCollection, KAccelActions, and KActionPtrList.
**********************************************************************/

/**
 * KShortcutList is an abstract base class for
 * KAccelShortcutList and KStdAccel::ShortcutList. It gives
 * you an unified interface for accessing the accelerator lists
 * of KAccel (using KAccelShortcutList),
 * TDEGlobalAccel (using KAccelShortcutList), and
 * KStdAccel (using KStdAccel::ShortcutList).
 *
 * @short Base class for accessing accelerator lists
 */
class TDECORE_EXPORT KShortcutList
{
 public:
        /**
	 * Default constructor.
	 */
	KShortcutList();
	virtual ~KShortcutList();

	/**
	 * Returns the number of entries.
	 * @return the number of entries
	 */
	virtual uint count() const = 0;

	/**
	 * Returns the name of the shortcut with the given @p index.
	 * @param index the index of the shortcut (must be < count())
	 * @return the name of the shortcut
	 */
	virtual TQString name( uint index ) const = 0;

	/**
	 * Returns the (i18n'd) label of the shortcut with the given @p index.
	 * @param index the index of the shortcut (must be < count())
	 * @return the label (i18n'd) of the shortcut
	 */
	virtual TQString label( uint index ) const = 0;

	/**
	 * Returns the (i18n'd) What's This text of the shortcut with the given @p index.
	 * @param index the index of the shortcut (must be < count())
	 * @return the What's This text (i18n'd) of the shortcut
	 */
	virtual TQString whatsThis( uint index ) const = 0;

	// TODO KDE4: add virtual TQString toolTip( uint index ) const = 0
	// Will then be used by the listview in kkeydialog

	/**
	 * Returns the shortcut with the given @p index.
	 * @param index the index of the shortcut (must be < count())
	 * @return the shortcut
	 * @see shortcutDefault()
	 */
	virtual const KShortcut& shortcut( uint index ) const = 0;

	/**
	 * Returns default shortcut with the given @p index.
	 * @param index the index of the shortcut (must be < count())
	 * @return the default shortcut
	 * @see shortcut()
	 */
	virtual const KShortcut& shortcutDefault( uint index ) const = 0;

	/**
	 * Checks whether the shortcut with the given @p index is configurable.
	 * @param index the index of the shortcut (must be < count())
	 * @return true if configurable, false otherwise
	 */
	virtual bool isConfigurable( uint index ) const = 0;

	/**
	 * Sets the shortcut of the given entry
	 * @param index the index of the shortcut (must be < count())
	 * @param shortcut the shortcut
	 */
	virtual bool setShortcut( uint index, const KShortcut &shortcut ) = 0;

	/**
	 * Checks whether the shortcut with the given @p index is saved in the
	 * global configuration.
	 * @param index the index of the shortcut (must be < count())
	 * @return true if global, false otherwise
	 */
	virtual bool isGlobal( uint index ) const;

	/**
	 * Returns the index of the shortcut with he given name.
	 * @param sName the name of the shortcut to search
	 * @return the index of the shortcut, of -1 if not found
	 */
	virtual int index( const TQString& sName ) const;

	/**
	 * Returns the index of the shortcut with he given key sequence.
	 * @param keySeq the key sequence to search for
	 * @return the index of the shortcut, of -1 if not found
	 */
	virtual int index( const KKeySequence& keySeq ) const;

	/**
	 * The TDEInstance.
	 * @return the TDEInstance of the list, can be 0 if not available
	 */
	virtual const TDEInstance* instance() const;

	// These are here in order to handle expansion.
	enum Other { };
	/** \internal */
	virtual TQVariant getOther( Other, uint index ) const = 0;
	/** \internal */
	virtual bool setOther( Other, uint index, TQVariant ) = 0;

	/**
	 * Save the shortcut list.
	 * @return true if successful, false otherwise
	 */
	virtual bool save() const = 0;

	/**
	 * Loads the shortcuts from the given configuration file.
	 *
	 * @param sConfigGroup the group in the configuration file
	 * @param pConfig the configuration file to load from
	 * @return true if successful, false otherwise
	 */
	virtual bool readSettings( const TQString& sConfigGroup = TQString::null, TDEConfigBase* pConfig = 0 );

	/**
	 * Writes the shortcuts to the given configuration file.
	 *
	 * @param sConfigGroup the group in the configuration file
	 * @param pConfig the configuration file to save to
	 * @param bWriteAll true to write all actions
	 * @param bGlobal true to write to the global configuration file
	 * @return true if successful, false otherwise
	 */
	virtual bool writeSettings( const TQString& sConfigGroup = TQString::null, TDEConfigBase* pConfig = 0,
			bool bWriteAll = false, bool bGlobal = false ) const;

 protected:
	/// used to extend the interface with virtuals without breaking binary compatibility
        virtual void virtual_hook( int id, void* data );
 private:
	class KShortcutListPrivate* d;
};

//---------------------------------------------------------------------
// KAccelShortcutList
//---------------------------------------------------------------------

class KAccel;
class KAccelActions;
class TDEGlobalAccel;

/**
 * KShortcutList implementation to access KAccel and
 * TDEGlobalAccel lists.
 */
class TDECORE_EXPORT KAccelShortcutList : public KShortcutList
{
 public:
        /**
	 * Creates a new KShortcutList that accesses the given KAccel.
	 * @param accel the accelerators to access
	 */
	KAccelShortcutList( KAccel* accel );

        /**
	 * Creates a new KShortcutList that accesses the given
	 * TDEGlobalAccel.
	 * @param accel the accelerators to access
	 */
	KAccelShortcutList( TDEGlobalAccel* accel );

        /**
	 * @internal
	 * Creates a new KShortcutList that accesses the given
	 * KAccelActions collection.
	 * @param actions the actions to access
	 * @param bGlobal true to save the actions in the global
	 *        configuration file
	 */
	KAccelShortcutList( KAccelActions &actions, bool bGlobal );
	virtual ~KAccelShortcutList();

	virtual uint count() const;
	virtual TQString name( uint index ) const;
	virtual TQString label( uint index ) const;
	virtual TQString whatsThis( uint index ) const;
	virtual const KShortcut& shortcut( uint index ) const;
	virtual const KShortcut& shortcutDefault( uint index ) const;
	virtual bool isConfigurable( uint index ) const;
	virtual bool setShortcut( uint index , const KShortcut& shortcut );
	virtual bool isGlobal( uint index ) const;

	/** \internal */
	virtual TQVariant getOther( Other, uint index ) const;
	/** \internal */
	virtual bool setOther( Other, uint index, TQVariant );

	virtual bool save() const;

 protected:
	/** Actions (collection) for this shortcut list. Set by constructor. */
	KAccelActions& m_actions;
	/** Is this shortcut list global? Access through isGlobal() */
	bool m_bGlobal;

 protected:
	virtual void virtual_hook( int id, void* data );
 private:
	class KAccelShortcutListPrivate* d;
};

namespace KStdAccel {
//---------------------------------------------------------------------
// ShortcutList
//---------------------------------------------------------------------

/**
 * KShortcutList implementation that accesses KStdAccel
 * actions.
 */
class TDECORE_EXPORT ShortcutList : public KShortcutList
{
 public:
        /**
	 * Creates a new ShortcutList.
	 */
	ShortcutList();
	virtual ~ShortcutList();

	virtual uint count() const;
	virtual TQString name( uint index ) const;
	virtual TQString label( uint index ) const;
	virtual TQString whatsThis( uint index ) const;
	virtual const KShortcut& shortcut( uint index ) const;
	virtual const KShortcut& shortcutDefault( uint index ) const;
	virtual bool isConfigurable( uint index ) const;
	virtual bool setShortcut( uint index , const KShortcut& shortcut );

	/** \internal */
	virtual TQVariant getOther( Other, uint index ) const;
	/** \internal */
	virtual bool setOther( Other, uint index, TQVariant );

	virtual bool save() const;

 protected:
	virtual void virtual_hook( int id, void* data );
 private:
	class ShortcutListPrivate* d;
};
}

#endif // __KSHORTCUTLIST_H
