/* This file is part of the KDE libraries
   Copyright (C) 2003 Chris Howells (howells@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPLASHSCREEN_H
#define KSPLASHSCREEN_H

#include <tqsplashscreen.h>

#include <tdelibs_export.h>

class TQPixmap;

/**
 *  @short %KDE splash screen
 *
 *  This class is based on TQSplashScreen and exists solely to make
 *  splash screens obey KDE's Xinerama settings.
 *
 *  For documentation on how to use the class, see the documentation
 *  for TQSplashScreen.
 *
 *  @author Chris Howells (howells@kde.org)
 *  @since 3.2
 */
class TDEUI_EXPORT KSplashScreen : public TQSplashScreen
{
  Q_OBJECT

public:

  /**
   *  Constructs a splash screen.
   */
  KSplashScreen(const TQPixmap &pixmap, WFlags f = 0);

  /**
   *  Destructor.
   *
   *  Deletes all internal objects.
   */
  ~KSplashScreen();

};

#endif //KSPLASHSCREEN_H

