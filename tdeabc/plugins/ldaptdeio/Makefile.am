INCLUDES = -I$(top_srcdir)/tdeabc -I$(top_builddir)/tdeabc $(all_includes)

# these are the headers for your project
noinst_HEADERS = resourceldaptdeioconfig.h

lib_LTLIBRARIES = libkabc_ldaptdeio.la
libkabc_ldaptdeio_la_SOURCES = resourceldaptdeio.cpp resourceldaptdeioconfig.cpp
libkabc_ldaptdeio_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libkabc_ldaptdeio_la_LIBADD	= $(LIB_KABC) $(LIB_KIO) $(LIB_QT) $(top_builddir)/tderesources/libtderesources.la $(LIB_TDEUI) $(LIB_TDECORE)
libkabc_ldaptdeio_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h

kde_module_LTLIBRARIES = kabc_ldaptdeio.la
kabc_ldaptdeio_la_SOURCES = resourceldaptdeioplugin.cpp
kabc_ldaptdeio_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_ldaptdeio_la_LIBADD	= libkabc_ldaptdeio.la $(LIB_QT) $(LIB_TDECORE)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabc_ldaptdeio.pot

kabcincludedir = $(includedir)/tdeabc
kabcinclude_HEADERS = resourceldaptdeio.h

servicedir = $(kde_servicesdir)/tderesources/tdeabc
service_DATA = ldaptdeio.desktop

resourceldaptdeioplugin.lo: ../../addressee.h
