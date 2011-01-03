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

#include <tqtooltip.h>
#include <tqlayout.h>
#include <tqwidgetstack.h>
#include <tqpushbutton.h>
#include <tqlabel.h>

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kprogress.h>

#include "jobclasses.h"
#include "statusbarprogress.h"

namespace KIO {

tqStatusbarProgress::tqStatusbarProgress( TQWidget* parent, bool button )
  : ProgressBase( parent ) {

  m_bShowButton = button;

  // only clean this dialog
  setOnlyClean(true);
  // TODO : is this really needed ?
  setStopOnClose(false);

  int w = fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
  box = new TQHBoxLayout( this, 0, 0 );

  m_pButton = new TQPushButton( "X", this );
  box->addWidget( m_pButton  );
  stack = new TQWidgetStack( this );
  box->addWidget( stack );
  connect( m_pButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotStop() ) );

  m_pProgressBar = new KProgress( this );
  m_pProgressBar->setFrameStyle( TQFrame::Box | TQFrame::Raised );
  m_pProgressBar->setLineWidth( 1 );
  m_pProgressBar->setBackgroundMode( TQWidget::PaletteBackground );
  m_pProgressBar->installEventFilter( this );
  m_pProgressBar->setMinimumWidth( w );
  stack->addWidget( m_pProgressBar, 1 );

  m_pLabel = new TQLabel( "", this );
  m_pLabel->tqsetAlignment( AlignHCenter | AlignVCenter );
  m_pLabel->installEventFilter( this );
  m_pLabel->setMinimumWidth( w );
  stack->addWidget( m_pLabel, 2 );
  setMinimumSize( tqsizeHint() );

  mode = None;
  setMode();
}


void tqStatusbarProgress::setJob( KIO::Job *job )
{
  ProgressBase::setJob( job );

  mode = Progress;
  setMode();
}


void tqStatusbarProgress::setMode() {
  switch ( mode ) {
  case None:
    if ( m_bShowButton ) {
      m_pButton->hide();
    }
    stack->hide();
    break;

  case Label:
    if ( m_bShowButton ) {
      m_pButton->show();
    }
    stack->show();
    stack->raiseWidget( m_pLabel );
    break;

  case Progress:
    if ( m_bShowButton ) {
      m_pButton->show();
    }
    stack->show();
    stack->raiseWidget( m_pProgressBar );
    break;
  }
}


void tqStatusbarProgress::slotClean() {
  // we don't want to delete this widget, only clean
  m_pProgressBar->setValue( 0 );
  m_pLabel->clear();

  mode = None;
  setMode();
}


void tqStatusbarProgress::slotTotalSize( KIO::Job*, KIO::filesize_t size ) {
  m_iTotalSize = size;  // size is measured in bytes
}

void tqStatusbarProgress::slotPercent( KIO::Job*, unsigned long percent ) {
  m_pProgressBar->setValue( percent );
}


void tqStatusbarProgress::slotSpeed( KIO::Job*, unsigned long speed ) {
  if ( speed == 0 ) { // spped is measured in bytes-per-second
    m_pLabel->setText( i18n( " Stalled ") );
  } else {
    m_pLabel->setText( i18n( " %1/s ").arg( KIO::convertSize( speed )) );
  }
}


bool tqStatusbarProgress::eventFilter( TQObject *, TQEvent *ev ) {
  if ( ! m_pJob ) { // don't react when there isn't any job doing IO
    return true;
  }

  if ( ev->type() == TQEvent::MouseButtonPress ) {
    TQMouseEvent *e = (TQMouseEvent*)ev;

    if ( e->button() == LeftButton ) {    // toggle view on left mouse button
      if ( mode == Label ) {
	mode = Progress;
      } else if ( mode == Progress ) {
	mode = Label;
      }
      setMode();
      return true;

    }
  }

  return false;
}

void tqStatusbarProgress::virtual_hook( int id, void* data )
{ ProgressBase::virtual_hook( id, data ); }

} /* namespace */
#include "statusbarprogress.moc"
