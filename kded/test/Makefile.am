# $Id$

INCLUDES= -I$(srcdir)/.. $(all_includes) 

####### Files

kde_module_LTLIBRARIES = kded_test.la

kded_test_la_SOURCES = test.cpp test.skel
kded_test_la_METASOURCES = AUTO
kded_test_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_test_la_LIBADD = $(LIB_TDECORE)

servicesdir = $(kde_servicesdir)/kded
services_DATA = test.desktop

