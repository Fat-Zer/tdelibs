/* This file is part of the KDE project
   Copyright (c) 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>

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

#include <tqdict.h>
#include <tqfile.h>
#include <tqtextstream.h>

#include <kdirwatch.h>
#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kconfig.h>

#include "knfsshare.h"

class KNFSSharePrivate
{
public:
  KNFSSharePrivate();
  
  bool readExportsFile();
  bool findExportsFile();
  
  TQDict<bool> sharedPaths;
  TQString exportsFile;
};

KNFSSharePrivate::KNFSSharePrivate() 
{
  if (findExportsFile())
      readExportsFile();
}  

/**
 * Try to find the nfs config file path
 * First tries the kconfig, then checks
 * several well-known paths
 * @return wether an 'exports' file was found.
 **/
bool KNFSSharePrivate::findExportsFile() {
  KConfig config("knfsshare");
  config.setGroup("General");
  exportsFile = config.readPathEntry("exportsFile");

  if ( TQFile::exists(exportsFile) )
    return true;

  if ( TQFile::exists("/etc/exports") )
    exportsFile = "/etc/exports";
  else {
    kdDebug(7000) << "KNFSShare: Could not found exports file!" << endl;
    return false;
  }
      
  config.writeEntry("exportsFile",exportsFile);
  return true;
}

/**
 * Reads all paths from the exports file
 * and fills the sharedPaths dict with the values
 */
bool KNFSSharePrivate::readExportsFile() {
  TQFile f(exportsFile);

  kdDebug(7000) << "KNFSShare::readExportsFile " << exportsFile << endl;
  
  if (!f.open(IO_ReadOnly)) {
    kdError() << "KNFSShare: Could not open " << exportsFile << endl;
    return false;
  }
  
 
  sharedPaths.clear();

  TQTextStream s( &f );
  
  bool continuedLine = false; // is true if the line before ended with a backslash
  TQString completeLine;
  
  while ( !s.eof() )
  {
    TQString currentLine = s.readLine().stripWhiteSpace();

    if (continuedLine) {
      completeLine += currentLine;
      continuedLine = false;
    }      
    else
      completeLine = currentLine;

    // is the line continued in the next line ?
    if ( completeLine[completeLine.length()-1] == '\\' )
    {
      continuedLine = true;
      // remove the ending backslash
      completeLine.truncate( completeLine.length()-1 ); 
      continue;
    }
    
    // comments or empty lines
    if (completeLine.isEmpty() ||
        '#' == completeLine[0])
    {
      continue;
    }

    TQString path;
    
    // Handle quotation marks
    if ( completeLine[0] == '"' ) {
      int i = completeLine.tqfind('"',1);
      if (i == -1) {
        kdError() << "KNFSShare: Parse error: Missing quotation mark: " << completeLine << endl;   
        continue;
      }
      path = completeLine.mid(1,i-1);
      
    } else { // no quotation marks
      int i = completeLine.tqfind(' ');
      if (i == -1)
          i = completeLine.tqfind('\t');
          
      if (i == -1) 
        path = completeLine;
      else 
        path = completeLine.left(i);
      
    }        
    
    kdDebug(7000) << "KNFSShare: Found path: " << path << endl;
    
    // normalize path
    if ( path[path.length()-1] != '/' )
             path += '/';
    
    bool b = true;             
    sharedPaths.insert(path,&b);             
  }

  f.close();

  return true;  

}

KNFSShare::KNFSShare() {
  d = new KNFSSharePrivate();
  if (TQFile::exists(d->exportsFile)) {
    KDirWatch::self()->addFile(d->exportsFile);
    connect(KDirWatch::self(), TQT_SIGNAL(dirty (const TQString&)),this,
   	        TQT_SLOT(slotFileChange(const TQString&)));
  }
}

KNFSShare::~KNFSShare() {
  if (TQFile::exists(d->exportsFile)) {
    KDirWatch::self()->removeFile(d->exportsFile);
  }
  delete d;
}


bool KNFSShare::isDirectoryShared( const TQString & path ) const {
  TQString fixedPath = path;
  if ( path[path.length()-1] != '/' )
       fixedPath += '/';
  
  return d->sharedPaths.tqfind(fixedPath) != 0;
}

TQStringList KNFSShare::sharedDirectories() const {
  TQStringList result;
  TQDictIterator<bool> it(d->sharedPaths);
  for( ; it.current(); ++it )
      result << it.currentKey();
      
  return result;       
}

TQString KNFSShare::exportsPath() const {
  return d->exportsFile;
}



void KNFSShare::slotFileChange( const TQString & path ) {
  if (path == d->exportsFile)
     d->readExportsFile();
     
  emit changed();     
}

KNFSShare* KNFSShare::_instance = 0L; 
static KStaticDeleter<KNFSShare> ksdNFSShare;

KNFSShare* KNFSShare::instance() {
  if (! _instance ) 
      _instance = ksdNFSShare.setObject(_instance, new KNFSShare());
      
  return _instance;      
}

#include "knfsshare.moc"

