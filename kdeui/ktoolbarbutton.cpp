/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              (C) 1999 Chris Schlaeger (cs@kde.org)
              (C) 1999 Kurt Granroth (granroth@kde.org)

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

#include <config.h>
#include <string.h>

#include "ktoolbarbutton.h"
#include "ktoolbar.h"

#include <tqstyle.h>
#include <tqimage.h>
#include <tqtimer.h>
#include <tqdrawutil.h>
#include <tqtooltip.h>
#include <tqbitmap.h>
#include <tqpopupmenu.h>
#include <tqcursor.h>
#include <tqpainter.h>
#include <tqlayout.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>

// needed to get our instance
#include <kmainwindow.h>

template class TQIntDict<KToolBarButton>;

class KToolBarButtonPrivate
{
public:
  KToolBarButtonPrivate()
  {
    m_buttonDown  = false;

    m_noStyle     = false;
    m_isSeparator = false;
    m_isRadio     = false;
    m_highlight   = false;
    m_isRaised    = false;
    m_isActive    = false;

    m_iconName    = TQString::null;
    m_iconText    = KToolBar::IconOnly;
    m_iconSize    = 0;

    m_parent   = 0;
    m_instance = KGlobal::instance();
  }
  ~KToolBarButtonPrivate()
  {
  }

  int     m_id;
  bool    m_buttonDown : 1;
  bool    m_noStyle: 1;
  bool    m_isSeparator: 1;
  bool    m_isRadio: 1;
  bool    m_highlight: 1;
  bool    m_isRaised: 1;
  bool    m_isActive: 1;

  TQString m_iconName;

  KToolBar *m_parent;
  KToolBar::IconText m_iconText;
  int m_iconSize;
  TQSize size;

  TQPoint m_mousePressPos;

  KInstance  *m_instance;
};

// This will construct a separator
KToolBarButton::KToolBarButton( TQWidget *_parent, const char *_name )
  : TQToolButton( _parent , _name)
{
  d = new KToolBarButtonPrivate;

  resize(6,6);
  hide();
  d->m_isSeparator = true;
}

KToolBarButton::KToolBarButton( const TQString& _icon, int _id,
                                TQWidget *_parent, const char *_name,
                                const TQString &_txt, KInstance *_instance )
    : TQToolButton( _parent, _name ), d( 0 )
{
  d = new KToolBarButtonPrivate;

  d->m_id     = _id;
  TQToolButton::setTextLabel(_txt);
  d->m_instance = _instance;

  d->m_parent = tqt_dynamic_cast<KToolBar*>(_parent);
  if (d->m_parent) {
    connect(d->m_parent, TQT_SIGNAL( modechange() ),
            this,         TQT_SLOT( modeChange() ));
  }

  setFocusPolicy( TQ_NoFocus );

  // connect all of our slots and start trapping events
  connect(this, TQT_SIGNAL( clicked() ),
          this, TQT_SLOT( slotClicked() ) );
  connect(this, TQT_SIGNAL( pressed() ),
          this, TQT_SLOT( slotPressed() ) );
  connect(this, TQT_SIGNAL( released() ),
          this, TQT_SLOT( slotReleased() ) );
  installEventFilter(this);

  d->m_iconName = _icon;

  // do our initial setup
  modeChange();
}

KToolBarButton::KToolBarButton( const TQPixmap& pixmap, int _id,
                                TQWidget *_parent, const char *name,
                                const TQString& txt)
    : TQToolButton( _parent, name ), d( 0 )
{
  d = new KToolBarButtonPrivate;

  d->m_id       = _id;
  TQToolButton::setTextLabel(txt);

  d->m_parent = tqt_dynamic_cast<KToolBar*>(_parent);
  if (d->m_parent) {
    connect(d->m_parent, TQT_SIGNAL( modechange() ),
            this,         TQT_SLOT( modeChange() ));
  }

  setFocusPolicy( TQ_NoFocus );

  // connect all of our slots and start trapping events
  connect(this, TQT_SIGNAL( clicked() ),
          this, TQT_SLOT( slotClicked() ));
  connect(this, TQT_SIGNAL( pressed() ),
          this, TQT_SLOT( slotPressed() ));
  connect(this, TQT_SIGNAL( released() ),
          this, TQT_SLOT( slotReleased() ));
  installEventFilter(this);

  // set our pixmap and do our initial setup
  setIconSet( TQIconSet( pixmap ));
  modeChange();
}

KToolBarButton::~KToolBarButton()
{
  delete d; d = 0;
}

void KToolBarButton::modeChange()
{
  TQSize mysize;

  // grab a few global variables for use in this function and others
  if (d->m_parent) {
    d->m_highlight = d->m_parent->highlight();
    d->m_iconText  = d->m_parent->iconText();

    d->m_iconSize = d->m_parent->iconSize();
  }
  if (!d->m_iconName.isNull())
    setIcon(d->m_iconName);

  // we'll start with the size of our pixmap
  int pix_width  = d->m_iconSize;
  if ( d->m_iconSize == 0 ) {
      if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
          pix_width = IconSize( KIcon::MainToolbar );
      else
          pix_width = IconSize( KIcon::Toolbar );
  }
  int pix_height = pix_width;

  int text_height = 0;
  int text_width = 0;

  TQToolTip::remove(this);
  if (d->m_iconText != KToolBar::IconOnly)
  {
    // okay, we have to deal with fonts.  let's get our information now
    TQFont tmp_font = KGlobalSettings::toolBarFont();

    // now parse out our font sizes from our chosen font
    TQFontMetrics fm(tmp_font);

    text_height = fm.lineSpacing();
    text_width  = fm.width(textLabel());

    // none of the other modes want tooltips
  }
  else
  {
    TQToolTip::add(this, textLabel());
  }

  switch (d->m_iconText)
  {
  case KToolBar::IconOnly:
    mysize = TQSize(pix_width, pix_height);
    break;

  case KToolBar::IconTextRight:
    mysize = TQSize(pix_width + text_width + 4, pix_height);
    break;

  case KToolBar::TextOnly:
    mysize = TQSize(text_width + 4, text_height);
    break;

  case KToolBar::IconTextBottom:
    mysize = TQSize((text_width + 4 > pix_width) ? text_width + 4 : pix_width, pix_height + text_height);
    break;

  default:
    break;
  }

  mysize = tqstyle().tqsizeFromContents(TQStyle::CT_ToolButton, this, mysize).
               expandedTo(TQApplication::globalStrut());

  // make sure that this isn't taller then it is wide
  if (mysize.height() > mysize.width())
    mysize.setWidth(mysize.height());

  d->size = mysize;
  updateGeometry();
}

void KToolBarButton::setTextLabel( const TQString& text, bool tipToo)
{
  if (text.isNull())
    return;

  TQString txt(text);
  if (txt.endsWith(TQString::tqfromLatin1("...")))
    txt.truncate(txt.length() - 3);

  TQToolButton::setTextLabel(txt, tipToo);
  update();
}

void KToolBarButton::setText( const TQString& text)
{
  setTextLabel(text, true);
  modeChange();
}

void KToolBarButton::setIcon( const TQString &icon )
{
  d->m_iconName = icon;
  if (d->m_parent)
    d->m_iconSize = d->m_parent->iconSize();
  // TQObject::name() return "const char *" instead of TQString.
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    TQToolButton::setIconSet( d->m_instance->iconLoader()->loadIconSet(
        d->m_iconName, KIcon::MainToolbar, d->m_iconSize ));
  else
    TQToolButton::setIconSet( d->m_instance->iconLoader()->loadIconSet(
        d->m_iconName, KIcon::Toolbar, d->m_iconSize ));
}

void KToolBarButton::setIconSet( const TQIconSet &iconset )
{
  TQToolButton::setIconSet( iconset );
}

// remove?
void KToolBarButton::setPixmap( const TQPixmap &pixmap )
{
  if( pixmap.isNull()) // called by TQToolButton
  {
    TQToolButton::setPixmap( pixmap );
    return;
  }
  TQIconSet set = iconSet();
  set.setPixmap( pixmap, TQIconSet::Automatic, TQIconSet::Active );
  TQToolButton::setIconSet( set );
}

void KToolBarButton::setDefaultPixmap( const TQPixmap &pixmap )
{
  TQIconSet set = iconSet();
  set.setPixmap( pixmap, TQIconSet::Automatic, TQIconSet::Normal );
  TQToolButton::setIconSet( set );
}

void KToolBarButton::setDisabledPixmap( const TQPixmap &pixmap )
{
  TQIconSet set = iconSet();
  set.setPixmap( pixmap, TQIconSet::Automatic, TQIconSet::Disabled );
  TQToolButton::setIconSet( set );
}

void KToolBarButton::setDefaultIcon( const TQString& icon )
{
  TQIconSet set = iconSet();
  TQPixmap pm;
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::MainToolbar,
        d->m_iconSize );
  else
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::Toolbar,
        d->m_iconSize );
  set.setPixmap( pm, TQIconSet::Automatic, TQIconSet::Normal );
  TQToolButton::setIconSet( set );
}

void KToolBarButton::setDisabledIcon( const TQString& icon )
{
  TQIconSet set = iconSet();
  TQPixmap pm;
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::MainToolbar,
        d->m_iconSize );
  else
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::Toolbar,
        d->m_iconSize );
  set.setPixmap( pm, TQIconSet::Automatic, TQIconSet::Disabled );
  TQToolButton::setIconSet( set );
}

TQPopupMenu *KToolBarButton::popup()
{
  // obsolete
  // KDE4: remove me
  return TQToolButton::popup();
}

void KToolBarButton::setPopup(TQPopupMenu *p, bool)
{
  TQToolButton::setPopup(p);
  TQToolButton::setPopupDelay(-1);
}


void KToolBarButton::setDelayedPopup (TQPopupMenu *p, bool)
{
  TQToolButton::setPopup(p);
  TQToolButton::setPopupDelay(TQApplication::startDragTime());
}

void KToolBarButton::leaveEvent(TQEvent *)
{
  if( d->m_isRaised || d->m_isActive )
  {
    d->m_isRaised = false;
    d->m_isActive = false;
    tqrepaint(false);
  }

  emit highlighted(d->m_id, false);
}

void KToolBarButton::enterEvent(TQEvent *)
{
  if (d->m_highlight)
  {
    if (isEnabled())
    {
      d->m_isActive = true;
      if (!isToggleButton())
        d->m_isRaised = true;
    }
    else
    {
      d->m_isRaised = false;
      d->m_isActive = false;
    }

    tqrepaint(false);
  }
  emit highlighted(d->m_id, true);
}

bool KToolBarButton::eventFilter(TQObject *o, TQEvent *ev)
{
  if (TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(this))
  {

    // Popup the menu when the left mousebutton is pressed and the mouse
    // is moved by a small distance.
    if (TQToolButton::popup())
    {
      if (ev->type() == TQEvent::MouseButtonPress)
      {
        TQMouseEvent* mev = TQT_TQMOUSEEVENT(ev);
        d->m_mousePressPos = mev->pos();
      }
      else if (ev->type() == TQEvent::MouseMove)
      {
        TQMouseEvent* mev = TQT_TQMOUSEEVENT(ev);
        if ((mev->pos() - d->m_mousePressPos).manhattanLength()
              > KGlobalSettings::dndEventDelay())
        {
          openPopup();
          return true;
        }
      }
    }

    if (d->m_isRadio &&
	(ev->type() == TQEvent::MouseButtonPress ||
         ev->type() == TQEvent::MouseButtonRelease ||
         ev->type() == TQEvent::MouseButtonDblClick) && isOn())
      return true;

    // From Kai-Uwe Sattler <kus@iti.CS.Uni-Magdeburg.De>
    if (ev->type() == TQEvent::MouseButtonDblClick)
    {
      emit doubleClicked(d->m_id);
      return false;
    }
  }

  return TQToolButton::eventFilter(o, ev);
}

void KToolBarButton::mousePressEvent( TQMouseEvent * e )
{
  d->m_buttonDown = true;

  if ( e->button() == Qt::MidButton )
  {
    // Get TQToolButton to show the button being down while pressed
    TQMouseEvent ev( TQEvent::MouseButtonPress, e->pos(), e->globalPos(), Qt::LeftButton, e->state() );
    TQToolButton::mousePressEvent(&ev);
    return;
  }
  TQToolButton::mousePressEvent(e);
}

void KToolBarButton::mouseReleaseEvent( TQMouseEvent * e )
{
  TQt::ButtonState state = TQt::ButtonState(e->button() | (e->state() & KeyButtonMask));
  if ( e->button() == Qt::MidButton )
  {
    TQMouseEvent ev( TQEvent::MouseButtonRelease, e->pos(), e->globalPos(), Qt::LeftButton, e->state() );
    TQToolButton::mouseReleaseEvent(&ev);
  }
  else
    TQToolButton::mouseReleaseEvent(e);

  if ( !d->m_buttonDown )
    return;
  d->m_buttonDown = false;

  if ( hitButton( e->pos() ) )
    emit buttonClicked( d->m_id, state );
}

void KToolBarButton::drawButton( TQPainter *_painter )
{
  TQStyle::SFlags flags   = TQStyle::Style_Default;
  TQStyle::SCFlags active = TQStyle::SC_None;

  if (isDown()) {
    flags  |= TQStyle::Style_Down;
    active |= TQStyle::SC_ToolButton;
  }
  if (isEnabled()) 	flags |= TQStyle::Style_Enabled;
  if (isOn()) 		flags |= TQStyle::Style_On;
  if (isEnabled() && hasMouse())	flags |= TQStyle::Style_Raised;
  if (hasFocus())	flags |= TQStyle::Style_HasFocus;

  // Draw a styled toolbutton
  tqstyle().tqdrawComplexControl(TQStyle::CC_ToolButton, _painter, this, rect(),
	tqcolorGroup(), flags, TQStyle::SC_ToolButton, active, TQStyleOption());

  int dx, dy;
  TQFont tmp_font(KGlobalSettings::toolBarFont());
  TQFontMetrics fm(tmp_font);
  TQRect textRect;
  int textFlags = 0;

  if (d->m_iconText == KToolBar::IconOnly) // icon only
  {
    TQPixmap pixmap = iconSet().pixmap( TQIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? TQIconSet::Active : TQIconSet::Normal) :
            	TQIconSet::Disabled,
        isOn() ? TQIconSet::On : TQIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = ( width() - pixmap.width() ) / 2;
      dy = ( height() - pixmap.height() ) / 2;
      if ( isDown() && tqstyle().tqstyleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }
  }
  else if (d->m_iconText == KToolBar::IconTextRight) // icon and text (if any)
  {
    TQPixmap pixmap = iconSet().pixmap( TQIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? TQIconSet::Active : TQIconSet::Normal) :
            	TQIconSet::Disabled,
        isOn() ? TQIconSet::On : TQIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = 4;
      dy = ( height() - pixmap.height() ) / 2;
      if ( isDown() && tqstyle().tqstyleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }

    if (!textLabel().isNull())
    {
      textFlags = AlignVCenter|AlignLeft;
      if (!pixmap.isNull())
        dx = 4 + pixmap.width() + 2;
      else
        dx = 4;
      dy = 0;
      if ( isDown() && tqstyle().tqstyleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = TQRect(dx, dy, width()-dx, height());
    }
  }
  else if (d->m_iconText == KToolBar::TextOnly)
  {
    if (!textLabel().isNull())
    {
      textFlags = AlignVCenter|AlignLeft;
      dx = (width() - fm.width(textLabel())) / 2;
      dy = (height() - fm.lineSpacing()) / 2;
      if ( isDown() && tqstyle().tqstyleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = TQRect( dx, dy, fm.width(textLabel()), fm.lineSpacing() );
    }
  }
  else if (d->m_iconText == KToolBar::IconTextBottom)
  {
    TQPixmap pixmap = iconSet().pixmap( TQIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? TQIconSet::Active : TQIconSet::Normal) :
            	TQIconSet::Disabled,
        isOn() ? TQIconSet::On : TQIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = (width() - pixmap.width()) / 2;
      dy = (height() - fm.lineSpacing() - pixmap.height()) / 2;
      if ( isDown() && tqstyle().tqstyleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }

    if (!textLabel().isNull())
    {
      textFlags = AlignBottom|AlignHCenter;
      dx = (width() - fm.width(textLabel())) / 2;
      dy = height() - fm.lineSpacing() - 4;

      if ( isDown() && tqstyle().tqstyleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = TQRect( dx, dy, fm.width(textLabel()), fm.lineSpacing() );
    }
  }

  // Draw the text at the position given by textRect, and using textFlags
  if (!textLabel().isNull() && !textRect.isNull())
  {
      _painter->setFont(KGlobalSettings::toolBarFont());
      if (!isEnabled())
        _painter->setPen(tqpalette().disabled().dark());
      else if(d->m_isRaised)
        _painter->setPen(KGlobalSettings::toolBarHighlightColor());
      else
	_painter->setPen( tqcolorGroup().buttonText() );
      _painter->drawText(textRect, textFlags, textLabel());
  }

  if (TQToolButton::popup())
  {
    TQStyle::SFlags arrowFlags = TQStyle::Style_Default;

    if (isDown())	arrowFlags |= TQStyle::Style_Down;
    if (isEnabled()) 	arrowFlags |= TQStyle::Style_Enabled;

      tqstyle().tqdrawPrimitive(TQStyle::PE_ArrowDown, _painter,
          TQRect(width()-7, height()-7, 7, 7), tqcolorGroup(),
	  arrowFlags, TQStyleOption() );
  }
}

void KToolBarButton::paletteChange(const TQPalette &)
{
  if(!d->m_isSeparator)
  {
    modeChange();
    tqrepaint(false); // no need to delete it first therefore only false
  }
}

bool KToolBarButton::event(TQEvent *e)
{
  if (e->type() == TQEvent::ParentFontChange || e->type() == TQEvent::ApplicationFontChange)
  {
     //If we use toolbar text, apply the settings again, to relayout...
     if (d->m_iconText != KToolBar::IconOnly)
       modeChange();
     return true;
  }

  return TQToolButton::event(e);
}


void KToolBarButton::showMenu()
{
  // obsolete
  // KDE4: remove me
}

void KToolBarButton::slotDelayTimeout()
{
  // obsolete
  // KDE4: remove me
}

void KToolBarButton::slotClicked()
{
  emit clicked( d->m_id );

  // emit buttonClicked when the button was clicked while being in an extension popupmenu
  if ( d->m_parent && !d->m_parent->rect().contains( geometry().center() ) ) {
    ButtonState state = KApplication::keyboardMouseState();
    if ( ( state & Qt::MouseButtonMask ) == Qt::NoButton )
      state = ButtonState( Qt::LeftButton | state );
    emit buttonClicked( d->m_id, state ); // Doesn't work with MidButton
  }
}

void KToolBarButton::slotPressed()
{
  emit pressed( d->m_id );
}

void KToolBarButton::slotReleased()
{
  emit released( d->m_id );
}

void KToolBarButton::slotToggled()
{
  emit toggled( d->m_id );
}

void KToolBarButton::setNoStyle(bool no_style)
{
    d->m_noStyle = no_style;

    modeChange();
    d->m_iconText = KToolBar::IconTextRight;
    tqrepaint(false);
}

void KToolBarButton::setRadio (bool f)
{
    if ( d )
	d->m_isRadio = f;
}

void KToolBarButton::on(bool flag)
{
  if(isToggleButton())
    setOn(flag);
  else
  {
    setDown(flag);
    leaveEvent((TQEvent *) 0);
  }
  tqrepaint();
}

void KToolBarButton::toggle()
{
  setOn(!isOn());
  tqrepaint();
}

void KToolBarButton::setToggle(bool flag)
{
  setToggleButton(flag);
  if (flag)
    connect(this, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotToggled()));
  else
    disconnect(this, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotToggled()));
}

TQSize KToolBarButton::tqsizeHint() const
{
   return d->size;
}

TQSize KToolBarButton::tqminimumSizeHint() const
{
   return d->size;
}

TQSize KToolBarButton::tqminimumSize() const
{
   return d->size;
}

bool KToolBarButton::isRaised() const
{
    return d->m_isRaised;
}

bool KToolBarButton::isActive() const
{
    return d->m_isActive;
}

int KToolBarButton::iconTextMode() const
{
    return static_cast<int>( d->m_iconText );
}

int KToolBarButton::id() const
{
    return d->m_id;
}

// KToolBarButtonList
KToolBarButtonList::KToolBarButtonList()
{
   setAutoDelete(false);
}

void KToolBarButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktoolbarbutton.moc"
