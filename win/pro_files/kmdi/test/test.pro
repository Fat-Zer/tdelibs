TEMPLATE        = app
#CONFIG		= qt warn_on thread

include( $(KDELIBS)/win/common.pro )


CONFIG -= release
CONFIG -= windows
CONFIG += debug
CONFIG += console

unix:DEFINES   = NO_INCLUDE_MOCFILES QT_NO_COMPAT
unix:LIBS       += -lkmdi -L$(TDEDIR)/lib -ltdecore -ltdeui -lDCOP -lkparts
unix:INCLUDEPATH     += . .. $(TDEDIR)/include

LIBS += $$KDELIBDESTDIR\kmdi$$KDELIB_SUFFIX $$KDELIBDESTDIR\kparts$$KDELIB_SUFFIX

HEADERS = hello.h \
          mainwidget.h

SOURCES = hello.cpp \
          mainwidget.cpp \
          main.cpp

TARGET    = kfourchildren
