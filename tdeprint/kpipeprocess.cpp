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

#include "kpipeprocess.h"

KPipeProcess::KPipeProcess(const TQString& cmd, int mode)
: TQFile()
{
	m_pipe = NULL;
	if (!cmd.isEmpty())
		open(cmd,mode);
}

KPipeProcess::~KPipeProcess()
{
	close();
}

bool KPipeProcess::open(const TQString& cmd, int mode)
{
	// close first if needed
	close();
	// check supported modes
	if (mode != IO_ReadOnly && mode != IO_WriteOnly)
		return false;

	// create the pipe
	m_pipe = popen(cmd.latin1(),(mode == IO_WriteOnly ? "w" : "r"));
	if (m_pipe)
		if (!TQFile::open(mode,m_pipe))
			close();
	return (m_pipe != NULL);
}

void KPipeProcess::close()
{
	if (m_pipe != NULL)
	{
		TQFile::close();
		/* pipe must be close to be sure the process is terminated */
		pclose( m_pipe );
		m_pipe = NULL;
	}
}
