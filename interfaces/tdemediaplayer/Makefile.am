if include_ARTS
kfileaudiopreview_subdir = kfileaudiopreview
endif

SUBDIRS = . $(kfileaudiopreview_subdir)
INCLUDES = -I$(srcdir)/.. $(all_includes)

lib_LTLIBRARIES = libtdemediaplayer.la

libtdemediaplayer_la_SOURCES = player.cpp playerdcopobject.skel view.cpp
libtdemediaplayer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libtdemediaplayer_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) $(top_builddir)/dcop/libDCOP.la $(LIB_TDEUI) $(LIB_TDECORE)
libtdemediaplayer_la_METASOURCES = AUTO

tdemediaplayerinclude_HEADERS = player.h playerdcopobject.h view.h
tdemediaplayerincludedir = $(includedir)/tdemediaplayer

kde_servicetypes_DATA = tdemediaplayerengine.desktop tdemediaplayer.desktop
