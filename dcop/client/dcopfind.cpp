/*****************************************************************
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <tqbuffer.h>
#include <tqvariant.h>
#include <tqcolor.h>
#include <tqimage.h>
#include "../kdatastream.h"
#include "../dcopclient.h"
#include "../dcopref.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "marshall.cpp"

static DCOPClient* dcop = 0;
static bool bAppIdOnly = 0;
static bool bLaunchApp = 0;

bool findObject( const char* app, const char* obj, const char* func, QCStringList args )
{
    TQString f = func; // Qt is better with unicode strings, so use one.
    int left = f.find( '(' );
    int right = f.find( ')' );

    if ( right <  left )
    {
	tqWarning( "parentheses do not match" );
        exit(1);
    }

    if ( !f.isEmpty() && (left < 0) )
	f += "()";

    // This may seem expensive but is done only once per invocation
    // of dcop, so it should be OK.
    //
    //
    TQStringList intTypes;
    intTypes << "int" << "unsigned" << "long" << "bool" ;

    TQStringList types;
    if ( left >0 && left + 1 < right - 1) {
	types = TQStringList::split( ',', f.mid( left + 1, right - left - 1) );
	for ( TQStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    TQString lt = (*it).simplifyWhiteSpace();

	    int s = lt.find(' ');

	    // If there are spaces in the name, there may be two
	    // reasons: the parameter name is still there, ie.
	    // "TQString URL" or it's a complicated int type, ie.
	    // "unsigned long long int bool".
	    //
	    //
	    if ( s > 0 )
	    {
		TQStringList partl = TQStringList::split(' ' , lt);

		// The zero'th part is -- at the very least -- a
		// type part. Any trailing parts *might* be extra
		// int-type keywords, or at most one may be the
		// parameter name.
		//
		//
	    	s=1;

		while (s < (int)partl.count() && intTypes.contains(partl[s]))
		{
			s++;
		}

		if (s<(int)partl.count()-1)
		{
			tqWarning("The argument `%s' seems syntactically wrong.",
				lt.latin1());
		}
		if (s==(int)partl.count()-1)
		{
			partl.remove(partl.at(s));
		}

	    	lt = partl.join(" ");
		lt = lt.simplifyWhiteSpace();
	    }

	    (*it) = lt;
	}
	TQString fc = f.left( left );
	fc += '(';
	bool first = true;
	for ( TQStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    if ( !first )
		fc +=",";
	    first = false;
	    fc += *it;
	}
	fc += ')';
	f = fc;
    }

    if ( types.count() != args.count() ) {
	tqWarning( "arguments do not match" );
	exit(1);
    }

    TQByteArray data;
    TQDataStream arg(data, IO_WriteOnly);

    uint i = 0;
    for ( TQStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
        marshall(arg, args, i, *it);
    }
    if ( (uint) i != args.count() ) {
	tqWarning( "arguments do not match" );
	exit(1);
    }

    TQCString foundApp;
    TQCString foundObj;
    if ( dcop->findObject( app, obj, f.latin1(),  data, foundApp, foundObj) )
    {
       if (bAppIdOnly)
          puts(foundApp.data());
       else
          printf("DCOPRef(%s,%s)\n", qStringToC(foundApp), qStringToC(foundObj));
       return true;
    }
    return false;
}

bool launchApp(TQString app)
{
    int l = app.length();
    if (l && (app[l-1] == '*'))
       l--;
    if (l && (app[l-1] == '-'))
       l--;
    if (!l) return false;
    app.truncate(l);

    TQStringList URLs;
    TQByteArray data, replyData;
    TQCString replyType;
    TQDataStream arg(data, IO_WriteOnly);
    arg << app << URLs;

    if ( !dcop->call( "tdelauncher", "tdelauncher", "start_service_by_desktop_name(TQString,TQStringList)",
                      data, replyType, replyData) ) {
	tqWarning( "call to tdelauncher failed.");
        return false;
    }
    TQDataStream reply(replyData, IO_ReadOnly);

    if ( replyType != "serviceResult" )
    {
        tqWarning( "unexpected result '%s' from tdelauncher.", replyType.data());
        return false;
    }
    int result;
    TQCString dcopName;
    TQString error;
    reply >> result >> dcopName >> error;
    if (result != 0)
    {
        tqWarning("Error starting '%s': %s", app.local8Bit().data(), error.local8Bit().data());
        return false;
    }
    return true;
}

void usage()
{
   fprintf( stderr, "[dcopfind] Usage: dcopfind [-l] [-a] application [object [function [arg1] [arg2] [arg3] ... ] ] ] \n" );
   exit(0);
}

#ifdef Q_OS_WIN
# define main kdemain
#endif

int main( int argc, char** argv )
{
    int argi = 1;

    while ((argi < argc) && (argv[argi][0] == '-'))
    {
       switch ( argv[argi][1] ) {
       case 'l':
            bLaunchApp = true;
            break;
       case 'a':
            bAppIdOnly = true;
            break;
       default:
            usage();
       }
       argi++;
    }

    if (argc <= argi)
       usage();

    DCOPClient client;
    client.attach();
    dcop = &client;

    TQCString app;
    TQCString objid;
    TQCString function;
    char **args = 0;
    if ((argc > argi) && (strncmp(argv[argi], "DCOPRef(", 8)) == 0)
    {
       char *delim = strchr(argv[argi], ',');
       if (!delim)
       {
          fprintf(stderr, "Error: '%s' is not a valid DCOP reference.\n", argv[argi]);
          return 1;
       }
       *delim = 0;
       app = argv[argi++] + 8;
       delim++;
       delim[strlen(delim)-1] = 0;
       objid = delim;
    }
    else
    {
       if (argc > argi)
          app = argv[argi++];
       if (argc > argi)
          objid = argv[argi++];
    }
    if (argc > argi)
       function = argv[argi++];

    if (argc > argi)
    {
       args = &argv[argi];
       argc = argc-argi;
    }
    else
    {
       argc = 0;
    }

    QCStringList params;
    for( int i = 0; i < argc; i++ )
        params.append( args[ i ] );
    bool ok = findObject( app, objid, function, params );
    if (ok)
       return 0;
    if (bLaunchApp)
    {
       ok = launchApp(app);
       if (!ok)
          return 2;
       ok = findObject( app, objid, function, params );
    }

    return 1;
}
