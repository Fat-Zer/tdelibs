/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "khtml_factory.h"
#include "khtml_part.h"
#include "khtml_settings.h"

#include "css/cssstyleselector.h"
#include "html/html_imageimpl.h"
#include "rendering/render_style.h"
#include "rendering/break_lines.h"
#include "misc/loader.h"
#include "misc/arena.h"

#include <kinstance.h>
#include <kaboutdata.h>
#include <klocale.h>

#include <assert.h>

#include <kdebug.h>

template class TQPtrList<KHTMLPart>;

extern "C" KDE_EXPORT void *init_libkhtml()
{
    // We can't use a plain self() here, because that would
    // return the global factory, which might already exist
    // at the time init_libkhtml is called! As soon as someone
    // does new KHTMLPart() in his application and loads up
    // an html document into that part which either embeds
    // embeds another KHTMLPart instance via <object> or
    // as html frame, then we cannot return self(), as
    // what we return here is what the KLibLoader deletes
    // in the end, and we don't want the libloader to
    // delete our global instance. Anyway, the new
    // KHTMLFactory we create here is very cheap :)
    // (Simon)
    return new KHTMLFactory( true );
}

KHTMLFactory *KHTMLFactory::s_self = 0;
unsigned long int KHTMLFactory::s_refcnt = 0;
KInstance *KHTMLFactory::s_instance = 0;
KAboutData *KHTMLFactory::s_about = 0;
KHTMLSettings *KHTMLFactory::s_settings = 0;
TQPtrList<KHTMLPart> *KHTMLFactory::s_parts = 0;
TQString *KHTMLSettings::avFamilies = 0;

KHTMLFactory::KHTMLFactory( bool clone )
{
    if ( clone )
        ref();
}

KHTMLFactory::~KHTMLFactory()
{
    if ( s_self == this )
    {
        assert( !s_refcnt );

        delete s_instance;
        delete s_about;
        delete s_settings;
	delete KHTMLSettings::avFamilies;
        if ( s_parts )
        {
            assert( s_parts->isEmpty() );
            delete s_parts;
        }

        s_instance = 0;
        s_about = 0;
        s_settings = 0;
        s_parts = 0;
	KHTMLSettings::avFamilies = 0;

        // clean up static data
        khtml::CSSStyleSelector::clear();
        khtml::RenderStyle::cleanup();
        khtml::Cache::clear();
        khtml::cleanup_thaibreaks();
        khtml::ArenaFinish();
    }
    else
        deref();
}

KParts::Part *KHTMLFactory::createPartObject( TQWidget *parentWidget, const char *widgetName, TQObject *parent, const char *name, const char *className, const TQStringList & )
{
  KHTMLPart::GUIProfile prof = KHTMLPart::DefaultGUI;
  if ( strcmp( className, "Browser/View" ) == 0 )
    prof = KHTMLPart::BrowserViewGUI;

  return new KHTMLPart( parentWidget, widgetName, parent, name, prof );
}

void KHTMLFactory::ref()
{
    if ( !s_refcnt && !s_self )
    {
        // we can't use a staticdeleter here, because that would mean
        // that the factory gets deleted from within a qPostRoutine, called
        // from the TQApplication destructor. That however is too late, because
        // we want to destruct a KInstance object, which involves destructing
        // a KConfig object, which might call KGlobal::dirs() (in sync()) which
        // probably is not going to work ;-)
        // well, perhaps I'm wrong here, but as I'm unsure I try to stay on the
        // safe side ;-) -> let's use a simple reference counting scheme
        // (Simon)
        s_self = new KHTMLFactory;
        khtml::Cache::init();
    }

    s_refcnt++;
}

void KHTMLFactory::deref()
{
    if ( !--s_refcnt && s_self )
    {
        delete s_self;
        s_self = 0;
    }
}

void KHTMLFactory::registerPart( KHTMLPart *part )
{
    if ( !s_parts )
        s_parts = new TQPtrList<KHTMLPart>;

    if ( !s_parts->tqcontainsRef( part ) )
    {
        s_parts->append( part );
        ref();
    }
}

void KHTMLFactory::deregisterPart( KHTMLPart *part )
{
    assert( s_parts );

    if ( s_parts->removeRef( part ) )
    {
        if ( s_parts->isEmpty() )
        {
            delete s_parts;
            s_parts = 0;
        }
        deref();
    }
}

KInstance *KHTMLFactory::instance()
{
  assert( s_self );

  if ( !s_instance )
  {
    s_about = new KAboutData( "khtml", I18N_NOOP( "KHTML" ), "4.0",
                              I18N_NOOP( "Embeddable HTML component" ),
                              KAboutData::License_LGPL );
    s_about->addAuthor( "Lars Knoll", 0, "knoll@kde.org" );
    s_about->addAuthor( "Antti Koivisto", 0, "koivisto@kde.org" );
    s_about->addAuthor( "Waldo Bastian", 0, "bastian@kde.org" );
    s_about->addAuthor( "Dirk Mueller", 0, "mueller@kde.org" );
    s_about->addAuthor( "Peter Kelly", 0, "pmk@kde.org" );
    s_about->addAuthor( "Torben Weis", 0, "weis@kde.org" );
    s_about->addAuthor( "Martin Jones", 0, "mjones@kde.org" );
    s_about->addAuthor( "Simon Hausmann", 0, "hausmann@kde.org" );
    s_about->addAuthor( "Tobias Anton", 0, "anton@stud.fbi.fh-darmstadt.de" );

    s_instance = new KInstance( s_about );
  }

  return s_instance;
}

KHTMLSettings *KHTMLFactory::defaultHTMLSettings()
{
  assert( s_self );
  if ( !s_settings )
    s_settings = new KHTMLSettings();

  return s_settings;
}

using namespace KParts;
#include "khtml_factory.moc"

