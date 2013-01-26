
INCLUDES =  -I$(srcdir)/.. $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

libtdemidincludedir = $(includedir)/libtdemid
libtdemidinclude_HEADERS = midiout.h player.h track.h midimapper.h \
	midfile.h dattypes.h midistat.h deviceman.h synthout.h \
	fmout.h gusout.h alsaout.h voiceman.h notearray.h mt32togm.h \
	midispec.h libtdemid.h

lib_LTLIBRARIES = libtdemid.la
libtdemid_la_SOURCES  = midiout.cc player.cc track.cc midimapper.cc \
	midfile.cc dattypes.cc midistat.cc deviceman.cc synthout.cc \
	fmout.cc gusout.cc alsaout.cc voiceman.cc mt32togm.cc notearray.cc \
	libtdemid.cc

libtdemid_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 0:95  -no-undefined
libtdemid_la_LIBADD = $(LIBASOUND) ../tdecore/libtdecore.la $(LIB_QT)

DOXYGEN_REFERENCES = tdecore
include ../admin/Doxyfile.am
