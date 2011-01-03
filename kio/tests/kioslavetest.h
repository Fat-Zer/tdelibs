 /*
  This file is or will be part of KDE desktop environment

  Copyright 1999 Matt Koss <koss@miesto.sk>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#ifndef _KIOSLAVETEST_H
#define _KIOSLAVETEST_H

#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqradiobutton.h>
#include <tqpushbutton.h>
#include <tqbuttongroup.h>
#include <tqwidget.h>

#include <kmainwindow.h>

#include "kio/job.h"
#include "kio/global.h"
#include "kio/statusbarprogress.h"
#include "kio/slave.h"

class KioslaveTest : public KMainWindow {
  Q_OBJECT

public:
  KioslaveTest( TQString src, TQString dest, uint op, uint pr );
  ~KioslaveTest() {}

  enum Operations { List, ListRecursive, Stat, Get, Put, Copy, Move, Delete, Shred, Mkdir, Mimetype };

  enum ProgressModes { ProgressNone, ProgressDefault, ProgresstqStatus };

protected:

  void closeEvent( TQCloseEvent * );

  void printUDSEntry( const KIO::UDSEntry & entry );

  // info stuff
  TQLabel *lb_from;
  TQLineEdit *le_source;

  TQLabel *lb_to;
  TQLineEdit *le_dest;

  // operation stuff
  TQButtonGroup *opButtons;

  TQRadioButton *rbList;
  TQRadioButton *rbListRecursive;
  TQRadioButton *rbStat;
  TQRadioButton *rbGet;
  TQRadioButton *rbPut;
  TQRadioButton *rbCopy;
  TQRadioButton *rbMove;
  TQRadioButton *rbDelete;
  TQRadioButton *rbShred;
  TQRadioButton *rbMkdir;
  TQRadioButton *rbMimetype;

  // progress stuff
  TQButtonGroup *progressButtons;

  TQRadioButton *rbProgressNone;
  TQRadioButton *rbProgressDefault;
  TQRadioButton *rbProgresstqStatus;

  TQPushButton *pbStart;
  TQPushButton *pbStop;

  TQPushButton *close;

protected slots:
  void changeOperation( int id );
  void changeProgressMode( int id );

  void startJob();
  void stopJob();

  void slotResult( KIO::Job * );
  void slotEntries( KIO::Job *, const KIO::UDSEntryList& );
  void slotData( KIO::Job *, const TQByteArray &data );
  void slotDataReq( KIO::Job *, TQByteArray &data );

  void slotQuit();
  void slotSlaveConnected();
  void slotSlaveError();

private:
  KIO::Job *job;
  TQWidget *main_widget;

  KIO::tqStatusbarProgress *statusProgress;

  int selectedOperation;
  int progressMode;
  int putBuffer;
  KIO::Slave *slave;
};

#endif // _KIOSLAVETEST_H
