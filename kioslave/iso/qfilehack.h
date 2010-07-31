/***************************************************************************
                          qfilehack.h  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002 by Szombathelyi Gy�rgy
    email                : gyurco@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QFILEHACK_H
#define QFILEHACK_H

#include <tqfile.h>
#include <tqstring.h>

/**
  *@author Szombathelyi Gy�rgy
  * Qt thinks if a file is not S_IFREG, you cannot seek in it. It's false (what about
  * block devices for example?
  */

class QFileHack : public TQFile  {
public: 
    QFileHack();
    QFileHack( const TQString & name );
    ~QFileHack();
    virtual bool open ( int m );
};

#endif
