/*
 *  Copyright (C) 2002, 2003 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "ktar.h"
#include <stdio.h>
#include <kinstance.h>
#include <tqfile.h>

#include <assert.h>

void recursive_print( const KTarDirectory * dir, const TQString & path )
{
  TQStringList l = dir->entries();
  TQStringList::Iterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    const KTarEntry* entry = dir->entry( (*it) );
    printf("mode=%07o %s %s %s%s %d isdir=%d\n", entry->permissions(), entry->user().latin1(), entry->group().latin1(), path.latin1(), (*it).latin1(),
           entry->isFile() ? static_cast<const KArchiveFile *>(entry)->size() : 0,
           entry->isDirectory());
    if (entry->isDirectory())
      recursive_print( (KTarDirectory *)entry, path+(*it)+"/" );
  }
}

int main( int argc, char** argv )
{
  if (argc != 3)
  {
    printf("\n"
 " Usage :\n"
 " ./ktartest list /path/to/existing_file.tar.gz       tests listing an existing tar.gz\n"
 " ./ktartest readwrite newfile.tar.gz                 will create the tar.gz, then close and reopen it.\n"
 " ./ktartest maxlength newfile.tar.gz                 tests the maximum filename length allowed.\n"
 " ./ktartest iodevice /path/to/existing_file.tar.gz   tests KArchiveFile::device()\n");
    return 1;
  }
  TDEInstance instance("ktartest");
  TQString command = argv[1];
  if ( command == "list" )
  {
    KTarGz tar( argv[2] );

    if ( !tar.open( IO_ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }

    const KTarDirectory* dir = tar.directory();

    //printf("calling recursive_print\n");
    recursive_print( dir, "" );
    //printf("recursive_print called\n");

    tar.close();

    return 0;
  }
  else if (command == "readwrite" )
  {
    KTarGz tar( argv[2] );

    if ( !tar.open( IO_WriteOnly ) )
    {
      printf("Could not open %s for writing\n", argv[1]);
      return 1;
    }

    tar.writeFile( "empty", "weis", "users", 0, "" );
    tar.writeFile( "test1", "weis", "users", 5, "Hallo" );
    tar.writeFile( "test2", "weis", "users", 8, "Hallo Du" );
    tar.writeFile( "mydir/test3", "weis", "users", 13, "Noch so einer" );
    tar.writeFile( "my/dir/test3", "dfaure", "hackers", 29, "I don't speak German (David)" );

#define SIZE1 100
    // Now a medium file : 100 null bytes
    char medium[ SIZE1 ];
    memset( medium, 0, SIZE1 );
    tar.writeFile( "mediumfile", "user", "group", SIZE1, medium );
    // Another one, with an absolute path
    tar.writeFile( "/dir/subdir/mediumfile2", "user", "group", SIZE1, medium );

    // Now a huge file : 20000 null bytes
    int n = 20000;
    char * huge = new char[ n ];
    memset( huge, 0, n );
    tar.writeFile( "hugefile", "user", "group", n, huge );
    delete [] huge;

    tar.close();

    printf("-----------------------\n");

    if ( !tar.open( IO_ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[1] );
      return 1;
    }

    const KTarDirectory* dir = tar.directory();
    recursive_print(dir, "");

    const KTarEntry* e = dir->entry( "mydir/test3" );
    Q_ASSERT( e && e->isFile() );
    const KTarFile* f = (KTarFile*)e;

    TQByteArray arr( f->data() );
    printf("SIZE=%i\n",arr.size() );
    TQString str( arr );
    printf("DATA=%s\n", str.latin1());

    tar.close();

    return 0;
  }
  else if ( command == "maxlength" )
  {
    KTarGz tar( argv[2] );

    if ( !tar.open( IO_WriteOnly ) )
    {
      printf("Could not open %s for writing\n", argv[1]);
      return 1;
    }
    // Generate long filenames of each possible length bigger than 98...
    // Also exceed 512 byte block size limit to see how well the ././@LongLink
    // implementation fares
    for (int i = 98; i < 514 ; i++ )
    {
      TQString str, num;
      str.fill( 'a', i-10 );
      num.setNum( i );
      num = num.rightJustify( 10, '0' );
      tar.writeFile( str+num, "testu", "testg", 3, "hum" );
    }
    // Result of this test : works perfectly now (failed at 482 formerly and
    // before that at 154).
    tar.close();
    printf("Now run 'tar tvzf %s'\n", argv[2]);
    return 0;
  }
  else if ( command == "iodevice" )
  {
    KTarGz tar( argv[2] );
    if ( !tar.open( IO_ReadOnly ) )
      return 1;
    const KTarDirectory* dir = tar.directory();
    assert(dir);
    const KTarEntry* entry = dir->entry( "my/dir/test3" );
    if ( entry && entry->isFile() )
    {
        TQIODevice *dev = static_cast<const KTarFile *>(entry)->device();
        TQByteArray contents = dev->readAll();
        printf("contents='%s'\n", TQCString(contents, contents.size()+1 ).data());
    } else
        printf("entry=%p - not found if 0, otherwise not a file\n", (void*)entry);
    return 0;
  }
  else
    printf("Unknown command\n");
}

