/*  This file is part of the KDE project
    Copyright (C) 2004 Matthias Kretz <kretz@kde.org>

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

#include "factory.h"
#include "backend.h"
#include "player.h"
#include "videoplayer.h"
#include "channel.h"

#include <ktrader.h>
#include <kservice.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <tqfile.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kdebug.h>

namespace KDE
{
namespace Multimedia
{
class Factory::Private
{
	public:
		Private()
			: backend( 0 )
		{
			createBackend();
		}

		void createBackend()
		{
			KTrader::OfferList offers = KTrader::self()->query( "KDEMultimediaBackend", "Type == 'Service'" );
			KTrader::OfferListIterator it = offers.begin();
			KTrader::OfferListIterator end = offers.end();
			TQStringList errormsg;
			for( ; it != end; ++it )
			{
				KService::Ptr ptr = *it;
				KLibFactory * factory = KLibLoader::self()->factory( TQFile::encodeName( ptr->library() ) );
				if( factory )
				{
					backend = ( Backend* )factory->create( 0, "Multimedia Backend", "KDE::Multimedia::Backend" );
					if( 0 == backend )
					{
						TQString e = i18n( "create method returned 0" );
						errormsg.append( e );
						kdDebug( 600 ) << "Error getting backend from factory for " <<
							ptr->name() << ":\n" << e << endl;
					}
					else
					{
						service = ptr;
						kdDebug( 600 ) << "using backend: " << ptr->name() << endl;
						break;
					}
				}
				else
				{
					TQString e = KLibLoader::self()->lastErrorMessage();
					errormsg.append( e );
					kdDebug( 600 ) << "Error getting factory for " << ptr->name() <<
						":\n" << e << endl;
				}
			}
#if 0
			if( 0 == backend )
			{
				if( offers.size() == 0 )
					KMessageBox::error( 0, i18n( "Unable to find a Multimedia Backend" ) );
				else
				{
					TQString details = "<qt><table>";
					TQStringList::Iterator eit = errormsg.begin();
					TQStringList::Iterator eend = errormsg.end();
					KTrader::OfferListIterator oit = offers.begin();
					KTrader::OfferListIterator oend = offers.end();
					for( ; eit != eend || oit != oend; ++eit, ++oit )
						details += TQString( "<tr><td><b>%1</b></td><td>%2</td></tr>" )
							.arg( ( *oit )->name() ).arg( *eit );
					details += "</table></qt>";

					KMessageBox::detailedError( 0,
							i18n( "Unable to use any of the available Multimedia Backends" ), details );
				}
			}
#endif
		}

		Backend * backend;
		KService::Ptr service;

		TQValueList<void*> objects;
};

Factory * Factory::m_self = 0;

Factory * Factory::self()
{
	if( ! m_self )
		m_self = new Factory();
	return m_self;
}

Factory::Factory()
	: DCOPObject( "KDEMMFactory" )
	, d( new Private )
{
	connectDCOPSignal( 0, 0, "kdemmBackendChanged()", "kdemmBackendChanged()", false);
}

Factory::~Factory()
{
	delete d;
}

void Factory::kdemmBackendChanged()
{
	if( d->backend )
	{
		// wouw, if we want to switch on the fly we have to exchange the
		// (Video)Player and Channel classes without the API user noticing. That
		// would mean to implement a kind of smartwrapper:
		// Player: Interface that accesses Player_skel which is abstract and is
		// implemented by the Backend. The API user would only get access to the
		// Player class and if you want to switch the backend you can delete the
		// Player_skel object and put another implementation in its place.
		//
		// Now we tell the kdemm using app to help us. With the first signal we
		// tell 'em to delete all the (Video)Players and Channels and with the
		// second to recreate them all again.
		emit deleteYourObjects();
		if( d->objects.size() > 0 )
		{
			kdWarning( 600 ) << "we were asked to change the backend but the application did\n"
				"not free all references to objects created by the factory. Therefor we can not\n"
				"change the backend without crashing. Now we have to wait for a restart to make\n"
				"backendswitching possible." << endl;
			// in case there were objects deleted give 'em a chance to recreate
			// them now
			emit recreateObjects();
			return;
		}
		delete d->backend;
		d->backend = 0;
	}
	d->createBackend();
	emit recreateObjects();
}

void Factory::objectDestroyed( TQObject * obj )
{
	kdDebug( 600 ) << k_funcinfo << obj << endl;
	void * p = ( void* )obj;
	d->objects.remove( p );
}

Player * Factory::createPlayer()
{
	if( d->backend )
	{
		Player * p = d->backend->createPlayer();
		connect( p, TQT_SIGNAL( destroyed( TQObject* ) ), TQT_SLOT( objectDestroyed( TQObject* ) ) );
		d->objects.append( p );
		return p;
	}
	else
		return 0;
}

VideoPlayer * Factory::createVideoPlayer()
{
	if( d->backend )
	{
		VideoPlayer * vp = d->backend->createVideoPlayer();
		connect( vp, TQT_SIGNAL( destroyed( TQObject* ) ), TQT_SLOT( objectDestroyed( TQObject* ) ) );
		d->objects.append( vp );
		return vp;
	}
	else
		return 0;
}

bool Factory::playSoundEvent(const KURL & url)
{
	if( d->backend )
		return d->backend->playSoundEvent(url);
	else
		return false;
}

Channel * Factory::createChannel( const TQString & title, const TQString & channeltype,
		Channel::Direction direction )
{
	if( d->backend )
	{
		Channel * c = d->backend->createChannel( title, channeltype, direction );
		connect( c, TQT_SIGNAL( destroyed( TQObject* ) ), TQT_SLOT( objectDestroyed( TQObject* ) ) );
		d->objects.append( c );
		return c;
	}
	else
		return 0;
}

TQStringList Factory::availableChannels( Channel::Direction direction ) const
{
	if( d->backend )
		return d->backend->availableChannels( direction );
	else
		return TQStringList();
}

TQStringList Factory::playableMimeTypes() const
{
	if( d->backend )
		return d->backend->playableMimeTypes();
	else
		return TQStringList();
}

bool Factory::isMimeTypePlayable( const TQString & type ) const
{
	if( d->backend )
	{
		KMimeType::Ptr mimetype = KMimeType::mimeType( type );
		TQStringList mimetypes = playableMimeTypes();
		for( TQStringList::ConstIterator i=mimetypes.begin(); i!=mimetypes.end(); i++ )
			if( mimetype->is( *i ) )
				return true;
	}
	return false;
}

TQString Factory::backendName() const
{
	if( d->service )
		return d->service->name();
	else
		return TQString::null;
}

TQString Factory::backendComment() const
{
	if( d->service )
		return d->service->comment();
	else
		return TQString::null;
}

TQString Factory::backendVersion() const
{
	if( d->service )
		return d->service->property( "X-KDE-MMBackendInfo-Version" ).toString();
	else
		return TQString::null;
}

TQString Factory::backendIcon() const
{
	if( d->service )
		return d->service->icon();
	else
		return TQString::null;
}

TQString Factory::backendWebsite() const
{
	if( d->service )
		return d->service->property( "X-KDE-MMBackendInfo-Website" ).toString();
	else
		return TQString::null;
}

}} //namespaces

#include "factory.moc"

// vim: sw=4 ts=4 noet
