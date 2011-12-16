/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#include "ksettings/pluginpage.h"
#include "kpluginselector.h"
#include <tqlayout.h>
#include <kdialog.h>
#include "ksettings/dispatcher.h"

namespace KSettings
{

class PluginPage::PluginPagePrivate
{
    public:
        PluginPagePrivate()
            : selwid( 0 )
        {
        }

        KPluginSelector * selwid;
};

    PluginPage::PluginPage( TQWidget * parent, const char * name, const TQStringList & args )
    : KCModule( parent, name, args )
    , d( new PluginPagePrivate )
{
    ( new TQVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    d->selwid = new KPluginSelector( this );
    connect( d->selwid, TQT_SIGNAL( changed( bool ) ), this, TQT_SIGNAL( changed( bool ) ) );
}

    PluginPage::PluginPage( KInstance * instance, TQWidget * parent, const TQStringList & args )
    : KCModule( instance, parent, args )
    , d( new PluginPagePrivate )
{
    ( new TQVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    d->selwid = new KPluginSelector( this );
    connect( d->selwid, TQT_SIGNAL( changed( bool ) ), this, TQT_SIGNAL( changed( bool ) ) );
    connect( d->selwid, TQT_SIGNAL( configCommitted( const TQCString & ) ),
            Dispatcher::self(), TQT_SLOT( reparseConfiguration( const TQCString & ) ) );
}

PluginPage::~PluginPage()
{
    delete d;
}

KPluginSelector * PluginPage::pluginSelector()
{
    return d->selwid;
}

void PluginPage::load()
{
    d->selwid->load();
}

void PluginPage::save()
{
    d->selwid->save();
}

void PluginPage::defaults()
{
    d->selwid->defaults();
}

} //namespace

#include "pluginpage.moc"
// vim: sw=4 sts=4 et
