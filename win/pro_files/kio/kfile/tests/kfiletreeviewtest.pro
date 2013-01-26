TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )


#allow to select target independently from debug information
tdebase_release:CONFIG -= console
tdebase_release:CONFIG += windows
tdebase_release:QMAKE_MAKEFILE = Makefile.release


TARGET		= kfiletreeviewtest

LIBS +=  $$KDELIBDESTDIR/tdefx$$KDELIB_SUFFIX \
  $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX \
  $$KDELIBDESTDIR/tdeparts$$KDELIB_SUFFIX \
  $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX

# icon
LIBS+=$(KDELIBS)/win/resources/kfind.res

system( bash kmoc )

SOURCES = \
kfiletreeviewtest.cpp

HEADERS		= 


