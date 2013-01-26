TEMPLATE	= lib
CONFIG += trinitylib #this is a dynamic kde library

include( $(KDELIBS)/win/common.pro )

system( makekdewidgets -o kdewidgets.cpp kde.widgets )

#no _d because it's a special case

TARGET = kdewidgets

DESTDIR		= $$KDELIBDESTDIR/trinity/plugins/designer

LIBS +=  $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kabc$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeutils$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tderesources$$KDELIB_SUFFIX 

SOURCES += classpreviews.cpp kdewidgets.cpp

