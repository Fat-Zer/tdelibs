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

#ifndef KMDBENTRY_H
#define KMDBENTRY_H

#include <tqstring.h>
#include <tqptrlist.h>

#include <tdelibs_export.h>

struct TDEPRINT_EXPORT KMDBEntry
{
	// the file location of the driver
	TQString	file;
	// normal information
	TQString	manufacturer;
	TQString	model;
	TQString	modelname;
	// information used for auto-detection
	TQString	pnpmanufacturer;
	TQString	pnpmodel;
	// short driver description (if any)
	TQString	description;
	// tell whether this is the recommended driver
	bool		recommended;
	TQString	drivercomment;

	KMDBEntry();
	bool validate(bool checkIt = true);
};

typedef TQPtrList<KMDBEntry>		KMDBEntryList;
typedef TQPtrListIterator<KMDBEntry>	KMDBEntryListIterator;

#endif
