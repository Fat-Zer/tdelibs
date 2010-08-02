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

#ifndef CUPSADDSMB_H
#define CUPSADDSMB_H

#include <tqobject.h>
#include <tqstringlist.h>
#include <kprocess.h>
#include <kdialog.h>

class TQProgressBar;
class SidePixmap;
class TQPushButton;
class TQLabel;
class KActiveLabel;
class TQLineEdit;

class CupsAddSmb : public KDialog
{
	Q_OBJECT

public:
	enum State { None, Start, MkDir, Copy, AddDriver, AddPrinter };
	CupsAddSmb(TQWidget *parent = 0, const char *name = 0);
	~CupsAddSmb();

	static bool exportDest(const TQString& dest, const TQString& datadir);

protected slots:
	void slotReceived(KProcess*, char*, int);
	void doNextAction();
	void slotProcessExited(KProcess*);
	void slotActionClicked();

protected:
	void checkActionStatus();
	void nextAction();
	bool startProcess();
	bool doExport();
	bool doInstall();
	void showError(const TQString& msg);

private:
	KProcess	m_proc;
	QStringList	m_buffer;
	int			m_state;
	QStringList	m_actions;
	int			m_actionindex;
	bool		m_status;
	QProgressBar	*m_bar;
	QString		m_dest;
	SidePixmap	*m_side;
	QPushButton	*m_doit, *m_cancel;
	KActiveLabel	*m_text;
	TQLabel *m_textinfo;
	TQLineEdit *m_logined, *m_passwded, *m_servered;
	QString	m_datadir;
};

#endif
