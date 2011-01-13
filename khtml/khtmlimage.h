/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __khtmlimage_h__
#define __khtmlimage_h__

#include "khtml_part.h"
#include <kparts/factory.h>
#include <kparts/browserextension.h>

#include "misc/loader_client.h"

class KHTMLPart;
class KInstance;

namespace khtml
{
    class CachedImage;
}

/**
 * @internal
 */
class KHTMLImageFactory : public KParts::Factory
{
    Q_OBJECT
public:
    KHTMLImageFactory();
    virtual ~KHTMLImageFactory();

    virtual KParts::Part *createPartObject( TQWidget *tqparentWidget, const char *widgetName,
                                            TQObject *parent, const char *name,
                                            const char *className, const TQStringList &args );

    static KInstance *instance() { return s_instance; }

private:
    static KInstance *s_instance;
};

/**
 * @internal
 */
class KHTMLImage : public KParts::ReadOnlyPart, public khtml::CachedObjectClient
{
    Q_OBJECT
public:
    KHTMLImage( TQWidget *tqparentWidget, const char *widgetName,
                TQObject *parent, const char *name, KHTMLPart::GUIProfile prof );
    virtual ~KHTMLImage();

    virtual bool openFile() { return true; } // grmbl, should be non-pure in part.h, IMHO

    virtual bool openURL( const KURL &url );

    virtual bool closeURL();

    KHTMLPart *doc() const { return m_khtml; }

    virtual void notifyFinished( khtml::CachedObject *o );

protected:
    virtual void guiActivateEvent( KParts::GUIActivateEvent *e );
    virtual bool eventFilter( TQObject *filterTarget, TQEvent *e );

private slots:
    void restoreScrollPosition();
//    void slotImageJobFinished( KIO::Job *job );

//    void updateWindowCaption();

private:
    void disposeImage();

    TQGuardedPtr<KHTMLPart> m_khtml;
    KParts::BrowserExtension *m_ext;
    TQString m_mimeType;
    khtml::CachedImage *m_image;
    int m_xOffset, m_yOffset;
};

/**
 * @internal
 */
class KHTMLImageBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
public:
    KHTMLImageBrowserExtension( KHTMLImage *parent, const char *name = 0 );

    virtual int xOffset();
    virtual int yOffset();

protected slots:
    void print();
    void reparseConfiguration();
    void disableScrolling();

private:
    KHTMLImage *m_imgPart;
};

#endif
