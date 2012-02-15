#ifndef __kdatastream__h
#define __kdatastream__h

#include <tqdatastream.h>

#ifdef USE_QT3
inline TQDataStream & operator << (TQDataStream & str, bool b)
{
  str << TQ_INT8(b);
  return str;
}

inline TQDataStream & operator >> (TQDataStream & str, bool & b)
{
  TQ_INT8 l;
  str >> l;
  b = bool(l);
  return str;
}
#endif // USE_QT3

#if TQT_VERSION < 0x030200 && !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
inline TQDataStream & operator << (TQDataStream & str, long long int ll)
{
  TQ_UINT32 l1,l2;
  l1 = ll & 0xffffffffLL;
  l2 = ll >> 32;
  str << l1 << l2;
  return str;
}

inline TQDataStream & operator >> (TQDataStream & str, long long int&ll)
{
  TQ_UINT32 l1,l2;
  str >> l1 >> l2;
  ll = ((unsigned long long int)(l2) << 32) + (long long int) l1;
  return str;
}

inline TQDataStream & operator << (TQDataStream & str, unsigned long long int ll)
{
  TQ_UINT32 l1,l2;
  l1 = ll & 0xffffffffLL;
  l2 = ll >> 32;
  str << l1 << l2;
  return str;
}

inline TQDataStream & operator >> (TQDataStream & str, unsigned long long int &ll)
{
  TQ_UINT32 l1,l2;
  str >> l1 >> l2;
  ll = ((unsigned long long int)(l2) << 32) + (unsigned long long int) l1;
  return str;
}
#endif

#endif
