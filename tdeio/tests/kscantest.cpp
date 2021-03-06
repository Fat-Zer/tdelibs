#include <tdeapplication.h>
#include <kscan.h>

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "kscantest" );
    KScanDialog *dlg = KScanDialog::getScanDialog();
    if ( !dlg ) {
	tqDebug("*** EEK, no Scan-service available, aborting!");
	return 0;
    }
    
    dlg->show();

    return app.exec();
}
