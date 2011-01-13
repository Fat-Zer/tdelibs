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

#include "gschecker.h"
#include "kpipeprocess.h"

#include <tqfile.h>
#include <tqtextstream.h>

GsChecker::GsChecker(TQObject *parent, const char *name)
: TQObject(parent,name)
{
}

bool GsChecker::checkGsDriver(const TQString& name)
{
	if (m_driverlist.count() == 0)
		loadDriverList();
	return m_driverlist.contains(name);
}

void GsChecker::loadDriverList()
{
	KPipeProcess	proc;
	if (proc.open("gs -h",IO_ReadOnly))
	{
		QTextStream	t(&proc);
		QString	buffer, line;
		bool	ok(false);
		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (ok)
			{
				if (line.tqfind(':') != -1)
					break;
				else
					buffer.append(line).append(" ");
			}
			else if (line.startsWith(TQString::tqfromLatin1("Available devices:")))
				ok = true;
		}
		m_driverlist = TQStringList::split(' ',buffer,false);
	}
}
