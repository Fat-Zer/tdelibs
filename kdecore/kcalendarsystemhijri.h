/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCALENDARSYSTEMHIJRI_H
#define KCALENDARSYSTEMHIJRI_H

#include <tqdatetime.h>
#include <tqstring.h>

#include "kcalendarsystem.h"

class KCalendarSystemHijriPrivate;

/**
 * @internal
 * This is the Hijri calendar implementation.
 *
 * The Hijri calendar is the traditional Islamic calendar used in the Midle
 * East.
 *
 * @see KLocale,KCalendarSystem,KCalendarSystemFactory
 *
 * @author Carlos Moro <cfmoro@correo.uniovi.es>
 * @since 3.2
 */
class KDECORE_EXPORT KCalendarSystemHijri : public KCalendarSystem
{
public:
  /** Constructor. Just like KCalendarSystem::KCalendarSystem(). */
  KCalendarSystemHijri(const KLocale * locale = 0);
  virtual ~KCalendarSystemHijri();

  virtual int year (const TQDate & date) const;
  virtual int month (const TQDate & date) const;
  virtual int day (const TQDate & date) const;
  virtual int dayOfWeek (const TQDate & date) const;
  virtual int dayOfYear (const TQDate & date) const;

  virtual bool setYMD(TQDate & date, int y, int m, int d) const;

  virtual TQDate addYears(const TQDate & date, int nyears) const;
  virtual TQDate addMonths(const TQDate & date, int nmonths) const;
  virtual TQDate addDays(const TQDate & date, int ndays) const;

  virtual int monthsInYear (const TQDate & date) const;
  virtual int daysInYear (const TQDate & date) const;
  virtual int daysInMonth (const TQDate & date) const;
  virtual int weeksInYear(int year) const;
  virtual int weekNumber(const TQDate& date, int * yearNum = 0) const;

  virtual TQString monthName (int month, int year, bool shortName = false) const;
  virtual TQString monthName (const TQDate & date, bool shortName = false ) const;
  virtual TQString monthNamePossessive(int month, int year, bool shortName = false) const;
  virtual TQString monthNamePossessive(const TQDate & date, bool shortName = false ) const;
  virtual TQString weekDayName (int weekDay, bool shortName = false) const;
  virtual TQString weekDayName (const TQDate & date, bool shortName = false) const;

  virtual int minValidYear () const;
  virtual int maxValidYear () const;
  virtual int weekDayOfPray () const;

  virtual TQString calendarName() const;

  virtual bool isLunar() const;
  virtual bool isLunisolar() const;
  virtual bool isSolar() const;

  private:
  /**
   * Gets the number of days in a month for a given date
   *
   * @param month month number
   * @param year given rear
   * @return number of days in month
   */
  int hndays(int month, int year) const;

  KCalendarSystemHijriPrivate * d;
};

#endif
