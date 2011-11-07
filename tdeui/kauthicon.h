/* This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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
#ifndef _KAUTHICON_H
#define _KAUTHICON_H "$Id$"

#include <tqfileinfo.h>
#include <tqpixmap.h>
#include <tqstring.h>
#include <tqwidget.h>

#include <kdelibs_export.h>

class TQHBoxLayout;
class TQLabel;
class KAuthIconPrivate;

/**
 * @short A base class for authorization icon widgets
 *
 * This is the base class from which different authorization icon widget
 * which actually do something should be derived.  You can use these
 * widgets to show that the user has (or doesn't have) the ability to do
 * something, and why that is.
 *
 * One of the most useful things you can do with this is connect
 * authChanged(bool) to setEnabled(bool) for a widget to turn it on and
 * off depending on the status of whatever it is you are monitoring.
 *
 * @see KRootPermsIcon, KWritePermsIcon
 * @author Preston Brown <pbrown@kde.org>
 */
class TDEUI_EXPORT KAuthIcon : public TQWidget
{
  Q_OBJECT

public:
  /**
   * Constructor.
   */
  KAuthIcon(TQWidget *parent = 0, const char *name = 0);
  ~KAuthIcon();

  virtual TQSize tqsizeHint() const;
  /**
   * return the status of whatever is being monitored.
   */
  virtual bool status() const = 0;

public slots:
  /**
   * Re-implement this method if you want the icon to update itself
   * when something external has changed (i.e. a file on disk, uid/gid).
   */
  virtual void updateStatus() = 0;

signals:
  /**
   * this signal is emitted when authorization has changed from
   * its previous state.
   * @param authorized will be true if the type of authorization
   * described by the icon is true, otherwise it will be false.
   */
  void authChanged(bool authorized);

protected:
  TQHBoxLayout *layout;

  TQLabel *lockBox;
  TQLabel *lockLabel;
  TQPixmap lockPM;
  TQPixmap openLockPM;
  TQString lockText;
  TQString openLockText;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KAuthIconPrivate *d;
};

class KRootPermsIconPrivate;
/**
 * Icon to show whether or not a user has root permissions.
 *
 * @see KAuthIcon
 * @author Preston Brown <pbrown@kde.org>
 */
class TDEUI_EXPORT KRootPermsIcon : public KAuthIcon
{
  Q_OBJECT

public:
  KRootPermsIcon(TQWidget *parent = 0, const char *name = 0);
  ~KRootPermsIcon();

  /**
   * return whether or not the current user has root permissions.
   */
  bool status() const { return root; }

public slots:
  void updateStatus();

protected:
  bool root;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KRootPermsIconPrivate *d;
};

class KWritePermsIconPrivate;
/**
 * Auth icon for write permission display.
 *
 * @see KAuthIcon
 * @author Preston Brown <pbrown@kde.org>
 */
class TDEUI_EXPORT KWritePermsIcon : public KAuthIcon
{
  Q_OBJECT
  Q_PROPERTY( TQString fileName READ fileName WRITE setFileName )

public:
  KWritePermsIcon(const TQString & fileName, TQWidget *parent = 0, const char *name = 0);
  ~KWritePermsIcon();
  /**
   * @return whether or not the monitored file is writable.
   */
  bool status() const { return writable; }

  /**
   * make the icon watch a new filename.
   * @param fileName the new file to monitor / display status for.
   */
  void setFileName(const TQString & fileName) { fi.setFile(fileName); updateStatus(); }

  /**
  * return the filename of the currently watched file.
  * @since 3.4
  */
  TQString fileName() const { return fi.fileName(); }

public slots:
  void updateStatus();

protected:
  bool writable;
  TQFileInfo fi;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KWritePermsIconPrivate *d;
};

#endif
