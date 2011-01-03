//----------------------------------------------------------------------------
//    filename             : kmditoolviewaccessor.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 08/2003       by Joseph Wenninger (jowenn@kde.org)
//    changes              : ---
//    patches              : ---
//
//    copyright            : (C) 2003 by Joseph Wenninger (jowenn@kde.org)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------
#ifndef _KMDITOOLVIEWACCESSOR_H_
#define _KMDITOOLVIEWACCESSOR_H_

#include <tqwidget.h>
#include <tqpixmap.h>
#include <tqrect.h>
#include <tqapplication.h>
#include <tqdatetime.h>

#include <kdockwidget.h>

namespace KMDIPrivate
{
class KMDIGUIClient;
}


class KMDI_EXPORT KMdiToolViewAccessor : public TQObject
{
	Q_OBJECT


	friend class KMdiMainFrm;
	friend class KMDIPrivate::KMDIGUIClient;

private:
	/**
	* Internally used by KMdiMainFrm to store a temporary information that the method
	* activate() is unnecessary and that it can by escaped.
	* This saves from unnecessary calls when activate is called directly.
	*/
	bool m_bInterruptActivation;
	/**
	* Internally used to prevent cycles between KMdiMainFrm::activateView() and KMdiChildView::activate().
	*/
	bool m_bMainframesActivateViewIsPending;
	/**
	*
	*/
	bool m_bFocusInEventIsPending;

private:
	KMdiToolViewAccessor( class KMdiMainFrm *parent , TQWidget *widgetToWrap, const TQString& tabToolTip = 0, const TQString& tabCaption = 0 );
	KMdiToolViewAccessor( class KMdiMainFrm *parent );
public:
	~KMdiToolViewAccessor();
	TQWidget *wrapperWidget();
	TQWidget *wrappedWidget();
	void place( KDockWidget::DockPosition pos = KDockWidget::DockNone, TQWidget* pTargetWnd = 0L, int percent = 50 );
	void placeAndShow( KDockWidget::DockPosition pos = KDockWidget::DockNone, TQWidget* pTargetWnd = 0L, int percent = 50 );
	void show();
public slots:
	void setWidgetToWrap( TQWidget* widgetToWrap, const TQString& tabToolTip = 0, const TQString& tabCaption = 0 );
	void hide();
private:
	class KMdiToolViewAccessorPrivate *d;
	class KMdiMainFrm *mdiMainFrm;

protected:
	bool eventFilter( TQObject *o, TQEvent *e );
};


#endif //_KMDITOOLVIEWACCESSOR_H_ 
// kate: space-indent off; tab-width 4; tqreplace-tabs off; indent-mode csands;

