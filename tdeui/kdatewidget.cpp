/*  This file is part of the KDE libraries
    Copyright (C) 2001 Waldo Bastian (bastian@kde.org)

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


#include <tqpopupmenu.h>
#include <tqcombobox.h>
#include <tqlayout.h>
#include <tqlineedit.h>

#include "knuminput.h"
#include "kglobal.h"
#include "klocale.h"
#include "kcalendarsystem.h"
//#include "kdatepicker.h"
#include "kdialog.h"

#include "kdatewidget.h"

class KDateWidgetSpinBox : public TQSpinBox
{
public:
  KDateWidgetSpinBox(int min, int max, TQWidget *parent)
    : TQSpinBox(min, max, 1, parent)
  {
     editor()->setAlignment(TQt::AlignRight);
  }
};

class KDateWidget::KDateWidgetPrivate
{
public:
   KDateWidgetSpinBox *m_day;
   TQComboBox *m_month;
   KDateWidgetSpinBox *m_year;
   TQDate m_dat;
};


KDateWidget::KDateWidget( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  init(TQDate());
  setDate(TQDate());
}

// ### HPB change TQDate to const TQDate & in KDE 4.0
KDateWidget::KDateWidget( TQDate date, TQWidget *parent,
			    const char *name )
  : TQWidget( parent, name )
{
  init(date);
  setDate(date);
}

// ### CFM Repaced by init(const TQDate&). Can be safely removed
//     when no risk of BIC
void KDateWidget::init()
{
  d = new KDateWidgetPrivate;
  TDELocale *locale = TDEGlobal::locale();
  TQHBoxLayout *layout = new TQHBoxLayout(this, 0, KDialog::spacingHint());
  layout->setAutoAdd(true);
  d->m_day = new KDateWidgetSpinBox(1, 1, this);
  d->m_month = new TQComboBox(false, this);
  for (int i = 1; ; ++i)
  {
    TQString str = locale->calendar()->monthName(i,
       locale->calendar()->year(TQDate()));
    if (str.isNull()) break;
    d->m_month->insertItem(str);
  }

  d->m_year = new KDateWidgetSpinBox(locale->calendar()->minValidYear(),
				     locale->calendar()->maxValidYear(), this);

  connect(d->m_day, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(slotDateChanged()));
  connect(d->m_month, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotDateChanged()));
  connect(d->m_year, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(slotDateChanged()));
}

void KDateWidget::init(const TQDate& date)
{
  d = new KDateWidgetPrivate;
  TDELocale *locale = TDEGlobal::locale();
  TQHBoxLayout *layout = new TQHBoxLayout(this, 0, KDialog::spacingHint());
  layout->setAutoAdd(true);
  d->m_day = new KDateWidgetSpinBox(1, 1, this);
  d->m_month = new TQComboBox(false, this);
  for (int i = 1; ; ++i)
  {
    TQString str = locale->calendar()->monthName(i,
       locale->calendar()->year(date));
    if (str.isNull()) break;
    d->m_month->insertItem(str);
  }

  d->m_year = new KDateWidgetSpinBox(locale->calendar()->minValidYear(),
				     locale->calendar()->maxValidYear(), this);

  connect(d->m_day, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(slotDateChanged()));
  connect(d->m_month, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotDateChanged()));
  connect(d->m_year, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(slotDateChanged()));
}

KDateWidget::~KDateWidget()
{
  delete d;
}

// ### HPB change TQDate to const TQDate & in KDE 4.0
void KDateWidget::setDate( TQDate date )
{
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();

  d->m_day->blockSignals(true);
  d->m_month->blockSignals(true);
  d->m_year->blockSignals(true);

  d->m_day->setMaxValue(calendar->daysInMonth(date));
  d->m_day->setValue(calendar->day(date));
  d->m_month->setCurrentItem(calendar->month(date)-1);
  d->m_year->setValue(calendar->year(date));

  d->m_day->blockSignals(false);
  d->m_month->blockSignals(false);
  d->m_year->blockSignals(false);

  d->m_dat = date;
  emit changed(d->m_dat);
}

TQDate KDateWidget::date() const
{
  return d->m_dat;
}

void KDateWidget::slotDateChanged( )
{
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();

  TQDate date;
  int y,m,day;

  y = d->m_year->value();
  y = QMIN(QMAX(y, calendar->minValidYear()), calendar->maxValidYear());

  calendar->setYMD(date, y, 1, 1);
  m = d->m_month->currentItem()+1;
  m = QMIN(QMAX(m,1), calendar->monthsInYear(date));

  calendar->setYMD(date, y, m, 1);
  day = d->m_day->value();
  day = QMIN(QMAX(day,1), calendar->daysInMonth(date));

  calendar->setYMD(date, y, m, day);
  setDate(date);
}

void KDateWidget::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdatewidget.moc"
