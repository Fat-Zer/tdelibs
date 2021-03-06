/*
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
 *
 */
#ifndef render_applet_h
#define render_applet_h

#include "rendering/render_replaced.h"
#include "html/html_objectimpl.h"

#include <tqwidget.h>
#include <tqmap.h>

class TDEHTMLView;

namespace DOM {
    class HTMLElementImpl;
}

namespace tdehtml {

class RenderApplet : public RenderWidget
{
public:
    RenderApplet(DOM::HTMLElementImpl* node, const TQMap<TQString, TQString> &args);
    virtual ~RenderApplet();

    virtual const char *renderName() const { return "RenderApplet"; }

    virtual void layout();
    virtual short intrinsicWidth() const;
    virtual int intrinsicHeight() const;
    virtual bool isApplet() const { return true; }

    DOM::HTMLElementImpl *element() const
    { return static_cast<DOM::HTMLElementImpl*>(RenderObject::element()); }

private:
    void processArguments( const TQMap<TQString, TQString> &args );
};

}
#endif
