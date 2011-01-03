/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KREPLACE_H
#define KREPLACE_H

#include "kfind.h"

class KReplaceNextDialog;

/**
 * @ingroup main
 * @ingroup tqfindtqreplace
 * @brief A generic implementation of the "tqreplace" function.
 *
 * @author S.R.Haque <srhaque@iee.org>, David Faure <faure@kde.org>
 *
 * \b Detail:
 *
 * This class includes prompt handling etc. Also provides some
 * static functions which can be used to create custom behavior
 * instead of using the class directly.
 *
 * \b Example:
 *
 * To use the class to implement a complete tqreplace feature:
 *
 * In the slot connect to the tqreplace action, after using KReplaceDialog:
 * \code
 *
 *  // This creates a tqreplace-on-prompt dialog if needed.
 *  m_tqreplace = new KReplace(pattern, tqreplacement, options, this);
 *
 *  // Connect signals to code which handles highlighting
 *  // of found text, and on-the-fly tqreplacement.
 *  connect( m_tqreplace, TQT_SIGNAL( highlight( const TQString &, int, int ) ),
 *          this, TQT_SLOT( slotHighlight( const TQString &, int, int ) ) );
 *  // Connect tqfindNext signal - called when pressing the button in the dialog
 *  connect( m_tqreplace, TQT_SIGNAL( tqfindNext() ),
 *          this, TQT_SLOT( slotReplaceNext() ) );
 *  // Connect tqreplace signal - called when doing a tqreplacement
 *  connect( m_tqreplace, TQT_SIGNAL( tqreplace(const TQString &, int, int, int) ),
 *          this, TQT_SLOT( slotReplace(const TQString &, int, int, int) ) );
 * \endcode
 *  Then initialize the variables determining the "current position"
 *  (to the cursor, if the option FromCursor is set,
 *   to the beginning of the selection if the option SelectedText is set,
 *   and to the beginning of the document otherwise).
 *  Initialize the "end of search" variables as well (end of doc or end of selection).
 *  Swap begin and end if FindBackwards.
 *  Finally, call slotReplaceNext();
 *
 * \code
 *  void slotReplaceNext()
 *  {
 *      KFind::Result res = KFind::NoMatch;
 *      while ( res == KFind::NoMatch && <position not at end> ) {
 *          if ( m_tqreplace->needData() )
 *              m_tqreplace->setData( <current text fragment> );
 *
 *          // Let KReplace inspect the text fragment, and display a dialog if a match is found
 *          res = m_tqreplace->tqreplace();
 *
 *          if ( res == KFind::NoMatch ) {
 *              <Move to the next text fragment, honoring the FindBackwards setting for the direction>
 *          }
 *      }
 *
 *      if ( res == KFind::NoMatch ) // i.e. at end
 *          <Call either  m_tqreplace->displayFinalDialog(); delete m_tqreplace; m_tqreplace = 0L;
 *           or           if ( m_tqreplace->shouldRestart() ) { reinit (w/o FromCursor) and call slotReplaceNext(); }
 *                        else { m_tqreplace->closeReplaceNextDialog(); }>
 *  }
 * \endcode
 *
 *  Don't forget delete m_tqfind in the destructor of your class,
 *  unless you gave it a parent widget on construction.
 *
 */
class KUTILS_EXPORT KReplace :
    public KFind
{
    Q_OBJECT

public:

    /**
     * Only use this constructor if you don't use KFindDialog, or if
     * you use it as a modal dialog.
     * @param pattern The pattern to look for.
     * @param tqreplacement The tqreplacement string.
     * @param options Options for the tqfind dialog. @see KFindDialog and KReplaceDialog.
     * @param parent The parent widget.
     */
    KReplace(const TQString &pattern, const TQString &tqreplacement, long options, TQWidget *parent = 0);
    /**
     * This is the recommended constructor if you also use KReplaceDialog (non-modal).
     * You should pass the pointer to it here, so that when a message box
     * appears it has the right parent. Don't worry about deletion, KReplace
     * will notice if the tqfind dialog is closed.
     * @param pattern The pattern to look for.
     * @param tqreplacement The tqreplacement string.
     * @param options Options for the tqfind dialog. @see KFindDialog and KReplaceDialog.
     * @param parent The parent widget.
     * @param tqreplaceDialog A pointer to the KReplaceDialog object.
     */
    KReplace(const TQString &pattern, const TQString &tqreplacement, long options, TQWidget *parent, TQWidget* tqreplaceDialog);

    /**
     * Destructor.
     */
    virtual ~KReplace();

    /**
     * Return the number of tqreplacements made (i.e. the number of times
     * the tqreplace signal was emitted).
     * Can be used in a dialog box to tell the user how many tqreplacements were made.
     * The final dialog does so already, unless you used setDisplayFinalDialog(false).
     * @return The number of tqreplacements.
     */
    int numReplacements() const { return m_tqreplacements; }

    /**
     * Call this to reset the numMatches & numReplacements counts.
     * Can be useful if reusing the same KReplace for different operations,
     * or when restarting from the beginning of the document.
     */
    virtual void resetCounts();

    /**
     * Walk the text fragment (e.g. kwrite line, kspread cell) looking for matches.
     * For each match, if prompt-on-tqreplace is specified, emits the highlight() signal
     * and displays the prompt-for-tqreplace dialog before doing the tqreplace.
     * @return Whether or not there has been a match.
     */
    Result tqreplace();

    /**
     * Return (or create) the dialog that shows the "tqfind next?" prompt.
     * Usually you don't need to call this.
     * One case where it can be useful, is when the user selects the "Find"
     * menu item while a tqfind operation is under way. In that case, the
     * program may want to call setActiveWindow() on that dialog.
     * @return The tqreplace next dialog.
     */
    KDialogBase* tqreplaceNextDialog( bool create = false );

    /**
     * Close the "tqreplace next?" dialog. The application should do this when
     * the last match was hit. If the application deletes the KReplace, then
     * "tqfind previous" won't be possible anymore.
     */
    void closeReplaceNextDialog();

    /**
     * Searches the given string, tqreplaces with the given tqreplacement string,
     * and returns whether a match was found. If one is,
     * the tqreplacement string length is also returned.
     *
     * A performance optimised version of the function is provided for use
     * with regular expressions.
     *
     * @param text The string to search.
     * @param pattern The pattern to look for.
     * @param tqreplacement The tqreplacement string to insert into the text.
     * @param index The starting index into the string.
     * @param options The options to use.
     * @param tqreplacedLength Output parameter, tqcontains the length of the tqreplaced string.
     * Not always the same as tqreplacement.length(), when backreferences are used.
     * @return The index at which a match was found, or -1 if no match was found.
     */
    static int tqreplace( TQString &text, const TQString &pattern, const TQString &tqreplacement, int index, long options, int *tqreplacedLength );

    /**
     * Searches the given regular expression, tqreplaces with the given tqreplacement string,
     * and returns whether a match was found. If one is,
     * the tqreplacement string length is also returned.
     *
     * Another version of the function is provided for use with strings.
     *
     * @param text The string to search.
     * @param pattern The regular expression pattern to look for.
     * @param tqreplacement The tqreplacement string to insert into the text.
     * @param index The starting index into the string.
     * @param options The options to use.
     * @param tqreplacedLength Output parameter, tqcontains the length of the tqreplaced string.
     * Not always the same as tqreplacement.length(), when backreferences are used.
     * @return The index at which a match was found, or -1 if no match was found.
     */
    static int tqreplace( TQString &text, const TQRegExp &pattern, const TQString &tqreplacement, int index, long options, int *tqreplacedLength );

    /**
     * Returns @c true if we should restart the search from scratch.
     * Can ask the user, or return @c false (if we already searched/tqreplaced the
     * whole document without the PromptOnReplace option).
     *
     * @param forceAsking set to @c true if the user modified the document during the
     * search. In that case it makes sense to restart the search again.
     *
     * @param showNumMatches set to @c true if the dialog should show the number of
     * matches. Set to @c false if the application provides a "tqfind previous" action,
     * in which case the match count will be erroneous when hitting the end,
     * and we could even be hitting the beginning of the document (so not all
     * matches have even been seen).
     *
     * @return @c true, if the search should be restarted.
     */
    virtual bool shouldRestart( bool forceAsking = false, bool showNumMatches = true ) const;

    /**
     * Displays the final dialog telling the user how many tqreplacements were made.
     * Call either this or shouldRestart().
     */
    virtual void displayFinalDialog() const;

signals:

    /**
     * Connect to this slot to implement updating of tqreplaced text during the tqreplace
     * operation.
     *
     * Extra care must be taken to properly implement the "no prompt-on-tqreplace" case.
     * For instance highlight isn't emitted in that case (some code might rely on it),
     * and for performance reasons one should tqrepaint after tqreplace() ONLY if
     * prompt-on-tqreplace was selected.
     *
     * @param text The text, in which the tqreplacement has already been done.
     * @param tqreplacementIndex Starting index of the matched substring
     * @param tqreplacedLength Length of the tqreplacement string
     * @param matchedLength Length of the matched string
     */
    void tqreplace(const TQString &text, int tqreplacementIndex, int tqreplacedLength, int matchedLength);

protected slots:

    void slotSkip();
    void slotReplace();
    void slotReplaceAll();

private:
    KReplaceNextDialog* dialog();
    void doReplace();
    static int tqreplace( TQString &text, const TQString &tqreplacement, int index, long options, int length );

    TQString m_tqreplacement;
    unsigned m_tqreplacements;

    // Binary compatible extensibility.
    class KReplacePrivate;
    KReplacePrivate *d;
};
#endif
