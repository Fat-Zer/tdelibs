TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
#DEFINES += MAKE_DCOP_LIB

DEFINES += KDECORE_EXPORT=

TARGET		= dcoptqfind

#system( bash ktqmoc .. )

#INCLUDEPATH += .. ../tqmoc

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX

SOURCES	+= ../dcoptqfind.cpp
