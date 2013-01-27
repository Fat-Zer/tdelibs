INCLUDES = $(all_includes)

noinst_HEADERS = plastitdeconf.h
kde_module_LTLIBRARIES = kstyle_plastik_config.la
kstyle_plastik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kstyle_plastik_config_la_LIBADD  = $(LIB_TDEUI) $(LIB_QT) $(LIB_TDECORE)
kstyle_plastik_config_la_SOURCES = plastitdeconf.cpp
kstyle_plastik_config_la_METASOURCES = AUTO


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_plastik_config.pot
