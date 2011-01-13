/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kconfig.h>
#include <kapplication.h>
#include <kdialog.h>
#include <kwhatsthismanager_p.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include <tqlayout.h>
#include <tqobjectlist.h>
#include <tqguardedptr.h>
#include <tqlineedit.h>
#include <tqvaluelist.h>
#include <tqtimer.h>
#include <tqcursor.h>

#include "config.h"
#ifdef Q_WS_X11
#include <netwm.h> 
#endif

const int KDialog::mMarginSize = 11;
const int KDialog::mSpacingSize = 6;

template class TQPtrList<TQLayoutItem>;

KDialog::KDialog(TQWidget *parent, const char *name, bool modal, WFlags f)
  : TQDialog(parent, name, modal, f), d(0)
{
    KWhatsThisManager::init ();
}

//
// Grab QDialogs keypresses if non-modal.
//
void KDialog::keyPressEvent(TQKeyEvent *e)
{
  if ( e->state() == 0 )
  {
    switch ( e->key() )
    {
      case Key_Escape:
      case Key_Enter:
      case Key_Return:
      {
        if(testWFlags((WFlags)(WType_Dialog | WShowModal)))
	{
          TQDialog::keyPressEvent(e);
	}
        else
        {
   	  e->ignore();
        }
      }
      break;
      default:
	e->ignore();
	return;
    }
  }
  else
  {
      // accept the dialog when Ctrl-Return is pressed
      if ( e->state() == ControlButton &&
           (e->key() == Key_Return || e->key() == Key_Enter) )
      {
          e->accept();
          accept();
      }
      else
      {
          e->ignore();
      }
  }
}


int KDialog::marginHint()
{
  return mMarginSize;
}


int KDialog::spacingHint()
{
  return mSpacingSize;
}

// KDE4: Remove me
void KDialog::polish()
{
  TQDialog::polish();
}


void KDialog::setCaption( const TQString &_caption )
{
  TQString caption = kapp ? kapp->makeStdCaption( _caption ) : _caption;
  setPlainCaption( caption );
}


void KDialog::setPlainCaption( const TQString &caption )
{
  TQDialog::setCaption( caption );

#ifdef Q_WS_X11
  NETWinInfo info( qt_xdisplay(), winId(), qt_xrootwin(), 0 );
  info.setName( caption.utf8().data() );
#endif
}


void KDialog::resizeLayout( TQWidget *w, int margin, int spacing )
{
  if( w->layout() )
  {
    resizeLayout( TQT_TQLAYOUTITEM(w->layout()), margin, spacing );
  }

  if( !w->childrenListObject().isEmpty() )
  {
    const TQObjectList l = w->childrenListObject();
    TQObjectListIterator itr(l);
    TQObject *o;
    while ((o = itr.current()) != 0) {
      if( o->isWidgetType() )
      {
	resizeLayout( (TQWidget*)o, margin, spacing );
      }
      ++itr;
    }
  }
}


void KDialog::resizeLayout( TQLayoutItem *lay, int margin, int spacing )
{
  TQLayoutIterator it = lay->iterator();
  TQLayoutItem *child;
  while ( (child = it.current() ) )
  {
    resizeLayout( child, margin, spacing );
    ++it;
  }
  if( lay->layout() )
  {
    lay->layout()->setMargin( margin );
    lay->layout()->setSpacing( spacing );
  }
}

static TQRect screenRect( TQWidget *w, int screen )
{
  TQDesktopWidget *desktop = TQApplication::desktop();
  KConfig gc("kdeglobals", false, false);
  gc.setGroup("Windows");
  if (desktop->isVirtualDesktop() &&
      gc.readBoolEntry("XineramaEnabled", true) &&
      gc.readBoolEntry("XineramaPlacementEnabled", true)) {
    if ( screen < 0 || screen >= desktop->numScreens() ) {
      if ( screen == -1 ) {
        screen = desktop->primaryScreen();
      } else if ( screen == -3 ) {
        screen = desktop->screenNumber( TQCursor::pos() );
      } else {
        screen = desktop->screenNumber( w );
      }
    }
    return desktop->availableGeometry(screen);
  } else {
    return desktop->geometry();
  }
}

void KDialog::centerOnScreen( TQWidget *w, int screen )
{
  if ( !w )
    return;
  TQRect r = screenRect( w, screen );

  w->move( r.center().x() - w->width()/2,
           r.center().y() - w->height()/2 );
}

bool KDialog::avoidArea( TQWidget *w, const TQRect& area, int screen )
{
  if ( !w )
    return false;
  TQRect fg = w->frameGeometry();
  if ( !fg.intersects( area ) )
    return true; // nothing to do.

  TQRect scr = screenRect( w, screen );
  TQRect avoid( area ); // let's add some margin
  avoid.moveBy( -5, -5 );
  avoid.rRight() += 10;
  avoid.rBottom() += 10;

  if ( QMAX( fg.top(), avoid.top() ) <= QMIN( fg.bottom(), avoid.bottom() ) )
  {
    // We need to move the widget up or down
    int spaceAbove = QMAX(0, avoid.top() - scr.top());
    int spaceBelow = QMAX(0, scr.bottom() - avoid.bottom());
    if ( spaceAbove > spaceBelow ) // where's the biggest side?
      if ( fg.height() <= spaceAbove ) // big enough?
        fg.setY( avoid.top() - fg.height() );
      else
        return false;
    else
      if ( fg.height() <= spaceBelow ) // big enough?
        fg.setY( avoid.bottom() );
      else
        return false;
  }

  if ( QMAX( fg.left(), avoid.left() ) <= QMIN( fg.right(), avoid.right() ) )
  {
    // We need to move the widget left or right
    int spaceLeft = QMAX(0, avoid.left() - scr.left());
    int spaceRight = QMAX(0, scr.right() - avoid.right());
    if ( spaceLeft > spaceRight ) // where's the biggest side?
      if ( fg.width() <= spaceLeft ) // big enough?
        fg.setX( avoid.left() - fg.width() );
      else
        return false;
    else
      if ( fg.width() <= spaceRight ) // big enough?
        fg.setX( avoid.right() );
      else
        return false;
  }
  //kdDebug() << "Moving window to " << fg.x() << "," << fg.y() << endl;
  w->move(fg.x(), fg.y());
  return true;
}

class KDialogQueuePrivate
{
public:
  TQValueList< TQGuardedPtr<TQDialog> > queue;
  bool busy;
};

static KStaticDeleter<KDialogQueue> ksdkdq;

KDialogQueue *KDialogQueue::_self=0;

KDialogQueue* KDialogQueue::self()
{
   if (!_self)
      _self = ksdkdq.setObject(_self, new KDialogQueue);
   return _self;
}

KDialogQueue::KDialogQueue() : d(new KDialogQueuePrivate)
{
   d->busy = false;
}

KDialogQueue::~KDialogQueue()
{
   delete d;
   _self = 0;
}

// static
void KDialogQueue::queueDialog(TQDialog *dialog)
{
   KDialogQueue *_this = self();
   _this->d->queue.append(dialog);
   TQTimer::singleShot(0, _this, TQT_SLOT(slotShowQueuedDialog()));
}

void KDialogQueue::slotShowQueuedDialog()
{
   if (d->busy)
      return;
   TQDialog *dialog;
   do {
       if(d->queue.isEmpty())
         return;
      dialog = d->queue.first();
      d->queue.pop_front();
   }
   while(!dialog);

   d->busy = true;
   dialog->exec();
   d->busy = false;
   delete dialog;

   if (!d->queue.isEmpty())
      TQTimer::singleShot(20, this, TQT_SLOT(slotShowQueuedDialog()));
   else
      ksdkdq.destructObject(); // Suicide.
}

void KDialog::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdialog.moc"
