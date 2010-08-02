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

#ifndef KPCOPIESPAGE_H
#define KPCOPIESPAGE_H

#include "kprintdialogpage.h"
#include <kdeprint/kpreloadobject.h>

class TQRadioButton;
class TQLineEdit;
class TQComboBox;
class TQCheckBox;
class TQSpinBox;
class TQLabel;
class KPrinter;

class KDEPRINT_EXPORT KPCopiesPage : public KPrintDialogPage, public KPReloadObject
{
	Q_OBJECT
public:
	KPCopiesPage(KPrinter *prt = 0, TQWidget *parent = 0, const char *name = 0);
	~KPCopiesPage();

	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);
	void initialize(bool usePlugin = true);

protected slots:
	void slotRangeEntered();
	void slotCollateClicked();

protected:
	void reload();

protected:
	QRadioButton	*m_all, *m_current, *m_range;
	QLineEdit	*m_rangeedit;
	QComboBox	*m_pageset;
	QCheckBox	*m_collate, *m_order;
	QSpinBox	*m_copies;
	QLabel		*m_collatepix;
	bool		m_useplugin;

	KPrinter	*m_printer;
};

#endif
