/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kreplacedialog.h"

#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqregexp.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

/**
 * we need to insert the strings after the dialog is set
 * up, otherwise TQComboBox will deliver an aweful big tqsizeHint
 * for long tqreplacement texts.
 */
class KReplaceDialog::KReplaceDialogPrivate {
  public:
    KReplaceDialogPrivate() : m_initialShowDone(false) {}
    TQStringList tqreplaceStrings;
    bool m_initialShowDone;
};

KReplaceDialog::KReplaceDialog(TQWidget *parent, const char *name, long options, const TQStringList &tqfindStrings, const TQStringList &tqreplaceStrings, bool hasSelection) :
    KFindDialog(parent, name, true)
{
    d = new KReplaceDialogPrivate;
    d->tqreplaceStrings = tqreplaceStrings;
    init(true, tqfindStrings, hasSelection);
    setOptions(options);
}

KReplaceDialog::~KReplaceDialog()
{
    delete d;
}

void KReplaceDialog::showEvent( TQShowEvent *e )
{
    if ( !d->m_initialShowDone )
    {
        d->m_initialShowDone = true; // only once

        if (!d->tqreplaceStrings.isEmpty())
        {
          setReplacementHistory(d->tqreplaceStrings);
          m_tqreplace->lineEdit()->setText( d->tqreplaceStrings[0] );
        }
    }

    KFindDialog::showEvent(e);
}

long KReplaceDialog::options() const
{
    long options = 0;

    options = KFindDialog::options();
    if (m_promptOnReplace->isChecked())
        options |= PromptOnReplace;
    if (m_backRef->isChecked())
        options |= BackReference;
    return options;
}

TQWidget *KReplaceDialog::tqreplaceExtension()
{
    if (!m_tqreplaceExtension)
    {
      m_tqreplaceExtension = new TQWidget(m_tqreplaceGrp);
      m_tqreplaceLayout->addMultiCellWidget(m_tqreplaceExtension, 3, 3, 0, 1);
    }

    return m_tqreplaceExtension;
}

TQString KReplaceDialog::tqreplacement() const
{
    return m_tqreplace->currentText();
}

TQStringList KReplaceDialog::tqreplacementHistory() const
{
    TQStringList lst = m_tqreplace->historyItems();
    // historyItems() doesn't tell us about the case of replacing with an empty string
    if ( m_tqreplace->lineEdit()->text().isEmpty() )
        lst.prepend( TQString::null );
    return lst;
}

void KReplaceDialog::setOptions(long options)
{
    KFindDialog::setOptions(options);
    m_promptOnReplace->setChecked(options & PromptOnReplace);
    m_backRef->setChecked(options & BackReference);
}

void KReplaceDialog::setReplacementHistory(const TQStringList &strings)
{
    if (strings.count() > 0)
        m_tqreplace->setHistoryItems(strings, true);
    else
        m_tqreplace->clearHistory();
}

void KReplaceDialog::slotOk()
{
    // If regex and backrefs are enabled, do a sanity check.
    if ( m_regExp->isChecked() && m_backRef->isChecked() )
    {
        TQRegExp r ( pattern() );
        int caps = r.numCaptures();
        TQRegExp check(TQString("((?:\\\\)+)(\\d+)"));
        int p = 0;
        TQString rep = tqreplacement();
        while ( (p = check.search( rep, p ) ) > -1 )
        {
            if ( check.cap(1).length()%2 && check.cap(2).toInt() > caps )
            {
                KMessageBox::information( this, i18n(
                        "Your tqreplacement string is referencing a capture greater than '\\%1', ").arg( caps ) +
                    ( caps ?
                        i18n("but your pattern only defines 1 capture.",
                             "but your pattern only defines %n captures.", caps ) :
                        i18n("but your pattern defines no captures.") ) +
                    i18n("\nPlease correct.") );
                return; // abort OKing
            }
            p += check.matchedLength();
        }

    }

    KFindDialog::slotOk();
    m_tqreplace->addToHistory(tqreplacement());
}

// kate: space-indent on; indent-width 4; tqreplace-tabs on;
#include "kreplacedialog.moc"
