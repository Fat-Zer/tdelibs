// -*- c++ -*-

/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Koos Vriezen <koos.vriezen@xs4all.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef KJAVAAPPLETVIEWER_H
#define KJAVAAPPLETVIEWER_H

#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kparts/statusbarextension.h>
#include <kparts/factory.h>
#include <kdialogbase.h>
#include <kurl.h>
#include <tqobject.h>
#include <tqstringlist.h>
#include <tqguardedptr.h>

#include "kjavaappletwidget.h"

class TQTable;
class TQLabel;
class KJavaProcess;
class KJavaAppletViewer;
class KAboutData;
class KInstance;
class KConfig;
class CoverWidget;

class KJavaAppletViewerBrowserExtension : public KParts::BrowserExtension {
    Q_OBJECT
public:
    KJavaAppletViewerBrowserExtension (KJavaAppletViewer *parent);
    void urlChanged (const TQString & url);
    void setLoadingProgress (int percentage);

    void setURLArgs (const KParts::URLArgs & args);
    void saveState (TQDataStream & stream);
    void restoreState (TQDataStream & stream);
public slots:
    void showDocument (const TQString & doc, const TQString & frame);
};

class KJavaAppletViewerLiveConnectExtension : public KParts::LiveConnectExtension {
    Q_OBJECT
public:
    KJavaAppletViewerLiveConnectExtension(KJavaAppletViewer * parent);

    bool get (const unsigned long objid, const TQString & field, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, TQString & value);
    bool put(const unsigned long, const TQString & field, const TQString & value);
    bool call (const unsigned long , const TQString & func, const TQStringList & args, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, TQString & value);
    void unregister (const unsigned long objid);

    int jsSessions () const { return m_jssessions; }
public slots:
    void jsEvent (const TQStringList & args);
signals:
    virtual void partEvent (const unsigned long objid, const TQString & event, const KParts::LiveConnectExtension::ArgList & args);

private:
    KJavaAppletViewer * m_viewer;
    static int m_jssessions;
};

class KJavaAppletViewer : public KParts::ReadOnlyPart {
    Q_OBJECT
public: 
    KJavaAppletViewer (TQWidget * wparent, const char * wname,
              TQObject * parent, const char * name, const TQStringList &args);
    ~KJavaAppletViewer ();
    CoverWidget * view () const { return m_view; }
    static KAboutData* createAboutData ();

    KJavaAppletViewerBrowserExtension * browserextension() const
        { return m_browserextension; }
    KParts::LiveConnectExtension * liveConnectExtension () const
        { return m_liveconnect; }

    bool eventFilter (TQObject *o, TQEvent *e);

    bool appletAlive () const;
public slots:
    virtual bool openURL (const KURL & url);
    virtual bool closeURL ();
    void appletLoaded ();
    void infoMessage (const TQString &);
protected:
    bool openFile();
private slots:
    void delayedCreateTimeOut ();
private:
    TQGuardedPtr <CoverWidget> m_view;
    KConfig * m_config;
    KJavaProcess * process;
    KJavaAppletViewerBrowserExtension * m_browserextension;
    KJavaAppletViewerLiveConnectExtension * m_liveconnect;
    KParts::StatusBarExtension * m_statusbar;
    TQGuardedPtr <TQLabel> m_statusbar_icon;
    TQString baseurl;
    bool m_closed;
};

class KJavaAppletViewerFactory : public KParts::Factory {
    Q_OBJECT
public:
    KJavaAppletViewerFactory ();
    virtual ~KJavaAppletViewerFactory ();
    virtual KParts::Part *createPartObject 
        (TQWidget *wparent, const char *wname,
         TQObject *parent, const char *name,
         const char *className, const TQStringList &args);
    static KInstance * instance () { return s_instance; }
private:
    static KInstance * s_instance;
};

class AppletParameterDialog : public KDialogBase {
    Q_OBJECT
public:
    AppletParameterDialog (KJavaAppletWidget * parent);
protected slots:
    void slotClose ();
private:
    KJavaAppletWidget * m_appletWidget;
    TQTable * table;
};

#endif
