#include <sys/times.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>

#include "addressbook.h"
#include "vcardformat.h"
#include "plugins/file/resourcefile.h"
#if 0
#include "resourcesql.h"
#endif

using namespace TDEABC;

int main(int argc,char **argv)
{
  TDEAboutData aboutData("bigread","BigReadKabc","0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);

  TDEApplication app( false, false );
  
  AddressBook ab; 
   
  ResourceFile r( "my.tdeabc", "vcard2" );
  ab.addResource( &r );

#if 0  
  ResourceSql rsql( &ab, "root", "kde4ever", "localhost" );
  ab.addResource( &rsql );
#endif

  struct tms start;

  times( &start );

#if 0
  kdDebug() << "utime : " << int( start.tms_utime ) << endl;
  kdDebug() << "stime : " << int( start.tms_stime ) << endl;
  kdDebug() << "cutime: " << int( start.tms_cutime ) << endl;
  kdDebug() << "cstime: " << int( start.tms_cstime ) << endl;
#endif
	    
  kdDebug() << "Start load" << endl;
  ab.load();
  kdDebug() << "Finished load" << endl;

  struct tms end;

  times( &end );

#if 0
  kdDebug() << "utime : " << int( end.tms_utime ) << endl;
  kdDebug() << "stime : " << int( end.tms_stime ) << endl;
  kdDebug() << "cutime: " << int( end.tms_cutime ) << endl;
  kdDebug() << "cstime: " << int( end.tms_cstime ) << endl;
#endif

  kdDebug() << "UTime: " << int( end.tms_utime ) - int( start.tms_utime ) << endl; 
  kdDebug() << "STime: " << int( end.tms_stime ) - int( start.tms_stime ) << endl; 

//  ab.dump();
}
