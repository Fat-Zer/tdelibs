/* This file is part of the KDE File Manager

   Copyright (C) 1998-2000 Waldo Bastian (bastian@kde.org)
   Copyright (C) 2000,2001 Dawit Alemayehu (adawit@kde.org)

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, and/or sell copies of the
   Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookies
// $Id$

//
// The cookie protocol is a mess. RFC2109 is a joke since nobody seems to
// use it. Apart from that it is badly written.
// We try to implement Netscape Cookies and try to behave us according to
// RFC2109 as much as we can.
//
// We assume cookies do not contain any spaces (Netscape spec.)
// According to RFC2109 this is allowed though.
//

#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_SOLARIS
#include <strings.h>
#endif

#include <stdlib.h>

//#include <netinet/in.h>
//#include <arpa/inet.h>

#include <tqstring.h>
#include <tqstrlist.h>
#include <tqptrlist.h>
#include <tqptrdict.h>
#include <tqfile.h>
#include <tqdir.h>
#include <tqregexp.h>

#include <kurl.h>
#include <krfcdate.h>
#include <tdeconfig.h>
#include <ksavefile.h>
#include <kdebug.h>

#include "kcookiejar.h"


// BR87227
// Waba: Should the number of cookies be limited?
// I am not convinced of the need of such limit
// Mozilla seems to limit to 20 cookies / domain
// but it is unclear which policy it uses to expire
// cookies when it exceeds that amount
#undef MAX_COOKIE_LIMIT

#define MAX_COOKIES_PER_HOST 25
#define READ_BUFFER_SIZE 8192
#define IP_ADDRESS_EXPRESSION "(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"

// Note with respect to TQString::fromLatin1( )
// Cookies are stored as 8 bit data and passed to tdeio_http as
// latin1 regardless of their actual encoding.

// L1 is used to indicate latin1 constants
#define L1(x) TQString::fromLatin1(x)

template class TQPtrList<KHttpCookie>;
template class TQPtrDict<KHttpCookieList>;

TQString KCookieJar::adviceToStr(KCookieAdvice _advice)
{
    switch( _advice )
    {
    case KCookieAccept: return L1("Accept");
    case KCookieReject: return L1("Reject");
    case KCookieAsk: return L1("Ask");
    default: return L1("Dunno");
    }
}

KCookieAdvice KCookieJar::strToAdvice(const TQString &_str)
{
    if (_str.isEmpty())
        return KCookieDunno;

    TQCString advice = _str.lower().latin1();

    if (advice == "accept")
        return KCookieAccept;
    else if (advice == "reject")
        return KCookieReject;
    else if (advice == "ask")
        return KCookieAsk;

    return KCookieDunno;
}

// KHttpCookie
///////////////////////////////////////////////////////////////////////////

//
// Cookie constructor
//
KHttpCookie::KHttpCookie(const TQString &_host,
                 const TQString &_domain,
                 const TQString &_path,
                 const TQString &_name,
                 const TQString &_value,
                 time_t _expireDate,
                 int _protocolVersion,
                 bool _secure,
                 bool _httpOnly,
                 bool _explicitPath) :
       mHost(_host),
       mDomain(_domain),
       mPath(_path.isEmpty() ? TQString::null : _path),
       mName(_name),
       mValue(_value),
       mExpireDate(_expireDate),
       mProtocolVersion(_protocolVersion),
       mSecure(_secure),
       mCrossDomain(false),
       mHttpOnly(_httpOnly),
       mExplicitPath(_explicitPath)
{
}

//
// Checks if a cookie has been expired
//
bool    KHttpCookie::isExpired(time_t currentDate)
{
    return (mExpireDate != 0) && (mExpireDate < currentDate);
}

//
// Returns a string for a HTTP-header
//
TQString KHttpCookie::cookieStr(bool useDOMFormat)
{
    TQString result;

    if (useDOMFormat || (mProtocolVersion == 0))
    {
        if ( !mName.isEmpty() )
           result = mName + '=';
        result += mValue;
    }
    else
    {
        result = mName + '=' + mValue;
        if (mExplicitPath)
            result += L1("; $Path=\"") + mPath + L1("\"");
        if (!mDomain.isEmpty())
            result += L1("; $Domain=\"") + mDomain + L1("\"");
    }
    return result;
}

//
// Returns whether this cookie should be send to this location.
bool KHttpCookie::match(const TQString &fqdn, const TQStringList &domains,
                        const TQString &path)
{
    // Cookie domain match check
    if (mDomain.isEmpty())
    {
       if (fqdn != mHost)
          return false;
    }
    else if (!domains.contains(mDomain))
    {
        if (mDomain[0] == '.')
            return false;

        // Maybe the domain needs an extra dot.
        TQString domain = '.' + mDomain;
        if ( !domains.contains( domain ) )
          if ( fqdn != mDomain )
            return false;
    }

    // Cookie path match check
    if (mPath.isEmpty())
        return true;

    // According to the netscape spec both http://www.acme.com/foobar,
    // http://www.acme.com/foo.bar and http://www.acme.com/foo/bar
    // match http://www.acme.com/foo.
    // We only match http://www.acme.com/foo/bar

    if( path.startsWith(mPath) &&
        (
         (path.length() == mPath.length() ) || 	// Paths are exact match
         (path[mPath.length()-1] == '/') || 	// mPath ended with a slash
         (path[mPath.length()] == '/')		// A slash follows.
         ))
        return true; // Path of URL starts with cookie-path

    return false;
}

// KHttpCookieList
///////////////////////////////////////////////////////////////////////////

int KHttpCookieList::compareItems( void * item1, void * item2)
{
    int pathLen1 = ((KHttpCookie *)item1)->path().length();
    int pathLen2 = ((KHttpCookie *)item2)->path().length();
    if (pathLen1 > pathLen2)
        return -1;
    if (pathLen1 < pathLen2)
        return 1;
    return 0;
}


// KCookieJar
///////////////////////////////////////////////////////////////////////////

//
// Constructs a new cookie jar
//
// One jar should be enough for all cookies.
//
KCookieJar::KCookieJar()
{
    m_cookieDomains.setAutoDelete( true );
    m_globalAdvice = KCookieDunno;
    m_configChanged = false;
    m_cookiesChanged = false;
    
    TDEConfig cfg("tdehtml/domain_info", true, false, "data");
    TQStringList countries = cfg.readListEntry("twoLevelTLD");
    for(TQStringList::ConstIterator it = countries.begin();
        it != countries.end(); ++it)
    {
       m_twoLevelTLD.replace(*it, (int *) 1);
    }
}

//
// Destructs the cookie jar
//
// Poor little cookies, they will all be eaten by the cookie monster!
//
KCookieJar::~KCookieJar()
{
    // Not much to do here
}

static void removeDuplicateFromList(KHttpCookieList *list, KHttpCookie *cookiePtr, bool nameMatchOnly=false, bool updateWindowId=false)
{
    TQString domain1 = cookiePtr->domain();
    if (domain1.isEmpty())
       domain1 = cookiePtr->host();

    for ( KHttpCookiePtr cookie=list->first(); cookie != 0; )
    {
       TQString domain2 = cookie->domain();
       if (domain2.isEmpty())
          domain2 = cookie->host();

       if ( 
            (cookiePtr->name() == cookie->name()) &&
            (
              nameMatchOnly ||
              ( (domain1 == domain2) && (cookiePtr->path() == cookie->path()) )
            )
          )
       {
          if (updateWindowId)
          {
            for(TQValueList<long>::ConstIterator it = cookie->windowIds().begin();
                it != cookie->windowIds().end(); ++it)
            {
               long windowId = *it;
               if (windowId && (cookiePtr->windowIds().find(windowId) == cookiePtr->windowIds().end()))
               {
                  cookiePtr->windowIds().append(windowId);
               }
            }
          }
          KHttpCookiePtr old_cookie = cookie;
          cookie = list->next();
          list->removeRef( old_cookie );
          break;
       }
       else
       {
          cookie = list->next();
       }
    }
}


//
// Looks for cookies in the cookie jar which are appropriate for _url.
// Returned is a string containing all appropriate cookies in a format
// which can be added to a HTTP-header without any additional processing.
//
TQString KCookieJar::findCookies(const TQString &_url, bool useDOMFormat, long windowId, KHttpCookieList *pendingCookies)
{
    TQString cookieStr;
    TQStringList domains;
    TQString fqdn;
    TQString path;
    KHttpCookiePtr cookie;
    KCookieAdvice advice = m_globalAdvice;

    if (!parseURL(_url, fqdn, path))
        return cookieStr;

    bool secureRequest = (_url.find( L1("https://"), 0, false) == 0 ||
                          _url.find( L1("webdavs://"), 0, false) == 0);

    // kdDebug(7104) << "findCookies: URL= " << _url << ", secure = " << secureRequest << endl;

    extractDomains(fqdn, domains);

    KHttpCookieList allCookies;

    for(TQStringList::ConstIterator it = domains.begin();
        true;
        ++it)
    {
       KHttpCookieList *cookieList;
       if (it == domains.end())
       {
          cookieList = pendingCookies; // Add pending cookies
          pendingCookies = 0;
          if (!cookieList)
             break;
       }
       else
       {
          TQString key = (*it).isNull() ? L1("") : (*it);
          cookieList = m_cookieDomains[key];
          if (!cookieList)
             continue; // No cookies for this domain
       }

       if (cookieList->getAdvice() != KCookieDunno)
          advice = cookieList->getAdvice();

       for ( cookie=cookieList->first(); cookie != 0; cookie=cookieList->next() )
       {
          // If the we are setup to automatically accept all session cookies and to
          // treat all cookies as session cookies or the current cookie is a session
          // cookie, then send the cookie back regardless of either policy.
          if (advice == KCookieReject &&
              !(m_autoAcceptSessionCookies && 
                (m_ignoreCookieExpirationDate || cookie->expireDate() == 0)))
              continue;

          if (!cookie->match(fqdn, domains, path))
             continue;

          if( cookie->isSecure() && !secureRequest )
             continue;

          if( cookie->isHttpOnly() && useDOMFormat )
             continue;

          // Do not send expired cookies.
          if ( cookie->isExpired (time(0)) )
          {
             // Note there is no need to actually delete the cookie here
             // since the cookieserver will invoke ::saveCookieJar because
             // of the state change below. This will then do the job of
             // deleting the cookie for us.
             m_cookiesChanged = true;
             continue;
          }

          if (windowId && (cookie->windowIds().find(windowId) == cookie->windowIds().end()))
          {
             cookie->windowIds().append(windowId);
          }

          if (it == domains.end()) // Only needed when processing pending cookies
             removeDuplicateFromList(&allCookies, cookie);

          allCookies.append(cookie);
       }
       if (it == domains.end())
          break; // Finished.
    }

    int cookieCount = 0;

    int protVersion=0; 
    for ( cookie=allCookies.first(); cookie != 0; cookie=allCookies.next() )
    {
       if (cookie->protocolVersion() > protVersion)
          protVersion = cookie->protocolVersion();
    }

    for ( cookie=allCookies.first(); cookie != 0; cookie=allCookies.next() )
    {
       if (useDOMFormat)
       {
          if (cookieCount > 0)
             cookieStr += L1("; ");
          cookieStr += cookie->cookieStr(true);
       }
       else
       {
          if (cookieCount == 0)
          {
             cookieStr += L1("Cookie: ");
             if (protVersion > 0)
             {
                TQString version;
                version.sprintf("$Version=%d; ", protVersion); // Without quotes
                cookieStr += version;
             }
          }
          else
          {
             cookieStr += L1("; ");
          }
          cookieStr += cookie->cookieStr(false);
       }
       cookieCount++;
    }

    return cookieStr;
}

//
// This function parses a string like 'my_name="my_value";' and returns
// 'my_name' in Name and 'my_value' in Value.
//
// A pointer to the end of the parsed part is returned.
// This pointer points either to:
// '\0' - The end of the string has reached.
// ';'  - Another my_name="my_value" pair follows
// ','  - Another cookie follows
// '\n' - Another header follows
static const char * parseNameValue(const char *header,
                                  TQString &Name,
                                  TQString &Value,
                                  bool keepQuotes=false,
                                  bool rfcQuotes=false)
{
    const char *s = header;
    // Parse 'my_name' part
    for(; (*s != '='); s++)
    {
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // No '=' sign -> use string as the value, name is empty
            // (behavior found in Mozilla and IE)
            Name = "";
            Value = TQString::fromLatin1(header);
            Value.truncate( s - header );
            Value = Value.stripWhiteSpace();
            return (s);
        }
    }

    Name = header;
    Name.truncate( s - header );
    Name = Name.stripWhiteSpace();

    // *s == '='
    s++;

    // Skip any whitespace
    for(; (*s == ' ') || (*s == '\t'); s++)
    {
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // End of Name
            Value = "";
            return (s);
        }
    }

    if ((rfcQuotes || !keepQuotes) && (*s == '\"'))
    {
        // Parse '"my_value"' part (quoted value)
        if (keepQuotes)
           header = s++;
        else
           header = ++s; // skip "
        for(;(*s != '\"');s++)
        {
            if ((*s=='\0') || (*s=='\n'))
            {
                // End of Name
                Value = TQString::fromLatin1(header);
                Value.truncate(s - header);
                return (s);
            }
        }
        Value = TQString::fromLatin1(header);
        // *s == '\"';
        if (keepQuotes)
           Value.truncate( ++s - header );
        else
           Value.truncate( s++ - header );

        // Skip any remaining garbage
        for(;; s++)
        {
            if ((*s=='\0') || (*s==';') || (*s=='\n'))
                break;
        }
    }
    else
    {
        // Parse 'my_value' part (unquoted value)
        header = s;
        while ((*s != '\0') && (*s != ';') && (*s != '\n'))
            s++;
        // End of Name
        Value = TQString::fromLatin1(header);
        Value.truncate( s - header );
        Value = Value.stripWhiteSpace();
    }
    return (s);

}

void KCookieJar::stripDomain(const TQString &_fqdn, TQString &_domain)
{
   TQStringList domains;
   extractDomains(_fqdn, domains);
   if (domains.count() > 3)
      _domain = domains[3];
   else
      _domain = domains[0];
}

TQString KCookieJar::stripDomain( KHttpCookiePtr cookiePtr)
{
    TQString domain; // We file the cookie under this domain.
    if (cookiePtr->domain().isEmpty())
       stripDomain( cookiePtr->host(), domain);
    else
       stripDomain (cookiePtr->domain(), domain);
    return domain;
}

bool KCookieJar::parseURL(const TQString &_url,
                          TQString &_fqdn,
                          TQString &_path)
{
    KURL kurl(_url);
    if (!kurl.isValid())
       return false;

    _fqdn = kurl.host().lower();
    if (kurl.port())
    {
       if (((kurl.protocol() == L1("http")) && (kurl.port() != 80)) ||
           ((kurl.protocol() == L1("https")) && (kurl.port() != 443)))
       {
          _fqdn = L1("%1:%2").arg(kurl.port()).arg(_fqdn);
       }
    }

    // Cookie spoofing protection.  Since there is no way a path separator
    // or escape encoded character is allowed in the hostname according
    // to RFC 2396, reject attempts to include such things there!
    if(_fqdn.find('/') > -1 || _fqdn.find('%') > -1)
    {
        return false;  // deny everything!!
    }

    _path = kurl.path();
    if (_path.isEmpty())
       _path = L1("/");

    TQRegExp exp(L1("[\\\\/]\\.\\.[\\\\/]"));
    // Weird path, cookie stealing attempt?
    if (exp.search(_path) != -1)
       return false; // Deny everything!!

    return true;
}

void KCookieJar::extractDomains(const TQString &_fqdn,
                                TQStringList &_domains)
{
    // Return numeric IPv6 addresses as is...
    if (_fqdn[0] == '[')
    {
       _domains.append( _fqdn );
       return;
    }
    // Return numeric IPv4 addresses as is...
    if ((_fqdn.at(0) >= TQChar('0')) && (_fqdn.at(0) <= TQChar('9')))
    {
       if (_fqdn.find(TQRegExp(IP_ADDRESS_EXPRESSION)) > -1)
       {
          _domains.append( _fqdn );
          return;
       }
    }

    TQStringList partList = TQStringList::split('.', _fqdn, false);

    if (partList.count())
        partList.remove(partList.begin()); // Remove hostname

    while(partList.count())
    {

       if (partList.count() == 1)
         break; // We only have a TLD left.

       if ((partList.count() == 2) && (m_twoLevelTLD[partList[1].lower()]))
       {
          // This domain uses two-level TLDs in the form xxxx.yy
          break;
       }

       if ((partList.count() == 2) && (partList[1].length() == 2))
       {
          // If this is a TLD, we should stop. (e.g. co.uk)
          // We assume this is a TLD if it ends with .xx.yy or .x.yy
          if (partList[0].length() <= 2)
             break; // This is a TLD.

          // Catch some TLDs that we miss with the previous check
          // e.g. com.au, org.uk, mil.co
          TQCString t = partList[0].lower().utf8();
          if ((t == "com") || (t == "net") || (t == "org") || (t == "gov") || (t == "edu") || (t == "mil") || (t == "int"))
              break;
       }

       TQString domain = partList.join(L1("."));
       _domains.append(domain);
       _domains.append('.' + domain);
       partList.remove(partList.begin()); // Remove part
    }

    // Always add the FQDN at the start of the list for
    // hostname == cookie-domainname checks!
    _domains.prepend( '.' + _fqdn );
    _domains.prepend( _fqdn );
}


/*
   Changes dates in from the following format

      Wed Sep 12 07:00:00 2007 GMT
   to
      Wed Sep 12 2007 07:00:00 GMT

   to allow KRFCDate::parseDate to properly parse expiration date formats 
   used in cookies by some servers such as amazon.com. See BR# 145244.
*/
static TQString fixupDateTime(const TQString& dt)
{
  const int index = dt.find(TQRegExp("[0-9]{1,2}:[0-9]{1,2}:[0-9]{1,2}"));

  if (index > -1)
  {
    TQStringList dateStrList = TQStringList::split(' ', dt.mid(index));
    if (dateStrList.count() > 1)
    {
      TQString date = dateStrList[0];
      dateStrList[0] = dateStrList[1];
      dateStrList[1] = date;
      date = dt;
      return date.replace(index, date.length(), dateStrList.join(" "));
    }
  }

  return dt;
}

//
// This function parses cookie_headers and returns a linked list of
// KHttpCookie objects for all cookies found in cookie_headers.
// If no cookies could be found 0 is returned.
//
// cookie_headers should be a concatenation of all lines of a HTTP-header
// which start with "Set-Cookie". The lines should be separated by '\n's.
//
KHttpCookieList KCookieJar::makeCookies(const TQString &_url,
                                       const TQCString &cookie_headers,
                                       long windowId)
{
    KHttpCookieList cookieList;
    KHttpCookieList cookieList2;
    KHttpCookiePtr lastCookie = 0;
    const char *cookieStr = cookie_headers.data();
    TQString Name;
    TQString Value;
    TQString fqdn;
    TQString path;
    bool crossDomain = false;

    if (!parseURL(_url, fqdn, path))
    {
        // Error parsing _url
        return KHttpCookieList();
    }
    TQString defaultPath;
    int i = path.findRev('/');
    if (i > 0)
       defaultPath = path.left(i);

    //  The hard stuff :)
    for(;;)
    {
        // check for "Set-Cookie"
        if (strncmp(cookieStr, "Cross-Domain\n", 13) == 0)
        {
            cookieStr += 13;
            crossDomain = true;
        }
        else if (strncasecmp(cookieStr, "Set-Cookie:", 11) == 0) 
        {
            cookieStr = parseNameValue(cookieStr+11, Name, Value, true);

            // Host = FQDN
            // Default domain = ""
            // Default path according to rfc2109

            KHttpCookie *cookie = new KHttpCookie(fqdn, L1(""), defaultPath, Name, Value);
            if (windowId)
               cookie->mWindowIds.append(windowId);
            cookie->mCrossDomain = crossDomain;

            // Insert cookie in chain
            cookieList.append(cookie);
            lastCookie = cookie;
        }
        else if (strncasecmp(cookieStr, "Set-Cookie2:", 12) == 0)
        {
            // Attempt to follow rfc2965
            cookieStr = parseNameValue(cookieStr+12, Name, Value, true, true);

            // Host = FQDN
            // Default domain = ""
            // Default path according to rfc2965

            KHttpCookie *cookie = new KHttpCookie(fqdn, L1(""), defaultPath, Name, Value);
            if (windowId)
               cookie->mWindowIds.append(windowId);
            cookie->mCrossDomain = crossDomain;

            // Insert cookie in chain
            cookieList2.append(cookie);
            lastCookie = cookie;
        }
        else
        {
            // This is not the start of a cookie header, skip till next line.
            while (*cookieStr && *cookieStr != '\n')
                cookieStr++;

            if (*cookieStr == '\n')
                cookieStr++;

            if (!*cookieStr)
                break; // End of cookie_headers
            else
                continue; // end of this header, continue with next.
        }

        while ((*cookieStr == ';') || (*cookieStr == ' '))
        {
            cookieStr++;

            // Name-Value pair follows
            cookieStr = parseNameValue(cookieStr, Name, Value);

            TQCString cName = Name.lower().latin1();
            if (cName == "domain")
            {
                TQString dom = Value.lower();
                // RFC2965 3.2.2: If an explicitly specified value does not
                // start with a dot, the user agent supplies a leading dot
                if(dom.length() && dom[0] != '.')
                    dom.prepend(".");
                // remove a trailing dot
                if(dom.length() > 2 && dom[dom.length()-1] == '.')
                    dom = dom.left(dom.length()-1);

                if(dom.contains('.') > 1 || dom == ".local")
                    lastCookie->mDomain = dom;
            }
            else if (cName == "max-age")
            {
                int max_age = Value.toInt();
                if (max_age == 0)
                    lastCookie->mExpireDate = 1;
                else
                    lastCookie->mExpireDate = time(0)+max_age;
            }
            else if (cName == "expires")
            {
                // Parse brain-dead netscape cookie-format
                lastCookie->mExpireDate = KRFCDate::parseDate(Value);

                // Workaround for servers that send the expiration date in
                // 'Wed Sep 12 07:00:00 2007 GMT' format. See BR# 145244.
                if (lastCookie->mExpireDate == 0)
                  lastCookie->mExpireDate = KRFCDate::parseDate(fixupDateTime(Value));
            }
            else if (cName == "path")
            {
                if (Value.isEmpty())
                   lastCookie->mPath = TQString::null; // Catch "" <> TQString::null
                else
                   lastCookie->mPath = KURL::decode_string(Value);
                lastCookie->mExplicitPath = true;
            }
            else if (cName == "version")
            {
                lastCookie->mProtocolVersion = Value.toInt();
            }
            else if ((cName == "secure") ||
                     (cName.isEmpty() && Value.lower() == L1("secure")))
            {
                lastCookie->mSecure = true;
            }
            else if ((cName == "httponly") ||
                     (cName.isEmpty() && Value.lower() == L1("httponly")))
            {
                lastCookie->mHttpOnly = true;
            }
        }

        if (*cookieStr == '\0')
            break; // End of header

        // Skip ';' or '\n'
        cookieStr++;
    }

    // RFC2965 cookies come last so that they override netscape cookies.
    while( !cookieList2.isEmpty() && (lastCookie = cookieList2.take(0)) )
    {
       removeDuplicateFromList(&cookieList, lastCookie, true);
       cookieList.append(lastCookie);
    }

    return cookieList;
}

/**
* Parses cookie_domstr and returns a linked list of KHttpCookie objects.
* cookie_domstr should be a semicolon-delimited list of "name=value"
* pairs. Any whitespace before "name" or around '=' is discarded.
* If no cookies are found, 0 is returned.
*/
KHttpCookieList KCookieJar::makeDOMCookies(const TQString &_url,
                                          const TQCString &cookie_domstring,
                                          long windowId)
{
    // A lot copied from above
    KHttpCookieList cookieList;
    KHttpCookiePtr lastCookie = 0;

    const char *cookieStr = cookie_domstring.data();
    TQString Name;
    TQString Value;
    TQString fqdn;
    TQString path;

    if (!parseURL(_url, fqdn, path))
    {
        // Error parsing _url
        return KHttpCookieList();
    }

    //  This time it's easy
    while(*cookieStr)
    {
        cookieStr = parseNameValue(cookieStr, Name, Value);

        // Host = FQDN
        // Default domain = ""
        // Default path = ""
        KHttpCookie *cookie = new KHttpCookie(fqdn, TQString::null, TQString::null,
                                Name, Value );
        if (windowId)
            cookie->mWindowIds.append(windowId);

        cookieList.append(cookie);
        lastCookie = cookie;

        if (*cookieStr != '\0')
            cookieStr++;         // Skip ';' or '\n'
     }

     return cookieList;
}

#ifdef MAX_COOKIE_LIMIT
static void makeRoom(KHttpCookieList *cookieList, KHttpCookiePtr &cookiePtr)
{
     // Too much cookies: throw one away, try to be somewhat clever
     KHttpCookiePtr lastCookie = 0;
     for(KHttpCookiePtr cookie = cookieList->first(); cookie; cookie = cookieList->next())
     {
         if (cookieList->compareItems(cookie, cookiePtr) < 0)
            break;
         lastCookie = cookie;
     }
     if (!lastCookie)
         lastCookie = cookieList->first();
     cookieList->removeRef(lastCookie);
}
#endif

//
// This function hands a KHttpCookie object over to the cookie jar.
//
// On return cookiePtr is set to 0.
//
void KCookieJar::addCookie(KHttpCookiePtr &cookiePtr)
{
    TQStringList domains;
    KHttpCookieList *cookieList = 0L;

    // We always need to do this to make sure that the
    // that cookies of type hostname == cookie-domainname
    // are properly removed and/or updated as necessary!
    extractDomains( cookiePtr->host(), domains );
    for ( TQStringList::ConstIterator it = domains.begin();
          (it != domains.end() && !cookieList);
          ++it )
    {
        TQString key = (*it).isNull() ? L1("") : (*it);
        KHttpCookieList *list= m_cookieDomains[key];
        if ( !list ) continue;

        removeDuplicateFromList(list, cookiePtr, false, true);
    }

    TQString domain = stripDomain( cookiePtr );
    TQString key = domain.isNull() ? L1("") : domain;
    cookieList = m_cookieDomains[ key ];
    if (!cookieList)
    {
        // Make a new cookie list
        cookieList = new KHttpCookieList();
        cookieList->setAutoDelete(true);

        // All cookies whose domain is not already
        // known to us should be added with KCookieDunno.
        // KCookieDunno means that we use the global policy.
        cookieList->setAdvice( KCookieDunno );

        m_cookieDomains.insert( domain, cookieList);

        // Update the list of domains
        m_domainList.append(domain);
    }

    // Add the cookie to the cookie list
    // The cookie list is sorted 'longest path first'
    if (!cookiePtr->isExpired(time(0)))
    {
#ifdef MAX_COOKIE_LIMIT
        if (cookieList->count() >= MAX_COOKIES_PER_HOST)
           makeRoom(cookieList, cookiePtr); // Delete a cookie
#endif
        cookieList->inSort( cookiePtr );
        m_cookiesChanged = true;
    }
    else
    {
        delete cookiePtr;
    }
    cookiePtr = 0;
}

//
// This function advices whether a single KHttpCookie object should
// be added to the cookie jar.
//
KCookieAdvice KCookieJar::cookieAdvice(KHttpCookiePtr cookiePtr)
{
    if (m_rejectCrossDomainCookies && cookiePtr->isCrossDomain())
       return KCookieReject;

    TQStringList domains;

    extractDomains(cookiePtr->host(), domains);

    // If the cookie specifies a domain, check whether it is valid. Otherwise,
    // accept the cookie anyways but remove the domain="" value to prevent
    // cross-site cookie injection.
    if (!cookiePtr->domain().isEmpty())
    {
      if (!domains.contains(cookiePtr->domain()) && 
          !cookiePtr->domain().endsWith("."+cookiePtr->host()))
          cookiePtr->fixDomain(TQString::null);
    }

    if (m_autoAcceptSessionCookies && (cookiePtr->expireDate() == 0 ||
        m_ignoreCookieExpirationDate))
       return KCookieAccept;

    KCookieAdvice advice = KCookieDunno;
    bool isFQDN = true; // First is FQDN
    TQStringList::Iterator it = domains.begin(); // Start with FQDN which first in the list.
    while( (advice == KCookieDunno) && (it != domains.end()))
    {
       TQString domain = *it;
       // Check if a policy for the FQDN/domain is set.
       if ( domain[0] == '.' || isFQDN )
       {
          isFQDN = false;
          KHttpCookieList *cookieList = m_cookieDomains[domain];
          if (cookieList)
             advice = cookieList->getAdvice();
       }
       domains.remove(it);
       it = domains.begin(); // Continue from begin of remaining list
    }

    if (advice == KCookieDunno)
        advice = m_globalAdvice;

    return advice;
}

//
// This function gets the advice for all cookies originating from
// _domain.
//
KCookieAdvice KCookieJar::getDomainAdvice(const TQString &_domain)
{
    KHttpCookieList *cookieList = m_cookieDomains[_domain];
    KCookieAdvice advice;

    if (cookieList)
    {
        advice = cookieList->getAdvice();
    }
    else
    {
        advice = KCookieDunno;
    }

    return advice;
}

//
// This function sets the advice for all cookies originating from
// _domain.
//
void KCookieJar::setDomainAdvice(const TQString &_domain, KCookieAdvice _advice)
{
    TQString domain(_domain);
    KHttpCookieList *cookieList = m_cookieDomains[domain];

    if (cookieList)
    {
        if (cookieList->getAdvice() != _advice)
        {
           m_configChanged = true;
           // domain is already known
           cookieList->setAdvice( _advice);
        }

        if ((cookieList->isEmpty()) &&
            (_advice == KCookieDunno))
        {
            // This deletes cookieList!
            m_cookieDomains.remove(domain);
            m_domainList.remove(domain);
        }
    }
    else
    {
        // domain is not yet known
        if (_advice != KCookieDunno)
        {
            // We should create a domain entry
            m_configChanged = true;
            // Make a new cookie list
            cookieList = new KHttpCookieList();
            cookieList->setAutoDelete(true);
            cookieList->setAdvice( _advice);
            m_cookieDomains.insert( domain, cookieList);
            // Update the list of domains
            m_domainList.append( domain);
        }
    }
}

//
// This function sets the advice for all cookies originating from
// the same domain as _cookie
//
void KCookieJar::setDomainAdvice(KHttpCookiePtr cookiePtr, KCookieAdvice _advice)
{
    TQString domain;
    stripDomain(cookiePtr->host(), domain); // We file the cookie under this domain.

    setDomainAdvice(domain, _advice);
}

//
// This function sets the global advice for cookies
//
void KCookieJar::setGlobalAdvice(KCookieAdvice _advice)
{
    if (m_globalAdvice != _advice)
       m_configChanged = true;
    m_globalAdvice = _advice;
}

//
// Get a list of all domains known to the cookie jar.
//
const TQStringList& KCookieJar::getDomainList()
{
    return m_domainList;
}

//
// Get a list of all cookies in the cookie jar originating from _domain.
//
const KHttpCookieList *KCookieJar::getCookieList(const TQString & _domain,
                                                 const TQString & _fqdn )
{
    TQString domain;

    if (_domain.isEmpty())
        stripDomain( _fqdn, domain );
    else
        domain = _domain;

    return m_cookieDomains[domain];
}

//
// Eat a cookie out of the jar.
// cookiePtr should be one of the cookies returned by getCookieList()
//
void KCookieJar::eatCookie(KHttpCookiePtr cookiePtr)
{
    TQString domain = stripDomain(cookiePtr); // We file the cookie under this domain.
    KHttpCookieList *cookieList = m_cookieDomains[domain];

    if (cookieList)
    {
        // This deletes cookiePtr!
        if (cookieList->removeRef( cookiePtr ))
           m_cookiesChanged = true;

        if ((cookieList->isEmpty()) &&
            (cookieList->getAdvice() == KCookieDunno))
        {
            // This deletes cookieList!
            m_cookieDomains.remove(domain);

            m_domainList.remove(domain);
        }
    }
}

void KCookieJar::eatCookiesForDomain(const TQString &domain)
{
   KHttpCookieList *cookieList = m_cookieDomains[domain];
   if (!cookieList || cookieList->isEmpty()) return;

   cookieList->clear();
   if (cookieList->getAdvice() == KCookieDunno)
   {
       // This deletes cookieList!
       m_cookieDomains.remove(domain);
       m_domainList.remove(domain);
   }
   m_cookiesChanged = true;
}

void KCookieJar::eatSessionCookies( long windowId )
{
    if (!windowId)
        return;

    TQStringList::Iterator it=m_domainList.begin();
    for ( ; it != m_domainList.end(); ++it )
        eatSessionCookies( *it, windowId, false );
}

void KCookieJar::eatAllCookies()
{
    for ( TQStringList::Iterator it=m_domainList.begin();
          it != m_domainList.end();)
    {
        TQString domain = *it++;
        // This might remove domain from domainList!
        eatCookiesForDomain(domain);
    }
}

void KCookieJar::eatSessionCookies( const TQString& fqdn, long windowId,
                                    bool isFQDN )
{
    KHttpCookieList* cookieList;
    if ( !isFQDN )
        cookieList = m_cookieDomains[fqdn];
    else
    {
        TQString domain;
        stripDomain( fqdn, domain );
        cookieList = m_cookieDomains[domain];
    }

    if ( cookieList )
    {
        KHttpCookiePtr cookie=cookieList->first();
        for (; cookie != 0;)
        {
            if ((cookie->expireDate() != 0) && !m_ignoreCookieExpirationDate)
            {
               cookie = cookieList->next();
               continue;
            }

            TQValueList<long> &ids = cookie->windowIds();
            if (!ids.remove(windowId) || !ids.isEmpty())
            {
               cookie = cookieList->next();
               continue;
            }
            KHttpCookiePtr old_cookie = cookie;
            cookie = cookieList->next();
            cookieList->removeRef( old_cookie );
        }
    }
}

//
// Saves all cookies to the file '_filename'.
// On succes 'true' is returned.
// On failure 'false' is returned.
bool KCookieJar::saveCookies(const TQString &_filename)
{
    KSaveFile saveFile(_filename, 0600);

    if (saveFile.status() != 0)
       return false;

    FILE *fStream = saveFile.fstream();

    time_t curTime = time(0);

    fprintf(fStream, "# KDE Cookie File v2\n#\n");

    fprintf(fStream, "%-20s %-20s %-12s %-10s %-4s %-20s %-4s %s\n",
                     "# Host", "Domain", "Path", "Exp.date", "Prot",
                     "Name", "Sec", "Value");

    for ( TQStringList::Iterator it=m_domainList.begin(); it != m_domainList.end();
          it++ )
    {
        const TQString &domain = *it;
        bool domainPrinted = false;

        KHttpCookieList *cookieList = m_cookieDomains[domain];
        KHttpCookiePtr cookie=cookieList->last();

        for (; cookie != 0;)
        {
            if (cookie->isExpired(curTime))
            {
                // Delete expired cookies
                KHttpCookiePtr old_cookie = cookie;
                cookie = cookieList->prev();
                cookieList->removeRef( old_cookie );
            }
            else if (cookie->expireDate() != 0 && !m_ignoreCookieExpirationDate)
            {
                if (!domainPrinted)
                {
                    domainPrinted = true;
                    fprintf(fStream, "[%s]\n", domain.local8Bit().data());
                }
                // Store persistent cookies
                TQString path = L1("\"");
                path += cookie->path();
                path += '"';
                TQString domain = L1("\"");
                domain += cookie->domain();
                domain += '"';
                fprintf(fStream, "%-20s %-20s %-12s %10lu  %3d %-20s %-4i %s\n",
                        cookie->host().latin1(), domain.latin1(),
                        path.latin1(), (unsigned long) cookie->expireDate(),
                        cookie->protocolVersion(),
                        cookie->name().isEmpty() ? cookie->value().latin1() : cookie->name().latin1(),
                        (cookie->isSecure() ? 1 : 0) + (cookie->isHttpOnly() ? 2 : 0) + 
                        (cookie->hasExplicitPath() ? 4 : 0) + (cookie->name().isEmpty() ? 8 : 0),
                        cookie->value().latin1());
                cookie = cookieList->prev();
            }
            else
            {
                // Skip session-only cookies
                cookie = cookieList->prev();
            }
        }
    }

    return saveFile.close();
}

typedef char *charPtr;

static const char *parseField(charPtr &buffer, bool keepQuotes=false)
{
    char *result;
    if (!keepQuotes && (*buffer == '\"'))
    {
        // Find terminating "
        buffer++;
        result = buffer;
        while((*buffer != '\"') && (*buffer))
            buffer++;
    }
    else
    {
        // Find first white space
        result = buffer;
        while((*buffer != ' ') && (*buffer != '\t') && (*buffer != '\n') && (*buffer))
            buffer++;
    }

    if (!*buffer)
        return result; //
    *buffer++ = '\0';

    // Skip white-space
    while((*buffer == ' ') || (*buffer == '\t') || (*buffer == '\n'))
        buffer++;

    return result;
}


//
// Reloads all cookies from the file '_filename'.
// On succes 'true' is returned.
// On failure 'false' is returned.
bool KCookieJar::loadCookies(const TQString &_filename)
{
    FILE *fStream = fopen( TQFile::encodeName(_filename), "r");
    if (fStream == 0)
    {
        return false;
    }

    time_t curTime = time(0);

    char *buffer = new char[READ_BUFFER_SIZE];

    bool err = false;
    err = (fgets(buffer, READ_BUFFER_SIZE, fStream) == 0);

    int version = 1;
    if (!err)
    {
        if (strcmp(buffer, "# KDE Cookie File\n") == 0)
        {
          // version 1
        }
        else if (sscanf(buffer, "# KDE Cookie File v%d\n", &version) != 1)
        {
          err = true;
        }
    }

    if (!err)
    {
        while(fgets(buffer, READ_BUFFER_SIZE, fStream) != 0)
        {
            char *line = buffer;
            // Skip lines which begin with '#' or '['
            if ((line[0] == '#') || (line[0] == '['))
                continue;

            const char *host( parseField(line) );
            const char *domain( parseField(line) );
            const char *path( parseField(line) );
            const char *expStr( parseField(line) );
            if (!expStr) continue;
            int expDate  = (time_t) strtoul(expStr, 0, 10);
            const char *verStr( parseField(line) );
            if (!verStr) continue;
            int protVer  = (time_t) strtoul(verStr, 0, 10);
            const char *name( parseField(line) );
            bool keepQuotes = false;
            bool secure = false;
            bool httpOnly = false;
            bool explicitPath = false;
            const char *value = 0;
            if ((version == 2) || (protVer >= 200))
            {
                if (protVer >= 200)
                    protVer -= 200;
                int i = atoi( parseField(line) );
                secure = i & 1;
                httpOnly = i & 2;
                explicitPath = i & 4;
                if (i & 8)
                   name = "";
                line[strlen(line)-1] = '\0'; // Strip LF.
                value = line;
            }
            else
            {
                if (protVer >= 100)
                {
                    protVer -= 100;
                    keepQuotes = true;
                }
                value = parseField(line, keepQuotes);
                secure = atoi( parseField(line) );
            }

            // Parse error
            if (!value) continue;

            // Expired or parse error
            if ((expDate == 0) || (expDate < curTime))
                continue;

            KHttpCookie *cookie = new KHttpCookie(TQString::fromLatin1(host),
                                                  TQString::fromLatin1(domain), 
                                                  TQString::fromLatin1(path), 
                                                  TQString::fromLatin1(name),
                                                  TQString::fromLatin1(value), 
                                                  expDate, protVer,
                                                  secure, httpOnly, explicitPath);
            addCookie(cookie);
        }
    }
    delete [] buffer;
    m_cookiesChanged = false;

    fclose( fStream);
    return err;
}

//
// Save the cookie configuration
//

void KCookieJar::saveConfig(TDEConfig *_config)
{
    if (!m_configChanged)
        return;

    _config->setGroup("Cookie Dialog");
    _config->writeEntry("PreferredPolicy", m_preferredPolicy);
    _config->writeEntry("ShowCookieDetails", m_showCookieDetails );
    _config->setGroup("Cookie Policy");
    _config->writeEntry("CookieGlobalAdvice", adviceToStr( m_globalAdvice));

    TQStringList domainSettings;
    for ( TQStringList::Iterator it=m_domainList.begin();
          it != m_domainList.end();
          it++ )
    {
         const TQString &domain = *it;
         KCookieAdvice advice = getDomainAdvice( domain);
         if (advice != KCookieDunno)
         {
             TQString value(domain);
             value += ':';
             value += adviceToStr(advice);
             domainSettings.append(value);
         }
    }
    _config->writeEntry("CookieDomainAdvice", domainSettings);
    _config->sync();
    m_configChanged = false;
}


//
// Load the cookie configuration
//

void KCookieJar::loadConfig(TDEConfig *_config, bool reparse )
{
    if ( reparse )
        _config->reparseConfiguration();

    _config->setGroup("Cookie Dialog");
    m_showCookieDetails = _config->readBoolEntry( "ShowCookieDetails" );
    m_preferredPolicy = _config->readNumEntry( "PreferredPolicy", 0 );

    _config->setGroup("Cookie Policy");
    TQStringList domainSettings = _config->readListEntry("CookieDomainAdvice");
    m_rejectCrossDomainCookies = _config->readBoolEntry( "RejectCrossDomainCookies", true );
    m_autoAcceptSessionCookies = _config->readBoolEntry( "AcceptSessionCookies", true );
    m_ignoreCookieExpirationDate = _config->readBoolEntry( "IgnoreExpirationDate", false );
    TQString value = _config->readEntry("CookieGlobalAdvice", L1("Ask"));
    m_globalAdvice = strToAdvice(value);

    // Reset current domain settings first.
    for ( TQStringList::Iterator it=m_domainList.begin(); it != m_domainList.end(); )
    {
         // Make sure to update iterator before calling setDomainAdvice()
         // setDomainAdvice() might delete the domain from domainList.
         TQString domain = *it++;
         setDomainAdvice(domain, KCookieDunno);
    }

    // Now apply the domain settings read from config file...
    for ( TQStringList::Iterator it=domainSettings.begin();
          it != domainSettings.end(); )
    {
        const TQString &value = *it++;

        int sepPos = value.findRev(':');

        if (sepPos <= 0)
          continue;

        TQString domain(value.left(sepPos));
        KCookieAdvice advice = strToAdvice( value.mid(sepPos + 1) );
        setDomainAdvice(domain, advice);
    }
}
