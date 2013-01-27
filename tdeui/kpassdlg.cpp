// vi: ts=8 sts=4 sw=4
/* This file is part of the KDE libraries
   Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
   Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
   Copyright (C) 2004,2005 Andrew Coles <andrew_coles@yahoo.co.uk>

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
#include <unistd.h>

#include <tqwidget.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqsize.h>
#include <tqevent.h>
#include <tqkeycode.h>
#include <tqcheckbox.h>
#include <tqregexp.h>
#include <tqhbox.h>
#include <tqwhatsthis.h>
#include <tqptrdict.h>
#include <tqtimer.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kaboutdialog.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <kprogress.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "kpassdlg.h"

#include "../tdesu/defaults.h"

/*
 * Password line editor.
 */

// BCI: Add a real d-pointer and put the int into that

static TQPtrDict<int>* d_ptr = 0;

static void cleanup_d_ptr() {
	delete d_ptr;
}

static int * ourMaxLength( const KPasswordEdit* const e ) {
	if ( !d_ptr ) {
		d_ptr = new TQPtrDict<int>;
		d_ptr->setAutoDelete(true);
		tqAddPostRoutine( cleanup_d_ptr );
	}
	int* ret = d_ptr->find( (void*) e );
	if ( ! ret ) {
		ret = new int;
		d_ptr->replace( (void*) e, ret );
	}
	return ret;
}

static void delete_d( const KPasswordEdit* const e ) {
	if ( d_ptr )
		d_ptr->remove( (void*) e );
}

const int KPasswordEdit::PassLen = 200;

class KPasswordDialog::KPasswordDialogPrivate
{
    public:
	KPasswordDialogPrivate()
	 : m_MatchLabel( 0 ), iconName( 0 ), allowEmptyPasswords( false ),
	   minimumPasswordLength(0), maximumPasswordLength(KPasswordEdit::PassLen - 1),
	   passwordStrengthWarningLevel(1), m_strengthBar(0),
	   reasonablePasswordLength(8)
	    {}
	TQLabel *m_MatchLabel;
	TQString iconName;
	bool allowEmptyPasswords;
	int minimumPasswordLength;
	int maximumPasswordLength;
	int passwordStrengthWarningLevel;
	KProgress* m_strengthBar;
	int reasonablePasswordLength;
};


KPasswordEdit::KPasswordEdit(TQWidget *parent, const char *name)
    : TQLineEdit(parent, name)
{
    init();

    TDEConfig* const cfg = TDEGlobal::config();
    TDEConfigGroupSaver saver(cfg, "Passwords");

    const TQString val = cfg->readEntry("EchoMode", "OneStar");
    if (val == "ThreeStars")
	m_EchoMode = ThreeStars;
    else if (val == "NoEcho")
	m_EchoMode = NoEcho;
    else
	m_EchoMode = OneStar;

    setInputMethodEnabled( true );
}

KPasswordEdit::KPasswordEdit(TQWidget *parent, const char *name, int echoMode)
    : TQLineEdit(parent, name), m_EchoMode(echoMode)
{
    init();
}

KPasswordEdit::KPasswordEdit(EchoModes echoMode, TQWidget *parent, const char *name)
    : TQLineEdit(parent, name), m_EchoMode(echoMode)
{
    init();
}

KPasswordEdit::KPasswordEdit(EchoMode echoMode, TQWidget *parent, const char *name)
    : TQLineEdit(parent, name)
    , m_EchoMode( echoMode == TQLineEdit::NoEcho ? NoEcho : OneStar )
{
    init();
}

void KPasswordEdit::init()
{
    setEchoMode(TQLineEdit::Password); // Just in case
    setAcceptDrops(false);
    int* t = ourMaxLength(this);
    *t = (PassLen - 1); // the internal max length
    m_Password = new char[PassLen];
    m_Password[0] = '\000';
    m_Length = 0;
}

KPasswordEdit::~KPasswordEdit()
{
    memset(m_Password, 0, PassLen * sizeof(char));
    delete[] m_Password;
    delete_d(this);
}

void KPasswordEdit::insert(const TQString &txt)
{
    const TQCString localTxt = txt.local8Bit();
    const unsigned int lim = localTxt.length();
    const int m_MaxLength = maxPasswordLength();
    for(unsigned int i=0; i < lim; ++i)
    {
        const unsigned char ke = localTxt[i];
        if (m_Length < m_MaxLength)
        {
            m_Password[m_Length] = ke;
            m_Password[++m_Length] = '\000';
        }
    }
    showPass();
}

void KPasswordEdit::erase()
{
    m_Length = 0;
    memset(m_Password, 0, PassLen * sizeof(char));
    setText("");
}

void KPasswordEdit::focusInEvent(TQFocusEvent *e)
{
    const TQString txt = text();
    setUpdatesEnabled(false);
    TQLineEdit::focusInEvent(e);
    setUpdatesEnabled(true);
    setText(txt);
}


void KPasswordEdit::keyPressEvent(TQKeyEvent *e)
{
    switch (e->key()) {
    case Key_Return:
    case Key_Enter:
    case Key_Escape:
	e->ignore();
	break;
    case Key_Backspace:
    case Key_Delete:
    case 0x7f: // Delete
	if (e->state() & (ControlButton | AltButton))
	    e->ignore();
	else if (m_Length) {
	    m_Password[--m_Length] = '\000';
	    showPass();
	}
	break;
    default:
	const unsigned char ke = TQString(e->text()).local8Bit()[0];
	if (ke >= 32) {
	    insert(e->text());
	} else
	    e->ignore();
	break;
    }
}

bool KPasswordEdit::event(TQEvent *e) {
    switch(e->type()) {

      case TQEvent::MouseButtonPress:
      case TQEvent::MouseButtonRelease:
      case TQEvent::MouseButtonDblClick:
      case TQEvent::MouseMove:
      case TQEvent::IMStart:
      case TQEvent::IMCompose:
        return true; //Ignore

      case TQEvent::IMEnd:
      {
        TQIMEvent* const ie = (TQIMEvent*) e;
        if (!ie->text().isEmpty())
          insert( ie->text() );
        return true;
      }

      case TQEvent::AccelOverride:
      {
        TQKeyEvent* const k = (TQKeyEvent*) e;
        switch (k->key()) {
            case Key_U:
                if (k->state() & ControlButton) {
                    m_Length = 0;
                    m_Password[m_Length] = '\000';
                    showPass();
                }
        }
        return true; // stop bubbling
      }

      default:
        // Do nothing
        break;
    }
    return TQLineEdit::event(e);
}

void KPasswordEdit::showPass()
{
    TQString tmp;

    switch (m_EchoMode) {
    case OneStar:
	tmp.fill('*', m_Length);
	setText(tmp);
	break;
    case ThreeStars:
	tmp.fill('*', m_Length*3);
	setText(tmp);
	break;
    case NoEcho: default:
	emit textChanged(TQString::null); //To update the password comparison if need be.
	break;
    }
}

void KPasswordEdit::setMaxPasswordLength(int newLength)
{
    if (newLength >= PassLen) newLength = PassLen - 1; // belt and braces
    if (newLength < 0) newLength = 0;
    int* t = ourMaxLength(this);
    *t = newLength; 
    while (m_Length > newLength) {
        m_Password[m_Length] = '\000';
        --m_Length;
    }
    showPass();
}

int KPasswordEdit::maxPasswordLength() const
{
    return *(ourMaxLength(this));
}
/*
 * Password dialog.
 */

KPasswordDialog::KPasswordDialog(Types type, bool enableKeep, int extraBttn,
                                 TQWidget *parent, const char *name)
    : KDialogBase(parent, name, true, "", Ok|Cancel|extraBttn,
                  Ok, true), m_Keep(enableKeep? 1 : 0), m_keepWarnLbl(0), m_Type(type), d(new KPasswordDialogPrivate)
{
    d->iconName = "password";
    init();
}

KPasswordDialog::KPasswordDialog(Types type, bool enableKeep, int extraBttn, const TQString& icon,
				  TQWidget *parent, const char *name )
    : KDialogBase(parent, name, true, "", Ok|Cancel|extraBttn,
                  Ok, true), m_Keep(enableKeep? 1 : 0), m_keepWarnLbl(0), m_Type(type), d(new KPasswordDialogPrivate)
{
    if ( icon.stripWhiteSpace().isEmpty() )
	d->iconName = "password";
    else
	d->iconName = icon;
    init();
}

KPasswordDialog::KPasswordDialog(int type, TQString prompt, bool enableKeep,
                                 int extraBttn)
    : KDialogBase(0L, "Password Dialog", true, "", Ok|Cancel|extraBttn,
                  Ok, true), m_Keep(enableKeep? 1 : 0), m_keepWarnLbl(0), m_Type(type), d(new KPasswordDialogPrivate)
{
    d->iconName = "password";
    init();
    setPrompt(prompt);
}

void KPasswordDialog::init()
{
    m_Row = 0;

    TDEConfig* const cfg = TDEGlobal::config();
    const TDEConfigGroupSaver saver(cfg, "Passwords");
    bool def = ( qstrcmp( tqAppName(), "tdesu" ) == 0 ? defKeep : false );
    if (m_Keep && cfg->readBoolEntry("Keep", def))
	++m_Keep;

    m_pMain = new TQWidget(this);
    setMainWidget(m_pMain);
    m_pGrid = new TQGridLayout(m_pMain, 10, 3, 0, 0);
    m_pGrid->addColSpacing(1, 10);

    // Row 1: pixmap + prompt
    TQLabel *lbl;
    const TQPixmap pix( TDEGlobal::iconLoader()->loadIcon( d->iconName, KIcon::NoGroup, KIcon::SizeHuge, 0, 0, true));
    if (!pix.isNull()) {
	lbl = new TQLabel(m_pMain);
	lbl->setPixmap(pix);
	lbl->setAlignment(AlignHCenter|AlignVCenter);
	lbl->setFixedSize(lbl->sizeHint());
	m_pGrid->addWidget(lbl, 0, 0, (TQ_Alignment)AlignCenter);
    }

    m_pHelpLbl = new TQLabel(m_pMain);
    m_pHelpLbl->setAlignment(AlignLeft|AlignVCenter|WordBreak);
    m_pGrid->addWidget(m_pHelpLbl, 0, 2, (TQ_Alignment)AlignLeft);
    m_pGrid->addRowSpacing(1, 10);
    m_pGrid->setRowStretch(1, 12);

    // Row 2+: space for 4 extra info lines
    m_pGrid->addRowSpacing(6, 5);
    m_pGrid->setRowStretch(6, 12);

    // Row 3: Password editor #1
    lbl = new TQLabel(m_pMain);
    lbl->setAlignment(AlignLeft|AlignVCenter);
    lbl->setText(i18n("&Password:"));
    lbl->setFixedSize(lbl->sizeHint());
    m_pGrid->addWidget(lbl, 7, 0, (TQ_Alignment)AlignLeft);

    TQHBoxLayout *h_lay = new TQHBoxLayout();
    m_pGrid->addLayout(h_lay, 7, 2);
    m_pEdit = new KPasswordEdit(m_pMain);
    m_pEdit2 = 0;
    lbl->setBuddy(m_pEdit);
    TQSize size = m_pEdit->sizeHint();
    m_pEdit->setFixedHeight(size.height());
    m_pEdit->setMinimumWidth(size.width());
    h_lay->addWidget(m_pEdit);

    // Row 4: Password editor #2 or keep password checkbox

    if ((m_Type == Password) && m_Keep) {
	m_pGrid->addRowSpacing(8, 10);
	m_pGrid->setRowStretch(8, 12);
	TQCheckBox* const cb = new TQCheckBox(i18n("&Keep password"), m_pMain);
	cb->setFixedSize(cb->sizeHint());
	m_keepWarnLbl = new TQLabel(m_pMain);
	m_keepWarnLbl->setAlignment(AlignLeft|AlignVCenter|WordBreak);
	if (m_Keep > 1) {
	    cb->setChecked(true);
	    m_keepWarnLbl->show();
	}
	else {
	    m_Keep = 0;
	    m_keepWarnLbl->hide();
	}
	connect(cb, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotKeep(bool)));
	m_pGrid->addWidget(cb, 9, 2, (TQ_Alignment)(AlignLeft|AlignVCenter));
//	m_pGrid->addWidget(m_keepWarnLbl, 13, 2, (TQ_Alignment)(AlignLeft|AlignVCenter));
	m_pGrid->addMultiCellWidget(m_keepWarnLbl, 13, 13, 0, 3);
    } else if (m_Type == NewPassword) {
	m_pGrid->addRowSpacing(8, 10);
	lbl = new TQLabel(m_pMain);
	lbl->setAlignment(AlignLeft|AlignVCenter);
	lbl->setText(i18n("&Verify:"));
	lbl->setFixedSize(lbl->sizeHint());
	m_pGrid->addWidget(lbl, 9, 0, (TQ_Alignment)AlignLeft);

	h_lay = new TQHBoxLayout();
	m_pGrid->addLayout(h_lay, 9, 2);
	m_pEdit2 = new KPasswordEdit(m_pMain);
	lbl->setBuddy(m_pEdit2);
	size = m_pEdit2->sizeHint();
	m_pEdit2->setFixedHeight(size.height());
	m_pEdit2->setMinimumWidth(size.width());
	h_lay->addWidget(m_pEdit2);

        // Row 6: Password strength meter
        m_pGrid->addRowSpacing(10, 10);
        m_pGrid->setRowStretch(10, 12);

        TQHBox* const strengthBox = new TQHBox(m_pMain);
        strengthBox->setSpacing(10);
        m_pGrid->addMultiCellWidget(strengthBox, 11, 11, 0, 2);
        TQLabel* const passStrengthLabel = new TQLabel(strengthBox);
        passStrengthLabel->setAlignment(AlignLeft|AlignVCenter);
        passStrengthLabel->setText(i18n("Password strength meter:"));
        d->m_strengthBar = new KProgress(100, strengthBox, "PasswordStrengthMeter");
        d->m_strengthBar->setPercentageVisible(false);

        const TQString strengthBarWhatsThis(i18n("The password strength meter gives an indication of the security "
                                                "of the password you have entered.  To improve the strength of "
                                                "the password, try:\n"
                                                " - using a longer password;\n"
                                                " - using a mixture of upper- and lower-case letters;\n"
                                                " - using numbers or symbols, such as #, as well as letters."));
        TQWhatsThis::add(passStrengthLabel, strengthBarWhatsThis);
        TQWhatsThis::add(d->m_strengthBar, strengthBarWhatsThis);

        // Row 6: Label saying whether the passwords match
        m_pGrid->addRowSpacing(12, 10);
        m_pGrid->setRowStretch(12, 12);

        d->m_MatchLabel = new TQLabel(m_pMain);
        d->m_MatchLabel->setAlignment(AlignLeft|AlignVCenter|WordBreak);
        m_pGrid->addMultiCellWidget(d->m_MatchLabel, 13, 13, 0, 2);
        d->m_MatchLabel->setText(i18n("Passwords do not match"));


        connect( m_pEdit, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(enableOkBtn()) );
        connect( m_pEdit2, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(enableOkBtn()) );
        enableOkBtn();
    }

    erase();
}


KPasswordDialog::~KPasswordDialog()
{
	delete d;
}


void KPasswordDialog::clearPassword()
{
    m_pEdit->erase();
}

/* KDE 4: Make it const TQString & */
void KPasswordDialog::setPrompt(TQString prompt)
{
    m_pHelpLbl->setText(prompt);
    m_pHelpLbl->setFixedSize(275, m_pHelpLbl->heightForWidth(275));
}

void KPasswordDialog::setKeepWarning(TQString warn)
{
    if (m_keepWarnLbl) {
        m_keepWarnLbl->setText(warn);
    }
}


TQString KPasswordDialog::prompt() const

{
    return m_pHelpLbl->text();
}


/* KDE 4: Make them const TQString & */
void KPasswordDialog::addLine(TQString key, TQString value)
{
    if (m_Row > 3)
	return;

    TQLabel *lbl = new TQLabel(key, m_pMain);
    lbl->setAlignment(AlignLeft|AlignTop);
    lbl->setFixedSize(lbl->sizeHint());
    m_pGrid->addWidget(lbl, m_Row+2, 0, (TQ_Alignment)AlignLeft);

    lbl = new TQLabel(value, m_pMain);
    lbl->setAlignment(AlignTop|WordBreak);
    lbl->setFixedSize(275, lbl->heightForWidth(275));
    m_pGrid->addWidget(lbl, m_Row+2, 2, (TQ_Alignment)AlignLeft);
    ++m_Row;
}


void KPasswordDialog::erase()
{
    m_pEdit->erase();
    m_pEdit->setFocus();
    if (m_Type == NewPassword)
	m_pEdit2->erase();
}


void KPasswordDialog::slotOk()
{
    if (m_Type == NewPassword) {
	if (strcmp(m_pEdit->password(), m_pEdit2->password())) {
	    KMessageBox::sorry(this, i18n("You entered two different "
		    "passwords. Please try again."));
	    erase();
	    return;
	}
	if (d->m_strengthBar && d->m_strengthBar->progress() < d->passwordStrengthWarningLevel) {
	    int retVal = KMessageBox::warningContinueCancel(this,
		i18n(   "The password you have entered has a low strength. "
			"To improve the strength of "
			"the password, try:\n"
			" - using a longer password;\n"
			" - using a mixture of upper- and lower-case letters;\n"
			" - using numbers or symbols as well as letters.\n"
			"\n"
			"Would you like to use this password anyway?"),
		i18n("Low Password Strength"));
	    if (retVal == KMessageBox::Cancel) return;
	}
    }
    if (!checkPassword(m_pEdit->password())) {
	erase();
	return;
    }
    accept();
}


void KPasswordDialog::slotCancel()
{
    reject();
}


void KPasswordDialog::slotKeep(bool keep)
{
    if (m_keepWarnLbl->text() != "") {
        if (keep) {
            m_keepWarnLbl->show();
        }
        else {
            m_keepWarnLbl->hide();
        }
        TQTimer::singleShot(0, this, SLOT(slotLayout()));
    }

    m_Keep = keep;
}

void KPasswordDialog::slotLayout()
{
    resize(sizeHint());
}


// static . antlarr: KDE 4: Make it const TQString & prompt
int KPasswordDialog::getPassword(TQCString &password, TQString prompt,
	int *keep)
{
    const bool enableKeep = (keep && *keep);
    KPasswordDialog* const dlg = new KPasswordDialog(int(Password), prompt, enableKeep);
    const int ret = dlg->exec();
    if (ret == Accepted) {
	password = dlg->password();
	if (enableKeep)
	    *keep = dlg->keep();
    }
    delete dlg;
    return ret;
}


// static . antlarr: KDE 4: Make it const TQString & prompt
int KPasswordDialog::getNewPassword(TQCString &password, TQString prompt)
{
    KPasswordDialog* const dlg = new KPasswordDialog(NewPassword, prompt);
    const int ret = dlg->exec();
    if (ret == Accepted)
	password = dlg->password();
    delete dlg;
    return ret;
}


// static
void KPasswordDialog::disableCoreDumps()
{
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rlim);
}

void KPasswordDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KPasswordDialog::enableOkBtn()
{
    if (m_Type == NewPassword) {
      const bool match = strcmp(m_pEdit->password(), m_pEdit2->password()) == 0
                   && (d->allowEmptyPasswords || m_pEdit->password()[0]);

      const TQString pass(m_pEdit->password());

      const int minPasswordLength = minimumPasswordLength();

      if ((int) pass.length() < minPasswordLength) {
          enableButtonOK(false);
      } else {
          enableButtonOK( match );
      }

      if ( match && d->allowEmptyPasswords && m_pEdit->password()[0] == 0 ) {
          d->m_MatchLabel->setText( i18n("Password is empty") );
      } else {
          if ((int) pass.length() < minPasswordLength) {
              d->m_MatchLabel->setText(i18n("Password must be at least 1 character long", "Password must be at least %n characters long", minPasswordLength));
          } else {
              d->m_MatchLabel->setText( match? i18n("Passwords match")
                                              :i18n("Passwords do not match") );
          }
      }

      // Password strength calculator
      // Based on code in the Master Password dialog in Firefox
      // (pref-masterpass.js)
      // Original code triple-licensed under the MPL, GPL, and LGPL
      // so is license-compatible with this file

      const double lengthFactor = d->reasonablePasswordLength / 8.0;

      
      int pwlength = (int) (pass.length() / lengthFactor);
      if (pwlength > 5) pwlength = 5;

      const TQRegExp numRxp("[0-9]", true, false);
      int numeric = (int) (pass.contains(numRxp) / lengthFactor);
      if (numeric > 3) numeric = 3;

      const TQRegExp symbRxp("\\W", false, false);
      int numsymbols = (int) (pass.contains(symbRxp) / lengthFactor);
      if (numsymbols > 3) numsymbols = 3;

      const TQRegExp upperRxp("[A-Z]", true, false);
      int upper = (int) (pass.contains(upperRxp) / lengthFactor);
      if (upper > 3) upper = 3;

      int pwstrength=((pwlength*10)-20) + (numeric*10) + (numsymbols*15) + (upper*10);

      if ( pwstrength < 0 ) {
	      pwstrength = 0;
      }
  
      if ( pwstrength > 100 ) {
	      pwstrength = 100;
      }
      d->m_strengthBar->setProgress(pwstrength);

   }
}


void KPasswordDialog::setAllowEmptyPasswords(bool allowed) {
    d->allowEmptyPasswords = allowed;
    enableOkBtn();
}


bool KPasswordDialog::allowEmptyPasswords() const {
    return d->allowEmptyPasswords;
}

void KPasswordDialog::setMinimumPasswordLength(int minLength) {
    d->minimumPasswordLength = minLength;
    enableOkBtn();
}

int KPasswordDialog::minimumPasswordLength() const {
    return d->minimumPasswordLength;
}

void KPasswordDialog::setMaximumPasswordLength(int maxLength) {

    if (maxLength < 0) maxLength = 0;
    if (maxLength >= KPasswordEdit::PassLen) maxLength = KPasswordEdit::PassLen - 1;

    d->maximumPasswordLength = maxLength;

    m_pEdit->setMaxPasswordLength(maxLength);
    if (m_pEdit2) m_pEdit2->setMaxPasswordLength(maxLength);

}

int KPasswordDialog::maximumPasswordLength() const {
    return d->maximumPasswordLength;
}

// reasonable password length code contributed by Steffen Mthing

void KPasswordDialog::setReasonablePasswordLength(int reasonableLength) {

    if (reasonableLength < 1) reasonableLength = 1;
    if (reasonableLength >= maximumPasswordLength()) reasonableLength = maximumPasswordLength();

    d->reasonablePasswordLength = reasonableLength;

}

int KPasswordDialog::reasonablePasswordLength() const {
  return d->reasonablePasswordLength;
}


void KPasswordDialog::setPasswordStrengthWarningLevel(int warningLevel) {
    if (warningLevel < 0) warningLevel = 0;
    if (warningLevel > 99) warningLevel = 99;
    d->passwordStrengthWarningLevel = warningLevel;
}

int KPasswordDialog::passwordStrengthWarningLevel() const {
    return d->passwordStrengthWarningLevel;
}

#include "kpassdlg.moc"
