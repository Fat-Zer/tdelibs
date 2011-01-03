TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KUTILS_LIB

TARGET		= kutils$$KDEBUG

LIBS += $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kio$$KDELIB_SUFFIX 

INCLUDEPATH += $(KDELIBS)/interfaces/kregexpeditor

system( bash ktqmoc )

SOURCES = \
ktqfind.cpp \
ktqfinddialog.cpp \
kmultitabbar.cpp \
kplugininfo.cpp \
ktqreplace.cpp \
ktqreplacedialog.cpp

HEADERS		=
