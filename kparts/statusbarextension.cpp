/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 David Faure <faure@kde.org>

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

#include "statusbarextension.h"

#include <tqvaluelist.h>
#include <tqobjectlist.h>

#include <kstatusbar.h>
#include <kmainwindow.h>
#include <kdebug.h>
#include <tdelibs_export.h>
#include <kparts/part.h>
#include <kparts/event.h>

using namespace KParts;

///////////////////////////////////////////////////////////////////
// Helper Classes
///////////////////////////////////////////////////////////////////

class KParts::StatusBarItem {
  public:
    StatusBarItem() // for QValueList
      : m_widget(0), m_visible(false)
      {}
    StatusBarItem( TQWidget * widget, int stretch, bool permanent )
      : m_widget(widget), m_stretch(stretch), m_permanent(permanent), m_visible(false)
      {}

    TQWidget * widget() const { return m_widget; }

    void ensureItemShown( KStatusBar * sb )
    {
      if ( !m_visible )
      {
        sb->addWidget( m_widget, m_stretch, m_permanent );
        m_visible = true;
        m_widget->show();
      }
    }
    void ensureItemHidden( KStatusBar * sb )
    {
      if ( m_visible )
      {
        sb->removeWidget( m_widget );
        m_visible = false;
        m_widget->hide();
      }
    }
  private:
    TQWidget * m_widget;
    int m_stretch;
    bool m_permanent;
    bool m_visible;  // true when the item has been added to the statusbar
};

///////////////////////////////////////////////////////////////////


StatusBarExtension::StatusBarExtension(KParts::ReadOnlyPart *parent, const char* name)
  : TQObject(parent, name), m_statusBar(0), d(0)
{
  parent->installEventFilter(this);
}

StatusBarExtension::~StatusBarExtension()
{
}


StatusBarExtension *StatusBarExtension::childObject( TQObject *obj )
{
    if ( !obj || obj->childrenListObject().isEmpty() )
        return 0L;

    // we try to do it on our own, in hope that we are faster than
    // queryList, which looks kind of big :-)
    const TQObjectList children = obj->childrenListObject();
    TQObjectListIt it( children );
    for (; it.current(); ++it )
        if ( it.current()->inherits( "KParts::StatusBarExtension" ) )
            return static_cast<KParts::StatusBarExtension *>( it.current() );

    return 0L;
}

bool StatusBarExtension::eventFilter(TQObject * watched, TQEvent* ev)
{
  if ( !GUIActivateEvent::test( ev ) ||
      !watched->inherits("KParts::ReadOnlyPart")  )
      return TQObject::eventFilter(watched, ev);

  KStatusBar * sb = statusBar();
  if ( !sb )
      return TQObject::eventFilter(watched, ev);

  GUIActivateEvent *gae = static_cast<GUIActivateEvent*>(ev);

  if ( gae->activated() )
  {
    TQValueListIterator<StatusBarItem> it = m_statusBarItems.begin();
    for ( ; it != m_statusBarItems.end() ; ++it )
      (*it).ensureItemShown( sb );
  }
  else
  {
    TQValueListIterator<StatusBarItem> it = m_statusBarItems.begin();
    for ( ; it != m_statusBarItems.end() ; ++it )
      (*it).ensureItemHidden( sb );
  }

  return false;

}

KStatusBar * StatusBarExtension::statusBar() const
{
  if ( !m_statusBar )  {
    TQWidget* w = static_cast<KParts::ReadOnlyPart*>(parent())->widget();
    KMainWindow* mw = tqt_dynamic_cast<KMainWindow *>( w->topLevelWidget() );
    if ( mw )
      m_statusBar = mw->statusBar();
  }
  return m_statusBar;
}

void StatusBarExtension::setStatusBar( KStatusBar* status )
{
  m_statusBar = status;
}

void StatusBarExtension::addStatusBarItem( TQWidget * widget, int stretch, bool permanent )
{
  m_statusBarItems.append( StatusBarItem( widget, stretch, permanent ) );
  TQValueListIterator<StatusBarItem> it = m_statusBarItems.fromLast();
  KStatusBar * sb = statusBar();
  Q_ASSERT(sb);
  if (sb)
    (*it).ensureItemShown( sb );
}

void StatusBarExtension::removeStatusBarItem( TQWidget * widget )
{
  KStatusBar * sb = statusBar();
  TQValueListIterator<StatusBarItem> it = m_statusBarItems.begin();
  for ( ; it != m_statusBarItems.end() ; ++it )
    if ( (*it).widget() == widget )
    {
      if ( sb )
        (*it).ensureItemHidden( sb );
      m_statusBarItems.remove( it );
      break;
    }
  if ( it == m_statusBarItems.end() )
    kdWarning(1000) << "StatusBarExtension::removeStatusBarItem. Widget not found : " << widget << endl;
}

#include "statusbarextension.moc"

// vim: ts=2 sw=2 et
