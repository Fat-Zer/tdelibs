#include <kde_terminal_interface.h>
#include <tdeparts/part.h>
#include <ktrader.h>
#include <klibloader.h>
#include <tdemainwindow.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <tqdir.h>
#include <assert.h>
#include <kmessagebox.h>
#include <cassert>
#include "main.h"
#include "main.moc"

Win::Win()
{
    KLibFactory* factory = KLibLoader::self()->factory( "libkonsolepart" );
    assert( factory );
    KParts::Part* p = static_cast<KParts::Part*>( factory->create( this, "tralala", TQOBJECT_OBJECT_NAME_STRING, "KParts::ReadOnlyPart" ) );
    setCentralWidget( p->widget() );

    TerminalInterface* t = ::tqqt_cast<TerminalInterface*>( p );
    t->showShellInDir( TQDir::home().path() );
//     TQStrList l;
//     l.append( "python" );
//     t->startProgram( TQString::fromUtf8( "/usr/bin/python" ), l );

    connect( p, TQT_SIGNAL( processExited( int ) ),
             this, TQT_SLOT( pythonExited( int ) ) );
}


int main( int argc, char** argv )
{
    TDEAboutData* about = new TDEAboutData( "tetest", "TETest", "0.1" );
    TDECmdLineArgs::init( argc, argv, about );
    TDEApplication a;
    Win* win = new Win();
    win->show();
    return a.exec();
};

#include <iostream>

void Win::pythonExited()
{
    std::cerr << "hee, " << p << std::endl;
    std::cerr << ( ::tqqt_cast<TerminalInterface>(p) ) << std::endl;
    // KMessageBox::sorry( this, TQString::fromUtf8( "Exited, status was %1" ).arg( status ) );
    disconnect(p, TQT_SIGNAL( processExited() ),
            this, TQT_SLOT( pythonExited() ));
    TerminalInterface* t = ::tqqt_cast<TerminalInterface*>( p );
    TQStrList l;
    l.append( "echo" );
    l.append( "hello world" );
    t->startProgram( TQString::fromUtf8( "/bin/echo" ), l );
}

void Win::forked()
{
    std::cerr << "hello from the child process!" << std::endl;
}
