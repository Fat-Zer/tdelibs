/*
 * High Contrast Style (version 1.0)
 *     Copyright (C) 2004 Olaf Schmidt <ojschmidt@kde.org>
 *
 * Derived from Axes Style
 *     Copyright (C) 2003 Maksim Orlovich <orlovich@cs.rochester.edu>
 * 
 * Axes Style based on KDE 3 HighColor Style,
 *     Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
 *               (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <tqdrawutil.h>
#include <tqpainter.h>
#include <tqpointarray.h>
#include <tqstyleplugin.h>

#include <tqfont.h>
#include <tqcombobox.h>
#include <tqheader.h>
#include <tqmenubar.h>
#include <tqpushbutton.h>
#include <tqscrollbar.h>
#include <tqslider.h>
#include <tqtabbar.h>
#include <tqtoolbutton.h>
#include <tqtoolbar.h>
#include <tqpopupmenu.h>
#include <tqprogressbar.h>
#include <tqlistview.h>
#include <tqsettings.h>

#include <kdrawutil.h>
#include <kpixmapeffect.h>

#include "highcontrast.h"
#include "highcontrast.moc"

// -- Style Plugin Interface -------------------------
class HighContrastStylePlugin : public QStylePlugin
{
	public:
		HighContrastStylePlugin() {}
		~HighContrastStylePlugin() {}

		TQStringList keys() const
		{
			return TQStringList() << "HighContrast";
		}

		TQStyle* create( const TQString& key )
		{
			if ( key == "highcontrast" )
				return new HighContrastStyle();
			return 0;
		}
};

KDE_Q_EXPORT_PLUGIN (HighContrastStylePlugin)
// ---------------------------------------------------



static const int itemFrame       = 1;
static const int itemHMargin     = 3;
static const int itemVMargin     = 0;
static const int arrowHMargin    = 6;
static const int rightBorder     = 12;


void addOffset (TQRect* r, int offset, int lineWidth = 0)
{
	int offset1 = offset;
	int offset2 = offset;

	*r = r->normalize();

	if (lineWidth > 0)
	{
		offset1 += lineWidth/2;
		offset2 += lineWidth - lineWidth/2 - 1;
	}

	if (offset1 + offset2 > r->width())
		r->addCoords (r->width()/2, 0, - (r->width() - r->width()/2), 0);
	else
		r->addCoords (offset1, 0, -offset2, 0);

	if (offset1 + offset2 > r->height())
		r->addCoords (0, r->height()/2, 0, - (r->height() - r->height()/2));
	else
		r->addCoords (0, offset1, 0, -offset2);
}


// ---------------------------------------------------------------------------

HighContrastStyle::HighContrastStyle()
	: KStyle( 0, ThreeButtonScrollBar )
{
	TQSettings settings;
	settings.beginGroup("/highcontraststyle/Settings/");
	bool useWideLines = settings.readBoolEntry("wideLines", false);
	hoverWidget = 0L;
	basicLineWidth = useWideLines ? 4 : 2;
}


HighContrastStyle::~HighContrastStyle()
{
}


void HighContrastStyle::polish( TQPalette& pal )
{
	//We do not want the disabled widgets to be greyed out, 
	//as that may be hard indeed (and since we use crossed-out text instead),
	//so we make disabled colors be the same as active foreground and
	//background colour
	for (int c = 0; c < TQColorGroup::NColorRoles; ++c)
		switch (c)
		{
			case TQColorGroup::Button:
			case TQColorGroup::Base:
			case TQColorGroup::Highlight:
				pal.setColor(TQPalette::Disabled, TQColorGroup::ColorRole(c), pal.color(TQPalette::Active, TQColorGroup::Background));
				break;
			case TQColorGroup::ButtonText:
			case TQColorGroup::Text:
			case TQColorGroup::HighlightedText:
				pal.setColor(TQPalette::Disabled, TQColorGroup::ColorRole(c), pal.color(TQPalette::Active, TQColorGroup::Foreground));
				break;
			default:
				pal.setColor(TQPalette::Disabled, TQColorGroup::ColorRole(c), pal.color(TQPalette::Active, TQColorGroup::ColorRole(c)));
		}
}


void HighContrastStyle::polish (TQWidget* widget)
{
	if (widget->inherits ("QButton")
		   || widget->inherits ("QComboBox")
		   || widget->inherits ("QSpinWidget")
		   || widget->inherits ("QLineEdit")
		   || widget->inherits ("QTextEdit"))
	{
		widget->installEventFilter (this);

		TQSpinWidget* spinwidget = dynamic_cast<TQSpinWidget*>(widget);
		if (spinwidget && spinwidget->editWidget())
			spinwidget->editWidget()->installEventFilter (this);
	}

	KStyle::polish (widget);
}


void HighContrastStyle::unPolish (TQWidget* widget)
{
	if (widget->inherits ("QWidget") || widget->inherits ("QComboBox") || widget->inherits ("QSpinWidget") || widget->inherits ("QLineEdit") || widget->inherits ("QTextEdit"))
		widget->removeEventFilter (this);
	KStyle::unPolish (widget);
}

void HighContrastStyle::setColorsNormal (TQPainter* p, const TQColorGroup& cg, int flags, int highlight) const
{
	setColorsByState (p, cg, cg.foreground(), cg.background(), flags, highlight);
}

void HighContrastStyle::setColorsButton (TQPainter* p, const TQColorGroup& cg, int flags, int highlight) const
{
	setColorsByState (p, cg, cg.buttonText(), cg.button(), flags, highlight);
}

void HighContrastStyle::setColorsText (TQPainter* p, const TQColorGroup& cg, int flags, int highlight) const
{
	setColorsByState (p, cg, cg.text(), cg.base(), flags, highlight);
}

void HighContrastStyle::setColorsHighlight (TQPainter* p, const TQColorGroup& cg, int flags) const
{
	setColorsByState (p, cg, cg.highlightedText(), cg.highlight(), flags, 0);
}

void HighContrastStyle::setColorsByState (TQPainter* p, const TQColorGroup& cg, const TQColor& fg, const TQColor& bg, int flags, int highlight) const
{
	TQFont font = p->font();
	font.setStrikeOut (! (flags & Style_Enabled));
	p->setFont (font);

	if ((flags & Style_Enabled) && (flags & highlight))
	{
		p->setPen  (TQPen (cg.highlightedText(), basicLineWidth, flags & Style_Enabled ? Qt::SolidLine : Qt::DotLine));
		p->setBackgroundColor (cg.highlight());
	}
	else
	{
		p->setPen  (TQPen (fg, basicLineWidth, flags & Style_Enabled ? Qt::SolidLine : Qt::DotLine));
		p->setBackgroundColor (bg);
	}

	p->setBrush (TQBrush ());
}

void HighContrastStyle::drawRect (TQPainter* p, TQRect r, int offset, bool filled) const
{
	addOffset (&r, offset, p->pen().width());
	if (filled)
		p->fillRect (r, p->backgroundColor());

	p->drawRect (r);
}

void HighContrastStyle::drawRoundRect (TQPainter* p, TQRect r, int offset, bool filled) const
{
	int lineWidth = p->pen().width();
	if ((r.width() >= 5*lineWidth + 2*offset) && (r.height() >= 5*lineWidth + 2*offset))
	{
		TQRect r2 (r);
		addOffset (&r2, offset, lineWidth);

		addOffset (&r, offset);	
		TQRect r3 (r);
		addOffset (&r3, lineWidth);

		p->save();
		p->setPen (Qt::NoPen);
		if (filled)
			p->fillRect (r3, p->backgroundColor());
		p->drawRect (r3);
		p->restore();
		
		p->drawLine (r.left()+lineWidth, r2.top(), r.right()+1-lineWidth, r2.top());
		p->fillRect (r.left()+1, r.top()+1, lineWidth, lineWidth, p->pen().color());
		p->drawLine (r2.left(), r.top()+lineWidth, r2.left(), r.bottom()+1-lineWidth);
		p->fillRect (r.left()+1, r.bottom()-lineWidth, lineWidth, lineWidth, p->pen().color());
		p->drawLine (r.left()+lineWidth, r2.bottom(), r.right()+1-lineWidth, r2.bottom());
		p->fillRect (r.right()-lineWidth, r.bottom()-lineWidth, lineWidth, lineWidth, p->pen().color());
		p->drawLine (r2.right(), r.top()+lineWidth, r2.right(), r.bottom()+1-lineWidth);
		p->fillRect (r.right()-lineWidth, r.top()+1, lineWidth, lineWidth, p->pen().color());
	}
	else
		drawRect (p, r, offset, filled);
}

void HighContrastStyle::drawEllipse (TQPainter* p, TQRect r, int offset, bool filled) const
{
	addOffset (&r, offset, p->pen().width());

	if (filled) {
		p->save();
		p->setBrush (p->backgroundColor());
		p->drawRoundRect (r, 99, 99);
		p->restore();
	}
	
	p->drawRoundRect (r, 99, 99);
}

void HighContrastStyle::drawArrow (TQPainter* p, TQRect r, PrimitiveElement arrow, int offset) const
{
	p->save();
	addOffset (&r, offset);

	TQPoint center = r.center();
	if (r.height() < r.width())
		r.setWidth (r.height());
	if (r.width() % 2 != 0)
		r.setWidth (r.width() - 1);
	r.setHeight (r.width());
	r.moveCenter (center);
			
	TQPointArray points (3);
	switch (arrow) {
		case PE_ArrowUp:
		case PE_SpinWidgetUp:
		case PE_SpinWidgetPlus: {
			points.setPoint (0, r.bottomLeft());
			points.setPoint (1, r.bottomRight());
			points.setPoint (2, r.center().x(), r.top() + r.height()/7);
			break;
		}
		case PE_ArrowDown:
		case PE_SpinWidgetDown:
		case PE_SpinWidgetMinus: {
			points.setPoint (0, r.topLeft());
			points.setPoint (1, r.topRight());
			points.setPoint (2, r.center().x(), r.bottom() - r.height()/7);
			break;
		}
		case PE_ArrowLeft: {
			points.setPoint (0, r.topRight());
			points.setPoint (1, r.bottomRight());
			points.setPoint (2, r.left() + r.width()/7, r.center().y());
			break;
		}
		default: {
			points.setPoint (0, r.topLeft());
			points.setPoint (1, r.bottomLeft());
			points.setPoint (2, r.right() - r.width()/7, r.center().y());
		}
	}

	p->setPen (p->pen().color());
	p->setBrush (p->pen().color());
	p->drawPolygon (points);
	p->restore();
}

// This function draws primitive elements
void HighContrastStyle::drawPrimitive (PrimitiveElement pe,
								TQPainter *p,
								const TQRect &r,
								const TQColorGroup &cg,
								SFlags flags,
								const TQStyleOption& opt ) const
{
	switch(pe)
	{
		case PE_StatusBarSection: {
			//### TODO: Not everything uses this!
			setColorsNormal (p, cg, Style_Enabled);
			drawRect (p, r);
			break;
		}
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault:
		case PE_ButtonDropDown:
		case PE_ButtonCommand:
		case PE_ButtonTool:
		case PE_ButtonBevel: {
			setColorsButton (p, cg, flags, Style_On|Style_MouseOver|Style_Down);
			drawRoundRect (p, r, 0, false);
			break;
		}

		// FOCUS RECT
		// -------------------------------------------------------------------
		case PE_FocusRect: {
			p->save();
			p->setBrush (TQBrush ());
			p->setPen (TQPen (cg.highlight(), basicLineWidth, Qt::SolidLine));
			drawRoundRect (p, r, basicLineWidth, false);
			p->setPen (TQPen (cg.highlightedText(), basicLineWidth, Qt::DashLine));
			drawRoundRect (p, r, basicLineWidth, false);
			p->restore();
			break;
		}

		case PE_HeaderArrow: {
			setColorsButton (p, cg, flags, 0);
			drawArrow (p, r, flags & Style_Down ? PE_ArrowDown : PE_ArrowUp, 2*basicLineWidth);
			break;
		}
		// HEADER SECTION
		// -------------------------------------------------------------------
		case PE_HeaderSection: {
			setColorsButton (p, cg, flags, 0);
			drawRect (p, r);
			break;
		}


		// SCROLLBAR
		// -------------------------------------------------------------------
		case PE_ScrollBarSlider: {
			setColorsNormal (p, cg);
			p->fillRect (r, p->backgroundColor());

			if (flags & Style_Enabled) {
				setColorsHighlight (p, cg, flags);
				drawRoundRect (p, r);

				if (r.width() >= 7*basicLineWidth && r.height() >= 7*basicLineWidth) {
					TQRect r2 (r);
					r2.setWidth (4*basicLineWidth);
					r2.setHeight (4*basicLineWidth);
					r2.moveCenter (r.center());
					drawRect (p, r2, 0, false);
				}
			}
			break;
		}

		case PE_ScrollBarAddPage:
		case PE_ScrollBarSubPage: {
			setColorsNormal (p, cg);
			p->fillRect (r, p->backgroundColor());

			TQRect r2 (r);
			if (flags & Style_Horizontal)
			{
				if (r2.height() > 5*basicLineWidth)
				{
					r2.setHeight (5*basicLineWidth);
					r2.moveCenter (r.center());
				}
			}
			else
			{
				if (r2.width() > 5*basicLineWidth)
				{
					r2.setWidth (5*basicLineWidth);
					r2.moveCenter (r.center());
				}
			}
			setColorsText (p, cg, flags);
			drawRect (p, r2);
			
			if (flags & Style_Horizontal)
				r2.addCoords (0, basicLineWidth, 0, -basicLineWidth);
			else
				r2.addCoords (basicLineWidth, 0, -basicLineWidth, 0);
			TQPen pen = p->pen();
			pen.setColor (p->backgroundColor());
			p->setPen (pen);
			drawRect (p, r2);

			break;
		}

		case PE_ScrollBarAddLine:
		case PE_ScrollBarSubLine:
		case PE_ScrollBarFirst:
		case PE_ScrollBarLast: {
			setColorsNormal (p, cg);
			p->fillRect (r, p->backgroundColor());

			if (flags & Style_Enabled) {
				setColorsButton (p, cg, flags);
				drawRoundRect (p, r);
				if (pe == PE_ScrollBarAddLine)
					drawArrow (p, r, flags & Style_Horizontal ? PE_ArrowRight : PE_ArrowDown, r.height()/3);
				else if (pe == PE_ScrollBarSubLine)
					drawArrow (p, r, flags & Style_Horizontal ? PE_ArrowLeft : PE_ArrowUp, r.height()/3);
			}
			break;
		}

		
		case PE_ProgressBarChunk: {
			p->fillRect (r, Qt::color1);
			break;
		}


		// CHECKBOX
		// -------------------------------------------------------------------
		case PE_Indicator: {
			setColorsText (p, cg, flags);

			//Draw the outer rect
			drawRect (p, r);

			if (!(flags & Style_Off))
			{
				TQRect r2 (r);
				addOffset (&r2, basicLineWidth);
				if (flags & Style_On)
				{
					p->drawLine (r2.topLeft(), r2.bottomRight());
					p->drawLine (r2.bottomLeft(), r2.topRight());
				}
				else
				{	// Tristate
					p->drawLine (r2.left(), r2.top()+r2.width()/2, r2.right(), r2.top()+r2.width()/2);
				}
				TQPen pen = p->pen();
				pen.setColor (p->backgroundColor());
				p->setPen (pen);
				drawRect (p, r2, 0, false);
			}
			break;
		}
		case PE_IndicatorMask: {
			p->fillRect (r, Qt::color1);
			break;
		}
		case PE_CheckMark: {
			setColorsText (p, cg, flags);

			if (flags & Style_On)
			{
				p->drawLine (r.topLeft(), r.bottomRight());
				p->drawLine (r.bottomLeft(), r.topRight());
			}
			break;
		}

		// RADIOBUTTON (exclusive indicator)
		// -------------------------------------------------------------------
		case PE_ExclusiveIndicator: {
			setColorsText (p, cg, flags);
			drawEllipse (p, r);

			// Indicator "dot"
			if (flags & Style_On) {
				p->setBackgroundColor (p->pen().color());
				drawEllipse (p, r, 2*p->pen().width());
			}

			break;
		}
		case PE_ExclusiveIndicatorMask: {
			p->fillRect (r, Qt::color0);
			p->setBackgroundColor (Qt::color1);
			p->setPen (Qt::NoPen);
			p->setBrush (Qt::color1);
			p->drawEllipse (r);
			break;
		}


		// SPLITTER/DOCKWINDOW HANDLES
		// -------------------------------------------------------------------
		case PE_DockWindowResizeHandle:
		case PE_Splitter: {
			setColorsButton (p, cg, flags);
			p->fillRect (r, p->backgroundColor());
			
			p->setPen (TQPen (p->pen().color(), 1, Qt::DashLine));
			if (flags & Style_Horizontal)
				p->drawLine (r.center().x(), r.top(), r.center().x(), r.bottom());
			else
				p->drawLine (r.left(), r.center().y(), r.right(), r.center().y());
			break;
		}


		// GENERAL PANELS
		// -------------------------------------------------------------------
		case PE_Panel:
		case PE_GroupBoxFrame:
		case PE_PanelPopup: {
			setColorsNormal (p, cg, flags, 0);
			if (!opt.isDefault())
			{
				TQPen pen = p->pen();
				pen.setWidth (opt.lineWidth());
				p->setPen (pen);
			}
			if (pe == PE_PanelPopup)
				drawRect (p, r, 0, false);
			else 
				drawRoundRect (p, r, 0, false);
			break;
		}
		case PE_WindowFrame:
		case PE_TabBarBase: {
			setColorsNormal (p, cg, flags, 0);
			drawRect (p, r, 0, false);
			break;
		}
		case PE_PanelLineEdit: {
			setColorsText (p, cg, flags, 0);
			drawRoundRect (p, r);
			if (flags & (Style_HasFocus | Style_Active))
				drawPrimitive (PE_FocusRect, p, r, cg, flags, TQStyleOption (p->backgroundColor()));
			break;
		}
		case PE_PanelTabWidget:
		case PE_PanelGroupBox: {
			setColorsNormal (p, cg, flags, 0);
			drawRoundRect (p, r);
			break;
		}
		case PE_PanelMenuBar: {			// Menu
			p->fillRect (r, cg.background());
			break;
		}
		case PE_PanelDockWindow: {		// Toolbar
			p->fillRect (r, cg.button());
			break;
		}



		// SEPARATORS
		// -------------------------------------------------------------------
		case PE_Separator: {
			setColorsNormal (p, cg);
			p->fillRect (r, p->backgroundColor());
			p->setPen (p->pen().color());
			if (flags & Style_Horizontal)
				p->drawLine (r.center().x(), r.top()+basicLineWidth, r.center().x(), r.bottom()-basicLineWidth + 1);
			else
				p->drawLine (r.left()+basicLineWidth, r.center().y(), r.right()-basicLineWidth + 1, r.center().y());
			break;
		}
		case PE_DockWindowSeparator: {
			setColorsButton (p, cg);
			p->fillRect (r, p->backgroundColor());
			p->setPen (p->pen().color());
			if (flags & Style_Horizontal)
				p->drawLine (r.center().x(), r.top()+basicLineWidth, r.center().x(), r.bottom()-basicLineWidth);
			else
				p->drawLine (r.left()+basicLineWidth, r.center().y(), r.right()-basicLineWidth, r.center().y());
			break;
		}


		// ARROWS
		// -------------------------------------------------------------------
		case PE_ArrowUp:
		case PE_ArrowDown:
		case PE_ArrowRight:
		case PE_ArrowLeft:
		case PE_SpinWidgetPlus:
		case PE_SpinWidgetUp:
		case PE_SpinWidgetMinus:
		case PE_SpinWidgetDown: {
			setColorsNormal (p, cg, flags);
			drawArrow (p, r, pe);
			break;
		}

		default: {
			KStyle::drawPrimitive( pe, p, r, cg, flags, opt );
		}
	}
}


void HighContrastStyle::drawKStylePrimitive (KStylePrimitive kpe,
										TQPainter* p,
										const TQWidget* widget,
										const TQRect &r,
										const TQColorGroup &cg,
										SFlags flags,
										const TQStyleOption &opt ) const
{
	if ( widget == hoverWidget )
		flags |= Style_MouseOver;

	switch ( kpe )
	{
		// TOOLBAR HANDLE
		// -------------------------------------------------------------------
		case KPE_ToolBarHandle:
		case KPE_DockWindowHandle:
		case KPE_GeneralHandle:
		{
			setColorsButton (p, cg);
			p->fillRect (r, p->backgroundColor());
			p->setBrush (TQBrush (p->pen().color(), Qt::BDiagPattern));
			drawRoundRect (p, r);
			break;
		}


		// SLIDER GROOVE
		// -------------------------------------------------------------------
		case KPE_SliderGroove: {
			setColorsText (p, cg, flags);
			TQRect r2 (r);
			const TQSlider *slider = dynamic_cast<const TQSlider*>(widget);
			if (slider != 0)
			{
				if (slider->orientation() == Qt::Horizontal)
				{
					if (r2.height() > 5*basicLineWidth)
					{
						r2.setHeight (5*basicLineWidth);
						r2.moveCenter (r.center());
					}
				}
				else
				{
					if (r2.width() > 5*basicLineWidth)
					{
						r2.setWidth (5*basicLineWidth);
						r2.moveCenter (r.center());
					}
				}
			}

			drawRoundRect (p, r2);
			break;
		}

		// SLIDER HANDLE
		// -------------------------------------------------------------------
		case KPE_SliderHandle: {
			setColorsHighlight (p, cg, flags);
			drawRoundRect (p, r);
			break;
		}

		case KPE_ListViewExpander: {
			// TODO There is no tqpixelMetric associated with the
			// ListViewExpander in KStyle.
			// To have a properly large expander, the CC_ListView case of
			// tqdrawComplexControl should be handled.
			// Probably it would be better to add a KPM_ListViewExpander metric
			// to the KStyle KStylePixelMetric enum, and have the KStyle
			// tqdrawComplexControl handle it.
			PrimitiveElement direction;
			if (flags & Style_On) { // Collapsed = On
				direction = PE_ArrowRight;

			} else {
				direction = PE_ArrowDown;
			}
			setColorsText (p, cg, flags);
			drawArrow (p, r, direction);
			break;
		}
		case KPE_ListViewBranch: 
			// TODO Draw (thick) dotted line. Check kstyle.cpp
			// Fall down for now
		default:
			KStyle::drawKStylePrimitive( kpe, p, widget, r, cg, flags, opt);
	}
}


void HighContrastStyle::tqdrawControl (ControlElement element,
								TQPainter *p,
								const TQWidget *widget,
								const TQRect &r,
								const TQColorGroup &cg,
								SFlags flags,
								const TQStyleOption& opt ) const
{
	if ( widget == hoverWidget )
		flags |= Style_MouseOver;

	switch (element)
	{
		// TABS
		// -------------------------------------------------------------------
		case CE_ToolBoxTab: {
			setColorsNormal (p, cg, flags, Style_Selected);
			drawRoundRect (p, r);
			break;
		}

		case CE_TabBarTab: {
			setColorsNormal (p, cg, flags, Style_Selected);
			drawRoundRect (p, r);
			
			const TQTabBar *tb = static_cast< const TQTabBar * >(widget);
            TQTabBar::Shape tqshape = tb->tqshape();
			if (tqshape == TQTabBar::TriangularBelow || 
				tqshape == TQTabBar::RoundedBelow) {
				p->fillRect (r.left(), r.top(), 
							 r.width(), 2*basicLineWidth, 
							 p->pen().color());
				p->fillRect (r.left()+basicLineWidth, 
							 flags & Style_Selected ? basicLineWidth : 2*basicLineWidth,
							 r.width()-2*basicLineWidth,
							 basicLineWidth,
							 p->backgroundColor());
			} else {
				p->fillRect (r.left(), r.bottom()-2*basicLineWidth+1, 
							 r.width(), 2*basicLineWidth, 
							 p->pen().color());
				p->fillRect (r.left()+basicLineWidth, 
						     r.bottom()-2*basicLineWidth+1, 
							 r.width()-2*basicLineWidth,
							 flags & Style_Selected ? 2*basicLineWidth : basicLineWidth,
							 p->backgroundColor());
			}
			break;
		}


		// PUSHBUTTON
		// -------------------------------------------------------------------
		case CE_PushButton: {
			TQPushButton *button = (TQPushButton*) widget;
			TQRect br = r;
			bool btnDefault = button->isDefault();

			if (( btnDefault || button->autoDefault() ) && (button->isEnabled())) {
				// Compensate for default indicator
				static int di = tqpixelMetric( PM_ButtonDefaultIndicator );
				addOffset (&br, di);
			}

			if ( btnDefault && (button->isEnabled()))
				drawPrimitive( PE_ButtonDefault, p, r, cg, flags );

			drawPrimitive( PE_ButtonCommand, p, br, cg, flags );

			break;
		}


		// LABEL
		// -------------------------------------------------------------------
		case CE_ProgressBarLabel:
		case CE_TabBarLabel:
		case CE_RadioButtonLabel:
		case CE_CheckBoxLabel:
		case CE_ToolButtonLabel:
		case CE_PushButtonLabel: {
			const TQPixmap* pixmap = 0;
			TQPixmap icon;
			TQString text;
			bool popup = false;
			
			TQIconSet::Mode  mode  = flags & Style_Enabled ? ((flags & Style_HasFocus) ? TQIconSet::Active : TQIconSet::Normal) : TQIconSet::Disabled;
			TQIconSet::State state = flags & Style_On ? TQIconSet::On : TQIconSet::Off;

			int x, y, w, h;
			r.rect( &x, &y, &w, &h );
			
			if (element == CE_ProgressBarLabel) {
				TQProgressBar* progressbar = (TQProgressBar*) widget;
				text = progressbar->progressString();
				setColorsNormal (p, cg, flags);
			}
			else if (element == CE_TabBarLabel) {
				if (!opt.isDefault()) {
					TQTab* tab = opt.tab();
					text = tab->text();
				}
				setColorsNormal (p, cg, flags, Style_Selected);
			}
			else if (element == CE_ToolButtonLabel) {
				TQToolButton* toolbutton = (TQToolButton*) widget;
				text = toolbutton->text();
				pixmap = toolbutton->pixmap();
				if (!toolbutton->iconSet().isNull())
					icon = toolbutton->iconSet().pixmap (TQIconSet::Small, mode, state);
				popup = toolbutton->popup();
				setColorsButton (p, cg, flags);
			}
			else if (element == CE_PushButtonLabel) {
				TQPushButton* pushbutton = (TQPushButton*) widget;
				text = pushbutton->text();
				pixmap = pushbutton->pixmap();
				if (pushbutton->iconSet() && !pushbutton->iconSet()->isNull())
					icon = pushbutton->iconSet()->pixmap (TQIconSet::Small, mode, state);
				popup = pushbutton->popup();
				setColorsButton (p, cg, flags);
			}
			else {
				const TQButton* button = (const TQButton*)widget;
				pixmap = button->pixmap();
				text = button->text();
				setColorsNormal (p, cg);
			}

			// Does the button have a popup menu?
			if (popup) {
				int dx = tqpixelMetric (PM_MenuButtonIndicator, widget);
				drawArrow (p, TQRect(x + w - dx - 2, y + 2, dx, h - 4), PE_ArrowDown);
				w -= dx;
			}

			// Draw the icon if there is one
			if (!icon.isNull())
			{
				// Center the iconset if there's no text or pixmap
				if (text.isEmpty() && ((pixmap == 0) || pixmap->isNull()))
					p->drawPixmap (x + (w - icon.width())  / 2,
								   y + (h - icon.height()) / 2, icon);
				else
					p->drawPixmap (x + 4, y + (h - icon.height()) / 2, icon);

				int  pw = icon.width();
				x += pw + 4;
				w -= pw + 4;
			}

			// Draw a focus rect if the button has focus
			if (flags & Style_HasFocus)
				drawPrimitive (PE_FocusRect, p, r, cg, flags, TQStyleOption (p->backgroundColor()));

			// Draw the label itself
			TQColor color = p->pen().color();
			drawItem (p, TQRect(x, y, w, h),
					  (element == CE_RadioButtonLabel || element == CE_CheckBoxLabel || element == CE_ProgressBarLabel) ? AlignVCenter|AlignLeft|ShowPrefix : AlignCenter|ShowPrefix,
					  cg, flags & Style_Enabled, pixmap, text, -1, &color);
			break;
		}

		// MENUBAR BACKGROUND
		// -------------------------------------------------------------------
		case CE_MenuBarEmptyArea:
		{
			p->fillRect (r, cg.background());
			break;
		}

		// DOCKWINDOW BACKGROUND
		// -------------------------------------------------------------------
		case CE_DockWindowEmptyArea:
		{
			p->fillRect (r, cg.button());
			break;
		}

		// MENUBAR ITEM
		// -------------------------------------------------------------------
		case CE_MenuBarItem: {
			setColorsNormal (p, cg, flags, Style_Active|Style_MouseOver);
			p->fillRect (r, p->backgroundColor ());
			if (!opt.isDefault()) {
				TQMenuItem *mi = opt.menuItem();

				TQColor color = p->pen().color();
				drawItem (p, r, AlignCenter | AlignVCenter | ShowPrefix
						| DontClip | SingleLine, cg, flags,
						mi->pixmap(), mi->text(), -1, &color);
			}
			break;
		}

		// CHECKBOX
		// -------------------------------------------------------------------
		case CE_CheckBox: {
			drawPrimitive (PE_Indicator, p, r, cg, flags);
			break;
		}

		// RADIOBUTTON
		// -------------------------------------------------------------------
		case CE_RadioButton: {
			drawPrimitive (PE_ExclusiveIndicator, p, r, cg, flags);
			break;
		}

		// PROGRESSBAR
		// -------------------------------------------------------------------
		case CE_ProgressBarGroove: {
			setColorsText (p, cg, flags);
			const TQProgressBar *progressbar = dynamic_cast<const TQProgressBar*>(widget);
			if (progressbar) {
				TQRect r2 (r);
				r2.setLeft (p->boundingRect (r, AlignVCenter|AlignLeft|ShowPrefix, progressbar->progressString()).right()
						+ 4*basicLineWidth);
				drawRoundRect (p, r2);
			}
			break;
		}
		case CE_ProgressBarContents: {
			const TQProgressBar *progressbar = dynamic_cast<const TQProgressBar*>(widget);
			if (progressbar)
			{
				TQRect r2 (r);
				r2.setLeft (p->boundingRect (r, AlignVCenter|AlignLeft|ShowPrefix, progressbar->progressString()).right()
						+ 4*basicLineWidth);
				long progress = r2.width() * progressbar->progress();
				if (progressbar->totalSteps() > 0)
				{
					r2.setWidth (progress / progressbar->totalSteps());
				}
				else
				{
					int width = r2.width() / 5;
					int left = progressbar->progress() % (2*(r2.width() - width));
					if (left > r2.width() - width)
						left = 2*(r2.width() - width) - left;
					r2.setLeft (r2.left() + left);
					r2.setWidth (width);
				}
				setColorsHighlight (p, cg, flags);
				if (r2.width() > 0)
					drawRoundRect (p, r2);
			}
			break;
		}

		// POPUPMENU ITEM
		// -------------------------------------------------------------------
		case CE_PopupMenuItem: {
			setColorsNormal (p, cg, flags, Style_Active|Style_MouseOver);
			p->fillRect (r, p->backgroundColor ());

			const TQPopupMenu *popupmenu = (const TQPopupMenu *) widget;
			TQMenuItem *mi = opt.menuItem();
			if (!mi)
				break;

			int  tab        = opt.tabWidth();
			int  checkcol   = opt.maxIconWidth();
			bool checkable  = popupmenu->isCheckable();
			bool reverse    = TQApplication::reverseLayout();
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );

			if ( checkable )
				checkcol = QMAX( checkcol, 20 );

			// Are we a menu item separator?
			if ( mi->isSeparator() ) {
				p->drawLine (r.left() + 1, r.center().y(), r.right(), r.center().y());
				break;
			}

			// Do we have an icon?
			if ( mi->iconSet() && !mi->iconSet()->isNull() ) {
				TQIconSet::Mode mode;
				TQRect cr = tqvisualRect( TQRect(x, y, checkcol, h), r );

				// Select the correct icon from the iconset
				if (!(flags & Style_Enabled))
					mode = TQIconSet::Disabled;
				else if (flags & Style_Active)
					mode = TQIconSet::Active;
				else
					mode = TQIconSet::Normal;

				// Draw the icon
				TQPixmap pixmap = mi->iconSet()->pixmap( TQIconSet::Small, mode );
				TQRect pmr( 0, 0, pixmap.width(), pixmap.height() );
				pmr.moveCenter( cr.center() );
				p->drawPixmap( pmr.topLeft(), pixmap );

				// Do we have an icon and are checked at the same time?
				// Then draw a square border around the icon
				if ( checkable && mi->isChecked() )
				{
					drawRect (p, cr, 0, false);
				}
			}

			// Are we checked? (This time without an icon)
			else if ( checkable && mi->isChecked() ) {
				int cx = reverse ? x+w - checkcol : x;

				TQRect rc (cx, y, checkcol, h);
				addOffset (&rc, 2*basicLineWidth);
				TQPoint center = rc.center();
				if (rc.width() > rc.height())
					rc.setWidth (rc.height());
				else
					rc.setHeight (rc.width());
				rc.moveCenter (center);
					
				p->drawLine (rc.topLeft(), rc.bottomRight());
				p->drawLine (rc.topRight(), rc.bottomLeft());
			}

			// Time to draw the menu item label...
			int xm = itemFrame + checkcol + itemHMargin; // X position margin

			int xp = reverse ? // X position
					x + tab + rightBorder + itemHMargin + itemFrame - 1 :
					x + xm;

			// Label width (minus the width of the accelerator portion)
			int tw = w - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1;

			// Does the menu item draw it's own label?
			if ( mi->custom() ) {
				int m = itemVMargin;
				// Save the painter state in case the custom
				// paint method changes it in some way
				p->save();
				mi->custom()->paint( p, cg, flags & Style_Active, flags & Style_Enabled, xp, y+m, tw, h-2*m );
				p->restore();
			}
			else {
				// The menu item doesn't draw it's own label
				TQString s = mi->text();

				// Does the menu item have a text label?
				if ( !s.isNull() ) {
					int t = s.tqfind( '\t' );
					int m = itemVMargin;
					int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
					text_flags |= reverse ? AlignRight : AlignLeft;

					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						int tabx = reverse ? x + rightBorder + itemHMargin + itemFrame :
							x + w - tab - rightBorder - itemHMargin - itemFrame;

						// Draw the right part of the label (accelerator text)
						p->drawText( tabx, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
						s = s.left( t );
					}

					// Draw the left part of the label (or the whole label
					// if there's no accelerator)

					p->drawText( xp, y+m, tw, h-2*m, text_flags, s, t );

				}

				// The menu item doesn't have a text label
				// Check if it has a pixmap instead
				else if ( mi->pixmap() ) {
					TQPixmap *pixmap = mi->pixmap();

					// Draw the pixmap
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( OpaqueMode );

					int diffw = ( ( w - pixmap->width() ) / 2 )
									+ ( ( w - pixmap->width() ) % 2 );
					p->drawPixmap( x+diffw, y+itemFrame, *pixmap );

					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( TransparentMode );
				}
			}

			// Does the menu item have a submenu?
			if ( mi->popup() ) {
				PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
				int dim = tqpixelMetric(PM_MenuButtonIndicator);
				TQRect vr = tqvisualRect( TQRect( x + w - arrowHMargin - 2*itemFrame - dim,
							y + h / 2 - dim / 2, dim, dim), r );

				// Draw an arrow at the far end of the menu item
				drawArrow (p, vr, arrow);
			}
			break;
		}

		default:
			KStyle::tqdrawControl(element, p, widget, r, cg, flags, opt);
	}
}

void HighContrastStyle::tqdrawControlMask (ControlElement element,
										TQPainter *p,
										const TQWidget *w,
										const TQRect &r,
										const TQStyleOption &opt) const
{
	switch (element) {
		case CE_PushButton:
		case CE_ToolBoxTab:
		case CE_TabBarTab: 
		case CE_ProgressBarLabel:
		case CE_TabBarLabel:
		case CE_RadioButtonLabel:
		case CE_CheckBoxLabel:
		case CE_ToolButtonLabel:
		case CE_PushButtonLabel: 
		case CE_MenuBarEmptyArea:
		case CE_MenuBarItem: 
		case CE_PopupMenuItem: {
			p->fillRect (r, color0);
			break;
		}

		default: {
			KStyle::tqdrawControlMask (element, p, w, r, opt);
		}
	}
}

// Helper to tqfind the next sibling that's not hidden
// Lifted from kstyle.cpp
static TQListViewItem* nextVisibleSibling(TQListViewItem* item)
{
    TQListViewItem* sibling = item;
    do
    {
        sibling = sibling->nextSibling();
    }
    while (sibling && !sibling->isVisible());
    
    return sibling;
}

void HighContrastStyle::tqdrawComplexControl (ComplexControl control,
									TQPainter *p,
									const TQWidget *widget,
									const TQRect &r,
									const TQColorGroup &cg,
									SFlags flags,
									SCFlags controls,
									SCFlags active,
									const TQStyleOption& opt ) const
{
	if ( widget == hoverWidget )
		flags |= Style_MouseOver;

	switch(control)
	{
		// COMBOBOX
		// -------------------------------------------------------------------
		case CC_ComboBox: {
			setColorsText (p, cg, flags);
			drawRoundRect (p, r);
			
			TQRect r2 = TQStyle::tqvisualRect (querySubControlMetrics (CC_ComboBox, widget, SC_ComboBoxArrow), widget);
			if (flags & Style_HasFocus) {
				TQRect r3 (r);
				if (r2.left() > 0)
					r3.setRight (r2.left()+basicLineWidth-1);
				else
					r3.setLeft (r2.right()-basicLineWidth+1);

				drawPrimitive (PE_FocusRect, p, r3, cg, flags, TQStyleOption (p->backgroundColor()));
			}
			
			setColorsButton (p, cg, flags);
			// Draw arrow if required
			if (controls & SC_ComboBoxArrow) {
				drawRoundRect (p, r2);
				drawArrow (p, r2, PE_ArrowDown, 2*basicLineWidth);
			}

			setColorsText (p, cg, flags);
			break;
		}

		// SPINWIDGET
		// -------------------------------------------------------------------
		case CC_SpinWidget: {
			if (controls & SC_SpinWidgetFrame) {
				setColorsText (p, cg, flags);
				drawRoundRect (p, r);
				if (flags & Style_HasFocus)
					drawPrimitive(PE_FocusRect, p, r, cg, flags, TQStyleOption (p->backgroundColor()));
			}
			
			setColorsButton (p, cg, flags);
			// Draw arrows if required
			if (controls & SC_SpinWidgetDown) {
				TQRect r2 = TQStyle::tqvisualRect (querySubControlMetrics (CC_SpinWidget, widget, SC_SpinWidgetDown), widget);
				drawRoundRect (p, r2);
				drawArrow (p, r2, PE_SpinWidgetDown, 2*basicLineWidth);
			}
			if (controls & SC_SpinWidgetUp) {
				TQRect r2 = TQStyle::tqvisualRect (querySubControlMetrics (CC_SpinWidget, widget, SC_SpinWidgetUp), widget);
				drawRoundRect (p, r2);
				drawArrow (p, r2, PE_SpinWidgetUp, 2*basicLineWidth);
			}

			setColorsText (p, cg, flags);
			break;
		}

		// TOOLBUTTON
		// -------------------------------------------------------------------
		case CC_ToolButton: {
			const TQToolButton *toolbutton = (const TQToolButton *) widget;

			setColorsButton (p, cg, flags);
			p->fillRect (r, p->backgroundColor ());

			TQRect button, menuarea;
			button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
			menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

			SFlags bflags = flags,
				   mflags = flags;

			if (active & SC_ToolButton)
				bflags |= Style_Down;
			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;

			if (controls & SC_ToolButton)
			{
				// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);

				// Check whether to draw a background pixmap
				else if ( toolbutton->tqparentWidget() &&
						  toolbutton->tqparentWidget()->backgroundPixmap() &&
						  !toolbutton->tqparentWidget()->backgroundPixmap()->isNull() )
				{
					TQPixmap pixmap = *(toolbutton->tqparentWidget()->backgroundPixmap());
					p->drawTiledPixmap( r, pixmap, toolbutton->pos() );
				}
			}

			// Draw a toolbutton menu indicator if required
			if (controls & SC_ToolButtonMenu)
			{
				if (mflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
				drawArrow (p, menuarea, PE_ArrowDown);
			}

			if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
				TQRect fr = toolbutton->rect();
				addOffset (&fr, 3);
				drawPrimitive(PE_FocusRect, p, fr, cg, flags, TQStyleOption (p->backgroundColor()));
			}

			break;
		}

		// LISTVIEW
		// -------------------------------------------------------------------
		case CC_ListView: {
			/*
			 * Sigh... Lifted and modified from kstyle.cpp 
			 */
			/* 
			 * Many thanks to TrollTech AS for donating CC_ListView from TQWindowsStyle.
			 * CC_ListView code is Copyright (C) 1998-2000 TrollTech AS.
			 */

			// Paint the icon and text.
			if ( controls & SC_ListView )
				TQCommonStyle::tqdrawComplexControl( control, p, widget, r, cg, flags, controls, active, opt );

			// If we're have a branch or are expanded...
			if ( controls & (SC_ListViewBranch | SC_ListViewExpand) )
			{
				// If no list view item was supplied, break
				if (opt.isDefault())
					break;

				TQListViewItem *item  = opt.listViewItem();
				TQListViewItem *child = item->firstChild();

				int y = r.y();
				int c;	// dotline vertice count
				int dotoffset = 0;
				TQPointArray dotlines;

				if ( active == SC_All && controls == SC_ListViewExpand ) {
					// We only need to draw a vertical line
					c = 2;
					dotlines.resize(2);
					dotlines[0] = TQPoint( r.right(), r.top() );
					dotlines[1] = TQPoint( r.right(), r.bottom() );

				} else {

					int linetop = 0, linebot = 0;
					// each branch needs at most two lines, ie. four end points
					dotoffset = (item->itemPos() + item->height() - y) % 2;
					dotlines.resize( item->childCount() * 4 );
					c = 0;

					// skip the stuff above the exposed rectangle
					while ( child && y + child->height() <= 0 )
					{
						y += child->totalHeight();
						child = nextVisibleSibling(child);
					}

					int bx = r.width() / 2;

					// paint stuff in the magical area
					TQListView* v = item->listView();
					int lh = QMAX( p->fontMetrics().height() + 2 * v->itemMargin(),
								   TQApplication::globalStrut().height() );
					if ( lh % 2 > 0 )
						lh++;

					// Draw all the expand/close boxes...
					TQRect boxrect;
					TQStyle::StyleFlags boxflags;
					while ( child && y < r.height() )
					{
						linebot = y + lh/2;
						if ( (child->isExpandable() || child->childCount()) &&
							 (child->height() > 0) )
						{
							int h = QMIN(lh, 24) - 4*basicLineWidth;
							if (h < 10) 
								h = 10;
							else 
								h &= ~1; // Force an even number of pixels
			
							// The primitive requires a rect.
							boxrect = TQRect( bx-h/2, linebot-h/2, h, h );
							boxflags = child->isOpen() ? TQStyle::Style_Off : TQStyle::Style_On;

							// KStyle extension: Draw the box and expand/collapse indicator
							drawKStylePrimitive( KPE_ListViewExpander, p, NULL, boxrect, cg, boxflags, opt );

							// dotlinery
							p->setPen( cg.mid() );
							dotlines[c++] = TQPoint( bx, linetop );
							dotlines[c++] = TQPoint( bx, linebot - 5 );
							dotlines[c++] = TQPoint( bx + 5, linebot );
							dotlines[c++] = TQPoint( r.width(), linebot );
							linetop = linebot + 5;
						} else {
							// just dotlinery
							dotlines[c++] = TQPoint( bx+1, linebot );
							dotlines[c++] = TQPoint( r.width(), linebot );
						}

						y += child->totalHeight();
						child = nextVisibleSibling(child);
					}

					if ( child ) // there's a child to draw, so move linebot to edge of rectangle
						linebot = r.height();

					if ( linetop < linebot )
					{
						dotlines[c++] = TQPoint( bx, linetop );
						dotlines[c++] = TQPoint( bx, linebot );
					}
				}

				// Draw all the branches...
				static int thickness = kPixelMetric( KPM_ListViewBranchThickness );
				int line; // index into dotlines
				TQRect branchrect;
				TQStyle::StyleFlags branchflags;
				for( line = 0; line < c; line += 2 )
				{
					// assumptions here: lines are horizontal or vertical.
					// lines always start with the numerically lowest
					// coordinate.

					// point ... relevant coordinate of current point
					// end ..... same coordinate of the end of the current line
					// other ... the other coordinate of the current point/line
					if ( dotlines[line].y() == dotlines[line+1].y() )
					{
						// Horizontal branch
						int end = dotlines[line+1].x();
						int point = dotlines[line].x();
						int other = dotlines[line].y();

						branchrect  = TQRect( point, other-(thickness/2), end-point, thickness );
						branchflags = TQStyle::Style_Horizontal;

						// KStyle extension: Draw the horizontal branch
						drawKStylePrimitive( KPE_ListViewBranch, p, NULL, branchrect, cg, branchflags, opt );

					} else {
						// Vertical branch
						int end = dotlines[line+1].y();
						int point = dotlines[line].y();
						int other = dotlines[line].x();
						int pixmapoffset = ((point & 1) != dotoffset ) ? 1 : 0;

						branchrect  = TQRect( other-(thickness/2), point, thickness, end-point );
						if (!pixmapoffset)	// ### Hackish - used to hint the offset
							branchflags = TQStyle::Style_NoChange;
						else
							branchflags = TQStyle::Style_Default;

						// KStyle extension: Draw the vertical branch
						drawKStylePrimitive( KPE_ListViewBranch, p, NULL, branchrect, cg, branchflags, opt );
					}
				}
			}
			break;
		}

		default:
			KStyle::tqdrawComplexControl(control, p, widget,
						r, cg, flags, controls, active, opt);
			break;
	}
}

void HighContrastStyle::tqdrawComplexControlMask(ComplexControl c,
											   TQPainter *p,
											   const TQWidget *w,
											   const TQRect &r,
											   const TQStyleOption &o) const
{
	switch (c) {
		case CC_SpinWidget:
		case CC_ToolButton:
		case CC_ComboBox: {
			p->fillRect (r, color0);
			break;
		}
		default: {
			KStyle::tqdrawComplexControlMask (c, p, w, r, o);
		}
	}
}

void HighContrastStyle::drawItem( TQPainter *p,
							   const TQRect &r,
							   int flags,
							   const TQColorGroup &cg,
							   bool enabled,
							   const TQPixmap *pixmap,
							   const TQString &text,
							   int len,
							   const TQColor *penColor ) const
{
	p->save();

	// make the disabled things use the cross-line
	TQFont font = p->font();
	font.setStrikeOut (!enabled);
	p->setFont (font);
	
	enabled = true; //do not ghost it in Qt

	KStyle::drawItem (p, r, flags, cg, enabled, pixmap, text, len, penColor);
	
	p->restore();
}

TQRect HighContrastStyle::querySubControlMetrics( ComplexControl control,
									const TQWidget* widget,
								  SubControl subcontrol,
								  const TQStyleOption& opt ) const
{
	switch (control)
	{
		case CC_ComboBox : {
			int arrow = tqpixelMetric (PM_ScrollBarExtent, widget);
			switch (subcontrol)
			{
				case SC_ComboBoxFrame:
					return TQRect (0, 0, widget->width(), widget->height());
				case SC_ComboBoxArrow:
					return TQRect (widget->width() - arrow, 0, arrow, widget->height());
				case SC_ComboBoxEditField:
					return TQRect (2*basicLineWidth, 2*basicLineWidth,
								widget->width() - arrow - 3*basicLineWidth, widget->height() - 4*basicLineWidth);
	
				default: break;
			}
			break;
		}
		case CC_SpinWidget : {
			int arrow = tqpixelMetric (PM_ScrollBarExtent, 0);
			switch (subcontrol)
			{
				case SC_SpinWidgetFrame:
					return TQRect (0, 0, widget->width(), widget->height());
				case SC_SpinWidgetButtonField:
					return TQRect (widget->width() - arrow, 0, arrow, widget->height());
				case SC_SpinWidgetUp:
					return TQRect (widget->width() - arrow, 0, arrow, widget->height()/2);
				case SC_SpinWidgetDown:
					return TQRect (widget->width() - arrow, widget->height()/2,
							arrow, widget->height()-widget->height()/2);
				case SC_SpinWidgetEditField:
					return TQRect (2*basicLineWidth, 2*basicLineWidth,
							widget->width() - arrow - 3*basicLineWidth, widget->height() - 4*basicLineWidth);
	
				default: break;
			}
			break;
		}

		default: break;
	}

	return KStyle::querySubControlMetrics (control, widget, subcontrol, opt);
}


int HighContrastStyle::tqpixelMetric(PixelMetric m, const TQWidget *widget) const
{
	//### TODO: Use the tab metrics changes from Ker.
	switch(m)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PM_ButtonMargin:				// Space btw. frame and label
			return 2*basicLineWidth;

		case PM_ButtonDefaultIndicator: {
			if ((widget != 0) && !widget->isEnabled())
				return 0;
			else
				return 2*basicLineWidth;
		}

		case PM_ButtonShiftHorizontal:
		case PM_ButtonShiftVertical:
			return 0;

		case PM_ScrollBarExtent: {
			int h = 0;
			if (widget != 0)
				h = (2*widget->fontMetrics().lineSpacing())/3;
			
			if (h > 9*basicLineWidth+4)
				return h;
			else
				return 9*basicLineWidth+4;
		}

		case PM_DefaultFrameWidth: {
			if (widget && (widget->inherits ("QLineEdit") || widget->inherits ("QTextEdit")))
				return 2*basicLineWidth;
			else 
				return basicLineWidth;
		}

		case PM_SpinBoxFrameWidth: {
			return 2*basicLineWidth;
		}

		case PM_MenuButtonIndicator: {		// Arrow width
			int h = 0;
			if (widget != 0)
				h = widget->fontMetrics().lineSpacing()/2;
			
			if (h > 3*basicLineWidth)
				return h;
			else
				return 3*basicLineWidth;
		}

		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
		case PM_ExclusiveIndicatorHeight:
		case PM_IndicatorWidth:				// Checkbox size
		case PM_IndicatorHeight: {
			int h = 0;
			if (widget != 0)
				h = widget->fontMetrics().lineSpacing()-2*basicLineWidth;
			
			if (h > 6*basicLineWidth)
				return h;
			else
				return 6*basicLineWidth;
		}
		
		case PM_DockWindowSeparatorExtent: {
			return 2*basicLineWidth + 1;
		}
		case PM_DockWindowHandleExtent: {
			int w = 0;
			if (widget != 0)
				w = widget->fontMetrics().lineSpacing()/4;
			if (w > 5*basicLineWidth)
				return w;
			else
				return 5*basicLineWidth;
		}

		default:
			return KStyle::tqpixelMetric(m, widget);
	}
}

int HighContrastStyle::kPixelMetric( KStylePixelMetric kpm, const TQWidget *widget ) const
{
	switch (kpm) {
		case KPM_ListViewBranchThickness:
			// XXX Proper support of thick branches requires reimplementation of
			// the drawKStylePrimitive KPE_ListViewBranch case.
			return basicLineWidth;
		default:
			return KStyle::kPixelMetric(kpm, widget);
	}
}

TQSize HighContrastStyle::sizeFromContents( ContentsType contents,
										const TQWidget* widget,
										const TQSize &contentSize,
										const TQStyleOption& opt ) const
{
	switch (contents)
	{
		// PUSHBUTTON SIZE
		// ------------------------------------------------------------------
		case CT_PushButton: {
			const TQPushButton* button = (const TQPushButton*) widget;
			int w  = contentSize.width();
			int h  = contentSize.height();
			int bm = tqpixelMetric( PM_ButtonMargin, widget );
			int fw = tqpixelMetric( PM_DefaultFrameWidth, widget ) * 2;

			w += bm + fw + 6;	// ### Add 6 to make way for bold font.
			h += bm + fw;

			// Ensure we stick to standard width and heights.
			if (( button->isDefault() || button->autoDefault() ) && (button->isEnabled())) {
				if ( w < 80 && !button->text().isEmpty() )
					w = 80;

				// Compensate for default indicator
				int di = tqpixelMetric( PM_ButtonDefaultIndicator );
				w += di * 2;
				h += di * 2;
			}

			if ( h < 22 )
				h = 22;

			return TQSize( w + basicLineWidth*2, h + basicLineWidth*2 );
		}

		// TOOLBUTTON SIZE
		// -----------------------------------------------------------------
		case CT_ToolButton: {
			int w  = contentSize.width();
			int h  = contentSize.height();
			return TQSize(w + basicLineWidth*2 + 6, h + basicLineWidth*2 + 5);
			break;
		}

		// COMBOBOX SIZE
		// -----------------------------------------------------------------
		case CT_ComboBox: {
			const TQComboBox *cb = static_cast< const TQComboBox* > (widget);
			int borderSize =  (cb->editable() ? 4 : 2) * basicLineWidth;
			int arrowSize = tqpixelMetric (PM_ScrollBarExtent, cb);
			return TQSize(borderSize + basicLineWidth + arrowSize, borderSize) + contentSize;
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
				w = 10; // Arbitrary
				h = 4;
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

				if ( mi->iconSet() && ! mi->iconSet()->isNull() )
					h = QMAX( h, mi->iconSet()->pixmap(
								TQIconSet::Small, TQIconSet::Normal).height() +
								2 * itemFrame );
			}

			if ( ! mi->text().isNull() && mi->text().tqfind('\t') >= 0 )
				w += 12;
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


		// LINEDIT SIZE
		// -----------------------------------------------------------------
		case CT_LineEdit: {
			return contentSize + TQSize (4*basicLineWidth, 4*basicLineWidth);
		}


		default:
			return KStyle::sizeFromContents( contents, widget, contentSize, opt );
	}
}

TQRect HighContrastStyle::subRect (SubRect subrect, const TQWidget * widget) const
{
	switch (subrect) {
		case SR_ProgressBarGroove:
		case SR_ProgressBarContents:
		case SR_ProgressBarLabel:
			return widget->rect();
		default:
			return KStyle::subRect (subrect, widget);
	}
}

bool HighContrastStyle::eventFilter (TQObject *object, TQEvent *event)
{
	TQWidget* widget = dynamic_cast<TQWidget*>(object);
	if (widget)
	{
		// Handle hover effects.
		if (event->type() == TQEvent::Enter
				&& (widget->inherits ("QButton")
					|| widget->inherits ("QComboBox")
					|| widget->inherits ("QSpinWidget")))
		{
			hoverWidget = widget;
			widget->tqrepaint (false);
		}
		else if (event->type() == TQEvent::Leave
					&& (widget->inherits ("QButton")
						|| widget->inherits ("QComboBox")
						|| widget->inherits ("QSpinWidget")))
		{
			if (object == hoverWidget)
				hoverWidget = 0L;
			widget->tqrepaint (false);
		}
		// Make sure the focus rectangle is shown correctly.
		else if (event->type() == TQEvent::FocusIn || event->type() == TQEvent::FocusOut)
		{
			TQWidget* widgetparent = dynamic_cast<TQWidget*>(widget->parent());
			while (widgetparent
							&& ! widgetparent->inherits ("QComboBox")
							&& ! widgetparent->inherits ("QSpinWidget"))
			{
				widgetparent = dynamic_cast<TQWidget*>(widgetparent->parent());
			}

			if (widgetparent)
				widgetparent->tqrepaint (false);
			else
				widget->tqrepaint (false);
		}
	}
	
	return KStyle::eventFilter (object, event);
}

// vim: set noet ts=4 sw=4:
// kate: indent-width 4; tqreplace-tabs off; smart-indent on; tab-width 4;
