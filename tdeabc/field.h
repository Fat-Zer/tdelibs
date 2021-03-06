/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KABC_FIELD_H
#define KABC_FIELD_H

#include <tqstring.h>
#include <tqvaluelist.h>

#include "addressee.h"

class TDEConfig;

namespace TDEABC {

class KABC_EXPORT Field
{
  class FieldImpl;
  friend class FieldImpl;

public:
  typedef TQValueList<Field *> List;

  /**
   * @li @p All -
   * @li @p Frequent -
   * @li @p Address -
   * @li @p Email -
   * @li @p Personal -
   * @li @p Organization -
   * @li @p CustomCategory -
   */  
  enum FieldCategory
  {
    All = 0x0,
    Frequent = 0x01,
    Address = 0x02,
    Email = 0x04,
    Personal = 0x08,
    Organization = 0x10,
    CustomCategory = 0x20
  };

  /**
   * Returns the translated label for this field.
   */
  virtual TQString label();

  /**
   * Returns the  ored categories the field belongs to.
   */
  virtual int category();

  /**
   * Returns the translated label for field category.
   */
  static TQString categoryLabel( int category );

  /**
   * Returns a string representation of the value the field has in the given
   * Addressee. Returns TQString::null, if it is not possible to convert the
   * value to a string.
   */
  virtual TQString value( const TDEABC::Addressee & );

  /**
   * Sets the value of the field in the given Addressee. Returns true on success
   * or false, if the given string couldn't be converted to a valid value.
   */
  virtual bool setValue( TDEABC::Addressee &, const TQString & );

  /**
   * Returns a string, that can be used for sorting.
   */
  TQString sortKey( const TDEABC::Addressee & );

  /**
   * Returns, if the field is a user-defined field.
   */
  virtual bool isCustom();

  /**
   * Returns, if the field is equal with @a field.
   */
  virtual bool equals( Field *field );

  /**
   * Returns a list of all fields.
   */
  static Field::List allFields();

  /**
   * Returns a list of the default fields.
   */
  static Field::List defaultFields();

  /**
   * Creates a custom field.
   *
   * @param label    The label for this field
   * @param category The category of this field
   * @param key      Unique key for this field
   * @param app      Unique app name for this field
   */
  static Field *createCustomField( const TQString &label, int category,
                                   const TQString &key, const TQString &app );

  /**
   * Delete all fields from list.
   */
  static void deleteFields();

  /**
   * Save the field settings to a config file.
   *
   * @param cfg        The config file object
   * @param identifier The unique identifier
   * @param fields     The list of the fields
   */
  static void saveFields( TDEConfig *cfg, const TQString &identifier,
                          const Field::List &fields );
  /**
   * This is the same as above, with the difference, that
   * the list is stored in TDEGlobal::config() in group "KABCFields".
   */
  static void saveFields( const TQString &identifier,
                          const Field::List &fields );

  /**
   * Load the field settings from a config file.
   *
   * @param cfg        The config file object
   * @param identifier The unique identifier
   */
  static Field::List restoreFields( TDEConfig *cfg, const TQString &identifier );

  /**
   * This is the same as above, with the difference, that
   * the list is loaded from TDEGlobal::config() from group "KABCFields".
   */
  static Field::List restoreFields( const TQString &identifier );

protected:
  static void createField( int id, int category = 0 );
  static void createDefaultField( int id, int category = 0 );

private:
  Field( FieldImpl * );
  virtual ~Field();

  FieldImpl *mImpl;

  static Field::List mAllFields;
  static Field::List mDefaultFields;
  static Field::List mCustomFields;
};

}
#endif
