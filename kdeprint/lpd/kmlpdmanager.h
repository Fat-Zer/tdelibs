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

#ifndef KMLPDMANAGER_H
#define KMLPDMANAGER_H

#include "kmmanager.h"
#include <tqdict.h>

class PrintcapEntry;
class PrinttoolEntry;
class GsChecker;

class KMLpdManager : public KMManager
{
public:
	KMLpdManager(TQObject *parent = 0, const char *name = 0);
	~KMLpdManager();

	bool completePrinterShort(KMPrinter*);
	bool completePrinter(KMPrinter*);
	bool createPrinter(KMPrinter*);
	bool removePrinter(KMPrinter*);
	bool enablePrinter(KMPrinter*);
	bool disablePrinter(KMPrinter*);

	// Driver DB functions
	TQString driverDbCreationProgram();
	TQString driverDirectory();

	// Driver loading functions
	DrMain* loadDbDriver(KMDBEntry*);
	DrMain* loadPrinterDriver(KMPrinter *p, bool config = false);
	bool savePrinterDriver(KMPrinter*, DrMain*);
	bool validateDbDriver(KMDBEntry*);

protected:
	void listPrinters();
	bool writePrinters();
	void loadPrintcapFile(const TQString& filename);
	bool writePrintcapFile(const TQString& filename);
	void loadPrinttoolDb(const TQString& filename);
	TQMap<TQString,TQString> loadPrinttoolCfgFile(const TQString& filename);
	bool savePrinttoolCfgFile(const TQString& templatefile, const TQString& dirname, const TQMap<TQString,TQString>& options);
	bool checkGsDriver(const TQString& gsdriver);
	bool createSpooldir(PrintcapEntry*);
	bool createPrinttoolEntry(KMPrinter*, PrintcapEntry*);
	PrintcapEntry* tqfindPrintcapEntry(const TQString& name);
	PrinttoolEntry* tqfindPrinttoolEntry(const TQString& name);
	TQString programName(int);
	void checktqStatus();
	bool enablePrinter(KMPrinter*, bool);

private:
	TQDict<PrintcapEntry>	m_entries;
	TQDict<PrinttoolEntry>	m_ptentries;
	GsChecker		*m_gschecker;
};

#endif
