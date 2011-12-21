/* This file is part of the KDE libraries
	 Copyright (C) 2001 Frerich Raabe <raabe@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 Library General Public License for more details.

	 You should have received a copy of the GNU Library General Public License
	 along with this library; see the file COPYING.LIB.  If not, write to
	 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	 Boston, MA 02110-1301, USA.
*/

#include "karrowbutton.h"

#include <tqstyle.h>
#include <tqpainter.h>

class KArrowButtonPrivate
{
	public:
		Qt::ArrowType arrow;
};

KArrowButton::KArrowButton(TQWidget *parent, Qt::ArrowType arrow,
		const char *name)
	: TQPushButton(parent, name)
{
	d = new KArrowButtonPrivate();
	d->arrow = arrow;
}

KArrowButton::~KArrowButton()
{
	delete d;
}

TQSize KArrowButton::sizeHint() const
{
	return TQSize( 12, 12 );
}

void KArrowButton::setArrowType(Qt::ArrowType a)
{
	if (d->arrow != a) {
		d->arrow = a;
		repaint();
	}
}
Qt::ArrowType KArrowButton::arrowType() const
{
	return d->arrow;
}

void KArrowButton::drawButton(TQPainter *p)
{
	const unsigned int arrowSize = 8;
	const unsigned int margin = 2;
	
        p->fillRect( rect(), colorGroup().brush( TQColorGroup::Background ) );
	style().tqdrawPrimitive( TQStyle::PE_Panel, p, TQRect( 0, 0, width(), height() ),
			       colorGroup(), 
			       isDown() ? TQStyle::Style_Sunken : TQStyle::Style_Default,
			       TQStyleOption( 2, 0 ) );

	if (static_cast<unsigned int>(width()) < arrowSize + margin ||
	    static_cast<unsigned int>(height()) < arrowSize + margin)
		return; // don't draw arrows if we are too small

	unsigned int x = 0, y = 0;
	if (d->arrow == DownArrow) {
		x = (width() - arrowSize) / 2;
		y = height() - (arrowSize + margin);
	} else if (d->arrow == UpArrow) {
		x = (width() - arrowSize) / 2;
		y = margin;
	} else if (d->arrow == RightArrow) {
		x = width() - (arrowSize + margin);
		y = (height() - arrowSize) / 2;
	} else { // arrowType == LeftArrow
		x = margin;
		y = (height() - arrowSize) / 2;
	}

	if (isDown()) {
		x++;
		y++;
	}

	TQStyle::PrimitiveElement e = TQStyle::PE_ArrowLeft;
	switch (d->arrow)
	{
		case Qt::LeftArrow: e = TQStyle::PE_ArrowLeft; break;
		case Qt::RightArrow: e = TQStyle::PE_ArrowRight; break;
		case Qt::UpArrow: e = TQStyle::PE_ArrowUp; break;
		case Qt::DownArrow: e = TQStyle::PE_ArrowDown; break;
	}
	int flags = TQStyle::Style_Enabled;
	if ( isDown() )
		flags |= TQStyle::Style_Down;
	style().tqdrawPrimitive( e, p, TQRect( TQPoint( x, y ), TQSize( arrowSize, arrowSize ) ),
			       colorGroup(), flags );
}

void KArrowButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "karrowbutton.moc"
