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

#ifndef LPRNGTOOLHANDLER_H
#define LPRNGTOOLHANDLER_H

#include "lprhandler.h"
#include <tqmap.h>
#include <tqstringlist.h>
#include <tqpair.h>

class LPRngToolHandler : public LprHandler
{
public:
	LPRngToolHandler(KMManager *mgr = 0);

	bool validate(PrintcapEntry*);
	bool completePrinter(KMPrinter*, PrintcapEntry*, bool = true);
	DrMain* loadDriver(KMPrinter*, PrintcapEntry*, bool = false);
	DrMain* loadDbDriver(const TQString&);
	PrintcapEntry* createEntry(KMPrinter*);
	bool savePrinterDriver(KMPrinter*, PrintcapEntry*, DrMain*, bool* = 0);
	TQString printOptions(KPrinter*);

protected:
	TQMap<TQString,TQString> parseXferOptions(const TQString&);
	void loadAuthFile(const TQString&, TQString&, TQString&);
	TQValueList< TQPair<TQString,TQStringList> > loadChoiceDict(const TQString&);
	TQMap<TQString,TQString> parseZOptions(const TQString&);
	TQString filterDir();
	TQString driverDirInternal();


private:
	TQValueList< TQPair<TQString,TQStringList> >	m_dict;
};

#endif
