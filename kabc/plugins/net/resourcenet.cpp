/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqfile.h>

#include <kdebug.h>
#include <tdeio/netaccess.h>
#include <tdeio/scheduler.h>
#include <klocale.h>
#include <ksavefile.h>
#include <ktempfile.h>
#include <kurlrequester.h>

#include "addressbook.h"
#include "formatfactory.h"
#include "resourcenetconfig.h"
#include "stdaddressbook.h"

#include "resourcenet.h"

using namespace KABC;

class ResourceNet::ResourceNetPrivate
{
  public:
    TDEIO::Job *mLoadJob;
    bool mIsLoading;

    TDEIO::Job *mSaveJob;
    bool mIsSaving;

    TQString mLastErrorString;
};

ResourceNet::ResourceNet( const TDEConfig *config )
  : Resource( config ), mFormat( 0 ),
    mTempFile( 0 ),
    d( new ResourceNetPrivate )
{
  if ( config ) {
    init( KURL( config->readPathEntry( "NetUrl" ) ), config->readEntry( "NetFormat" ) );
  } else {
    init( KURL(), TQString("vcard").latin1() );
  }
}

ResourceNet::ResourceNet( const KURL &url, const TQString &format )
  : Resource( 0 ), mFormat( 0 ),
    mTempFile( 0 ),
    d( new ResourceNetPrivate )
{
  init( url, format );
}

void ResourceNet::init( const KURL &url, const TQString &format )
{
  d->mLoadJob = 0;
  d->mIsLoading = false;
  d->mSaveJob = 0;
  d->mIsSaving = false;

  mFormatName = format;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );
  if ( !mFormat ) {
    mFormatName = TQString("vcard").latin1();
    mFormat = factory->format( mFormatName );
  }

  setUrl( url );
}

ResourceNet::~ResourceNet()
{
  if ( d->mIsLoading )
    d->mLoadJob->kill();
  if ( d->mIsSaving )
    d->mSaveJob->kill();

  delete d;
  d = 0;

  delete mFormat;
  mFormat = 0;

  deleteLocalTempFile();
}

void ResourceNet::writeConfig( TDEConfig *config )
{
  Resource::writeConfig( config );

  config->writePathEntry( "NetUrl", mUrl.url() );
  config->writeEntry( "NetFormat", mFormatName );
}

Ticket *ResourceNet::requestSaveTicket()
{
  kdDebug(5700) << "ResourceNet::requestSaveTicket()" << endl;

  return createTicket( this );
}

void ResourceNet::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceNet::doOpen()
{
  return true;
}

void ResourceNet::doClose()
{
}

bool ResourceNet::load()
{
  TQString tempFile;

  if ( !TDEIO::NetAccess::download( mUrl, tempFile, 0 ) ) {
    addressBook()->error( i18n( "Unable to download file '%1'." ).arg( mUrl.prettyURL() ) );
    return false;
  }

  TQFile file( tempFile );
  if ( !file.open( IO_ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( tempFile ) );
    TDEIO::NetAccess::removeTempFile( tempFile );
    return false;
  }

  bool result = clearAndLoad( &file );
  if ( !result )
      addressBook()->error( i18n( "Problems during parsing file '%1'." ).arg( tempFile ) );

  TDEIO::NetAccess::removeTempFile( tempFile );

  return result;
}

bool ResourceNet::clearAndLoad( TQFile *file )
{
  clear();
  return mFormat->loadAll( addressBook(), this, file );
}

bool ResourceNet::asyncLoad()
{
  if ( d->mIsLoading ) {
    abortAsyncLoading();
  }

  if (d->mIsSaving) {
    kdWarning(5700) << "Aborted asyncLoad() because we're still asyncSave()ing!" << endl;
    return false;
  }

  bool ok = createLocalTempFile();
  if ( ok )
    mTempFile->sync();
    ok = mTempFile->close();

  if ( !ok ) {
    emit loadingError( this, i18n( "Unable to open file '%1'." ).arg( mTempFile->name() ) );
    deleteLocalTempFile();
    return false;
  }

  KURL dest;
  dest.setPath( mTempFile->name() );

  TDEIO::Scheduler::checkSlaveOnHold( true );
  d->mLoadJob = TDEIO::file_copy( mUrl, dest, -1, true, false, false );
  d->mIsLoading = true;
  connect( d->mLoadJob, TQT_SIGNAL( result( TDEIO::Job* ) ),
           this, TQT_SLOT( downloadFinished( TDEIO::Job* ) ) );

  return true;
}

void ResourceNet::abortAsyncLoading()
{
  kdDebug(5700) << "ResourceNet::abortAsyncLoading()" << endl;

  if ( d->mLoadJob ) {
    d->mLoadJob->kill(); // result not emitted
    d->mLoadJob = 0;
  }

  deleteLocalTempFile();
  d->mIsLoading = false;
}

void ResourceNet::abortAsyncSaving()
{
  kdDebug(5700) << "ResourceNet::abortAsyncSaving()" << endl;

  if ( d->mSaveJob ) {
    d->mSaveJob->kill(); // result not emitted
    d->mSaveJob = 0;
  }

  deleteLocalTempFile();
  d->mIsSaving = false;
}

bool ResourceNet::save( Ticket* )
{
  kdDebug(5700) << "ResourceNet::save()" << endl;

  if (d->mIsSaving) {
    abortAsyncSaving();
  }

  KTempFile tempFile;
  tempFile.setAutoDelete( true );
  bool ok = false;

  if ( tempFile.status() == 0 && tempFile.file() ) {
    saveToFile( tempFile.file() );
    tempFile.sync();
    ok = tempFile.close();
  }

  if ( !ok ) {
    addressBook()->error( i18n( "Unable to save file '%1'." ).arg( tempFile.name() ) );
    return false;
  }

  ok = TDEIO::NetAccess::upload( tempFile.name(), mUrl, 0 );
  if ( !ok )
    addressBook()->error( i18n( "Unable to upload to '%1'." ).arg( mUrl.prettyURL() ) );

  return ok;
}

bool ResourceNet::asyncSave( Ticket* )
{
  kdDebug(5700) << "ResourceNet::asyncSave()" << endl;

  if (d->mIsSaving) {
    abortAsyncSaving();
  }

  if (d->mIsLoading) {
    kdWarning(5700) << "Aborted asyncSave() because we're still asyncLoad()ing!" << endl;
    return false;
  }

  bool ok = createLocalTempFile();
  if ( ok ) {
    saveToFile( mTempFile->file() );
    mTempFile->sync();
    ok = mTempFile->close();
  }

  if ( !ok ) {
    emit savingError( this, i18n( "Unable to save file '%1'." ).arg( mTempFile->name() ) );
    deleteLocalTempFile();
    return false;
  }

  KURL src;
  src.setPath( mTempFile->name() );

  TDEIO::Scheduler::checkSlaveOnHold( true );
  d->mIsSaving = true;
  d->mSaveJob = TDEIO::file_copy( src, mUrl, -1, true, false, false );
  connect( d->mSaveJob, TQT_SIGNAL( result( TDEIO::Job* ) ),
           this, TQT_SLOT( uploadFinished( TDEIO::Job* ) ) );

  return true;
}

bool ResourceNet::createLocalTempFile()
{
  deleteStaleTempFile();
  mTempFile = new KTempFile();
  mTempFile->setAutoDelete( true );
  return mTempFile->status() == 0;
}

void ResourceNet::deleteStaleTempFile()
{
  if ( hasTempFile() ) {
    kdDebug(5700) << "stale temp file detected " << mTempFile->name() << endl;
    deleteLocalTempFile();
  }
}

void ResourceNet::deleteLocalTempFile()
{
  delete mTempFile;
  mTempFile = 0;
}

void ResourceNet::saveToFile( TQFile *file )
{
  mFormat->saveAll( addressBook(), this, file );
}

void ResourceNet::setUrl( const KURL &url )
{
  mUrl = url;
}

KURL ResourceNet::url() const
{
  return mUrl;
}

void ResourceNet::setFormat( const TQString &name )
{
  mFormatName = name;
  if ( mFormat )
    delete mFormat;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );
}

TQString ResourceNet::format() const
{
  return mFormatName;
}

void ResourceNet::downloadFinished( TDEIO::Job* )
{
  kdDebug(5700) << "ResourceNet::downloadFinished()" << endl;

  d->mIsLoading = false;

  if ( !hasTempFile() || mTempFile->status() != 0 ) {
    d->mLastErrorString = i18n( "Download failed: Unable to create temporary file" );
    TQTimer::singleShot( 0, this, TQT_SLOT( signalError() ) );
    return;
  }

  TQFile file( mTempFile->name() );
  if ( file.open( IO_ReadOnly ) ) {
    if ( clearAndLoad( &file ) )
      emit loadingFinished( this );
    else
      emit loadingError( this, i18n( "Problems during parsing file '%1'." ).arg( mTempFile->name() ) );
  }
  else {
    emit loadingError( this, i18n( "Unable to open file '%1'." ).arg( mTempFile->name() ) );
  }

  deleteLocalTempFile();
}

void ResourceNet::uploadFinished( TDEIO::Job *job )
{
  kdDebug(5700) << "ResourceFile::uploadFinished()" << endl;

  d->mIsSaving = false;

  if ( job->error() )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );

  deleteLocalTempFile();
}

void ResourceNet::signalError()
{
  emit loadingError( this, d->mLastErrorString );
  d->mLastErrorString.truncate( 0 );
}

#include "resourcenet.moc"
