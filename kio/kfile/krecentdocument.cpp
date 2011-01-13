/* -*- c++ -*-
 * Copyright (C)2000 Daniel M. Duley <mosfet@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <krecentdocument.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kurl.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kdesktopfile.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqtextstream.h>
#include <tqstringlist.h>
#include <tqregexp.h>

#include <sys/types.h>
#include <utime.h>

TQString KRecentDocument::recentDocumentDirectory()
{
    // need to change this path, not sure where
    return locateLocal("data", TQString::tqfromLatin1("RecentDocuments/"));
}

TQStringList KRecentDocument::recentDocuments()
{
    TQDir d(recentDocumentDirectory(), "*.desktop", TQDir::Time,
           TQDir::Files | TQDir::Readable | TQDir::Hidden);

    if (!d.exists())
        d.mkdir(recentDocumentDirectory());

    TQStringList list = d.entryList();
    TQStringList fullList;

    for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
       TQString pathDesktop = d.absFilePath( *it );
       KDesktopFile tmpDesktopFile( pathDesktop, false);
       KURL urlDesktopFile(tmpDesktopFile.readURL());
       if( urlDesktopFile.isLocalFile() && !TQFile(urlDesktopFile.path()).exists())
           d.remove(pathDesktop);
       else
           fullList.append( pathDesktop );
    }

    return fullList;
}

void KRecentDocument::add(const KURL& url)
{
    KRecentDocument::add(url, tqApp->argv()[0]); // ### argv[0] might not match the service filename!
}

void KRecentDocument::add(const KURL& url, const TQString& desktopEntryName)
{
	if ( url.isLocalFile() && !KGlobal::dirs()->relativeLocation("tmp", url.path()).startsWith("/"))
		return;

    TQString openStr = url.url();
    openStr.replace( TQRegExp("\\$"), "$$" ); // Desktop files with type "Link" are $-variable expanded

    kdDebug(250) << "KRecentDocument::add for " << openStr << endl;
    KConfig *config = KGlobal::config();
    TQString oldGrp = config->group();
    config->setGroup(TQString::tqfromLatin1("RecentDocuments"));
    bool useRecent = config->readBoolEntry(TQString::tqfromLatin1("UseRecent"), true);
    int maxEntries = config->readNumEntry(TQString::tqfromLatin1("MaxEntries"), 10);

    config->setGroup(oldGrp);
    if(!useRecent)
        return;

    TQString path = recentDocumentDirectory();

    TQString dStr = path + url.fileName();

    TQString ddesktop = dStr + TQString::tqfromLatin1(".desktop");

    int i=1;
    // check for duplicates
    while(TQFile::exists(ddesktop)){
        // see if it points to the same file and application
        KSimpleConfig tmp(ddesktop);
        tmp.setDesktopGroup();
        if(tmp.readEntry(TQString::tqfromLatin1("X-KDE-LastOpenedWith"))
	   == desktopEntryName)
	{
            utime(TQFile::encodeName(ddesktop), NULL);
            return;
        }
        // if not append a (num) to it
        ++i;
        if ( i > maxEntries )
            break;
        ddesktop = dStr + TQString::tqfromLatin1("[%1].desktop").arg(i);
    }

    TQDir dir(path);
    // check for max entries, delete oldest files if exceeded
    TQStringList list = dir.entryList(TQDir::Files | TQDir::Hidden, TQDir::Time | TQDir::Reversed);
    i = list.count();
    if(i > maxEntries-1){
        TQStringList::Iterator it;
        it = list.begin();
        while(i > maxEntries-1){
            TQFile::remove(dir.absPath() + TQString::tqfromLatin1("/") + (*it));
            --i, ++it;
        }
    }

    // create the applnk
    KSimpleConfig conf(ddesktop);
    conf.setDesktopGroup();
    conf.writeEntry( TQString::tqfromLatin1("Type"), TQString::tqfromLatin1("Link") );
    conf.writePathEntry( TQString::tqfromLatin1("URL"), openStr );
    // If you change the line below, change the test in the above loop
    conf.writeEntry( TQString::tqfromLatin1("X-KDE-LastOpenedWith"), desktopEntryName );
    TQString name = url.fileName();
    if (name.isEmpty())
      name = openStr;
    conf.writeEntry( TQString::tqfromLatin1("Name"), name );
    conf.writeEntry( TQString::tqfromLatin1("Icon"), KMimeType::iconForURL( url ) );
}

void KRecentDocument::add(const TQString &openStr, bool isUrl)
{
    if( isUrl ) {
        add( KURL( openStr ) );
    } else {
        KURL url;
        url.setPath( openStr );
        add( url );
    }
}

void KRecentDocument::clear()
{
  TQStringList list = recentDocuments();
  TQDir dir;
  for(TQStringList::Iterator it = list.begin(); it != list.end() ; ++it)
    dir.remove(*it);
}

int KRecentDocument::maximumItems()
{
    KConfig *config = KGlobal::config();
    KConfigGroupSaver sa(config, TQString::tqfromLatin1("RecentDocuments"));
    return config->readNumEntry(TQString::tqfromLatin1("MaxEntries"), 10);
}


