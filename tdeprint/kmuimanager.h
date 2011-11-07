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

#ifndef KMUIMANAGER_H
#define KMUIMANAGER_H

#include <tqobject.h>
#include <tqptrlist.h>

#include "kprinter.h"

class KMPropertyPage;
class KMWizard;
class KPrintDialogPage;
class KPrintDialog;
class KPrinterPropertyDialog;
class KMConfigDialog;
class TQListView;

class TDEPRINT_EXPORT KMUiManager : public TQObject
{
	Q_OBJECT
public:
	enum PrintDialogFlagType
	{
		Properties     = 0x0001,
		Default        = 0x0002,
		Preview        = 0x0004,
		OutputToFile   = 0x0008,
		PrintCommand   = 0x0020,
		Persistent     = 0x0040,
		PrintDialogAll = 0x001F
	};
	enum CopyFlagType
	{
		Current       = 0x001,
		Range         = 0x002,
		Collate       = 0x004,
		Order         = 0x008,
		PageSet       = 0x010,
		CopyAll       = 0x0FF,
		PSSelect      = Range|Order|PageSet,
		NoAutoCollate = 0x100
	};

	KMUiManager(TQObject *parent = 0, const char *name = 0);
	virtual ~KMUiManager();

	// print management
	virtual void setupPropertyPages(KMPropertyPage*);
	virtual void setupWizard(KMWizard*);
	virtual void setupConfigDialog(KMConfigDialog*);

	// print dialog
	void addPrintDialogPage(KPrintDialogPage *page)		{ m_printdialogpages.append(page); }
	int copyFlags(KPrinter *pr = 0, bool usePlugin = true);
	int dialogFlags();
	void setupPrintDialog(KPrintDialog*);
	virtual void setupPrintDialogPages(TQPtrList<KPrintDialogPage>*);

	// printer property dialog
	void setupPropertyDialog(KPrinterPropertyDialog*);
	virtual void setupPrinterPropertyDialog(KPrinterPropertyDialog*);

	// page processing capabilities
	int systemPageCap();
	virtual int pluginPageCap();
	int pageCap();

	// job management
	virtual void setupJobViewer(TQListView*);

protected:
	int			m_printdialogflags;
	TQPtrList<KPrintDialogPage>	m_printdialogpages;
};

#endif
