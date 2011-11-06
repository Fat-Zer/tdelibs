/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2000-2001 Wolfram Diestel <wolfram@steloj.de>
   Copyright (C) 2003 Zack Rusin <zack@kde.org>

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

#include <tqstringlist.h>
#include <tqpushbutton.h>
#include <tqlabel.h>
#include <tqlayout.h>

#include <kapplication.h>
#include <klocale.h>
#include <klistbox.h>
#include <kcombobox.h>
#include <klistview.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kprogress.h>
#include <kbuttonbox.h>
#include <kdebug.h>

#include "ksconfig.h"
#include "kspelldlg.h"
#include "kspellui.h"

//to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class KSpellDlg::KSpellDlgPrivate {
public:
  KSpellUI* ui;
  KSpellConfig* spellConfig;
};

KSpellDlg::KSpellDlg( TQWidget * parent, const char * name, bool _progressbar, bool _modal )
  : KDialogBase(
      parent, name, _modal, i18n("Check Spelling"), Help|Cancel|User1,
      Cancel, true, i18n("&Finished")
    ),
    progressbar( false )
{
  Q_UNUSED( _progressbar );
  d = new KSpellDlgPrivate;
  d->ui = new KSpellUI( this );
  setMainWidget( d->ui );

  connect( d->ui->m_replaceBtn, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(replace()));
  connect( this, TQT_SIGNAL(ready(bool)),
           d->ui->m_replaceBtn, TQT_SLOT(setEnabled(bool)) );

  connect( d->ui->m_replaceAllBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(replaceAll()));
  connect(this, TQT_SIGNAL(ready(bool)), d->ui->m_replaceAllBtn, TQT_SLOT(setEnabled(bool)));

  connect( d->ui->m_skipBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(ignore()));
  connect( this, TQT_SIGNAL(ready(bool)), d->ui->m_skipBtn, TQT_SLOT(setEnabled(bool)));

  connect( d->ui->m_skipAllBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(ignoreAll()));
  connect( this, TQT_SIGNAL(ready(bool)), d->ui->m_skipAllBtn, TQT_SLOT(setEnabled(bool)));

  connect( d->ui->m_addBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(add()));
  connect( this, TQT_SIGNAL(ready(bool)), d->ui->m_addBtn, TQT_SLOT(setEnabled(bool)));

  connect( d->ui->m_suggestBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(suggest()));
  connect( this, TQT_SIGNAL(ready(bool)), d->ui->m_suggestBtn, TQT_SLOT(setEnabled(bool)) );
  d->ui->m_suggestBtn->hide();

  connect(this, TQT_SIGNAL(user1Clicked()), this, TQT_SLOT(stop()));

  connect( d->ui->m_replacement, TQT_SIGNAL(textChanged(const TQString &)),
           TQT_SLOT(textChanged(const TQString &)) );

  connect( d->ui->m_replacement, TQT_SIGNAL(returnPressed()),   TQT_SLOT(replace()) );
  connect( d->ui->m_suggestions, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
           TQT_SLOT(slotSelectionChanged(TQListViewItem*)) );

  connect( d->ui->m_suggestions, TQT_SIGNAL( doubleClicked ( TQListViewItem *, const TQPoint &, int ) ),
           TQT_SLOT( replace() ) );
  d->spellConfig = new KSpellConfig( 0, 0 ,0, false );
  d->spellConfig->fillDicts( d->ui->m_language );
  connect( d->ui->m_language, TQT_SIGNAL(activated(int)),
	   d->spellConfig, TQT_SLOT(sSetDictionary(int)) );
  connect( d->spellConfig, TQT_SIGNAL(configChanged()),
           TQT_SLOT(slotConfigChanged()) );

  setHelp( "spelldlg", "kspell" );
  setMinimumSize( tqsizeHint() );
  emit ready( false );
}

KSpellDlg::~KSpellDlg()
{
  delete d->spellConfig;
  delete d;
}

void
KSpellDlg::init( const TQString & _word, TQStringList * _sugg )
{
  sugg = _sugg;
  word = _word;

  d->ui->m_suggestions->clear();
  d->ui->m_suggestions->setSorting( NONSORTINGCOLUMN );
  for ( TQStringList::Iterator it = _sugg->begin(); it != _sugg->end(); ++it ) {
    TQListViewItem *item = new TQListViewItem( d->ui->m_suggestions,
                                             d->ui->m_suggestions->lastItem() );
    item->setText( 0, *it );
  }
  kdDebug(750) << "KSpellDlg::init [" << word << "]" << endl;

  emit ready( true );

  d->ui->m_unknownWord->setText( _word );

  if ( sugg->count() == 0 ) {
    d->ui->m_replacement->setText( _word );
    d->ui->m_replaceBtn->setEnabled( false );
    d->ui->m_replaceAllBtn->setEnabled( false );
    d->ui->m_suggestBtn->setEnabled( false );
  } else {
    d->ui->m_replacement->setText( (*sugg)[0] );
    d->ui->m_replaceBtn->setEnabled( true );
    d->ui->m_replaceAllBtn->setEnabled( true );
    d->ui->m_suggestBtn->setEnabled( false );
    d->ui->m_suggestions->setSelected( d->ui->m_suggestions->firstChild(), true );
  }
}

void
KSpellDlg::init( const TQString& _word, TQStringList* _sugg,
                 const TQString& context )
{
  sugg = _sugg;
  word = _word;

  d->ui->m_suggestions->clear();
  d->ui->m_suggestions->setSorting( NONSORTINGCOLUMN );
  for ( TQStringList::Iterator it = _sugg->begin(); it != _sugg->end(); ++it ) {
      TQListViewItem *item = new TQListViewItem( d->ui->m_suggestions,
                                               d->ui->m_suggestions->lastItem() );
      item->setText( 0, *it );
  }

  kdDebug(750) << "KSpellDlg::init [" << word << "]" << endl;

  emit ready( true );

  d->ui->m_unknownWord->setText( _word );
  d->ui->m_contextLabel->setText( context );

  if ( sugg->count() == 0 ) {
    d->ui->m_replacement->setText( _word );
    d->ui->m_replaceBtn->setEnabled( false );
    d->ui->m_replaceAllBtn->setEnabled( false );
    d->ui->m_suggestBtn->setEnabled( false );
  } else {
    d->ui->m_replacement->setText( (*sugg)[0] );
    d->ui->m_replaceBtn->setEnabled( true );
    d->ui->m_replaceAllBtn->setEnabled( true );
    d->ui->m_suggestBtn->setEnabled( false );
    d->ui->m_suggestions->setSelected( d->ui->m_suggestions->firstChild(), true );
  }
}

void
KSpellDlg::slotProgress( unsigned int p )
{
  if (!progressbar)
    return;

  progbar->setValue( (int) p );
}

void
KSpellDlg::textChanged( const TQString & )
{
  d->ui->m_replaceBtn->setEnabled( true );
  d->ui->m_replaceAllBtn->setEnabled( true );
  d->ui->m_suggestBtn->setEnabled( true );
}

void
KSpellDlg::slotSelectionChanged( TQListViewItem* item )
{
  if ( item )
    d->ui->m_replacement->setText( item->text( 0 ) );
}

/*
  exit functions
  */

void
KSpellDlg::closeEvent( TQCloseEvent * )
{
  cancel();
}

void
KSpellDlg::done( int result )
{
  emit command( result );
}
void
KSpellDlg::ignore()
{
  newword = word;
  done( KS_IGNORE );
}

void
KSpellDlg::ignoreAll()
{
  newword = word;
  done( KS_IGNOREALL );
}

void
KSpellDlg::add()
{
  newword = word;
  done( KS_ADD );
}


void
KSpellDlg::cancel()
{
  newword = word;
  done( KS_CANCEL );
}

void
KSpellDlg::replace()
{
  newword = d->ui->m_replacement->text();
  done( KS_REPLACE );
}

void
KSpellDlg::stop()
{
  newword = word;
  done( KS_STOP );
}

void
KSpellDlg::replaceAll()
{
  newword = d->ui->m_replacement->text();
  done( KS_REPLACEALL );
}

void
KSpellDlg::suggest()
{
  newword = d->ui->m_replacement->text();
  done( KS_SUGGEST );
}

void
KSpellDlg::slotConfigChanged()
{
  d->spellConfig->writeGlobalSettings();
  done( KS_CONFIG );
}

#include "kspelldlg.moc"
