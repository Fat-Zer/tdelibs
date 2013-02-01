#include <tqtimer.h>
#include <tqlabel.h>

#include <kapplication.h>
#include <kstatusbar.h>
#include <kmenubar.h>

#include "kmainwindowtest.h"

MainWindow::MainWindow()
{
    TQTimer::singleShot( 2*1000, this, TQT_SLOT( showMessage() ) );

    setCentralWidget( new TQLabel( "foo", this ) );

    menuBar()->insertItem( "hi" );
}

void MainWindow::showMessage()
{
    statusBar()->show();
    statusBar()->message( "test" );
}

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "kmainwindowtest" );

    MainWindow* mw = new MainWindow; // deletes itself when closed
    mw->show();

    return app.exec();
}

#include "kmainwindowtest.moc"

/* vim: et sw=4 ts=4
 */
