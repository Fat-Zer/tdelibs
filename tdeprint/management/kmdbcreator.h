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

#ifndef KMDBCREATOR_H
#define KMDBCREATOR_H

#include <tqobject.h>
#include <tqstring.h>
#include <tqdatetime.h>
#include <kprocess.h>

class TQWidget;
class TQProgressDialog;

class KMDBCreator : public TQObject
{
	Q_OBJECT
public:
	KMDBCreator(TQObject *parent = 0, const char *name = 0);
	~KMDBCreator();

	bool checkDriverDB(const TQString& dirname, const TQDateTime& d);
	bool createDriverDB(const TQString& dirname, const TQString& filename, TQWidget *parent = 0);
	bool status() const	{ return m_status; }

protected slots:
	void slotReceivedStdout(TDEProcess *p, char *bufm, int len);
	void slotReceivedStderr(TDEProcess *p, char *bufm, int len);
	void slotProcessExited(TDEProcess *p);
	void slotCancelled();

signals:
	void dbCreated();

private:
	TDEProcess	m_proc;
	TQProgressDialog	*m_dlg;
	bool		m_status;
	bool		m_firstflag;
};

#endif
