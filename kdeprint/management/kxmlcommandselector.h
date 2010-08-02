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

#ifndef KXMLCOMMANDSELECTOR_H
#define KXMLCOMMANDSELECTOR_H

#include <tqwidget.h>
#include <tqstringlist.h>

#include <kdelibs_export.h>

class TQComboBox;
class TQLineEdit;
class TQCheckBox;
class TQLabel;
class TQPushButton;
class KDialogBase;

class KDEPRINT_EXPORT KXmlCommandSelector : public QWidget
{
	Q_OBJECT
public:
	KXmlCommandSelector(bool canBeNull = true, TQWidget *parent = 0, const char *name = 0, KDialogBase *dlg = 0);

	void setCommand(const TQString&);
	TQString command() const;

protected:
	void loadCommands();

protected slots:
	void slotAddCommand();
	void slotEditCommand();
	void slotBrowse();
	void slotCommandSelected(int);
	void slotHelpCommand();
	void slotXmlCommandToggled( bool );

signals:
	void commandValid( bool );

private:
	QComboBox	*m_cmd;
	QLineEdit	*m_line;
	QCheckBox	*m_usefilter;
	QStringList	m_cmdlist;
	QLabel		*m_shortinfo;
	TQPushButton *m_helpbtn;
	TQString m_help;
};

#endif
