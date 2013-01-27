## $Id$
## Makefile.am of tdebase/tdeioslave/metainfo

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
METASOURCES = AUTO

kde_module_LTLIBRARIES = kio_metainfo.la

kio_metainfo_la_SOURCES = metainfo.cpp
kio_metainfo_la_LIBADD = $(LIB_KIO) $(LIB_QT) $(LIB_TDECORE)
kio_metainfo_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = metainfo.h 

kdelnk_DATA = metainfo.protocol
kdelnkdir = $(kde_servicesdir)

#servicetypes_DATA = thumbcreator.desktop
#servicetypesdir = $(kde_servicetypesdir)

#services_DATA = imagethumbnail.desktop textthumbnail.desktop 
#                htmlthumbnail.desktop gsthumbnail.desktop
#servicesdir = $(kde_servicesdir)
