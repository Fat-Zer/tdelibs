/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

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

#include "jobclasses.h"
#include "progressbase.h"

namespace KIO {

ProgressBase::ProgressBase( TQWidget *parent )
  : TQWidget( parent )
{
  m_pJob = 0;

  // delete dialog after the job is finished / canceled
  m_bOnlyClean = false;

  // stop job on close
  m_bStopOnClose = true;
}


void ProgressBase::setJob( KIO::Job *job )
{
  // first connect all slots
  connect( job, TQT_SIGNAL( percent( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotPercent( KIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
	   TQT_SLOT( slotFinished( KIO::Job* ) ) );

  connect( job, TQT_SIGNAL( canceled( KIO::Job* ) ),
	   TQT_SLOT( slotFinished( KIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( KIO::CopyJob *job )
{
  // first connect all slots
  connect( job, TQT_SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ),
	   TQT_SLOT( slotTotalSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, TQT_SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ),
	   TQT_SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, TQT_SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( percent( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotPercent( KIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( copying( KIO::Job*, const KURL& , const KURL& ) ),
	   TQT_SLOT( slotCopying( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( job, TQT_SIGNAL( moving( KIO::Job*, const KURL& , const KURL& ) ),
	   TQT_SLOT( slotMoving( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( job, TQT_SIGNAL( creatingDir( KIO::Job*, const KURL& ) ),
 	   TQT_SLOT( slotCreatingDir( KIO::Job*, const KURL& ) ) );

  connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
	   TQT_SLOT( slotFinished( KIO::Job* ) ) );

  connect( job, TQT_SIGNAL( canceled( KIO::Job* ) ),
	   TQT_SLOT( slotFinished( KIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( KIO::DeleteJob *job )
{
  // first connect all slots
  connect( job, TQT_SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ),
	   TQT_SLOT( slotTotalSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, TQT_SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ),
	   TQT_SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, TQT_SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( percent( KIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotPercent( KIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( deleting( KIO::Job*, const KURL& ) ),
	   TQT_SLOT( slotDeleting( KIO::Job*, const KURL& ) ) );

  connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
	   TQT_SLOT( slotFinished( KIO::Job* ) ) );

  connect( job, TQT_SIGNAL( canceled( KIO::Job* ) ),
	   TQT_SLOT( slotFinished( KIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::closeEvent( TQCloseEvent* ) {
  // kill job when desired
  if ( m_bStopOnClose ) {
    slotStop();
  } else {
    // clean or delete dialog
    if ( m_bOnlyClean ) {
      slotClean();
    } else {
      delete this;
    }
  }
}

void ProgressBase::finished() {
  // clean or delete dialog
  if ( m_bOnlyClean ) {
    slotClean();
  } else {
    deleteLater();
  }
}

void ProgressBase::slotFinished( KIO::Job* ) {
  finished();
}


void ProgressBase::slotStop() {
  if ( m_pJob ) {
    m_pJob->kill(); // this will call slotFinished
    m_pJob = 0L;
  } else {
    slotFinished( 0 ); // here we call it ourselves
  }

  emit stopped();
}


void ProgressBase::slotClean() {
  hide();
}

void ProgressBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

} /* namespace */

#include "progressbase.moc"

