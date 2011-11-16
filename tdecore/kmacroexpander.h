/*
    This file is part of the KDE libraries

    Copyright (c) 2002-2003 Oswald Buddenhagen <ossi@kde.org>
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
#ifndef _KMACROEXPANDER_H
#define _KMACROEXPANDER_H

#include <tqstringlist.h>
#include <tqstring.h>
#include <tqmap.h>
#include "tdelibs_export.h"

/**
 * Abstract base class for the worker classes behind the KMacroExpander namespace
 * and the KCharMacroExpander and KWordMacroExpander classes.
 *
 * @since 3.1.3
 * @author Oswald Buddenhagen <ossi@kde.org>
 */
class TDECORE_EXPORT KMacroExpanderBase {

public:
    /**
     * Constructor.
     * @param c escape char indicating start of macros, or TQChar::null for none
     */
    KMacroExpanderBase( TQChar c = '%' );

    /**
     * Destructor.
     */
    virtual ~KMacroExpanderBase();

    /**
     * Perform safe macro expansion (substitution) on a string.
     *
     * @param str the string in which macros are expanded in-place
     */
    void expandMacros( TQString &str );

    /*
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     *
     * Explicitly supported shell constructs:
     *   \ '' "" $'' $"" {} () $(()) ${} $() ``
     *
     * Implicitly supported shell constructs:
     *   (())
     *
     * Unsupported shell constructs that will cause problems:
     *  @li Shortened "case $v in pat)" syntax. Use "case $v in (pat)" instead.
     *
     * The rest of the shell (incl. bash) syntax is simply ignored,
     * as it is not expected to cause problems.
     *
     * Note that bash contains a bug which makes macro expansion within 
     * double quoted substitutions ("${VAR:-%macro}") inherently insecure.
     *
     * @param str the string in which macros are expanded in-place
     * @param pos the position inside the string at which parsing/substitution
     *  should start, and upon exit where processing stopped
     * @return false if the string could not be parsed and therefore no safe
     *  substitution was possible. Note that macros will have been processed
     *  up to the point where the error occurred. An unmatched closing paren
     *  or brace outside any shell construct is @em not an error (unlike in
     *  the function below), but still prematurely terminates processing.
     */
    bool expandMacrosShellQuote( TQString &str, uint &pos );

    /**
     * Same as above, but always starts at position 0, and unmatched closing
     * parens and braces are treated as errors.
     */
    bool expandMacrosShellQuote( TQString &str );

    /**
     * Set the macro escape character.
     * @param c escape char indicating start of macros, or TQChar::null if none
     */
    void setEscapeChar( TQChar c );

    /**
     * Obtain the macro escape character.
     * @return escape char indicating start of macros, or TQChar::null if none
     */
    TQChar escapeChar() const;

protected:
    /**
     * This function is called for every single char within the string if
     * the escape char is TQChar::null. It should determine whether the
     * string starting at @p pos within @p str is a valid macro and return
     * the substitution value for it if so.
     * @param str the input string
     * @param pos the offset within @p str
     * @param ret return value: the string to substitute for the macro
     * @return if greater than zero, the number of chars at @p pos in @p str
     *  to substitute with @p ret (i.e., a valid macro was found). if less
     *  than zero, subtract this value from @p pos (to skip a macro, i.e.,
     *  substitute it with itself). zero requests no special action.
     */
    virtual int expandPlainMacro( const TQString &str, uint pos, TQStringList &ret );

    /**
     * This function is called every time the escape char is found if it is
     * not TQChar::null. It should determine whether the
     * string starting at @p pos witin @p str is a valid macro and return
     * the substitution value for it if so.
     * @param str the input string
     * @param pos the offset within @p str. Note that this is the position of
     *  the occurrence of the escape char
     * @param ret return value: the string to substitute for the macro
     * @return if greater than zero, the number of chars at @p pos in @p str
     *  to substitute with @p ret (i.e., a valid macro was found). if less
     *  than zero, subtract this value from @p pos (to skip a macro, i.e.,
     *  substitute it with itself). zero requests no special action.
     */
    virtual int expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret );

private:
    TQChar escapechar;
};

/**
 * Abstract base class for simple word macro substitutors. Use this instead of
 * the functions in the KMacroExpander namespace if speculatively pre-filling
 * the substitution map would be too expensive.
 *
 * A typical application:
 *
 * \code
 * class MyClass {
 * ...
 *   private:
 *     TQString m_str;
 * ...
 *   friend class MyExpander;
 * };
 *
 * class MyExpander : public KWordMacroExpander {
 *   public:
 *     MyExpander( MyClass *_that ) : KWordMacroExpander(), that( _that ) {}
 *   protected:
 *     virtual bool expandMacro( const TQString &str, TQStringList &ret );
 *   private:
 *     MyClass *that;
 * };
 *
 * bool MyExpander::expandMacro( const TQString &str, TQStringList &ret )
 * {
 *   if (str == "macro") {
 *     ret += complexOperation( that->m_str );
 *     return true;
 *   }
 *   return false;
 * }
 *
 * ... MyClass::...(...)
 * {
 *   TQString str;
 *   ...
 *   MyExpander mx( this );
 *   mx.expandMacrosShellQuote( str );
 *   ...
 * }
 * \endcode
 *
 * Alternatively MyClass could inherit from KWordMacroExpander directly.
 *
 * @since 3.3
 * @author Oswald Buddenhagen <ossi@kde.org>
 */
class TDECORE_EXPORT KWordMacroExpander : public KMacroExpanderBase {

public:
    /**
     * Constructor.
     * @param c escape char indicating start of macros, or TQChar::null for none
     */
    KWordMacroExpander( TQChar c = '%' ) : KMacroExpanderBase( c ) {}

protected:
    virtual int expandPlainMacro( const TQString &str, uint pos, TQStringList &ret );
    virtual int expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret );

    /**
     * Return substitution list @p ret for string macro @p str.
     * @param str the macro to expand
     * @param ret return variable reference. It is guaranteed to be empty
     *  when expandMacro is entered.
     * @return @c true iff @p chr was a recognized macro name
     */
    virtual bool expandMacro( const TQString &str, TQStringList &ret ) = 0;
};

/**
 * Abstract base class for single char macro substitutors. Use this instead of
 * the functions in the KMacroExpander namespace if speculatively pre-filling
 * the substitution map would be too expensive.
 *
 * See KWordMacroExpander for a sample application.
 *
 * @since 3.3
 * @author Oswald Buddenhagen <ossi@kde.org>
 */
class TDECORE_EXPORT KCharMacroExpander : public KMacroExpanderBase {

public:
    /**
     * Constructor.
     * @param c escape char indicating start of macros, or TQChar::null for none
     */
    KCharMacroExpander( TQChar c = '%' ) : KMacroExpanderBase( c ) {}

protected:
    virtual int expandPlainMacro( const TQString &str, uint pos, TQStringList &ret );
    virtual int expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret );

    /**
     * Return substitution list @p ret for single-character macro @p chr.
     * @param chr the macro to expand
     * @param ret return variable reference. It is guaranteed to be empty
     *  when expandMacro is entered.
     * @return @c true iff @p chr was a recognized macro name
     */
    virtual bool expandMacro( TQChar chr, TQStringList &ret ) = 0;
};

/**
 * A group of functions providing macro expansion (substitution) in strings,
 * optionally with quoting appropriate for shell execution.
 * @since 3.1.3
 */
namespace KMacroExpander {
    /**
     * Perform safe macro expansion (substitution) on a string.
     * The escape char must be quoted with itself to obtain its literal
     * representation in the resulting string.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro, or TQChar::null if none
     * @return the string with all valid macros expanded
     *
     * \code
     * // Code example
     * TQMap<TQChar,TQString> map;
     * map.insert('u', "/tmp/myfile.txt");
     * map.insert('n', "My File");
     * TQString s = "%% Title: %u:%n";
     * s = KMacroExpander::expandMacros(s, map);
     * // s is now "% Title: /tmp/myfile.txt:My File";
     * \endcode
     */
    TDECORE_EXPORT TQString expandMacros( const TQString &str, const TQMap<TQChar,TQString> &map, TQChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     * The escape char must be quoted with itself to obtain its literal
     * representation in the resulting string.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro, or TQChar::null if none
     * @return the string with all valid macros expanded, or a null string
     *  if a shell syntax error was detected in the command
     *
     * \code
     * // Code example
     * TQMap<TQChar,TQString> map;
     * map.insert('u', "/tmp/myfile.txt");
     * map.insert('n', "My File");
     * TQString s = "kedit --caption %n %u";
     * s = KMacroExpander::expandMacrosShellQuote(s, map);
     * // s is now "kedit --caption 'My File' '/tmp/myfile.txt'";
     * system(TQFile::encodeName(s));
     * \endcode
     */
    TDECORE_EXPORT TQString expandMacrosShellQuote( const TQString &str, const TQMap<TQChar,TQString> &map, TQChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string.
     * The escape char must be quoted with itself to obtain its literal
     * representation in the resulting string.
     * Macro names can consist of chars in the range [A-Za-z0-9_];
     * use braces to delimit macros from following words starting
     * with these chars, or to use other chars for macro names.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro, or TQChar::null if none
     * @return the string with all valid macros expanded
     *
     * \code
     * // Code example
     * TQMap<TQString,TQString> map;
     * map.insert("url", "/tmp/myfile.txt");
     * map.insert("name", "My File");
     * TQString s = "Title: %{url}-%name";
     * s = KMacroExpander::expandMacros(s, map);
     * // s is now "Title: /tmp/myfile.txt-My File";
     * \endcode
     */
    TDECORE_EXPORT TQString expandMacros( const TQString &str, const TQMap<TQString,TQString> &map, TQChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     * The escape char must be quoted with itself to obtain its literal
     * representation in the resulting string.
     * Macro names can consist of chars in the range [A-Za-z0-9_];
     * use braces to delimit macros from following words starting
     * with these chars, or to use other chars for macro names.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro, or TQChar::null if none
     * @return the string with all valid macros expanded, or a null string
     *  if a shell syntax error was detected in the command
     *
     * \code
     * // Code example
     * TQMap<TQString,TQString> map;
     * map.insert("url", "/tmp/myfile.txt");
     * map.insert("name", "My File");
     * TQString s = "kedit --caption %name %{url}";
     * s = KMacroExpander::expandMacrosShellQuote(s, map);
     * // s is now "kedit --caption 'My File' '/tmp/myfile.txt'";
     * system(TQFile::encodeName(s));
     * \endcode
     */
    TDECORE_EXPORT TQString expandMacrosShellQuote( const TQString &str, const TQMap<TQString,TQString> &map, TQChar c = '%' );

    /**
     * Same as above, except that the macros expand to string lists that
     * are simply join(" ")ed together.
     */
    TDECORE_EXPORT TQString expandMacros( const TQString &str, const TQMap<TQChar,TQStringList> &map, TQChar c = '%' );
    /**
     * Same as above, except that the macros expand to string lists that
     * are simply join(" ")ed together.
     */
    TDECORE_EXPORT TQString expandMacros( const TQString &str, const TQMap<TQString,TQStringList> &map, TQChar c = '%' );

    /**
     * Same as above, except that the macros expand to string lists.
     * If the macro appears inside a quoted string, the list is simply
     * join(" ")ed together; otherwise every element expands to a separate
     * quoted string.
     */
    TDECORE_EXPORT TQString expandMacrosShellQuote( const TQString &str, const TQMap<TQChar,TQStringList> &map, TQChar c = '%' );
    /**
     * Same as above, except that the macros expand to string lists.
     * If the macro appears inside a quoted string, the list is simply
     * join(" ")ed together; otherwise every element expands to a separate
     * quoted string.
     */
    TDECORE_EXPORT TQString expandMacrosShellQuote( const TQString &str, const TQMap<TQString,TQStringList> &map, TQChar c = '%' );
}

#endif /* _KMACROEXPANDER_H */
