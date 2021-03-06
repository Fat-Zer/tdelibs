/*****************************************************************
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

#include "../kdatastream.h"
#include "../dcopclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <tqstringlist.h>

static DCOPClient* dcop = 0;

void startApp(const char *_app, int argc, const char **args)
{
    const char *function = 0;
    TQString app = TQString::fromLatin1(_app);
    if (app.endsWith(".desktop"))
       function = "start_service_by_desktop_path(TQString,TQStringList)";
    else
       function = "start_service_by_desktop_name(TQString,TQStringList)";
    TQStringList URLs;
    for(int i = 0; i < argc; i++)
    {
       URLs.append(TQString::fromLocal8Bit(args[i]));
    }

    TQByteArray data, replyData;
    TQCString replyType;
    TQDataStream arg(data, IO_WriteOnly);
    arg << app << URLs;

    if ( !dcop->call( "tdelauncher", "tdelauncher", function,  data, replyType, replyData) ) {
	tqWarning( "call failed");
        exit(1);
    } else {
	TQDataStream reply(replyData, IO_ReadOnly);

        if ( replyType != "serviceResult" )
        {
            tqWarning( "unexpected result '%s'", replyType.data());
            exit(1);
        }
        int result;
        TQCString dcopName;
        TQString error;
        reply >> result >> dcopName >> error;
        if (result != 0)
        {
            tqWarning("Error: %s", error.local8Bit().data());
            exit(1);
        }
        if (!dcopName.isEmpty())
            puts(dcopName.data());
    }
}

#ifdef Q_OS_WIN
# define main kdemain
#endif

int main( int argc, char** argv )
{
    if (( argc < 2) || (argv[1][0] == '-' )) {
	fprintf( stderr, "Usage: dcopstart <application> [url1] [url2] ...\n" );
	exit(0);
    }

    DCOPClient client;
    client.attach();
    dcop = &client;

    TQCString app;
    TQCString objid;
    TQCString function;
    /*char **args = 0;*/
    startApp( argv[1], argc - 2, (const char**)&argv[2] );

    return 0;
}
