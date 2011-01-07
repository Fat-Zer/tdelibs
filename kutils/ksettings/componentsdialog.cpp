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

#include "ksettings/componentsdialog.h"
#include <klocale.h>
#include <tqlayout.h>
#include <klistview.h>
#include <tqlabel.h>
#include <tqheader.h>
#include <kplugininfo.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kseparator.h>

namespace KSettings
{

class ComponentsDialog::ComponentsDialogPrivate
{
    public:
        KListView * listview;
        TQFrame * infowidget;
        TQLabel * iconwidget;
        TQLabel * commentwidget;
        TQLabel * descriptionwidget;
        TQMap<TQCheckListItem*, KPluginInfo*> plugininfomap;
        TQValueList<KPluginInfo*> plugininfolist;
};

ComponentsDialog::ComponentsDialog( TQWidget * parent, const char * name )
    : KDialogBase( parent, name, false, i18n( "Select Components" ) )
, d( new ComponentsDialogPrivate )
{
    TQWidget * page = new TQWidget( this );
    setMainWidget( page );
    ( new TQHBoxLayout( page, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    d->listview = new KListView( page );
    d->listview->setMinimumSize( 200, 200 );
    d->infowidget = new TQFrame( page );
    d->infowidget->setMinimumSize( 200, 200 );
    ( new TQVBoxLayout( d->infowidget, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    d->iconwidget = new TQLabel( d->infowidget );
    ( void )new KSeparator( d->infowidget );
    d->commentwidget = new TQLabel( d->infowidget );
    d->commentwidget->setAlignment( Qt::WordBreak );
    d->descriptionwidget = new TQLabel( d->infowidget );
    d->descriptionwidget->setAlignment( Qt::WordBreak );

    d->listview->addColumn( TQString::null );
    d->listview->header()->hide();
    d->listview->setRootIsDecorated( true );
    d->listview->setSorting( -1 );
    d->listview->setAcceptDrops( false );
    d->listview->setSelectionModeExt( KListView::Single );
    d->listview->setAllColumnsShowFocus( true );

    connect( d->listview, TQT_SIGNAL( pressed( TQListViewItem * ) ), this,
            TQT_SLOT( executed( TQListViewItem * ) ) );
    connect( d->listview, TQT_SIGNAL( spacePressed( TQListViewItem * ) ), this,
            TQT_SLOT( executed( TQListViewItem * ) ) );
    connect( d->listview, TQT_SIGNAL( returnPressed( TQListViewItem * ) ), this,
            TQT_SLOT( executed( TQListViewItem * ) ) );
    connect( d->listview, TQT_SIGNAL( selectionChanged( TQListViewItem * ) ), this,
            TQT_SLOT( executed( TQListViewItem * ) ) );
}

ComponentsDialog::~ComponentsDialog()
{
}

void ComponentsDialog::addPluginInfo( KPluginInfo * info )
{
    d->plugininfolist.append( info );
}

void ComponentsDialog::setPluginInfos( const TQMap<TQString, KPluginInfo*> &
        plugininfos )
{
    for( TQMap<TQString, KPluginInfo*>::ConstIterator it = plugininfos.begin();
            it != plugininfos.end(); ++it )
    {
        d->plugininfolist.append( it.data() );
    }
}

void ComponentsDialog::setPluginInfos( const TQValueList<KPluginInfo *> &plugins )
{
    d->plugininfolist = plugins;
}

void ComponentsDialog::show()
{
    // clear the treelist
    d->listview->clear();
    d->plugininfomap.clear();

    // construct the treelist
    for( TQValueList<KPluginInfo*>::ConstIterator it = d->plugininfolist.begin();
            it != d->plugininfolist.end(); ++it )
    {
        ( *it )->load();
        TQCheckListItem * item = new TQCheckListItem( d->listview, ( *it )->name(),
                TQCheckListItem::CheckBox );
        if( ! ( *it )->icon().isEmpty() )
            item->setPixmap( 0, SmallIcon( ( *it )->icon(), IconSize( KIcon::Small ) ) );
        item->setOn( ( *it )->isPluginEnabled() );
        d->plugininfomap[ item ] = ( *it );
    }
    KDialogBase::show();
}

void ComponentsDialog::executed( TQListViewItem * item )
{
    kdDebug( 704 ) << k_funcinfo << endl;
    if( item == 0 )
        return;
    if( item->rtti() != 1 ) // check for QCheckListItem
        return;

    TQCheckListItem * citem = static_cast<TQCheckListItem *>( item );
    bool checked = citem->isOn();

    kdDebug( 704 ) << "it's a " << ( checked ? "checked" : "unchecked" )
        << " TQCheckListItem" << endl;

    KPluginInfo * info = d->plugininfomap[ citem ];
    info->setPluginEnabled( checked );
    //checkDependencies( info );
    // show info about the component on the right
    d->iconwidget->setPixmap( SmallIcon( info->icon(), KIcon::SizeLarge ) );
    d->commentwidget->setText( info->comment() );
    //d->descriptionwidget->setText( info->description() );
}

void ComponentsDialog::savePluginInfos()
{
    for( TQValueList<KPluginInfo*>::ConstIterator it = d->plugininfolist.begin();
            it != d->plugininfolist.end(); ++it )
    {
        if( ( *it )->config() )
        {
            ( *it )->save();
            ( *it )->config()->sync();
        }
    }
}

void ComponentsDialog::slotOk()
{
    savePluginInfos();
    KDialogBase::slotOk();
}

void ComponentsDialog::slotApply()
{
    savePluginInfos();
    KDialogBase::slotApply();
}

} //namespace

#include "componentsdialog.moc"
// vim: sw=4 sts=4 et
