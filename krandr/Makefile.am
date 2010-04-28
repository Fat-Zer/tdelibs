
INCLUDES =  -I$(srcdir)/.. $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

libkrandrincludedir = $(includedir)/libkrandr
libkrandrinclude_HEADERS = randr.h lowlevel_randr.h ktimerdialog.h libkrandr.h

lib_LTLIBRARIES = libkrandr.la
libkrandr_la_SOURCES  = randr.cpp lowlevel_randr.c ktimerdialog.cpp libkrandr.cc
METASOURCES = AUTO

libkrandr_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 0:95  -no-undefined
libkrandr_la_LIBADD = $(LIBASOUND) ../kdecore/libkdecore.la $(LIB_QT) $(LIB_XRANDR)

DOXYGEN_REFERENCES = kdecore
include ../admin/Doxyfile.am
