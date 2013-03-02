/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@kde.org>

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
#ifndef KLISTVIEWLINEEDIT_H
#define KLISTVIEWLINEEDIT_H

#include <klineedit.h>
#include <tdelistview.h>

/**
 * the editor for a TDEListView.  please don't use this.
 * @internal
 **/
class TDEUI_EXPORT TDEListViewLineEdit : public KLineEdit
{
Q_OBJECT
public:
	TDEListViewLineEdit(TDEListView *parent);
	~TDEListViewLineEdit();

	TQListViewItem *currentItem() const;

signals:
	void done(TQListViewItem*, int);

public slots:
	void terminate();
	void load(TQListViewItem *i, int c);

protected:
	virtual void focusOutEvent(TQFocusEvent *);
	virtual void keyPressEvent(TQKeyEvent *e);
	virtual void paintEvent(TQPaintEvent *e);
	virtual bool event (TQEvent *pe);

	/// @since 3.1
	void selectNextCell (TQListViewItem *pi, int column, bool forward);
	void terminate(bool commit);
	TQListViewItem *item;
	int col;
	TDEListView* const p;

protected slots:
	void slotSelectionChanged();

	/// @since 3.5.4
	void slotItemRemoved(TQListViewItem *i);

};

#endif
