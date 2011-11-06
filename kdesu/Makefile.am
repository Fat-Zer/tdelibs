## Makefile.am for libtdesu

INCLUDES = -I$(top_srcdir)/kio/ $(all_includes)

lib_LTLIBRARIES = libtdesu.la
libtdesu_la_SOURCES = client.cpp process.cpp kcookie.cpp su.cpp ssh.cpp stub.cpp kdesu_pty.cpp
libtdesu_la_LDFLAGS = -version-info 6:0:2 -no-undefined $(all_libraries) 
libtdesu_la_LIBADD = $(LIB_KDECORE) $(LIB_QT) $(top_builddir)/dcop/libDCOP.la
libtdesu_la_NMCHECK = $(srcdir)/libtdesu.nmcheck
libtdesu_la_NMCHECKWEAK = $(srcdir)/libtdesu_weak.nmcheck $(top_srcdir)/kdecore/libtdecore_weak.nmcheck \
	$(top_srcdir)/dcop/libDCOP_weak.nmcheck $(top_srcdir)/kdecore/libqt-mt_weak.nmcheck

kdesudir = $(includedir)/kdesu
kdesu_HEADERS = defaults.h client.h process.h kdesu_pty.h kcookie.h su.h ssh.h stub.h

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(kdesudir)
	-rm -f $(DESTDIR)$(kdesudir)/pty.h
	ln -s kdesu_pty.h $(DESTDIR)$(kdesudir)/pty.h

uninstall-local:
	-rm -f $(DESTDIR)$(kdesudir)/pty.h

bin_PROGRAMS = kdesu_stub
kdesu_stub_SOURCES = kdesu_stub.c
kdesu_stub_LDFLAGS = $(all_libraries)
kdesu_stub_LDADD =

include $(top_srcdir)/admin/Doxyfile.am

