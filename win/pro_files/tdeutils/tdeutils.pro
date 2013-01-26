TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KUTILS_LIB

TARGET		= tdeutils$$KDEBUG

LIBS += $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kio$$KDELIB_SUFFIX 

INCLUDEPATH += $(KDELIBS)/interfaces/kregexpeditor

system( bash kmoc )

SOURCES = \
kfind.cpp \
kfinddialog.cpp \
tdemultitabbar.cpp \
kplugininfo.cpp \
kreplace.cpp \
kreplacedialog.cpp

HEADERS		=
