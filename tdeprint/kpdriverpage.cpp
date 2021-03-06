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

#include "kpdriverpage.h"
#include "driverview.h"
#include "driver.h"

#include <tqlayout.h>
#include <tdelocale.h>

KPDriverPage::KPDriverPage(KMPrinter *p, DrMain *d, TQWidget *parent, const char *name)
: KPrintDialogPage(p,d,parent,name)
{
	setTitle(i18n("Driver Settings"));

	m_view = new DriverView(this);
	m_view->setAllowFixed(false);
	if (driver()) m_view->setDriver(driver());

	TQVBoxLayout	*lay1 = new TQVBoxLayout(this, 0, 0);
	lay1->addWidget(m_view);
}

KPDriverPage::~KPDriverPage()
{
}

bool KPDriverPage::isValid(TQString& msg)
{
	if (m_view->hasConflict())
	{
		msg = i18n("<qt>Some options selected are in conflict. You must resolve these conflicts "
 			   "before continuing. See <b>Driver Settings</b> tab for detailed information.</qt>");
		return false;
	}
	return true;
}

void KPDriverPage::setOptions(const TQMap<TQString,TQString>& opts)
{
	m_view->setOptions(opts);
}

void KPDriverPage::getOptions(TQMap<TQString,TQString>& opts, bool incldef)
{
	m_view->getOptions(opts,incldef);
}
