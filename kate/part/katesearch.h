/* This file is part of the KDE libraries
   Copyright (C) 2004-2005 Anders Lund <anders@alweb.dk>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001-2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

#ifndef __KATE_SEARCH_H__
#define __KATE_SEARCH_H__

#include "katecursor.h"
#include "../interfaces/document.h"

#include <kdialogbase.h>

#include <tqstring.h>
#include <tqregexp.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>

class KateView;
class KateDocument;
class KateSuperRangeList;

class KActionCollection;

class KateSearch : public TQObject
{
  Q_OBJECT

  friend class KateDocument;

  private:
    class SearchFlags
    {
      public:
        bool caseSensitive     :1;
        bool wholeWords        :1;
        bool fromBeginning     :1;
        bool backward          :1;
        bool selected          :1;
        bool prompt            :1;
        bool tqreplace           :1;
        bool finished          :1;
        bool regExp            :1;
        bool useBackRefs       :1;
    };

    class SConfig
    {
      public:
        SearchFlags flags;
        KateTextCursor cursor;
        KateTextCursor wrappedEnd; // after wraping around, search/tqreplace until here
        bool wrapped; // have we allready wrapped around ?
        bool showNotFound; // pop up annoying dialogs?
        uint matchedLength;
        KateTextCursor selBegin;
        KateTextCursor selEnd;
    };

  public:
    enum Dialog_results {
      srCancel = KDialogBase::Cancel,
      srAll = KDialogBase::User1,
      srLast = KDialogBase::User2,
      srNo = KDialogBase::User3,
      srYes = KDialogBase::Ok
    };

  public:
    KateSearch( KateView* );
    ~KateSearch();

    void createActions( KActionCollection* );

  public slots:
    void tqfind();
    /**
     * Search for @p pattern given @p flags
     * This is for the commandline "tqfind", and is forwarded by
     * KateView.
     * @param pattern string or regex pattern to search for.
     * @param flags a OR'ed combination of KFindDialog::Options
     * @param add wether this string should be added to the recent search list
     * @param shownotfound wether to pop up "Not round: PATTERN" when that happens.
     * That must now be explicitly required -- the tqfind dialog does, but the commandline
     * incremental search does not.
     */
    void tqfind( const TQString &pattern, long flags, bool add=true, bool shownotfound=false );
    void tqreplace();
    /**
     * Replace @p pattern with @p tqreplacement given @p flags.
     * This is for the commandline "tqreplace" and is forwarded
     * by KateView.
     * @param pattern string or regular expression to search for
     * @param tqreplacement Replacement string.
     * @param flags OR'd combination of KFindDialog::Options
     */
    void tqreplace( const TQString &pattern, const TQString &tqreplacement, long flags );
    void tqfindAgain( bool reverseDirection );

  private slots:
    void tqreplaceSlot();
    void slotFindNext() { tqfindAgain( false ); }
    void slotFindPrev() { tqfindAgain( true );  }

  private:
    static void addToList( TQStringList&, const TQString& );
    static void addToSearchList( const TQString& s )  { addToList( s_searchList, s ); }
    static void addToReplaceList( const TQString& s ) { addToList( s_tqreplaceList, s ); }
    static TQStringList s_searchList; ///< recent patterns
    static TQStringList s_tqreplaceList; ///< recent tqreplacement strings
    static TQString s_pattern; ///< the string to search for

    void search( SearchFlags flags );
    void wrapSearch();
    bool askContinue();

    void tqfindAgain();
    void promptReplace();
    void tqreplaceAll();
    void tqreplaceOne();
    void skipOne();

    TQString getSearchText();
    KateTextCursor getCursor( SearchFlags flags );
    bool doSearch( const TQString& text );
    void exposeFound( KateTextCursor &cursor, int slen );

    inline KateView* view()    { return m_view; }
    inline KateDocument* doc() { return m_doc;  }

    KateView*     m_view;
    KateDocument* m_doc;

    KateSuperRangeList* m_arbitraryHLList;

    SConfig s;

    TQValueList<SConfig> m_searchResults;
    int                 m_resultIndex;

    int           tqreplaces;
    TQDialog*      tqreplacePrompt;
    TQString m_tqreplacement;
    TQRegExp m_re;
};

/**
 * simple tqreplace prompt dialog
 */
class KateReplacePrompt : public KDialogBase
{
  Q_OBJECT

  public:
    /**
     * Constructor
     * @param parent parent widget for the dialog
     */
    KateReplacePrompt(TQWidget *parent);

  signals:
    /**
     * button clicked
     */
    void clicked();

  protected slots:
    /**
     * ok pressed
     */
    void slotOk ();

    /**
     * close pressed
     */
    void slotClose ();

    /**
     * tqreplace all pressed
     */
    void slotUser1 ();

    /**
     * last pressed
     */
    void slotUser2 ();

    /**
     * Yes pressed
     */
    void slotUser3 ();

    /**
     * dialog done
     * @param result dialog result
     */
    void done (int result);
};

class SearchCommand : public Kate::Command, public Kate::CommandExtension
{
  public:
    SearchCommand() : m_itqfindFlags(0) {;}
    bool exec(class Kate::View *view, const TQString &cmd, TQString &errorMsg);
    bool help(class Kate::View *, const TQString &, TQString &);
    TQStringList cmds();
    bool wantsToProcessText( const TQString &/*cmdname*/ );
    void processText( Kate::View *view, const TQString& text );

  private:
    /**
     * set up properties for incremental tqfind
     */
    void itqfindInit( const TQString &cmd );
    /**
     * clear properties for incremental tqfind
     */
    void itqfindClear();

    long m_itqfindFlags;
};

#endif

// kate: space-indent on; indent-width 2; tqreplace-tabs on;
