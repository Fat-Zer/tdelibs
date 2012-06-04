/* -*- C++ -*-
   This file implements the application programming interface
   for using kab's addressbook files within other programs.
   Parse it with kdoc to get the API documentation.

   the TDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2001, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id$
*/

#include "kabapi.h"
#include <klistbox.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>


#include "kabapi.moc"

#ifdef KAB_KDEBUG_AREA
#undef KAB_KDEBUG_AREA
#endif

#define KAB_KDEBUG_AREA 800

using namespace std;

KabAPI::KabAPI(TQWidget* parent, const char* name)
  : KDialogBase(parent, name),
    book(0),
    listbox(new KListBox(this)),
    selection(-1)
{
  TQ_CHECK_PTR(listbox);
  setMainWidget(listbox);
  showButtonApply(false);
  enableButtonSeparator(true);
  connect(listbox, TQT_SIGNAL(highlighted(int)), TQT_SLOT(entrySelected(int)));
  connect(listbox, TQT_SIGNAL(doubleClicked ( TQListBoxItem * )),TQT_SLOT(slotDoubleClicked ( TQListBoxItem * )));
}


void KabAPI::slotDoubleClicked ( TQListBoxItem * )
{
    accept();
}

int KabAPI::exec()
{
  TQStringList names;
  // -----
  if(book==0)
    {
      kdDebug(KAB_KDEBUG_AREA)
	<< "KabAPI::exec: you have to call init before using the API."
	<< endl;
      return -1;
    } else {
      if(book->getListOfNames(&names, true, false)==AddressBook::NoError)
	{
	  listbox->clear();
	  listbox->insertStringList(names);
	  if(names.count()>0)
	    {
	      listbox->setCurrentItem(0);
	    }
	  listbox->setMinimumSize(listbox->sizeHint());
	  adjustSize();
	  resize(minimumSize());
	  return KDialogBase::exec();
	} else {
	  kdDebug(KAB_KDEBUG_AREA) << "KabAPI::exec: error creating interface."
				   << endl;
	  return -1;
	}
    }
}

AddressBook::ErrorCode KabAPI::init()
{
  // ############################################################################
  book=new AddressBook(0, "KABAPI::book", true);  //change parent from "this" to "0" //dsweet
  if(book->getState()==AddressBook::NoError)
    {
      connect(book, TQT_SIGNAL(setStatus(const TQString&)),
	      TQT_SLOT(setStatusSlot(const TQString&)));
      return AddressBook::NoError;
    } else {
      return AddressBook::InternError;
    }
  // ############################################################################
}

AddressBook::ErrorCode KabAPI::getEntry(AddressBook::Entry& entry, KabKey& key)
{
  // ############################################################################
  if(book->noOfEntries()==0)
    {
      return AddressBook::NoEntry;
    }
  if(selection>=0)
    {
      if(book->getKey(selection, key)==AddressBook::NoError)
	{
	  if(book->getEntry(key, entry)==AddressBook::NoError)
	    {
	      return AddressBook::NoError;
	    } else {
	      return AddressBook::InternError; // this may not happen
	    }
	} else {
	  return AddressBook::NoEntry;
	}
    } else {
      return AddressBook::InternError;
    }
  // ############################################################################
}

AddressBook::ErrorCode KabAPI::add(const AddressBook::Entry& entry, KabKey& key,
				   bool update)
{
  // ############################################################################
  if(book->add(entry, key, update)!=AddressBook::NoError)
    {
      KMessageBox::sorry(this, i18n("Your new entry could not be added."));
      return AddressBook::InternError;
    } else {
      return AddressBook::NoError;
    }
  // ############################################################################
}

AddressBook::ErrorCode KabAPI::remove(const KabKey& key)
{
  TQ_CHECK_PTR(book);
  // ############################################################################
  if(book->AddressBook::remove(key)==AddressBook::NoError)
    {
      return AddressBook::NoError;
    } else {
      return AddressBook::NoEntry;
    }
  // ############################################################################
}

AddressBook::ErrorCode KabAPI::getEntryByName(const TQString&,
					 list<AddressBook::Entry>&, const int)
{
  // ############################################################################
  return AddressBook::NotImplemented;
  // ############################################################################
}

AddressBook::ErrorCode KabAPI::getEntryByName(const AddressBook::Entry&,
					 list<AddressBook::Entry>&, const int)
{
  // ############################################################################
  return AddressBook::NotImplemented;
  // ############################################################################
}

AddressBook::ErrorCode KabAPI::getEntries(list<AddressBook::Entry>& entries)
{
  kdDebug(KAB_KDEBUG_AREA) << "KabAPI::getEntries: called." << endl;
  // ############################################################################
  if(book->noOfEntries()==0)
    { // ----- database is valid, but empty:
      kdDebug(KAB_KDEBUG_AREA) << "KabAPI::getEntries: no entries." << endl;
      return AddressBook::NoEntry;
    }
  if(book->getEntries(entries)!=AddressBook::NoError)
    {
      kdDebug(KAB_KDEBUG_AREA) << "KabAPI::getEntries: intern error." << endl;
      return AddressBook::InternError;
    } else {
      kdDebug(KAB_KDEBUG_AREA) << "KabAPI::getEntries: done." << endl;
      return AddressBook::NoError;
    }
  // ############################################################################
}

AddressBook* KabAPI::addressbook()
{
  // ############################################################################
  return book;
  // ############################################################################
}

AddressBook::ErrorCode KabAPI::save(bool force)
{
  // ############################################################################
  if(book->save("", force)!=AddressBook::NoError)
    {
      return AddressBook::PermDenied;
    } else {
      return AddressBook::NoError;
    }
  // ############################################################################
}

void KabAPI::entrySelected(int index)
{
  kdDebug(KAB_KDEBUG_AREA) << "KabAPI::entrySelected: entry " << index
			   <<" selected." << endl;
  selection=index;
}

void KabAPI::setStatusSlot(const TQString& text)
{
  emit(setStatus(text));
}
