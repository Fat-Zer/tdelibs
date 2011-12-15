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

#include "lprhandler.h"
#include "kmprinter.h"
#include "printcapentry.h"
#include "kmmanager.h"
#include "lprsettings.h"
#include "driver.h"

#include <tqfile.h>
#include <textstream.h>
#include <tqvaluestack.h>
#include <klocale.h>

#include <unistd.h>

LprHandler::LprHandler(const TQString& name, KMManager *mgr)
: m_name(name), m_manager(mgr)
{
}

LprHandler::~LprHandler()
{
}

bool LprHandler::validate(PrintcapEntry*)
{
	return true;
}

KMPrinter* LprHandler::createPrinter(PrintcapEntry *entry)
{
	KMPrinter	*prt = new KMPrinter;
	prt->setPrinterName(entry->name);
	prt->setName(entry->name);
	prt->setType(KMPrinter::Printer);
	return prt;
}

bool LprHandler::completePrinter(KMPrinter *prt, PrintcapEntry *entry, bool)
{
	prt->setDescription(i18n("Unknown (unrecognized entry)"));
	TQString val = entry->field("lp");
	KURL uri;
	if (!val.isEmpty() && val != "/dev/null")
	{
		int	p = val.find('@');
		if (p != -1)
		{
			prt->setLocation(i18n("Remote queue (%1) on %2").arg(val.left(p)).arg(val.mid(p+1)));
			uri.setProtocol("lpd");
			uri.setHost(val.mid(p+1));
			uri.setPath("/" + val.left(p));
		}
		else if ((p = val.find('%')) != -1)
		{
			prt->setLocation(i18n("Network printer (%1)").arg("socket"));
			uri.setProtocol("socket");
			uri.setHost(val.left(p));
			uri.setPort(val.mid(p+1).toInt());
		}
		else
		{
			prt->setLocation(i18n("Local printer on %1").arg(val));
			uri.setProtocol("parallel");
			uri.setPath(val);
		}
	}
	else if (!(val = entry->field("rp")).isEmpty())
	{
		TQString rm = entry->has("rm") ?
				entry->field("rm") :
				LprSettings::self()->defaultRemoteHost();
		prt->setLocation(i18n("Remote queue (%1) on %2").arg(val).arg(rm));
		uri.setProtocol("lpd");
		uri.setHost(rm);
		uri.setPath("/" + val);
	}
	else
		prt->setLocation(i18n("Unknown (unrecognized entry)"));
	prt->setDevice(uri.url());
	return true;
}

DrMain* LprHandler::loadDriver(KMPrinter*, PrintcapEntry*, bool)
{
	manager()->setErrorMsg(i18n("Unrecognized entry."));
	return NULL;
}

bool LprHandler::savePrinterDriver(KMPrinter*, PrintcapEntry*, DrMain*, bool*)
{
	manager()->setErrorMsg(i18n("Unrecognized entry."));
	return false;
}

DrMain* LprHandler::loadDbDriver(const TQString&)
{
	manager()->setErrorMsg(i18n("Unrecognized entry."));
	return NULL;
}

PrintcapEntry* LprHandler::createEntry(KMPrinter *prt)
{
	// this default handler only supports local parallel and remote lpd URIs
	KURL	uri ( prt->device() );
	TQString	prot = uri.protocol();
	if (!prot.isEmpty() && prot != "parallel" && prot != "file" && prot != "lpd" && prot != "socket")
	{
		manager()->setErrorMsg(i18n("Unsupported backend: %1.").arg(prot));
		return NULL;
	}
	PrintcapEntry	*entry = new PrintcapEntry;
	entry->comment = "# Default handler";
	if (prot == "lpd")
	{
		entry->addField("rm", Field::String, uri.host());
		TQString	rp = uri.path();
		if (rp[0] == '/')
			rp = rp.mid(1);
		entry->addField("rp", Field::String, rp);
		// force this entry to null (otherwise it seems it's redirected
		// to /dev/lp0 by default)
		entry->addField("lp", Field::String, TQString::null);
	}
	else if ( prot == "socket" )
	{
		TQString lp = uri.host();
		if ( uri.port() == 0 )
			lp.append( "%9100" );
		else
			lp.append( "%" ).append( TQString::number( uri.port() ) );
		entry->addField("lp", Field::String, lp);
	}
	else
	{
		entry->addField("lp", Field::String, uri.path());
	}
	return entry;
}

bool LprHandler::removePrinter(KMPrinter*, PrintcapEntry*)
{
	return true;
}

TQString LprHandler::printOptions(KPrinter*)
{
	return TQString::null;
}

void LprHandler::reset()
{
}

DrMain* LprHandler::loadToolDriver(const TQString& filename)
{
	TQFile	f(filename);
	if (f.open(IO_ReadOnly))
	{
		DrMain	*driver = new DrMain;
		TQValueStack<DrGroup*>	groups;
		TQTextStream	t(&f);
		TQStringList	l;
		DrListOption	*lopt(0);
		DrBase	*opt(0);

		groups.push(driver);
		driver->set("text", "Tool Driver");
		while (!t.atEnd())
		{
			l = TQStringList::split('|', t.readLine().stripWhiteSpace(), false);
			if (l.count() == 0)
				continue;
			if (l[0] == "GROUP")
			{
				DrGroup	*grp = new DrGroup;
				grp->setName(l[1]);
				grp->set("text", l[2]);
				groups.top()->addGroup(grp);
				groups.push(grp);
			}
			else if (l[0] == "ENDGROUP")
			{
				groups.pop();
			}
			else if (l[0] == "OPTION")
			{
				opt = 0;
				lopt = 0;
				if (l.count() > 3)
				{
					if (l[3] == "STRING")
						opt = new DrStringOption;
					else if (l[3] == "BOOLEAN")
					{
						lopt = new DrBooleanOption;
						opt = lopt;
					}
				}
				else
				{
					lopt = new DrListOption;
					opt = lopt;
				}
				if (opt)
				{
					opt->setName(l[1]);
					opt->set("text", l[2]);
					groups.top()->addOption(opt);
				}
			}
			else if (l[0] == "CHOICE" && lopt)
			{
				DrBase	*ch = new DrBase;
				ch->setName(l[1]);
				ch->set("text", l[2]);
				lopt->addChoice(ch);
			}
			else if (l[0] == "DEFAULT" && opt)
			{
				opt->setValueText(l[1]);
				opt->set("default", l[1]);
			}
		}
		return driver;
	}
	return NULL;
}

TQString LprHandler::driverDirectory()
{
	if (m_cacheddriverdir.isEmpty())
		m_cacheddriverdir = driverDirInternal();
	return m_cacheddriverdir;
}

TQString LprHandler::driverDirInternal()
{
	return TQString::null;
}

TQString LprHandler::locateDir(const TQString& dirname, const TQString& paths)
{
	TQStringList	pathlist = TQStringList::split(':', paths, false);
	for (TQStringList::ConstIterator it=pathlist.begin(); it!=pathlist.end(); ++it)
	{
		TQString	testpath = *it + "/" + dirname;
		if (::access(TQFile::encodeName(testpath), F_OK) == 0)
			return testpath;
	}
	return TQString::null;
}
