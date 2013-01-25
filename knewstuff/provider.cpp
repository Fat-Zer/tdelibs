/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <kconfig.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <tqptrdict.h>
#include <tqwindowdefs.h>

#include "provider.h"
#include "provider.moc"

using namespace KNS;

// BCI for KDE 3.5 only

class ProviderPrivate
{
  public:
  ProviderPrivate(){}
  KURL mDownloadUrlLatest;
  KURL mDownloadUrlScore;
  KURL mDownloadUrlDownloads;
};

static TQPtrDict<ProviderPrivate> *d_ptr_prov = 0;

static ProviderPrivate *d_prov(const Provider *p)
{
  if(!d_ptr_prov)
  {
    d_ptr_prov = new TQPtrDict<ProviderPrivate>();
    d_ptr_prov->setAutoDelete(true);
  }
  ProviderPrivate *ret = d_ptr_prov->find((void*)p);
  if(!ret)
  {
    ret = new ProviderPrivate();
    d_ptr_prov->replace((void*)p, ret);
  }
  return ret;
}

KURL Provider::downloadUrlVariant( TQString variant ) const
{
  if((variant == "latest") && (d_prov(this)->mDownloadUrlLatest.isValid()))
	return d_prov(this)->mDownloadUrlLatest;
  if((variant == "score") && (d_prov(this)->mDownloadUrlScore.isValid()))
	return d_prov(this)->mDownloadUrlScore;
  if((variant == "downloads") && (d_prov(this)->mDownloadUrlDownloads.isValid()))
	return d_prov(this)->mDownloadUrlDownloads;

  return mDownloadUrl;
}

// BCI part ends here

Provider::Provider() : mNoUpload( false )
{
}

Provider::Provider( const TQDomElement &e ) : mNoUpload( false )
{
  parseDomElement( e );
}

Provider::~Provider()
{
    if (d_ptr_prov)
    {
        ProviderPrivate *p = d_ptr_prov->find(this);
        if (p)
            d_ptr_prov->remove(p);

        if (d_ptr_prov->isEmpty())
        {
            delete d_ptr_prov;
            d_ptr_prov = 0L;
        }
    }
}


void Provider::setName( const TQString &name )
{
  mName = name;
}

TQString Provider::name() const
{
  return mName;
}


void Provider::setIcon( const KURL &url )
{
  mIcon = url;
}

KURL Provider::icon() const
{
  return mIcon;
}


void Provider::setDownloadUrl( const KURL &url )
{
  mDownloadUrl = url;
}

KURL Provider::downloadUrl() const
{
  return mDownloadUrl;
}


void Provider::setUploadUrl( const KURL &url )
{
  mUploadUrl = url;
}

KURL Provider::uploadUrl() const
{
  return mUploadUrl;
}


void Provider::setNoUploadUrl( const KURL &url )
{
  mNoUploadUrl = url;
}

KURL Provider::noUploadUrl() const
{
  return mNoUploadUrl;
}


void Provider::setNoUpload( bool enabled )
{
  mNoUpload = enabled;
}

bool Provider::noUpload() const
{
  return mNoUpload;
}


void Provider::parseDomElement( const TQDomElement &element )
{
  if ( element.tagName() != "provider" ) return;

  setDownloadUrl( KURL( element.attribute("downloadurl") ) );
  setUploadUrl( KURL( element.attribute("uploadurl") ) );
  setNoUploadUrl( KURL( element.attribute("nouploadurl") ) );

  d_prov(this)->mDownloadUrlLatest = KURL( element.attribute("downloadurl-latest") );
  d_prov(this)->mDownloadUrlScore = KURL( element.attribute("downloadurl-score") );
  d_prov(this)->mDownloadUrlDownloads = KURL( element.attribute("downloadurl-downloads") );

  KURL iconurl( element.attribute("icon") );
  if(!iconurl.isValid()) iconurl.setPath( element.attribute("icon") );
  setIcon( iconurl );

  TQDomNode n;
  for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement p = n.toElement();
    
    if ( p.tagName() == "noupload" ) setNoUpload( true );
    if ( p.tagName() == "title" ) setName( p.text().stripWhiteSpace() );
  }
}

TQDomElement Provider::createDomElement( TQDomDocument &doc, TQDomElement &parent )
{
  TQDomElement entry = doc.createElement( "stuff" );
  parent.appendChild( entry );

  TQDomElement n = doc.createElement( "name" );
  n.appendChild( doc.createTextNode( name() ) );
  entry.appendChild( n );
  
  return entry;
}


ProviderLoader::ProviderLoader( TQWidget *parentWidget ) :
  TQObject( parentWidget )
{
  mProviders.setAutoDelete( true );
}

void ProviderLoader::load( const TQString &type, const TQString &providersList )
{
  kdDebug() << "ProviderLoader::load()" << endl;

  mProviders.clear();
  mJobData = "";

  TDEConfig *cfg = TDEGlobal::config();
  cfg->setGroup("KNewStuff");

  TQString providersUrl = providersList;
  if( providersUrl.isEmpty() )
  	providersUrl = cfg->readEntry( "ProvidersUrl" );

  if ( providersUrl.isEmpty() ) {
    // TODO: Replace the default by the real one.
    TQString server = cfg->readEntry( "MasterServer",
                                     "http://korganizer.kde.org" );
  
    providersUrl = server + "/knewstuff/" + type + "/providers.xml";
  }

  kdDebug() << "ProviderLoader::load(): providersUrl: " << providersUrl << endl;
  
  TDEIO::TransferJob *job = TDEIO::get( KURL( providersUrl ), false, false );
  connect( job, TQT_SIGNAL( result( TDEIO::Job * ) ),
           TQT_SLOT( slotJobResult( TDEIO::Job * ) ) );
  connect( job, TQT_SIGNAL( data( TDEIO::Job *, const TQByteArray & ) ),
           TQT_SLOT( slotJobData( TDEIO::Job *, const TQByteArray & ) ) );

//  job->dumpObjectInfo();
}

void ProviderLoader::slotJobData( TDEIO::Job *, const TQByteArray &data )
{
  kdDebug() << "ProviderLoader::slotJobData()" << endl;

  if ( data.size() == 0 ) return;

  TQCString str( data, data.size() + 1 );

  mJobData.append( TQString::fromUtf8( str ) );
}

void ProviderLoader::slotJobResult( TDEIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( TQT_TQWIDGET(parent()) );
  }

  kdDebug() << "--PROVIDERS-START--" << endl << mJobData << "--PROV_END--"
            << endl;

  TQDomDocument doc;
  if ( !doc.setContent( mJobData ) ) {
    KMessageBox::error( TQT_TQWIDGET(parent()), i18n("Error parsing providers list.") );
    return;
  }

  TQDomElement providers = doc.documentElement();

  if ( providers.isNull() ) {
    kdDebug() << "No document in Providers.xml." << endl;
  }

  TQDomNode n;
  for ( n = providers.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement p = n.toElement();
 
    if ( p.tagName() == "provider" ) {
      mProviders.append( new Provider( p ) );
    }
  }
  
  emit providersLoaded( &mProviders );
}
