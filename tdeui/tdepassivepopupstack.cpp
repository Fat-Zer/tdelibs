/* This file is part of the TDE libraries
 * Copyright (C) 2011 - 2015 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.

 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "tdepassivepopupstack.h"

TDEPassivePopupStackContainer::TDEPassivePopupStackContainer(TQWidget *parent, const char *name) : TQWidget(parent, name) {
	mPopupList.clear();

	// Determine bottom of desktop
	TQPoint cursorPos = TQCursor::pos();
	TQRect r = TDEGlobalSettings::desktopGeometry(cursorPos);
	mTopOfStack = r.height();
	mRightOfStack = r.width();
}

TDEPassivePopupStackContainer::~TDEPassivePopupStackContainer() {
	//
}

KPassivePopup* TDEPassivePopupStackContainer::displayMessage(TQString title, TQString message, TQString icon, int x, int y) {
	TQPixmap px;
	TDEIconLoader* il = TDEGlobal::iconLoader();
	px = il->loadIcon(icon, TDEIcon::NoGroup);

	KPassivePopup *pop = new KPassivePopup(KPassivePopup::Boxed, this, "");
	pop->setAutoDelete(true);
	pop->setView(title, message, icon);
	pop->setTimeout(-1);
	TQPoint leftCorner(x, y);
	if (leftCorner.isNull()) {
		if (mPopupList.isEmpty()) {
			// Determine bottom of desktop
			TQPoint cursorPos = TQCursor::pos();
			TQRect r = TDEGlobalSettings::desktopGeometry(cursorPos);
			mTopOfStack = r.height();
			mRightOfStack = r.width();
		}
		TQSize popupSize = pop->sizeHint();
		mTopOfStack = mTopOfStack-popupSize.height();
		if (mTopOfStack < 0) mTopOfStack = 0;
		leftCorner.setX(mRightOfStack-popupSize.width());
		leftCorner.setY(mTopOfStack);
	}
	connect(pop, SIGNAL(hidden(KPassivePopup*)), this, SLOT(popupClosed(KPassivePopup*)));
	connect(pop, SIGNAL(clicked(TQPoint)), this, SLOT(popupClicked(TQPoint)));
	mPopupList.append(pop);
	pop->show(leftCorner);

	return pop;
}

void TDEPassivePopupStackContainer::processEvents() {
	tqApp->processEvents();
}

void TDEPassivePopupStackContainer::popupClosed(KPassivePopup* popup) {
	// Remove the popup from our list of popups
	mPopupList.remove(popup);

	if (mPopupList.isEmpty()) {
		// Determine bottom of desktop
		TQPoint cursorPos = TQCursor::pos();
		TQRect r = TDEGlobalSettings::desktopGeometry(cursorPos);
		mTopOfStack = r.height();
		mRightOfStack = r.width();
	}
}

void TDEPassivePopupStackContainer::popupClicked(TQPoint point) {
	emit(popupClicked(dynamic_cast<KPassivePopup*>(const_cast<TQObject*>(TQObject::sender())), point));
}

#include "tdepassivepopupstack.moc"