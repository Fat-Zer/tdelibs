INCLUDES = -I$(top_builddir)/tdeabc -I$(top_srcdir)/tdeabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcefileconfig.h

lib_LTLIBRARIES = libkabc_file.la
libkabc_file_la_SOURCES = resourcefile.cpp resourcefileconfig.cpp
libkabc_file_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libkabc_file_la_LIBADD = $(LIB_KABC) $(LIB_QT) $(top_builddir)/tderesources/libtderesources.la $(LIB_TDEFILE) $(LIB_TDECORE) $(LIB_TDEUI)
libkabc_file_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h


kde_module_LTLIBRARIES = kabc_file.la
kabc_file_la_SOURCES = resourcefileplugin.cpp
kabc_file_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
kabc_file_la_LIBADD = libkabc_file.la $(LIB_QT) $(LIB_TDECORE)
kabc_file_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabc_file.pot

kabcincludedir = $(includedir)/tdeabc
kabcinclude_HEADERS = resourcefile.h

servicedir = $(kde_servicesdir)/tderesources/tdeabc
service_DATA = file.desktop
