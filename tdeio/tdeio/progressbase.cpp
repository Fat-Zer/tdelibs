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

namespace TDEIO {

ProgressBase::ProgressBase( TQWidget *parent )
  : TQWidget( parent )
{
  m_pJob = 0;

  // delete dialog after the job is finished / canceled
  m_bOnlyClean = false;

  // stop job on close
  m_bStopOnClose = true;
}


void ProgressBase::setJob( TDEIO::Job *job )
{
  // first connect all slots
  connect( job, TQT_SIGNAL( percent( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotPercent( TDEIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ),
	   TQT_SLOT( slotFinished( TDEIO::Job* ) ) );

  connect( job, TQT_SIGNAL( canceled( TDEIO::Job* ) ),
	   TQT_SLOT( slotFinished( TDEIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( TDEIO::CopyJob *job )
{
  // first connect all slots
  connect( job, TQT_SIGNAL( totalSize( TDEIO::Job*, TDEIO::filesize_t ) ),
	   TQT_SLOT( slotTotalSize( TDEIO::Job*, TDEIO::filesize_t ) ) );
  connect( job, TQT_SIGNAL( totalFiles( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotTotalFiles( TDEIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( totalDirs( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotTotalDirs( TDEIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( processedSize( TDEIO::Job*, TDEIO::filesize_t ) ),
	   TQT_SLOT( slotProcessedSize( TDEIO::Job*, TDEIO::filesize_t ) ) );
  connect( job, TQT_SIGNAL( processedFiles( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotProcessedFiles( TDEIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( processedDirs( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotProcessedDirs( TDEIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( speed( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotSpeed( TDEIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( percent( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotPercent( TDEIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( copying( TDEIO::Job*, const KURL& , const KURL& ) ),
	   TQT_SLOT( slotCopying( TDEIO::Job*, const KURL&, const KURL& ) ) );
  connect( job, TQT_SIGNAL( moving( TDEIO::Job*, const KURL& , const KURL& ) ),
	   TQT_SLOT( slotMoving( TDEIO::Job*, const KURL&, const KURL& ) ) );
  connect( job, TQT_SIGNAL( creatingDir( TDEIO::Job*, const KURL& ) ),
 	   TQT_SLOT( slotCreatingDir( TDEIO::Job*, const KURL& ) ) );

  connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ),
	   TQT_SLOT( slotFinished( TDEIO::Job* ) ) );

  connect( job, TQT_SIGNAL( canceled( TDEIO::Job* ) ),
	   TQT_SLOT( slotFinished( TDEIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( TDEIO::DeleteJob *job )
{
  // first connect all slots
  connect( job, TQT_SIGNAL( totalSize( TDEIO::Job*, TDEIO::filesize_t ) ),
	   TQT_SLOT( slotTotalSize( TDEIO::Job*, TDEIO::filesize_t ) ) );
  connect( job, TQT_SIGNAL( totalFiles( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotTotalFiles( TDEIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( totalDirs( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotTotalDirs( TDEIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( processedSize( TDEIO::Job*, TDEIO::filesize_t ) ),
	   TQT_SLOT( slotProcessedSize( TDEIO::Job*, TDEIO::filesize_t ) ) );
  connect( job, TQT_SIGNAL( processedFiles( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotProcessedFiles( TDEIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( processedDirs( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotProcessedDirs( TDEIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( speed( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotSpeed( TDEIO::Job*, unsigned long ) ) );
  connect( job, TQT_SIGNAL( percent( TDEIO::Job*, unsigned long ) ),
	   TQT_SLOT( slotPercent( TDEIO::Job*, unsigned long ) ) );

  connect( job, TQT_SIGNAL( deleting( TDEIO::Job*, const KURL& ) ),
	   TQT_SLOT( slotDeleting( TDEIO::Job*, const KURL& ) ) );

  connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ),
	   TQT_SLOT( slotFinished( TDEIO::Job* ) ) );

  connect( job, TQT_SIGNAL( canceled( TDEIO::Job* ) ),
	   TQT_SLOT( slotFinished( TDEIO::Job* ) ) );

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

void ProgressBase::slotFinished( TDEIO::Job* ) {
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

