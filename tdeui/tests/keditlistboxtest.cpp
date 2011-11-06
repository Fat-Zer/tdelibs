#include <kapplication.h>
#include <keditlistbox.h>
#include <kcombobox.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "keditlistboxtest" );

    KEditListBox::CustomEditor editor( new KComboBox( true, 0L, "test" ) );
    KEditListBox *box = new KEditListBox( TQString::tqfromLatin1("KEditListBox"),
                                          editor );
    
    box->insertItem( TQString::tqfromLatin1("Test") );
    box->insertItem( TQString::tqfromLatin1("for") );
    box->insertItem( TQString::tqfromLatin1("this") );
    box->insertItem( TQString::tqfromLatin1("KEditListBox") );
    box->insertItem( TQString::tqfromLatin1("Widget") );
    box->show();

    return app.exec();
}
