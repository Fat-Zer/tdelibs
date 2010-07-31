/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003,2004 Matthias Kretz <kretz@kde.org>
  Copyright (c) 2004 Frans Englich <frans.englich@telia.com>

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

#include <tqfile.h>
#include <tqlabel.h>
#include <tqlayout.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

#include "kcmoduleloader.h"


/***************************************************************/
/**
 * When something goes wrong in loading the module, this one
 * jumps in as a "dummy" module.
 */
class KCMError : public KCModule
{
	public:
		KCMError( const TQString& msg, const TQString& details, TQWidget* parent )
			: KCModule( parent, "KCMError" )
		{
			TQVBoxLayout* topLayout = new TQVBoxLayout( this );
			topLayout->addWidget( new TQLabel( msg, this ) );
			topLayout->addWidget( new TQLabel( details, this ) );
		}
};
/***************************************************************/




KCModule* KCModuleLoader::load(const KCModuleInfo &mod, const TQString &libname,
    KLibLoader *loader, ErrorReporting report, TQWidget * parent,
    const char * name, const TQStringList & args )
{
  // attempt to load modules with ComponentFactory, only if the symbol init_<lib> exists
  // (this is because some modules, e.g. kcmkio with multiple modules in the library,
  // cannot be ported to KGenericFactory)
  KLibrary *lib = loader->library(TQFile::encodeName(libname.arg(mod.library())));
  if (lib) {
    TQString initSym("init_");
    initSym += libname.arg(mod.library());

    if ( lib->hasSymbol(TQFile::encodeName(initSym)) )
    {
      KLibFactory *factory = lib->factory();
      if ( factory )
      {
        KCModule *module = KParts::ComponentFactory::createInstanceFromFactory<KCModule>( factory, parent, name ? name : mod.handle().latin1(), args );
        if (module)
          return module;
      }
      // else do a fallback
      kdDebug(1208) << "Unable to load module using ComponentFactory. Falling back to old loader." << endl;
    }

    // get the create_ function
    TQString factory("create_%1");
    void *create = lib->symbol(TQFile::encodeName(factory.arg(mod.handle())));

    if (create)
    {
      // create the module
      KCModule* (*func)(TQWidget *, const char *);
      func = (KCModule* (*)(TQWidget *, const char *)) create;
      return  func( parent, name ? name : mod.handle().latin1() );
    }
    else
    {
      TQString libFileName = lib->fileName();
      lib->unload();
      return reportError( report, i18n("<qt>There was an error when loading the module '%1'.<br><br>"
          "The desktop file (%2) as well as the library (%3) was found but "
          "yet the module could not be loaded properly. Most likely "
          "the factory declaration was wrong, or the "
          "create_* function was missing.</qt>")
          .arg( mod.moduleName() )
          .arg( mod.fileName() )
          .arg( libFileName ),
          TQString::null, parent );
    }

    lib->unload();
  }
  return reportError( report, i18n("The specified library %1 could not be found.")
      .arg( mod.library() ), TQString::null, parent );
  return 0;
}

KCModule* KCModuleLoader::loadModule(const KCModuleInfo &mod, bool withfallback, TQWidget * parent, const char * name, const TQStringList & args )
{
  return loadModule( mod, None, withfallback, parent, name, args );
}

KCModule* KCModuleLoader::loadModule(const KCModuleInfo &mod, ErrorReporting report, bool withfallback, TQWidget * parent, const char * name, const TQStringList & args )
{
  /*
   * Simple libraries as modules are the easiest case:
   *  We just have to load the library and get the module
   *  from the factory.
   */

  if ( !mod.service() )
  {
    if ( mod.moduleName() == "kcmlisa" || mod.moduleName() == "kcmkiolan" )
    {
      return reportError( report,
          i18n("The module %1 could not be found.")
          .arg( mod.moduleName() ),
          i18n("<qt><p>The Lisa and lan:/ ioslave modules "
            "are not installed by default in Kubuntu, because they are obsolete "
            "and replaced by zeroconf.<br> If you still wish to use them, you "
            "should install the lisa package from the Universe repository.</p></qt>"),
          parent );
    } else { 
      return reportError( report,
          i18n("The module %1 could not be found.")
          .arg( mod.moduleName() ),
          i18n("<qt><p>The diagnostics is:<br>The desktop file %1 could not be found.</p></qt>").arg(mod.fileName()),
          parent );
    }
  }

  if (!mod.library().isEmpty())
  {
    // get the library loader instance

    KLibLoader *loader = KLibLoader::self();

    KCModule *module = load(mod, "kcm_%1", loader, report, parent, name, args );
    /*
     * Only try to load libkcm_* if it exists, otherwise KLibLoader::lastErrorMessage would say
     * "libkcm_foo not found" instead of the real problem with loading kcm_foo.
     */
    if (!KLibLoader::findLibrary( TQCString( "libkcm_" ) + TQFile::encodeName( mod.library() ) ).isEmpty() )
      module = load(mod, "libkcm_%1", loader, report, parent, name, args );
    if (module)
      return module;
    return reportError( report,
        i18n("The module %1 could not be loaded.")
        .arg( mod.moduleName() ), TQString::null, parent );
  }

  /*
   * Ok, we could not load the library.
   * Try to run it as an executable.
   * This must not be done when calling from kcmshell, or you'll
   * have infinite recursion
   * (startService calls kcmshell which calls modloader which calls startService...)
   *
   */
  if(withfallback)
  {
    KApplication::startServiceByDesktopPath(mod.fileName(), TQString::null);
  }
  else
  {
    return reportError( report,
        i18n("The module %1 is not a valid configuration module.")
        .arg( mod.moduleName() ), i18n("<qt><p>The diagnostics is:<br>The desktop file %1 does not specify a library.</qt>").arg(mod.fileName()), parent );
  }

  return 0;
}

KCModule* KCModuleLoader::loadModule(const TQString &module, TQWidget *parent,
      const char *name, const TQStringList & args)
{
  return loadModule(KCModuleInfo(module), None, false, parent, name, args);
}

KCModule* KCModuleLoader::loadModule(const TQString &module, ErrorReporting
    report, TQWidget *parent, const char *name, const TQStringList & args)
{
  return loadModule(KCModuleInfo(module), report, false, parent, name, args);
}

void KCModuleLoader::unloadModule(const KCModuleInfo &mod)
{
  // get the library loader instance
  KLibLoader *loader = KLibLoader::self();

  // try to unload the library
  TQString libname("libkcm_%1");
  loader->unloadLibrary(TQFile::encodeName(libname.arg(mod.library())));

  libname = "kcm_%1";
  loader->unloadLibrary(TQFile::encodeName(libname.arg(mod.library())));
}

void KCModuleLoader::showLastLoaderError(TQWidget *parent)
{
  KMessageBox::detailedError(parent,
      i18n("There was an error loading the module."),i18n("<qt><p>The diagnostics is:<br>%1"
        "<p>Possible reasons:</p><ul><li>An error occurred during your last "
        "KDE upgrade leaving an orphaned control module<li>You have old third party "
        "modules lying around.</ul><p>Check these points carefully and try to remove "
        "the module mentioned in the error message. If this fails, consider contacting "
        "your distributor or packager.</p></qt>")
      .arg(KLibLoader::self()->lastErrorMessage()));

}

bool KCModuleLoader::testModule( const TQString& module )
{
  return testModule( KCModuleInfo( module ) );
}

bool KCModuleLoader::testModule( const KCModuleInfo& module )
{
  if (!module.service())
  {
    kdDebug(1208) << "Module '" << module.fileName() << "' not found." << endl;
    return true;
  }

  bool doLoad = module.service()->property( "X-KDE-Test-Module", TQVariant::Bool ).toBool();
  if( !doLoad )
  {
    return true;
  }
  else
  {
    /**
     * If something fails we return true - we can't risk functionality becoming
     * unavailable because of a buggy test. Furthermore, the error needs to
     * show so it is discovered. KCModuleProxy will detect the error and load
     * a corresponding KCMError.
     * */
    KLibLoader* loader = KLibLoader::self();
    KLibrary* library = loader->library( TQFile::encodeName((TQString("kcm_%1").arg(module.library()))) );
    if( library )
    {
      void *test_func = library->symbol( TQString("test_%1").arg(module.factoryName()).utf8() );
      if( test_func )
      {
        bool (*func)() = (bool(*)())test_func;
        if( func() )
        {
          return true;
        }
        else
        {
          return false;
        }
      }
      else
      {
        kdDebug(1208) << "The test function for module '" << module.fileName() << "' could not be found." << endl;
        return true;
      }
    }
    kdDebug(1208) << "The library '" << module.library() << "' could not be found." << endl;
    return true;
  }
}

KCModule* KCModuleLoader::reportError( ErrorReporting report, const TQString & text,
        TQString details, TQWidget * parent )
{
  if( details.isNull() )
    details = i18n("<qt><p>The diagnostics is:<br>%1"
        "<p>Possible reasons:</p><ul><li>An error occurred during your last "
        "KDE upgrade leaving an orphaned control module<li>You have old third party "
        "modules lying around.</ul><p>Check these points carefully and try to remove "
        "the module mentioned in the error message. If this fails, consider contacting "
        "your distributor or packager.</p></qt>").arg(KLibLoader::self()->lastErrorMessage());
  if( report & Dialog )
    KMessageBox::detailedError( parent, text, details );
  if( report & Inline )
    return new KCMError( text, details, parent );
  return 0;
}

// vim: ts=2 sw=2 et

