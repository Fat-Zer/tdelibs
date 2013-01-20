#include <tqwidget.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addresseedialog.h"

using namespace KABC;

static const KCmdLineOptions options[] =
{
  {"multiple", I18N_NOOP("Allow selection of multiple addressees"), 0},
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testkabcdlg",I18N_NOOP("TestKabc"),"0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);
  TDECmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
  if (args->isSet("multiple")) {
    Addressee::List al = AddresseeDialog::getAddressees( 0 );
    Addressee::List::ConstIterator it;
    kdDebug() << "Selected Addressees:" << endl;
    for( it = al.begin(); it != al.end(); ++it ) {
      kdDebug() << "  " << (*it).fullEmail() << endl;
    }
  } else {
    Addressee a = AddresseeDialog::getAddressee( 0 );
  
    if ( !a.isEmpty() ) {
      kdDebug() << "Selected Addressee:" << endl;
      a.dump();
    } else {
      kdDebug() << "No Addressee selected." << endl;
    }
  }
}
