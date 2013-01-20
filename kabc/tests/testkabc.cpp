#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include <tqfile.h>
#include <tqimage.h>

#include "geo.h"
#include "secrecy.h"
#include "stdaddressbook.h"
#include "timezone.h"
#include "key.h"
#include "agent.h"
#include "vcardconverter.h"

using namespace KABC;

int main(int argc,char **argv)
{
    KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
    TDECmdLineArgs::init(argc, argv, &aboutData);

    KApplication app( false, false );
    AddressBook *ab = StdAddressBook::self();

#define READ

#ifdef READ
    AddressBook::Iterator it;
    for ( it = ab->begin(); it != ab->end(); ++it ) {
      TQString vcard;
      VCardConverter converter;
      converter.addresseeToVCard( *it, vcard );
      kdDebug() << "card=" << vcard << endl;
    }
#else
    Addressee addr;

    addr.setGivenName("Tobias");
    addr.setFamilyName("Koenig");


    Picture pic;
    TQImage img;
    img.load("/home/tobias/test.png");
/*
    pic.setData(img);
    pic.setType(TQImage::imageFormat("/home/tobias/test.png"));
*/
    pic.setUrl("http://www.mypict.de");
    addr.setLogo( pic );

    ab->insertAddressee( addr );

    StdAddressBook::save();
#endif

    return 0;
}
