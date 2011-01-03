/*
 * dialog.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "dialog.h"
#include "kspell2ui.h"

#include "backgroundchecker.h"
#include "broker.h"
#include "filter.h"
#include "dictionary.h"
#include "settings.h"

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>

#include <tqlistview.h>
#include <tqpushbutton.h>
#include <tqcombobox.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqtimer.h>
#include <tqdict.h>

namespace KSpell2
{

//to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class Dialog::Private
{
public:
    KSpell2UI *ui;
    TQString   originalBuffer;
    BackgroundChecker *checker;

    Word   currentWord;
    TQMap<TQString, TQString> tqreplaceAllMap;
};

Dialog::Dialog( BackgroundChecker *checker,
                TQWidget *parent, const char *name )
    : KDialogBase( parent, name, true,
                   i18n( "Check Spelling" ),
                   Help|Cancel|User1, Cancel,  true,
                   i18n( "&Finished" ) )
{
    d = new Private;

    d->checker = checker;

    initGui();
    initConnections();
    setMainWidget( d->ui );
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::initConnections()
{
    connect( d->ui->m_addBtn, TQT_SIGNAL(clicked()),
             TQT_SLOT(slotAddWord()) );
    connect( d->ui->m_tqreplaceBtn, TQT_SIGNAL(clicked()),
             TQT_SLOT(slotReplaceWord()) );
    connect( d->ui->m_tqreplaceAllBtn, TQT_SIGNAL(clicked()),
             TQT_SLOT(slotReplaceAll()) );
    connect( d->ui->m_skipBtn, TQT_SIGNAL(clicked()),
             TQT_SLOT(slotSkip()) );
    connect( d->ui->m_skipAllBtn, TQT_SIGNAL(clicked()),
             TQT_SLOT(slotSkipAll()) );
    connect( d->ui->m_suggestBtn, TQT_SIGNAL(clicked()),
             TQT_SLOT(slotSuggest()) );
    connect( d->ui->m_language, TQT_SIGNAL(activated(const TQString&)),
             TQT_SLOT(slotChangeLanguage(const TQString&)) );
    connect( d->ui->m_suggestions, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
             TQT_SLOT(slotSelectionChanged(TQListViewItem*)) );
    connect( d->checker, TQT_SIGNAL(misspelling(const TQString&, int)),
             TQT_SIGNAL(misspelling(const TQString&, int)) );
    connect( d->checker, TQT_SIGNAL(misspelling(const TQString&, int)),
             TQT_SLOT(slotMisspelling(const TQString&, int)) );
    connect( d->checker, TQT_SIGNAL(done()),
             TQT_SLOT(slotDone()) );
    connect( d->ui->m_suggestions, TQT_SIGNAL(doubleClicked(TQListViewItem*, const TQPoint&, int)),
             TQT_SLOT( slotReplaceWord() ) );
    connect( this, TQT_SIGNAL(user1Clicked()), this, TQT_SLOT(slotFinished()) );
    connect( this, TQT_SIGNAL(cancelClicked()),this, TQT_SLOT(slotCancel()) );
    connect( d->ui->m_tqreplacement, TQT_SIGNAL(returnPressed()), this, TQT_SLOT(slotReplaceWord()) );
    connect( d->ui->m_autoCorrect, TQT_SIGNAL(clicked()),
             TQT_SLOT(slotAutocorrect()) );
    // button use by kword/kpresenter
    // hide by default
    d->ui->m_autoCorrect->hide();
}

void Dialog::initGui()
{
    d->ui = new KSpell2UI( this );
    d->ui->m_suggestions->setSorting( NONSORTINGCOLUMN );
    d->ui->m_language->clear();
    d->ui->m_language->insertStringList( d->checker->broker()->languages() );
    for ( int i = 0; !d->ui->m_language->text( i ).isNull(); ++i ) {
        TQString ct = d->ui->m_language->text( i );
        if ( ct == d->checker->broker()->settings()->defaultLanguage() ) {
            d->ui->m_language->setCurrentItem( i );
            break;
        }
    }
}

void Dialog::activeAutoCorrect( bool _active )
{
    if ( _active )
        d->ui->m_autoCorrect->show();
    else
        d->ui->m_autoCorrect->hide();
}

void Dialog::slotAutocorrect()
{
    kdDebug()<<"void Dialog::slotAutocorrect()\n";
    emit autoCorrect(d->currentWord.word, d->ui->m_tqreplacement->text() );
    slotReplaceWord();
}

void Dialog::slotFinished()
{
    kdDebug()<<"void Dialog::slotFinished() \n";
    emit stop();
    //FIXME: should we emit done here?
    emit done( d->checker->filter()->buffer() );
    accept();
}

void Dialog::slotCancel()
{
    kdDebug()<<"void Dialog::slotCancel() \n";
    emit cancel();
    reject();
}

TQString Dialog::originalBuffer() const
{
    return d->originalBuffer;
}

TQString Dialog::buffer() const
{
    return d->checker->filter()->buffer();
}

void Dialog::setBuffer( const TQString& buf )
{
    d->originalBuffer = buf;
}

void Dialog::setFilter( Filter *filter )
{
    filter->setBuffer( d->checker->filter()->buffer() );
    d->checker->setFilter( filter );
}

void Dialog::updateDialog( const TQString& word )
{
    d->ui->m_unknownWord->setText( word );
    d->ui->m_contextLabel->setText( d->checker->filter()->context() );
    TQStringList suggs = d->checker->suggest( word );
    d->ui->m_tqreplacement->setText( suggs.first() );
    fillSuggestions( suggs );
}

void Dialog::show()
{
    kdDebug()<<"Showing dialog"<<endl;
    if ( d->originalBuffer.isEmpty() )
        d->checker->start();
    else
        d->checker->checkText( d->originalBuffer );
}

void Dialog::slotAddWord()
{
   d->checker->addWord( d->currentWord.word ); 
   d->checker->continueChecking();
}

void Dialog::slotReplaceWord()
{
    emit tqreplace( d->currentWord.word, d->currentWord.start,
                  d->ui->m_tqreplacement->text() );
    d->checker->filter()->tqreplace( d->currentWord, d->ui->m_tqreplacement->text() );
    d->checker->continueChecking();
}

void Dialog::slotReplaceAll()
{
    d->tqreplaceAllMap.insert( d->currentWord.word,
                             d->ui->m_tqreplacement->text() );
    slotReplaceWord();
}

void Dialog::slotSkip()
{
    d->checker->continueChecking();
}

void Dialog::slotSkipAll()
{
    //### do we want that or should we have a d->ignoreAll list?
    d->checker->broker()->settings()->addWordToIgnore( d->ui->m_tqreplacement->text() );
    d->checker->continueChecking();
}

void Dialog::slotSuggest()
{
    TQStringList suggs = d->checker->suggest( d->ui->m_tqreplacement->text() );
    fillSuggestions( suggs );
}

void Dialog::slotChangeLanguage( const TQString& lang )
{
    d->checker->changeLanguage( lang );
    slotSuggest();
}

void Dialog::slotSelectionChanged( TQListViewItem *item )
{
    d->ui->m_tqreplacement->setText( item->text( 0 ) );
}

void Dialog::fillSuggestions( const TQStringList& suggs )
{
    d->ui->m_suggestions->clear();
    for ( TQStringList::ConstIterator it = suggs.begin(); it != suggs.end(); ++it ) {
        new TQListViewItem( d->ui->m_suggestions, d->ui->m_suggestions->firstChild(),
                           *it );
    }
}

void Dialog::slotMisspelling(const TQString& word, int start )
{
    kdDebug()<<"Dialog misspelling!!"<<endl;
    d->currentWord = Word( word, start );
    if ( d->tqreplaceAllMap.tqcontains( word ) ) {
        d->ui->m_tqreplacement->setText( d->tqreplaceAllMap[ word ] );
        slotReplaceWord();
    } else {
        updateDialog( word );
    }
    KDialogBase::show();
}

void Dialog::slotDone()
{
    kdDebug()<<"Dialog done!"<<endl;
    emit done( d->checker->filter()->buffer() );
    accept();
}

}

#include "dialog.moc"
