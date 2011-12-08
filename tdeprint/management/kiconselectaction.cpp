/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kiconselectaction.h"

#include <tqpopupmenu.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>

class KIconSelectActionPrivate
{
public:
	KIconSelectActionPrivate()
	{
		m_menu = 0;
	}
	TQStringList	m_iconlst;
	TQPopupMenu*	m_menu;
};

KIconSelectAction::KIconSelectAction(const TQString& text, int accel, TQObject* parent, const char* name)
: KSelectAction(text, accel, parent, name)
{
	d = new KIconSelectActionPrivate;
}

KIconSelectAction::~KIconSelectAction()
{
	delete d;
}

void KIconSelectAction::updateIcons()
{
	if (d->m_menu)
	{
		TQStringList	lst = items();
		for (uint id=0; id<lst.count(); ++id)
			d->m_menu->changeItem(id, SmallIconSet(d->m_iconlst[id]), lst[id]);
	}
}

void KIconSelectAction::createPopupMenu()
{
	if (!d->m_menu)
	{
		d->m_menu = popupMenu();
		updateIcons();
	}
}

void KIconSelectAction::setItems(const TQStringList& lst, const TQStringList& iconlst)
{
	KSelectAction::setItems(lst);
	d->m_iconlst = iconlst;
	updateIcons();
}

int KIconSelectAction::plug(TQWidget* widget, int index)
{
	int	value(-1);
	if (widget->inherits(TQPOPUPMENU_OBJECT_NAME_STRING))
	{
		createPopupMenu();
		value = KSelectAction::plug(widget, index);
	}
	else if (widget->inherits("KToolBar"))
	{
		KToolBar* bar = static_cast<KToolBar*>(widget);
		int id = KAction::getToolButtonID();
		// To have a correct layout in the toolbar, a non
		// empty icon has to be used. Use "unknown" by default.
		TQString	iconName = (currentItem() != -1 ? d->m_iconlst[currentItem()] : "unknown");

		createPopupMenu();
		bar->insertButton(iconName, id, true, plainText(), index);
		bar->getButton(id)->setPopup(d->m_menu, true);
		bar->setItemEnabled(id, isEnabled());
		addContainer(bar, id);
		connect(bar, TQT_SIGNAL(destroyed()), TQT_SLOT(slotDestroyed()));

		value = containerCount()-1;
	}
	return value;
}

void KIconSelectAction::updateCurrentItem(int id)
{
	TQWidget*	w = container(id);
	if (w->inherits("KToolBar"))
		static_cast<KToolBar*>(w)->getButton(itemId(id))->setIcon(d->m_iconlst[currentItem()]);
	else
		KSelectAction::updateCurrentItem(id);
}

void KIconSelectAction::setCurrentItem(int index)
{
	KSelectAction::setCurrentItem(index);
}

#include "kiconselectaction.moc"