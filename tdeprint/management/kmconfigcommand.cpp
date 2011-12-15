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

#include "kmconfigcommand.h"
#include "kxmlcommandselector.h"

#include <tqlabel.h>
#include <tqgroupbox.h>
#include <layout.h>

#include <klocale.h>
#include <kdialog.h>

KMConfigCommand::KMConfigCommand(TQWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Commands"));
	setPageHeader(i18n("Command Settings"));
	setPagePixmap("exec");

	TQGroupBox	*gb = new TQGroupBox(0, Qt::Horizontal, i18n("Edit/Create Commands"), this);
	TQLabel	*lab = new TQLabel(i18n(
				"<p>Command objects perform a conversion from input to output.<br>"
                "They are used as the basis to build both print filters "
				"and special printers. They are described by a command string, a "
				"set of options, a set of requirements and associated mime types. "
				"Here you can create new command objects and edit existing ones. All "
				"changes will only be effective for you."), gb);
	KXmlCommandSelector	*sel = new KXmlCommandSelector(false, gb);

	TQVBoxLayout	*l0 = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	l0->addWidget(gb);
	l0->addStretch(1);
	TQVBoxLayout	*l2 = new TQVBoxLayout(TQT_TQLAYOUT(gb->layout()), KDialog::spacingHint());
	l2->addWidget(lab);
	l2->addWidget(sel);
}
