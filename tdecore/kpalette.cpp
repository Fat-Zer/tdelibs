/* This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//-----------------------------------------------------------------------------
// KDE color palette 

#include "kpalette.h"

#include <tqfile.h>
#include <tqtextstream.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <ksavefile.h>
#include <kstringhandler.h>

template class TQPtrList<KPalette::kolor>;

TQStringList
KPalette::getPaletteList()
{
  TQStringList paletteList;
  KGlobal::dirs()->findAllResources("config", "colors/*", false, true, paletteList);

  int strip = strlen("colors/");
  for(TQStringList::Iterator it = paletteList.begin();
      it != paletteList.end();
      it++)
  { 
      (*it) = (*it).mid(strip); 
  }

  return paletteList;
}

KPalette::KPalette(const TQString &name)
 : mName(name)
{
  mKolorList.setAutoDelete(true);
  if (mName.isEmpty()) return;

  TQString filename = locate("config", "colors/"+mName);
  if (filename.isEmpty()) return;

  TQFile paletteFile(filename);
  if (!paletteFile.exists()) return;
  if (!paletteFile.open(IO_ReadOnly)) return;

  uint maxLength = 1024;
  TQString line;

  // Read first line
  // Expected "GIMP Palette"
  if (paletteFile.readLine(line, maxLength) == -1) return;
  if (line.find(" Palette") == -1) return;

  while( paletteFile.readLine(line, maxLength) != -1)
  {
     if (line[0] == '#') 
     {
        // This is a comment line
        line = line.mid(1); // Strip '#' 
        line = line.stripWhiteSpace(); // Strip remaining white space..
        if (!line.isEmpty())
        {
           mDesc += line+"\n"; // Add comment to description
        }
     }
     else
     {
        // This is a color line, hopefully
        line = line.stripWhiteSpace();
        if (line.isEmpty()) continue;
        int red, green, blue;
        int pos = 0;
        if (sscanf(line.ascii(), "%d %d %d%n", &red, &green, &blue, &pos) >= 3)
        {
           if (red > 255) red = 255;
           if (red < 0) red = 0;	
           if (green > 255) green = 255;
           if (green < 0) green = 0;	
           if (blue > 255) blue = 255;
           if (blue < 0) blue = 0;	
           kolor *node = new kolor();
           node->color.setRgb(red, green, blue);
           node->name = line.mid(pos).stripWhiteSpace();
           if (node->name.isNull()) node->name = "";
           mKolorList.append( node );
        }
     }
  }
}

KPalette::KPalette(const KPalette &p)
 : mName(p.mName), mDesc(p.mDesc), mEditable(p.mEditable)
{
   mKolorList.setAutoDelete(true);
   // Make a deep copy of the color list
   // We can't iterate a const list :(
   // DF: yes you can - use the proper iterator, not first/next
   TQPtrList<kolor> *nonConstList = (TQPtrList<kolor> *) &p.mKolorList;
   for(kolor *node = nonConstList->first(); node; node = nonConstList->next())
   {
       mKolorList.append(new kolor(*node));
   }
}

KPalette::~KPalette()
{
  // Need auto-save?
}

bool
KPalette::save()
{
   TQString filename = locateLocal("config", "colors/"+mName);
   KSaveFile sf(filename);
   if (sf.status() != 0) return false;

   TQTextStream *str = sf.textStream();

   TQString description = mDesc.stripWhiteSpace();
   description = "#"+TQStringList::split("\n", description, true).join("\n#");

   (*str) << "KDE RGB Palette\n";   
   (*str) << description << "\n";
   // We can't iterate a const list :(
   // DF: yes you can - use the proper iterator, not first/next
   TQPtrList<kolor> *nonConstList = (TQPtrList<kolor> *) (&mKolorList);
   for(kolor *node = nonConstList->first(); node; node = nonConstList->next())
   {
       int r,g,b;
       node->color.rgb(&r, &g, &b);
       (*str) << r << " " << g << " " << b << " " << node->name << "\n";
   }
   return sf.close();
}


KPalette&
KPalette::operator=( const KPalette &p)
{
  if (&p == this) return *this;
  mKolorList.clear();
  // Make a deep copy of the color list
  // We can't iterate a const list :(
   // DF: yes you can - use the proper iterator, not first/next
  TQPtrList<kolor> *nonConstList = (TQPtrList<kolor> *) &p.mKolorList;
  for(kolor *node = nonConstList->first(); node; node = nonConstList->next())
  {
     mKolorList.append(new kolor(*node));
  }
  mName = p.mName;
  mDesc = p.mDesc;
  mEditable = p.mEditable; 
  return *this;
}

TQColor
KPalette::color(int index) 
{
  if ((index < 0) || (index >= nrColors()))
	return TQColor();

  kolor *node = mKolorList.tqat(index);
  if (!node)
	return TQColor();

  return node->color;
}

int
KPalette::findColor(const TQColor &color) const
{
  int index;
  TQPtrListIterator<kolor> it( mKolorList );
  for (index = 0; it.current(); ++it, ++index)
  {
     if (it.current()->color == color)
         return index;
  }
  return -1;
}

TQString
KPalette::colorName(int index) 
{
  if ((index < 0) || (index >= nrColors()))
	return TQString::null;

  kolor *node = mKolorList.tqat(index);
  if (!node)
	return TQString::null;

  return node->name;
}

int
KPalette::addColor(const TQColor &newColor, const TQString &newColorName)
{
  kolor *node = new kolor();
  node->color = newColor;
  node->name = newColorName;
  mKolorList.append( node );
  return nrColors()-1;
}

int
KPalette::changeColor(int index, 
                      const TQColor &newColor, 
                      const TQString &newColorName)
{
  if ((index < 0) || (index >= nrColors()))
	return -1;

  kolor *node = mKolorList.tqat(index);
  if (!node)
	return -1;

  node->color = newColor;
  node->name = newColorName;
  return index;
}
