include( ../common.pro )

LIBS += $$KDELIBDESTDIR\kdecore$$KDELIB_SUFFIX

TARGET		= kthemestyle$$KDEBUG

SOURCES = \
kthemestyle.cpp \
kthemebase.cpp \
kstyledirs.cpp

system( tqmoc kthemestyle.h -o tqmoc/kthemestyle.moc )
system( tqmoc kthemebase.h -o tqmoc/kthemebase.moc )


