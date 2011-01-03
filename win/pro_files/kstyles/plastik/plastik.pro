include( ../common.pro )

TARGET = plastik$$KDEBUG

system( bash ktqmoc )

SOURCES = \
plastik.cpp \
misc.cpp
