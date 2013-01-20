#include "kdatewidget.h"
#include <tqlineedit.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalogue("tdelibs");
  TDEApplication app(argc, argv, "KDateWidgettest");
  KDateWidget dateWidget;
  app.setMainWidget(&dateWidget);
  dateWidget.show();
  // dateWidget.setEnabled(false);
  return app.exec();
}

