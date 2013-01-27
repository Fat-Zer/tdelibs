#include "kfdtest.h"

#include <tqstringlist.h>
#include <tdefiledialog.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <tqtimer.h>

KFDTest::KFDTest( const TQString& startDir, TQObject *parent, const char *name )
    : TQObject( parent, name ),
      m_startDir( startDir )
{
    TQTimer::singleShot( 1000, this, TQT_SLOT( doit() ));
}

void KFDTest::doit()
{
    KFileDialog *dlg = new KFileDialog( m_startDir, TQString::null, 0L,
                                        "file dialog", true );
    dlg->setMode( KFile::File);
    dlg->setOperationMode( KFileDialog::Saving );
    TQStringList filter;
    filter << "all/allfiles" << "text/plain";
    dlg->setMimeFilter( filter, "all/allfiles" );

    if ( dlg->exec() == KDialog::Accepted )
    {
        KMessageBox::information(0, TQString::fromLatin1("You selected the file: %1").arg( dlg->selectedURL().prettyURL() ));
    }
    
//     tqApp->quit();
}

#include "kfdtest.moc"
