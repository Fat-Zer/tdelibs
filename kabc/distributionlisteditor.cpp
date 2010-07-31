/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqlistview.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqcombobox.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>

#include <kinputdialog.h>
#include <klocale.h>
#include <kdebug.h>

#include "addressbook.h"
#include "addresseedialog.h"
#include "distributionlist.h"

#include "distributionlisteditor.h"
#include "distributionlisteditor.moc"

using namespace KABC;

EmailSelectDialog::EmailSelectDialog( const TQStringList &emails, const TQString &current,
                                      TQWidget *parent ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Email Address"), Ok, Ok,
               parent )
{
  TQFrame *topFrame = plainPage();
  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mButtonGroup = new TQButtonGroup( 1, Horizontal, i18n("Email Addresses"),
                                   topFrame );
  mButtonGroup->setRadioButtonExclusive( true );
  topLayout->addWidget( mButtonGroup );

  TQStringList::ConstIterator it;
  for( it = emails.begin(); it != emails.end(); ++it ) {
    TQRadioButton *button = new TQRadioButton( *it, mButtonGroup );
    if ( (*it) == current ) {
      button->setDown( true );
    }
  }
}

TQString EmailSelectDialog::selected()
{
  TQButton *button = mButtonGroup->selected();
  if ( button ) return button->text();
  return TQString::null;
}

TQString EmailSelectDialog::getEmail( const TQStringList &emails, const TQString &current,
                                     TQWidget *parent )
{
  EmailSelectDialog *dlg = new EmailSelectDialog( emails, current, parent );
  dlg->exec();

  TQString result = dlg->selected();

  delete dlg;

  return result;
}

class EditEntryItem : public QListViewItem
{
  public:
    EditEntryItem( TQListView *parent, const Addressee &addressee,
               const TQString &email=TQString::null ) :
      TQListViewItem( parent ),
      mAddressee( addressee ),
      mEmail( email )
    {
      setText( 0, addressee.realName() );
      if( email.isEmpty() ) {
        setText( 1, addressee.preferredEmail() );
        setText( 2, i18n("Yes") );
      } else {
        setText( 1, email );
        setText( 2, i18n("No") );
      }
    }

    Addressee addressee() const
    {
      return mAddressee;
    }

    TQString email() const
    {
      return mEmail;
    }

  private:
    Addressee mAddressee;
    TQString mEmail;
};

DistributionListEditor::DistributionListEditor( AddressBook *addressBook, TQWidget *parent) :
  TQWidget( parent ),
  mAddressBook( addressBook )
{
  kdDebug(5700) << "DistributionListEditor()" << endl;

  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  TQBoxLayout *nameLayout = new TQHBoxLayout( topLayout) ;

  mNameCombo = new TQComboBox( this );
  nameLayout->addWidget( mNameCombo );
  connect( mNameCombo, TQT_SIGNAL( activated( int ) ), TQT_SLOT( updateEntryView() ) );

  newButton = new TQPushButton( i18n("New List"), this );
  nameLayout->addWidget( newButton );
  connect( newButton, TQT_SIGNAL( clicked() ), TQT_SLOT( newList() ) );

  removeButton = new TQPushButton( i18n("Remove List"), this );
  nameLayout->addWidget( removeButton );
  connect( removeButton, TQT_SIGNAL( clicked() ), TQT_SLOT( removeList() ) );

  mEntryView = new TQListView( this );
  mEntryView->addColumn( i18n("Name") );
  mEntryView->addColumn( i18n("Email") );
  mEntryView->addColumn( i18n("Use Preferred") );
  topLayout->addWidget( mEntryView );
  connect(mEntryView,TQT_SIGNAL(selectionChanged ()),this, TQT_SLOT(slotSelectionEntryViewChanged()));

  changeEmailButton = new TQPushButton( i18n("Change Email"), this );
  topLayout->addWidget( changeEmailButton );
  connect( changeEmailButton, TQT_SIGNAL( clicked() ), TQT_SLOT( changeEmail() ) );

  removeEntryButton = new TQPushButton( i18n("Remove Entry"), this );
  topLayout->addWidget( removeEntryButton );
  connect( removeEntryButton, TQT_SIGNAL( clicked() ), TQT_SLOT( removeEntry() ) );

  addEntryButton = new TQPushButton( i18n("Add Entry"), this );
  topLayout->addWidget( addEntryButton );
  connect( addEntryButton, TQT_SIGNAL( clicked() ), TQT_SLOT( addEntry() ) );

  mAddresseeView = new TQListView( this );
  mAddresseeView->addColumn( i18n("Name") );
  mAddresseeView->addColumn( i18n("Preferred Email") );
  topLayout->addWidget( mAddresseeView );


  connect(mAddresseeView,TQT_SIGNAL(selectionChanged ()),this, TQT_SLOT(slotSelectionAddresseeViewChanged()));

  mManager = new DistributionListManager( mAddressBook );
  mManager->load();

  updateAddresseeView();
  updateNameCombo();
  removeButton->setEnabled(!mManager->listNames().isEmpty());
}

DistributionListEditor::~DistributionListEditor()
{
  kdDebug(5700) << "~DistributionListEditor()" << endl;

  mManager->save();
  delete mManager;
}

void DistributionListEditor::slotSelectionEntryViewChanged()
{
    EditEntryItem *entryItem = dynamic_cast<EditEntryItem *>( mEntryView->selectedItem() );
    bool state = (entryItem != 0L);

    changeEmailButton->setEnabled(state);
    removeEntryButton->setEnabled(state);
}

void DistributionListEditor::newList()
{
  bool ok = false;
  TQString name = KInputDialog::getText( i18n("New Distribution List"),
                                        i18n("Please enter name:"),
                                        TQString::null, &ok, this );
  if ( !ok )
    return;

  new DistributionList( mManager, name );

  mNameCombo->insertItem( name );
  removeButton->setEnabled(true);
  updateEntryView();
}

void DistributionListEditor::removeList()
{
  mManager->remove( mManager->list( mNameCombo->currentText() ) );
  mNameCombo->removeItem( mNameCombo->currentItem() );
  removeButton->setEnabled(!mManager->listNames().isEmpty());
  addEntryButton->setEnabled( !mNameCombo->currentText().isEmpty());
  updateEntryView();
}

void DistributionListEditor::addEntry()
{
  AddresseeItem *addresseeItem =
      dynamic_cast<AddresseeItem *>( mAddresseeView->selectedItem() );

  if( !addresseeItem ) {
    kdDebug(5700) << "DLE::addEntry(): No addressee selected." << endl;
    return;
  }

  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) {
    kdDebug(5700) << "DLE::addEntry(): No dist list '" << mNameCombo->currentText() << "'" << endl;
    return;
  }

  list->insertEntry( addresseeItem->addressee() );
  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditor::removeEntry()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  EditEntryItem *entryItem =
      dynamic_cast<EditEntryItem *>( mEntryView->selectedItem() );
  if ( !entryItem ) return;

  list->removeEntry( entryItem->addressee(), entryItem->email() );
  delete entryItem;
}

void DistributionListEditor::changeEmail()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  EditEntryItem *entryItem =
      dynamic_cast<EditEntryItem *>( mEntryView->selectedItem() );
  if ( !entryItem ) return;

  TQString email = EmailSelectDialog::getEmail( entryItem->addressee().emails(),
                                               entryItem->email(), this );
  list->removeEntry( entryItem->addressee(), entryItem->email() );
  list->insertEntry( entryItem->addressee(), email );

  updateEntryView();
}

void DistributionListEditor::updateEntryView()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  mEntryView->clear();
  DistributionList::Entry::List entries = list->entries();
  DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    new EditEntryItem( mEntryView, (*it).addressee, (*it).email );
  }
   EditEntryItem *entryItem = dynamic_cast<EditEntryItem *>( mEntryView->selectedItem() );
   bool state = (entryItem != 0L);

   changeEmailButton->setEnabled(state);
   removeEntryButton->setEnabled(state);
}

void DistributionListEditor::updateAddresseeView()
{
  mAddresseeView->clear();

  AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    new AddresseeItem( mAddresseeView, *it );
  }
}

void DistributionListEditor::updateNameCombo()
{
  mNameCombo->insertStringList( mManager->listNames() );

  updateEntryView();
}

void DistributionListEditor::slotSelectionAddresseeViewChanged()
{
    AddresseeItem *addresseeItem =
        dynamic_cast<AddresseeItem *>( mAddresseeView->selectedItem() );
    bool state = (addresseeItem != 0L);
    addEntryButton->setEnabled( state && !mNameCombo->currentText().isEmpty());
}
