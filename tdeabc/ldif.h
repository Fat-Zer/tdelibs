/*
    This file is part of libtdeabc.
    Copyright (c) 2004 Szombathelyi Gyorgy <gyurco@freemail.hu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General  Public
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

#ifndef _K_LDIF_H_
#define _K_LDIF_H_

#include <tqstring.h>
#include <tqcstring.h>
#include <tqmemarray.h>

#include <tdelibs_export.h>

namespace TDEABC {

  /**
   * LDIF
 
   * LDIF implements an RFC 2849 compliant LDIF parser. LDIF files are used to
   * represent directory information on LDAP-based servers, or to describe a set
   * of changes which are to be applied to a directory. 
   */

  class KABC_EXPORT LDIF 
  {
  public:

    enum ParseVal{ None, NewEntry, EndEntry, Item, Control, Err, MoreData };
    enum EntryType{ Entry_None, Entry_Add, Entry_Del, Entry_Mod, Entry_Modrdn };
    enum ModType{ Mod_None, Mod_Add, Mod_Replace, Mod_Del };
    LDIF();
    virtual ~LDIF();
    
    /**
     * Assembles fieldname and value into a valid LDIF line, BASE64 encodes the 
     * value if neccessary and optionally splits into more lines.
     * @param fieldname The name of the entry.
     * @param value The value of the entry.
     * @param linelen Maximum length of the lines in the result.
     * @param url If true, encode value as url ( use :< ).
     */
    static TQCString assembleLine( const TQString &fieldname, 
      const TQByteArray &value, uint linelen=0, bool url=false );
    /**
     * This is the same as the above function, the only difference that 
     * this accepts TQCString as the value.
     */
    static TQCString assembleLine( const TQString &fieldname, 
      const TQCString &value, uint linelen=0, bool url=false );
    /**
     * This is the same as the above function, the only difference that 
     * this accepts TQString as the value.
     */
    static TQCString assembleLine( const TQString &fieldname, 
      const TQString &value, uint linelen=0, bool url=false );
      
    /**
     * Splits one line from an LDIF file to attribute and value components.
     * @returns true if value is an URL, false otherwise
     */
    static bool splitLine( const TQCString &line, TQString &fieldname, TQByteArray &value );
    /**
     * Splits a control specification (without the "control:" directive)
     * @param line is the control directive
     * @param oid will contain the OID
     * @param critical will contain the criticality of control
     * @param value is the control value
     */
    static bool splitControl( const TQCString &line, TQString &oid, bool &critical,
      TQByteArray &value );
    /**
     * Starts the parsing of a new LDIF
     */
    void startParsing();
    /**
     * Process one LDIF line
     */
    ParseVal processLine();
    /**
     * Process the LDIF until a complete item can be returned
     * @returns NewEntry if a new DN encountered, 
     * Item if a new item returned, 
     * Err if the LDIF contains error,
     * EndEntry if the parser reached the end of the current entry
     * and MoreData if the parser encountered the end of the current chunk of 
     * the LDIF. If you want to finish the parsing after receiving 
     * MoreData, then call endLDIF(), so the parser can safely flush 
     * the current entry.
     */
    ParseVal nextItem();
    /**
     * Sets a chunk of LDIF. Call before startParsing(), or if nextItem() returned
     * MoreData.
     */
    void setLDIF( const TQByteArray &ldif ) { mLdif = ldif; mPos = 0; }
    /**
      * Indicates the end of the LDIF file/stream. Call if nextItem() returned
      * MoreData, but actually you don't have more data.
      */
    void endLDIF();
    /**
     * Returns the requested LDAP operation extracted from the current entry.
     */
    EntryType entryType() const { return mEntryType; }
    /**
     * Returns the LDAP modify request type if entryType() returned Entry_Mod.
     */
    int modType() const { return mModType; }
    /**
     * Returns the Distinguished Name of the current entry.
     */
    const TQString& dn() const { return mDn; }
    /**
     * Returns the new Relative Distinguished Name if modType() returned Entry_Modrdn.
     */
    const TQString& newRdn() const { return mNewRdn; }
    /**
     * Returns the new parent of the entry if modType() returned Entry_Modrdn.
     */
    const TQString& newSuperior() const { return mNewSuperior; }
    /**
     * Returns if the delete of the old RDN is required.
     */
    bool delOldRdn() const { return mDelOldRdn; }
    /**
     * Returns the attribute name.
     */
    const TQString& attr() const { return mAttr; }
    /**
     * Returns the attribute value.
     */
    const TQByteArray& val() const { return mVal; }
    /**
     * Returns if val() is an url
     */
    bool isUrl() const { return mUrl; }
    /**
     * Returns the criticality level when modType() returned Control.
     */
    bool critical() const { return mCritical; }
    /**
     * Returns the OID when modType() returned Control.
     */
    const TQString& oid() const { return mOid; }
    /**
     * Returns the line number which the parser processes.
     */
    uint lineNo() const { return mLineNo; }
  private:
    int mModType;
    bool mDelOldRdn, mUrl;
    TQString mDn,mAttr,mNewRdn,mNewSuperior, mOid;
    TQByteArray mLdif, mVal;
    EntryType mEntryType;
    
    bool mIsNewLine, mIsComment,mCritical;
    ParseVal mLastParseVal;
    uint mPos,mLineNo;  
    TQCString line;
        
    class LDIFPrivate;
    LDIFPrivate *d;
  };
}

#endif
