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

#ifndef KMWBACKEND_H
#define KMWBACKEND_H

#include "kmwizardpage.h"
#include <tdelibs_export.h>
#include <tqmap.h>

class TQButtonGroup;
class TQVBoxLayout;

class TDEPRINT_EXPORT KMWBackend : public KMWizardPage
{
public:
	KMWBackend(TQWidget *parent = 0, const char *name = 0);

	bool isValid(TQString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

	void addBackend(int ID, const TQString& txt, bool on = true, const TQString& whatsThis = TQString::null, int nextpage = -1);
	void addBackend(int ID = -1, bool on = true, int nextpage = -1);
	void enableBackend(int ID, bool on = true);

private:
	TQButtonGroup	*m_buttons;
	TQVBoxLayout	*m_layout;
	// keep a map between button ID and the real next page to switch to. This enables
	// to have different backends switching to the same page (like backends requiring
	// a password). If the next page is not given when adding the backend, the ID is
	// used by default.
	TQMap<int,int>	m_map;
	int 		m_count;
};

#endif
