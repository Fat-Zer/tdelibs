/* This file is part of the KDE project
  Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

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

#include "kmdifocuslist.h"
#include "kmdifocuslist.moc"
#include <tqobjectlist.h>
#include <kdebug.h>

KMdiFocusList::KMdiFocusList( TQObject *parent ) : TQObject( parent )
{}

KMdiFocusList::~KMdiFocusList()
{}

void KMdiFocusList::addWidgetTree( TQWidget* w )
{
	//this method should never be called twice on the same hierarchy
	m_list.insert( w, w->focusPolicy() );
	w->setFocusPolicy( TQ_ClickFocus );
	kdDebug( 760 ) << "KMdiFocusList::addWidgetTree: adding toplevel" << endl;
	connect( w, TQT_SIGNAL( destroyed( TQObject * ) ), this, TQT_SLOT( objectHasBeenDestroyed( TQObject* ) ) );
	TQObjectList *l = w->queryList( TQWIDGET_OBJECT_NAME_STRING );
	TQObjectListIt it( *l );
	TQObject *obj;
	while ( ( obj = it.current() ) != 0 )
	{
		TQWidget * wid = ( TQWidget* ) obj;
		m_list.insert( wid, wid->focusPolicy() );
		wid->setFocusPolicy( TQ_ClickFocus );
		kdDebug( 760 ) << "KMdiFocusList::addWidgetTree: adding widget" << endl;
		connect( wid, TQT_SIGNAL( destroyed( TQObject * ) ), this, TQT_SLOT( objectHasBeenDestroyed( TQObject* ) ) );
		++it;
	}
	delete l;
}

void KMdiFocusList::restore()
{
	for ( TQMap<TQWidget*, TQ_FocusPolicy>::const_iterator it = m_list.constBegin();it != m_list.constEnd();++it )
	{
		it.key() ->setFocusPolicy( it.data() );
	}
	m_list.clear();
}


void KMdiFocusList::objectHasBeenDestroyed( TQObject * o )
{
	if ( !o || !o->isWidgetType() )
		return ;
	TQWidget *w = ( TQWidget* ) o;
	m_list.remove( w );
}

// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
