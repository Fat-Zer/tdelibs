/* Keramik Style for KDE3
   Copyright (c) 2002 Malte Starostik <malte@kde.org>
             (c) 2002,2003 Maksim Orlovich <mo002j@mail.rochester.edu>

   based on the KDE3 HighColor Style

   Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
             (C) 2001-2002 Fredrik H�glund  <fredrik@kde.org>

   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
             (C) 2000 Dirk Mueller          <mueller@kde.org>
             (C) 2001 Martijn Klingens      <klingens@kde.org>

   Progressbar code based on KStyle, Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>,
   Improvements to progressbar animation from Plastik, Copyright (C) 2003 Sandro Giessl <sandro@giessl.com>

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

// $Id$

#include <tqapplication.h>
#include <tqbitmap.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqdrawutil.h>
#include <tqframe.h>
#include <tqheader.h>
#include <tqintdict.h>
#include <tqlineedit.h>
#include <tqlistbox.h>
#include <tqmenubar.h>
#include <tqpainter.h>
#include <tqpointarray.h>
#include <tqprogressbar.h>
#include <tqpushbutton.h>
#include <tqsettings.h>
#include <tqslider.h>
#include <tqstyleplugin.h>
#include <tqtabbar.h>
#include <tqtimer.h>
#include <tqtoolbar.h>
#include <tqtoolbutton.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

#include "keramik.moc"

#include "gradients.h"
#include "colorutil.h"
#include "keramikrc.h"
#include "keramikimage.h"

#include "bitmaps.h"
#include "pixmaploader.h"

#define loader Keramik::PixmapLoader::the()

// -- Style Plugin Interface -------------------------
class KeramikStylePlugin : public TQStylePlugin
{
public:
	KeramikStylePlugin() {}
	~KeramikStylePlugin() {}

	TQStringList keys() const
	{
		if (TQPixmap::defaultDepth() > 8)
			return TQStringList() << "Keramik";
		else
			return TQStringList();
	}

	TQStyle* create( const TQString& key )
	{
		if ( key == "keramik" ) return new KeramikStyle();
		return 0;
	}
};

KDE_Q_EXPORT_PLUGIN( KeramikStylePlugin )
// ---------------------------------------------------


// ### Remove globals
/*
TQBitmap lightBmp;
TQBitmap grayBmp;
TQBitmap dgrayBmp;
TQBitmap centerBmp;
TQBitmap maskBmp;
TQBitmap xBmp;
*/
namespace
{
	const int itemFrame       = 2;
	const int itemHMargin     = 6;
	const int itemVMargin     = 0;
	const int arrowHMargin    = 6;
	const int rightBorder     = 12;
	const char* kdeToolbarWidget = "kde toolbar widget";

	const int smallButMaxW    = 27;
	const int smallButMaxH    = 20;
	const int titleBarH       = 22;
}
// ---------------------------------------------------------------------------

namespace
{
	void drawKeramikArrow(TQPainter* p, TQColorGroup cg, TQRect r, TQStyle::TQ_PrimitiveElement pe, bool down, bool enabled)
	{
		TQPointArray a;

		switch(pe)
		 {
			case TQStyle::PE_ArrowUp:
				a.setPoints(TQCOORDARRLEN(keramik_up_arrow), keramik_up_arrow);
				break;

			case TQStyle::PE_ArrowDown:
				a.setPoints(TQCOORDARRLEN(keramik_down_arrow), keramik_down_arrow);
				break;

			case TQStyle::PE_ArrowLeft:
				a.setPoints(TQCOORDARRLEN(keramik_left_arrow), keramik_left_arrow);
				break;

			default:
				a.setPoints(TQCOORDARRLEN(keramik_right_arrow), keramik_right_arrow);
		}

		p->save();
		/*if ( down )
			p->translate( tqpixelMetric( PM_ButtonShiftHorizontal ),
						tqpixelMetric( PM_ButtonShiftVertical ) );
		*/

		if ( enabled ) {
			//CHECKME: Why is the -1 needed?
			a.translate( r.x() + r.width() / 2 - 1, r.y() + r.height() / 2 );

			if (!down)
				p->setPen( cg.buttonText() );
			else
				p->setPen ( cg.button() );
			p->drawLineSegments( a );
		} else {
			a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 + 1 );
			p->setPen( cg.light() );
			p->drawLineSegments( a );
			a.translate( -1, -1 );
			p->setPen( cg.mid() );
			p->drawLineSegments( a );
		}
		p->restore();
	}
}

// XXX
/* reimp. */
void KeramikStyle::renderMenuBlendPixmap( KPixmap& pix, const TQColorGroup &cg,
		const TQPopupMenu* /* popup */ ) const
{
	TQColor col = cg.button();

#ifdef Q_WS_X11 // Only draw menu gradients on TrueColor, X11 visuals
	if ( TQPaintDevice::x11AppDepth() >= 24 )
		KPixmapEffect::gradient( pix, col.light(120), col.dark(115),
				KPixmapEffect::HorizontalGradient );
	else
#endif
	pix.fill( col );
}

// XXX
TQRect KeramikStyle::subRect(SubRect r, const TQWidget *widget) const
{
	// We want the focus rect for buttons to be adjusted from
	// the Qt3 defaults to be similar to Qt 2's defaults.
	// -------------------------------------------------------------------
	switch ( r )
	{
		case SR_PushButtonFocusRect:
		{
			const TQPushButton* button = (const TQPushButton*) widget;
			TQRect wrect(widget->rect());

			if (button->isDefault() || button->autoDefault())
			{
				return TQRect(wrect.x() + 6, wrect.y() + 5, wrect.width() - 12,  wrect.height() - 10);
			}
			else
			{
				return TQRect(wrect.x() + 3, wrect.y() + 5, wrect.width() - 8,  wrect.height() - 10);
			}

			break;
		}

		case SR_ComboBoxFocusRect:
		{
			return querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxEditField );
		}

		case SR_CheckBoxFocusRect:
		{
			const TQCheckBox* cb = static_cast<const TQCheckBox*>(widget);

			//Only checkbox, no label
			if (cb->text().isEmpty() && (cb->pixmap() == 0) )
			{
				TQRect bounding = cb->rect();
				TQSize checkDim = loader.size( keramik_checkbox_on);
				int   cw = checkDim.width();
				int   ch = checkDim.height();

				TQRect checkbox(bounding.x() + 1, bounding.y() + 1 + (bounding.height() - ch)/2,
								cw - 3, ch - 4);

				return checkbox;
			}

			//Fallthrough intentional
		}

		default:
			return KStyle::subRect( r, widget );
	}
}


TQPixmap KeramikStyle::stylePixmap(StylePixmap stylepixmap,
									const TQWidget* widget,
									const TQStyleOption& opt) const
{
    switch (stylepixmap) {
		case SP_TitleBarMinButton:
			return Keramik::PixmapLoader::the().pixmap(keramik_title_iconify,
				Qt::black, Qt::black, false, false);
			//return qpixmap_from_bits( iconify_bits, "title-iconify.png" );
		case SP_TitleBarMaxButton:
			return Keramik::PixmapLoader::the().pixmap(keramik_title_maximize,
				Qt::black, Qt::black, false, false);
		case SP_TitleBarCloseButton:
			if (widget && widget->inherits("KDockWidgetHeader"))
				return Keramik::PixmapLoader::the().pixmap(keramik_title_close_tiny,
				Qt::black, Qt::black, false, false);
			else	return Keramik::PixmapLoader::the().pixmap(keramik_title_close,
				Qt::black, Qt::black, false, false);
		case SP_TitleBarNormalButton:
			return Keramik::PixmapLoader::the().pixmap(keramik_title_restore,
				Qt::black, Qt::black, false, false);
		default:
			break;
	}

	return KStyle::stylePixmap(stylepixmap, widget, opt);
}




KeramikStyle::KeramikStyle()
	:KStyle( AllowMenuTransparency | FilledFrameWorkaround, ThreeButtonScrollBar ),
		maskMode(false), formMode(false),
		toolbarBlendWidget(0), titleBarMode(None), flatMode(false), customScrollMode(false), kickerMode(false)
{
	forceSmallMode = false;
	hoverWidget    = 0;

	TQSettings settings;

	highlightScrollBar = settings.readBoolEntry("/keramik/Settings/highlightScrollBar", true);
	animateProgressBar = settings.readBoolEntry("/keramik/Settings/animateProgressBar", false);

	if (animateProgressBar)
	{
		animationTimer = new TQTimer( this );
		connect( animationTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(updateProgressPos()) );
	}
	
	firstComboPopupRelease = false;
}

void KeramikStyle::updateProgressPos()
{
	//Update the registered progressbars.
	TQMap<TQProgressBar*, int>::iterator iter;
	bool visible = false;
	for (iter = progAnimWidgets.begin(); iter != progAnimWidgets.end(); ++iter)
	{
		TQProgressBar* pbar = iter.key(); 
		if (pbar->isVisible() && pbar->isEnabled() &&
			pbar->progress() != pbar->totalSteps())
		{
			++iter.data();
			if (iter.data() == 28)
				iter.data() = 0;
			iter.key()->update();
		}
		if (iter.key()->isVisible())
			visible = true;
		
	}
	if (!visible)
		animationTimer->stop();
}

KeramikStyle::~KeramikStyle()
{
	Keramik::PixmapLoader::release();
	Keramik::GradientPainter::releaseCache();
	KeramikDbCleanup();
}

void KeramikStyle::polish(TQApplication* app)
{
	if (!qstrcmp(app->argv()[0], "kicker"))
		kickerMode = true;
}

void KeramikStyle::polish(TQWidget* widget)
{
	// Put in order of highest occurrence to maximise hit rate
	if ( widget->inherits( TQPUSHBUTTON_OBJECT_NAME_STRING )  || widget->inherits( TQCOMBOBOX_OBJECT_NAME_STRING ) || widget->inherits(TQTOOLBUTTON_OBJECT_NAME_STRING) )
	{
		widget->installEventFilter(this);
		if ( widget->inherits( TQCOMBOBOX_OBJECT_NAME_STRING ) )
			widget->setBackgroundMode( NoBackground );
	}
	else if ( widget->inherits( TQMENUBAR_OBJECT_NAME_STRING ) || widget->inherits( TQPOPUPMENU_OBJECT_NAME_STRING ) )
		widget->setBackgroundMode( NoBackground );

 	else if ( widget->parentWidget() &&
			  ( ( widget->inherits( TQLISTBOX_OBJECT_NAME_STRING ) && widget->parentWidget()->inherits( TQCOMBOBOX_OBJECT_NAME_STRING ) ) ||
	            widget->inherits( "KCompletionBox" ) ) ) {
		TQListBox* listbox = (TQListBox*) widget;
		listbox->setLineWidth( 4 );
		listbox->setBackgroundMode( NoBackground );
		widget->installEventFilter( this );

	} else if (widget->inherits("QToolBarExtensionWidget")) {
		widget->installEventFilter(this);
		//widget->setBackgroundMode( NoBackground );
 	}
	else if ( !qstrcmp( widget->name(), kdeToolbarWidget ) ) {
		widget->setBackgroundMode( NoBackground );
		widget->installEventFilter(this);
	}

	if (animateProgressBar && ::tqqt_cast<TQProgressBar*>(widget))
	{
		widget->installEventFilter(this);
		progAnimWidgets[static_cast<TQProgressBar*>(widget)] = 0;
		connect(widget, TQT_SIGNAL(destroyed(TQObject*)), this, TQT_SLOT(progressBarDestroyed(TQObject*)));
		if (!animationTimer->isActive())
			animationTimer->start( 50, false );
	}

	KStyle::polish(widget);
}

void KeramikStyle::unPolish(TQWidget* widget)
{
	//### TODO: This needs major cleanup (and so does polish() )
	if ( widget->inherits( TQPUSHBUTTON_OBJECT_NAME_STRING ) || widget->inherits( TQCOMBOBOX_OBJECT_NAME_STRING  ) )
	{
		if ( widget->inherits( TQCOMBOBOX_OBJECT_NAME_STRING ) )
			widget->setBackgroundMode( PaletteButton );
		widget->removeEventFilter(this);
	}
	else if ( widget->inherits( TQMENUBAR_OBJECT_NAME_STRING ) || widget->inherits( TQPOPUPMENU_OBJECT_NAME_STRING ) )
		widget->setBackgroundMode( PaletteBackground );

 	else if ( widget->parentWidget() &&
			  ( ( widget->inherits( TQLISTBOX_OBJECT_NAME_STRING ) && widget->parentWidget()->inherits( TQCOMBOBOX_OBJECT_NAME_STRING ) ) ||
	            widget->inherits( "KCompletionBox" ) ) ) {
		TQListBox* listbox = (TQListBox*) widget;
		listbox->setLineWidth( 1 );
		listbox->setBackgroundMode( PaletteBackground );
		widget->removeEventFilter( this );
		widget->clearMask();
	} else if (widget->inherits("QToolBarExtensionWidget")) {
		widget->removeEventFilter(this);
	}
	else if ( !qstrcmp( widget->name(), kdeToolbarWidget ) ) {
		widget->setBackgroundMode( PaletteBackground );
		widget->removeEventFilter(this);
	}
	else if ( ::tqqt_cast<TQProgressBar*>(widget) )
	{
		progAnimWidgets.remove(static_cast<TQProgressBar*>(widget));
	}

	KStyle::unPolish(widget);
}

void KeramikStyle::progressBarDestroyed(TQObject* obj)
{
	progAnimWidgets.remove(static_cast<TQProgressBar*>(TQT_TQWIDGET(obj)));
}


void KeramikStyle::polish( TQPalette& )
{
	loader.clear();
}

/** 
 Draws gradient background for toolbar buttons, handles and spacers
*/
static void renderToolbarEntryBackground(TQPainter* paint,
		 const TQToolBar* parent, TQRect r, const TQColorGroup& cg, bool horiz)
{
	int  toolWidth, toolHeight;
	
	//Do we have a parent toolbar to use?
	if (parent)
	{
		//Calculate the toolbar geometry.
		//The initial guess is the size of the parent widget
		toolWidth  = parent->width();
		toolHeight = parent->height();
					
		//If we're floating, however, wee need to fiddle
		//with height to skip the titlebar
		if (parent->place() == TQDockWindow::OutsideDock)
		{
			toolHeight = toolHeight - titleBarH - 2*parent->frameWidth() + 2;
			//2 at the end = the 2 pixels of border of a "regular"
			//toolbar we normally paint over.
		}
	}
	else
	{
		//No info, make a guess.
		//We take the advantage of the fact that the non-major
		//sizing direction parameter is ignored
		toolWidth  = r.width () + 2;
		toolHeight = r.height() + 2;
	}

	//Calculate where inside the toolbar we're
	int xoff = 0, yoff = 0;
	if (horiz)
		yoff = (toolHeight - r.height())/2;
	else
		xoff = (toolWidth - r.width())/2;
						
	Keramik::GradientPainter::renderGradient( paint, r, cg.button(),
		horiz, false /*Not a menubar*/,
		xoff, yoff,
		toolWidth, toolHeight);
}

static void renderToolbarWidgetBackground(TQPainter* painter, const TQWidget* widget)
{
	// Draw a gradient background for custom widgets in the toolbar
	// that have specified a "kde toolbar widget" name, or
	// are caught as toolbar widgets otherwise

	// Find the top-level toolbar of this widget, since it may be nested in other
	// widgets that are on the toolbar.
	TQWidget *parent = TQT_TQWIDGET(widget->parentWidget());
	int x_offset = widget->x(), y_offset = widget->y();
	while (parent && parent->parent() && !qstrcmp( parent->name(), kdeToolbarWidget ) )
	{
		x_offset += parent->x();
		y_offset += parent->y();
		parent = TQT_TQWIDGET(parent->parent());
	}

	TQRect pr = parent->rect();
	bool horiz_grad = pr.width() > pr.height();

	int  toolHeight = parent->height();
	int  toolWidth  = parent->width ();

	// Check if the parent is a QToolbar, and use its orientation, else guess.
	//Also, get the height to use in the gradient from it.
	TQToolBar* tb = dynamic_cast<TQToolBar*>(parent);
	if (tb)
	{
		horiz_grad = tb->orientation() == Qt::Horizontal;

		//If floating, we need to skip the titlebar.
		if (tb->place() == TQDockWindow::OutsideDock)
		{
			toolHeight = tb->height() - titleBarH - 2*tb->frameWidth() + 2;
			//Calculate offset here by looking at the bottom edge difference, and height.
			//First, calculate by how much the widget needs to be extended to touch
			//the bottom edge, minus the frame (except we use the current y_offset
			// to map to parent coordinates)
			int needToTouchBottom = tb->height() - tb->frameWidth() -
									(widget->rect().bottom() + y_offset);

			//Now, with that, we can see which portion to skip in the full-height
			//gradient -- which is the stuff other than the extended
			//widget
			y_offset = toolHeight - (widget->height() + needToTouchBottom) - 1;
		}
	}

	if (painter)
	{
		Keramik::GradientPainter::renderGradient( painter, widget->rect(),
			 widget->tqcolorGroup().button(), horiz_grad, false,
			 x_offset, y_offset, toolWidth, toolHeight);
	}
	else
	{
		TQPainter p( widget );
		Keramik::GradientPainter::renderGradient( &p, widget->rect(),
			 widget->tqcolorGroup().button(), horiz_grad, false,
			 x_offset, y_offset, toolWidth, toolHeight);
	}
}

// This function draws primitive elements as well as their masks.
void KeramikStyle::tqdrawPrimitive( TQ_PrimitiveElement pe,
									TQPainter *p,
									const TQRect &r,
									const TQColorGroup &cg,
									SFlags flags,
									const TQStyleOption& opt ) const
{
	bool down = flags & Style_Down;
	bool on   = flags & Style_On;
	bool disabled = ( flags & Style_Enabled ) == 0;
	int  x, y, w, h;
	r.rect(&x, &y, &w, &h);

	int x2 = x+w-1;
	int y2 = y+h-1;

	switch(pe)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault:
		{
			bool sunken = on || down;

			int id;
			if ( sunken ) id  = keramik_pushbutton_default_pressed;
				else id = keramik_pushbutton_default;

			if (flags & Style_MouseOver && id == keramik_pushbutton_default )
				id = keramik_pushbutton_default_hov;


			//p->fillRect( r, cg.background() );
			Keramik::RectTilePainter( id, false ).draw(p, r, cg.button(), cg.background(), disabled,  pmode() );
			break;
		}

		case PE_ButtonDropDown:
		case PE_ButtonTool:
		{
			if (titleBarMode)
			{
				TQRect nr;
				if (titleBarMode == Maximized)
				{
					//### What should we draw at sides?
					nr = TQRect(r.x(), r.y(), r.width()-1, r.height() );
				}
				else
				{
					int offX = (r.width() - 15)/2;
					int offY = (r.height() - 15)/2;

					if (flags & Style_Down)
						offY += 1;

					nr = TQRect(r.x()+offX, r.y()+offY, 15, 15);
				}

				Keramik::ScaledPainter(flags & Style_Down ? keramik_titlebutton_pressed : keramik_titlebutton,
										Keramik::ScaledPainter::Both).draw( p, nr, cg.button(), cg.background());
				return;
			}

			if (on)
			{
				Keramik::RectTilePainter(keramik_toolbar_clk).draw(p, r, cg.button(), cg.background());
				p->setPen(cg.dark());
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
			}
			else if (down)
			{
				Keramik::RectTilePainter(keramik_toolbar_clk).draw(p, r, cg.button(), cg.background());
			}
			else {
				if (flags & Style_MouseOver)
				{
				Keramik::GradientPainter::renderGradient( p,
					TQRect(r.x(), 0, r.width(), r.height()),
					Keramik::ColorUtil::lighten(cg.button(), 115), flags & Style_Horizontal, false );
				}
				else
					Keramik::GradientPainter::renderGradient( p,
						TQRect(r.x(), 0, r.width(), r.height()),
						cg.button(), flags & Style_Horizontal, false );

				p->setPen(cg.button().light(70));
				p->drawLine(x, y, x2-1, y);
				p->drawLine(x, y, x, y2-1);
				p->drawLine(x+2, y2-1, x2-1, y2-1);
				p->drawLine(x2-1, y+2, x2-1, y2-2);

				p->setPen(Keramik::ColorUtil::lighten(cg.button(), 115) );
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+1, x+1, y2);
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
			}

			break;
		}

		// PUSH BUTTON
		// -------------------------------------------------------------------
		case PE_ButtonCommand:
		{
			bool sunken = on || down;

			int  name;

			if ( w <= smallButMaxW || h <= smallButMaxH || forceSmallMode)
			{
				if (sunken)
					name = keramik_pushbutton_small_pressed;
				else
					name =  keramik_pushbutton_small;
				forceSmallMode = false;
			}
			else
			{
				if (sunken)
					name = keramik_pushbutton_pressed;
				else
					name =  keramik_pushbutton;
			}

			if (flags & Style_MouseOver && name == keramik_pushbutton )
				name = keramik_pushbutton_hov;

			if (toolbarBlendWidget && !flatMode )
			{
				//Render the toolbar gradient.
				renderToolbarWidgetBackground(p, toolbarBlendWidget);

				//Draw and blend the actual bevel..
				Keramik::RectTilePainter( name, false ).draw(p, r, cg.button(), cg.background(),
					disabled, Keramik::TilePainter::PaintFullBlend  );
			}
			else if (!flatMode)
				Keramik::RectTilePainter( name, false ).draw(p, r, cg.button(), cg.background(), disabled, pmode() );
			else {
				Keramik::ScaledPainter( name + KeramikTileCC, Keramik::ScaledPainter::Vertical).draw(
					p, r, cg.button(), cg.background(), disabled, pmode() );

				p->setPen(cg.button().light(75));

				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
				flatMode = false;
			}

			break;

		}

		// BEVELS
		// -------------------------------------------------------------------
		case PE_ButtonBevel:
		{
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			bool sunken = on || down;
			int x2 = x+w-1;
			int y2 = y+h-1;

			// Outer frame
			p->setPen(cg.shadow());
			p->drawRect(r);

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);

			if (w > 4 && h > 4)
			{
				if (sunken)
					p->fillRect(x+2, y+2, w-4, h-4, cg.button());
				else
					Keramik::GradientPainter::renderGradient( p, TQRect(x+2, y+2, w-4, h-4),
							cg.button(), flags & Style_Horizontal );
			}
			break;
		}


			// FOCUS RECT
			// -------------------------------------------------------------------
		case PE_FocusRect:
			//Qt may pass the background color to use for the focus indicator
			//here. This particularly happens within the iconview.
			//If that happens, pass it on to drawWinFocusRect() so it can
			//honor it
			if ( opt.isDefault() )
				p->drawWinFocusRect( r );
			else
				p->drawWinFocusRect( r, opt.color() );
			break;

			// HEADER SECTION
			// -------------------------------------------------------------------
		case PE_HeaderSectionMenu:
		case PE_HeaderSection:
			if ( flags & Style_Down )
				Keramik::RectTilePainter( keramik_listview_pressed, false ).draw( p, r, cg.button(), cg.background() );
			else
				Keramik::RectTilePainter( keramik_listview, false ).draw( p, r, cg.button(), cg.background() );
			break;

		case PE_HeaderArrow:
			if ( flags & Style_Up )
				tqdrawPrimitive( PE_ArrowUp, p, r, cg, Style_Enabled );
			else tqdrawPrimitive( PE_ArrowDown, p, r, cg, Style_Enabled );
			break;


		// 	// SCROLLBAR
		// -------------------------------------------------------------------

		case PE_ScrollBarSlider:
		{
			bool horizontal = flags & Style_Horizontal;
			bool active = ( flags & Style_Active ) || ( flags & Style_Down );
			int name = KeramikSlider1;
			unsigned int count = 3;



			if ( horizontal )
			{
				if ( w > ( loader.size( keramik_scrollbar_hbar+KeramikSlider1 ).width() +
				           loader.size( keramik_scrollbar_hbar+KeramikSlider4 ).width() +
				           loader.size( keramik_scrollbar_hbar+KeramikSlider3 ).width() + 2 ) )
					count = 5;
			}
			else if ( h > ( loader.size( keramik_scrollbar_vbar+KeramikSlider1 ).height() +
			                loader.size( keramik_scrollbar_vbar+KeramikSlider4 ).height() +
			                loader.size( keramik_scrollbar_vbar+KeramikSlider3 ).height() + 2 ) )
					count = 5;

			TQColor col = cg.highlight();

			if (customScrollMode || !highlightScrollBar)
				col = cg.button();

			if (!active)
				Keramik::ScrollBarPainter( name, count, horizontal ).draw( p, r, col, cg.background(), false, pmode() );
			else
				Keramik::ScrollBarPainter( name, count, horizontal ).draw( p, r, Keramik::ColorUtil::lighten(col ,110),
																					cg.background(), false, pmode() );
			break;
		}

		case PE_ScrollBarAddLine:
		{
			bool down = flags & Style_Down;

			if ( flags & Style_Horizontal )
			{
				Keramik::CenteredPainter painter(  keramik_scrollbar_hbar_arrow2 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );

				p->setPen( cg.buttonText() );
				p->drawLine(r.x()+r.width()/2 - 1, r.y() + r.height()/2 - 3,
				            r.x()+r.width()/2 - 1, r.y() + r.height()/2 + 3);


				drawKeramikArrow(p, cg, TQRect(r.x(), r.y(), r.width()/2, r.height()), PE_ArrowLeft, down, !disabled);

				drawKeramikArrow(p, cg, TQRect(r.x()+r.width()/2, r.y(), r.width() - r.width()/2, r.height()),
									PE_ArrowRight, down, !disabled);
			}
			else
			{
				Keramik::CenteredPainter painter(  keramik_scrollbar_vbar_arrow2 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );

				p->setPen( cg.buttonText() );
				p->drawLine(r.x()+r.width()/2 - 4, r.y()+r.height()/2,
				            r.x()+r.width()/2 + 2, r.y()+r.height()/2);


				drawKeramikArrow(p, cg, TQRect(r.x(), r.y(), r.width(), r.height()/2), PE_ArrowUp, down, !disabled);

				drawKeramikArrow(p, cg, TQRect(r.x(), r.y()+r.height()/2, r.width(), r.height() - r.height()/2),
									PE_ArrowDown, down, !disabled);
				//drawKeramikArrow(p, cg, r, PE_ArrowUp, down, !disabled);
			}


			break;
		}

		case PE_ScrollBarSubLine:
		{
			bool down = flags & Style_Down;

			if ( flags & Style_Horizontal )
			{
				Keramik::CenteredPainter painter(keramik_scrollbar_hbar_arrow1 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );
				drawKeramikArrow(p, cg, r, PE_ArrowLeft, down, !disabled);

			}
			else
			{
				Keramik::CenteredPainter painter( keramik_scrollbar_vbar_arrow1 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );
				drawKeramikArrow(p, cg, r, PE_ArrowUp, down, !disabled);
			}
			break;
		}
		
		// CHECKBOX (indicator)
		// -------------------------------------------------------------------
		case PE_Indicator:
		case PE_IndicatorMask:

			if (flags & Style_On)
				Keramik::ScaledPainter( keramik_checkbox_on ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			else if (flags & Style_Off)
				Keramik::ScaledPainter( keramik_checkbox_off ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			else
				Keramik::ScaledPainter( keramik_checkbox_tri ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );

			break;

			// RADIOBUTTON (exclusive indicator)
			// -------------------------------------------------------------------
		case PE_ExclusiveIndicator:
		case PE_ExclusiveIndicatorMask:
		{

			Keramik::ScaledPainter( on ? keramik_radiobutton_on : keramik_radiobutton_off ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			break;
		}

			// line edit frame
		case PE_PanelLineEdit:
		{
			if ( opt.isDefault() || opt.lineWidth() == 1 )
			{
				//1-pixel frames can not be simply clipped wider frames, as that would produce too little contrast on the lower border
				p->setPen( cg.dark() );
				p->drawLine( x, y, x + w - 1, y );
				p->drawLine( x, y, x, y + h - 1 );
				
				p->setPen( cg.light().dark( 110 ) );
				p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );
				p->drawLine( x, y + h - 1, x + w - 1, y + h - 1);
			}
			else
			{
				p->setPen( cg.dark() );
				p->drawLine( x, y, x + w - 1, y );
				p->drawLine( x, y, x, y + h - 1 );
				p->setPen( cg.mid() );
				p->drawLine( x + 1, y + 1, x + w - 1, y + 1 );
				p->drawLine( x + 1, y + 1, x + 1, y + h - 1 );
				p->setPen( cg.light() );
				p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );
				p->drawLine( x, y + h - 1, x + w - 1, y + h - 1);
				p->setPen( cg.light().dark( 110 ) );
				p->drawLine( x + w - 2, y + 1, x + w - 2, y + h - 2 );
				p->drawLine( x + 1, y + h - 2, x + w - 2, y + h - 2);
			}
			break;
		}

			// SPLITTER/DOCKWINDOW HANDLES
			// -------------------------------------------------------------------
		case PE_DockWindowResizeHandle:
		case PE_Splitter:
		{
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.dark());
			p->drawRect( r );
			p->setPen(cg.background());
			p->drawPoint(x, y);
			p->drawPoint(x2, y);
			p->drawPoint(x, y2);
			p->drawPoint(x2, y2);
			p->setPen(cg.light());
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->setPen(cg.midlight());
			p->drawLine(x+2, y+2, x+2, y2-2);
			p->drawLine(x+2, y+2, x2-2, y+2);
			p->setPen(cg.mid());
			p->drawLine(x2-1, y+1, x2-1, y2-1);
			p->drawLine(x+1, y2-1, x2-1, y2-1);
			p->fillRect(x+3, y+3, w-5, h-5, cg.brush(TQColorGroup::Background));
			break;
		}


		//case PE_PanelPopup:
			//p->setPen (cg.light() );
			//p->setBrush( cg.background().light( 110 ) );
			//p->drawRect( r );

			//p->setPen( cg.shadow() );
			//p->drawRect( r.x()+1, r.y()+1, r.width()-2, r.height()-2);
			//p->fillRect( tqvisualRect( TQRect( x + 1, y + 1, 23, h - 2 ), r ), cg.background().dark( 105 ) );
			//break;

			// GENERAL PANELS
			// -------------------------------------------------------------------
		case PE_Panel:
		{
			if (kickerMode)
			{
				if (p->device() && p->device()->devType() == TQInternal::Widget &&
											 TQCString(static_cast<TQWidget*>(static_cast<QWidget*>(static_cast<QPaintDevice*>(p->tqdevice())))->className()) == "FittsLawFrame" )
				{
					int x2 = x + r.width() - 1;
					int y2 = y + r.height() - 1;
					p->setPen(cg.dark());
					p->drawLine(x+1,y2,x2-1,y2);
					p->drawLine(x2,y+1,x2,y2);

					p->setPen( cg.light() );
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y, x, y2);


					return;
				}
			}
			KStyle::tqdrawPrimitive(pe, p, r, cg, flags, opt);
			break;
		}
		case PE_WindowFrame:
		{
			bool sunken  = flags & Style_Sunken;
			int lw = opt.isDefault() ? tqpixelMetric(PM_DefaultFrameWidth)
				: opt.lineWidth();
			if (lw == 2)
			{
				TQPen oldPen = p->pen();
				int x,y,w,h;
				r.rect(&x, &y, &w, &h);
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(sunken ? cg.light() : cg.dark());
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
				p->setPen(sunken ? cg.mid() : cg.light());
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
				p->setPen(sunken ? cg.midlight() : cg.mid());
				p->drawLine(x+1, y2-1, x2-1, y2-1);
				p->drawLine(x2-1, y+1, x2-1, y2-1);
				p->setPen(sunken ? cg.dark() : cg.midlight());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+1, x+1, y2-1);
				p->setPen(oldPen);
			} else
				KStyle::tqdrawPrimitive(pe, p, r, cg, flags, opt);

			break;
		}


			// MENU / TOOLBAR PANEL
			// -------------------------------------------------------------------
		case PE_PanelMenuBar: 			// Menu
		{
			Keramik::GradientPainter::renderGradient( p, r, cg.button(), true, true);
			//Keramik::ScaledPainter( keramik_menubar , Keramik::ScaledPainter::Vertical).draw( p, r, cg.button(), cg.background() );

			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;
			int lw = opt.isDefault() ? tqpixelMetric(PM_DefaultFrameWidth)
				: opt.lineWidth();
			if (lw)
			{
				p->setPen(cg.mid());
				p->drawLine(x2, y, x2, y2);
			}

			break;
		}

		case PE_PanelDockWindow:		// Toolbar
		{
			bool horiz = r.width() > r.height();

			//Here, we just draw the border.
			int x  = r.x();
			int y  = r.y();
			int x2 = r.x() + r.width()  - 1;
			int y2 = r.y() + r.height() - 1;
			int lw = opt.isDefault() ? tqpixelMetric(PM_DefaultFrameWidth)
				: opt.lineWidth();

			if (lw)
			{
				//Gradient border colors.
				//(Same as in gradients.cpp)
				TQColor gradTop = Keramik::ColorUtil::lighten(cg.button(),110);
				TQColor gradBot = Keramik::ColorUtil::lighten(cg.button(),109);
				if (horiz)
				{
					//Top line
					p->setPen(gradTop);
					p->drawLine(x, y, x2, y);

					//Bottom line
					p->setPen(gradBot);
					p->drawLine(x, y2, x2, y2);

					//Left line
					Keramik::GradientPainter::renderGradient(
						p, TQRect(r.x(), r.y(), 1, r.height()),
						cg.button(), true);

					//Right end-line
					p->setPen(cg.mid());
					p->drawLine(x2, y, x2, y2);
				}
				else
				{
					//Left line
					p->setPen(gradTop);
					p->drawLine(x, y, x, y2);

					//Right line
					p->setPen(gradBot);
					p->drawLine(x2, y, x2, y2);

					//Top line
					Keramik::GradientPainter::renderGradient(
						p, TQRect(r.x(), r.y(), r.width(), 1),
						cg.button(), false);

					//Bottom end-line
					p->setPen(cg.mid());
					p->drawLine(x, y2, x2, y2);
				}
			}
			break;
		}

		// TOOLBAR SEPARATOR
		// -------------------------------------------------------------------
		case PE_DockWindowSeparator:
		{
			TQWidget*  paintWidget = dynamic_cast<TQWidget*>(p->device());
			TQToolBar* parent      = 0;
			if (paintWidget)
				parent = ::tqqt_cast<TQToolBar*>(paintWidget->parentWidget());

			renderToolbarEntryBackground(p, parent, r, cg, (flags & Style_Horizontal) );
			if ( !(flags & Style_Horizontal) )
			{
				p->setPen(cg.mid());
				p->drawLine(4, r.height()/2-1, r.width()-5, r.height()/2-1);
				p->setPen(cg.light());
				p->drawLine(4, r.height()/2, r.width()-5, r.height()/2);
			}
			else
			{
				p->setPen(cg.mid());
				p->drawLine(r.width()/2-1, 4, r.width()/2-1, r.height()-5);
				p->setPen(cg.light());
				p->drawLine(r.width()/2, 4, r.width()/2, r.height()-5);
			}
			break;
		}

		default:
		{
			// ARROWS
			// -------------------------------------------------------------------
			if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft)
			{
				TQPointArray a;

				switch(pe)
				{
					case PE_ArrowUp:
						a.setPoints(TQCOORDARRLEN(u_arrow), u_arrow);
						break;

					case PE_ArrowDown:
						a.setPoints(TQCOORDARRLEN(d_arrow), d_arrow);
						break;

					case PE_ArrowLeft:
						a.setPoints(TQCOORDARRLEN(l_arrow), l_arrow);
						break;

					default:
						a.setPoints(TQCOORDARRLEN(r_arrow), r_arrow);
				}

				p->save();
				if ( flags & Style_Down )
					p->translate( tqpixelMetric( PM_ButtonShiftHorizontal ),
							tqpixelMetric( PM_ButtonShiftVertical ) );

				if ( flags & Style_Enabled )
				{
					a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
					p->setPen( cg.buttonText() );
					p->drawLineSegments( a );
				}
				else
				{
					a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
					p->setPen( cg.light() );
					p->drawLineSegments( a );
					a.translate( -1, -1 );
					p->setPen( cg.mid() );
					p->drawLineSegments( a );
				}
				p->restore();

			}
			else
				KStyle::tqdrawPrimitive( pe, p, r, cg, flags, opt );
		}
	}
}

void KeramikStyle::drawKStylePrimitive( KStylePrimitive kpe,
										  TQPainter* p,
										  const TQWidget* widget,
										  const TQRect &r,
										  const TQColorGroup &cg,
										  SFlags flags,
										  const TQStyleOption &opt ) const
{
	bool disabled = ( flags & Style_Enabled ) == 0;
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );

	switch ( kpe )
	{
		// SLIDER GROOVE
		// -------------------------------------------------------------------
		case KPE_SliderGroove:
		{
			const TQSlider* slider = static_cast< const TQSlider* >( widget );
			bool horizontal = slider->orientation() == Qt::Horizontal;
			
			Keramik::TilePainter::PaintMode pmod = Keramik::TilePainter::PaintNormal;
			
			if (slider->erasePixmap() && !slider->erasePixmap()->isNull())
				pmod = Keramik::TilePainter::PaintFullBlend;

			if ( horizontal )
				Keramik::RectTilePainter( keramik_slider_hgroove, false ).draw(p, r, cg.button(), cg.background(), disabled, pmod);
			else
				Keramik::RectTilePainter( keramik_slider_vgroove, true, false ).draw( p, r, cg.button(), cg.background(), disabled, pmod);

			break;
		}

		// SLIDER HANDLE
		// -------------------------------------------------------------------
		case KPE_SliderHandle:
			{
				const TQSlider* slider = static_cast< const TQSlider* >( widget );
				bool horizontal = slider->orientation() == Qt::Horizontal;

				TQColor hl = cg.highlight();
				if (!disabled && flags & Style_Active)
					hl = Keramik::ColorUtil::lighten(cg.highlight(),110);

				if (horizontal)
					Keramik::ScaledPainter( keramik_slider ).draw( p, r, disabled ? cg.button() : hl,
						Qt::black,  disabled, Keramik::TilePainter::PaintFullBlend );
				else
					Keramik::ScaledPainter( keramik_vslider ).draw( p, r, disabled ? cg.button() : hl,
						Qt::black,  disabled, Keramik::TilePainter::PaintFullBlend );
				break;
			}

		// TOOLBAR HANDLE
		// -------------------------------------------------------------------
		case KPE_ToolBarHandle: {
			int x = r.x(); int y = r.y();
			int x2 = r.x() + r.width()-1;
			int y2 = r.y() + r.height()-1;
			
			TQToolBar* parent = 0;
			
			if (widget && widget->parent() && widget->parent()->inherits(TQTOOLBAR_OBJECT_NAME_STRING))
				parent = static_cast<TQToolBar*>(TQT_TQWIDGET(widget->parent()));
				
			renderToolbarEntryBackground(p, parent, r, cg, (flags & Style_Horizontal));
			if (flags & Style_Horizontal) {
				p->setPen(cg.light());
				p->drawLine(x+1, y+4, x+1, y2-4);
				p->drawLine(x+3, y+4, x+3, y2-4);
				p->drawLine(x+5, y+4, x+5, y2-4);

				p->setPen(cg.mid());
				p->drawLine(x+2, y+4, x+2, y2-4);
				p->drawLine(x+4, y+4, x+4, y2-4);
				p->drawLine(x+6, y+4, x+6, y2-4);
			} else {
				p->setPen(cg.light());
				p->drawLine(x+4, y+1, x2-4, y+1);
				p->drawLine(x+4, y+3, x2-4, y+3);
				p->drawLine(x+4, y+5, x2-4, y+5);

				p->setPen(cg.mid());
				p->drawLine(x+4, y+2, x2-4, y+2);
				p->drawLine(x+4, y+4, x2-4, y+4);
				p->drawLine(x+4, y+6, x2-4, y+6);

			}
			break;
		}


		// GENERAL/KICKER HANDLE
		// -------------------------------------------------------------------
		case KPE_GeneralHandle: {
			int x = r.x(); int y = r.y();
			int x2 = r.x() + r.width()-1;
			int y2 = r.y() + r.height()-1;

			if (flags & Style_Horizontal) {

				p->setPen(cg.light());
				p->drawLine(x+1, y, x+1, y2);
				p->drawLine(x+3, y, x+3, y2);
				p->drawLine(x+5, y, x+5, y2);

				p->setPen(cg.mid());
				p->drawLine(x+2, y, x+2, y2);
				p->drawLine(x+4, y, x+4, y2);
				p->drawLine(x+6, y, x+6, y2);

			} else {

				p->setPen(cg.light());
				p->drawLine(x, y+1, x2, y+1);
				p->drawLine(x, y+3, x2, y+3);
				p->drawLine(x, y+5, x2, y+5);

				p->setPen(cg.mid());
				p->drawLine(x, y+2, x2, y+2);
				p->drawLine(x, y+4, x2, y+4);
				p->drawLine(x, y+6, x2, y+6);

			}
			break;
		}


		default:
			KStyle::drawKStylePrimitive( kpe, p, widget, r, cg, flags, opt);
	}
}

bool KeramikStyle::isFormWidget(const TQWidget* widget) const
{
	//Form widgets are in the KHTMLView, but that has 2 further inner levels
	//of widgets - QClipperWidget, and outside of that, QViewportWidget
	TQWidget* potentialClipPort = widget->parentWidget();
	if ( !potentialClipPort || potentialClipPort->isTopLevel() )
	return false;

	TQWidget* potentialViewPort = potentialClipPort->parentWidget();
	if (!potentialViewPort || potentialViewPort->isTopLevel() ||
			qstrcmp(potentialViewPort->name(), "qt_viewport") )
		return false;

	TQWidget* potentialKHTML  = potentialViewPort->parentWidget();
	if (!potentialKHTML || potentialKHTML->isTopLevel() ||
			qstrcmp(potentialKHTML->className(), "KHTMLView") )
		return false;


	return true;
}

void KeramikStyle::tqdrawControl( TQ_ControlElement element,
								  TQPainter *p,
								  const TQWidget *widget,
								  const TQRect &r,
								  const TQColorGroup &cg,
								  SFlags flags,
								  const TQStyleOption& opt ) const
{
	bool disabled = ( flags & Style_Enabled ) == 0;
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );

	switch (element)
	{
		// PUSHBUTTON
		// -------------------------------------------------------------------
		case CE_PushButton:
		{
			const TQPushButton* btn = static_cast< const TQPushButton* >( widget );

			if (isFormWidget(btn))
				formMode = true;

			if ( widget == hoverWidget )
				flags |= Style_MouseOver;

			if ( btn->isFlat( ) )
				flatMode = true;

			if ( btn->isDefault( ) && !flatMode )
			{
				tqdrawPrimitive( PE_ButtonDefault, p, r, cg, flags );
			}
			else
			{
				if (widget->parent() && widget->parent()->inherits(TQTOOLBAR_OBJECT_NAME_STRING))
					toolbarBlendWidget = widget;

				tqdrawPrimitive( PE_ButtonCommand, p, r, cg, flags );
				toolbarBlendWidget = 0;
			}

			formMode = false;
			break;
		}


		// PUSHBUTTON LABEL
		// -------------------------------------------------------------------
		case CE_PushButtonLabel:
		{
			const TQPushButton* button = static_cast<const TQPushButton *>( widget );
			bool active = button->isOn() || button->isDown();
			bool cornArrow = false;

			// Shift button contents if pushed.
			if ( active )
			{
				x += tqpixelMetric(PM_ButtonShiftHorizontal, widget);
				y += tqpixelMetric(PM_ButtonShiftVertical, widget);
				flags |= Style_Sunken;
			}

			// Does the button have a popup menu?
			if ( button->isMenuButton() )
			{
				int dx = tqpixelMetric( PM_MenuButtonIndicator, widget );
				if ( button->iconSet() && !button->iconSet()->isNull()  &&
					(dx + button->iconSet()->pixmap (TQIconSet::Small, TQIconSet::Normal, TQIconSet::Off ).width()) >= w )
				{
					cornArrow = true; //To little room. Draw the arrow in the corner, don't adjust the widget
				}
				else
				{
					tqdrawPrimitive( PE_ArrowDown, p, tqvisualRect( TQRect(x + w - dx - 8, y + 2, dx, h - 4), r ),
							   cg, flags, opt );
					w -= dx;
				}
			}

			// Draw the icon if there is one
			if ( button->iconSet() && !button->iconSet()->isNull() )
			{
				TQIconSet::Mode  mode  = TQIconSet::Disabled;
				TQIconSet::State state = TQIconSet::Off;

				if (button->isEnabled())
					mode = button->hasFocus() ? TQIconSet::Active : TQIconSet::Normal;
				if (button->isToggleButton() && button->isOn())
					state = TQIconSet::On;

				TQPixmap icon = button->iconSet()->pixmap( TQIconSet::Small, mode, state );

				if (!button->text().isEmpty())
				{
					const int TextToIconMargin = 6;
					//Center text + icon w/margin in between..
					
					//Calculate length of both.
					int length = icon.width() + TextToIconMargin
					              + p->fontMetrics().size(ShowPrefix, button->text()).width();
					
					//Calculate offset.
					int offset = (w - length)/2;
					
					//draw icon
					p->drawPixmap( x + offset, y + h / 2 - icon.height() / 2, icon );
					
					//new bounding rect for the text
					x += offset + icon.width() + TextToIconMargin;
					w =  length - icon.width() - TextToIconMargin;
				}
				else
				{
					//Icon only. Center it. 
					if (!button->pixmap())
						p->drawPixmap( x + w/2 - icon.width()/2, y + h / 2 - icon.height() / 2,
										icon );
					else  //icon + pixmap. Ugh. 
						p->drawPixmap( x + button->isDefault() ? 8 : 4 , y + h / 2 - icon.height() / 2, icon );
				}

				if (cornArrow) //Draw over the icon
					tqdrawPrimitive( PE_ArrowDown, p, tqvisualRect( TQRect(x + w - 6, x + h - 6, 7, 7), r ),
							   cg, flags, opt );
			}

			// Make the label indicate if the button is a default button or not
			drawItem( p, TQRect(x, y, w, h), AlignCenter | ShowPrefix, button->tqcolorGroup(),
						button->isEnabled(), button->pixmap(), button->text(), -1,
						&button->tqcolorGroup().buttonText() );

			if ( flags & Style_HasFocus )
				tqdrawPrimitive( PE_FocusRect, p,
				               tqvisualRect( subRect( SR_PushButtonFocusRect, widget ), widget ),
				               cg, flags );
			break;
		}

		case CE_ToolButtonLabel:
		{
		    //const TQToolButton *toolbutton = static_cast<const TQToolButton * >(widget);
			bool onToolbar = widget->parentWidget() && widget->parentWidget()->inherits( TQTOOLBAR_OBJECT_NAME_STRING );
			TQRect nr = r;

			if (!onToolbar)
			{
				if (widget->parentWidget() &&
				 !qstrcmp(widget->parentWidget()->name(),"qt_maxcontrols" ) )
				{
					//Make sure we don't cut into the endline
					if (!qstrcmp(widget->name(), "close"))
					{
						nr.addCoords(0,0,-1,0);
						p->setPen(cg.dark());
						p->drawLine(r.x() + r.width() - 1, r.y(),
								r.x() + r.width() - 1, r.y() + r.height() - 1 );
					}
				}
				//else if ( w > smallButMaxW && h > smallButMaxH )
				//	nr.setWidth(r.width()-2); //Account for shadow
			}

			KStyle::tqdrawControl(element, p, widget, nr, cg, flags, opt);
			break;
		}

		case CE_TabBarTab:
		{
			const TQTabBar* tabBar = static_cast< const TQTabBar* >( widget );

			bool bottom = tabBar->tqshape() == TQTabBar::RoundedBelow ||
			              tabBar->tqshape() == TQTabBar::TriangularBelow;

			if ( flags & Style_Selected )
			{
				TQRect tabRect = r;
				//If not the right-most tab, readjust the painting to be one pixel wider
				//to avoid a doubled line
				int rightMost = TQApplication::reverseLayout() ? 0 : tabBar->count() - 1;

				if (tabBar->indexOf( opt.tab()->identifier() ) != rightMost)
					tabRect.setWidth( tabRect.width() + 1);
				Keramik::ActiveTabPainter( bottom ).draw( p, tabRect, cg.button().light(110), cg.background(), !tabBar->isEnabled(), pmode());
			}
			else
			{
				Keramik::InactiveTabPainter::Mode mode;
				int index = tabBar->indexOf( opt.tab()->identifier() );
				if ( index == 0 ) mode = Keramik::InactiveTabPainter::First;
				else if ( index == tabBar->count() - 1 ) mode = Keramik::InactiveTabPainter::Last;
				else mode = Keramik::InactiveTabPainter::Middle;

				if ( bottom )
				{
					Keramik::InactiveTabPainter( mode, bottom ).draw( p, x, y, w, h - 3, cg.button(), cg.background(), disabled, pmode() );
					p->setPen( cg.dark() );
					p->drawLine( x, y, x + w, y );
				}
				else
				{
					Keramik::InactiveTabPainter( mode, bottom ).draw( p, x, y + 3, w, h - 3, cg.button(), cg.background(), disabled, pmode() );
					p->setPen( cg.light() );
					p->drawLine( x, y + h - 1, x + w, y + h - 1 );
				}
			}

			break;
		}

		case CE_DockWindowEmptyArea:
		{
			TQRect pr = r;
			if (widget && widget->inherits(TQTOOLBAR_OBJECT_NAME_STRING))
			{
				const TQToolBar* tb = static_cast<const TQToolBar*>(widget);
				if (tb->place() == TQDockWindow::OutsideDock)
				{
					//Readjust the paint rectangle to skip the titlebar
					pr = TQRect(r.x(), titleBarH + tb->frameWidth(),
						r.width(), tb->height() - titleBarH - 2 * tb->frameWidth() + 2);
					//2 at the end = the 2 pixels of border of a "regular"
					//toolbar we normally paint over.
				}
				Keramik::GradientPainter::renderGradient( p, pr, cg.button(),
										 tb->orientation() == Qt::Horizontal);
			}
			else
				KStyle::tqdrawControl( (TQ_ControlElement)CE_DockWindowEmptyArea, p, 
					widget, r, cg, flags, opt );
			break;
		}
		case CE_MenuBarEmptyArea:
		{
			Keramik::GradientPainter::renderGradient( p, r, cg.button(), true, true);
			break;
		}
		// MENUBAR ITEM (sunken panel on mouse over)
		// -------------------------------------------------------------------
		case CE_MenuBarItem:
		{
			TQMenuBar  *mb = (TQMenuBar*)widget;
			TQMenuItem *mi = opt.menuItem();
			TQRect      pr = mb->rect();

			bool active  = flags & Style_Active;
			bool focused = flags & Style_HasFocus;

			if ( active && focused )
				qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
								cg, true, 1, &cg.brush(TQColorGroup::Midlight));
			else
				Keramik::GradientPainter::renderGradient( p, pr, cg.button(), true, true);

			drawItem( p, r, AlignCenter | AlignVCenter | ShowPrefix
					| DontClip | SingleLine, cg, flags & Style_Enabled,
					mi->pixmap(), mi->text() );

			break;
		}


		// POPUPMENU ITEM
		// -------------------------------------------------------------------
		case CE_PopupMenuItem: {
			const TQPopupMenu *popupmenu = static_cast< const TQPopupMenu * >( widget );
			TQRect main = r;

			TQMenuItem *mi = opt.menuItem();

			if ( !mi )
			{
				// Don't leave blank holes if we set NoBackground for the TQPopupMenu.
				// This only happens when the popupMenu spans more than one column.
				if (! ( widget->erasePixmap() && !widget->erasePixmap()->isNull() ) )
					p->fillRect( r, cg.background().light( 105 ) );

				break;
			}
			int  tab        = opt.tabWidth();
			int  checkcol   = opt.maxIconWidth();
			bool enabled    = mi->isEnabled();
			bool checkable  = popupmenu->isCheckable();
			bool active     = flags & Style_Active;
			bool etchtext   = tqstyleHint( SH_EtchDisabledText );
			bool reverse    = TQApplication::reverseLayout();
			if ( checkable )
				checkcol = QMAX( checkcol, 20 );

			// Draw the menu item background
			if ( active )
			{
				if ( enabled )
					Keramik::RowPainter( keramik_menuitem ).draw( p, main, cg.highlight(), cg.background() );
				else {
					if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() )
						p->drawPixmap( main.topLeft(), *widget->erasePixmap(), main );
					else p->fillRect( main, cg.background().light( 105 ) );
					p->drawWinFocusRect( r );
				}
			}
			// Draw the transparency pixmap
			else if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() )
				p->drawPixmap( main.topLeft(), *widget->erasePixmap(), main );
			// Draw a solid background
			else
				p->fillRect( main, cg.background().light( 105 ) );
			// Are we a menu item separator?

			if ( mi->isSeparator() )
			{
				p->setPen( cg.mid() );
				p->drawLine( main.x()+5, main.y() + 1, main.right()-5, main.y() + 1 );
				p->setPen( cg.light() );
				p->drawLine( main.x()+5, main.y() + 2, main.right()-5, main.y() + 2 );
				break;
			}

			TQRect cr = tqvisualRect( TQRect( x + 2, y + 2, checkcol - 1, h - 4 ), r );
			// Do we have an icon?
			if ( mi->iconSet() )
			{
				TQIconSet::Mode mode;



				// Select the correct icon from the iconset
				if ( active )
					mode = enabled ? TQIconSet::Active : TQIconSet::Disabled;
				else
					mode = enabled ? TQIconSet::Normal : TQIconSet::Disabled;

				// Do we have an icon and are checked at the same time?
				// Then draw a "pressed" background behind the icon
				if ( checkable && /*!active &&*/ mi->isChecked() )
					qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(),
									 cg, true, 1, &cg.brush(TQColorGroup::Midlight) );
				// Draw the icon
				TQPixmap pixmap = mi->iconSet()->pixmap( TQIconSet::Small, mode );
				TQRect pmr( 0, 0, pixmap.width(), pixmap.height() );
				pmr.moveCenter( cr.center() );
				p->drawPixmap( pmr.topLeft(), pixmap );
			}

			// Are we checked? (This time without an icon)
			else if ( checkable && mi->isChecked() )
			{
				// We only have to draw the background if the menu item is inactive -
				// if it's active the "pressed" background is already drawn
			//	if ( ! active )
					qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(), cg, true, 1,
					                 &cg.brush(TQColorGroup::Midlight) );

				// Draw the checkmark
				SFlags cflags = Style_Default;
				cflags |= active ? Style_Enabled : Style_On;

				tqdrawPrimitive( PE_CheckMark, p, cr, cg, cflags );
			}

			// Time to draw the menu item label...
			int xm = itemFrame + checkcol + itemHMargin; // X position margin

			int xp = reverse ? // X position
					x + tab + rightBorder + itemHMargin + itemFrame - 1 :
					x + xm;

			int offset = reverse ? -1 : 1;	// Shadow offset for etched text

			// Label width (minus the width of the accelerator portion)
			int tw = w - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1;

			// Set the color for enabled and disabled text
			// (used for both active and inactive menu items)
			p->setPen( enabled ? cg.buttonText() : cg.mid() );

			// This color will be used instead of the above if the menu item
			// is active and disabled at the same time. (etched text)
			TQColor discol = cg.mid();

			// Does the menu item draw it's own label?
			if ( mi->custom() ) {
				int m = itemVMargin;
				// Save the painter state in case the custom
				// paint method changes it in some way
				p->save();

				// Draw etched text if we're inactive and the menu item is disabled
				if ( etchtext && !enabled && !active ) {
					p->setPen( cg.light() );
					mi->custom()->paint( p, cg, active, enabled, xp+offset, y+m+1, tw, h-2*m );
					p->setPen( discol );
				}
				mi->custom()->paint( p, cg, active, enabled, xp, y+m, tw, h-2*m );
				p->restore();
			}
			else {
				// The menu item doesn't draw it's own label
				TQString s = mi->text();
				// Does the menu item have a text label?
				if ( !s.isNull() ) {
					int t = s.find( '\t' );
					int m = itemVMargin;
					int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
					text_flags |= reverse ? AlignRight : AlignLeft;

					//TQColor draw = cg.text();
					TQColor draw = (active && enabled) ? cg.highlightedText () : cg.text();
					p->setPen(draw);


					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						int tabx = reverse ? x + rightBorder + itemHMargin + itemFrame :
							x + w - tab - rightBorder - itemHMargin - itemFrame;

						// Draw the right part of the label (accelerator text)
						if ( etchtext && !enabled ) {
							// Draw etched text if we're inactive and the menu item is disabled
							p->setPen( cg.light() );
							p->drawText( tabx+offset, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ) );
							p->setPen( discol );
						}
						p->drawText( tabx, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
						s = s.left( t );
					}

					// Draw the left part of the label (or the whole label
					// if there's no accelerator)
					if ( etchtext && !enabled ) {
						// Etched text again for inactive disabled menu items...
						p->setPen( cg.light() );
						p->drawText( xp+offset, y+m+1, tw, h-2*m, text_flags, s, t );
						p->setPen( discol );
					}


					p->drawText( xp, y+m, tw, h-2*m, text_flags, s, t );

					p->setPen(cg.text());

				}

				// The menu item doesn't have a text label
				// Check if it has a pixmap instead
				else if ( mi->pixmap() ) {
					TQPixmap *pixmap = mi->pixmap();

					// Draw the pixmap
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( Qt::OpaqueMode );

					int diffw = ( ( w - pixmap->width() ) / 2 )
									+ ( ( w - pixmap->width() ) % 2 );
					p->drawPixmap( x+diffw, y+itemFrame, *pixmap );

					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( Qt::TransparentMode );
				}
			}

			// Does the menu item have a submenu?
			if ( mi->popup() ) {
				TQ_PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
				int dim = tqpixelMetric(PM_MenuButtonIndicator) - itemFrame;
				TQRect vr = tqvisualRect( TQRect( x + w - arrowHMargin - itemFrame - dim,
							y + h / 2 - dim / 2, dim, dim), r );

				// Draw an arrow at the far end of the menu item
				if ( active ) {
					if ( enabled )
						discol = cg.buttonText();

					TQColorGroup g2( discol, cg.highlight(), white, white,
									enabled ? white : discol, discol, white );

					tqdrawPrimitive( arrow, p, vr, g2, Style_Enabled );
				} else
					tqdrawPrimitive( arrow, p, vr, cg,
							enabled ? Style_Enabled : Style_Default );
			}
			break;
		}
		case CE_ProgressBarContents: {
			const TQProgressBar* pb = (const TQProgressBar*)widget;
			TQRect cr = subRect(SR_ProgressBarContents, widget);
			double progress = pb->progress();
			bool reverse = TQApplication::reverseLayout();
			int steps = pb->totalSteps();

			if (!cr.isValid())
				return;

			// Draw progress bar
			if (progress > 0 || steps == 0) {
				double pg = (steps == 0) ? 0.1 : progress / steps;
				int width = QMIN(cr.width(), (int)(pg * cr.width()));
				if (steps == 0)
					width = QMIN(width,20); //Don't cross squares

				if (steps == 0) { //Busy indicator

					if (width < 1) width = 1; //A busy indicator with width 0 is kind of useless

					int remWidth = cr.width() - width; //Never disappear completely
					if (remWidth <= 0) remWidth = 1; //Do something non-crashy when too small...

					int pstep =  int(progress) % ( 2 *  remWidth );

					if ( pstep > remWidth ) {
						//Bounce about.. We're remWidth + some delta, we want to be remWidth - delta...
						// - ( (remWidth + some delta) - 2* remWidth )  = - (some deleta - remWidth) = remWidth - some delta..
						pstep = - (pstep - 2 * remWidth );
					}

					//Store the progress rect.
					TQRect progressRect;
					if (reverse)
						progressRect = TQRect(cr.x() + cr.width() - width - pstep, cr.y(),
										width, cr.height());
					else
						progressRect = TQRect(cr.x() + pstep, cr.y(), width, cr.height());

					Keramik::RowPainter(keramik_progressbar).draw(p, progressRect,
									 cg.highlight(), cg.background() );
					return;
				}

				TQRect progressRect;

				if (reverse)
					progressRect = TQRect(cr.x()+(cr.width()-width), cr.y(), width, cr.height());
				else
					progressRect = TQRect(cr.x(), cr.y(), width, cr.height());

				//Apply the animation rectangle.
				//////////////////////////////////////
				if (animateProgressBar)
				{
					int progAnimShift = progAnimWidgets[const_cast<TQProgressBar*>(pb)];
					if (reverse)
					{
						//Here, we can't simply shift, as the painter code calculates everything based
						//on the left corner, so we need to draw the 2 portions ourselves.

						//Start off by checking the geometry of the end pixmap - it introduces a bit of an offset
						TQSize   endDim = loader.size(keramik_progressbar + 3); //3 = 3*1 + 0 = (1,0) = cl

						//We might not have anything to animate at all, though, if the ender is the only thing to paint
						if (endDim.width() < progressRect.width())
						{
							//OK, so we do have some stripes.
							// Render the endline now - the clip region below will protect it from getting overwriten
							TQPixmap endline = loader.scale(keramik_progressbar + 3, endDim.width(), progressRect.height(),
															cg.highlight(), cg.background());
							p->drawPixmap(progressRect.x(), progressRect.y(), endline);

							//Now, calculate where the stripes should be, and set a clip region to that
							progressRect.setLeft(progressRect.x() + endline.width());
							p->setClipRect(progressRect, TQPainter::CoordPainter);

							//Expand the paint region slightly to get the animation offset.
							progressRect.setLeft(progressRect.x() - progAnimShift);

							//Paint the stripes.
							TQPixmap stripe = loader.scale(keramik_progressbar + 4, 28, progressRect.height(),
														  cg.highlight(), cg.background());
														  //4 = 3*1 + 1 = (1,1) = cc

							p->drawTiledPixmap(progressRect,  stripe);
							//Exit out here to skip the regular paint path
							return;
						}
					}
					else
					{
						//Clip to the old rectangle
						p->setClipRect(progressRect, TQPainter::CoordPainter);
						//Expand the paint region
						progressRect.setLeft(progressRect.x() - 28 + progAnimShift);
					}
				}

				Keramik::ProgressBarPainter(keramik_progressbar, reverse).draw( p,
							progressRect , cg.highlight(), cg.background());
			}
			break;
		}


		default:
			KStyle::tqdrawControl(element, p, widget, r, cg, flags, opt);
	}
}

void KeramikStyle::tqdrawControlMask( TQ_ControlElement element,
								    TQPainter *p,
								    const TQWidget *widget,
								    const TQRect &r,
								    const TQStyleOption& opt ) const
{
	p->fillRect(r, color1);
	maskMode = true;
	tqdrawControl( element, p, widget, r, TQApplication::tqpalette().active(), TQStyle::Style_Default, opt);
	maskMode = false;
}

bool KeramikStyle::isSizeConstrainedCombo(const TQComboBox* combo) const
{
	if (combo->width() >= 80)
		return false;
	int suggestedWidth = combo->tqsizeHint().width();
	
	if (combo->width() - suggestedWidth < -5)
		return true;

	return false;
}

void KeramikStyle::tqdrawComplexControl( TQ_ComplexControl control,
                                         TQPainter *p,
                                         const TQWidget *widget,
                                         const TQRect &r,
                                         const TQColorGroup &cg,
                                         SFlags flags,
									     SCFlags controls,
									     SCFlags active,
                                         const TQStyleOption& opt ) const
{
	bool disabled = ( flags & Style_Enabled ) == 0;
	switch(control)
	{
		// COMBOBOX
		// -------------------------------------------------------------------
		case CC_ComboBox:
		{
			bool             toolbarMode = false;
			const TQComboBox* cb          = static_cast< const TQComboBox* >( widget );
			bool             compact     = isSizeConstrainedCombo(cb);

			if (isFormWidget(cb))
				formMode = true;

			TQPixmap * buf = 0;
			TQPainter* p2 = p;

			TQRect br = r;

			if (controls == SC_All)
			{
				//Double-buffer only when we are in the slower full-blend mode
				if (widget->parent() &&
						( widget->parent()->inherits(TQTOOLBAR_OBJECT_NAME_STRING)|| !qstrcmp(widget->parent()->name(), kdeToolbarWidget) ) )
				{
					buf = new TQPixmap( r.width(), r.height() );
					br.setX(0);
					br.setY(0);
					p2 = new TQPainter(buf);
					
					//Ensure that we have clipping on, and have a sane base.
					//If need be, Qt will shrink us to the paint region.
					p->setClipRect(r);
					toolbarMode = true;
				}
			}


			if ( br.width() >= 28 && br.height() > 20 && !compact )
				br.addCoords( 0, -2, 0, 0 );
				
			//When in compact mode, we force the shadow-less bevel mode,
			//but that also alters height and not just width.
			//readjust height to fake the other metrics (plus clear 
			//the other areas, as appropriate). The automasker
			//will take care of the overall tqshape.
			if ( compact )
			{
				forceSmallMode = true;
				br.setHeight( br.height() - 2);
				p->fillRect ( r.x(), r.y() + br.height(), r.width(), 2, cg.background());
			}
				
				
			if ( controls & SC_ComboBoxFrame )
			{
				if (toolbarMode)
					toolbarBlendWidget = widget;

				if ( widget == hoverWidget )
					flags |= Style_MouseOver;
					
				tqdrawPrimitive( PE_ButtonCommand, p2, br, cg, flags );

				toolbarBlendWidget = 0;
			}

			// don't draw the focus rect etc. on the mask
			if ( cg.button() == color1 && cg.background() == color0 )
				break;

			if ( controls & SC_ComboBoxArrow )
			{
				if ( active )
					flags |= Style_On;
					
				TQRect ar = querySubControlMetrics( CC_ComboBox, widget,
													 SC_ComboBoxArrow );
				if (!compact)
				{
					ar.setWidth(ar.width()-13);
					TQRect rr = tqvisualRect( TQRect( ar.x(), ar.y() + 4,
							loader.size(keramik_ripple ).width(), ar.height() - 8 ),
							widget );

					ar = tqvisualRect( TQRect( ar.x() + loader.size( keramik_ripple ).width() + 4, ar.y(), 
											11, ar.height() ), 
									widget );

					TQPointArray a;
	
					a.setPoints(TQCOORDARRLEN(keramik_combo_arrow), keramik_combo_arrow);
	
					a.translate( ar.x() + ar.width() / 2, ar.y() + ar.height() / 2 );
					p2->setPen( cg.buttonText() );
					p2->drawLineSegments( a );
	
					Keramik::ScaledPainter( keramik_ripple ).draw( p2, rr, cg.button(), Qt::black, disabled, Keramik::TilePainter::PaintFullBlend );
				}
				else //Size-constrained combo -- loose the ripple.
				{
					ar.setWidth(ar.width() - 7);
					ar = tqvisualRect( TQRect( ar.x(), ar.y(), 11, ar.height() ), widget);
					TQPointArray a;
	
					a.setPoints(TQCOORDARRLEN(keramik_combo_arrow), keramik_combo_arrow);
	
					a.translate( ar.x() + ar.width() / 2, ar.y() + ar.height() / 2 );
					p2->setPen( cg.buttonText() );
					p2->drawLineSegments( a );
				}
			}

			if ( controls & SC_ComboBoxEditField )
			{
				if ( cb->editable() )
				{
					TQRect er = tqvisualRect( querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxEditField ), widget );
					er.addCoords( -2, -2, 2, 2 );
					p2->fillRect( er, cg.base() );
					tqdrawPrimitive( PE_PanelLineEdit, p2, er, cg );
					Keramik::RectTilePainter( keramik_frame_shadow, false, false, 2, 2 ).draw( p2, er, cg.button(),
						Qt::black, false, pmodeFullBlend() );
				}
				else if ( cb->hasFocus() )
				{
					TQRect re = TQStyle::tqvisualRect(subRect(SR_ComboBoxFocusRect, cb), widget);
					if ( compact )
						re.addCoords( 3, 3, 0, -3 );
					p2->fillRect( re, cg.brush( TQColorGroup::Highlight ) );
					tqdrawPrimitive( PE_FocusRect, p2, re, cg,
					Style_FocusAtBorder, TQStyleOption( cg.highlight() ) );
				}
				// TQComboBox draws the text on its own and uses the painter's current colors
				if ( cb->hasFocus() )
				{
					p->setPen( cg.highlightedText() );
					p->setBackgroundColor( cg.highlight() );
				}
				else
				{
					p->setPen( cg.text() );
					p->setBackgroundColor( cg.button() );
				}
			}

			if (p2 != p)
			{
				p2->end();
				delete p2;
				p->drawPixmap(r.x(), r.y(), *buf);
				delete buf;
			}

			formMode = false;
			break;
		}

		case CC_SpinWidget:
		{
			const TQSpinWidget* sw = static_cast< const TQSpinWidget* >( widget );
			TQRect br = tqvisualRect( querySubControlMetrics( (TQ_ComplexControl)CC_SpinWidget, widget, SC_SpinWidgetButtonField ), widget );
			if ( controls & SC_SpinWidgetButtonField )
			{
				Keramik::SpinBoxPainter().draw( p, br, cg.button(), cg.background(), !sw->isEnabled() );
				if ( active & SC_SpinWidgetUp )
					Keramik::CenteredPainter( keramik_spinbox_pressed_arrow_up ).draw( p, br.x(), br.y() + 3, br.width(), br.height() / 2, cg.button(), cg.background() );
				else
					Keramik::CenteredPainter( keramik_spinbox_arrow_up ).draw( p, br.x(), br.y() + 3, br.width(), br.height() / 2, cg.button(), cg.background(), !sw->isUpEnabled() );
				if ( active & SC_SpinWidgetDown )
					Keramik::CenteredPainter( keramik_spinbox_pressed_arrow_down ).draw( p, br.x(), br.y() + br.height() / 2 , br.width(), br.height() / 2 - 8, cg.button(), cg.background()  );
				else
					Keramik::CenteredPainter( keramik_spinbox_arrow_down ).draw( p, br.x(), br.y() + br.height() / 2, br.width(), br.height() / 2 - 8, cg.background(), cg.button(), !sw->isDownEnabled() );
			}

			if ( controls & SC_SpinWidgetFrame )
				tqdrawPrimitive( PE_PanelLineEdit, p, r, cg );

			break;
		}
		case CC_ScrollBar:
		{
			const TQScrollBar* sb = static_cast< const TQScrollBar* >( widget );
			if (highlightScrollBar && sb->parentWidget()) //Don't do the check if not highlighting anyway
			{
				if (sb->parentWidget()->tqcolorGroup().button() != sb->tqcolorGroup().button())
					customScrollMode = true;
			}
			bool horizontal = sb->orientation() == Qt::Horizontal;
			TQRect slider, subpage, addpage, subline, addline;
			if ( sb->minValue() == sb->maxValue() ) flags &= ~Style_Enabled;

			slider = querySubControlMetrics( control, widget, SC_ScrollBarSlider, opt );
			subpage = querySubControlMetrics( control, widget, SC_ScrollBarSubPage, opt );
			addpage = querySubControlMetrics( control, widget, SC_ScrollBarAddPage, opt );
			subline = querySubControlMetrics( control, widget, SC_ScrollBarSubLine, opt );
			addline = querySubControlMetrics( control, widget, SC_ScrollBarAddLine, opt );

			if ( controls & SC_ScrollBarSubLine )
				tqdrawPrimitive( PE_ScrollBarSubLine, p, subline, cg,
				               flags | ( ( active & SC_ScrollBarSubLine ) ? Style_Down : 0 ) );

			TQRegion clip;
			if ( controls & SC_ScrollBarSubPage ) clip |= subpage;
			if ( controls & SC_ScrollBarAddPage ) clip |= addpage;
			if ( horizontal )
				clip |= TQRect( slider.x(), 0, slider.width(), sb->height() );
			else
				clip |= TQRect( 0, slider.y(), sb->width(), slider.height() );
			clip ^= slider;

			p->setClipRegion( clip );
			Keramik::ScrollBarPainter( KeramikGroove1, 2, horizontal ).draw( p, slider | subpage | addpage, cg.button(), cg.background(), disabled );

			if ( controls & SC_ScrollBarSlider )
			{
				if ( horizontal )
					p->setClipRect( slider.x(), slider.y(), addpage.right() - slider.x() + 1, slider.height() );
				else
					p->setClipRect( slider.x(), slider.y(), slider.width(), addpage.bottom() - slider.y() + 1 );
				tqdrawPrimitive( PE_ScrollBarSlider, p, slider, cg,
					flags | ( ( active == SC_ScrollBarSlider ) ? Style_Down : 0 )  );
			}
			p->setClipping( false );

			if ( controls & ( SC_ScrollBarSubLine | SC_ScrollBarAddLine ) )
			{
				tqdrawPrimitive( PE_ScrollBarAddLine, p, addline, cg, flags );
				if ( active & SC_ScrollBarSubLine )
				{
					if ( horizontal )
						p->setClipRect( TQRect( addline.x(), addline.y(), addline.width() / 2, addline.height() ) );
					else
						p->setClipRect( TQRect( addline.x(), addline.y(), addline.width(), addline.height() / 2 ) );
					tqdrawPrimitive( PE_ScrollBarAddLine, p, addline, cg, flags | Style_Down );
				}
				else if ( active & SC_ScrollBarAddLine )
				{
					if ( horizontal )
						p->setClipRect( TQRect( addline.x() + addline.width() / 2, addline.y(), addline.width() / 2, addline.height() ) );
					else
						p->setClipRect( TQRect( addline.x(), addline.y() + addline.height() / 2, addline.width(), addline.height() / 2 ) );
					tqdrawPrimitive( PE_ScrollBarAddLine, p, addline, cg, flags | Style_Down );
				}
			}

			customScrollMode = false;


			break;
		}

		// TOOLBUTTON
		// -------------------------------------------------------------------
		case CC_ToolButton: {
			const TQToolButton *toolbutton = (const TQToolButton *) widget;
			bool onToolbar = widget->parentWidget() && widget->parentWidget()->inherits( TQTOOLBAR_OBJECT_NAME_STRING );
			bool onExtender = !onToolbar &&
				widget->parentWidget() && widget->parentWidget()->inherits( "QToolBarExtensionWidget") &&
				widget->parentWidget()->parentWidget()->inherits( TQTOOLBAR_OBJECT_NAME_STRING );

			bool onControlButtons = false;
			if (!onToolbar && !onExtender && widget->parentWidget() &&
				 !qstrcmp(widget->parentWidget()->name(),"qt_maxcontrols" ) )
			{
				onControlButtons = true;
				titleBarMode = Maximized;
			}

			TQRect button, menuarea;
			button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
			menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

			SFlags bflags = flags,
				   mflags = flags;

			if (active & SC_ToolButton)
				bflags |= Style_Down;
			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;

			if ( widget == hoverWidget )
				bflags |= Style_MouseOver;


			if (onToolbar &&  static_cast<TQToolBar*>(TQT_TQWIDGET(widget->parent()))->orientation() == Qt::Horizontal)
				bflags |= Style_Horizontal;

			if (controls & SC_ToolButton)
			{
				// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised) || onControlButtons)
				{
					//Make sure the standalone toolbuttons have a gradient in the right direction
					if (!onToolbar && !onControlButtons)
						bflags |= Style_Horizontal;

					tqdrawPrimitive( PE_ButtonTool, p, button, cg,
					 bflags, opt);
				}

				// Check whether to draw a background pixmap
				else if ( toolbutton->parentWidget() &&
						  toolbutton->parentWidget()->backgroundPixmap() &&
						  !toolbutton->parentWidget()->backgroundPixmap()->isNull() )
				{
					TQPixmap pixmap = *(toolbutton->parentWidget()->backgroundPixmap());
					p->drawTiledPixmap( r, pixmap, toolbutton->pos() );
				}
				else if (onToolbar)
				{
					renderToolbarWidgetBackground(p, widget);
				}
				else if (onExtender)
				{
					// This assumes floating toolbars can't have extenders,
					//(so if we're on an extender, we're not floating)
					TQWidget*  parent  = static_cast<TQWidget*> (TQT_TQWIDGET(widget->parent()));
					TQToolBar* toolbar = static_cast<TQToolBar*>(TQT_TQWIDGET(parent->parent()));
					TQRect tr    = toolbar->rect();
					bool  horiz = toolbar->orientation() == Qt::Horizontal;

					//Calculate offset. We do this by translating our coordinates,
					//which are relative to the parent, to be relative to the toolbar.
					int xoff = 0, yoff = 0;
					if (horiz)
						yoff = parent->mapToParent(widget->pos()).y();
					else
						xoff = parent->mapToParent(widget->pos()).x();

					Keramik::GradientPainter::renderGradient( p, r, cg.button(),
							horiz, false, /*Not a menubar*/
							xoff,  yoff,
							tr.width(), tr.height());
				}
			}

			// Draw a toolbutton menu indicator if required
			if (controls & SC_ToolButtonMenu)
			{
				if (mflags & (Style_Down | Style_On | Style_Raised))
					tqdrawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
				tqdrawPrimitive(PE_ArrowDown, p, menuarea, cg, mflags, opt);
			}

			if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
				TQRect fr = toolbutton->rect();
				fr.addCoords(3, 3, -3, -3);
				tqdrawPrimitive(PE_FocusRect, p, fr, cg);
			}

			titleBarMode = None;

			break;
		}

		case CC_TitleBar:
			titleBarMode = Regular; //Handle buttons on titlebar different from toolbuttons
		default:
			KStyle::tqdrawComplexControl( control, p, widget,
						r, cg, flags, controls, active, opt );

			titleBarMode = None;
	}
}

void KeramikStyle::tqdrawComplexControlMask( TQ_ComplexControl control,
                                         TQPainter *p,
                                         const TQWidget *widget,
                                         const TQRect &r,
                                         const TQStyleOption& opt ) const
{
	if (control == CC_ComboBox)
	{
		maskMode = true;
		tqdrawComplexControl(CC_ComboBox, p, widget, r,
				TQApplication::tqpalette().active(), Style_Default,
				SC_ComboBoxFrame,SC_None, opt);
		maskMode = false;

	}
	else
		p->fillRect(r, color1);

}

int KeramikStyle::tqpixelMetric(PixelMetric m, const TQWidget *widget) const
{
	switch(m)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PM_ButtonMargin:				// Space btw. frame and label
			return 4;

		case PM_SliderLength:
			return 12;
		case PM_SliderControlThickness:
			return loader.size( keramik_slider ).height() - 4;
		case PM_SliderThickness:
			return loader.size( keramik_slider ).height();

		case PM_ButtonShiftHorizontal:
			return 0;
		case PM_ButtonShiftVertical: // Offset by 1
			return 1;


		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
			return loader.size( keramik_radiobutton_on ).width();
		case PM_ExclusiveIndicatorHeight:
			return loader.size( keramik_radiobutton_on ).height();
		case PM_IndicatorWidth:				// Checkbox size
			return loader.size( keramik_checkbox_on ).width();
		case PM_IndicatorHeight:
			return loader.size( keramik_checkbox_on) .height();

		case PM_ScrollBarExtent:
			return loader.size( keramik_scrollbar_vbar + KeramikGroove1).width();
		case PM_ScrollBarSliderMin:
			return loader.size( keramik_scrollbar_vbar + KeramikSlider1 ).height() +
                        loader.size( keramik_scrollbar_vbar + KeramikSlider3 ).height();

		case PM_SpinBoxFrameWidth:		
		case PM_DefaultFrameWidth:
			return 1;

		case PM_MenuButtonIndicator:
			return 13;

		case PM_TabBarTabVSpace:
			return 12;

		case PM_TabBarTabOverlap:
			return 0;
			
		case PM_TabBarTabShiftVertical:
		{
			const TQTabBar* tb = ::tqqt_cast<const TQTabBar*>(widget);
			if (tb)
			{
				if (tb->tqshape() == TQTabBar::RoundedBelow || 
					tb->tqshape() == TQTabBar::TriangularBelow)
					return 0;
			}
			
			return 2; //For top, or if not sure
		}
			

		case PM_TitleBarHeight:
			return titleBarH;

		default:
			return KStyle::tqpixelMetric(m, widget);
	}
}


TQSize KeramikStyle::tqsizeFromContents( ContentsType contents,
										const TQWidget* widget,
										const TQSize &contentSize,
										const TQStyleOption& opt ) const
{
	switch (contents)
	{
		// PUSHBUTTON SIZE
		// ------------------------------------------------------------------
		case CT_PushButton:
		{
			const TQPushButton* btn = static_cast< const TQPushButton* >( widget );

			int w = contentSize.width() + 2 * tqpixelMetric( PM_ButtonMargin, widget );
			int h = contentSize.height() + 2 * tqpixelMetric( PM_ButtonMargin, widget );
			if ( btn->text().isEmpty() && contentSize.width() < 32 ) return TQSize( w, h );


			//For some reason kcontrol no longer does this...
			//if ( btn->isDefault() || btn->autoDefault() )
			//            w = QMAX( w, 40 );

			return TQSize( w + 30, h + 5 ); //MX: No longer blank space -- can make a bit smaller
		}

		case CT_ToolButton:
		{
			bool onToolbar = widget->parentWidget() && widget->parentWidget()->inherits( TQTOOLBAR_OBJECT_NAME_STRING );
			if (!onToolbar) //Behaves like a button, so scale appropriately to the border
			{
				int w = contentSize.width();
				int h = contentSize.height();
				return TQSize( w + 12, h + 10 );
			}
			else
			{
				return KStyle::tqsizeFromContents( contents, widget, contentSize, opt );
			}
		}

		case CT_ComboBox: {
			int arrow = 11 + loader.size( keramik_ripple ).width();
			const TQComboBox *cb = static_cast<const TQComboBox*>( widget );
			return TQSize( contentSize.width() + arrow + (cb->editable() ? 26 : 22),
					contentSize.height() + 10 );
		}

		// POPUPMENU ITEM SIZE
		// -----------------------------------------------------------------
		case CT_PopupMenuItem: {
			if ( ! widget || opt.isDefault() )
				return contentSize;

			const TQPopupMenu *popup = (const TQPopupMenu *) widget;
			bool checkable = popup->isCheckable();
			TQMenuItem *mi = opt.menuItem();
			int maxpmw = opt.maxIconWidth();
			int w = contentSize.width(), h = contentSize.height();

			if ( mi->custom() ) {
				w = mi->custom()->tqsizeHint().width();
				h = mi->custom()->tqsizeHint().height();
				if ( ! mi->custom()->fullSpan() )
					h += 2*itemVMargin + 2*itemFrame;
			}
			else if ( mi->widget() ) {
			} else if ( mi->isSeparator() ) {
				w = 30; // Arbitrary
				h = 3;
			}
			else {
				if ( mi->pixmap() )
					h = QMAX( h, mi->pixmap()->height() + 2*itemFrame );
				else {
					// Ensure that the minimum height for text-only menu items
					// is the same as the icon size used by KDE.
					h = QMAX( h, 16 + 2*itemFrame );
					h = QMAX( h, popup->fontMetrics().height()
							+ 2*itemVMargin + 2*itemFrame );
				}

				if ( mi->iconSet() )
					h = QMAX( h, mi->iconSet()->pixmap(
								TQIconSet::Small, TQIconSet::Normal).height() +
								2 * itemFrame );
			}

			if ( ! mi->text().isNull() && mi->text().find('\t') >= 0 )
				w += itemHMargin + itemFrame*2 + 7;
			else if ( mi->popup() )
				w += 2 * arrowHMargin;

			if ( maxpmw )
				w += maxpmw + 6;
			if ( checkable && maxpmw < 20 )
				w += 20 - maxpmw;
			if ( checkable || maxpmw > 0 )
				w += 12;

			w += rightBorder;

			return TQSize( w, h );
		}
		
		default:
			return KStyle::tqsizeFromContents( contents, widget, contentSize, opt );
	}
}


TQStyle::SubControl KeramikStyle::querySubControl( TQ_ComplexControl control,
	                                              const TQWidget* widget,
                                                  const TQPoint& point,
                                                  const TQStyleOption& opt ) const
{
	SubControl result = KStyle::querySubControl( control, widget, point, opt );
	if ( control == CC_ScrollBar && result == SC_ScrollBarAddLine )
	{
		TQRect addline = querySubControlMetrics( control, widget, result, opt );
		if ( static_cast< const TQScrollBar* >( widget )->orientation() == Qt::Horizontal )
		{
			if ( point.x() < addline.center().x() ) result = SC_ScrollBarSubLine;
		}
		else if ( point.y() < addline.center().y() ) result = SC_ScrollBarSubLine;
	}
	return result;
}

TQRect KeramikStyle::querySubControlMetrics( TQ_ComplexControl control,
									const TQWidget* widget,
	                              SubControl subcontrol,
	                              const TQStyleOption& opt ) const
{
	switch ( control )
	{
		case CC_ComboBox:
		{
			int arrow;
			bool compact = false;
			if ( isSizeConstrainedCombo(static_cast<const TQComboBox*>(widget) ) ) //### constant
				compact = true;
				
			if ( compact )
				arrow = 11;
			else
				arrow = 11 + loader.size( keramik_ripple ).width();
				
			switch ( subcontrol )
			{
				case SC_ComboBoxArrow:
					if ( compact )
						return TQRect( widget->width() - arrow - 7, 0, arrow + 6, widget->height() );
					else
						return TQRect( widget->width() - arrow - 14, 0, arrow + 13, widget->height() );

				case SC_ComboBoxEditField:
				{
					if ( compact )
						return TQRect( 2, 4, widget->width() - arrow - 2 - 7, widget->height() - 8 );
					else if ( widget->width() < 36 || widget->height() < 22 )
						return TQRect( 4, 3, widget->width() - arrow - 20, widget->height() - 6 );
					else if ( static_cast< const TQComboBox* >( widget )->editable() )
						return TQRect( 8, 4, widget->width() - arrow - 26, widget->height() - 11 );
					else
						return TQRect( 6, 4, widget->width() - arrow - 22, widget->height() - 9 );
				}

				case SC_ComboBoxListBoxPopup:
				{
					//Note that the widget here == the combo, not the completion
					//box, so we don't get any recursion
					int suggestedWidth = widget->tqsizeHint().width(); 
					TQRect def = opt.rect();
					def.addCoords( 4, -4, -6, 4 );
					
					if ((def.width() - suggestedWidth < -12) && (def.width() < 80))
						def.setWidth(QMIN(80, suggestedWidth - 10));

					return def;
				}

				default: break;
			}
			break;
		}

		case CC_ScrollBar:
		{
			const TQScrollBar* sb = static_cast< const TQScrollBar* >( widget );
			bool horizontal = sb->orientation() == Qt::Horizontal;
			int addline, subline, sliderpos, sliderlen, maxlen, slidermin;
			if ( horizontal )
			{
				subline = loader.size( keramik_scrollbar_hbar_arrow1 ).width();
				addline = loader.size( keramik_scrollbar_hbar_arrow2 ).width();
				maxlen = sb->width() - subline - addline + 2;
			}
			else
			{
				subline = loader.size( keramik_scrollbar_vbar_arrow1 ).height();
				addline = loader.size( keramik_scrollbar_vbar_arrow2 ).height();
				maxlen = sb->height() - subline - addline + 2;
			}
			sliderpos = sb->sliderStart();
			if ( sb->minValue() != sb->maxValue() )
			{
				int range = sb->maxValue() - sb->minValue();
				sliderlen = ( sb->pageStep() * maxlen ) / ( range + sb->pageStep() );
				slidermin = tqpixelMetric( PM_ScrollBarSliderMin, sb );
				if ( sliderlen < slidermin ) sliderlen = slidermin;
				if ( sliderlen > maxlen ) sliderlen = maxlen;
			}
			else sliderlen = maxlen;

			switch ( subcontrol )
			{
				case SC_ScrollBarGroove:
					if ( horizontal ) return TQRect( subline, 0, maxlen, sb->height() );
					else return TQRect( 0, subline, sb->width(), maxlen );

				case SC_ScrollBarSlider:
					if (horizontal) return TQRect( sliderpos, 0, sliderlen, sb->height() );
					else return TQRect( 0, sliderpos, sb->width(), sliderlen );

				case SC_ScrollBarSubLine:
					if ( horizontal ) return TQRect( 0, 0, subline, sb->height() );
					else return TQRect( 0, 0, sb->width(), subline );

				case SC_ScrollBarAddLine:
					if ( horizontal ) return TQRect( sb->width() - addline, 0, addline, sb->height() );
					else return TQRect( 0, sb->height() - addline, sb->width(), addline );

				case SC_ScrollBarSubPage:
					if ( horizontal ) return TQRect( subline, 0, sliderpos - subline, sb->height() );
					else return TQRect( 0, subline, sb->width(), sliderpos - subline );

				case SC_ScrollBarAddPage:
					if ( horizontal ) return TQRect( sliderpos + sliderlen, 0, sb->width() - addline  - (sliderpos + sliderlen) , sb->height() );
					else return TQRect( 0, sliderpos + sliderlen, sb->width(), sb->height() - addline - (sliderpos + sliderlen)
                    /*maxlen - sliderpos - sliderlen + subline - 5*/ );

				default: break;
			};
			break;
		}
		case CC_Slider:
		{
			const TQSlider* sl = static_cast< const TQSlider* >( widget );
			bool horizontal = sl->orientation() == Qt::Horizontal;
			TQSlider::TickSetting ticks = sl->tickmarks();
			int pos = sl->sliderStart();
			int size = tqpixelMetric( PM_SliderControlThickness, widget );
			int handleSize = tqpixelMetric( PM_SliderThickness, widget );
			int len = tqpixelMetric( PM_SliderLength, widget );

			//Shrink the metrics if the widget is too small
			//to fit our normal values for them.
			if (horizontal)
				handleSize = QMIN(handleSize, sl->height());
			else
				handleSize = QMIN(handleSize, sl->width());

			size = QMIN(size, handleSize);

			switch ( subcontrol )
			{
				case SC_SliderGroove:
					if ( horizontal )
					{
						if ( ticks == TQSlider::Both )
							return TQRect( 0, ( sl->height() - size ) / 2, sl->width(), size );
						else if ( ticks == TQSlider::Above )
							return TQRect( 0, sl->height() - size - ( handleSize - size ) / 2, sl->width(), size );
						return TQRect( 0, ( handleSize - size ) / 2, sl->width(), size );
					}
					else
					{
						if ( ticks == TQSlider::Both )
							return TQRect( ( sl->width() - size ) / 2, 0, size, sl->height() );
						else if ( ticks == TQSlider::Above )
							return TQRect( sl->width() - size - ( handleSize - size ) / 2, 0, size, sl->height() );
						return TQRect( ( handleSize - size ) / 2, 0, size, sl->height() );
					}
				case SC_SliderHandle:
					if ( horizontal )
					{
						if ( ticks == TQSlider::Both )
							return TQRect( pos, ( sl->height() - handleSize ) / 2, len, handleSize );
						else if ( ticks == TQSlider::Above )
							return TQRect( pos, sl->height() - handleSize, len, handleSize );
						return TQRect( pos, 0, len, handleSize );
					}
					else
					{
						if ( ticks == TQSlider::Both )
							return TQRect( ( sl->width() - handleSize ) / 2, pos, handleSize, len );
						else if ( ticks == TQSlider::Above )
							return TQRect( sl->width() - handleSize, pos, handleSize, len );
						return TQRect( 0, pos, handleSize, len );
					}
				default: break;
			}
			break;
		}
		default: break;
	}
	return KStyle::querySubControlMetrics( control, widget, subcontrol, opt );
}


#include <config.h>

#if !defined Q_WS_X11 || defined K_WS_QTONLY
#undef HAVE_X11_EXTENSIONS_SHAPE_H
#endif 

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
//Xlib headers are a mess -> include them down here (any way to ensure that we go second in enable-final order?)
#include <X11/Xlib.h> 
#include <X11/extensions/shape.h> 
#undef   KeyPress
#undef   KeyRelease
#endif

bool KeramikStyle::eventFilter( TQObject* object, TQEvent* event )
{
	if (KStyle::eventFilter( object, event ))
		return true;

	if ( !object->isWidgetType() ) return false;

	//Clear hover highlight when needed
	if ( (event->type() == TQEvent::Leave) && (TQT_BASE_OBJECT(object) == TQT_BASE_OBJECT(hoverWidget)) )
	{
		TQWidget* button = TQT_TQWIDGET(object);
		hoverWidget = 0;
		button->tqrepaint( false );
		return false;
	}

	//Hover highlight on buttons, toolbuttons and combos
	if ( ::tqqt_cast<TQPushButton*>(object) || ::tqqt_cast<TQComboBox*>(object) || ::tqqt_cast<TQToolButton*>(object) )
	{
		if (event->type() == TQEvent::Enter && TQT_TQWIDGET(object)->isEnabled() )
		{
			hoverWidget = TQT_TQWIDGET(object);
			hoverWidget->tqrepaint( false );
		}
		return false;
	}

	//Combo line edits get special frames
	if ( event->type() == TQEvent::Paint && ::tqqt_cast<TQLineEdit*>(object) )
	{
		static bool recursion = false;
		if (recursion )
			return false;

		recursion = true;
		object->event( TQT_TQPAINTEVENT( event ) );
		TQWidget* widget = TQT_TQWIDGET( object );
		TQPainter p( widget );
		Keramik::RectTilePainter( keramik_frame_shadow, false, false, 2, 2 ).draw( &p, widget->rect(),
			widget->tqpalette().color( TQPalette::Normal, TQColorGroup::Button ),
			Qt::black, false, Keramik::TilePainter::PaintFullBlend);
		recursion = false;
		return true;
	}
	else if ( ::tqqt_cast<TQListBox*>(object) ) 
	{
		//Handle combobox drop downs
		switch (event->type())
		{
#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
			//Combo dropdowns are tqshaped	
			case TQEvent::Resize:
			{
				TQListBox* listbox = static_cast<TQListBox*>(TQT_TQWIDGET(object));
				TQResizeEvent* resize = TQT_TQRESIZEEVENT(event);
				if (resize->size().height() < 6)
					return false;
		
				//CHECKME: Not sure the rects are perfect..
				XRectangle rects[5] = {
					{0, 0, resize->size().width()-2, resize->size().height()-6},
					{0, resize->size().height()-6, resize->size().width()-2, 1},
					{1, resize->size().height()-5, resize->size().width()-3, 1},
					{2, resize->size().height()-4, resize->size().width()-5, 1},
					{3, resize->size().height()-3, resize->size().width()-7, 1}
				};
		
				XShapeCombineRectangles(qt_xdisplay(), listbox->handle(), ShapeBounding, 0, 0,
					rects, 5, ShapeSet, YXSorted);
			}
			break;
#endif
			//Combo dropdowns get fancy borders
			case TQEvent::Paint:
			{
				static bool recursion = false;
				if (recursion )
					return false;
				TQListBox* listbox = (TQListBox*) object;
				TQPaintEvent* paint = (TQPaintEvent*) event;
		
		
				if ( !listbox->contentsRect().contains( paint->rect() ) )
				{
					TQPainter p( listbox );
					Keramik::RectTilePainter( keramik_combobox_list, false, false ).draw( &p, 0, 0, listbox->width(), listbox->height(),
							listbox->tqpalette().color( TQPalette::Normal, TQColorGroup::Button ),
							listbox->tqpalette().color( TQPalette::Normal, TQColorGroup::Background ) );
		
					TQPaintEvent newpaint( paint->region().intersect( listbox->contentsRect() ), paint->erased() );
					recursion = true;
					object->event( &newpaint );
					recursion = false;
					return true;
				}
			}
			break;
			
			/**
			 Since our popup is shown a bit overlapping the combo body, a mouse click at the bottom of the
			 widget will result in the release going to the popup, which will cause it to close (#56435). 
			 We solve it by filtering out the first release, if it's in the right area. To do this, we notices shows,
			 move ourselves to front of event filter list, and then capture the first release event, and if it's
			 in the overlap area, filter it out.
			 */
			case TQEvent::Show:
				//Prioritize ourselves to see the mouse events first
				object->removeEventFilter(this);
				object->installEventFilter(this);
				firstComboPopupRelease = true;
				break;
			
			//We need to filter some clicks out.
			case TQEvent::MouseButtonRelease:
				if (firstComboPopupRelease)
				{
					firstComboPopupRelease = false;

					TQMouseEvent* mev = TQT_TQMOUSEEVENT(event);
					TQListBox*    box = static_cast<TQListBox*>(TQT_TQWIDGET(object));
					
					TQWidget* parent = box->parentWidget();
					if (!parent)
						return false;
					
					TQPoint inParCoords = parent->mapFromGlobal(mev->globalPos());
					if (parent->rect().contains(inParCoords))
						return true;
				}
				break;
			case TQEvent::MouseButtonPress:
			case TQEvent::MouseButtonDblClick:
			case TQEvent::Wheel:
			case TQEvent::KeyPress:
			case TQEvent::KeyRelease:
				firstComboPopupRelease = false;
			default:
				return false;
		}
	}
	//Toolbar background gradient handling
	else if (event->type() == TQEvent::Paint &&
			 object->parent() && !qstrcmp(object->name(), kdeToolbarWidget) )
	{
		// Draw a gradient background for custom widgets in the toolbar
		// that have specified a "kde toolbar widget" name.
		renderToolbarWidgetBackground(0, TQT_TQWIDGET(object));

		return false;	// Now draw the contents
	}
	else if (event->type() == TQEvent::Paint  &&  object->parent() && ::tqqt_cast<TQToolBar*>(object->parent()) 
			&& !::tqqt_cast<TQPopupMenu*>(object) )
	{
		// We need to override the paint event to draw a
		// gradient on a QToolBarExtensionWidget.
		TQToolBar* toolbar = static_cast<TQToolBar*>(TQT_TQWIDGET(object->parent()));
		TQWidget* widget = TQT_TQWIDGET(object);
		TQRect wr = widget->rect (), tr = toolbar->rect();
		TQPainter p( widget );

		if ( toolbar->orientation() == Qt::Horizontal )
		{
			Keramik::GradientPainter::renderGradient( &p, wr, widget->tqcolorGroup().button(),
													  true /*horizontal*/, false /*not a menu*/,
													  0, widget->y(), wr.width(), tr.height());
		}
		else
		{
			Keramik::GradientPainter::renderGradient( &p, wr, widget->tqcolorGroup().button(),
													  false /*vertical*/, false /*not a menu*/,
													  widget->x(), 0, tr.width(), wr.height());
		}

		
		//Draw terminator line, too
		p.setPen( toolbar->tqcolorGroup().mid() );
		if ( toolbar->orientation() == Qt::Horizontal )
			p.drawLine( wr.width()-1, 0, wr.width()-1, wr.height()-1 );
		else
			p.drawLine( 0, wr.height()-1, wr.width()-1, wr.height()-1 );
		return true;

	}
	// Track show events for progress bars
	if ( animateProgressBar && ::tqqt_cast<TQProgressBar*>(object) )
	{
		if ((event->type() == TQEvent::Show) && !animationTimer->isActive())
		{
			animationTimer->start( 50, false );
		}
	}
	return false;
}

// vim: ts=4 sw=4 noet
// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
