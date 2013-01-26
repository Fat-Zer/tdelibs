/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 Simon Hausmann <hausmann@kde.org>
 *               2001-2003 George Staikos <staikos@kde.org>
 *               2001-2003 Laurent Montel <montel@kde.org>
 *               2001-2003 Dirk Mueller <mueller@kde.org>
 *               2001-2003 Waldo Bastian <bastian@kde.org>
 *               2001-2003 David Faure <faure@kde.org>
 *               2001-2003 Daniel Naber <dnaber@kde.org>
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

#ifndef __tdehtml_ext_h__
#define __tdehtml_ext_h__

#include "tdehtml_part.h"

#include <tqguardedptr.h>

#include <kaction.h>
#include <kio/global.h>

/**
 * This is the BrowserExtension for a KHTMLPart document. Please see the KParts documentation for
 * more information about the BrowserExtension.
 */
class KHTMLPartBrowserExtension : public KParts::BrowserExtension
{
  Q_OBJECT
  friend class KHTMLPart;
  friend class KHTMLView;
public:
  KHTMLPartBrowserExtension( KHTMLPart *parent, const char *name = 0L );

  virtual int xOffset();
  virtual int yOffset();

  virtual void saveState( TQDataStream &stream );
  virtual void restoreState( TQDataStream &stream );

    // internal
    void editableWidgetFocused( TQWidget *widget );
    void editableWidgetBlurred( TQWidget *widget );

    void setExtensionProxy( KParts::BrowserExtension *proxyExtension );

public slots:
    void cut();
    void copy();
    void paste();
    void searchProvider();
    void openSelection();
    void reparseConfiguration();
    void print();
    void disableScrolling();

    // internal . updates the state of the cut/copt/paste action based
    // on whether data is available in the clipboard
    void updateEditActions();

private slots:
    // connected to a frame's browserextensions enableAction signal
    void extensionProxyActionEnabled( const char *action, bool enable );
    void extensionProxyEditableWidgetFocused();
    void extensionProxyEditableWidgetBlurred();

signals:
    void editableWidgetFocused();
    void editableWidgetBlurred();
private:
    void callExtensionProxyMethod( const char *method );

    KHTMLPart *m_part;
    TQGuardedPtr<TQWidget> m_editableFormWidget;
    TQGuardedPtr<KParts::BrowserExtension> m_extensionProxy;
    bool m_connectedToClipboard;
};

class KHTMLPartBrowserHostExtension : public KParts::BrowserHostExtension
{
public:
  KHTMLPartBrowserHostExtension( KHTMLPart *part );
  virtual ~KHTMLPartBrowserHostExtension();

  virtual TQStringList frameNames() const;

  virtual const TQPtrList<KParts::ReadOnlyPart> frames() const;

  virtual bool openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KHTMLPart *m_part;
};

/**
 * @internal
 * INTERNAL class. *NOT* part of the public API.
 */
class KHTMLPopupGUIClient : public TQObject, public KXMLGUIClient
{
  Q_OBJECT
public:
  KHTMLPopupGUIClient( KHTMLPart *tdehtml, const TQString &doc, const KURL &url );
  virtual ~KHTMLPopupGUIClient();

  static void saveURL( TQWidget *parent, const TQString &caption, const KURL &url,
                       const TQMap<TQString, TQString> &metaData = TDEIO::MetaData(),
                       const TQString &filter = TQString::null, long cacheId = 0,
                       const TQString &suggestedFilename = TQString::null );

  static void saveURL( const KURL &url, const KURL &destination,
                       const TQMap<TQString, TQString> &metaData = TDEIO::MetaData(),
                       long cacheId = 0 );
private slots:
  void slotSaveLinkAs();
  void slotSaveImageAs();
  void slotCopyLinkLocation();
  void slotSendImage();
  void slotStopAnimations();
  void slotCopyImageLocation();
  void slotCopyImage();
  void slotViewImage();
  void slotReloadFrame();
  void slotFrameInWindow();
  void slotFrameInTop();
  void slotFrameInTab();
  void slotBlockImage();
  void slotBlockHost();
  void slotBlockIFrame();

private:
  class KHTMLPopupGUIClientPrivate;
  KHTMLPopupGUIClientPrivate *d;
};

class KHTMLZoomFactorAction : public KAction
{
    Q_OBJECT
public:
    //BCI: remove in KDE 4
    KHTMLZoomFactorAction( KHTMLPart *part, bool direction, const TQString &text, const TQString &icon, const TQObject *receiver, const char *slot, TQObject *parent, const char *name );
    KHTMLZoomFactorAction( KHTMLPart *part, bool direction, const TQString &text,
            const TQString &icon, const KShortcut& cut, const TQObject *receiver,
            const char *slot, TQObject *parent, const char *name );
    virtual ~KHTMLZoomFactorAction();

    virtual int plug( TQWidget *widget, int index );

private slots:
    void slotActivated( int );
protected slots:
    void slotActivated() { KAction::slotActivated(); }
private:
    void init(KHTMLPart *part, bool direction);
private:
    TQPopupMenu *m_popup;
    bool m_direction;
    KHTMLPart *m_part;
};

#endif
