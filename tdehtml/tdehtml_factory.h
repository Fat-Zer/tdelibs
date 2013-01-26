/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
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
#ifndef __tdehtml_factory_h__
#define __tdehtml_factory_h__

#include <tdeparts/factory.h>
#include <tdeparts/historyprovider.h>
#include <tqptrlist.h>
#include <kurl.h>

class TDEInstance;
class TDEAboutData;
class HistoryProvider;
class KHTMLSettings;
class KHTMLPart;

namespace DOM
{
  class DocumentImpl;
}

class KDE_EXPORT KHTMLFactory : public KParts::Factory
{
  Q_OBJECT
  friend class DOM::DocumentImpl;
  friend class KHTMLViewPrivate;
public:
  KHTMLFactory( bool clone = false );
  virtual ~KHTMLFactory();

  virtual KParts::Part *createPartObject( TQWidget *parentWidget, const char *widgetName, TQObject *parent, const char *name, const char *className, const TQStringList &args );

  static void registerPart( KHTMLPart *part );
  static void deregisterPart( KHTMLPart *part );

  static TQPtrList<KHTMLPart> *partList() { return s_parts; }

  static TDEInstance *instance();

  static KHTMLSettings *defaultHTMLSettings();

  // list of visited URLs
  static KParts::HistoryProvider *vLinks() {
    return KParts::HistoryProvider::self();
  }

protected:
  static void ref();
  static void deref();
private:
  static unsigned long s_refcnt;
  static KHTMLFactory *s_self;
  static TDEInstance *s_instance;
  static TDEAboutData *s_about;
  static KHTMLSettings *s_settings;
  static TQPtrList<KHTMLPart> *s_parts;
};

#endif
