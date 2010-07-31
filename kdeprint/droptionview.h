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

#ifndef DROPTIONVIEW_H
#define DROPTIONVIEW_H

#include <tqwidget.h>
#include <tqgroupbox.h>
#include <tqstringlist.h>

class QLineEdit;
class QSlider;
class QLabel;
class KListBox;
class QListBoxItem;
class QVButtonGroup;
class QWidgetStack;
class QListViewItem;
class DrBase;
class DriverItem;

class OptionBaseView : public QWidget
{
	Q_OBJECT
public:
	OptionBaseView(TQWidget *parent = 0, const char *name = 0);
	virtual void setOption(DrBase*);
	virtual void setValue(const TQString&);

signals:
	void valueChanged(const TQString&);

protected:
	bool	blockSS;
};

class OptionNumericView : public OptionBaseView
{
	Q_OBJECT
public:
	OptionNumericView(TQWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const TQString& val);

protected slots:
	void slotSliderChanged(int);
	void slotEditChanged(const TQString&);

private:
	QLineEdit	*m_edit;
	QSlider		*m_slider;
	QLabel		*m_minval, *m_maxval;
	bool		m_integer;
};

class OptionStringView : public OptionBaseView
{
public:
	OptionStringView(TQWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const TQString& val);

private:
	QLineEdit	*m_edit;
};

class OptionListView : public OptionBaseView
{
	Q_OBJECT
public:
	OptionListView(TQWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const TQString& val);

protected slots:
	void slotSelectionChanged();

private:
	KListBox	*m_list;
	QStringList	m_choices;
};

class OptionBooleanView : public OptionBaseView
{
	Q_OBJECT
public:
	OptionBooleanView(TQWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const TQString& val);

protected slots:
	void slotSelected(int);

private:
	QVButtonGroup	*m_group;
	QStringList	m_choices;
};

class DrOptionView : public QGroupBox
{
	Q_OBJECT
public:
	DrOptionView(TQWidget *parent = 0, const char *name = 0);
	void setAllowFixed(bool on) 	{ m_allowfixed = on; }

signals:
	void changed();

public slots:
	void slotValueChanged(const TQString&);
	void slotItemSelected(TQListViewItem*);

private:
	QWidgetStack	*m_stack;
	DriverItem	*m_item;
	bool		m_block;
	bool		m_allowfixed;
};

#endif
