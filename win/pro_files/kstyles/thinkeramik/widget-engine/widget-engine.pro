include( ../../common.pro )

TARGET		= thinkeramik$$KDEBUG

unix { 
	INCLUDEPATH += tqmoc
	#force tqfinding libraries in current dir (for installer, etc):
	QMAKE_LFLAGS += -Wl,-rpath,.
}

system( bash ktqmoc )

SOURCES = \
colorutil.cpp \
gradients.cpp \
thinkeramik.cpp \
pixmaploader.cpp

tqcontains( KW_CONFIG, release ) {
	system( bash ./genemb.sh _rel )
}

!tqcontains( KW_CONFIG, release ) {
	system( bash ./genemb.sh )
}

