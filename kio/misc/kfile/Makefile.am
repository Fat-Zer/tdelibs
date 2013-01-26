METASOURCES = AUTO

noinst_HEADERS = fileprops.h
INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio/kio $(all_includes)

bin_PROGRAMS = kfile
kfile_SOURCES = fileprops.cpp
kfile_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
kfile_LDADD = $(LIB_KIO)

