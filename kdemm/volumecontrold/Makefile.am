#    This file is part of the KDE project
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
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

INCLUDES = -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kded_volumecontrold.la

kded_volumecontrold_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_volumecontrold_la_LIBADD = ../libkdemm.la
kded_volumecontrold_la_SOURCES = volumecontrold.cpp volumecontrold.skel

METASOURCES = AUTO

noinst_HEADERS = volumecontrold.h

services_DATA = volumecontrold.desktop
servicesdir = $(kde_servicesdir)/kded
