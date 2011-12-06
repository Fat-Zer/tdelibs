/* This file is part of the KDE libraries
    Copyright (C) 2004 Felix Berger <felixberger@beldesign.de>
    
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

#include "ktoolbarlabelaction.h"

#include <tqlabel.h>
#include <tqapplication.h>

class KToolBarLabelAction::KToolBarLabelActionPrivate
{
public:
  KToolBarLabelActionPrivate()
    : m_label(0)
  {
  }
  TQLabel* m_label;
};


KToolBarLabelAction::KToolBarLabelAction(const TQString &text,
					 const KShortcut &cut,
					 const TQObject *receiver, 
					 const char *slot,
					 KActionCollection *parent,
					 const char *name)
  : KWidgetAction(new TQLabel(text, 0, "kde toolbar widget"), text, cut,
		  receiver, slot, parent, name), 
    d(new KToolBarLabelActionPrivate)
{
  init();
}

KToolBarLabelAction::KToolBarLabelAction(TQWidget* buddy, 
					 const TQString &text,
					 const KShortcut &cut,
					 const TQObject *receiver, 
					 const char *slot,
 					 KActionCollection *parent, 
					 const char *name)
  : KWidgetAction(new TQLabel(buddy, text, 0, "kde toolbar widget"), text, 
		  cut, receiver, slot, parent, name),
    d(new KToolBarLabelActionPrivate)
{
  init();
}

KToolBarLabelAction::KToolBarLabelAction(TQLabel* label, 
					 const KShortcut &cut, 
					 const TQObject *receiver,
					 const char *slot,
					 KActionCollection* parent, 
					 const char *name)
  : KWidgetAction(label, label->text(), cut, receiver, slot, parent, name),
    d(new KToolBarLabelActionPrivate)
{
  Q_ASSERT(TQString::tqfromLatin1("kde toolbar widget") == label->name());
  init();
}

KToolBarLabelAction::~KToolBarLabelAction()
{
  delete d;
  d = 0;
}

void KToolBarLabelAction::init()
{
  d->m_label = static_cast<TQLabel*>(widget());
  /* these lines were copied from Konqueror's KonqDraggableLabel class in
     konq_misc.cc */
  d->m_label->setBackgroundMode(TQt::PaletteButton);
  d->m_label->setAlignment((TQApplication::reverseLayout()
			 ? Qt::AlignRight : Qt::AlignLeft) |
 			Qt::AlignVCenter | TQt::ShowPrefix );
  d->m_label->adjustSize();
}

void KToolBarLabelAction::setText(const TQString& text)
{
  KWidgetAction::setText(text);
  d->m_label->setText(text);
}

void KToolBarLabelAction::setBuddy(TQWidget* buddy)
{
  d->m_label->setBuddy(buddy);
}

TQWidget* KToolBarLabelAction::buddy() const
{
  return d->m_label->buddy();
}

TQLabel* KToolBarLabelAction::label() const
{
  return d->m_label;
}

void KToolBarLabelAction::virtual_hook(int, void*)
{

}
