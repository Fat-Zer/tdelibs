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

#include "editlist.h"

#include <klistbox.h>
#include <kpushbutton.h>
#include <layout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>

EditList::EditList(TQWidget *parent, const char *name)
	: TQWidget(parent, name)
{
	list_ = new KListBox(this);
	addbtn_ = new KPushButton(KGuiItem(i18n("Add..."), "filenew"), this);
	editbtn_ = new KPushButton(KGuiItem(i18n("Edit..."), "edit"), this);
	delbtn_ = new KPushButton(KGuiItem(i18n("Delete"), "editdelete"), this);
	defbtn_ = new KPushButton(KGuiItem(i18n("Default List"), "history"), this);

	TQGridLayout *m1 = new TQGridLayout(this, 4, 2, 0, 0);
	m1->setColStretch(0, 1);
	m1->addMultiCellWidget(list_, 0, 3, 0, 1);
	m1->addWidget(addbtn_, 0, 1);
	m1->addWidget(editbtn_, 1, 1);
	m1->addWidget(delbtn_, 2, 1);
	m1->addWidget(defbtn_, 3, 1);

	connect(addbtn_, TQT_SIGNAL(clicked()), TQT_SIGNAL(add()));
	connect(editbtn_, TQT_SIGNAL(clicked()), TQT_SLOT(slotEdit()));
	connect(delbtn_, TQT_SIGNAL(clicked()), TQT_SLOT(slotDelete()));
	connect(defbtn_, TQT_SIGNAL(clicked()), TQT_SIGNAL(defaultList()));
	connect(list_, TQT_SIGNAL(highlighted(int)), TQT_SLOT(slotSelected(int)));
	slotSelected(-1);
}

void EditList::slotEdit()
{
	int index = list_->currentItem();
	if (index >= 0)
		emit edit(index);
}

void EditList::slotDelete()
{
	int	index = list_->currentItem();
	list_->removeItem(index);
	slotSelected((list_->count() > 0 ? list_->currentItem() : -1));
	emit deleted(index);
}

void EditList::slotSelected(int index)
{
	editbtn_->setEnabled(index >= 0);
	delbtn_->setEnabled(index >= 0);
}

TQString EditList::text(int index)
{
	return list_->text(index);
}

void EditList::setText(int index, const TQString& s)
{
	if (list_->text(index) != s)
	{
		TQListBoxItem	*it = list_->findItem(s, TQt::ExactMatch);
		if (!it)
			list_->changeItem(s, index);
		else
			list_->removeItem(index);
	}
}

void EditList::clear()
{
	list_->clear();
	slotSelected(-1);
}

void EditList::insertItem(const TQString& s)
{
	if (!list_->findItem(s, TQt::ExactMatch))
		list_->insertItem(s);
}

void EditList::insertItem(const TQPixmap& icon, const TQString& s)
{
	if (!list_->findItem(s, TQt::ExactMatch))
		list_->insertItem(icon, s);
}

void EditList::insertItems(const TQStringList& l)
{
	for (TQStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
		insertItem(*it);
}

TQStringList EditList::items()
{
	TQStringList l;
	for (uint i=0; i<list_->count(); i++)
		l << list_->text(i);
	return l;
}

#include "editlist.moc"
