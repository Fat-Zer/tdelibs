/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef _KACTIVELABEL_H_
#define _KACTIVELABEL_H_

#include <tqtextbrowser.h>

#include <kdelibs_export.h>

// ### inherit KTextBrowser in KDE4

class KActiveLabelPrivate;
 /**
  * Label with support for selection and clickable links.
  * openLink() the actions that will be taken when the user
  * clicks on a link.
  *
  * @author Waldo Bastian (bastian@kde.org)
  * @version $Id$
  */
class TDEUI_EXPORT KActiveLabel : public TQTextBrowser
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * It connects the "linkClicked(const TQString &)" signal to the
     * "openLink(const TQString &)" slot. You will need to disconnect
     * this if you want to process linkClicked() yourself.
     */
    KActiveLabel(TQWidget * parent, const char * name = 0);

    /**
     * Constructor.
     *
     * It connects the "linkClicked(const TQString &)" signal to the
     * "openLink(const TQString &)" slot. You will need to disconnect
     * this if you want to process linkClicked() yourself.
     */
    KActiveLabel(const TQString & text, TQWidget * parent, const char * name = 0);

    TQSize tqminimumSizeHint() const;
    TQSize tqsizeHint() const;

public slots:
    /**
     * Opens @p link in the default browser.
     *
     * If @p link starts with the text "whatsthis:" a QWhatsThis
     * box will appear and then display the rest of the text. The WhatsThis
     * functionality is available since KDE 3.2.
     */
    virtual void openLink(const TQString & link);

private slots:
    void paletteChanged();
    void setSource( const TQString &) { }

private:
    void init();
protected:
    virtual void virtual_hook( int id, void* data );
    virtual void focusInEvent( TQFocusEvent* fe );
    virtual void focusOutEvent( TQFocusEvent* fe );
    virtual void keyPressEvent ( TQKeyEvent * e );
private:
    KActiveLabelPrivate *d;
};

#endif
