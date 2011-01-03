#ifndef __help_h__
#define __help_h__


#include <sys/types.h>
#include <sys/stat.h>


#include <stdio.h>
#include <unistd.h>


#include <tqintdict.h>
#include <tqstring.h>
#include <tqvaluelist.h>


#include <kio/global.h>
#include <kio/slavebase.h>

class HelpProtocol : public KIO::SlaveBase
{
public:

    HelpProtocol( bool ghelp, const TQCString &pool, const TQCString &app);
    virtual ~HelpProtocol() { }

    virtual void get( const KURL& url );

    virtual void mimetype( const KURL& url );

private:

    TQString langLookup(const TQString& fname);
    void emitFile( const KURL &url );
    void get_file( const KURL& url );
    TQString lookupFile(const TQString &fname, const TQString &query,
                       bool &redirect);

    void tqunicodeError( const TQString &t );

    TQString mParsed;
    bool mGhelp;
};


#endif
