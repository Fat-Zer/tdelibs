#include <tqwidget.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <tdecmdlineargs.h>

#include "stdaddressbook.h"

#include "distributionlisteditor.h"
#include "distributionlist.h"

using namespace KABC;

static const TDECmdLineOptions options[] =
{
  {"list <listname>", I18N_NOOP("Show distribution list with name <listname>"), 0},
  TDECmdLineLastOption
};


int main(int argc,char **argv)
{
  TDEAboutData aboutData("testdistlist",I18N_NOOP("Test Distribution Lists"),"0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);
  TDECmdLineArgs::addCmdLineOptions( options );

  TDEApplication app;

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
  if (args->isSet("list")) {
    TQString name = args->getOption("list");
    
    DistributionListManager *manager =
        new DistributionListManager( StdAddressBook::self() );
    manager->load();
    DistributionList *list = manager->list( name );
    if ( !list ) {
      kdDebug() << "No list with name '" << name << "'" << endl;
      return 1;
    } else {
      kdDebug() << "RESULT: " << list->emails().join(", ") << endl;
      return 0; 
    }
  }

  DistributionListEditor *editor =
      new DistributionListEditor( StdAddressBook::self(), 0 );

  editor->show();
  app.setMainWidget(editor);
  
  TQObject::connect( &app, TQT_SIGNAL( lastWindowClosed() ), &app, TQT_SLOT( quit() ) );

  app.exec();
  
  delete editor;
}
