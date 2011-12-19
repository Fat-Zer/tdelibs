/* Keramik Style for KDE3
   Copyright (c) 2002 Malte Starostik <malte@kde.org>

   based on the KDE3 HighColor Style

   Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
             (C) 2001-2002 Fredrik H�glund  <fredrik@kde.org>

   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
             (C) 2000 Dirk Mueller          <mueller@kde.org>
             (C) 2001 Martijn Klingens      <klingens@kde.org>

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

#ifndef __keramik_h__
#define __keramik_h__

#include <tqframe.h>
#include <kstyle.h>

#include "pixmaploader.h"

class TQProgressBar;

class KeramikStyle : public KStyle
{
	Q_OBJECT

public:
	KeramikStyle();
	virtual ~KeramikStyle();

	void renderMenuBlendPixmap( KPixmap& pix, const TQColorGroup &cg, const TQPopupMenu* ) const;
	TQPixmap stylePixmap(StylePixmap stylepixmap, const TQWidget* widget, const TQStyleOption& opt) const;

	void polish( TQWidget* widget );
	void unPolish( TQWidget* widget );
	void polish( TQPalette& );
	void polish( TQApplication *app );

	void drawKStylePrimitive( KStylePrimitive kpe,
	                          TQPainter* p,
	                          const TQWidget* widget,
	                          const TQRect& r,
	                          const TQColorGroup& cg,
	                          SFlags flags = Style_Default,
	                          const TQStyleOption& = TQStyleOption::Default ) const;

	void tqdrawPrimitive( TQ_PrimitiveElement pe,
	                    TQPainter* p,
	                    const TQRect& r,
	                    const TQColorGroup& cg,
	                    SFlags flags = Style_Default,
	                    const TQStyleOption& = TQStyleOption::Default ) const;

	void drawControl( TQ_ControlElement element,
	                  TQPainter* p,
	                  const TQWidget* widget,
	                  const TQRect& r,
	                  const TQColorGroup& cg,
	                  SFlags flags = Style_Default,
	                  const TQStyleOption& opt = TQStyleOption::Default ) const;

	void drawControlMask( TQ_ControlElement element,
	                      TQPainter* p,
	                      const TQWidget* widget,
	                      const TQRect& r,
	                      const TQStyleOption& opt = TQStyleOption::Default ) const;

	void drawComplexControl( TQ_ComplexControl control,
	                         TQPainter* p,
	                         const TQWidget* widget,
	                         const TQRect& r,
	                         const TQColorGroup& cg,
	                         SFlags flags = Style_Default,
	                         SCFlags controls = SC_All,
	                         SCFlags active = SC_None,
	                         const TQStyleOption& = TQStyleOption::Default ) const;

	void drawComplexControlMask( TQ_ComplexControl control,
	                             TQPainter* p,
	                             const TQWidget* widget,
	                             const TQRect& r,
	                             const TQStyleOption& = TQStyleOption::Default ) const;

	int pixelMetric( PixelMetric m, const TQWidget* widget = 0 ) const;

	TQSize tqsizeFromContents( ContentsType contents,
	                        const TQWidget* widget,
	                        const TQSize& contentSize,
	                        const TQStyleOption& opt ) const;

	SubControl querySubControl( TQ_ComplexControl control,
	                            const TQWidget* widget,
	                            const TQPoint& point,
						        const TQStyleOption& opt = TQStyleOption::Default ) const;

	TQRect querySubControlMetrics( TQ_ComplexControl control,
	                              const TQWidget* widget,
	                              SubControl subcontrol,
	                              const TQStyleOption& opt = TQStyleOption::Default ) const;

private slots:
	//Animation slots.
	void updateProgressPos();
	void progressBarDestroyed(TQObject* bar);

private:

	bool isSizeConstrainedCombo(const TQComboBox* combo) const;
	bool isFormWidget          (const TQWidget*   widget) const;

	///Configuration settings
	bool animateProgressBar;
	bool highlightScrollBar;

	//Rendering flags
	mutable bool forceSmallMode;
	mutable bool maskMode;   //Ugly round trip flag to permit masking with little code;
	mutable bool formMode;   //Set when rendering form widgets

	mutable const TQWidget* toolbarBlendWidget;  //Ditto for blending with toolbars

	enum TitleBarMode
	{
		None = 0,
		Regular,
		Maximized
	};

	mutable TitleBarMode titleBarMode; //Set when passing back CC_TilteBar modes to handle
	//PE_ButtonTool properly for them, as well as when handling CC_ToolButton from
	//The maximized window controls.

	mutable bool flatMode; //Set when calling PE_PushButton or PE_ButtonDefault
	// on a flat button.

	mutable bool customScrollMode; //Set when drawing scrollbars with custom colors.
	
	bool firstComboPopupRelease;

	//Animation support.
	TQMap<TQProgressBar*, int> progAnimWidgets;

	bool eventFilter( TQObject* object, TQEvent* event );

	Keramik::TilePainter::PaintMode pmode() const
	{
		if (formMode)
		{
			//If we're a form widget, we blend on painting, and consider ourselves
			//not to have a mask (so we don't get clipped to it)
			if (maskMode)
				return Keramik::TilePainter::PaintTrivialMask;
			else
				return Keramik::TilePainter::PaintFullBlend;
		}
		else
		{
			if (maskMode)
				return Keramik::TilePainter::PaintMask;
			else
				return Keramik::TilePainter::PaintNormal;
		}
	}

	Keramik::TilePainter::PaintMode pmodeFullBlend() const
	{
		return maskMode?Keramik::TilePainter::PaintMask : Keramik::TilePainter::PaintFullBlend;
	}

	TQWidget* hoverWidget;


	bool kickerMode;
	
	// For progress bar animation
	TQTimer *animationTimer;

	TQRect subRect(SubRect r, const TQWidget *widget) const;

	// Disable copy constructor and = operator
	KeramikStyle( const KeramikStyle&  );
	KeramikStyle& operator=( const KeramikStyle&  );
};

#endif

// vim: ts=4 sw=4 noet
// kate: indent-width 4; replace-tabs off; tab-width 4;
