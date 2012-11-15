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
	
public:
	AsteroidStyle();
	virtual ~AsteroidStyle();

	void polish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *);
	void unPolish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *);
	void applicationPolish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *);
	void applicationUnPolish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *);

	
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
	                         const TQStyleControlElementData &ceData,
	                         ControlElementFlags elementFlags,
	                         const TQRect &,
	                         const TQColorGroup &,
	                         SFlags = Style_Default,
	                         const TQStyleOption & = TQStyleOption::Default,
	                         const TQWidget * = 0) const;

	int styleHint(TQ_StyleHint, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags,
			    const TQStyleOption & = TQStyleOption::Default,
			    TQStyleHintReturn * = 0,
			    const TQWidget * = 0 ) const;

	void drawPrimitive(TQ_PrimitiveElement,
	                   TQPainter *,
	                   const TQStyleControlElementData &ceData,
	                   ControlElementFlags elementFlags,
	                   const TQRect &,
	                   const TQColorGroup &,
	                   SFlags = Style_Default,
	                   const TQStyleOption & = TQStyleOption::Default) const;

	void drawControl(TQ_ControlElement,
	                 TQPainter *,
	                 const TQStyleControlElementData &ceData,
	                 ControlElementFlags elementFlags,
	                 const TQRect &,
	                 const TQColorGroup &,
	                 SFlags = Style_Default,
	                 const TQStyleOption & = TQStyleOption::Default,
	                 const TQWidget * = 0) const;

	void drawControlMask(TQ_ControlElement,
	                     TQPainter *,
	                     const TQStyleControlElementData &ceData,
	                     ControlElementFlags elementFlags,
	                     const TQRect &,
	                     const TQStyleOption &,
	                     const TQWidget * = 0) const;

	void drawComplexControl(TQ_ComplexControl,
	                        TQPainter *,
	                        const TQStyleControlElementData &ceData,
	                        ControlElementFlags elementFlags,
	                        const TQRect &,
	                        const TQColorGroup &,
	                        SFlags = Style_Default,
	                        SCFlags controls = SC_All,
	                        SCFlags active = SC_None,
	                        const TQStyleOption & = TQStyleOption::Default,
	                        const TQWidget * = 0) const;

	void drawComplexControlMask(TQ_ComplexControl,
	                            TQPainter *,
	                            const TQStyleControlElementData &ceData,
	                            const ControlElementFlags elementFlags,
	                            const TQRect &r,
	                            const TQStyleOption & = TQStyleOption::Default,
	                            const TQWidget * = 0) const;

	int pixelMetric(PixelMetric, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget * = 0) const;

	int kPixelMetric( KStylePixelMetric kpm, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget* /* widget */) const;

	TQRect subRect(SubRect, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget *) const;

	TQRect querySubControlMetrics(TQ_ComplexControl,
	                             const TQStyleControlElementData &ceData,
	                             ControlElementFlags elementFlags,
	                             SubControl,
	                             const TQStyleOption & = TQStyleOption::Default,
	                             const TQWidget * = 0) const;

	TQSize sizeFromContents(ContentsType,
	                       const TQStyleControlElementData &ceData,
	                       ControlElementFlags elementFlags,
	                       const TQSize &,
	                       const TQStyleOption &,
	                       const TQWidget * = 0) const;

	virtual bool objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *e );

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
