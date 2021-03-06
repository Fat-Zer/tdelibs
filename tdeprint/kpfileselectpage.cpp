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

#include "kpfileselectpage.h"
#include "tdefilelist.h"

#include <tqlayout.h>
#include <tqstringlist.h>
#include <tqregexp.h>
#include <tqheader.h>
#include <tdelocale.h>
#include <kiconloader.h>

KPFileSelectPage::KPFileSelectPage(TQWidget *parent, const char *name)
: KPrintDialogPage(parent, name)
{
	setTitle(i18n("&Files"));
	m_first = true;

	m_files = new KFileList(this);

	TQHBoxLayout	*l0 = new TQHBoxLayout(this, 0, 10);
	l0->addWidget(m_files);

	resize(100, 100);
}

void KPFileSelectPage::getOptions(TQMap<TQString,TQString>& opts, bool incldef)
{
	// (incldef == false) is a hint telling that it should be the last time
	// and we want to do it only once
	if (!incldef)
	{
		TQStringList	l = m_files->fileList();
		opts["kde-filelist"] = l.join("@@");
	}
}

void KPFileSelectPage::setOptions(const TQMap<TQString,TQString>& opts)
{
	// do it only once as files will only be selected there
	if (m_first)
	{
		TQStringList	l = TQStringList::split("@@", opts["kde-filelist"], false);
		m_files->setFileList(l);

		m_first = false;
	}
}
