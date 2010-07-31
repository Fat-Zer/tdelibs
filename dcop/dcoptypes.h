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
inline const char* dcopTypeName( const char* ) { return "TQCString"; }

// dcop specialities
class DCOPRef; inline const char* dcopTypeName( const DCOPRef& ) { return "DCOPRef"; }

// Qt variant types
class QString; inline const char* dcopTypeName( const TQString& ) { return "TQString"; }
class QCString; inline const char* dcopTypeName( const TQCString& ) { return "TQCString"; }
class QFont; inline const char* dcopTypeName( const TQFont& ) { return "TQFont"; }
class QPixmap; inline const char* dcopTypeName( const TQPixmap& ) { return "TQPixmap"; }
class QBrush; inline const char* dcopTypeName( const TQBrush& ) { return "TQBrush"; }
class QRect; inline const char* dcopTypeName( const TQRect& ) { return "TQRect"; }
class QPoint; inline const char* dcopTypeName( const TQPoint& ) { return "TQPoint"; }
class QImage; inline const char* dcopTypeName( const TQImage& ) { return "TQImage"; }
class QSize; inline const char* dcopTypeName( const TQSize& ) { return "TQSize"; }
class QColor; inline const char* dcopTypeName( const TQColor& ) { return "TQColor"; }
class QPalette; inline const char* dcopTypeName( const TQPalette& ) { return "TQPalette"; }
class QColorGroup; inline const char* dcopTypeName( const TQColorGroup& ) { return "TQColorGroup"; }
class QIconSet; inline const char* dcopTypeName( const TQIconSet& ) { return "TQIconSet"; }
class QDataStream; inline const char* dcopTypeName( const TQDataStream& ) { return "TQDataStream"; }
class QPointArray; inline const char* dcopTypeName( const TQPointArray& ) { return "TQPointArray"; }
class QRegion; inline const char* dcopTypeName( const TQRegion& ) { return "TQRegion"; }
class QBitmap; inline const char* dcopTypeName( const TQBitmap& ) { return "TQBitmap"; }
class QCursor; inline const char* dcopTypeName( const TQCursor& ) { return "TQCursor"; }
class QStringList; inline const char* dcopTypeName( const TQStringList& ) { return "TQStringList"; }
class QSizePolicy; inline const char* dcopTypeName( const TQSizePolicy& ) { return "TQSizePolicy"; }
class QDate; inline const char* dcopTypeName( const TQDate& ) { return "TQDate"; }
class QTime; inline const char* dcopTypeName( const TQTime& ) { return "TQTime"; }
class QDateTime; inline const char* dcopTypeName( const TQDateTime& ) { return "TQDateTime"; }
class QBitArray; inline const char* dcopTypeName( const TQBitArray& ) { return "TQBitArray"; }
class QKeySequence; inline const char* dcopTypeName( const TQKeySequence& ) { return "TQKeySequence"; }
class QVariant; inline const char* dcopTypeName( const TQVariant& ) { return "TQVariant"; }

// And some KDE types
class KURL; inline const char* dcopTypeName( const KURL& ) { return "KURL"; }

// generic template fallback for unknown types
template <class T> inline const char* dcopTypeName( const T& ) { return "<unknown>"; }

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
