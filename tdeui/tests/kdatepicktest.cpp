#include "kdatepicker.h"
#include <tqlineedit.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalogue("tdelibs");
  TDEApplication app(argc, argv, "KDatePickertest");
  KDatePicker picker;
  app.setMainWidget(&picker);
  picker.show();
  // picker.setEnabled(false);
  return app.exec();
}

