/*
Copyright (c) 2002 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DCOPTYPES_H
#define DCOPTYPES_H

// generic template fallback for unknown types
template <class T> inline const char* dcopTypeName( const T& ) { return "<unknown>"; }

#include <dcopref.h>

// standard c/c++ types
inline const char* dcopTypeName( bool ) { return "bool"; }
inline const char* dcopTypeName( char ) { return "char"; }
inline const char* dcopTypeName( uchar ) { return "uchar"; }
inline const char* dcopTypeName( int ) { return "int"; }
inline const char* dcopTypeName( uint ) { return "uint"; }
inline const char* dcopTypeName( long ) { return "long int"; }
inline const char* dcopTypeName( ulong ) { return "ulong"; }
inline const char* dcopTypeName( double ) { return "double"; }
inline const char* dcopTypeName( float ) { return "float"; }
inline const char* dcopTypeName( const char* ) { return TQCSTRING_OBJECT_NAME_STRING; }

// dcop specialities
class DCOPRef; inline const char* dcopTypeName( const DCOPRef& ) { return "DCOPRef"; }

// Qt variant types
class TQString; inline const char* dcopTypeName( const TQString& ) { return TQSTRING_OBJECT_NAME_STRING; }
class TQCString; inline const char* dcopTypeName( const TQCString& ) { return TQCSTRING_OBJECT_NAME_STRING; }
class TQFont; inline const char* dcopTypeName( const TQFont& ) { return TQFONT_OBJECT_NAME_STRING; }
class TQPixmap; inline const char* dcopTypeName( const TQPixmap& ) { return TQPIXMAP_OBJECT_NAME_STRING; }
class TQBrush; inline const char* dcopTypeName( const TQBrush& ) { return TQBRUSH_OBJECT_NAME_STRING; }
class TQRect; inline const char* dcopTypeName( const TQRect& ) { return TQRECT_OBJECT_NAME_STRING; }
class TQPoint; inline const char* dcopTypeName( const TQPoint& ) { return TQPOINT_OBJECT_NAME_STRING; }
class TQImage; inline const char* dcopTypeName( const TQImage& ) { return TQIMAGE_OBJECT_NAME_STRING; }
class TQSize; inline const char* dcopTypeName( const TQSize& ) { return TQSIZE_OBJECT_NAME_STRING; }
class TQColor; inline const char* dcopTypeName( const TQColor& ) { return TQCOLOR_OBJECT_NAME_STRING; }
class TQPalette; inline const char* dcopTypeName( const TQPalette& ) { return TQPALETTE_OBJECT_NAME_STRING; }
class TQColorGroup; inline const char* dcopTypeName( const TQColorGroup& ) { return TQCOLORGROUP_OBJECT_NAME_STRING; }
class TQIconSet; inline const char* dcopTypeName( const TQIconSet& ) { return TQICONSET_OBJECT_NAME_STRING; }
class TQDataStream; inline const char* dcopTypeName( const TQDataStream& ) { return TQDATASTREAM_OBJECT_NAME_STRING; }
class TQPointArray; inline const char* dcopTypeName( const TQPointArray& ) { return TQPOINTARRAY_OBJECT_NAME_STRING; }
class TQRegion; inline const char* dcopTypeName( const TQRegion& ) { return TQREGION_OBJECT_NAME_STRING; }
class TQBitmap; inline const char* dcopTypeName( const TQBitmap& ) { return TQBITMAP_OBJECT_NAME_STRING; }
class TQCursor; inline const char* dcopTypeName( const TQCursor& ) { return TQCURSOR_OBJECT_NAME_STRING; }
class TQStringList; inline const char* dcopTypeName( const TQStringList& ) { return TQSTRINGLIST_OBJECT_NAME_STRING; }
class TQSizePolicy; inline const char* dcopTypeName( const TQSizePolicy& ) { return TQSIZEPOLICY_OBJECT_NAME_STRING; }
class TQDate; inline const char* dcopTypeName( const TQDate& ) { return TQDATE_OBJECT_NAME_STRING; }
class TQTime; inline const char* dcopTypeName( const TQTime& ) { return TQTIME_OBJECT_NAME_STRING; }
class TQDateTime; inline const char* dcopTypeName( const TQDateTime& ) { return TQDATETIME_OBJECT_NAME_STRING; }
class TQBitArray; inline const char* dcopTypeName( const TQBitArray& ) { return TQBITARRAY_OBJECT_NAME_STRING; }
class TQKeySequence; inline const char* dcopTypeName( const TQKeySequence& ) { return TQKEYSEQUENCE_OBJECT_NAME_STRING; }
class TQVariant; inline const char* dcopTypeName( const TQVariant& ) { return TQVARIANT_OBJECT_NAME_STRING; }

// And some KDE types
class KURL; inline const char* dcopTypeName( const KURL& ) { return "KURL"; }

// type initialization for standard c/c++ types
inline void dcopTypeInit(bool& b){b=false;}
inline void dcopTypeInit(char& c){c=0;}
inline void dcopTypeInit(uchar& c){c=0;}
inline void dcopTypeInit(int& i){i=0;}
inline void dcopTypeInit(uint& i){i=0;}
inline void dcopTypeInit(long& l){l=0;}
inline void dcopTypeInit(ulong& l){l=0;}
inline void dcopTypeInit(float& f){f=0;}
inline void dcopTypeInit(double& d){d=0;}
inline void dcopTypeInit(const char* s ){s=0;}

// generic template fallback for self-initializing classes
template <class T> inline void dcopTypeInit(T&){}

#endif
