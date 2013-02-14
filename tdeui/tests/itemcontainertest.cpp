/*
* Tests the item container widgets TDEIconView, TDEListView, TDEListBox
*
* Copyright (c) 2000 by Michael Reiher <michael.reiher@gmx.de>
*
* License: GPL, version 2
* Version: $Id:
*
*/

#include <tqlayout.h>
#include <tqvbox.h>
#include <tqhbox.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>
#include <tqcheckbox.h>
#include <tqlabel.h>

#include <tdeapplication.h>
#include <kglobal.h>
#include <tdeconfig.h>
#include <kiconview.h>
#include <tdelistview.h>
#include <tdelistbox.h>

#include "itemcontainertest.h"

static const char * item_xpm[] = {
"22 22 3 1",
" 	c None",
".	c #000000",
"+	c #FF0000",
"        ......        ",
"     ....++++....     ",
"    ..++++..++++..    ",
"   ..++++++++++++..   ",
"  ..++++++..++++++..  ",
" ..++++++++++++++++.. ",
" .++++++++..++++++++. ",
" .++++++++++++++++++. ",
"..++++++++..++++++++..",
".++++++++++++++++++++.",
".+.+.+.+.+..+.+.+.+.+.",
".+.+.+.+.+..+.+.+.+.+.",
".++++++++++++++++++++.",
"..++++++++..++++++++..",
" .++++++++++++++++++. ",
" .++++++++..++++++++. ",
" ..++++++++++++++++.. ",
"  ..++++++..++++++..  ",
"   ..++++++++++++..   ",
"    ..++++..++++..    ",
"     ....++++....     ",
"        ......        "};


TDEApplication *app;

TopLevel::TopLevel(TQWidget *parent, const char *name)
    : TQWidget(parent, name)
{
    setCaption("Item container test application");

    TQHBoxLayout* hBox = new TQHBoxLayout( this );
    TQVBoxLayout* vBox = new TQVBoxLayout( hBox );
    hBox->addSpacing( 5 );

    //Selection mode selection
    m_pbgMode = new TQButtonGroup( 1, Qt::Horizontal, "Selection Mode", this);
    m_pbgMode->insert(new TQRadioButton("NoSlection", m_pbgMode), TopLevel::NoSelection );
    m_pbgMode->insert(new TQRadioButton("Single", m_pbgMode), TopLevel::Single );
    m_pbgMode->insert(new TQRadioButton("Multi", m_pbgMode), TopLevel::Multi );
    m_pbgMode->insert(new TQRadioButton("Extended", m_pbgMode), TopLevel::Extended );
    m_pbgMode->setExclusive( true );
    vBox->addWidget( m_pbgMode );

    connect( m_pbgMode, TQT_SIGNAL( clicked( int ) ),
	     this, TQT_SLOT( slotSwitchMode( int ) ) );

    //Signal labels
    TQGroupBox* gbWiget = new TQGroupBox( 1, Qt::Horizontal, "Widget", this);
    m_plblWidget = new TQLabel( gbWiget );
    vBox->addWidget( gbWiget );
    TQGroupBox* gbSignal = new TQGroupBox( 1, Qt::Horizontal, "emitted Signal", this);
    m_plblSignal = new TQLabel( gbSignal );
    vBox->addWidget( gbSignal );
    TQGroupBox* gbItem = new TQGroupBox( 1, Qt::Horizontal, "on Item", this);
    m_plblItem = new TQLabel( gbItem );
    vBox->addWidget( gbItem );

    TQButtonGroup* bgListView = new TQButtonGroup( 1, Qt::Horizontal, "TDEListView", this);
    TQCheckBox* cbListView = new TQCheckBox("Single Column", bgListView);
    vBox->addWidget( bgListView );
    connect( cbListView, TQT_SIGNAL( toggled( bool ) ),
	     this, TQT_SLOT( slotToggleSingleColumn( bool ) ) );

    TDEGlobal::config()->reparseConfiguration();

    //Create IconView
    TQGroupBox* gbIconView = new TQGroupBox( 1, Qt::Horizontal, "TDEIconView", this);
    m_pIconView = new TDEIconView( gbIconView );
    hBox->addWidget( gbIconView );
    hBox->addSpacing( 5 );
    connect( m_pIconView, TQT_SIGNAL( executed( TQIconViewItem* ) ),
	     this, TQT_SLOT( slotIconViewExec( TQIconViewItem* ) ) );

    //Create ListView
    TQGroupBox* gbListView = new TQGroupBox( 1, Qt::Horizontal, "TDEListView", this);
    m_pListView = new TDEListView( gbListView );
    m_pListView->addColumn("Item");
    m_pListView->addColumn("Text");
    hBox->addWidget( gbListView );
    hBox->addSpacing( 5 );
    connect( m_pListView, TQT_SIGNAL( executed( TQListViewItem* ) ),
	     this, TQT_SLOT( slotListViewExec( TQListViewItem* ) ) );

    //Create ListBox
    TQGroupBox* gbListBox = new TQGroupBox( 1, Qt::Horizontal, "TDEListBox", this);
    m_pListBox = new TDEListBox( gbListBox );
    hBox->addWidget( gbListBox );
    connect( m_pListBox, TQT_SIGNAL( executed( TQListBoxItem* ) ),
	     this, TQT_SLOT( slotListBoxExec( TQListBoxItem* ) ) );

    //Initialize buttons
    cbListView->setChecked( !m_pListView->allColumnsShowFocus() );
    m_pbgMode->setButton( TopLevel::Extended );
    slotSwitchMode( TopLevel::Extended );

    //Fill container widgets
    for( int i = 0; i < 10; i++ ) {
      new TQIconViewItem( m_pIconView, TQString("Item%1").arg(i), TQPixmap(item_xpm) );

      TQListViewItem* lv = new TQListViewItem( m_pListView, TQString("Item%1").arg(i), TQString("Text%1").arg(i) );
      lv->setPixmap( 0, TQPixmap(item_xpm));
      lv->setPixmap( 1, TQPixmap(item_xpm));
      
      new TQListBoxPixmap( m_pListBox, TQPixmap(item_xpm), TQString("Item%1").arg(i));
    }

    connect( m_pIconView, TQT_SIGNAL( clicked( TQIconViewItem* ) ),
	     this, TQT_SLOT( slotClicked( TQIconViewItem* ) ) );
    connect( m_pIconView, TQT_SIGNAL( doubleClicked( TQIconViewItem* ) ),
	     this, TQT_SLOT( slotDoubleClicked( TQIconViewItem* ) ) );
}

void TopLevel::slotSwitchMode( int id ) 
{
  m_pIconView->clearSelection();
  m_pListView->clearSelection();
  m_pListBox->clearSelection();

  switch( id ) {
  case TopLevel::NoSelection:
    m_pIconView->setSelectionMode( TDEIconView::NoSelection );
    m_pListView->setSelectionMode( TQListView::NoSelection );
    m_pListBox->setSelectionMode( TDEListBox::NoSelection );
    break;
  case TopLevel::Single:
    m_pIconView->setSelectionMode( TDEIconView::Single );
    m_pListView->setSelectionMode( TQListView::Single );
    m_pListBox->setSelectionMode( TDEListBox::Single );
    break;
  case TopLevel::Multi:
    m_pIconView->setSelectionMode( TDEIconView::Multi );
    m_pListView->setSelectionMode( TQListView::Multi );
    m_pListBox->setSelectionMode( TDEListBox::Multi );
    break;
  case TopLevel::Extended:
    m_pIconView->setSelectionMode( TDEIconView::Extended );
    m_pListView->setSelectionMode( TQListView::Extended );
    m_pListBox->setSelectionMode( TDEListBox::Extended );
    break;
  default:
    Q_ASSERT(0);
  }
}

void TopLevel::slotIconViewExec( TQIconViewItem* item )
{
  m_plblWidget->setText("TDEIconView");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text() );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotListViewExec( TQListViewItem* item )
{
  m_plblWidget->setText("TDEListView");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text(0) );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotListBoxExec( TQListBoxItem* item )
{
  m_plblWidget->setText("TDEListBox");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text() );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotToggleSingleColumn( bool b )
{
  m_pListView->setAllColumnsShowFocus( !b );
}

int main( int argc, char ** argv )
{
    app = new TDEApplication ( argc, argv, "ItemContainerTest" );

    TopLevel *toplevel = new TopLevel(0, "itemcontainertest");

    toplevel->show();
    toplevel->resize( 600, 300 );
    app->setMainWidget(toplevel);
    app->exec();
}

#include "itemcontainertest.moc"
