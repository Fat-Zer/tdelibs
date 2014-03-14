/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
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
#include <tdeabouttde.h>
#include <tdeapplication.h>
#include <tdelocale.h>
#include <kstandarddirs.h>


TDEAboutKDE::TDEAboutKDE( TQWidget *parent, const char *name, bool modal )
  :TDEAboutDialog( TDEAboutDialog::AbtKDEStandard, TQString::fromLatin1("TDE"),
		 KDialogBase::Help|KDialogBase::Close, KDialogBase::Close,
		 parent, name, modal )
{
  const TQString text1 = i18n(""
    "The <b>Trinity Desktop Environment</b> was born as a fork of the "
    "K Desktop Environment version 3.5, which was originally written by the KDE Team, "
    "a world-wide network of software engineers committed to <a "
    "href=\"http://www.gnu.org/philosophy/free-sw.html\">Free Software</a> "
    "development. The name <i>Trinity</i> was chosen because the word means "
    "<i>Three</i> as in <i>continuation of KDE 3</i>.<br>Since then, TDE has evolved to be "
    "an indipendent and standalone computer desktop environment project. The developers "
    "have molded the code to its own identity without giving up on the efficiency, "
    "productivity and traditional user interface experience characteristic of the "
    "original KDE 3 series.<br><br>No single group, company or organization controls the "
    "Trinity source code. Everyone is welcome to contribute to Trinity.<br><br>Visit <A "
    "HREF=\"http://www.trinitydesktop.org\">http://www.trinitydesktop.org</A> for more information "
    "about Trinity, and <A HREF=\"http://www.kde.org\">http://www.kde.org</A> "
    "for more information on the KDE project. ");

  const TQString text2 = i18n(""
    "Software can always be improved, and the Trinity Team is ready to "
    "do so. However, you - the user - must tell us when "
    "something does not work as expected or could be done better.<br><br>"
    "The Trinity Desktop Environment has a bug tracking system. Visit "
    "<A HREF=\"http://bugs.pearsoncomputing.net\">http://bugs.pearsoncomputing.net</A> or "
    "use the \"Report Bug...\" dialog from the \"Help\" menu to report bugs.<br><br>"
    "If you have a suggestion for improvement then you are welcome to use "
    "the bug tracking system to register your wish. Make sure you use the "
    "severity called \"Wishlist\"." );

  const TQString text3 = i18n(""
    "You do not have to be a software developer to be a member of the "
    "Trinity team. You can join the national teams that translate "
    "program interfaces. You can provide graphics, themes, sounds, and "
    "improved documentation. You decide!"
    "<br><br>"
    "Visit the "
    "<A HREF=\"http://www.trinitydesktop.org/jobs\">TDE Development</A> "
    "webpage to find out how you can contribute or mail us using one of the "
    "available <A HREF=\"http://www.trinitydesktop.org/mailinglist.php\">mailing lists</A>."
    "<br><br>"
    "If you need more information or documentation, then a visit to "
    "<A HREF=\"http://www.trinitydesktop.org/docs\">http://www.trinitydesktop.org/docs</A> "
    "will provide you with what you need.");

  const TQString text4 = i18n(""
    "TDE is available free of charge, but making it is not free.<br><br>"
    "The Trinity team <i>does need</i> financial support. The money is used to "
    "support the expenses incurred to keep the TDE servers running, so that you - "
    "the user - can access them at any time. You are encouraged to support Trinity "
    "through a financial or hardware donation, using one of the ways described at "
    "<a href=\"http://www.trinitydesktop.org/donate.php\">http://www.trinitydesktop.org/donate.php</a>."
    "<br><br>Thank you very much in advance for your support!");

  setHelp( TQString::fromLatin1("khelpcenter/main.html"), TQString::null );
  setTitle(i18n("Trinity Desktop Environment. Release %1").
	   arg(TQString::fromLatin1(TDE_VERSION_STRING)) );
  addTextPage( i18n("About Trinity","&About"), text1, true );
  addTextPage( i18n("&Report Bugs/Request Enhancements"), text2, true );
  addTextPage( i18n("&Join the Trinity Team"), text3, true );
  addTextPage( i18n("&Support Trinity"), text4, true );
  setImage( locate( "data", TQString::fromLatin1("tdeui/pics/abouttde.png")) );
  setImageBackgroundColor( white );
}
