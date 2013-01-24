/*
 * This file is part of KDE.
 * 
 * Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
 * Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KCONFIGSKELETON_H
#define _KCONFIGSKELETON_H

#include <tqcolor.h>
#include <tqdatetime.h>
#include <tqfont.h>
#include <tqpoint.h>
#include <tqptrlist.h>
#include <tqdict.h>
#include <tqrect.h>
#include <tqsize.h>
#include <tqstringlist.h>
#include <tqvariant.h>
#include <kconfig.h>
#include <kglobalsettings.h>

  /**
   * @short Class for storing a preferences setting
   * @author Cornelius Schumacher
   * @see KConfigSkeleton
   * 
   * This class represents one preferences setting as used by @ref KConfigSkeleton.
   * Subclasses of KConfigSkeletonItem implement storage functions for a certain type of
   * setting. Normally you don't have to use this class directly. Use the special
   * addItem() functions of KConfigSkeleton instead. If you subclass this class you will
   * have to register instances with the function KConfigSkeleton::addItem().
   */
  class TDECORE_EXPORT KConfigSkeletonItem
  {
  public:
    typedef TQValueList < KConfigSkeletonItem * >List;
    typedef TQDict < KConfigSkeletonItem > Dict;
    typedef TQDictIterator < KConfigSkeletonItem > DictIterator;

    /**
     * Constructor.
     * 
     * @param group Config file group.
     * @param key Config file key.
     */
    KConfigSkeletonItem(const TQString & group, const TQString & key)
      :mGroup(group),mKey(key), mIsImmutable(true)
    {
    }

    /**
     * Destructor.
     */
    virtual ~KConfigSkeletonItem()
    {
    }

    /**
     * Set config file group.
     */
    void setGroup( const TQString &group )
    {
      mGroup = group;
    }

    /**
     * Return config file group.
     */
    TQString group() const
    {
      return mGroup;
    }

    /**
     * Set config file key.
     */
    void setKey( const TQString &key )
    {
      mKey = key;
    }
    
    /**
     * Return config file key.
     */
    TQString key() const
    {
      return mKey;
    }

    /**
     * Set internal name of entry.
     */
    void setName(const TQString &name)
    {
      mName = name;
    }

    /**
     * Return internal name of entry.
     */
    TQString name() const
    {
      return mName;
    }

    /**
      Set label providing a translated one-line description of the item.
    */
    void setLabel( const TQString &l )
    {
      mLabel = l;
    }
    
    /**
      Return label of item. See setLabel().
    */
    TQString label() const
    {
      return mLabel;
    }

    /**
      Set WhatsThis description og item.
    */
    void setWhatsThis( const TQString &w )
    {
      mWhatsThis = w;
    }
    
    /**
      Return WhatsThis description of item. See setWhatsThis().
    */
    TQString whatsThis() const
    {
      return mWhatsThis;
    }

    /**
     * This function is called by @ref KConfigSkeleton to read the value for this setting
     * from a config file.
     * value.
     */
    virtual void readConfig(KConfig *) = 0;

    /**
     * This function is called by @ref KConfigSkeleton to write the value of this setting
     * to a config file.
     */
    virtual void writeConfig(KConfig *) = 0;

    /**
     * Read global default value.
     */
    virtual void readDefault(KConfig *) = 0;

    /**
     * Set item to @p p
     */
    virtual void setProperty(const TQVariant &p) = 0;
    
    /**
     * Return item as property
     */
    virtual TQVariant property() const = 0;

    /**
     * Return minimum value of item or invalid if not specified
     */
    virtual TQVariant minValue() const { return TQVariant(); }

    /**
     * Return maximum value of item or invalid if not specified
     */
    virtual TQVariant maxValue() const { return TQVariant(); }

    /**
      Sets the current value to the default value.
    */
    virtual void setDefault() = 0;

    /**
     * Exchanges the current value with the default value
     * Used by KConfigSkeleton::useDefaults(bool);
     */
    virtual void swapDefault() = 0;

    /**
     * Return if the entry can be modified.
     */
    bool isImmutable() const
    {
      return mIsImmutable;
    }

  protected:
    /**
     * sets mIsImmutable to true if mKey in config is immutable
     * @param config KConfig to check if mKey is immutable in
     */
    void readImmutability(KConfig *config);

    TQString mGroup;
    TQString mKey;
    TQString mName;

  private:
    bool mIsImmutable;

    TQString mLabel;
    TQString mWhatsThis;
  };


template < typename T > class KConfigSkeletonGenericItem:public KConfigSkeletonItem
  {
  public:
    KConfigSkeletonGenericItem(const TQString & group, const TQString & key, T & reference,
                T defaultValue)
      : KConfigSkeletonItem(group, key), mReference(reference),
        mDefault(defaultValue), mLoadedValue(defaultValue)
    {
    }

    /**
     * Set value of this KConfigSkeletonItem.
     */
    void setValue(const T & v)
    {
      mReference = v;
    }

    /**
     * Return value of this KConfigSkeletonItem.
     */
    T & value()
    {
      return mReference;
    }

    /**
     * Return const value of this KConfigSkeletonItem.
     */
    const T & value() const
    {
      return mReference;
    }

    /**
      Set default value for this item.
    */
    virtual void setDefaultValue( const T &v )
    {
      mDefault = v;
    }

    virtual void setDefault()
    {
      mReference = mDefault;
    }

    virtual void writeConfig(KConfig * config)
    {
      if ( mReference != mLoadedValue ) // Is this needed?
      {
        config->setGroup(mGroup);
        if ((mDefault == mReference) && !config->hasDefault( mKey))
          config->revertToDefault( mKey );
        else
          config->writeEntry(mKey, mReference);
      }
    }

    void readDefault(KConfig * config)
    {
      config->setReadDefaults(true);
      readConfig(config);
      config->setReadDefaults(false);
      mDefault = mReference;
    }

    void swapDefault()
    {
      T tmp = mReference;
      mReference = mDefault;
      mDefault = tmp;
    }

  protected:
    T & mReference;
    T mDefault;
    T mLoadedValue;
  };

  /**
   * @short Class for handling preferences settings for an application.
   * @author Cornelius Schumacher
   * @see KConfigSkeletonItem
   * 
   * This class provides an interface to preferences settings. Preferences items
   * can be registered by the addItem() function corresponding to the data type of
   * the seetting. KConfigSkeleton then handles reading and writing of config files and
   * setting of default values.
   * 
   * Normally you will subclass KConfigSkeleton, add data members for the preferences
   * settings and register the members in the constructor of the subclass.
   * 
   * Example:
   * \code
   * class MyPrefs : public KConfigSkeleton
   * {
   *   public:
   *     MyPrefs()
   *     {
   *       setCurrentGroup("MyGroup");
   *       addItemBool("MySetting1",mMyBool,false);
   *       addItemColor("MySetting2",mMyColor,TQColor(1,2,3));
   * 
   *       setCurrentGroup("MyOtherGroup");
   *       addItemFont("MySetting3",mMyFont,TQFont("helvetica",12));
   *     }
   * 
   *     bool mMyBool;
   *     TQColor mMyColor;
   *     TQFont mMyFont;
   * }
   * \endcode
   * 
   * It might be convenient in many cases to make this subclass of KConfigSkeleton a
   * singleton for global access from all over the application without passing
   * references to the KConfigSkeleton object around.
   * 
   * You can write the data to the configuration file by calling @ref writeConfig()
   * and read the data from the configuration file by calling @ref readConfig().
   * 
   * If you have items, which are not covered by the existing addItem() functions
   * you can add customized code for reading, writing and default setting by
   * implementing the functions @ref usrUseDefaults(), @ref usrReadConfig() and
   * @ref usrWriteConfig().
   * 
   * Internally preferences settings are stored in instances of subclasses of
   * @ref KConfigSkeletonItem. You can also add KConfigSkeletonItem subclasses 
   * for your own types and call the generic @ref addItem() to register them.
   *
   * In many cases you don't have to write the specific KConfigSkeleton
   * subclasses yourself, but you can use \ref kconfig_compiler to automatically
   * generate the C++ code from an XML description of the configuration options.
   */
class TDECORE_EXPORT KConfigSkeleton
{
public:

  /**
   * Class for handling a string preferences item.
   */
  class TDECORE_EXPORT ItemString:public KConfigSkeletonGenericItem < TQString >
  {
  public:
    enum Type { Normal, Password, Path };

    ItemString(const TQString & group, const TQString & key,
               TQString & reference,
               const TQString & defaultValue = TQString::fromLatin1(""), // NOT TQString::null !!
               Type type = Normal);

    void writeConfig(KConfig * config);
    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;

  private:
    Type mType;
  };

  /**
   * Class for handling a password preferences item.
   */
  class TDECORE_EXPORT ItemPassword:public ItemString
  {
  public:
    ItemPassword(const TQString & group, const TQString & key,
               TQString & reference,
               const TQString & defaultValue = TQString::fromLatin1("")); // NOT TQString::null !!
  };

  /**
   * Class for handling a path preferences item.
   */
  class TDECORE_EXPORT ItemPath:public ItemString
  {
  public:
    ItemPath(const TQString & group, const TQString & key,
             TQString & reference,
             const TQString & defaultValue = TQString::null);
  };


  /**
   * Class for handling a TQVariant preferences item.
   */
  class TDECORE_EXPORT ItemProperty:public KConfigSkeletonGenericItem < TQVariant >
  {
  public:
    ItemProperty(const TQString & group, const TQString & key,
                 TQVariant & reference, TQVariant defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling a bool preferences item.
   */
  class TDECORE_EXPORT ItemBool:public KConfigSkeletonGenericItem < bool >
  {
  public:
    ItemBool(const TQString & group, const TQString & key, bool & reference,
             bool defaultValue = true);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling an integer preferences item.
   */
  class TDECORE_EXPORT ItemInt:public KConfigSkeletonGenericItem < int >
  {
  public:
    ItemInt(const TQString & group, const TQString & key, int &reference,
            int defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
    TQVariant minValue() const;
    TQVariant maxValue() const;

    void setMinValue(int);
    void setMaxValue(int);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    int mMin;
    int mMax;
  };

  /**
   * Class for handling an 64-bit integer preferences item.
   */
  class TDECORE_EXPORT ItemInt64:public KConfigSkeletonGenericItem < TQ_INT64 >
  {
  public:
    ItemInt64(const TQString & group, const TQString & key, TQ_INT64 &reference,
            TQ_INT64 defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;

    TQVariant minValue() const;
    TQVariant maxValue() const;

    void setMinValue(TQ_INT64);
    void setMaxValue(TQ_INT64);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    TQ_INT64 mMin;
    TQ_INT64 mMax;
  };

  /**
   * Class for handling enums.
   */
  class TDECORE_EXPORT ItemEnum:public ItemInt
  {
  public:
    struct Choice
    {
      TQString name;
      TQString label;
      TQString whatsThis;
    };

    ItemEnum(const TQString & group, const TQString & key, int &reference,
             const TQValueList<Choice> &choices, int defaultValue = 0);

    TQValueList<Choice> choices() const;

    void readConfig(KConfig * config);
    void writeConfig(KConfig * config);

  private:
      TQValueList<Choice> mChoices;
  };


  /**
   * Class for handling an unsingend integer preferences item.
   */
  class TDECORE_EXPORT ItemUInt:public KConfigSkeletonGenericItem < unsigned int >
  {
  public:
    ItemUInt(const TQString & group, const TQString & key,
             unsigned int &reference, unsigned int defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
    TQVariant minValue() const;
    TQVariant maxValue() const;

    void setMinValue(unsigned int);
    void setMaxValue(unsigned int);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    unsigned int mMin;
    unsigned int mMax;
  };


  /**
   * Class for hanlding a long integer preferences item.
   */
  class TDECORE_EXPORT ItemLong:public KConfigSkeletonGenericItem < long >
  {
  public:
    ItemLong(const TQString & group, const TQString & key, long &reference,
             long defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
    TQVariant minValue() const;
    TQVariant maxValue() const;

    void setMinValue(long);
    void setMaxValue(long);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    long mMin;
    long mMax;
  };


  /**
   * Class for handling an unsigned long integer preferences item.
   */
  class TDECORE_EXPORT ItemULong:public KConfigSkeletonGenericItem < unsigned long >
  {
  public:
    ItemULong(const TQString & group, const TQString & key,
              unsigned long &reference, unsigned long defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
    TQVariant minValue() const;
    TQVariant maxValue() const;

    void setMinValue(unsigned long);
    void setMaxValue(unsigned long);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    unsigned long mMin;
    unsigned long mMax;
  };

  /**
   * Class for handling unsigned 64-bit integer preferences item.
   */
  class TDECORE_EXPORT ItemUInt64:public KConfigSkeletonGenericItem < TQ_UINT64 >
  {
  public:
    ItemUInt64(const TQString & group, const TQString & key, TQ_UINT64 &reference,
            TQ_UINT64 defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;

    TQVariant minValue() const;
    TQVariant maxValue() const;

    void setMinValue(TQ_UINT64);
    void setMaxValue(TQ_UINT64);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    TQ_UINT64 mMin;
    TQ_UINT64 mMax;
  };

  /**
   * Class for handling a floating point preference item.
   */
  class TDECORE_EXPORT ItemDouble:public KConfigSkeletonGenericItem < double >
  {
  public:
    ItemDouble(const TQString & group, const TQString & key,
               double &reference, double defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
    TQVariant minValue() const;
    TQVariant maxValue() const;

    void setMinValue(double);
    void setMaxValue(double);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    double mMin;
    double mMax;
  };


  /**
   * Class for handling a color preferences item.
   */
  class TDECORE_EXPORT ItemColor:public KConfigSkeletonGenericItem < TQColor >
  {
  public:
    ItemColor(const TQString & group, const TQString & key,
              TQColor & reference,
              const TQColor & defaultValue = TQColor(128, 128, 128));

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling a font preferences item.
   */
  class TDECORE_EXPORT ItemFont:public KConfigSkeletonGenericItem < TQFont >
  {
  public:
    ItemFont(const TQString & group, const TQString & key, TQFont & reference,
             const TQFont & defaultValue = TDEGlobalSettings::generalFont());

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling a TQRect preferences item.
   */
  class TDECORE_EXPORT ItemRect:public KConfigSkeletonGenericItem < TQRect >
  {
  public:
    ItemRect(const TQString & group, const TQString & key, TQRect & reference,
             const TQRect & defaultValue = TQRect());

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling a TQPoint preferences item.
   */
  class TDECORE_EXPORT ItemPoint:public KConfigSkeletonGenericItem < TQPoint >
  {
  public:
    ItemPoint(const TQString & group, const TQString & key, TQPoint & reference,
              const TQPoint & defaultValue = TQPoint());

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling a TQSize preferences item.
   */
  class TDECORE_EXPORT ItemSize:public KConfigSkeletonGenericItem < TQSize >
  {
  public:
    ItemSize(const TQString & group, const TQString & key, TQSize & reference,
             const TQSize & defaultValue = TQSize());

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling a TQDateTime preferences item.
   */
  class TDECORE_EXPORT ItemDateTime:public KConfigSkeletonGenericItem < TQDateTime >
  {
  public:
    ItemDateTime(const TQString & group, const TQString & key,
                 TQDateTime & reference,
                 const TQDateTime & defaultValue = TQDateTime());

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling a string list preferences item.
   */
  class TDECORE_EXPORT ItemStringList:public KConfigSkeletonGenericItem < TQStringList >
  {
  public:
    ItemStringList(const TQString & group, const TQString & key,
                   TQStringList & reference,
                   const TQStringList & defaultValue = TQStringList());

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


  /**
   * Class for handling a path list preferences item.
   */
  class TDECORE_EXPORT ItemPathList:public ItemStringList
  {
  public:
    ItemPathList(const TQString & group, const TQString & key,
                   TQStringList & reference,
                   const TQStringList & defaultValue = TQStringList());

    void readConfig(KConfig * config);
    void writeConfig(KConfig * config);
  };


  /**
   * Class for handling an integer list preferences item.
   */
  class TDECORE_EXPORT ItemIntList:public KConfigSkeletonGenericItem < TQValueList < int > >
  {
  public:
    ItemIntList(const TQString & group, const TQString & key,
                TQValueList < int >&reference,
                const TQValueList < int >&defaultValue = TQValueList < int >());

    void readConfig(KConfig * config);
    void setProperty(const TQVariant & p);
    TQVariant property() const;
  };


public:
  /**
   * Constructor.
   * 
   * @param configname name of config file. If no name is given, the default
   * config file as returned by kapp()->config() is used.
   */
  KConfigSkeleton(const TQString & configname = TQString::null);

  /**
   * Constructor.
   * 
   * @param config configuration object to use.
   */
  KConfigSkeleton(KSharedConfig::Ptr config);

  /**
   * Destructor
   */
    virtual ~ KConfigSkeleton();

  /**
    Set all registered items to their default values.
  */
  void setDefaults();

  /**
   * Read preferences from config file. All registered items are set to the
   * values read from disk.
   */
  void readConfig();

  /**
   * Write preferences to config file. The values of all registered items are
   * written to disk.
   */
  void writeConfig();

  /**
   * Set the config file group for subsequent addItem() calls. It is valid
   * until setCurrentGroup() is called with a new argument. Call this before
   * you add any items. The default value is "No Group".
   */
  void setCurrentGroup(const TQString & group);

  /**
   * Returns the current group used for addItem() calls. 
   */
  TQString currentGroup() // ### KDE 4.0: make const
  {
    return mCurrentGroup;
  }

  /**
   * Register a custom @ref KConfigSkeletonItem with a given name. If the name
   * parameter is null, take the name from KConfigSkeletonItem::key().
   * Note that all names must be unique but that multiple entries can have
   * the same key if they reside in different groups. 
   */
  void addItem(KConfigSkeletonItem *, const TQString & name = TQString::null );

  /**
   * Register an item of type TQString.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file 
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemString *addItemString(const TQString & name, TQString & reference,
                            const TQString & defaultValue = TQString::fromLatin1(""), // NOT TQString::null !!
                            const TQString & key = TQString::null);

  /**
   * Register a password item of type TQString. The string value is written 
   * encrypted to the config file. Note that the current encryption scheme 
   * is very weak.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPassword *addItemPassword(const TQString & name, TQString & reference,
                              const TQString & defaultValue = TQString::fromLatin1(""),
                              const TQString & key = TQString::null);

  /**
   * Register a path item of type TQString. The string value is interpreted
   * as a path. This means, dollar expension is activated for this value, so
   * that e.g. $HOME gets expanded. 
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPath *addItemPath(const TQString & name, TQString & reference,
                          const TQString & defaultValue = TQString::fromLatin1(""),
                          const TQString & key = TQString::null);

  /**
   * Register a property item of type TQVariant. Note that only the following
   * TQVariant types are allowed: String, StringList, Font, Point, Rect, Size,
   * Color, Int, UInt, Bool, Double, DateTime and Date.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemProperty *addItemProperty(const TQString & name, TQVariant & reference,
                                const TQVariant & defaultValue = TQVariant(),
                                const TQString & key = TQString::null);
  /**
   * Register an item of type bool.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemBool *addItemBool(const TQString & name, bool & reference,
                        bool defaultValue = false,
                        const TQString & key = TQString::null);

  /**
   * Register an item of type int.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemInt *addItemInt(const TQString & name, int &reference, int defaultValue = 0,
                      const TQString & key = TQString::null);

  /**
   * Register an item of type unsigned int.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemUInt *addItemUInt(const TQString & name, unsigned int &reference,
                        unsigned int defaultValue = 0,
                        const TQString & key = TQString::null);

  /**
   * Register an item of type long.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemLong *addItemLong(const TQString & name, long &reference,
                        long defaultValue = 0,
                        const TQString & key = TQString::null);

  /**
   * Register an item of type unsigned long.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemULong *addItemULong(const TQString & name, unsigned long &reference,
                          unsigned long defaultValue = 0,
                          const TQString & key = TQString::null);

  /**
   * Register an item of type TQ_INT64.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemInt64 *addItemInt64(const TQString & name, TQ_INT64 &reference,
                          TQ_INT64 defaultValue = 0,
                          const TQString & key = TQString::null);

  /**
   * Register an item of type TQ_UINT64
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemUInt64 *addItemUInt64(const TQString & name, TQ_UINT64 &reference,
                            TQ_UINT64 defaultValue = 0,
                            const TQString & key = TQString::null);

  /**
   * Register an item of type double.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemDouble *addItemDouble(const TQString & name, double &reference,
                            double defaultValue = 0.0,
                            const TQString & key = TQString::null);

  /**
   * Register an item of type TQColor.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemColor *addItemColor(const TQString & name, TQColor & reference,
                          const TQColor & defaultValue = TQColor(128, 128, 128),
                          const TQString & key = TQString::null);

  /**
   * Register an item of type TQFont.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemFont *addItemFont(const TQString & name, TQFont & reference,
                        const TQFont & defaultValue =
                        TDEGlobalSettings::generalFont(),
                        const TQString & key = TQString::null);

  /**
   * Register an item of type TQRect.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemRect *addItemRect(const TQString & name, TQRect & reference,
                        const TQRect & defaultValue = TQRect(),
                        const TQString & key = TQString::null);

  /**
   * Register an item of type TQPoint.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPoint *addItemPoint(const TQString & name, TQPoint & reference,
                          const TQPoint & defaultValue = TQPoint(),
                          const TQString & key = TQString::null);

  /**
   * Register an item of type TQSize.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemSize *addItemSize(const TQString & name, TQSize & reference,
                        const TQSize & defaultValue = TQSize(),
                        const TQString & key = TQString::null);

  /**
   * Register an item of type TQDateTime.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemDateTime *addItemDateTime(const TQString & name, TQDateTime & reference,
                                const TQDateTime & defaultValue = TQDateTime(),
                                const TQString & key = TQString::null);

  /**
   * Register an item of type TQStringList.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemStringList *addItemStringList(const TQString & name, TQStringList & reference,
                                    const TQStringList & defaultValue = TQStringList(),
                                    const TQString & key = TQString::null);

  /**
   * Register an item of type TQValueList<int>.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemIntList *addItemIntList(const TQString & name, TQValueList < int >&reference,
                              const TQValueList < int >&defaultValue =
                              TQValueList < int >(),
                              const TQString & key = TQString::null);

  /**
   * Return the @ref KConfig object used for reading and writing the settings.
   */
  KConfig *config() const;

  /**
   * Return list of items managed by this KConfigSkeleton object.
   */
  KConfigSkeletonItem::List items() const
  {
    return mItems;
  }

  /**
   * Return whether a certain item is immutable
   */
  bool isImmutable(const TQString & name);

  /**
   * Lookup item by name
   */
  KConfigSkeletonItem * findItem(const TQString & name);

  /**
   * Indicate whether this object should reflect the actual
   * values or the default values.
   * @param b If true this object reflects the default values.
   * @return The state prior to this call
   */
  bool useDefaults(bool b);

protected:
  /**
   * Implemented by subclasses that use special defaults.
   * It should replace the default values with the actual
   * values and vice versa.
   */
  virtual void usrUseDefaults(bool)
  {
  }

  virtual void usrSetDefaults()
  {
  }

  /**
   * Implemented by subclasses that read special config values.
   */
  virtual void usrReadConfig()
  {
  }

  /**
   * Implemented by subclasses that write special config values.
   */
  virtual void usrWriteConfig()
  {
  }

private:
  TQString mCurrentGroup;

  KSharedConfig::Ptr mConfig; // pointer to KConfig object

  KConfigSkeletonItem::List mItems;
  KConfigSkeletonItem::Dict mItemDict;
  
  bool mUseDefaults;

  class Private;
  Private *d;

};

#endif
