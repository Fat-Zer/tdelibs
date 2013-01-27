#include "kcustommenueditor.h"
#include <kapplication.h>
#include <klocale.h>
#include <tdeconfig.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalogue("tdelibs");
  TDEApplication app(argc, argv, "KCustomMenuEditorTest");
  KCustomMenuEditor editor(0);
  TDEConfig *cfg = new TDEConfig("kdesktop_custom_menu2");
  editor.load(cfg);
  if (editor.exec())
  {
     editor.save(cfg);
     cfg->sync();
  }
}

