 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <tqfile.h>

#include <kdebug.h>
#include <kcmdlineargs.h>

#include "kdirwatchtest.h"

static const KCmdLineOptions options[] =
{
  {"+[directory ...]", "Directory(ies) to watch", 0},
  KCmdLineLastOption
};


int main (int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, "KDirWatchTest",
		     "Test for KDirWatch", "1.0");
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication::addCmdLineOptions();

  KApplication a;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  myTest testObject;

  KDirWatch *dirwatch1 = KDirWatch::self();
  KDirWatch *dirwatch2 = new KDirWatch;

  testObject.connect(dirwatch1, TQT_SIGNAL( dirty( const TQString &)), TQT_SLOT( dirty( const TQString &)) );
  testObject.connect(dirwatch1, TQT_SIGNAL( created( const TQString &)), TQT_SLOT( created( const TQString &)) );
  testObject.connect(dirwatch1, TQT_SIGNAL( deleted( const TQString &)), TQT_SLOT( deleted( const TQString &)) );

  if (args->count() >0) {
    for(int i = 0; i < args->count(); i++) {
      kdDebug() << "Watching: " << args->arg(i) << endl;
      dirwatch2->addDir( TQFile::decodeName( args->arg(i)));
    }
  }

  TQString home = TQString(getenv ("HOME")) + "/";
  TQString desk = home + "Desktop/";
  kdDebug() << "Watching: " << home << endl;
  dirwatch1->addDir(home);
  kdDebug() << "Watching file: " << home << "foo " << endl;
  dirwatch1->addFile(home+"foo");
  kdDebug() << "Watching: " << desk << endl;
  dirwatch1->addDir(desk);
  TQString test = home + "test/";
  kdDebug() << "Watching: (but skipped) " << test << endl;
  dirwatch1->addDir(test);

  dirwatch1->startScan();
  dirwatch2->startScan();

  if(!dirwatch1->stopDirScan(home))
      kdDebug() << "stopDirscan: " << home << " error!" << endl;
  if(!dirwatch1->restartDirScan(home))
      kdDebug() << "restartDirScan: " << home << "error!" << endl;
  if (!dirwatch1->stopDirScan(test))
     kdDebug() << "stopDirScan: error" << endl;

  KDirWatch::statistics();

  delete dirwatch2;

  KDirWatch::statistics();

  return a.exec();
}
#include "kdirwatchtest.moc"
