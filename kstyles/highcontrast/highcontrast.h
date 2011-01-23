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
 * KDE 3 HighColor Style drawing routines adapted from the KDE2 HCStyle,
 *     Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *               (C) 2000 Dirk Mueller          <mueller@kde.org>
 *               (C) 2001 Martijn Klingens      <klingens@kde.org>
 *
 * Includes portions from KStyle,
 *     Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *
 * Many thanks to Bradley T. Hughes for the 3 button scrollbar code.
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

#ifndef __HIGHCONTRAST_H
#define __HIGHCONTRAST_H

#include <tqbitmap.h>
#include <tqintdict.h>
#include <kdrawutil.h>
#include <kpixmap.h>
#include <kstyle.h>


class TQPopupMenu;

class HighContrastStyle : public KStyle
{
	Q_OBJECT

	public:
		HighContrastStyle();
		virtual ~HighContrastStyle();
        
		void polish( TQPalette& pal );        

		void polish( TQWidget* widget );
		void unPolish( TQWidget* widget );

		void drawKStylePrimitive( KStylePrimitive kpe,
					TQPainter* p,
					const TQWidget* widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void tqdrawPrimitive( TQ_PrimitiveElement pe,
					TQPainter* p,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void tqdrawControl( TQ_ControlElement element,
					TQPainter *p,
					const TQWidget *widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void tqdrawControlMask( TQ_ControlElement element,
					TQPainter *p,
					const TQWidget *widget,
					const TQRect &r,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void tqdrawComplexControl( TQ_ComplexControl control,
					TQPainter *p,
					const TQWidget *widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					SCFlags controls = SC_All,
					SCFlags active = SC_None,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void tqdrawComplexControlMask( TQ_ComplexControl control,
					TQPainter *p,
					const TQWidget *widget,
					const TQRect &r,
					const TQStyleOption& = TQStyleOption::Default ) const;

		TQRect querySubControlMetrics( TQ_ComplexControl control,
					const TQWidget* widget,
					SubControl subcontrol,
					const TQStyleOption& opt = TQStyleOption::Default ) const;


		void drawItem( TQPainter *p,
					const TQRect &r,
					int flags,
					const TQColorGroup &cg,
					bool enabled,
					const TQPixmap *pixmap,
					const TQString &text,
					int len = -1,
					const TQColor *penColor = 0 ) const;

		int tqpixelMetric( PixelMetric m,
					const TQWidget *widget = 0 ) const;

		int kPixelMetric( KStylePixelMetric m,
					const TQWidget *widget = 0 ) const;

		TQSize tqsizeFromContents( ContentsType contents,
					const TQWidget *widget,
					const TQSize &contentSize,
					const TQStyleOption& opt ) const;

		TQRect subRect (SubRect subrect, const TQWidget * widget) const;

	protected:
		bool eventFilter( TQObject *object, TQEvent *event );

		TQWidget     *hoverWidget;

	private:
		void setColorsNormal (TQPainter* p, const TQColorGroup& cg, int flags = Style_Enabled, int highlight = Style_Down|Style_MouseOver) const;
		void setColorsButton (TQPainter* p, const TQColorGroup& cg, int flags = Style_Enabled, int highlight = Style_Down|Style_MouseOver) const;
		void setColorsText (TQPainter* p, const TQColorGroup& cg, int flags = Style_Enabled, int highlight = Style_Down|Style_MouseOver) const;
		void setColorsHighlight (TQPainter* p, const TQColorGroup& cg, int flags = Style_Enabled) const;
		void setColorsByState (TQPainter* p, const TQColorGroup& cg, const TQColor& fg, const TQColor& bg, int flags, int highlight) const;

		void drawRect (TQPainter* p, TQRect r, int offset = 0, bool filled = true) const;
		void drawRoundRect (TQPainter* p, TQRect r, int offset = 0, bool filled = true) const;
		void drawEllipse (TQPainter* p, TQRect r, int offset = 0, bool filled = true) const;
		void drawArrow (TQPainter* p, TQRect r, TQ_PrimitiveElement arrow, int offset = 0) const;

		int basicLineWidth;
		// Disable copy constructor and = operator
		HighContrastStyle( const HighContrastStyle & );
		HighContrastStyle& operator=( const HighContrastStyle & );
};

// vim: set noet ts=4 sw=4:
// kate: indent-width 4; replace-tabs off; smart-indent on; tab-width 4;

#endif
