#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addressbook.h"
#include "vcardformat.h"
#include "resourcesql.h"

using namespace KABC;

int main(int argc,char **argv)
{
  TDEAboutData aboutData("testdb","TestKabcDB","0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);

//  TDEApplication app( false, false );
  TDEApplication app;

  AddressBook ab;
  
  ResourceSql r( &ab, "root", "kde4ever", "localhost" );
  if ( ! r.open() ) {
    kdDebug() << "Failed to open resource." << endl;
  }
  
  r.load( &ab );
  
  r.close();
  
  ab.dump();
}
