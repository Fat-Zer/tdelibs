#include <kiconloader.h>
#include <tqdatetime.h>
#include <stdio.h>
#include <kapplication.h>
#include <stdlib.h>
#include <kdebug.h>

int main(int argc, char *argv[])
{
  KApplication app(argc,argv,"kiconloadertest"/*,false,false*/);

  KIconLoader * mpLoader = KGlobal::iconLoader();
  KIcon::Context mContext = KIcon::Application;
  TQTime dt;
  dt.start();
  int count = 0;
  for ( int mGroup = 0; mGroup < KIcon::LastGroup ; ++mGroup )
  {
      kdDebug() << "queryIcons " << mGroup << "," << mContext << endl;
      TQStringList filelist=mpLoader->queryIcons(mGroup, mContext);
      kdDebug() << " -> found " << filelist.count() << " icons." << endl;
      int i=0;
      for(TQStringList::Iterator it = filelist.begin();
          it != filelist.end() /*&& i<10*/;
          ++it, ++i )
      {
          //kdDebug() << ( i==9 ? "..." : (*it) ) << endl;
          mpLoader->loadIcon( (*it), (KIcon::Group)mGroup );
	  ++count;
      }
  }
  kdDebug() << "Loading " << count << " icons took " << (float)(dt.elapsed()) / 1000 << " seconds" << endl;
}

