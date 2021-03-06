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

noinst_LTLIBRARIES = libtdehtmlrender.la
libtdehtmlrender_la_SOURCES = \
	bidi.cpp break_lines.cpp render_block.cpp render_inline.cpp \
	render_style.cpp render_object.cpp render_container.cpp render_box.cpp \
	render_flow.cpp render_text.cpp render_arena.cpp render_layer.cpp \
	render_image.cpp render_table.cpp table_layout.cpp \
	render_replaced.cpp render_form.cpp render_list.cpp \
	render_canvas.cpp render_frames.cpp render_br.cpp \
	render_body.cpp font.cpp render_line.cpp render_generated.cpp \
	enumerate.cpp counter_tree.cpp

libtdehtmlrender_la_METASOURCES = AUTO

noinst_HEADERS = \
	bidi.h break_lines.h \
	render_arena.h render_layer.h \
	render_style.h render_object.h render_container.h render_box.h \
	render_flow.h render_text.h render_table.h render_replaced.h \
	render_form.h render_list.h render_canvas.h render_frames.h \
	render_br.h render_applet.h font.h table_layout.h render_line.h \
	render_generated.h enumerate.h

INCLUDES = -I$(top_srcdir)/kimgio -I$(top_srcdir)/tdeio -I$(top_srcdir)/dcop \
 -I$(top_srcdir)/tdefile -I$(top_srcdir)/tdehtml -I$(top_srcdir)/tdeutils -I$(top_srcdir) $(all_includes)

SRCDOC_DEST=$(kde_htmldir)/en/tdelibs/tdehtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) tdecore -lqt

## maintainer: regen loading icon
loading-icon:
	bin2c -sploading_icon $(srcdir)/img-loading.png > $(srcdir)/loading_icon.cpp

