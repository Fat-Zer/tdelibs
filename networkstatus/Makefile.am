#SUBDIRS = networkstatustray

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kded -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kded_networkstatus.la 

kded_networkstatus_la_SOURCES = networkstatus.cpp networkstatus.skel \
								network.cpp
kded_networkstatus_la_LIBADD = $(LIB_KDECORE) $(LIB_KIO) ./libnetworkstatus.la
kded_networkstatus_la_LDFLAGS = $(all_libraries) -module -avoid-version 

servicesdir = $(kde_servicesdir)/kded

services_DATA = networkstatus.desktop

lib_LTLIBRARIES = libnetworkstatus.la libconnectionmanager.la

libnetworkstatus_la_LIBADD = $(LIB_KDECORE)
libnetworkstatus_la_LDFLAGS = $(all_libraries)
libnetworkstatus_la_SOURCES = networkstatuscommon.cpp

libconnectionmanager_la_LIBADD = $(LIB_KDECORE) libnetworkstatus.la
libconnectionmanager_la_LDFLAGS = $(all_libraries)
libconnectionmanager_la_SOURCES = connectionmanager.cpp connectionmanager_p.cpp networkstatusindicator.cpp connectionmanager.skel networkstatusiface.stub

noinst_PROGRAMS = networkstatustestservice networkstatustestclient managedconnectiontestclient

networkstatustestservice_LDFLAGS =  $(all_libraries)
networkstatustestservice_LDADD = $(LIB_KFILE) libnetworkstatus.la
networkstatustestservice_SOURCES = testservice.cpp testserviceview.ui networkstatusiface.stub

networkstatustestclient_LDFLAGS = $(all_libraries)
networkstatustestclient_LDADD   = $(LIB_KFILE) libnetworkstatus.la libconnectionmanager.la 
networkstatustestclient_SOURCES = testclient.cpp testclientview.ui 

managedconnectiontestclient_LDFLAGS = $(all_libraries)
managedconnectiontestclient_LDADD   = $(LIB_KFILE) libnetworkstatus.la libconnectionmanager.la 
managedconnectiontestclient_SOURCES = testclient2.cpp testclientview.ui 

noinst_HEADERS = network.h testservice.h testclient.h

include_HEADERS = networkstatuscommon.h connectionmanager.h networkstatusindicator.h \
				  networkstatusiface.h
