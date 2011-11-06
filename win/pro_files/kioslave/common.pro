TEMPLATE	= lib

CONFIG += trinitylib #this is a dynamic kde library

include( $(KDELIBS)/win/common.pro )

LIBS += $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX

