include( ../common.pro )

TARGET		= keramik$$KDEBUG

system( bash ktqmoc )

SOURCES = \
colorutil.cpp \
gradients.cpp \
keramik.cpp \
pixmaploader.cpp

system( bash ./genemb.sh )

