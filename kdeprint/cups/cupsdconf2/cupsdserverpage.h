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

#ifndef CUPSDSERVERPAGE_H
#define CUPSDSERVERPAGE_H

#include "cupsdpage.h"

class TQLineEdit;
class TQCheckBox;
class TQComboBox;

class CupsdServerPage : public CupsdPage
{
	Q_OBJECT

public:
	CupsdServerPage(TQWidget *parent = 0, const char *name = 0);

	bool loadConfig(CupsdConf*, TQString&);
	bool saveConfig(CupsdConf*, TQString&);
	void setInfos(CupsdConf*);

protected slots:
	void classChanged(int);

private:
	TQLineEdit	*servername_, *serveradmin_, *language_, *printcap_, *otherclassname_;
	TQComboBox	*classification_, *charset_, *printcapformat_;
	TQCheckBox	*classoverride_;
};

#endif
