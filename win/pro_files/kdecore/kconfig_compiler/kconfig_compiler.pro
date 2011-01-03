TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

TARGET = kconfig_compiler

win32 {
#CONFIG -= console
#CONFIG += windows
}

#system( bash ktqmoc )

SOURCES = kconfig_compiler.cpp

HEADERS = 


