/*  This file is part of the KDE libraries
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <stdlib.h>

#include <tqfile.h>

#include <dcopclient.h>
#include <dcopref.h>

#include "tdeaboutdata.h"
#include "tdeapplication.h"
#include "tdecmdlineargs.h"
#include "kglobal.h"
#include "klocale.h"
#include "kservice.h"
#include "kservicegroup.h"
#include "kstandarddirs.h"

static TDECmdLineOptions options[] = {
   { "utf8", I18N_NOOP("Output data in UTF-8 instead of local encoding"), 0 },
   { "print-menu-id", I18N_NOOP("Print menu-id of the menu that contains\nthe application"), 0 },
   { "print-menu-name", I18N_NOOP("Print menu name (caption) of the menu that\ncontains the application"), 0 },
   { "highlight", I18N_NOOP("Highlight the entry in the menu"), 0 },
   { "nocache-update", I18N_NOOP("Do not check if sycoca database is up to date"), 0 },
   { "+<application-id>", I18N_NOOP("The id of the menu entry to locate"), 0 },
   TDECmdLineLastOption
};

static const char appName[] = "tde-menu";
static const char appVersion[] = "1.0";
static bool utf8;

static bool bPrintMenuId;
static bool bPrintMenuName;
static bool bHighlight;

static void result(const TQString &txt)
{
   if (utf8)
      puts( txt.utf8() );
   else
      puts( txt.local8Bit() );
}

static void error(int exitCode, const TQString &txt)
{
   tqWarning("tde-menu: %s", txt.local8Bit().data());
   exit(exitCode);
}

static void findMenuEntry(KServiceGroup::Ptr parent, const TQString &name, const TQString &menuId)
{
   KServiceGroup::List list = parent->entries(true, true, false);
   KServiceGroup::List::ConstIterator it = list.begin();
   for (; it != list.end(); ++it)
   {
      KSycocaEntry * e = *it;

      if (e->isType(KST_KServiceGroup))
      {
         KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));
         
         findMenuEntry(g, name.isEmpty() ? g->caption() : name+"/"+g->caption(), menuId);
      }
      else if (e->isType(KST_KService))
      {
         KService::Ptr s(static_cast<KService *>(e));
         if (s->menuId() == menuId)
         {
            if (bPrintMenuId)
            {
               result(parent->relPath());
            }
            if (bPrintMenuName)
            {
               result(name);
            }
            if (bHighlight)
            {
               DCOPRef kicker( "kicker", "kicker" );
               bool result = kicker.call( "highlightMenuItem", menuId );
               if (!result)
                  error(3, TQString(i18n("Menu item '%1' could not be highlighted.").arg(menuId)).local8Bit());
            }
            exit(0);
         }
      }
   }
}


int main(int argc, char **argv)
{
   TDELocale::setMainCatalogue("tdelibs");
   const char *description = I18N_NOOP("TDE Menu query tool.\n"
   "This tool can be used to find in which menu a specific application is shown.\n"
   "The --highlight option can be used to visually indicate to the user where\n"
   "in the TDE menu a specific application is located.");
   
   TDEAboutData d(appName, I18N_NOOP("tde-menu"), appVersion,
                description,
                TDEAboutData::License_GPL, "(c) 2003 Waldo Bastian");
   d.addAuthor("Waldo Bastian", I18N_NOOP("Author"), "bastian@kde.org");

   TDECmdLineArgs::init(argc, argv, &d);
   TDECmdLineArgs::addCmdLineOptions(options);

//   TDEApplication k(false, false);
   TDEApplication k(false);
   k.disableSessionManagement();

   // this program is in tdelibs so it uses tdelibs as catalog
   TDELocale::setMainCatalogue("tdelibs");

   TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
   if (args->count() != 1)
      TDECmdLineArgs::usage(i18n("You must specify an application-id such as 'tde-konsole.desktop'"));

   utf8 = args->isSet("utf8");

   bPrintMenuId = args->isSet("print-menu-id");
   bPrintMenuName = args->isSet("print-menu-name");
   bHighlight = args->isSet("highlight");

   if (!bPrintMenuId && !bPrintMenuName && !bHighlight)
      TDECmdLineArgs::usage(i18n("You must specify at least one of --print-menu-id, --print-menu-name or --highlight"));

   if (args->isSet("cache-update"))
   {
      TQStringList args;
      args.append("--incremental");
      args.append("--checkstamps");
      TQString command = "tdebuildsycoca";
      TQCString _launcher = TDEApplication::launcher();
      if (!DCOPRef(_launcher, _launcher).call("tdeinit_exec_wait", command, args).isValid())
      {
         tqWarning("Can't talk to tdelauncher!");
         command = TDEGlobal::dirs()->findExe(command);
         command += " " + args.join(" ");
         system(command.local8Bit());
      }
   }

   TQString menuId = TQFile::decodeName(args->arg(0));
   KService::Ptr s = KService::serviceByMenuId(menuId);
   
   if (!s)
      error(1, i18n("No menu item '%1'.").arg(menuId));

   findMenuEntry(KServiceGroup::root(), "", menuId);

   error(2, i18n("Menu item '%1' not found in menu.").arg(menuId));
   return 2;
}

