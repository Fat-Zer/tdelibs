#include <knotifyclient.h>
#include <kapplication.h>

int main( int argc, char **argv )
{
	TDEApplication app( argc, argv, "knotifytest" );
    KNotifyClient::userEvent( "This is a notification to notify you :)", 
                              KNotifyClient::Messagebox,
                              KNotifyClient::Error );
}
