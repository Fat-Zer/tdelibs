#    This file is part of the KDE libraries
#    Copyright (C) 1997 Torben Weis (weis@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

AM_CPPFLAGS = -D_LARGEFILE64_SOURCE

INCLUDES=  -I$(top_srcdir) -I$(srcdir)/.. -I$(top_srcdir)/tdecore/network -I$(srcdir)/../kssl -I../kssl -I$(srcdir)/../../interfaces  $(all_includes) $(SSL_INCLUDES)

noinst_LTLIBRARIES = libtdeiocore.la libtdesycoca.la

# convenience lib - no LDFLAGS or LIBADD !

libtdesycoca_la_SOURCES = \
	kdirwatch.cpp \
	tdefileshare.cpp ksambashare.cpp knfsshare.cpp \
	ktrader.cpp ktraderparse.cpp ktraderparsetree.cpp \
        kservicetypefactory.cpp kservicetype.cpp \
	kmimetype.cpp kservicegroup.cpp \
	kservice.cpp kservicefactory.cpp \
	kuserprofile.cpp kservicegroupfactory.cpp \
	kurifilter.cpp \
	kfilterbase.cpp kfilterdev.cpp \
	kshellcompletion.cpp kurlcompletion.cpp \
	kautomount.cpp krun.cpp \
	tdefileitem.cpp kdirlister.cpp kimageio.cpp \
	yacc.c lex.c  \
	chmodjob.cpp kscan.cpp kar.cpp ktar.cpp kzip.cpp previewjob.cpp metainfojob.cpp davjob.cpp \
	kdatatool.cpp karchive.cpp tdefilefilter.cpp \
	tdefilemetainfo.cpp kdcopservicestarter.cpp \
	dataslave.cpp dataprotocol.cpp 
#if USE_POSIX_ACL
  libtdesycoca_la_SOURCES += kacl.cpp posixacladdons.cpp
#endif

include_HEADERS = \
	kservicetype.h kmimetype.h kmimemagic.h kservice.h \
	krun.h kdirwatch.h kautomount.h kuserprofile.h \
	kshred.h kar.h ktar.h kzip.h ktrader.h kurifilter.h kurlcompletion.h \
	kshellcompletion.h tdefileitem.h tdefileshare.h ksambashare.h knfsshare.h \
    kdirlister.h kservicegroup.h \
	kimageio.h kdirnotify.h kdirnotify_stub.h \
	kurlpixmapprovider.h kprotocolinfo.h tdeprotocolmanager.h \
	kfilterbase.h kfilterdev.h tdeemailsettings.h kscan.h kdatatool.h \
	karchive.h tdefilefilter.h tdefilemetainfo.h renamedlgplugin.h \
	kmimetyperesolver.h kdcopservicestarter.h kremoteencoding.h \
	kmimetypechooser.h 
#if USE_POSIX_ACL
include_HEADERS += kacl.h
#endif

#libtdeiocore_la_LDFLAGS = $(all_libraries)
#libtdeiocore_la_LIBADD = ../../tdeui/libtdeui.la ../../tdesu/libtdesu.la $(LIBZ) $(LIBFAM) $(LIBVOLMGT)

libtdeiocore_la_SOURCES = authinfo.cpp \
	kshred.cpp \
	tdeprotocolmanager.cpp \
	slave.cpp slaveinterface.cpp observer.stub \
	sessiondata.cpp	scheduler.cpp \
	connection.cpp \
	job.cpp global.cpp \
	slaveconfig.cpp kurlpixmapprovider.cpp \
	netaccess.cpp paste.cpp pastedialog.cpp \
	kmimemagic.cpp \
	tcpslavebase.cpp slavebase.cpp passdlg.cpp \
	forwardingslavebase.cpp \
	progressbase.cpp defaultprogress.cpp \
	statusbarprogress.cpp \
	kdirnotify.cpp kdirnotify.skel kdirnotify_stub.cpp \
	observer.cpp uiserver.stub observer.skel \
	tdeemailsettings.cpp  \
	kprotocolinfo.cpp \
	renamedlg.cpp skipdlg.cpp kremoteencoding.cpp \
	kmimetypechooser.cpp

uiserver_DIR = $(top_srcdir)/tdeio/misc

METASOURCES = AUTO

tdeioincludedir = $(includedir)/tdeio
tdeioinclude_HEADERS = connection.h \
        slaveinterface.h slave.h slaveconfig.h \
        sessiondata.h global.h passdlg.h \
	netaccess.h job.h scheduler.h \
        jobclasses.h paste.h slavebase.h \
	progressbase.h defaultprogress.h \
	statusbarprogress.h tcpslavebase.h \
	forwardingslavebase.h \
	observer.h chmodjob.h uiserver_stub.h \
        kmdbase.h authinfo.h \
	ioslave_defaults.h http_slave_defaults.h previewjob.h thumbcreator.h \
	metainfojob.h davjob.h renamedlg.h skipdlg.h

# Internal
noinst_HEADERS = kservicetypefactory.h kservicefactory.h \
	kmessageboxwrapper.h \
	ktraderparse.h ktraderparsetree.h yacc.h \
	kimageiofactory.h kdirwatch_p.h kdirlister_p.h \
	renamedlg.h skipdlg.h dataslave.h dataprotocol.h \
	kservice_p.h
#if USE_POSIX_ACL
noinst_HEADERS += posixacladdons.h
#endif

parserfiles = yacc.y lex.l

EXTRA_DIST = $(parserfiles)

parser: $(parserfiles)
	cd $(srcdir) ;\
	flex -olex.c -Pkiotrader lex.l ;\
	bison -d -p kiotrader yacc.y && mv yacc.tab.c yacc.c; mv yacc.tab.h yacc.h

.PHONY: parser

include ../../admin/Doxyfile.am
