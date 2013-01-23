#include "kxmlguitest.h"
#include <kapplication.h>
#include <kmainwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kaction.h>
#include <kdebug.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <tqlineedit.h>
#include <tqdir.h>

void Client::slotSec()
{
    kdDebug() << "Client::slotSec()" << endl;
}

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "kxmlguitest" );

    // KXMLGUIClient looks in the "data" resource for the .rc files
    // Let's add $PWD (ideally $srcdir instead...) to it
    KGlobal::dirs()->addResourceDir( "data", TQDir::currentDirPath() );

    KMainWindow *mainwindow = new KMainWindow;

    TQLineEdit* line = new TQLineEdit( mainwindow );
    mainwindow->setCentralWidget( line );

    mainwindow->show();

    KXMLGUIBuilder *builder = new KXMLGUIBuilder( mainwindow );

    KXMLGUIFactory *factory = new KXMLGUIFactory( builder );

    Client *shell = new Client;
    shell->setInstance( new TDEInstance( "konqueror" ) );
    shell->instance()->dirs()->addResourceDir( "data", TQDir::currentDirPath() );

    (void)new KAction( "Split", "view_left_right", 0, 0, 0, shell->actionCollection(), "splitviewh" );

    shell->setXMLFile( "./kxmlguitest_shell.rc" );

    factory->addClient( shell );

    Client *part = new Client;

    (void)new KAction( "decfont", "viewmag-", 0, 0, 0, part->actionCollection(), "decFontSizes" );
    (void)new KAction( "sec", "unlock", Qt::ALT + Qt::Key_1, part, TQT_SLOT( slotSec() ), part->actionCollection(), "security" );

    part->setXMLFile( "./kxmlguitest_part.rc" );

    factory->addClient( part );
    for ( int i = 0; i < 10; ++i )
    {
        factory->removeClient( part );
        factory->addClient( part );
    }

    return app.exec();
}
#include "kxmlguitest.moc"
