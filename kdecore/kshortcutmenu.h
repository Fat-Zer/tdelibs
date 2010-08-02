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

#ifndef __KSHORTCUTMENU_H
#define __KSHORTCUTMENU_H

#include <tqmap.h>
#include <tqpopupmenu.h>

#include "kshortcut.h"

class TQLabel;

class KAccelActions;

/**
 * @internal
 */
class KDECORE_EXPORT KShortcutMenu : public QPopupMenu
{
	Q_OBJECT
 public:
	KShortcutMenu( TQWidget* pParent, KAccelActions* pActions, KKeySequence seq );

	bool insertAction( uint iAction, KKeySequence seq );

	void updateShortcuts();
 
 protected:
	void keyPressEvent( TQKeyEvent* pEvent );

 private:
	int searchForKey( KKey key );
	void keepItemsMatching( KKey key );
 
 private:
	typedef TQMap<uint, KKeySequence> IndexToKKeySequence;
	
	KAccelActions* m_pActions;
	KKeySequence m_seq;
	TQLabel* pTitle;
	IndexToKKeySequence m_seqs;
};

#endif // __KSHORTCUTMENU_H
