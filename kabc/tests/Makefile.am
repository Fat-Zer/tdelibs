# Make sure $(all_includes) remains last!
INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc -I$(top_srcdir)/kab \
           -I$(srcdir)/../vcardparser/ -I$(srcdir)/../vcard/include \
           -I$(srcdir)/../vcard/include/generated \
           -I$(srcdir)/../vcardparser $(all_includes)
LDADD = ../libkabc.la

METASOURCES = AUTO

check_PROGRAMS = testlock testldapclient

testlock_LDFLAGS = $(all_libraries)
testlock_SOURCES = testlock.cpp

testldapclient_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testldapclient_SOURCES = testldapclient.cpp

EXTRA_PROGRAMS = testkabc testkabcdlg testdistlist bigread bigwrite testdb \
  testaddressee testaddresseelist testaddressfmt kabcargl testaddresslineedit

testkabc_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testkabc_SOURCES = testkabc.cpp

testaddressee_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testaddressee_SOURCES = testaddressee.cpp

testaddresseelist_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testaddresseelist_SOURCES = testaddresseelist.cpp

testaddressfmt_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testaddressfmt_SOURCES = testaddressfmt.cpp

testkabcdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testkabcdlg_SOURCES = testkabcdlg.cpp

testdistlist_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testdistlist_SOURCES = testdistlist.cpp

testaddresslineedit_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testaddresslineedit_SOURCES = testaddresslineedit.cpp

bigread_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
bigread_LDADD = ../libkabc.la $(top_builddir)/kabc/plugins/file/libkabc_file.la
bigread_SOURCES = bigread.cpp

bigwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
bigwrite_LDADD = ../libkabc.la $(top_builddir)/kabc/plugins/file/libkabc_file.la
bigwrite_SOURCES = bigwrite.cpp

testdb_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
testdb_SOURCES = testdb.cpp

kabcargl_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_KDECORE) $(LIB_KDEUI) -ltdefx $(LIB_KIO) -lktexteditor
kabcargl_SOURCES = kabcargl.cpp

