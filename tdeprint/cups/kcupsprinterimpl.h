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

#ifndef KCUPSPRINTERIMPL_H
#define KCUPSPRINTERIMPL_H

#include "kprinterimpl.h"

class KCupsPrinterImpl : public KPrinterImpl
{
public:
	KCupsPrinterImpl(TQObject *parent, const char *name, const TQStringList & /*args*/ = TQStringList());
	~KCupsPrinterImpl();

	bool setupCommand(TQString&, KPrinter*);
	void preparePrinting(KPrinter*);
	void broadcastOption(const TQString& key, const TQString& value);
};

#endif
