INCLUDES=$(all_includes)

lib_LTLIBRARIES = libtdesasl.la
METASOURCES     = AUTO

kdesaslincludedir = $(includedir)/kio
kdesaslinclude_HEADERS = kdesasl.h

libtdesasl_la_SOURCES = kdesasl.cpp
libtdesasl_la_LDFLAGS = $(all_libraries) -version-info 3:0:2 -no-undefined
libtdesasl_la_LIBADD = $(LIB_KDECORE) $(LIB_QT)

