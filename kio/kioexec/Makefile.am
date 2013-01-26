AM_CPPFLAGS = $(all_includes)

bin_PROGRAMS = kioexec

kioexec_SOURCES = main.cpp
kioexec_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
kioexec_LDADD   = $(LIB_KIO)

noinst_HEADERS = main.h
METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kioexec_SOURCES) -o $(podir)/kioexec.pot

install-exec-local:
	@rm -f $(DESTDIR)$(bindir)/kfmexec
	@$(LN_S) kioexec $(DESTDIR)$(bindir)/kfmexec

include $(top_srcdir)/admin/Doxyfile.am
