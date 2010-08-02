/*
 * $Id$
 *
 * KDE3 HighColor Style (version 1.0)
 * Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
 *           (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org> 
 *
 * Drawing routines adapted from the KDE2 HCStyle,
 * Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *           (C) 2000 Dirk Mueller          <mueller@kde.org>
 *           (C) 2001 Martijn Klingens      <klingens@kde.org>
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

#ifndef __HIGHCOLOR_H
#define __HIGHCOLOR_H

#include <tqbitmap.h>
#include <tqintdict.h>
#include <kdrawutil.h>
#include <kpixmap.h>
#include <kstyle.h>


enum GradientType{ VSmall=0, VMed, VLarge, HMed, HLarge, GradientCount };
 
class GradientSet
{
	public:
		GradientSet(const TQColor &baseColor);
		~GradientSet();

		KPixmap* gradient(GradientType type);
		TQColor* color() { return(&c); }
	private:
		KPixmap *gradients[5];
		TQColor c;
};


class TQPopupMenu;

class HighColorStyle : public KStyle
{
	Q_OBJECT

	public:
		enum StyleType { HighColor = 0, Default, B3 };
		
		HighColorStyle( StyleType );
		virtual ~HighColorStyle();

		void polish( TQWidget* widget );
		void unPolish( TQWidget* widget );

		void renderMenuBlendPixmap( KPixmap& pix, const TQColorGroup &cg,
								 	const TQPopupMenu* popup ) const;

		void drawKStylePrimitive( KStylePrimitive kpe,
					TQPainter* p,
					const TQWidget* widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default ) const;
		
		void drawPrimitive( PrimitiveElement pe,
					TQPainter* p,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void drawControl( ControlElement element,
					TQPainter *p,
					const TQWidget *widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void drawControlMask( ControlElement element,
					TQPainter *p,
					const TQWidget *widget,
					const TQRect &r,
					const TQStyleOption& = TQStyleOption::Default ) const;
		
		void drawComplexControl( ComplexControl control,
					TQPainter *p,
					const TQWidget *widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					SCFlags controls = SC_All,
					SCFlags active = SC_None,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void drawComplexControlMask( ComplexControl control,
					TQPainter *p,
					const TQWidget *widget,
					const TQRect &r,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void drawItem( TQPainter *p,
		                const TQRect &r,
		                int flags,
		                const TQColorGroup &cg,
		                bool enabled,
		                const TQPixmap *pixmap,
		                const TQString &text,
		                int len = -1,
		                const TQColor *penColor = 0 ) const;

		int pixelMetric( PixelMetric m, 
					const TQWidget *widget = 0 ) const;

		TQSize sizeFromContents( ContentsType contents,
					const TQWidget *widget,
					const TQSize &contentSize,
					const TQStyleOption& opt ) const;

		TQRect subRect( SubRect r, 
					const TQWidget *widget ) const;

		// Fix Qt3's wacky image positions
		TQPixmap stylePixmap( StylePixmap stylepixmap,
					const TQWidget *widget = 0,
					const TQStyleOption& = TQStyleOption::Default ) const;

	protected:
		bool eventFilter( TQObject *object, TQEvent *event );

		void renderGradient( TQPainter* p, 
					const TQRect& r, 
					TQColor clr,
					bool horizontal,
					int px=0, 
					int py=0,
					int pwidth=-1,
					int pheight=-1 ) const;

		TQWidget     *hoverWidget;
		StyleType    type;
		bool         highcolor;
		mutable bool selectionBackground;

	private:
		// Disable copy constructor and = operator
		HighColorStyle( const HighColorStyle & );
		HighColorStyle& operator=( const HighColorStyle & );
};

// vim: set noet ts=4 sw=4:

#endif
