/*
    This file is part of libtderesources.

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

#include <tqlayout.h>

#include <tdeaboutdata.h>
#include <kgenericfactory.h>
#include <tdelocale.h>

#include "configpage.h"

#include "kcmtderesources.h"

typedef KGenericFactory<KCMKResources, TQWidget> ResourcesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_tderesources, ResourcesFactory( "kcmtderesources" ) )

KCMKResources::KCMKResources( TQWidget *parent, const char *name, const TQStringList& )
  : TDECModule( ResourcesFactory::instance(), parent, name )
{
  TQVBoxLayout *layout = new TQVBoxLayout( this );
  mConfigPage = new KRES::ConfigPage( this );
  layout->addWidget( mConfigPage );
  connect( mConfigPage, TQT_SIGNAL( changed( bool ) ), TQT_SIGNAL( changed( bool ) ) );
  setButtons( Help | Apply );
  TDEAboutData *about =
   new TDEAboutData( I18N_NOOP( "kcmtderesources" ),
                   I18N_NOOP( "TDE Resources configuration module" ),
                   0, 0, TDEAboutData::License_GPL,
                   I18N_NOOP( "(c) 2003 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  setAboutData(about);
}

void KCMKResources::load()
{
  mConfigPage->load();
}

void KCMKResources::save()
{
  mConfigPage->save();
}

void KCMKResources::defaults()
{
  mConfigPage->defaults();
}

#include "kcmtderesources.moc"
