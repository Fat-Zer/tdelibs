TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KMDI_LIB

TARGET		= kmdi$$KDEBUG

LIBS += $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kutils$$KDELIB_SUFFIX $$KDELIBDESTDIR/kparts$$KDELIB_SUFFIX

INCLUDEPATH += res $(KDELIBS)/kutils $(KDELIBS)/tdeui

system( bash kmoc )

SOURCES = \
kmdichildarea.cpp \
kmdichildfrm.cpp \
kmdichildfrmcaption.cpp \
kmdichildview.cpp \
kmdidockcontainer.cpp \
kmdidocumentviewtabwidget.cpp \
kmdifocuslist.cpp \
kmdiguiclient.cpp \
kmdimainfrm.cpp \
kmditaskbar.cpp \
kmditoolviewaccessor.cpp

HEADERS		=
