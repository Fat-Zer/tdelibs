/* This file is part of the KDE libraries
   Copyright (C) 2001 David Faure <david@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef kwordwrap_h
#define kwordwrap_h

#include <tqfontmetrics.h>
#include <tqvaluelist.h>
#include <tqrect.h>
#include <tqstring.h>

#include <kdelibs_export.h>

/**
 * Word-wrap algorithm that takes into account beautifulness ;)
 *
 * That means:
 * @li not letting a letter alone on the last line,
 * @li breaking at punctuation signs (not only at spaces)
 * @li improved handling of (), [] and {}
 * @li improved handling of '/' (e.g. for paths)
 *
 * Usage: call the static method, formatText, with the text to
 * wrap and the constraining rectangle etc., it will return an instance of KWordWrap
 * containing internal data, result of the word-wrapping.
 * From that instance you can retrieve the boundingRect, and invoke drawing.
 *
 * This design allows to call the word-wrap algorithm only when the text changes
 * and not every time we want to know the bounding rect or draw the text.
 *
 * @author David Faure <faure@kde.org>
 */
class KDEUI_EXPORT KWordWrap
{
public:
    /**
     * Use this flag in drawText() if you want to fade out the text if it does
     * not fit into the constraining rectangle.
     * @since 3.2
     */
    enum { FadeOut = 0x10000000, Truncate = 0x20000000 };

    /**
     * Main method for wrapping text.
     *
     * @param fm Font metrics, for the chosen font. Better cache it, creating a TQFontMetrics is expensive.
     * @param r Constraining rectangle. Only the width and height matter. With
     *          negative height the complete text will be rendered
     * @param flags currently unused
     * @param str The text to be wrapped.
     * @param len Length of text to wrap (default is -1 for all).
     * @return a KWordWrap instance. The caller is responsible for storing and deleting the result.
     */
    static KWordWrap* formatText( TQFontMetrics &fm, const TQRect & r, int flags, const TQString & str, int len = -1 );

    /**
     * @return the bounding rect, calculated by formatText. The width is the
     *         width of the widest text line, and never wider than
     *         the rectangle given to formatText. The height is the
     *         text block. X and Y are always 0.
     */
    TQRect boundingRect() const { return m_boundingRect; }

    /**
     * @return the original string, with '\n' inserted where
     * the text is broken by the wordwrap algorithm.
     */
    TQString wrappedString() const; // gift for Dirk :)

    /**
     * @return the original string, truncated to the first line.
     * If @p dots was set, '...' is appended in case the string was truncated.
     * Bug: Note that the '...' come out of the bounding rect.
     */
    TQString truncatedString( bool dots = true ) const;

    /**
     * Draw the text that has been previously wrapped, at position x,y.
     * Flags are for tqalignment, e.g. Qt::AlignHCenter. Default is
     * Qt::AlignAuto.
     * @param painter the TQPainter to use.
     * @param x the horizontal position of the text
     * @param y the vertical position of the text
     * @param flags the ORed text tqalignment flags from the Qt namespace,
     *              ORed with FadeOut if you want the text to fade out if it
     *              does not fit (the @p painter's background must be set
     *              accordingly)
     */
    void drawText( TQPainter *painter, int x, int y, int flags = TQt::AlignAuto ) const;

    /**
     * Destructor.
     */
    ~KWordWrap();

    /**
     * Draws the string @p t at the given coordinates, if it does not
     * @p fit into @p maxW the text will be faded out.
     * @param p the painter to use. Must have set the pen for the text
     *        color and the background for the color to fade out
     * @param x the horizontal position of the text
     * @param y the vertical position of the text
     * @param maxW the maximum width of the text (including the fade-out
     *             effect)
     * @param t the text to draw
     * @since 3.2
     */
    static void drawFadeoutText( TQPainter *p, int x, int y, int maxW,
                                 const TQString &t );

    /**
     * Draws the string @p t at the given coordinates, if it does not
     * @p fit into @p maxW the text will be truncated.
     * @param p the painter to use
     * @param x the horizontal position of the text
     * @param y the vertical position of the text
     * @param maxW the maximum width of the text (including the '...')
     * @param t the text to draw
     * @since 3.3
     */
    static void drawTruncateText( TQPainter *p, int x, int y, int maxW,
                                  const TQString &t );

private:
    KWordWrap( const TQRect & r );
    TQValueList<int> m_breakPositions;
    TQValueList<int> m_lineWidths;
    TQRect m_boundingRect;
    TQString m_text;
private:
    class KWordWrapPrivate* d;
};

#endif
