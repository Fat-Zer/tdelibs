#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqdragobject.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kinputdialog.h>
#include <kdebug.h>

#include "ktabwidgettest.h"

Test::Test( TQWidget* parent, const char *name )
  :TQVBox( parent, name ), mChange(0), mLeftWidget(0), mRightWidget(0),
  mLeftPopup( false ), mRightPopup( false ), mTabbarContextPopup( false ), mContextPopup( false )

{
  resize( 600,300 );

  mWidget = new KTabWidget( this );
  mWidget->addTab( new TQLabel( "Testlabel 1", mWidget ), "One" );
  mWidget->addTab( new TQLabel( "Testlabel 2", mWidget ), "Two" );
  mWidget->addTab( new TQWidget( mWidget), SmallIcon( "konsole" ), "Three" );
  mWidget->addTab( new TQWidget( mWidget), "Four" );
  mWidget->setTabColor( mWidget->page(0), Qt::red );
  mWidget->setTabColor( mWidget->page(1), Qt::blue );

  connect( mWidget, TQT_SIGNAL( currentChanged( TQWidget * ) ), TQT_SLOT( currentChanged( TQWidget * ) ) );
  connect( mWidget, TQT_SIGNAL( contextMenu( TQWidget *, const TQPoint & )), TQT_SLOT(contextMenu( TQWidget *, const TQPoint & )));
  connect( mWidget, TQT_SIGNAL( contextMenu( const TQPoint & )), TQT_SLOT(tabbarContextMenu( const TQPoint & )));
  connect( mWidget, TQT_SIGNAL( mouseDoubleClick( TQWidget * )), TQT_SLOT(mouseDoubleClick( TQWidget * )));
  connect( mWidget, TQT_SIGNAL( mouseMiddleClick() ), TQT_SLOT(addTab() ));
  connect( mWidget, TQT_SIGNAL( mouseMiddleClick( TQWidget * )), TQT_SLOT(mouseMiddleClick( TQWidget * )));
  connect( mWidget, TQT_SIGNAL( closeRequest( TQWidget * )), TQT_SLOT(mouseMiddleClick( TQWidget * )));
  connect( mWidget, TQT_SIGNAL( testCanDecode(const TQDragMoveEvent *, bool & )), TQT_SLOT(testCanDecode(const TQDragMoveEvent *, bool & )));
  connect( mWidget, TQT_SIGNAL( receivedDropEvent( TQDropEvent * )), TQT_SLOT(receivedDropEvent( TQDropEvent * )));
  connect( mWidget, TQT_SIGNAL( receivedDropEvent( TQWidget *, TQDropEvent * )), TQT_SLOT(receivedDropEvent( TQWidget *, TQDropEvent * )));
  connect( mWidget, TQT_SIGNAL( initiateDrag( TQWidget * )), TQT_SLOT(initiateDrag( TQWidget * )));
  connect( mWidget, TQT_SIGNAL( movedTab( int, int )), TQT_SLOT(movedTab( int, int )));
  mWidget->setTabReorderingEnabled( true );

  TQWidget * grid = new TQWidget(this);
  TQGridLayout * gridtqlayout = new TQGridLayout( grid, 5, 2 );

  TQPushButton * addTab = new TQPushButton( "Add Tab", grid );
  gridtqlayout->addWidget( addTab, 0, 0 );
  connect( addTab, TQT_SIGNAL( clicked() ), TQT_SLOT( addTab() ) );

  TQPushButton * removeTab = new TQPushButton( "Remove Current Tab", grid );
  gridtqlayout->addWidget( removeTab, 0, 1 );
  connect( removeTab, TQT_SIGNAL( clicked() ), TQT_SLOT( removeCurrentTab() ) );

  mLeftButton = new TQCheckBox( "Show left button", grid );
  gridtqlayout->addWidget( mLeftButton, 1, 0 );
  connect( mLeftButton, TQT_SIGNAL( toggled(bool) ), TQT_SLOT( toggleLeftButton(bool) ) );
  mLeftButton->setChecked(true);

  TQCheckBox * leftPopup = new TQCheckBox( "Enable left popup", grid );
  gridtqlayout->addWidget( leftPopup, 2, 0 );
  connect( leftPopup, TQT_SIGNAL( toggled(bool) ), TQT_SLOT( toggleLeftPopup(bool) ) );
  leftPopup->setChecked(true);

  mRightButton = new TQCheckBox( "Show right button", grid );
  gridtqlayout->addWidget( mRightButton, 1, 1 );
  connect( mRightButton, TQT_SIGNAL( toggled(bool) ), TQT_SLOT( toggleRightButton(bool) ) );
  mRightButton->setChecked(true);

  TQCheckBox * rightPopup = new TQCheckBox( "Enable right popup", grid );
  gridtqlayout->addWidget( rightPopup, 2, 1 );
  connect( rightPopup, TQT_SIGNAL( toggled(bool) ), TQT_SLOT( toggleRightPopup(bool) ) );
  rightPopup->setChecked(true);

  mTabsBottom = new TQCheckBox( "Show tabs at bottom", grid );
  gridtqlayout->addWidget( mTabsBottom, 3, 0 );
  connect( mTabsBottom, TQT_SIGNAL( toggled(bool) ), TQT_SLOT( toggleTabPosition(bool) ) );

  TQCheckBox * tabtqshape = new TQCheckBox( "Triangular tab tqshape", grid );
  gridtqlayout->addWidget( tabtqshape, 3, 1 );
  connect( tabtqshape, TQT_SIGNAL( toggled(bool) ), TQT_SLOT( toggleTabShape(bool) ) );

  TQCheckBox *tabClose = new TQCheckBox( "Close button on icon hover", grid );
  gridtqlayout->addWidget( tabClose, 4, 0 );
  connect( tabClose, TQT_SIGNAL( toggled(bool) ), TQT_SLOT( toggleCloseButtons(bool) ) );
  tabClose->setChecked(true);

  TQCheckBox * showlabels = new TQCheckBox( "Show labels", grid );
  gridtqlayout->addWidget( showlabels, 4, 1 );
  connect( showlabels, TQT_SIGNAL( toggled(bool) ), this, TQT_SLOT( toggleLabels(bool) ) );
}

void Test::currentChanged(TQWidget* w)
{
  mWidget->setTabColor( w, Qt::black );
}

void Test::addTab()
{
  mWidget->addTab( new TQWidget( mWidget ), SmallIcon( "konsole" ), TQString("Tab %1").arg( mWidget->count()+1 ) );
}

void Test::testCanDecode(const TQDragMoveEvent *e, bool &accept /* result */)
{
  if ( TQTextDrag::canDecode(e) )    // don't accept=false if it cannot be decoded!
    accept = true;
}

void Test::receivedDropEvent( TQDropEvent *e )
{
  TQString dropText;
  if (TQTextDrag::decode(e, dropText)) {
    mWidget->addTab( new TQWidget( mWidget), dropText );
  }
}

void Test::receivedDropEvent( TQWidget *w, TQDropEvent *e )
{
  TQString dropText;
  if (TQTextDrag::decode(e, dropText)) {
    mWidget->changeTab( w, dropText );
  }
}

void Test::initiateDrag( TQWidget *w )
{
   TQDragObject *d = new TQTextDrag( mWidget->label( mWidget->indexOf( w ) ), this );
   d->dragCopy(); // do NOT delete d.
}

void Test::removeCurrentTab()
{
  if ( mWidget->count()==1 ) return;

  mWidget->removePage( mWidget->currentPage() );
}

void Test::toggleLeftButton(bool state)
{
  if (state) {
    if (!mLeftWidget) {
      mLeftWidget = new TQToolButton( mWidget );
      connect( mLeftWidget, TQT_SIGNAL( clicked() ), TQT_SLOT( addTab() ) );
      mLeftWidget->setIconSet( SmallIcon( "tab_new" ) );
      mLeftWidget->setTextLabel("New");
      mLeftWidget->setTextPosition(TQToolButton::Right);
      mLeftWidget->adjustSize();
    //mLeftWidget->setGeometry( 0, 0, h, h );
      mLeftWidget->setPopup(mLeftPopup);
      mWidget->setCornerWidget( mLeftWidget, TopLeft );
    }
    mLeftWidget->show();
  }
  else
    mLeftWidget->hide();
}

void Test::toggleLeftPopup(bool state)
{
  if (state) {
    if (!mLeftPopup) {
      mLeftPopup = new TQPopupMenu(this);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Empty Tab", 0);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Empty Tab After First", 3);
      mLeftPopup->insertSeparator();
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Button Tab", 1);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Label Tab", 2);
      connect(mLeftPopup, TQT_SIGNAL(activated(int)), TQT_SLOT(leftPopupActivated(int)));
    }
    mLeftWidget->setPopup(mLeftPopup);
  }
  else
    mLeftWidget->setPopup(0);
}

void Test::leftPopupActivated(int item)
{
  switch (item) {
    case 0: mWidget->addTab( new TQWidget( mWidget), TQString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 1: mWidget->addTab( new TQPushButton( "Testbutton", mWidget ), TQString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 2: mWidget->addTab( new TQLabel( "Testlabel", mWidget ), TQString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 3: mWidget->insertTab( new TQWidget( mWidget), TQString("Tab %1").arg( mWidget->count()+1 ), 1 );
  }
}

void Test::toggleRightButton(bool state)
{
if (state) {
    if ( !mRightWidget) {
      mRightWidget = new TQToolButton( mWidget );
      TQObject::connect( mRightWidget, TQT_SIGNAL( clicked() ), TQT_SLOT( removeCurrentTab() ) );
      mRightWidget->setIconSet( SmallIcon( "tab_remove" ) );
      mRightWidget->setTextLabel("Close");
      mRightWidget->setTextPosition(TQToolButton::Right);
      mRightWidget->adjustSize();
    //mRightButton->setGeometry( 0, 0, h, h );
      mRightWidget->setPopup(mRightPopup);
      mWidget->setCornerWidget( mRightWidget, TopRight );
    }
    mRightWidget->show();
  }
  else
    mRightWidget->hide();
}

void Test::toggleRightPopup(bool state)
{
  if (state) {
    if (!mRightPopup) {
      mRightPopup = new TQPopupMenu(this);
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Current Tab", 1);
      mRightPopup->insertSeparator();
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Most Left Tab", 0);
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Most Right Tab", 2);
      connect(mRightPopup, TQT_SIGNAL(activated(int)), TQT_SLOT(rightPopupActivated(int)));
    }
    mRightWidget->setPopup(mRightPopup);
  }
  else
    mRightWidget->setPopup(0);
}

void Test::rightPopupActivated(int item)
{
  switch (item) {
    case 0: if ( mWidget->count() >1) {
              mWidget->removePage( mWidget->page(0) );
            }
            break;
    case 1: removeCurrentTab();
            break;
    case 2: int count = mWidget->count();
            if (count>1) {
              mWidget->removePage( mWidget->page(count-1) );
            }
  }
}

void Test::toggleTabPosition(bool state)
{
  mWidget->setTabPosition(state ? TQTabWidget::Bottom : TQTabWidget::Top);
}

void Test::toggleTabShape(bool state)
{
  mWidget->setTabShape(state ? TQTabWidget::Triangular : TQTabWidget::Rounded);
}

void Test::toggleCloseButtons(bool state)
{
  mWidget->setHoverCloseButton( state );
}

void Test::contextMenu(TQWidget *w, const TQPoint &p)
{
  if (mContextPopup)
      delete mContextPopup;

  mContextPopup = new TQPopupMenu(this);
  mContextPopup->insertItem( "Activate Tab", 4);
  mContextPopup->insertSeparator();
  mContextPopup->insertItem(SmallIcon( "konsole" ), "Set This Icon", 0);
  mContextPopup->insertItem(SmallIcon( "konqueror" ), "Set This Icon", 1);
  mContextPopup->insertSeparator();
  mContextPopup->insertItem( mWidget->isTabEnabled(w) ? "Disable Tab" : "Enable Tab", 2);
  mContextPopup->insertItem( mWidget->tabToolTip(w).isEmpty() ? "Set Tooltip" : "Remove Tooltip", 3);
  connect(mContextPopup, TQT_SIGNAL(activated(int)), TQT_SLOT(contextMenuActivated(int)));

  mContextWidget = w;
  mContextPopup->popup(p);
}

void Test::contextMenuActivated(int item)
{
  switch (item) {
    case 0: mWidget->changeTab( mContextWidget, SmallIcon( "konsole" ), mWidget->label( mWidget->indexOf( mContextWidget ) )  );
            break;
    case 1: mWidget->changeTab( mContextWidget, SmallIcon( "konqueror" ), mWidget->label( mWidget->indexOf( mContextWidget ) ) );
            break;
    case 2: mWidget->setTabEnabled( mContextWidget, !(mWidget->isTabEnabled(mContextWidget)) );
            break;
    case 3: if ( mWidget->tabToolTip(mContextWidget).isEmpty() )
              mWidget->setTabToolTip( mContextWidget, "This is a tool tip.");
            else
              mWidget->removeTabToolTip( mContextWidget );
            break;
    case 4: mWidget->showPage( mContextWidget );
  }
  delete mContextPopup;
  mContextPopup = 0;
}

void Test::tabbarContextMenu(const TQPoint &p)
{
  if (mTabbarContextPopup)
      delete mTabbarContextPopup;

  mTabbarContextPopup = new TQPopupMenu(this);
  mTabbarContextPopup->insertItem(SmallIcon( "tab_new" ), mLeftWidget->isVisible() ? "Hide \"Add\" Button" : "Show \"Add\" Button", 0);
  mTabbarContextPopup->insertItem(SmallIcon( "tab_remove" ), mRightWidget->isVisible() ? "Hide \"Remove\" Button" : "Show \"Remove\" Button", 1);
  mTabbarContextPopup->insertSeparator();
  mTabbarContextPopup->insertItem(mWidget->tabPosition()==TQTabWidget::Top ? "Put Tabbar to Bottom" : "Put Tabbar to Top", 2);
  connect(mTabbarContextPopup, TQT_SIGNAL(activated(int)), TQT_SLOT(tabbarContextMenuActivated(int)));

  mTabbarContextPopup->popup(p);
}

void Test::tabbarContextMenuActivated(int item)
{
  switch (item) {
    case 0: mLeftButton->toggle();
            break;
    case 1: mRightButton->toggle();
            break;
    case 2: mTabsBottom->toggle();
  }
  delete mTabbarContextPopup;
  mTabbarContextPopup = 0;
}

void Test::mouseDoubleClick(TQWidget *w)
{
  bool ok;
  TQString text = KInputDialog::getText(
            "Rename Tab", "Enter new name:",
            mWidget->label( mWidget->indexOf( w ) ), &ok, this );
  if ( ok && !text.isEmpty() ) {
     mWidget->changeTab( w, text );
     mWidget->setTabColor( w, Qt::green );
  }
}

void Test::mouseMiddleClick(TQWidget *w)
{
  if ( mWidget->count()==1 ) return;

  mWidget->removePage( w );
}

void Test::movedTab(int from, int to)
{
  kdDebug() << "Moved tab from index " << from << " to " << to << endl;
}

void Test::toggleLabels(bool state)
{
  mLeftWidget->setUsesTextLabel(state);
  mLeftWidget->adjustSize();
  mRightWidget->setUsesTextLabel(state);
  mRightWidget->adjustSize();
  mWidget->hide();   // trigger update
  mWidget->show();
}

int main(int argc, char** argv )
{
    KCmdLineArgs::init(argc, argv, "ktabwidgettest", "ktabwidget test app", "1.0");
    KApplication app(argc, argv);
    Test *t = new Test();

    app.setMainWidget( t );
    t->show();
    app.exec();
}

#include "ktabwidgettest.moc"
