// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/*
 * client.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef KSPELL_CLIENT_H
#define KSPELL_CLIENT_H

#include <tqobject.h>
#include <tqstringlist.h>
#include <tqstring.h>

#include <kdelibs_export.h>

namespace KSpell2
{
    class Dictionary;

    /**
     * The fact that this class inherits from TQObject makes me
     * hugely unhappy. The reason for as of writting is that
     * I don't really feel like writting my own KLibFactory
     * that would load anything else then TQObject derivatives.
     */
    class KDE_EXPORT Client : public TQObject
    {
        Q_OBJECT
    public:
        Client( TQObject *parent = 0, const char *name=0 );

        virtual int reliability() const = 0;

        virtual Dictionary* dictionary( const TQString& language ) =0;

        virtual TQStringList languages() const =0;

        virtual TQString name() const =0;
    };
}

#endif
