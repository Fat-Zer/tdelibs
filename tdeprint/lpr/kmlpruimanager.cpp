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

#include "kmlpruimanager.h"
#include "kmpropertypage.h"
#include "kprinterpropertydialog.h"
#include "kpqtpage.h"
#include "kmconfigdialog.h"

#include "kmpropdriver.h"
#include "kmpropbackend.h"
#include "kmwizard.h"
#include "kmwbackend.h"
#include "kmconfiglpr.h"

#include <tdelocale.h>

KMLprUiManager::KMLprUiManager(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KMUiManager(parent,name)
{
}

KMLprUiManager::~KMLprUiManager()
{
}

void KMLprUiManager::setupPropertyPages(KMPropertyPage *pages)
{
	pages->addPropPage(new KMPropBackend(pages, "Backend"));
	pages->addPropPage(new KMPropDriver(pages, "Driver"));
}

void KMLprUiManager::setupPrinterPropertyDialog(KPrinterPropertyDialog *dlg)
{
	dlg->addPage(new KPQtPage(dlg->driver(), dlg, "QtPage"));
}

void KMLprUiManager::setupWizard(KMWizard *wizard)
{
	KMWBackend	*backend = wizard->backendPage();
	
	backend->addBackend(KMWizard::Local, true);
	backend->addBackend(KMWizard::LPD, true);
	backend->addBackend(KMWizard::TCP, true);
	backend->addBackend(KMWizard::SMB, true, KMWizard::Password);
}

void KMLprUiManager::setupConfigDialog(KMConfigDialog *dlg)
{
	dlg->addConfigPage(new KMConfigLpr(dlg));
}
