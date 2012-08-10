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

#include "kmwfax.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "ipprequest.h"
#include "cupsinfos.h"

#include <tqlabel.h>
#include <tqlayout.h>
#include <klistbox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurl.h>

#include "config.h"

KMWFax::KMWFax(TQWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Custom+2;
	m_title = i18n("Fax Serial Device");
	m_nextpage = KMWizard::Driver;

	TQLabel	*lab = new TQLabel(this);
	lab->setText(i18n("<p>Select the device which your serial Fax/Modem is connected to.</p>"));
	m_list = new KListBox(this);

	TQVBoxLayout	*l1 = new TQVBoxLayout(this,0,10);
	l1->addWidget(lab,0);
	l1->addWidget(m_list,1);

	// initialize
	IppRequest	req;
	req.setOperation(CUPS_GET_DEVICES);
	TQString	uri = TQString::fromLatin1("ipp://%1/printers/").arg(CupsInfos::self()->hostaddr());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	if (req.doRequest("/"))
	{
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
#ifdef HAVE_CUPS_1_6
			if (ippGetName(attr) && strcmp(ippGetName(attr),"device-uri") == 0 && strncmp(ippGetString(attr, 0, NULL),"fax",3) == 0)
			{
				m_list->insertItem(SmallIcon("blockdevice"),TQString::fromLatin1(ippGetString(attr, 0, NULL)));
			}
			attr = ippNextAttribute(req.request());
#else // HAVE_CUPS_1_6
			if (attr->name && strcmp(attr->name,"device-uri") == 0 && strncmp(attr->values[0].string.text,"fax",3) == 0)
			{
				m_list->insertItem(SmallIcon("blockdevice"),TQString::fromLatin1(attr->values[0].string.text));
			}
			attr = attr->next;
#endif // HAVE_CUPS_1_6
		}
	}
}

bool KMWFax::isValid(TQString& msg)
{
	if (m_list->currentItem() == -1)
	{
		msg = i18n("You must select a device.");
		return false;
	}
	return true;
}

void KMWFax::updatePrinter(KMPrinter *printer)
{
	TQString	uri = m_list->currentText();
	printer->setDevice(uri);
}
