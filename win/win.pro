TEMPLATE	= lib
DEFINES += MAKE_KDEWIN32_LIB

include( $(KDELIBS)/win/common.pro )

TARGET		= kdewin32$$KDEBUG

QMAKE_CXXFLAGS += /FI$(KDELIBS)/kdecore/kdelibs_export.h
QMAKE_CFLAGS += /FI$(KDELIBS)/kdecore/kdelibs_export.h

system( tqmoc qeventloopex.h -o tqmoc/qeventloopex.moc )

WIN9x {
	DEFINES += KDEWIN32_9x

	DESTDIR = $$KDELIBDESTDIR/win9x

	tqcontains(KW_CONFIG,release) {
		OBJECTS_DIR = 9x/obj_rel
	}
	!tqcontains(KW_CONFIG,release) {
		OBJECTS_DIR = 9x/obj
	}
}

SOURCES = \
realpath.c \
unistd.c \
readdir.c \
resource.c \
pwd.c \
fcntl.c \
signal.c \
uname.c \
net.c \
time.c \
dummy.cpp \
strndup.c \
fsync.c \
grp.c \
syslog.c \
win32_utils.c \
win32_utils2.cpp \
kde_file_win.c \
mmap.c \
getenv.c \
qeventloopex.cpp \
bootstrap.cpp

#mkdir.c
#strlcpy.c \
#strlcat.c \


