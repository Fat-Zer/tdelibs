include( ../common.pro )

TARGET		= light$$KDEBUG

system( bash ktqmoc )

SOURCES = \
light.cpp \
lightstyle-v2.cpp \
lightstyle-v3.cpp

