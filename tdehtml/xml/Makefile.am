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

noinst_LTLIBRARIES = libtdehtmlxml.la
libtdehtmlxml_la_SOURCES = \
	dom_docimpl.cpp      dom_nodeimpl.cpp     dom_textimpl.cpp \
	dom_elementimpl.cpp  dom_stringimpl.cpp   dom2_rangeimpl.cpp \
	dom2_traversalimpl.cpp xml_tokenizer.cpp  dom_xmlimpl.cpp \
	dom2_eventsimpl.cpp  dom2_viewsimpl.cpp	  dom_restyler.cpp

#libtdehtmlxml_la_LDFLAGS = -no-undefined
libtdehtmlxml_la_METASOURCES = AUTO

noinst_HEADERS = \
	dom_docimpl.h      dom_nodeimpl.h     dom_textimpl.h \
	dom_elementimpl.h  dom_stringimpl.h   dom2_rangeimpl.h \
	dom2_traversalimpl.h xml_tokenizer.h  dom_xmlimpl.h \
	dom2_eventsimpl.h  dom2_viewsimpl.h dom_restyler.h

INCLUDES = -I$(top_srcdir)/kimgio -I$(top_srcdir)/tdeio -I$(top_srcdir)/dcop \
 -I$(top_srcdir)/tdehtml -I$(top_srcdir) -I$(top_srcdir)/tdewallet/client \
 -I$(top_srcdir)/tdeutils -I$(top_builddir)/kjs $(all_includes)

SRCDOC_DEST=$(kde_htmldir)/en/tdelibs/tdehtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) tdecore -lqt

