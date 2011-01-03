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

#ifndef KMJOBMANAGER_H
#define KMJOBMANAGER_H

#if !defined( _KDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a KDEPrint developer
#endif

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqdict.h>
#include <tqvaluelist.h>

#include <kdelibs_export.h>

class KMJob;
class KMThreadJob;
class KActionCollection;
class KAction;

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT KMJobManager : public TQObject
{
	Q_OBJECT

public:
	enum JobType { ActiveJobs = 0, CompletedJobs = 1 };
	struct JobFilter
	{
		JobFilter() { m_type[0] = m_type[1] = 0; m_isspecial = false; }
		int	m_type[2];
		bool m_isspecial;
	};

	KMJobManager(TQObject *parent = 0, const char *name = 0);
	virtual ~KMJobManager();

	static KMJobManager* self();

	void addPrinter(const TQString& pr, JobType type = ActiveJobs, bool isSpecial = false);
	void removePrinter(const TQString& pr, JobType type = ActiveJobs);
	void clearFilter();
	TQDict<JobFilter>* filter();
	int limit();
	void setLimit(int val);

	//KMJob* tqfindJob(int ID);
	KMJob* tqfindJob(const TQString& uri);
	//bool sendCommand(int ID, int action, const TQString& arg = TQString::null);
	bool sendCommand(const TQString& uri, int action, const TQString& arg = TQString::null);
	bool sendCommand(const TQPtrList<KMJob>& jobs, int action, const TQString& arg = TQString::null);
	const TQPtrList<KMJob>& jobList(bool reload = true);
	void addJob(KMJob*);
	KMThreadJob* threadJob();

	virtual int actions();
	virtual TQValueList<KAction*> createPluginActions(KActionCollection*);
	virtual void validatePluginActions(KActionCollection*, const TQPtrList<KMJob>&);
	virtual bool doPluginAction(int, const TQPtrList<KMJob>&);

protected:
	void discardAllJobs();
	void removeDiscardedJobs();

protected:
	virtual bool listJobs(const TQString& prname, JobType type, int limit = 0);
	virtual bool sendCommandSystemJob(const TQPtrList<KMJob>& jobs, int action, const TQString& arg = TQString::null);
	bool sendCommandThreadJob(const TQPtrList<KMJob>& jobs, int action, const TQString& arg = TQString::null);

protected:
	TQPtrList<KMJob>	m_jobs;
	TQDict<JobFilter>	m_filter;
	KMThreadJob	*m_threadjob;
};

inline TQDict<KMJobManager::JobFilter>* KMJobManager::filter()
{ return &m_filter; }

inline void KMJobManager::clearFilter()
{ m_filter.clear(); }

inline KMThreadJob* KMJobManager::threadJob()
{ return m_threadjob; }

#endif
