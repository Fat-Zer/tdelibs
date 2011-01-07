/* This file is part of the KDE project
  Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef KMDI_FOCUS_LIST
#define KMDI_FOCUS_LIST

#include <tqobject.h>
#include <tqmap.h>
#include <tqwidget.h>
#include <kdelibs_export.h>

class KMDI_EXPORT KMdiFocusList: public QObject
{
	Q_OBJECT
public:
	KMdiFocusList( TQObject *parent );
	~KMdiFocusList();
	void addWidgetTree( TQWidget* );
	void restore();
protected slots:
	void objectHasBeenDestroyed( TQObject* );
private:
	TQMap<TQWidget*, TQWidget::FocusPolicy> m_list;

};

#endif 
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
