/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

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

#include "kkeybutton.h"
#include "kshortcutdialog.h"

#include <tqcursor.h>
#include <tqdrawutil.h>
#include <tqpainter.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <klocale.h>

#include "config.h"
#ifdef Q_WS_X11
#define XK_XKB_KEYS
#define XK_MISCELLANY
#include <X11/Xlib.h>	// For x11Event() 
#include <X11/keysymdef.h> // For XK_... 

#ifdef KeyPress
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyRelease
#undef KeyPress
#undef FocusOut
#undef FocusIn
#endif // KeyPress
#endif // Q_WS_X11

//static const char* psTemp[] = { 
//  I18N_NOOP("Primary"), I18N_NOOP("Alternate"), I18N_NOOP("Multi-Key") 
//};

class KKeyButtonPrivate
{
 public:
	bool bQtShortcut;
};

/***********************************************************************/
/* KKeyButton                                                          */
/*                                                                     */
/* Initially added by Mark Donohoe <donohoe@kde.org>                   */
/*                                                                     */
/***********************************************************************/

KKeyButton::KKeyButton(TQWidget *parent, const char *name)
:	TQPushButton( parent, name )
{
	d = new KKeyButtonPrivate;
	setFocusPolicy( TQ_StrongFocus );
	m_bEditing = false;
	connect( this, TQT_SIGNAL(clicked()), this, TQT_SLOT(captureShortcut()) );
	setShortcut( TDEShortcut(), true );
}

KKeyButton::~KKeyButton ()
{
	delete d;
}

void KKeyButton::setShortcut( const TDEShortcut& cut, bool bQtShortcut )
{
	d->bQtShortcut = bQtShortcut;
	m_cut = cut;
	TQString keyStr = m_cut.toString();
	keyStr.replace('&', TQString::fromLatin1("&&"));
	setText( keyStr.isEmpty() ? i18n("None") : keyStr );
}

// deprecated //
void KKeyButton::setShortcut( const TDEShortcut& cut )
{
	setShortcut( cut, false );
}

void KKeyButton::setText( const TQString& text )
{
	TQPushButton::setText( text );
	setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}

void KKeyButton::captureShortcut()
{
	TDEShortcut cut;

	m_bEditing = true;
	repaint();

        {
	TDEShortcutDialog dlg( m_cut, d->bQtShortcut, this );
	if( dlg.exec() == KDialog::Accepted )
                cut = dlg.shortcut();
        } // emit the signal after the dialog is destroyed, otherwise it still has grab
        if( !cut.isNull())
	    emit capturedShortcut( cut );

	m_bEditing = false;
	repaint();
}

void KKeyButton::drawButton( TQPainter *painter )
{
  TQPointArray a( 4 );
  a.setPoint( 0, 0, 0) ;
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, 0, 0 );

  TQRegion r1( a );
  painter->setClipRegion( r1 );
  painter->setBrush( backgroundColor().light() );
  painter->drawRoundRect( 0, 0, width(), height(), 20, 20);

  a.setPoint( 0, width(), height() );
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, width(), height() );

  TQRegion r2( a );
  painter->setClipRegion( r2 );
  painter->setBrush( backgroundColor().dark() );
  painter->drawRoundRect( 0, 0, width(), height(), 20, 20 );

  painter->setClipping( false );
  if( width() > 12 && height() > 8 )
    qDrawShadePanel( painter, 6, 4, width() - 12, height() - 8,
                     colorGroup(), true, 1, 0L );
  if ( m_bEditing )
  {
    painter->setPen( colorGroup().base() );
    painter->setBrush( colorGroup().base() );
  }
  else
  {
    painter->setPen( backgroundColor() );
    painter->setBrush( backgroundColor() );
  }
  if( width() > 14 && height() > 10 )
    painter->drawRect( 7, 5, width() - 14, height() - 10 );

  drawButtonLabel( painter );

  painter->setPen( colorGroup().text() );
  painter->setBrush( NoBrush );
  if( hasFocus() || m_bEditing )
  {
    if( width() > 16 && height() > 12 )
      painter->drawRect( 8, 6, width() - 16, height() - 12 );
  }

}

void KKeyButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kkeybutton.moc"
