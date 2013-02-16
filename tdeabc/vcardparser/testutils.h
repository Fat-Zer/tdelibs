#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <tdeabc/addressee.h>
#include <tqstring.h>

KABC::Addressee vcard1();
KABC::Addressee vcard2();
KABC::Addressee vcard3();
KABC::Addressee::List vCardsAsAddresseeList();
TQString   vCardAsText( const TQString& location );
TQString   vCardsAsText();

#endif
