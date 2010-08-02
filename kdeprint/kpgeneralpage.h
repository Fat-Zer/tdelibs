/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#ifndef KPGENERALPAGE_H
#define KPGENERALPAGE_H

#include "kprintdialogpage.h"

class TQComboBox;
class TQButtonGroup;
class TQGroupBox;
class TQLabel;

class KDEPRINT_EXPORT KPGeneralPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPGeneralPage(KMPrinter *pr, DrMain *dr, TQWidget *parent = 0, const char *name = 0);
	~KPGeneralPage();

	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);

protected:
	void initialize();

protected slots:
	void slotOrientationChanged(int);
	void slotDuplexChanged(int);
	void slotNupChanged(int);

protected:
	QComboBox	*m_pagesize, *m_papertype, *m_inputslot;
	QComboBox	*m_startbanner, *m_endbanner;
	QButtonGroup	*m_orientbox, *m_duplexbox, *m_nupbox;
	QGroupBox	*m_bannerbox;
	QLabel		*m_orientpix, *m_duplexpix, *m_nuppix;
};

#endif
