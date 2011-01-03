#
# common.pro - Common definitions for KDElibs/win32 .pro files
# (c) 2003-2005, Jaroslaw Staniek, js@iidea.pl
#

# to avoid a need for using Q_WS_WIN in C source code
DEFINES += _WINDOWS WIN32_LEAN_AND_MEAN

# custom definitions, options on which KDElibs do not depend
exists( custom_defs.pro ) {
  include( custom_defs.pro )
}

# common version info for all libs:
!tqcontains( CONFIG, kde3lib ) {
	VER_MAJ = $(KDE_VER_MAJ)
	VER_MIN = $(KDE_VER_MIN)
	VER_PAT = $(KDE_VER_PAT)
}

CONFIG += qt thread warn_on 
isEmpty( KW_CONFIG ) {
  KW_CONFIG += debug
  #KW_CONFIG += release
  #KW_CONFIG += windows
  KW_CONFIG += console
}

# release switch has priority over debug
tqcontains(KW_CONFIG,release) {
  CONFIG -= debug
  CONFIG += release
}
tqcontains(KW_CONFIG,debug) {
  CONFIG += debug
  CONFIG -= release
}
tqcontains(KW_CONFIG,windows) {
  CONFIG += windows
  CONFIG -= console
}
tqcontains(KW_CONFIG,console) {
  CONFIG -= windows
  CONFIG += console
}

# global binary destination directory
isEmpty( KDEBINDESTDIR ) {
	KDEBINDESTDIR = $(KDEDIR)
}

# global library destination directory 
KDELIBDESTDIR = $$KDEBINDESTDIR\bin #shared with all binaries
###KDELIBDESTDIR = $$KDEBINDESTDIR\lib

# dlls suffixes for given target
isEmpty( KDEBUG ) {
  tqcontains(CONFIG,debug) {
		KDEBUG=_d
		KDELIBDEBUG=_d
		KDELIBDEBUGLIB=_d.lib
  }
  !tqcontains(CONFIG,debug) {
		KDEBUG=_
		tqcontains(CONFIG,kde3lib) {
			KDELIBDEBUG=
		}
		!tqcontains(CONFIG,kde3lib) {
			KDELIBDEBUG=_
		}
		KDELIBDEBUGLIB=.lib
  }
}
KDELIB_SUFFIX=$$KDEBUG$(KDE_VER).lib

tqcontains( TEMPLATE, app ) {
  # default dest dir for "app"
#  tqcontains(CONFIG,debug) {
    DESTDIR = $$KDEBINDESTDIR\bin
#  }
#  !tqcontains(CONFIG,debug) {
#    DESTDIR = $$KDEBINDESTDIR\release-bin
#  }
	!tqcontains(CONFIG,nokdecore) {
			LIBS += $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX
	}
	!tqcontains(CONFIG,nokdeui) {
			LIBS += $$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX
	}
	!tqcontains(CONFIG,nokdefx) {
			LIBS += $$KDELIBDESTDIR/kdefx$$KDELIB_SUFFIX
	}
}
# default template is "lib"
isEmpty( TEMPLATE ) {
  TEMPLATE = lib
}
tqcontains( TEMPLATE, lib ) {
	CONFIG	+= dll

	# indicate that we building a library
	QMAKE_CXXFLAGS += -DKDE_MAKE_LIB=1

	# lib/kde3 dest dir (for modules)
	tqcontains( CONFIG, kde3lib ) {
	  DESTDIR = $$KDEBINDESTDIR/lib/kde3
	  TARGET_EXT = .dll #- no ver. in filename
	}
	!tqcontains( CONFIG, kde3lib ) {
	  DESTDIR = $$KDELIBDESTDIR
	}
	tqcontains( CONFIG, kstyle ) {
	  DESTDIR = $$KDEBINDESTDIR/lib/kde3/plugins/styles
	  CONFIG += plugin
	}
#  !tqcontains(CONFIG,debug) {
#    DESTDIR = $$KDEBINDESTDIR\release-lib
#  }

  VERSION		= $(KDE_VER_MAJ).$(KDE_VER_MIN).$(KDE_VER_PAT)
#  VER_MAJ	= $(KDE_VER_MAJ)
#  VER_MIN = $(KDE_VER_MIN)
#  VER_PAT = $(KDE_VER_PAT)
}

# win32 dependent lib
!tqcontains( DEFINES, MAKE_KDEWIN32_LIB ) {
  LIBS += $$KDELIBDESTDIR/kdewin32$$KDELIB_SUFFIX
}

# libltdl:
!tqcontains( DEFINES, MAKE_LTDL_LIB ) {
  !tqcontains( DEFINES, MAKE_KDEWIN32_LIB ) {
    LIBS += $$KDELIBDESTDIR/ltdl$$KDELIB_SUFFIX
    DEFINES += LIBLTDL_DLL_IMPORT
  }
}

!tqcontains( DEFINES, QT_DLL) {
  DEFINES += QT_DLL
}

# global definitions
win32-borland {
    QMAKE_CXXFLAGS += /I $(KDELIBS)/win/kdelibs_global_win.h
    QMAKE_CFLAGS += /I $(KDELIBS)/win/kdelibs_global_win.h
}
win32-msvc* {
    QMAKE_CXXFLAGS += /FI$(KDELIBS)/win/kdelibs_global_win.h
    QMAKE_CFLAGS += /FI$(KDELIBS)/win/kdelibs_global_win.h

    # Language Extensions
    QMAKE_CXXFLAGS += /Ze
}

# enable Run-Time Type Information (needed by dynamic_cast)
QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_RTTI_ON  # /GR for msvc

# enables synchronous exception 
QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_EXCEPTIONS_ON # /GX for msvc

# catch Release-Build Errors in Debug Build
#DISABLED for msvc.net
#tqcontains( CONFIG, debug ) {
#	tqcontains( KW_CONFIG, debug ) {
#		QMAKE_CXXFLAGS += /GZ
#	}
#}

# create an output file whether or not LINK tqfinds an undefined symbol
# (warning 4006 will be raised instead of error):
QMAKE_LFLAGS += /FORCE:MULTIPLE

# Specify that filename is a C++ source file, even if it doesn’t have 
# a .cpp or .cxx extension, thus .cc files are compiled properly with msvc
QMAKE_CXXFLAGS += /TP

INCLUDEPATH	+= tqmoc $(KDELIBS)/win $(KDELIBS)

tqcontains(KW_CONFIG,release) {
OBJECTS_DIR = obj_rel
}
!tqcontains(KW_CONFIG,release) {
OBJECTS_DIR = obj
}

TQMOC_DIR = tqmoc

# enable this to temporary add debug info!
# CONFIG += debug
# CONFIG -= release

!tqcontains(CONFIG,debug) {
QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRTD /NODEFAULTLIB:MSVCR71D /NODEFAULTLIB:MSVCP71D
}
tqcontains(CONFIG,debug) {
QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCR71 /NODEFAULTLIB:MSVCP71 /NODEFAULTLIB:libc
}
