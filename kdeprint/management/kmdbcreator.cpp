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

#include "kmdbcreator.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <tqprogressdialog.h>
#include <tqfileinfo.h>
#include <tqdir.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>

KMDBCreator::KMDBCreator(TQObject *parent, const char *name)
: TQObject(parent,name)
{
	m_dlg = 0;
	m_status = true;

	connect(&m_proc,TQT_SIGNAL(receivedStdout(KProcess*,char*,int)),TQT_SLOT(slotReceivedStdout(KProcess*,char*,int)));
	connect(&m_proc,TQT_SIGNAL(receivedStderr(KProcess*,char*,int)),TQT_SLOT(slotReceivedStderr(KProcess*,char*,int)));
	connect(&m_proc,TQT_SIGNAL(processExited(KProcess*)),TQT_SLOT(slotProcessExited(KProcess*)));
}

KMDBCreator::~KMDBCreator()
{
	if (m_proc.isRunning())
		m_proc.kill();
	// do not delete the progress dialog anymore: it's persistent and owned by
	// it's parent. It will be destroyed along with its parent.
}

bool KMDBCreator::checkDriverDB(const TQString& dirname, const TQDateTime& d)
{
	// don't block GUI
	kapp->processEvents();

	// first check current directory
	TQFileInfo	dfi(dirname);
	if (dfi.lastModified() > d)
		return false;

	// then check most recent file in current directory
	TQDir	dir(dirname);
	const TQFileInfoList	*list = dir.entryInfoList(TQDir::Files,TQDir::Time);
	if (list && list->count() > 0 && list->getFirst()->lastModified() > d)
		return false;

	// then loop into subdirs
	TQStringList	slist = dir.entryList(TQDir::Dirs,TQDir::Time);
	for (TQStringList::ConstIterator it=slist.begin(); it!=slist.end(); ++it)
		if ((*it) != "." && (*it) != ".." && !checkDriverDB(dir.absFilePath(*it),d))
			return false;

	// everything is OK
	return true;
}

bool KMDBCreator::createDriverDB(const TQString& dirname, const TQString& filename, TQWidget *parent)
{
	bool	started(true);

	// initialize status
	m_status = false;
	m_firstflag = true;

	// start the child process
	m_proc.clearArguments();
	TQString	exestr = KMFactory::self()->manager()->driverDbCreationProgram();
	m_proc << exestr << dirname << filename;
	kdDebug() << "executing : " << exestr << " " << dirname << " " << filename << endl;
	TQString	msg;
	if (exestr.isEmpty())
		msg = i18n("No executable defined for the creation of the "
		           "driver database. This operation is not implemented.");
	else if (KStandardDirs::findExe(exestr).isEmpty())
		msg = i18n("The executable %1 could not be found in your "
		           "PATH. Check that this program exists and is "
			   "accessible in your PATH variable.").arg(exestr);
	else if (!m_proc.start(KProcess::NotifyOnExit, KProcess::AllOutput))
		msg = i18n("Unable to start the creation of the driver "
		           "database. The execution of %1 failed.").arg(exestr);
	if (!msg.isEmpty())
	{
		KMManager::self()->setErrorMsg(msg);
		started = false;
	}

	// Create the dialog if the process is running and if needed
	if (started)
	{
		if (!m_dlg)
		{
			m_dlg = new TQProgressDialog(parent->tqtopLevelWidget(),"progress-dialog",true);
			m_dlg->setLabelText(i18n("Please wait while KDE rebuilds a driver database."));
			m_dlg->setCaption(i18n("Driver Database"));
			connect(m_dlg,TQT_SIGNAL(canceled()),TQT_SLOT(slotCancelled()));
		}
		m_dlg->setMinimumDuration(0);	// always show the dialog
		m_dlg->setProgress(0);		// to force showing
	}
	else
		// be sure to emit this signal otherwise the DB widget won't never be notified
		emit dbCreated();

	return started;
}

void KMDBCreator::slotReceivedStdout(KProcess*, char *buf, int len)
{
	// save buffer
	TQString	str( TQCString(buf, len) );

	// get the number, cut the string at the first '\n' otherwise
	// the toInt() will return 0. If that occurs for the first number,
	// then the number of steps will be also 0.
	bool	ok;
	int	p = str.find('\n');
	int	n = str.mid(0, p).toInt(&ok);

	// process the number received
	if (ok && m_dlg)
	{
		if (m_firstflag)
		{
			m_dlg->setTotalSteps(n);
			m_firstflag = false;
		}
		else
		{
			m_dlg->setProgress(n);
		}
	}
}

void KMDBCreator::slotReceivedStderr(KProcess*, char*, int)
{
	// just discard it for the moment
}

void KMDBCreator::slotProcessExited(KProcess*)
{
	// delete the progress dialog
	if (m_dlg)
	{
		m_dlg->reset();
	}

	// set exit status
	m_status = (m_proc.normalExit() && m_proc.exitStatus() == 0);
	if (!m_status)
	{
		KMFactory::self()->manager()->setErrorMsg(i18n("Error while creating driver database: abnormal child-process termination."));
		// remove the incomplete driver DB file so that, it will be
		// reconstructed on next check
		TQFile::remove(m_proc.args()[2]);
	}
	//else
		emit dbCreated();
}

void KMDBCreator::slotCancelled()
{
	if (m_proc.isRunning())
		m_proc.kill();
	else
		emit dbCreated();
}
#include "kmdbcreator.moc"
