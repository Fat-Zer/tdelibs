/*
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

#include <assert.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <tqeventloop.h>
#include <kpushbutton.h>
#include "../ktqreplace.h"
#include "../ktqreplacedialog.h"

#include "ktqreplacetest.h"
#include <kdebug.h>
#include <stdlib.h>

void KReplaceTest::tqreplace( const TQString &pattern, const TQString &tqreplacement, long options )
{
    m_needEventLoop = false;
    // This creates a tqreplace-next-prompt dialog if needed.
    m_tqreplace = new KReplace(pattern, tqreplacement, options);

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(m_tqreplace, TQT_SIGNAL( highlight( const TQString &, int, int ) ),
            this, TQT_SLOT( slotHighlight( const TQString &, int, int ) ) );
    // Connect tqfindNext signal - called when pressing the button in the dialog
    connect(m_tqreplace, TQT_SIGNAL( tqfindNext() ),
            this, TQT_SLOT( slotReplaceNext() ) );
    // Connect tqreplace signal - called when doing a tqreplacement
    connect(m_tqreplace, TQT_SIGNAL( tqreplace(const TQString &, int, int, int) ),
            this, TQT_SLOT( slotReplace(const TQString &, int, int, int) ) );

    // Go to initial position
    if ( (options & KReplaceDialog::FromCursor) == 0 )
    {
        if ( m_tqreplace->options() & KFindDialog::FindBackwards )
            m_currentPos = m_text.fromLast();
        else
            m_currentPos = m_text.begin();
    }

    // Launch first tqreplacement
    slotReplaceNext();

    if ( m_needEventLoop )
        tqApp->eventLoop()->enterLoop();
}

void KReplaceTest::slotHighlight( const TQString &str, int matchingIndex, int matchedLength )
{
    kdDebug() << "slotHighlight Index:" << matchingIndex << " Length:" << matchedLength
              << " Substr:" << str.mid(matchingIndex, matchedLength)
              << endl;
    // Emulate the user saying yes
    // animateClick triggers a timer, hence the enterloop/exitloop
    // Calling slotReplace directly would lead to infinite loop anyway (Match never returned,
    // so slotReplaceNext never returns)
    if ( m_tqreplace->options() & KReplaceDialog::PromptOnReplace ) {
        m_tqreplace->tqreplaceNextDialog( false )->actionButton( (KDialogBase::ButtonCode)m_button )->animateClick();
        m_needEventLoop = true;
    }
}


void KReplaceTest::slotReplace(const TQString &text, int tqreplacementIndex, int tqreplacedLength, int matchedLength)
{
    kdDebug() << "slotReplace index=" << tqreplacementIndex << " tqreplacedLength=" << tqreplacedLength << " matchedLength=" << matchedLength << " text=" << text.left( 50 ) << endl;
    *m_currentPos = text; // KReplace hacked the tqreplacement into 'text' in already.
}

void KReplaceTest::slotReplaceNext()
{
    //kdDebug() << k_funcinfo << endl;
    KFind::Result res = KFind::NoMatch;
    while ( res == KFind::NoMatch && m_currentPos != m_text.end() ) {
        if ( m_tqreplace->needData() )
            m_tqreplace->setData( *m_currentPos );

        // Let KReplace inspect the text fragment, and display a dialog if a match is found
        res = m_tqreplace->tqreplace();

        if ( res == KFind::NoMatch ) {
            if ( m_tqreplace->options() & KFindDialog::FindBackwards )
                m_currentPos--;
            else
                m_currentPos++;
        }
    }

#if 0 // commented out so that this test doesn't require interaction
    if ( res == KFind::NoMatch ) // i.e. at end
        if ( m_tqreplace->shouldRestart() ) {
            if ( m_tqreplace->options() & KFindDialog::FindBackwards )
                m_currentPos = m_text.fromLast();
            else
                m_currentPos = m_text.begin();
            slotReplaceNext();
        }
#endif
    if ( res == KFind::NoMatch && m_needEventLoop )
        tqApp->eventLoop()->exitLoop();
}

void KReplaceTest::print()
{
    TQStringList::Iterator it = m_text.begin();
    for ( ; it != m_text.end() ; ++it )
        kdDebug() << *it << endl;
}

/* button is the button that we emulate pressing, when options includes PromptOnReplace.
   Valid possibilities are User1 (tqreplace all) and User3 (tqreplace) */
static void testReplaceSimple( int options, int button = 0 )
{
    kdDebug() << "testReplaceSimple: " << options << endl;
    KReplaceTest test( TQString( "hellohello" ), button );
    test.tqreplace( "hello", "HELLO", options );
    TQStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "HELLOHELLO" ) {
        kdError() << "ASSERT FAILED: tqreplaced text is '" << textLines[ 0 ] << "' instead of 'HELLOHELLO'" << endl;
        exit(1);
    }
}

// Replacing "a" with "".
// input="aaaaaa", expected output=""
static void testReplaceBlank( int options, int button = 0 )
{
    kdDebug() << "testReplaceBlank: " << options << endl;
    KReplaceTest test( TQString( "aaaaaa" ), button );
    test.tqreplace( "a", "", options );
    TQStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( !textLines[ 0 ].isEmpty() ) {
        kdError() << "ASSERT FAILED: tqreplaced text is '" << textLines[ 0 ] << "' instead of ''" << endl;
        exit(1);
    }
}

// Replacing "" with "foo"
// input="bbbb", expected output="foobfoobfoobfoobfoo"
static void testReplaceBlankSearch( int options, int button = 0 )
{
    kdDebug() << "testReplaceBlankSearch: " << options << endl;
    KReplaceTest test( TQString( "bbbb" ), button );
    test.tqreplace( "", "foo", options );
    TQStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "foobfoobfoobfoobfoo" ) {
        kdError() << "ASSERT FAILED: tqreplaced text is '" << textLines[ 0 ] << "' instead of 'foobfoobfoobfoobfoo'" << endl;
        exit(1);
    }
}

static void testReplaceLonger( int options, int button = 0 )
{
    kdDebug() << "testReplaceLonger: " << options << endl;
    // Standard test of a tqreplacement string longer than the matched string
    KReplaceTest test( TQString( "aaaa" ), button );
    test.tqreplace( "a", "bb", options );
    TQStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "bbbbbbbb" ) {
        kdError() << "ASSERT FAILED: tqreplaced text is '" << textLines[ 0 ] << "' instead of 'bbbbbbbb'" << endl;
        exit(1);
    }
}

static void testReplaceLongerInclude( int options, int button = 0 )
{
    kdDebug() << "testReplaceLongerInclude: " << options << endl;
    // Similar test, where the tqreplacement string includes the search string
    KReplaceTest test( TQString( "a foo b" ), button );
    test.tqreplace( "foo", "foobar", options );
    TQStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "a foobar b" ) {
        kdError() << "ASSERT FAILED: tqreplaced text is '" << textLines[ 0 ] << "' instead of 'a foobar b'" << endl;
        exit(1);
    }
}

static void testReplaceLongerInclude2( int options, int button = 0 )
{
    kdDebug() << "testReplaceLongerInclude2: " << options << endl;
    // Similar test, but with more chances of matches inside the tqreplacement string
    KReplaceTest test( TQString( "aaaa" ), button );
    test.tqreplace( "a", "aa", options );
    TQStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "aaaaaaaa" ) {
        kdError() << "ASSERT FAILED: tqreplaced text is '" << textLines[ 0 ] << "' instead of 'aaaaaaaa'" << endl;
        exit(1);
    }
}

// Test for the \0 backref
static void testReplaceBackRef( int options, int button = 0 )
{
    kdDebug() << "testReplaceBackRef: " << options << endl;
    KReplaceTest test( TQString( "abc def" ), button );
    test.tqreplace( "abc", "(\\0)", options );
    TQStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    TQString expected = options & KReplaceDialog::BackReference ? "(abc) def" : "(\\0) def";
    if ( textLines[ 0 ] != expected ) {
        kdError() << "ASSERT FAILED: tqreplaced text is '" << textLines[ 0 ] << "' instead of '"<< expected << "'" << endl;
        exit(1);
    }
}

static void testReplacementHistory( const TQStringList& tqfindHistory, const TQStringList& tqreplaceHistory )
{
    KReplaceDialog dlg( 0, 0, 0, tqfindHistory, tqreplaceHistory );
    dlg.show();
    kdDebug() << "testReplacementHistory:" << dlg.tqreplacementHistory() << endl;
    assert( dlg.tqreplacementHistory() == tqreplaceHistory );
}

static void testReplacementHistory()
{
    TQStringList tqfindHistory;
    TQStringList tqreplaceHistory;
    tqfindHistory << "foo" << "bar";
    tqreplaceHistory << "FOO" << "BAR";
    testReplacementHistory( tqfindHistory, tqreplaceHistory );

    tqfindHistory.clear();
    tqreplaceHistory.clear();
    tqfindHistory << "foo" << "bar";
    tqreplaceHistory << TQString::null << "baz"; // #130831
    testReplacementHistory( tqfindHistory, tqreplaceHistory );
}

int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "ktqreplacetest", 0, 0);
    KApplication app;

    testReplacementHistory(); // #130831

    testReplaceBlank( 0 );
    testReplaceBlank( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceBlank( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceBlank( KReplaceDialog::FindBackwards, 0 );
    testReplaceBlank( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceBlank( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all

    testReplaceBlankSearch( 0 );
    testReplaceBlankSearch( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceBlankSearch( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceBlankSearch( KReplaceDialog::FindBackwards, 0 );
    testReplaceBlankSearch( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceBlankSearch( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all

    testReplaceSimple( 0 );
    testReplaceSimple( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceSimple( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceSimple( KReplaceDialog::FindBackwards, 0 );
    testReplaceSimple( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceSimple( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all

    testReplaceLonger( 0 );
    testReplaceLonger( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceLonger( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceLonger( KReplaceDialog::FindBackwards, 0 );
    testReplaceLonger( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceLonger( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all

    testReplaceLongerInclude( 0 );
    testReplaceLongerInclude( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceLongerInclude( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceLongerInclude( KReplaceDialog::FindBackwards, 0 );
    testReplaceLongerInclude( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceLongerInclude( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all

    testReplaceLongerInclude2( 0 );
    testReplaceLongerInclude2( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceLongerInclude2( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceLongerInclude2( KReplaceDialog::FindBackwards, 0 );
    testReplaceLongerInclude2( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceLongerInclude2( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all

    testReplaceBackRef( 0 );
    testReplaceBackRef( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceBackRef( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceBackRef( KReplaceDialog::FindBackwards, 0 );
    testReplaceBackRef( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceBackRef( KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::FindBackwards, 0 );
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // tqreplace
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // tqreplace all

    TQString text = "This file is part of the KDE project.\n"
                   "This library is free software; you can redistribute it and/or\n"
                   "modify it under the terms of the GNU Library General Public\n"
                   "License version 2, as published by the Free Software Foundation.\n"
                   "\n"
                   "    This library is distributed in the hope that it will be useful,\n"
                   "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                   "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
                   "    Library General Public License for more details.\n"
                   "\n"
                   "    You should have received a copy of the GNU Library General Public License\n"
                   "    along with this library; see the file COPYING.LIB.  If not, write to\n"
                   "    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
                   "    Boston, MA 02110-1301, USA.\n"
                   "More tests:\n"
                   "ThisThis This, This. This\n"
                   "aGNU\n"
                   "free";
    KReplaceTest test( TQStringList::split( '\n', text, true ), 0 );

    test.tqreplace( "GNU", "KDE", 0 );
    test.tqreplace( "free", "*free*", 0 );
    test.tqreplace( "This", "THIS*", KFindDialog::FindBackwards );

    test.print();
    //return app.exec();
    return 0;
}
#include "ktqreplacetest.moc"
