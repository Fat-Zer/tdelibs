#    This file is part of the KDE libraries
#    Copyright (C) 1997 Martin Jones (mjones@kde.org)
#              (C) 1997 Torben Weis (weis@kde.org)

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

KDE_CXXFLAGS = $(WOVERLOADED_VIRTUAL)

noinst_LTLIBRARIES = libtdehtmlmisc.la
libtdehtmlmisc_la_SOURCES = \
	decoder.cpp    loader.cpp loader_jpeg.cpp guess_ja.cpp\
	htmlhashes.cpp helper.cpp arena.cpp stringit.cpp knsplugininstaller.cpp
libtdehtmlmisc_la_LIBADD = $(LIBJPEG)
libtdehtmlmisc_la_LDFLAGS = $(USER_LDFLAGS)
libtdehtmlmisc_la_METASOURCES = AUTO

noinst_HEADERS = \
	decoder.h tdehtmllayout.h loader_jpeg.h loader.h guess_ja.h\
	stringit.h htmlhashes.h helper.h shared.h arena.h knsplugininstaller.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/dcop -I$(top_srcdir)/tdeio -I$(top_srcdir)/libltdl \
           -I$(top_srcdir)/tdehtml -I$(top_srcdir)/tdeutils $(all_includes)

SRCDOC_DEST=$(kde_htmldir)/en/tdelibs/tdehtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) tdecore -lqt

