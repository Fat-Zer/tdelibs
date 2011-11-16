#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <tqstring.h>
#include <stdio.h>
#include <unistd.h>

#include "java/kjavaappletserver.h"
#include "java/kjavaapplet.h"
#include "java/kjavaappletwidget.h"

static KCmdLineOptions options[] =
{
    { "+[tdelibs_path]", "path to tdelibs directory", 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "testKJASSever", "testKJASServer", "test program", "0.0" );

    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    TQString path_to_tdelibs = "/build/wynnw/kde-src";

    KJavaAppletWidget *a = new KJavaAppletWidget;

    a->show();

    a->applet()->setBaseURL( "file:" + path_to_tdelibs + "/tdelibs/khtml/test/" );
    a->applet()->setAppletName( "Lake" );
    a->applet()->setAppletClass( "lake.class" );
    a->applet()->setParameter( "image", "konqi.gif" );

    a->showApplet();
    a->applet()->start();

    app.exec();
}
