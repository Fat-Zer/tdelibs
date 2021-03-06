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

#ifndef KMRLPRMANAGER_H
#define KMRLPRMANAGER_H

#include "kmmanager.h"
#include <tqdict.h>
#include <tqdatetime.h>

class KMRlprManager : public KMManager
{
public:
	KMRlprManager(TQObject *parent, const char *name, const TQStringList & /*args*/);
	~KMRlprManager();

	bool createPrinter(KMPrinter*);
	bool removePrinter(KMPrinter*);
	bool testPrinter(KMPrinter*);

protected:
	void listPrinters();
	void savePrinters();
	void loadPrintersConf(const TQString& filename);
	void savePrintersConf(const TQString& filename);
	TQString printerFile();

private:
	TQDateTime	m_checktime;
};

#endif
