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

#include "kcupsprinterimpl.h"
#include "kprinter.h"
#include "driver.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "cupsinfos.h"

#include <tqfile.h>
#include <cups/cups.h>
#include <stdlib.h>
#include <kprocess.h>

static void mapToCupsOptions(const TQMap<TQString,TQString>& opts, TQString& cmd);

TQSize rangeToSize(const TQString& s)
{
	TQString	range = s;
	int	p(-1);
	int	from, to;

	if ((p=range.find(',')) != -1)
		range.truncate(p);
	if ((p=range.find('-')) != -1)
	{
		from = range.left(p).toInt();
		to = range.right(range.length()-p-1).toInt();
	}
	else if (!range.isEmpty())
		from = to = range.toInt();
	else
		from = to = 0;

	return TQSize(from,to);
}
//******************************************************************************************************

KCupsPrinterImpl::KCupsPrinterImpl(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KPrinterImpl(parent,name)
{
}

KCupsPrinterImpl::~KCupsPrinterImpl()
{
}

bool KCupsPrinterImpl::setupCommand(TQString& cmd, KPrinter *printer)
{
	// check printer object
	if (!printer) return false;

	TQString	hoststr = TQString::fromLatin1("%1:%2").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
	cmd = TQString::fromLatin1("cupsdoprint -P %1 -J %3 -H %2").arg(quote(printer->printerName())).arg(quote(hoststr)).arg(quote(printer->docName()));
	if (!CupsInfos::self()->login().isEmpty())
	{
		TQString	userstr(CupsInfos::self()->login());
		//if (!CupsInfos::self()->password().isEmpty())
		//	userstr += (":" + CupsInfos::self()->password());
		cmd.append(" -U ").append(quote(userstr));
	}
	mapToCupsOptions(printer->options(),cmd);
	return true;
}

void KCupsPrinterImpl::preparePrinting(KPrinter *printer)
{
	// process orientation
	TQString	o = printer->option("orientation-requested");
	printer->setOption("kde-orientation",(o == "4" || o == "5" ? "Landscape" : "Portrait"));
	// if it's a TQt application, then convert orientation as it will be handled by TQt directly
	if (printer->applicationType() == KPrinter::Dialog)
		printer->setOption("orientation-requested",(o == "5" || o == "6" ? "6" : "3"));

	// translate copies number
	if (!printer->option("kde-copies").isEmpty()) printer->setOption("copies",printer->option("kde-copies"));

	// page ranges are handled by CUPS, so application should print all pages
	if (printer->pageSelection() == KPrinter::SystemSide)
	{ // TQt => CUPS
		// translations
		if (!printer->option("kde-range").isEmpty())
			printer->setOption("page-ranges",printer->option("kde-range"));
		if (printer->option("kde-pageorder") == "Reverse")
			printer->setOption("OutputOrder",printer->option("kde-pageorder"));
		o = printer->option("kde-pageset");
		if (!o.isEmpty() && o != "0")
			printer->setOption("page-set",(o == "1" ? "odd" : "even"));
		printer->setOption("multiple-document-handling",(printer->option("kde-collate") == "Collate" ? "separate-documents-collated-copies" : "separate-documents-uncollated-copies"));
	}
	else
	{ // No translation needed (but range => (from,to))
		TQString range = printer->option("kde-range");
		if (!range.isEmpty())
		{
			TQSize	s = rangeToSize(range);
			printer->setOption("kde-from",TQString::number(s.width()));
			printer->setOption("kde-to",TQString::number(s.height()));
		}
	}

	// needed for page size and margins
	KPrinterImpl::preparePrinting(printer);
}

void KCupsPrinterImpl::broadcastOption(const TQString& key, const TQString& value)
{
	KPrinterImpl::broadcastOption(key,value);
	if (key == "kde-orientation")
		KPrinterImpl::broadcastOption("orientation-requested",(value == "Landscape" ? "4" : "3"));
	else if (key == "kde-pagesize")
	{
		TQString	pagename = TQString::fromLatin1(pageSizeToPageName((KPrinter::PageSize)value.toInt()));
		KPrinterImpl::broadcastOption("PageSize",pagename);
		// simple hack for classes
		KPrinterImpl::broadcastOption("media",pagename);
	}
}

//******************************************************************************************************

static void mapToCupsOptions(const TQMap<TQString,TQString>& opts, TQString& cmd)
{
	TQString	optstr;
	for (TQMap<TQString,TQString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		// only encode those options that doesn't start with "kde-" or "app-".
		if (!it.key().startsWith("kde-") && !it.key().startsWith("app-") && !it.key().startsWith("_kde"))
		{
			TQString key = it.key();
			if (key.startsWith("KDEPrint-"))
				/* Those are keys added by the "Additional Tags" page. *
				 * Strip the prefix to build valid a CUPS option.      */
				key = key.mid(9);
			optstr.append(" ").append(key);
			if (!it.data().isEmpty())
				optstr.append("=").append(it.data());
		}
	}
	if (!optstr.isEmpty())
		cmd.append(" -o ").append( TDEProcess::quote( optstr ) );
}
