# release version with compiled-in tdefx classes
# this is used eg. for QT-only apps like Installer

TEMPLATE	= lib

unix { 
	INCLUDEPATH += moc
	#force finding libraries in current dir (for installer, etc):
	QMAKE_LFLAGS += -Wl,-rpath,.
	DEFINES += KDEFX_EXPORT=
}


CONFIG += kstyle

KW_CONFIG = release
CONFIG -= debug
CONFIG += release

include( $(KDELIBS)/win/common.pro )


LIBS= #DONT BE DEPENDENT ON ANY OTHER LIBS

CONFIG -= debug

# needed to export library classes:
DEFINES += MAKE_KSTYLE_LIB MAKE_KDEFX_LIB

TARGET		= thinkeramik

SOURCES = \
colorutil.cpp \
gradients.cpp \
thinkeramik.cpp \
pixmaploader.cpp

#compile in tdefx
SOURCES += \
../../../tdefx/kstyle.cpp \
../../../tdefx/kimageeffect.cpp \
../../../tdefx/kpixmapeffect.cpp \
../../../tdefx/kpixmap.cpp

system( moc thinkeramik.h -o moc/thinkeramik.moc )
system( moc ../../../tdefx/kstyle.h -o moc/kstyle.moc )


system( bash ./genemb.sh )

