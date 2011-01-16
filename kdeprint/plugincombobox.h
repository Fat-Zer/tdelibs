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

#ifndef PLUGINCOMBOBOX_H
#define PLUGINCOMBOBOX_H

#include <tqwidget.h>
#include <tqstringlist.h>

#include "kpreloadobject.h"

class TQComboBox;
class TQLabel;

class KDEPRINT_EXPORT PluginComboBox : public TQWidget, public KPReloadObject
{
	Q_OBJECT
public:
	PluginComboBox(TQWidget *parent = 0, const char *name = 0);

protected slots:
	void slotActivated(int);

protected:
	void reload();
	void configChanged();

private:
	TQComboBox	*m_combo;
	TQLabel		*m_plugininfo;
	TQStringList	m_pluginlist;
};

#endif
