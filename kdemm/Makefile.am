#    This file is part of the KDE libraries
#    Copyright (C) 2004 Matthias Kretz <kretz@kde.org>

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License version 2 as published by the Free Software Foundation.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

INCLUDES = -I$(top_srcdir)/kdecore -I$(top_srcdir)/kio $(all_includes)

SUBDIRS = . test

lib_LTLIBRARIES = libkdemm.la

kdemmincludedir = $(includedir)/kdemm
kdemminclude_HEADERS = backend.h factory.h player.h channel.h videoplayer.h mixeriface.h simpleplayer.h

libkdemm_la_SOURCES = backend.cpp factory.cpp player.cpp channel.cpp videoplayer.cpp mixeriface.skel simpleplayer.cpp factory.skel

libkdemm_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_MT_LDFLAGS) $(USER_LDFLAGS) -version-info 0:0:0 -no-undefined
libkdemm_la_LIBADD = ../kdecore/libkdecore.la ../kio/libkio.la

factory_DCOPIDLNG = true

METASOURCES = AUTO

kde_servicetypes_DATA = mmbackendinfo.desktop

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdemm

DOXYGEN_REFERENCES = dcop kdecore kio

include ../admin/Doxyfile.am

