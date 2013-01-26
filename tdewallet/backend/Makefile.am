
INCLUDES = -I$(srcdir)/../client $(all_includes)

lib_LTLIBRARIES = libtdewalletbackend.la

libtdewalletbackend_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0 -no-undefined
libtdewalletbackend_la_LIBADD = $(LIB_QT) ../../tdecore/libtdecore.la
libtdewalletbackend_la_SOURCES = blockcipher.cc		\
			       blowfish.cc		\
			       cbc.cc			\
			       sha1.cc			\
			       tdewalletentry.cc		\
			       tdewalletbackend.cc


libtdewalletbackend_la_METASOURCES = AUTO

noinst_HEADERS = blowfishtables.h	\
		 cbc.h			\
		 sha1.h			\
		 blockcipher.h		\
		 tdewalletentry.h		\
		 tdewalletbackend.h	\
		 blowfish.h

