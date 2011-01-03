/*
  Copyright (c) 2000-2001 Trolltech AS (info@trolltech.com)

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#ifndef LIGHTSTYLE_V2_H
#define LIGHTSTYLE_V2_H


#include <kstyle.h>


#ifdef QT_PLUGIN
#  define Q_EXPORT_STYLE_LIGHT_V2
#else
#  define Q_EXPORT_STYLE_LIGHT_V2 Q_EXPORT
#endif // QT_PLUGIN


class Q_EXPORT_STYLE_LIGHT_V2 LightStyleV2 : public KStyle
{
    Q_OBJECT

public:
    LightStyleV2();
    virtual ~LightStyleV2();

    void polishPopupMenu( TQPopupMenu * );

    void drawPrimitive(PrimitiveElement, TQPainter *, const TQRect &, const TQColorGroup &,
		       SFlags = Style_Default,
		       const TQStyleOption & = TQStyleOption::Default ) const;

    void tqdrawControl(ControlElement, TQPainter *, const TQWidget *, const TQRect &,
		     const TQColorGroup &, SFlags = Style_Default,
		     const TQStyleOption & = TQStyleOption::Default ) const;
    void tqdrawControlMask(ControlElement, TQPainter *, const TQWidget *, const TQRect &,
			 const TQStyleOption & = TQStyleOption::Default) const;

    TQRect subRect(SubRect, const TQWidget *) const;

    void tqdrawComplexControl(ComplexControl, TQPainter *, const TQWidget *, const TQRect &,
			    const TQColorGroup &, SFlags = Style_Default,
			    SCFlags = SC_All, SCFlags = SC_None,
			    const TQStyleOption & = TQStyleOption::Default ) const;

    TQRect querySubControlMetrics(ComplexControl, const TQWidget *, SubControl,
				 const TQStyleOption & = TQStyleOption::Default ) const;

    SubControl querySubControl(ComplexControl, const TQWidget *, const TQPoint &,
			       const TQStyleOption &data = TQStyleOption::Default ) const;

    int tqpixelMetric(PixelMetric, const TQWidget * = 0 ) const;

    TQSize sizeFromContents(ContentsType, const TQWidget *, const TQSize &,
			   const TQStyleOption & = TQStyleOption::Default ) const;

    int tqstyleHint(StyleHint, const TQWidget * = 0,
		  const TQStyleOption & = TQStyleOption::Default,
		  QStyleHintReturn * = 0 ) const;

    TQPixmap stylePixmap( StylePixmap stylepixmap,
			 const TQWidget* widget = 0,
			 const TQStyleOption& = TQStyleOption::Default ) const;
};


#endif // LIGHTSTYLE_V2_H
