/*
 * This file is part of the KDE Libraries
 * Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

// I (espen) prefer that header files are included alphabetically
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqtimer.h>
#include <tqtoolbutton.h>
#include <tqwhatsthis.h>
#include <tqwidget.h>

#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <kaboutkde.h>
#include <kaction.h>
#include <kapplication.h>
#include <kbugreport.h>
#include <kdialogbase.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kstandarddirs.h>

#include "kswitchlanguagedialog.h"

#include "config.h"
#include <qxembed.h>

class KHelpMenuPrivate
{
public:
    KHelpMenuPrivate():mSwitchApplicationLanguage(NULL)
    {
    }
    ~KHelpMenuPrivate()
    {
        delete mSwitchApplicationLanguage;
    }

    const TDEAboutData *mAboutData;
    KSwitchLanguageDialog *mSwitchApplicationLanguage;
};

KHelpMenu::KHelpMenu( TQWidget *parent, const TQString &aboutAppText,
		      bool showWhatsThis )
  : TQObject(parent), mMenu(0), mAboutApp(0), mAboutKDE(0), mBugReport(0),
    d(new KHelpMenuPrivate)
{
  mParent = parent;
  mAboutAppText = aboutAppText;
  mShowWhatsThis = showWhatsThis;
  d->mAboutData = 0;
}

KHelpMenu::KHelpMenu( TQWidget *parent, const TDEAboutData *aboutData,
		      bool showWhatsThis, TDEActionCollection *actions )
  : TQObject(parent), mMenu(0), mAboutApp(0), mAboutKDE(0), mBugReport(0),
    d(new KHelpMenuPrivate)
{
  mParent = parent;
  mShowWhatsThis = showWhatsThis;

  d->mAboutData = aboutData;

  if (!aboutData)
    mAboutAppText = TQString::null;

  if (actions)
  {
    KStdAction::helpContents(this, TQT_SLOT(appHelpActivated()), actions);
    if (showWhatsThis)
      KStdAction::whatsThis(this, TQT_SLOT(contextHelpActivated()), actions);
    KStdAction::reportBug(this, TQT_SLOT(reportBug()), actions);
    KStdAction::aboutApp(this, TQT_SLOT(aboutApplication()), actions);
    KStdAction::aboutKDE(this, TQT_SLOT(aboutKDE()), actions);
    KStdAction::switchApplicationLanguage(this, TQT_SLOT(switchApplicationLanguage()), actions);
  }
}

KHelpMenu::~KHelpMenu()
{
  delete mMenu;
  delete mAboutApp;
  delete mAboutKDE;
  delete mBugReport;
  delete d;
}


TDEPopupMenu* KHelpMenu::menu()
{
  if( !mMenu )
  {
    //
    // 1999-12-02 Espen Sand:
    // I use hardcoded menu id's here. Reason is to stay backward
    // compatible.
    //
    const TDEAboutData *aboutData = d->mAboutData ? d->mAboutData : TDEGlobal::instance()->aboutData();
    TQString appName = (aboutData)? aboutData->programName() : TQString::fromLatin1(tqApp->name());

    mMenu = new TDEPopupMenu();
    connect( mMenu, TQT_SIGNAL(destroyed()), this, TQT_SLOT(menuDestroyed()));

    bool need_separator = false;
    if (kapp->authorizeTDEAction("help_contents"))
    {
      mMenu->insertItem( BarIcon( "contents", KIcon::SizeSmall),
                     TQString(i18n( "%1 &Handbook" ).arg( appName)) ,menuHelpContents );
      mMenu->connectItem( menuHelpContents, this, TQT_SLOT(appHelpActivated()) );
      mMenu->setAccel( TDEStdAccel::shortcut(TDEStdAccel::Help), menuHelpContents );
      need_separator = true;
    }

    if( mShowWhatsThis && kapp->authorizeTDEAction("help_whats_this") )
    {
      TQToolButton* wtb = TQWhatsThis::whatsThisButton(0);
      mMenu->insertItem( wtb->iconSet(),i18n( "What's &This" ), menuWhatsThis);
      mMenu->connectItem( menuWhatsThis, this, TQT_SLOT(contextHelpActivated()) );
      delete wtb;
      mMenu->setAccel( SHIFT + Key_F1, menuWhatsThis );
      need_separator = true;
    }

    if (kapp->authorizeTDEAction("help_report_bug") && aboutData && !aboutData->bugAddress().isEmpty() )
    {
      if (need_separator)
        mMenu->insertSeparator();
      mMenu->insertItem( i18n( "&Report Bug/Request Enhancement..." ), menuReportBug );
      mMenu->connectItem( menuReportBug, this, TQT_SLOT(reportBug()) );
      need_separator = true;
    }

    if (kapp->authorizeTDEAction("switch_application_language"))
    {
      if (need_separator)
        mMenu->insertSeparator();
      mMenu->insertItem( i18n( "Switch application &language..." ), menuSwitchLanguage );
      mMenu->connectItem( menuSwitchLanguage, this, TQT_SLOT(switchApplicationLanguage()) );
      need_separator = true;
    }
    
    if (need_separator)
      mMenu->insertSeparator();

    if (kapp->authorizeTDEAction("help_about_app"))
    {
      mMenu->insertItem( kapp->miniIcon(),
        TQString(i18n( "&About %1" ).arg(appName)), menuAboutApp );
      mMenu->connectItem( menuAboutApp, this, TQT_SLOT( aboutApplication() ) );
    }
    
    if (kapp->authorizeTDEAction("help_about_kde"))
    {
      mMenu->insertItem( SmallIcon("about_kde"), i18n( "About &Trinity" ), menuAboutKDE );
      mMenu->connectItem( menuAboutKDE, this, TQT_SLOT( aboutKDE() ) );
    }
  }

  return mMenu;
}



void KHelpMenu::appHelpActivated()
{
  kapp->invokeHelp();
}


void KHelpMenu::aboutApplication()
{
  if (d->mAboutData)
  {
    if( !mAboutApp )
    {
      mAboutApp = new KAboutApplication( d->mAboutData, mParent, "about", false );
      connect( mAboutApp, TQT_SIGNAL(finished()), this, TQT_SLOT( dialogFinished()) );
    }
    mAboutApp->show();
  }
  else if( mAboutAppText.isEmpty() )
  {
    emit showAboutApplication();
  }
  else
  {
    if( !mAboutApp )
    {
      mAboutApp = new KDialogBase( TQString::null, // Caption is defined below
				   KDialogBase::Yes, KDialogBase::Yes,
				   KDialogBase::Yes, mParent, "about",
				   false, true, KStdGuiItem::ok() );
      connect( mAboutApp, TQT_SIGNAL(finished()), this, TQT_SLOT( dialogFinished()) );

      TQHBox *hbox = new TQHBox( mAboutApp );
      mAboutApp->setMainWidget( hbox );
      hbox->setSpacing(KDialog::spacingHint()*3);
      hbox->setMargin(KDialog::marginHint()*1);

      TQLabel *label1 = new TQLabel(hbox);
      label1->setPixmap( kapp->icon() );
      TQLabel *label2 = new TQLabel(hbox);
      label2->setText( mAboutAppText );

      mAboutApp->setPlainCaption( i18n("About %1").arg(kapp->caption()) );
      mAboutApp->disableResize();
    }

    mAboutApp->show();
  }
}


void KHelpMenu::aboutKDE()
{
  if( !mAboutKDE )
  {
    mAboutKDE = new KAboutKDE( mParent, "aboutkde", false );
    connect( mAboutKDE, TQT_SIGNAL(finished()), this, TQT_SLOT( dialogFinished()) );
  }
  mAboutKDE->show();
}


void KHelpMenu::reportBug()
{
  if( !mBugReport )
  {
    mBugReport = new KBugReport( mParent, false, d->mAboutData );
    connect( mBugReport, TQT_SIGNAL(finished()),this,TQT_SLOT( dialogFinished()) );
  }
  mBugReport->show();
}

void KHelpMenu::switchApplicationLanguage()
{
  if ( !d->mSwitchApplicationLanguage )
  {
    d->mSwitchApplicationLanguage = new KSwitchLanguageDialog( mParent, "switchlanguagedialog", false );
    connect( d->mSwitchApplicationLanguage, TQT_SIGNAL(finished()), this, TQT_SLOT( dialogFinished()) );
  }
  d->mSwitchApplicationLanguage->show();
}


void KHelpMenu::dialogFinished()
{
  TQTimer::singleShot( 0, this, TQT_SLOT(timerExpired()) );
}


void KHelpMenu::timerExpired()
{
  if( mAboutKDE && !mAboutKDE->isVisible() )
  {
    delete mAboutKDE; mAboutKDE = 0;
  }

  if( mBugReport && !mBugReport->isVisible() )
  {
    delete mBugReport; mBugReport = 0;
  }

  if( mAboutApp && !mAboutApp->isVisible() )
  {
    delete mAboutApp; mAboutApp = 0;
  }
  
  if (d->mSwitchApplicationLanguage && !d->mSwitchApplicationLanguage->isVisible())
  {
    delete d->mSwitchApplicationLanguage; d->mSwitchApplicationLanguage = 0;
  }
}


void KHelpMenu::menuDestroyed()
{
  mMenu = 0;
}


void KHelpMenu::contextHelpActivated()
{
  TQWhatsThis::enterWhatsThisMode();
  TQWidget* w = TQApplication::widgetAt( TQCursor::pos(), true );
  while ( w && !w->isTopLevel() && !w->inherits("QXEmbed")  )
      w = w->parentWidget();
#ifdef Q_WS_X11
   if ( w && w->inherits("QXEmbed") )
	  (( QXEmbed*) w )->enterWhatsThisMode();
#endif
}

void KHelpMenu::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


#include "khelpmenu.moc"
