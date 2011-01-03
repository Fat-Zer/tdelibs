TEMPLATE = app

include( $(KDELIBS)/win/common.pro )

TARGET = testdcop
DESTDIR = .

system( bash ktqmoc .. )

INCLUDEPATH += ../tqmoc

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX

SOURCES = ../testdcop.cpp
