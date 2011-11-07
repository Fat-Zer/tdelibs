SUBDIRS = example tests

AM_CPPFLAGS = -I$(top_srcdir)/tdecore -I$(top_srcdir) $(all_includes)

bin_PROGRAMS = kconfig_compiler

kconfig_compiler_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -lktexteditor
kconfig_compiler_LDADD = $(LIB_TDECORE)
kconfig_compiler_SOURCES = kconfig_compiler.cpp

TESTFILES = test1.kcfg test2.kcfg test3.kcfg test4.kcfg test_dpointer.kcfg

check-local:
	for i in $(TESTFILES); \
	  do xmllint --noout --schema $(srcdir)/kcfg.xsd $(srcdir)/tests/$$i; \
	  perl $(top_srcdir)/tdecore/kconfig_compiler/checkkcfg.pl \
          $(top_srcdir)/tdecore/kconfig_compiler/tests/$$i; done
	  
