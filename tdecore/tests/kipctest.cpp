#include <stdio.h>
#include <stdlib.h>
#include <tqobject.h>
#include <kapplication.h>
#include <kipc.h>
#include "kipctest.h"

MyObject::MyObject()
    : TQObject(0L, "testobj")
{
    connect(kapp, TQT_SIGNAL(kdisplayPaletteChanged()), TQT_SLOT(slotPaletteChanged()));
    connect(kapp, TQT_SIGNAL(kdisplayFontChanged()), TQT_SLOT(slotFontChanged()));
    connect(kapp, TQT_SIGNAL(kdisplayStyleChanged()), TQT_SLOT(slotStyleChanged()));
    connect(kapp, TQT_SIGNAL(backgroundChanged(int)), TQT_SLOT(slotBackgroundChanged(int)));
    connect(kapp, TQT_SIGNAL(appearanceChanged()), TQT_SLOT(slotAppearanceChanged()));
    connect(kapp, TQT_SIGNAL(kipcMessage(int,int)), TQT_SLOT(slotMessage(int,int)));
}

int main(int argc, char **argv)
{
    KApplication app(argc, argv, "kipc");

    if (argc == 3) 
    {
	KIPC::sendMessageAll((KIPC::Message) atoi(argv[1]), atoi(argv[2]));
	return 0;
    }

    MyObject obj;
    return app.exec();
}

#include "kipctest.moc"
