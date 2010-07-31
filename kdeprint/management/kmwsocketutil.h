/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef	KMWSOCKETUTIL_H
#define	KMWSOCKETUTIL_H

#include <tqstring.h>
#include <tqptrlist.h>
#include <kdialogbase.h>

struct SocketInfo
{
	QString	IP;
	QString	Name;
	int	Port;
};

class QProgressBar;
class QLineEdit;
class QComboBox;
class KMWSocketUtil;

class SocketConfig : public KDialogBase
{
	friend class KMWSocketUtil;
	Q_OBJECT
public:
	SocketConfig(KMWSocketUtil *util, TQWidget *parent = 0, const char *name = 0);
	~SocketConfig();

protected slots:
	void slotOk();

private:
	QLineEdit	*mask_, *tout_;
	QComboBox	*port_;
};

class KMWSocketUtil
{
	friend class SocketConfig;

public:
	KMWSocketUtil();

	bool checkPrinter(const TQString& host, int port, TQString* hostname = 0);

	const TQPtrList<SocketInfo>* printerList() { return &printerlist_; }
	bool scanNetwork(TQProgressBar *bar = 0);
	void configureScan(TQWidget *parent = 0);
	void setDefaultPort(int p) { port_ = p; }

private:
	TQPtrList<SocketInfo>	printerlist_;
	QString			root_;
	int			port_;
	int			timeout_;	// in milliseconds
};

#endif
