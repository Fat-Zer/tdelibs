TEMPLATE	= lib

KW_CONFIG = release
CONFIG -= debug
CONFIG += release

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KDEFX_LIB

system( tqmoc kstyle.h -o tqmoc/kstyle.moc )

LIBS -= "$(KDEDIR)\lib\kdewin32_$(KDE_VER).lib" "$(KDEDIR)\lib\ltdl_$(KDE_VER).lib"

SOURCES = \
kimageeffect.cpp \
kpixmapeffect.cpp \
kpixmap.cpp \
kstyle.cpp \
kdrawutil.cpp \
kcpuinfo.cpp

HEADERS		=

TARGET		= kdefx
