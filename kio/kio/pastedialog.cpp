/* This file is part of the KDE libraries
   Copyright (C) 2005 David Faure <faure@kde.org>

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

#include "pastedialog.h"

#include <klineedit.h>
#include <kmimetype.h>
#include <klocale.h>

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqapplication.h>
#include <tqclipboard.h>

KIO::PasteDialog::PasteDialog( const TQString &caption, const TQString &label,
                               const TQString &value, const TQStringList& items,
                               TQWidget *parent,
                               bool clipboard )
    : KDialogBase( parent, 0 /*name*/, true, caption, Ok|Cancel, Ok, true )
{
    TQFrame *frame = makeMainWidget();
    TQVBoxLayout *tqlayout = new TQVBoxLayout( frame, 0, spacingHint() );

    m_label = new TQLabel( label, frame );
    tqlayout->addWidget( m_label );

    m_lineEdit = new KLineEdit( value, frame );
    tqlayout->addWidget( m_lineEdit );

    m_lineEdit->setFocus();
    m_label->setBuddy( m_lineEdit );

    tqlayout->addWidget( new TQLabel( i18n( "Data format:" ), frame ) );
    m_comboBox = new TQComboBox( frame );
    m_comboBox->insertStringList( items );
    tqlayout->addWidget( m_comboBox );

    tqlayout->addStretch();

    //connect( m_lineEdit, TQT_SIGNAL( textChanged( const TQString & ) ),
    //    TQT_SLOT( slotEditTextChanged( const TQString & ) ) );
    //connect( this, TQT_SIGNAL( user1Clicked() ), m_lineEdit, TQT_SLOT( clear() ) );

    //slotEditTextChanged( value );
    setMinimumWidth( 350 );

    m_clipboardChanged = false;
    if ( clipboard )
        connect( TQApplication::clipboard(), TQT_SIGNAL( dataChanged() ),
                 this, TQT_SLOT( slotClipboardDataChanged() ) );
}

void KIO::PasteDialog::slotClipboardDataChanged()
{
    m_clipboardChanged = true;
}

TQString KIO::PasteDialog::lineEditText() const
{
    return m_lineEdit->text();
}

int KIO::PasteDialog::comboItem() const
{
    return m_comboBox->currentItem();
}

#include "pastedialog.moc"
