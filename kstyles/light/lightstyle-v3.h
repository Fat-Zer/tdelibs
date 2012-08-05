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

#ifndef LIGHTSTYLE_V3_H
#define LIGHTSTYLE_V3_H


#ifndef QT_H
#include <tqcommonstyle.h>
#endif // QT_H

#include "kstyle.h"

#ifdef QT_PLUGIN
#  define Q_EXPORT_STYLE_LIGHT_V3
#else
#  define Q_EXPORT_STYLE_LIGHT_V3 Q_EXPORT
#endif // QT_PLUGIN


class Q_EXPORT_STYLE_LIGHT_V3 LightStyleV3 : public KStyle
{
    Q_OBJECT
    TQStyle *basestyle;

public:
    LightStyleV3();
    virtual ~LightStyleV3();

    void polishPopupMenu( TQPopupMenu * );

    void drawPrimitive(TQ_PrimitiveElement, TQPainter *, TQStyleControlElementData ceData, ControlElementFlags elementFlags, const TQRect &, const TQColorGroup &,
		       SFlags = Style_Default,
		       const TQStyleOption & = TQStyleOption::Default ) const;

    void drawControl(TQ_ControlElement, TQPainter *, TQStyleControlElementData ceData, ControlElementFlags elementFlags, const TQRect &,
		     const TQColorGroup &, SFlags = Style_Default,
		     const TQStyleOption & = TQStyleOption::Default, const TQWidget * = 0 ) const;
    void drawControlMask(TQ_ControlElement, TQPainter *, TQStyleControlElementData ceData, ControlElementFlags elementFlags, const TQRect &,
			 const TQStyleOption & = TQStyleOption::Default, const TQWidget * = 0) const;

    TQRect subRect(SubRect, const TQStyleControlElementData ceData, const ControlElementFlags elementFlags, const TQWidget *) const;

    void drawComplexControl(TQ_ComplexControl, TQPainter *, TQStyleControlElementData ceData, ControlElementFlags elementFlags, const TQRect &,
			    const TQColorGroup &, SFlags = Style_Default,
			    SCFlags = SC_All, SCFlags = SC_None,
			    const TQStyleOption & = TQStyleOption::Default, const TQWidget * = 0 ) const;

    TQRect querySubControlMetrics(TQ_ComplexControl, TQStyleControlElementData ceData, ControlElementFlags elementFlags, SubControl,
				 const TQStyleOption & = TQStyleOption::Default, const TQWidget * = 0 ) const;

    SubControl querySubControl(TQ_ComplexControl, TQStyleControlElementData ceData, ControlElementFlags elementFlags, const TQPoint &,
			       const TQStyleOption &data = TQStyleOption::Default, const TQWidget * = 0 ) const;

    int pixelMetric(PixelMetric, TQStyleControlElementData ceData, ControlElementFlags elementFlags, const TQWidget * = 0 ) const;

    TQSize sizeFromContents(ContentsType, TQStyleControlElementData ceData, ControlElementFlags elementFlags, const TQSize &,
			   const TQStyleOption & = TQStyleOption::Default, const TQWidget * = 0 ) const;

    int styleHint(TQ_StyleHint, TQStyleControlElementData ceData, ControlElementFlags elementFlags,
		  const TQStyleOption & = TQStyleOption::Default,
		  TQStyleHintReturn * = 0,
		  const TQWidget * = 0 ) const;

    TQPixmap stylePixmap( StylePixmap stylepixmap,
			 TQStyleControlElementData ceData,
			 ControlElementFlags elementFlags,
			 const TQStyleOption& = TQStyleOption::Default,
			 const TQWidget* widget = 0 ) const;
};


#endif // LIGHTSTYLE_V3_H
