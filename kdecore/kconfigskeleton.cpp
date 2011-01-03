/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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

#include <tqcolor.h>
#include <tqvariant.h>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include "kstringhandler.h"

#include "kconfigskeleton.h"

void KConfigSkeletonItem::readImmutability( KConfig *config )
{
  mIsImmutable = config->entryIsImmutable( mKey );
}


KConfigSkeleton::ItemString::ItemString( const TQString &group, const TQString &key,
                                    TQString &reference,
                                    const TQString &defaultValue,
                                    Type type )
  : KConfigSkeletonGenericItem<TQString>( group, key, reference, defaultValue ),
    mType( type )
{
}

void KConfigSkeleton::ItemString::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else if ( mType == Path )
      config->writePathEntry( mKey, mReference );
    else if ( mType == Password )
      config->writeEntry( mKey, KStringHandler::obscure( mReference ) );
    else
      config->writeEntry( mKey, mReference );
  }
}


void KConfigSkeleton::ItemString::readConfig( KConfig *config )
{
  config->setGroup( mGroup );

  if ( mType == Path )
  {
    mReference = config->readPathEntry( mKey, mDefault );
  }
  else if ( mType == Password ) 
  {
    TQString value = config->readEntry( mKey,
                                       KStringHandler::obscure( mDefault ) );
    mReference = KStringHandler::obscure( value );
  }
  else
  {
    mReference = config->readEntry( mKey, mDefault );
  }

  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemString::setProperty(const TQVariant & p)
{
  mReference = p.toString();
}

TQVariant KConfigSkeleton::ItemString::property() const
{
  return TQVariant(mReference);
}

KConfigSkeleton::ItemPassword::ItemPassword( const TQString &group, const TQString &key,
                                    TQString &reference,
                                    const TQString &defaultValue)
  : ItemString( group, key, reference, defaultValue, Password )
{
}

KConfigSkeleton::ItemPath::ItemPath( const TQString &group, const TQString &key,
                                    TQString &reference,
                                    const TQString &defaultValue)
  : ItemString( group, key, reference, defaultValue, Path )
{
}

KConfigSkeleton::ItemProperty::ItemProperty( const TQString &group,
                                        const TQString &key,
                                        TQVariant &reference,
                                        TQVariant defaultValue )
  : KConfigSkeletonGenericItem<TQVariant>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemProperty::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readPropertyEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemProperty::setProperty(const TQVariant & p)
{
  mReference = p;
}

TQVariant KConfigSkeleton::ItemProperty::property() const
{
  return mReference;
}

KConfigSkeleton::ItemBool::ItemBool( const TQString &group, const TQString &key,
                                bool &reference, bool defaultValue )
  : KConfigSkeletonGenericItem<bool>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemBool::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readBoolEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemBool::setProperty(const TQVariant & p)
{
  mReference = p.toBool();
}

TQVariant KConfigSkeleton::ItemBool::property() const
{
  return TQVariant( mReference, 42 /* dummy */ );
}


KConfigSkeleton::ItemInt::ItemInt( const TQString &group, const TQString &key,
                              int &reference, int defaultValue )
  : KConfigSkeletonGenericItem<int>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemInt::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = QMAX(mReference, mMin);
  if (mHasMax)
    mReference = QMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemInt::setProperty(const TQVariant & p)
{
  mReference = p.toInt();
}

TQVariant KConfigSkeleton::ItemInt::property() const
{
  return TQVariant(mReference);
}

TQVariant KConfigSkeleton::ItemInt::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant KConfigSkeleton::ItemInt::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void KConfigSkeleton::ItemInt::setMinValue(int v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemInt::setMaxValue(int v)
{
  mHasMax = true;
  mMax = v;
}


KConfigSkeleton::ItemInt64::ItemInt64( const TQString &group, const TQString &key,
                              TQ_INT64 &reference, TQ_INT64 defaultValue )
  : KConfigSkeletonGenericItem<TQ_INT64>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemInt64::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readNum64Entry( mKey, mDefault );
  if (mHasMin)
    mReference = QMAX(mReference, mMin);
  if (mHasMax)
    mReference = QMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemInt64::setProperty(const TQVariant & p)
{
  mReference = p.toLongLong();
}

TQVariant KConfigSkeleton::ItemInt64::property() const
{
  return TQVariant(mReference);
}

TQVariant KConfigSkeleton::ItemInt64::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant KConfigSkeleton::ItemInt64::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void KConfigSkeleton::ItemInt64::setMinValue(TQ_INT64 v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemInt64::setMaxValue(TQ_INT64 v)
{
  mHasMax = true;
  mMax = v;
}

KConfigSkeleton::ItemEnum::ItemEnum( const TQString &group, const TQString &key,
                                     int &reference,
                                     const TQValueList<Choice> &choices,
                                     int defaultValue )
  : ItemInt( group, key, reference, defaultValue ), mChoices( choices )
{
}

void KConfigSkeleton::ItemEnum::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if (!config->hasKey(mKey))
  {
    mReference = mDefault;
  }
  else
  {
    int i = 0;
    mReference = -1;
    TQString tmp = config->readEntry( mKey ).lower();
    for(TQValueList<Choice>::ConstIterator it = mChoices.begin();
        it != mChoices.end(); ++it, ++i)
    {
      if ((*it).name.lower() == tmp)
      {
        mReference = i;
        break;
      }
    }
    if (mReference == -1)
       mReference = config->readNumEntry( mKey, mDefault );
  }
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemEnum::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else if ((mReference >= 0) && (mReference < (int) mChoices.count()))
      config->writeEntry( mKey, mChoices[mReference].name );
    else
      config->writeEntry( mKey, mReference );
  }
}

TQValueList<KConfigSkeleton::ItemEnum::Choice> KConfigSkeleton::ItemEnum::choices() const
{
  return mChoices;
}


KConfigSkeleton::ItemUInt::ItemUInt( const TQString &group, const TQString &key,
                                unsigned int &reference,
                                unsigned int defaultValue )
  : KConfigSkeletonGenericItem<unsigned int>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemUInt::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = QMAX(mReference, mMin);
  if (mHasMax)
    mReference = QMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemUInt::setProperty(const TQVariant & p)
{
  mReference = p.toUInt();
}

TQVariant KConfigSkeleton::ItemUInt::property() const
{
  return TQVariant(mReference);
}

TQVariant KConfigSkeleton::ItemUInt::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant KConfigSkeleton::ItemUInt::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void KConfigSkeleton::ItemUInt::setMinValue(unsigned int v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemUInt::setMaxValue(unsigned int v)
{
  mHasMax = true;
  mMax = v;
}


KConfigSkeleton::ItemUInt64::ItemUInt64( const TQString &group, const TQString &key,
                              TQ_UINT64 &reference, TQ_UINT64 defaultValue )
  : KConfigSkeletonGenericItem<TQ_UINT64>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemUInt64::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedNum64Entry( mKey, mDefault );
  if (mHasMin)
    mReference = QMAX(mReference, mMin);
  if (mHasMax)
    mReference = QMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemUInt64::setProperty(const TQVariant & p)
{
  mReference = p.toULongLong();
}

TQVariant KConfigSkeleton::ItemUInt64::property() const
{
  return TQVariant(mReference);
}

TQVariant KConfigSkeleton::ItemUInt64::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant KConfigSkeleton::ItemUInt64::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void KConfigSkeleton::ItemUInt64::setMinValue(TQ_UINT64 v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemUInt64::setMaxValue(TQ_UINT64 v)
{
  mHasMax = true;
  mMax = v;
}

KConfigSkeleton::ItemLong::ItemLong( const TQString &group, const TQString &key,
                                long &reference, long defaultValue )
  : KConfigSkeletonGenericItem<long>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemLong::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readLongNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = QMAX(mReference, mMin);
  if (mHasMax)
    mReference = QMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemLong::setProperty(const TQVariant & p)
{
  mReference = p.toLongLong();
}

TQVariant KConfigSkeleton::ItemLong::property() const
{
  return TQVariant((Q_LLONG) mReference);
}

TQVariant KConfigSkeleton::ItemLong::minValue() const
{
  if (mHasMin)
    return TQVariant((Q_LLONG) mMin);
  return TQVariant();
}

TQVariant KConfigSkeleton::ItemLong::maxValue() const
{
  if (mHasMax)
    return TQVariant((Q_LLONG) mMax);
  return TQVariant();
}

void KConfigSkeleton::ItemLong::setMinValue(long v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemLong::setMaxValue(long v)
{
  mHasMax = true;
  mMax = v;
}


KConfigSkeleton::ItemULong::ItemULong( const TQString &group, const TQString &key,
                                  unsigned long &reference,
                                  unsigned long defaultValue )
  : KConfigSkeletonGenericItem<unsigned long>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemULong::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedLongNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = QMAX(mReference, mMin);
  if (mHasMax)
    mReference = QMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemULong::setProperty(const TQVariant & p)
{
  mReference = p.toULongLong();
}

TQVariant KConfigSkeleton::ItemULong::property() const
{
  return TQVariant((Q_ULLONG) mReference);
}

TQVariant KConfigSkeleton::ItemULong::minValue() const
{
  if (mHasMin)
    return TQVariant((Q_ULLONG) mMin);
  return TQVariant();
}

TQVariant KConfigSkeleton::ItemULong::maxValue() const
{
  if (mHasMax)
    return TQVariant((Q_ULLONG) mMax);
  return TQVariant();
}

void KConfigSkeleton::ItemULong::setMinValue(unsigned long v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemULong::setMaxValue(unsigned long v)
{
  mHasMax = true;
  mMax = v;
}


KConfigSkeleton::ItemDouble::ItemDouble( const TQString &group, const TQString &key,
                                    double &reference, double defaultValue )
  : KConfigSkeletonGenericItem<double>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemDouble::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readDoubleNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = QMAX(mReference, mMin);
  if (mHasMax)
    mReference = QMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemDouble::setProperty(const TQVariant & p)
{
  mReference = p.toDouble();
}

TQVariant KConfigSkeleton::ItemDouble::property() const
{
  return TQVariant(mReference);
}

TQVariant KConfigSkeleton::ItemDouble::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant KConfigSkeleton::ItemDouble::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void KConfigSkeleton::ItemDouble::setMinValue(double v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemDouble::setMaxValue(double v)
{
  mHasMax = true;
  mMax = v;
}


KConfigSkeleton::ItemColor::ItemColor( const TQString &group, const TQString &key,
                                  TQColor &reference,
                                  const TQColor &defaultValue )
  : KConfigSkeletonGenericItem<TQColor>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemColor::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readColorEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemColor::setProperty(const TQVariant & p)
{
  mReference = p.toColor();
}

TQVariant KConfigSkeleton::ItemColor::property() const
{
  return TQVariant(mReference);
}


KConfigSkeleton::ItemFont::ItemFont( const TQString &group, const TQString &key,
                                TQFont &reference,
                                const TQFont &defaultValue )
  : KConfigSkeletonGenericItem<TQFont>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemFont::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readFontEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemFont::setProperty(const TQVariant & p)
{
  mReference = p.toFont();
}

TQVariant KConfigSkeleton::ItemFont::property() const
{
  return TQVariant(mReference);
}


KConfigSkeleton::ItemRect::ItemRect( const TQString &group, const TQString &key,
                                TQRect &reference,
                                const TQRect &defaultValue )
  : KConfigSkeletonGenericItem<TQRect>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemRect::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readRectEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemRect::setProperty(const TQVariant & p)
{
  mReference = p.toRect();
}

TQVariant KConfigSkeleton::ItemRect::property() const
{
  return TQVariant(mReference);
}


KConfigSkeleton::ItemPoint::ItemPoint( const TQString &group, const TQString &key,
                                  TQPoint &reference,
                                  const TQPoint &defaultValue )
  : KConfigSkeletonGenericItem<TQPoint>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemPoint::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readPointEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemPoint::setProperty(const TQVariant & p)
{
  mReference = p.toPoint();
}

TQVariant KConfigSkeleton::ItemPoint::property() const
{
  return TQVariant(mReference);
}


KConfigSkeleton::ItemSize::ItemSize( const TQString &group, const TQString &key,
                                TQSize &reference,
                                const TQSize &defaultValue )
  : KConfigSkeletonGenericItem<TQSize>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemSize::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readSizeEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemSize::setProperty(const TQVariant & p)
{
  mReference = p.toSize();
}

TQVariant KConfigSkeleton::ItemSize::property() const
{
  return TQVariant(mReference);
}


KConfigSkeleton::ItemDateTime::ItemDateTime( const TQString &group, const TQString &key,
                                        TQDateTime &reference,
                                        const TQDateTime &defaultValue )
  : KConfigSkeletonGenericItem<TQDateTime>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemDateTime::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readDateTimeEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemDateTime::setProperty(const TQVariant & p)
{
  mReference = p.toDateTime();
}

TQVariant KConfigSkeleton::ItemDateTime::property() const
{
  return TQVariant(mReference);
}


KConfigSkeleton::ItemStringList::ItemStringList( const TQString &group, const TQString &key,
                                            TQStringList &reference,
                                            const TQStringList &defaultValue )
  : KConfigSkeletonGenericItem<TQStringList>( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemStringList::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readListEntry( mKey );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemStringList::setProperty(const TQVariant & p)
{
  mReference = p.toStringList();
}

TQVariant KConfigSkeleton::ItemStringList::property() const
{
  return TQVariant(mReference);
}


KConfigSkeleton::ItemPathList::ItemPathList( const TQString &group, const TQString &key,
                                            TQStringList &reference,
                                            const TQStringList &defaultValue )
  : ItemStringList( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemPathList::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readPathListEntry( mKey );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemPathList::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else {
      TQStringList sl = mReference;
      config->writePathEntry( mKey, sl );
    }
  }
}


KConfigSkeleton::ItemIntList::ItemIntList( const TQString &group, const TQString &key,
                                      TQValueList<int> &reference,
                                      const TQValueList<int> &defaultValue )
  : KConfigSkeletonGenericItem<TQValueList<int> >( group, key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemIntList::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readIntListEntry( mKey );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemIntList::setProperty(const TQVariant &)
{
  // TODO: Not yet supported
}

TQVariant KConfigSkeleton::ItemIntList::property() const
{
  // TODO: Not yet supported
  return TQVariant();  
}


KConfigSkeleton::KConfigSkeleton( const TQString &configname )
  : mCurrentGroup( "No Group" ), mUseDefaults(false)
{
  kdDebug(177) << "Creating KConfigSkeleton (" << (void *)this << ")" << endl;

  if ( !configname.isEmpty() )
  {
    mConfig = KSharedConfig::openConfig( configname );
  }
  else
  {
    mConfig = KGlobal::sharedConfig();
  }
}

KConfigSkeleton::KConfigSkeleton(KSharedConfig::Ptr config)
  : mCurrentGroup( "No Group" ), mUseDefaults(false)
{
  kdDebug(177) << "Creating KConfigSkeleton (" << (void *)this << ")" << endl;
  mConfig = config;
}


KConfigSkeleton::~KConfigSkeleton()
{
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    delete *it;
  }
}

void KConfigSkeleton::setCurrentGroup( const TQString &group )
{
  mCurrentGroup = group;
}

KConfig *KConfigSkeleton::config() const
{
  return mConfig;
}

bool KConfigSkeleton::useDefaults(bool b)
{
  if (b == mUseDefaults)
     return mUseDefaults;

  mUseDefaults = b;
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->swapDefault();
  }

  usrUseDefaults(b);
  return !mUseDefaults;
}

void KConfigSkeleton::setDefaults()
{
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it ) {
    (*it)->setDefault();
  }

  usrSetDefaults();
}

void KConfigSkeleton::readConfig()
{
  kdDebug(177) << "KConfigSkeleton::readConfig()" << endl;
  
  TQString origGroup = mConfig->group();

  mConfig->reparseConfiguration();
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->readConfig( mConfig );
  }

  usrReadConfig();
  
  mConfig->setGroup(origGroup);
}

void KConfigSkeleton::writeConfig()
{
  kdDebug(177) << "KConfigSkeleton::writeConfig()" << endl;

  TQString origGroup = mConfig->group();

  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->writeConfig( mConfig );
  }

  usrWriteConfig();

  mConfig->sync();

  readConfig();

  mConfig->setGroup(origGroup);
}

void KConfigSkeleton::addItem( KConfigSkeletonItem *item, const TQString &name )
{
  item->setName( name.isEmpty() ? item->key() : name );
  mItems.append( item );
  mItemDict.insert( item->name(), item );
  item->readDefault( mConfig );
  item->readConfig( mConfig );
}

KConfigSkeleton::ItemString *KConfigSkeleton::addItemString( const TQString &name, TQString &reference,
                                     const TQString &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemString *item;
  item = new KConfigSkeleton::ItemString( mCurrentGroup, key.isEmpty() ? name : key,
                                          reference, defaultValue,
                                          KConfigSkeleton::ItemString::Normal );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPassword *KConfigSkeleton::addItemPassword( const TQString &name, TQString &reference,
                                       const TQString &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemPassword *item;
  item = new KConfigSkeleton::ItemPassword( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPath *KConfigSkeleton::addItemPath( const TQString &name, TQString &reference,
                                   const TQString &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemPath *item;
  item = new KConfigSkeleton::ItemPath( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemProperty *KConfigSkeleton::addItemProperty( const TQString &name, TQVariant &reference,
                                       const TQVariant &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemProperty *item;
  item = new KConfigSkeleton::ItemProperty( mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemBool *KConfigSkeleton::addItemBool( const TQString &name, bool &reference,
                                   bool defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemBool *item;
  item = new KConfigSkeleton::ItemBool( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemInt *KConfigSkeleton::addItemInt( const TQString &name, int &reference,
                                  int defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemInt *item;
  item = new KConfigSkeleton::ItemInt( mCurrentGroup, key.isNull() ? name : key,
                                       reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemUInt *KConfigSkeleton::addItemUInt( const TQString &name, unsigned int &reference,
                                   unsigned int defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemUInt *item;
  item = new KConfigSkeleton::ItemUInt( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemInt64 *KConfigSkeleton::addItemInt64( const TQString &name, TQ_INT64 &reference,
                                    TQ_INT64 defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemInt64 *item;
  item = new KConfigSkeleton::ItemInt64( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemUInt64 *KConfigSkeleton::addItemUInt64( const TQString &name, TQ_UINT64 &reference,
                                     TQ_UINT64 defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemUInt64 *item;
  item = new KConfigSkeleton::ItemUInt64( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemLong *KConfigSkeleton::addItemLong( const TQString &name, long &reference,
                                   long defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemLong *item;
  item = new KConfigSkeleton::ItemLong( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemULong *KConfigSkeleton::addItemULong( const TQString &name, unsigned long &reference,
                                    unsigned long defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemULong *item;
  item = new KConfigSkeleton::ItemULong( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemDouble *KConfigSkeleton::addItemDouble( const TQString &name, double &reference,
                                     double defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemDouble *item;
  item = new KConfigSkeleton::ItemDouble( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemColor *KConfigSkeleton::addItemColor( const TQString &name, TQColor &reference,
                                    const TQColor &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemColor *item;
  item = new KConfigSkeleton::ItemColor( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemFont *KConfigSkeleton::addItemFont( const TQString &name, TQFont &reference,
                                   const TQFont &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemFont *item;
  item = new KConfigSkeleton::ItemFont( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemRect *KConfigSkeleton::addItemRect( const TQString &name, TQRect &reference,
                                   const TQRect &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemRect *item;
  item = new KConfigSkeleton::ItemRect( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPoint *KConfigSkeleton::addItemPoint( const TQString &name, TQPoint &reference,
                                    const TQPoint &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemPoint *item;
  item = new KConfigSkeleton::ItemPoint( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemSize *KConfigSkeleton::addItemSize( const TQString &name, TQSize &reference,
                                   const TQSize &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemSize *item;
  item = new KConfigSkeleton::ItemSize( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemDateTime *KConfigSkeleton::addItemDateTime( const TQString &name, TQDateTime &reference,
                                       const TQDateTime &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemDateTime *item;
  item = new KConfigSkeleton::ItemDateTime( mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemStringList *KConfigSkeleton::addItemStringList( const TQString &name, TQStringList &reference,
                                         const TQStringList &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemStringList *item;
  item = new KConfigSkeleton::ItemStringList( mCurrentGroup, key.isNull() ? name : key,
                                              reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemIntList *KConfigSkeleton::addItemIntList( const TQString &name, TQValueList<int> &reference,
                                      const TQValueList<int> &defaultValue, const TQString &key )
{
  KConfigSkeleton::ItemIntList *item;
  item = new KConfigSkeleton::ItemIntList( mCurrentGroup, key.isNull() ? name : key,
                                           reference, defaultValue );
  addItem( item, name );
  return item;
}

bool KConfigSkeleton::isImmutable(const TQString &name)
{
  KConfigSkeletonItem *item = tqfindItem(name);
  return !item || item->isImmutable();
}

KConfigSkeletonItem *KConfigSkeleton::tqfindItem(const TQString &name)
{
  return mItemDict.tqfind(name);
}
