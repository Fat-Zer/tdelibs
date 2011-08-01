/*
 *	Copyright 2003, Chris Lee <clee@kde.org>
 *
 *	See LICENSE for details about copyright.
 */

#ifndef __ASTEROID_H
#define __ASTEROID_H

#include <kstyle.h>

class AsteroidStyle : public KStyle
{
	Q_OBJECT
	TQ_OBJECT
public:
	AsteroidStyle();
	virtual ~AsteroidStyle();

	void polish(TQWidget *);
	void unPolish(TQWidget *);

	
	void renderSliderHandle(TQPainter *p,
                                     const TQRect &r,
                                     const TQColorGroup &g,
                                     const bool mouseOver,
                                     const bool horizontal,
                                     const bool enabled) const;

	void renderMenuBlendPixmap(KPixmap &,
	                           const TQColorGroup &,
	                           const TQPopupMenu *) const;
	
	void drawKStylePrimitive(KStylePrimitive,
	                         TQPainter *,
	                         const TQWidget *,
	                         const TQRect &,
	                         const TQColorGroup &,
	                         SFlags = Style_Default,
	                         const TQStyleOption & = TQStyleOption::Default) const;

	void tqdrawPrimitive(TQ_PrimitiveElement,
	                   TQPainter *,
	                   const TQRect &,
	                   const TQColorGroup &,
	                   SFlags = Style_Default,
	                   const TQStyleOption & = TQStyleOption::Default) const;

	void tqdrawControl(TQ_ControlElement,
	                 TQPainter *,
	                 const TQWidget *,
	                 const TQRect &,
	                 const TQColorGroup &,
	                 SFlags = Style_Default,
	                 const TQStyleOption & = TQStyleOption::Default) const;

	void tqdrawControlMask(TQ_ControlElement,
	                     TQPainter *,
	                     const TQWidget *,
	                     const TQRect &,
	                     const TQStyleOption &) const;

	void tqdrawComplexControl(TQ_ComplexControl,
	                        TQPainter *,
	                        const TQWidget *,
	                        const TQRect &,
	                        const TQColorGroup &,
	                        SFlags = Style_Default,
	                        SCFlags controls = SC_All,
	                        SCFlags active = SC_None,
	                        const TQStyleOption & = TQStyleOption::Default) const;

	void tqdrawComplexControlMask(TQ_ComplexControl,
	                            TQPainter *,
	                            const TQWidget *,
	                            const TQRect &r,
	                            const TQStyleOption & = TQStyleOption::Default) const;

	int tqpixelMetric(PixelMetric, const TQWidget * = 0) const;

	int kPixelMetric( KStylePixelMetric kpm, const TQWidget* /* widget */) const;

	TQRect subRect(SubRect, const TQWidget *) const;

	TQRect querySubControlMetrics(TQ_ComplexControl,
	                             const TQWidget *,
	                             SubControl,
	                             const TQStyleOption & = TQStyleOption::Default) const;

	TQSize sizeFromContents(ContentsType,
	                       const TQWidget *,
	                       const TQSize &,
	                       const TQStyleOption &) const;

protected:
	bool eventFilter(TQObject *, TQEvent *);

protected slots:
	void paletteChanged();

private:
	AsteroidStyle(const AsteroidStyle &);
	AsteroidStyle &operator = (const AsteroidStyle &);

/*	Settings not needed yet. */
//	TQSettings *settings;
	bool backwards;
};

#endif /* __ASTEROID_H */
