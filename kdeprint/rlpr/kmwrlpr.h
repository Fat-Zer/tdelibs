/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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

#ifndef KMWRLPR_H
#define KMWRLPR_H

#include "kmwizardpage.h"

class KListView;
class TQLineEdit;
class TQListViewItem;

class KMWRlpr : public KMWizardPage
{
	Q_OBJECT
public:
	KMWRlpr(TQWidget *parent = 0, const char *name = 0);

	bool isValid(TQString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

protected slots:
	void slotPrinterSelected(TQListViewItem*);

protected:
	void initialize();

private:
	KListView	*m_view;
	TQLineEdit	*m_host, *m_queue;
};

#endif
