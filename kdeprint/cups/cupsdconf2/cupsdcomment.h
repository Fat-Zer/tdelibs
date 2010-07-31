/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef CUPSDCOMMENT_H
#define	CUPSDCOMMENT_H

#include <tqstringlist.h>
#include <tqdict.h>

class QFile;

class Comment
{
public:
        bool load(TQFile* f);
        TQString toolTip();
        TQString comment();
	TQString key();
private:
        TQString comment_;
        TQString example_;
	TQString key_;
};

class CupsdComment
{
public:
	TQString operator[] (const TQString& key);
        TQString comment(const TQString& key);
        TQString toolTip(const TQString& key);

private:
	bool loadComments();

private:
	TQDict<Comment> comments_;
};

#endif
