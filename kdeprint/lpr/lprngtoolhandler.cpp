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

#include "lprngtoolhandler.h"
#include "printcapentry.h"
#include "kmprinter.h"
#include "util.h"
#include "lprsettings.h"
#include "driver.h"
#include "kmmanager.h"
#include "kprinter.h"

#include <tqfile.h>
#include <tqtextstream.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

LPRngToolHandler::LPRngToolHandler(KMManager *mgr)
: LprHandler("lprngtool", mgr)
{
}

bool LPRngToolHandler::validate(PrintcapEntry *entry)
{
	if (entry->comment.startsWith("##LPRNGTOOL##") &&
	    entry->comment.tqfind("UNKNOWN") == -1)
		return true;
	return false;
}

bool LPRngToolHandler::completePrinter(KMPrinter *prt, PrintcapEntry *entry, bool shortmode)
{
	TQString	str, lp;

	// look for type in comment
	TQStringList	l = TQStringList::split(' ', entry->comment, false);
	lp = entry->field("lp");
	if (l.count() < 1)
		return false;

	if (l[1] == "DEVICE" || l[1] == "SOCKET" || l[1] == "QUEUE")
		LprHandler::completePrinter(prt, entry, shortmode);
	else if (l[1] == "SMB")
	{
		TQMap<TQString,TQString>	opts = parseXferOptions(entry->field("xfer_options"));
		TQString	user, pass;
		loadAuthFile(LprSettings::self()->baseSpoolDir() + "/" + entry->name + "/" + opts["authfile"], user, pass);
		TQString uri = buildSmbURI(
				opts[ "workgroup" ],
				opts[ "host" ],
				opts[ "printer" ],
				user,
				pass );
		prt->setDevice( uri );
		prt->setLocation(i18n("Network printer (%1)").arg("smb"));
	}

	// look for comment
	if (!(str=entry->field("cm")).isEmpty())
		prt->setDescription(str);

	// driver
	//if (!shortmode)
	//{
		if (!(str=entry->field("ifhp")).isEmpty())
		{
			TQString	model;
			int	p = str.tqfind("model");
			if (p != -1)
			{
				p = str.tqfind('=', p);
				if (p != -1)
				{
					p++;
					int	q = str.tqfind(',', p);
					if (q == -1)
						model = str.mid(p);
					else
						model = str.mid(p, q-p);
				}
			}
			prt->setDriverInfo(i18n("IFHP Driver (%1)").arg((model.isEmpty() ? i18n("unknown") : model)));
			prt->setOption("driverID", model);
		}
	//}
	return true;
}

TQMap<TQString,TQString> LPRngToolHandler::parseXferOptions(const TQString& str)
{
	uint	p(0), q;
	TQMap<TQString,TQString>	opts;
	TQString	key, val;

	while (p < str.length())
	{
		key = val = TQString::null;
		// skip spaces
		while (p < str.length() && str[p].isSpace())
			p++;
		q = p;
		while (q < str.length() && str[q] != '=')
			q++;
		key = str.mid(p, q-p);
		p = q+2;
		while (p < str.length() && str[p] != '"')
			p++;
		val = str.mid(q+2, p-q-2);
		if (!key.isEmpty())
			opts[key] = val;
		p++;
	}
	return opts;
}

void LPRngToolHandler::loadAuthFile(const TQString& filename, TQString& user, TQString& pass)
{
	TQFile	f(filename);
	if (f.open(IO_ReadOnly))
	{
		TQTextStream	t(&f);
		TQString	line;
		while (!t.atEnd())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.isEmpty())
				continue;
			int	p = line.tqfind('=');
			if (p != -1)
			{
				TQString	key = line.left(p);
				if (key == "username")
					user = line.mid(p+1);
				else if (key == "password")
					pass = line.mid(p+1);
			}
		}
	}
}

DrMain* LPRngToolHandler::loadDriver(KMPrinter *prt, PrintcapEntry *entry, bool config)
{
	if (entry->field("lprngtooloptions").isEmpty())
	{
		manager()->setErrorMsg(i18n("No driver defined for that printer. It might be a raw printer."));
		return NULL;
	}

	DrMain*	driver = loadToolDriver(locate("data", "kdeprint/lprngtooldriver1"));
	if (driver)
	{
		TQString	model = prt->option("driverID");
		driver->set("text", i18n("LPRngTool Common Driver (%1)").arg((model.isEmpty() ? i18n("unknown") : model)));
		if (!model.isEmpty())
			driver->set("driverID", model);
		TQMap<TQString,TQString>	opts = parseZOptions(entry->field("prefix_z"));
		opts["lpr"] = entry->field("lpr");
		driver->setOptions(opts);
		// if not configuring, don't show the "lpr" options
		if (!config)
			driver->removeOptionGlobally("lpr");
	}
	return driver;
}

DrMain* LPRngToolHandler::loadDbDriver(const TQString& s)
{
	int	p = s.tqfind('/');
	DrMain*	driver = loadToolDriver(locate("data", "kdeprint/lprngtooldriver1"));
	if (driver)
		driver->set("driverID", s.mid(p+1));
	return driver;
}

TQValueList< TQPair<TQString,TQStringList> > LPRngToolHandler::loadChoiceDict(const TQString& filename)
{
	TQFile	f(filename);
	TQValueList< TQPair<TQString,TQStringList> >	dict;
	if (f.open(IO_ReadOnly))
	{
		TQTextStream	t(&f);
		TQString	line, key;
		TQStringList	l;
		while (!t.atEnd())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.startsWith("OPTION"))
			{
				if (l.count() > 0 && !key.isEmpty())
					dict << TQPair<TQString,TQStringList>(key, l);
				l.clear();
				key = TQString::null;
				if (line.contains('|') == 2 || line.right(7) == "BOOLEAN")
				{
					int	p = line.tqfind('|', 7);
					key = line.mid(7, p-7);
				}
			}
			else if (line.startsWith("CHOICE"))
			{
				int	p = line.tqfind('|', 7);
				l << line.mid(7, p-7);
			}
		}
	}
	return dict;
}

TQMap<TQString,TQString> LPRngToolHandler::parseZOptions(const TQString& optstr)
{
	TQMap<TQString,TQString>	opts;
	TQStringList	l = TQStringList::split(',', optstr, false);
	if (l.count() == 0)
		return opts;
	
	if (m_dict.count() == 0)
		m_dict = loadChoiceDict(locate("data", "kdeprint/lprngtooldriver1"));

	TQString	unknown;
	for (TQStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
	{
		bool	found(false);
		for (TQValueList< TQPair<TQString,TQStringList> >::ConstIterator p=m_dict.begin(); p!=m_dict.end() && !found; ++p)
		{
			if ((*p).second.tqfind(*it) != (*p).second.end())
			{
				opts[(*p).first] = (*it);
				found = true;
			}
		}
		if (!found)
		{
			unknown.append(*it).append(',');
		}
	}
	if (!unknown.isEmpty())
	{
		unknown.truncate(unknown.length()-1);
		opts["filter"] = unknown;
	}
	return opts;
}

TQString LPRngToolHandler::filterDir()
{
	return driverDirectory();
}

TQString LPRngToolHandler::driverDirInternal()
{
	return locateDir("filters", "/usr/lib:/usr/local/lib:/opt/lib:/usr/libexec:/usr/local/libexec:/opt/libexec");
}

PrintcapEntry* LPRngToolHandler::createEntry(KMPrinter *prt)
{
	TQString	prot = prt->deviceProtocol();
	if (prot != "parallel" && prot != "lpd" && prot != "smb" && prot != "socket")
	{
		manager()->setErrorMsg(i18n("Unsupported backend: %1.").arg(prot));
		return NULL;
	}
	PrintcapEntry	*entry = new PrintcapEntry;
	entry->addField("cm", Field::String, prt->description());
	TQString	lp, comment("##LPRNGTOOL## ");
	if (prot == "parallel")
	{
		comment.append("DEVICE ");
		lp = prt->device().mid( 9 );
		entry->addField("rw@", Field::Boolean);
	}
	else if (prot == "socket")
	{
		comment.append("SOCKET ");
		KURL url( prt->device() );
		lp = url.host();
		if (url.port() == 0)
			lp.append("%9100");
		else
			lp.append("%").append(TQString::number(url.port()));
	}
	else if (prot == "lpd")
	{
		comment.append("QUEUE ");
		KURL url( prt->device() );
		lp = url.path().mid(1) + "@" + url.host();
	}
	else if (prot == "smb")
	{
		comment.append("SMB ");
		lp = "| " + filterDir() + "/smbprint";
		TQString	work, server, printer, user, passwd;
		if ( splitSmbURI( prt->device(), work, server, printer, user, passwd ) )
		{
			entry->addField("xfer_options", Field::String, TQString::tqfromLatin1("authfile=\"auth\" crlf=\"0\" hostip=\"\" host=\"%1\" printer=\"%2\" remote_mode=\"SMB\" share=\"//%3/%4\" workgroup=\"%5\"").arg(server).arg(printer).arg(server).arg(printer).arg(work));
			TQFile	authfile(LprSettings::self()->baseSpoolDir() + "/" + prt->printerName() + "/auth");
			if (authfile.open(IO_WriteOnly))
			{
				TQTextStream	t(&authfile);
				t << "username=" << user << endl;
				t << "password=" << passwd << endl;
				authfile.close();
			}
		}
		else
		{
			manager()->setErrorMsg( i18n( "Invalid printer backend specification: %1" ).arg( prt->device() ) );
			delete entry;
			return NULL;
		}
	}

	if (prt->driver())
	{
		DrMain	*driver = prt->driver();
		comment.append("filtertype=IFHP ifhp_options=status@,sync@,pagecount@,waitend@ printerdb_entry=");
		comment.append(driver->get("driverID"));
		entry->addField("ifhp", Field::String, TQString::tqfromLatin1("model=%1,status@,sync@,pagecount@,waitend@").arg(driver->get("driverID")));
		entry->addField("lprngtooloptions", Field::String, TQString::tqfromLatin1("FILTERTYPE=\"IFHP\" IFHP_OPTIONS=\"status@,sync@,pagecount@,waitend@\" PRINTERDB_ENTRY=\"%1\"").arg(driver->get("driverID")));
		TQMap<TQString,TQString>	opts;
		TQString	optstr;
		driver->getOptions(opts, false);
		for (TQMap<TQString,TQString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
			if (it.key() != "lpr")
				optstr.append(*it).append(",");
		if (!optstr.isEmpty())
		{
			optstr.truncate(optstr.length()-1);
			entry->addField("prefix_z", Field::String, optstr);
		}
		if (!opts["lpr"].isEmpty())
			entry->addField("lpr", Field::String, opts["lpr"]);
	}

	entry->addField("lp", Field::String, lp);
	entry->comment = comment;

	return entry;
}

bool LPRngToolHandler::savePrinterDriver(KMPrinter*, PrintcapEntry *entry, DrMain *driver, bool *mustSave)
{
	// save options in the "prefix_z" field and tell the manager to save the printcap file
	TQMap<TQString,TQString>	opts;
	TQString	optstr;
	driver->getOptions(opts, false);
	for (TQMap<TQString,TQString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		if (it.key() != "lpr")
			optstr.append(*it).append(",");
	if (!optstr.isEmpty())
		optstr.truncate(optstr.length()-1);
	// save options in any case, otherwise nothing will happen whn
	// options are reset to their default value
	entry->addField("prefix_z", Field::String, optstr);
	entry->addField("lpr", Field::String, opts["lpr"]);
	if (mustSave)
		*mustSave = true;
	return true;
}

TQString LPRngToolHandler::printOptions(KPrinter *printer)
{
	TQString	optstr;
	TQMap<TQString,TQString>	opts = printer->options();
	for (TQMap<TQString,TQString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		if (it.key().startsWith("kde-") || it.key().startsWith("_kde-") || it.key() == "lpr" || it.key().startsWith( "app-" ))
			continue;
		optstr.append(*it).append(",");
	}
	if (!optstr.isEmpty())
	{
		optstr.truncate(optstr.length()-1);
		optstr.prepend("-Z '").append("'");
	}
	return optstr;
}
