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

#include "kmextuimanager.h"
#include "kpqtpage.h"
#include "kprinterpropertydialog.h"

#include <tdelocale.h>

KMExtUiManager::KMExtUiManager(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KMUiManager(parent,name)
{
	m_printdialogflags |= KMUiManager::PrintCommand;
}

KMExtUiManager::~KMExtUiManager()
{
}

int KMExtUiManager::pluginPageCap()
{
	return KMUiManager::NoAutoCollate;
}

void KMExtUiManager::setupPrinterPropertyDialog( KPrinterPropertyDialog *dlg )
{
	dlg->addPage( new KPQtPage( dlg, "QtPage" ) );
}
