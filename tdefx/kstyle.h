/*
 * $Id$
 * 
 * KStyle
 * Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 * 
 * TQWindowsStyle CC_ListView and style images were kindly donated by TrollTech,
 * Copyright (C) 1998-2000 TrollTech AS.
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

#ifndef __KSTYLE_H
#define __KSTYLE_H

// W A R N I N G
// -------------
// This API is still subject to change.
// I will remove this warning when I feel the API is sufficiently flexible.

#include <tqcommonstyle.h>

#include <tdelibs_export.h>

class KPixmap;

struct KStylePrivate;
/** 
 * Simplifies and extends the TQStyle API to make style coding easier.
 *  
 * The KStyle class provides a simple internal menu transparency engine
 * which attempts to use XRender for accelerated blending where requested,
 * or falls back to fast internal software tinting/blending routines.
 * It also simplifies more complex portions of the TQStyle API, such as
 * the PopupMenuItems, ScrollBars and Sliders by providing extra "primitive
 * elements" which are simple to implement by the style writer.
 *
 * @see TQStyle::QStyle
 * @see TQCommonStyle::QCommonStyle
 * @author Karol Szwed (gallium@kde.org)
 * @version $Id$
 */
class TDEFX_EXPORT KStyle: public TQCommonStyle
{
	Q_OBJECT
	TQ_OBJECT

	public:

		/**
		 * KStyle Flags:
		 * 
		 * @li Default - Default style setting, where menu transparency
		 * and the FilledFrameWorkaround are disabled.
		 * 
		 * @li AllowMenuTransparency - Enable this flag to use KStyle's 
		 * internal menu transparency engine.
		 * 
		 * @li FilledFrameWorkaround - Enable this flag to facilitate 
		 * proper repaints of QMenuBars and QToolBars when the style chooses 
		 * to paint the interior of a TQFrame. The style primitives in question 
		 * are PE_PanelMenuBar and PE_PanelDockWindow. The HighColor style uses
		 * this workaround to enable painting of gradients in menubars and 
		 * toolbars.
		 */
		typedef uint KStyleFlags;
		enum KStyleOption {
			Default 	      =		0x00000000, //!< All options disabled
			AllowMenuTransparency =		0x00000001, //!< Internal transparency enabled
			FilledFrameWorkaround = 	0x00000002  //!< Filled frames enabled
		};

		/**
		 * KStyle ScrollBarType:
		 *
		 * Allows the style writer to easily select what type of scrollbar
		 * should be used without having to duplicate large amounts of source
		 * code by implementing the complex control CC_ScrollBar.
		 *
		 * @li WindowsStyleScrollBar - Two button scrollbar with the previous
		 * button at the top/left, and the next button at the bottom/right.
		 *
		 * @li PlatinumStyleScrollBar - Two button scrollbar with both the 
		 * previous and next buttons at the bottom/right.
		 *
		 * @li ThreeButtonScrollBar - %KDE style three button scrollbar with
		 * two previous buttons, and one next button. The next button is always
		 * at the bottom/right, whilst the two previous buttons are on either 
		 * end of the scrollbar.
		 *
		 * @li NextStyleScrollBar - Similar to the PlatinumStyle scroll bar, but
		 * with the buttons grouped on the opposite end of the scrollbar.
		 *
		 * @see KStyle::KStyle()
		 */
		enum KStyleScrollBarType {
			WindowsStyleScrollBar  = 	0x00000000, //!< two button, windows style
			PlatinumStyleScrollBar = 	0x00000001, //!< two button, platinum style
			ThreeButtonScrollBar   = 	0x00000002, //!< three buttons, %KDE style
			NextStyleScrollBar     = 	0x00000004  //!< two button, NeXT style
		};

		/** 
		 * Constructs a KStyle object.
		 *
		 * Select the appropriate KStyle flags and scrollbar type
		 * for your style. The user's style preferences selected in KControl
		 * are read by using TQSettings and are automatically applied to the style.
		 * As a fallback, KStyle paints progressbars and tabbars. It inherits from
		 * TQCommonStyle for speed, so don't expect much to be implemented. 
		 *
		 * It is advisable to use a currently implemented style such as the HighColor
		 * style as a foundation for any new KStyle, so the limited number of
		 * drawing fallbacks should not prove problematic.
		 *
		 * @param flags the style to be applied
		 * @param sbtype the scroll bar type
		 * @see KStyle::KStyleFlags
		 * @see KStyle::KStyleScrollBarType
		 * @author Karol Szwed (gallium@kde.org)
		 */
		KStyle( KStyleFlags flags = KStyle::Default, 
			KStyleScrollBarType sbtype = KStyle::WindowsStyleScrollBar );

		/** 
		 * Destructs the KStyle object.
		 */
		~KStyle();

		/**
		 * Returns the default widget style depending on color depth.
		 */
		static TQString defaultStyle();

		/**
		 * Modifies the scrollbar type used by the style.
		 * 
		 * This function is only provided for convenience. It allows
		 * you to make a late decision about what scrollbar type to use for the
		 * style after performing some processing in your style's constructor.
		 * In most situations however, setting the scrollbar type via the KStyle
		 * constructor should suffice.
		 * @param sbtype the scroll bar type
		 * @see KStyle::KStyleScrollBarType
		 */
		void setScrollBarType(KStyleScrollBarType sbtype);

		/**
		 * Returns the KStyle flags used to initialize the style.
		 *
		 * This is used solely for the kcmstyle module, and hence is internal.
		 */
		KStyleFlags styleFlags() const;

		// ---------------------------------------------------------------------------

		/**
		 * This virtual function defines the pixmap used to blend between the popup
		 * menu and the background to create different menu transparency effects.
		 * For example, you can fill the pixmap "pix" with a gradient based on the
		 * popup's tqcolorGroup, a texture, or some other fancy painting routine.
		 * KStyle will then internally blend this pixmap with a snapshot of the
		 * background behind the popupMenu to create the illusion of transparency.
		 * 
		 * This virtual is never called if XRender/Software blending is disabled by
		 * the user in KDE's style control module.
		 */
		virtual void renderMenuBlendPixmap( KPixmap& pix, const TQColorGroup& cg, 
						    const TQPopupMenu* popup ) const;

		/**
		 * KStyle Primitive Elements:
		 *
		 * The KStyle class extends the Qt's Style API by providing certain 
		 * simplifications for parts of TQStyle. To do this, the KStylePrimitive
		 * elements were defined, which are very similar to Qt's PrimitiveElement.
		 * 
		 * The first three Handle primitives simplify and extend PE_DockWindowHandle, 
		 * so do not reimplement PE_DockWindowHandle if you want the KStyle handle 
		 * simplifications to be operable. Similarly do not reimplement CC_Slider,
		 * SC_SliderGroove and SC_SliderHandle when using the KStyle slider
		 * primitives. KStyle automatically double-buffers slider painting
		 * when they are drawn via these KStyle primitives to avoid flicker.
		 *
		 * @li KPE_DockWindowHandle - This primitive is already implemented in KStyle,
		 * and paints a bevelled rect with the DockWindow caption text. Re-implement
		 * this primitive to perform other more fancy effects when drawing the dock window
		 * handle.
		 *
		 * @li KPE_ToolBarHandle - This primitive must be reimplemented. It currently
		 * only paints a filled rectangle as default behavior. This primitive is used
		 * to render TQToolBar handles.
		 *
		 * @li KPE_GeneralHandle - This primitive must be reimplemented. It is used
		 * to render general handles that are not part of a TQToolBar or TQDockWindow, such
		 * as the applet handles used in Kicker. The default implementation paints a filled
		 * rect of arbitrary color.
		 *
		 * @li KPE_SliderGroove - This primitive must be reimplemented. It is used to 
		 * paint the slider groove. The default implementation paints a filled rect of
		 * arbitrary color.
		 *
		 * @li KPE_SliderHandle - This primitive must be reimplemented. It is used to
		 * paint the slider handle. The default implementation paints a filled rect of
		 * arbitrary color.
		 *
		 * @li KPE_ListViewExpander - This primitive is already implemented in KStyle. It
		 * is used to draw the Expand/Collapse element in QListViews. To indicate the 
		 * expanded state, the style flags are set to Style_Off, while Style_On implies collapsed.
		 *
		 * @li KPE_ListViewBranch - This primitive is already implemented in KStyle. It is
		 * used to draw the ListView branches where necessary.
		 */
		enum KStylePrimitive {
			KPE_DockWindowHandle,
			KPE_ToolBarHandle,
			KPE_GeneralHandle,

			KPE_SliderGroove,
			KPE_SliderHandle,

			KPE_ListViewExpander,
			KPE_ListViewBranch
		};

		/**
		 * This function is identical to Qt's TQStyle::tqdrawPrimitive(), except that 
		 * it adds one further parameter, 'widget', that can be used to determine 
		 * the widget state of the KStylePrimitive in question.
		 *
		 * @see KStyle::KStylePrimitive
		 * @see TQStyle::tqdrawPrimitive
		 * @see TQStyle::tqdrawComplexControl
		 */
		virtual void drawKStylePrimitive( KStylePrimitive kpe,
					TQPainter* p,
					const TQWidget* widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::SO_Default ) const;


		enum KStylePixelMetric {
			KPM_MenuItemSeparatorHeight		= 0x00000001,
			KPM_MenuItemHMargin			= 0x00000002,
			KPM_MenuItemVMargin			= 0x00000004,
			KPM_MenuItemHFrame			= 0x00000008,
			KPM_MenuItemVFrame			= 0x00000010,
			KPM_MenuItemCheckMarkHMargin	        = 0x00000020,
			KPM_MenuItemArrowHMargin		= 0x00000040,
			KPM_MenuItemTabSpacing			= 0x00000080,
			KPM_ListViewBranchThickness		= 0x00000100
		};

		int kPixelMetric( KStylePixelMetric kpm, const TQWidget* widget = 0 ) const;

		// ---------------------------------------------------------------------------

		void polish( TQWidget* widget );
		void unPolish( TQWidget* widget );
		void polishPopupMenu( TQPopupMenu* );

		void tqdrawPrimitive( TQ_PrimitiveElement pe,
					TQPainter* p,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::SO_Default ) const;

// #ifdef USE_QT4 // tdebindings / smoke needs this function declaration available at all times.  Furthermore I don't think it would hurt to have the declaration available at all times...so leave these commented out for now

//		void tqdrawPrimitive( TQ_ControlElement pe,
//					TQPainter* p,
//					const TQRect &r,
//					const TQColorGroup &cg,
//					SFlags flags = Style_Default,
//					const TQStyleOption& = TQStyleOption::SO_Default ) const;

// #endif // USE_QT4

		void tqdrawControl( TQ_ControlElement element,
					TQPainter* p,
					const TQWidget* widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::SO_Default ) const;

		void tqdrawComplexControl( TQ_ComplexControl control,
					TQPainter *p,
					const TQWidget* widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					SCFlags controls = SC_All,
					SCFlags active = SC_None,
					const TQStyleOption& = TQStyleOption::SO_Default ) const;

		SubControl querySubControl( TQ_ComplexControl control,
					const TQWidget* widget,
					const TQPoint &pos,
					const TQStyleOption& = TQStyleOption::SO_Default ) const;

		TQRect querySubControlMetrics( TQ_ComplexControl control,
					const TQWidget* widget,
					SubControl sc,
					const TQStyleOption& = TQStyleOption::SO_Default ) const;

		int tqpixelMetric( PixelMetric m, 
					const TQWidget* widget = 0 ) const;

		TQRect subRect( SubRect r, 
					const TQWidget* widget ) const;

		TQPixmap stylePixmap( StylePixmap stylepixmap,
					const TQWidget* widget = 0,
					const TQStyleOption& = TQStyleOption::SO_Default ) const;

		int tqstyleHint( TQ_StyleHint sh, 
					const TQWidget* w = 0,
					const TQStyleOption &opt = TQStyleOption::SO_Default,
					TQStyleHintReturn* shr = 0 ) const;

	protected:
		bool eventFilter( TQObject* object, TQEvent* event );

	private:
		// Disable copy constructor and = operator
		KStyle( const KStyle & );
		KStyle& operator=( const KStyle & );

	protected:
		virtual void virtual_hook( int id, void* data );
	private:
		KStylePrivate *d;
};


// vim: set noet ts=4 sw=4:
#endif

