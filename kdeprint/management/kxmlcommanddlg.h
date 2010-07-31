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

#ifndef KXMLCOMMANDDLG_H
#define KXMLCOMMANDDLG_H

#include <tqwidget.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <kdialogbase.h>

class KListView;
class QListViewItem;
class QLineEdit;
class QComboBox;
class QWidgetStack;
class QToolButton;
class KListBox;
class QListBoxItem;
class QTextEdit;
class QCheckBox;

class DrGroup;
class DrBase;
class KXmlCommand;

class KXmlCommandAdvancedDlg : public QWidget
{
	Q_OBJECT
public:
	KXmlCommandAdvancedDlg(TQWidget *parent = 0, const char *name = 0);
	~KXmlCommandAdvancedDlg();

	void setCommand(KXmlCommand*);
	static bool editCommand(KXmlCommand *xmlcmd, TQWidget *parent = 0);

protected:
	void parseGroupItem(DrGroup*, TQListViewItem*);
	void parseXmlCommand(KXmlCommand*);
	void viewItem(TQListViewItem*);
	void removeItem(TQListViewItem*);
	void recreateGroup(TQListViewItem*, DrGroup*);

protected slots:
	void slotSelectionChanged(TQListViewItem*);
	void slotTypeChanged(int);
	void slotAddValue();
	void slotRemoveValue();
	void slotApplyChanges();
	void slotAddGroup();
	void slotAddOption();
	void slotRemoveItem();
	void slotMoveUp();
	void slotMoveDown();
	void slotCommandChanged(const TQString&);
	void slotValueSelected(TQListViewItem*);
	void slotOptionRenamed(TQListViewItem*, int);
	void slotChanged();

private:
	KListView	*m_view;
	QLineEdit	*m_name, *m_desc, *m_format, *m_default, *m_command;
	QComboBox	*m_type;
	QWidget		*m_dummy;
	KListView	*m_values;
	QLineEdit	*m_edit1, *m_edit2;
	QWidgetStack	*m_stack;
	QToolButton	*m_apply, *m_addgrp, *m_addopt, *m_delopt, *m_up, *m_down;
	QLineEdit	*m_inputfile, *m_inputpipe, *m_outputfile, *m_outputpipe;
	QToolButton	*m_addval, *m_delval;
	TQTextEdit *m_comment;
	TQCheckBox *m_persistent;

	KXmlCommand	*m_xmlcmd;
	TQMap<TQString, DrBase*>	m_opts;
};

class KXmlCommandDlg : public KDialogBase
{
	Q_OBJECT
public:
	KXmlCommandDlg(TQWidget *parent = 0, const char *name = 0);

	void setCommand(KXmlCommand*);
	static bool editCommand(KXmlCommand*, TQWidget *parent = 0);

protected slots:
	void slotAddMime();
	void slotRemoveMime();
	void slotEditCommand();
	void slotAddReq();
	void slotRemoveReq();
	void slotReqSelected(TQListViewItem*);
	void slotAvailableSelected(TQListBoxItem*);
	void slotSelectedSelected(TQListBoxItem*);
	void slotOk();

private:
	QLineEdit	*m_description;
	QLabel		*m_idname;
	QComboBox	*m_mimetype;
	KListBox	*m_availablemime, *m_selectedmime;
	QToolButton	*m_addmime, *m_removemime;
	KListView	*m_requirements;
	QToolButton	*m_removereq, *m_addreq;

	QStringList	m_mimelist;
	KXmlCommand	*m_cmd;
};

#endif
