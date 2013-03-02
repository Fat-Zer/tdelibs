/* This file is part of the KDE libraries
    Copyright (C) 2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KURLBAR_H
#define KURLBAR_H

#include <tqevent.h>
#include <tqframe.h>
#include <tqtooltip.h>

#include <kdialogbase.h>
#include <kicontheme.h>
#include <tdelistbox.h>
#include <kurl.h>

class TDEConfig;
class KURLBar;

/**
 * An item to be used in KURLBar / KURLBarListBox. All the properties
 * (url, icon, description, tooltip) can be changed dynamically.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @see KURLBar
 * @see KURLBarListBox
 */
class TDEIO_EXPORT KURLBarItem : public TQListBoxPixmap
{
public:
    /**
     * Creates a KURLBarItem to be used in the @p parent KURLBar. You need
     * to insert the item into the listbox manually, if you don't use
     * KURLBar::insertItem().
     *
     * If description is empty, it will try to use the filename/directory
     * of @p url, which will be shown as text of the item.
     * @p url will be used as tooltip, unless you set a different tip with
     * setToolTip().
     * @p persistent specifies whether this item is a persistent item or a
     * dynamic item, that is not saved with KURLBar::writeConfig().
     * @since 3.2
     */
    KURLBarItem( KURLBar *parent, const KURL& url, bool persistent,
                 const TQString& description = TQString::null,
                 const TQString& icon = TQString::null,
                 TDEIcon::Group group = TDEIcon::Panel );

    /**
     * Creates a persistent KURLBarItem to be used in the @p parent KURLBar. You need
     * to insert the item into the listbox manually, if you don't use
     * KURLBar::insertItem().
     *
     * If description is empty, it will try to use the filename/directory
     * of @p url, which will be shown as text of the item.
     * @p url will be used as tooltip, unless you set a different tip with
     * setToolTip().
     * @p persistent specifies whether this item is a persistent item or a
     * dynamic item, that is not saved with KURLBar::writeConfig().
     */
    KURLBarItem( KURLBar *parent, const KURL& url,
                 const TQString& description = TQString::null,
                 const TQString& icon = TQString::null,
                 TDEIcon::Group group = TDEIcon::Panel );

    /**
     * Destroys the item
     */
    ~KURLBarItem();

    /**
     * Sets @p url for this item. Also updates the visible text to the
     * filename/directory of the url, if no description is set.
     * @see url
     */
    void setURL( const KURL& url );
    /**
     * @p sets the icon for this item. See TDEIconLoader for a description
     * of the icon groups.
     * @see icon
     */
    void setIcon( const TQString& icon, TDEIcon::Group group = TDEIcon::Panel );
    /**
     * Sets the description of this item that will be shown as item-text.
     * @see description
     */
    void setDescription( const TQString& desc );
    /**
     * Sets a tooltip to be used for this item.
     * @see toolTip
     */
    void setToolTip( const TQString& tip );

    /**
     * returns the preferred size of this item
     * @since 3.1
     */
    TQSize sizeHint() const;

    /**
     * returns the width of this item.
     */
    virtual int width( const TQListBox * ) const;
    /**
     * returns the height of this item.
     */
    virtual int height( const TQListBox * ) const;

    /**
     * returns the url of this item.
     * @see setURL
     */
    const KURL& url() const                     { return m_url; }
    /**
     * returns the description of this item.
     * @see setDescription
     */
    const TQString& description() const          { return m_description; }
    /**
     * returns the icon of this item.
     * @see setIcon
     */
    const TQString& icon() const                 { return m_icon; }
    /**
     * returns the tooltip of this item.
     * @see setToolTip
     */
    TQString toolTip() const;
    /**
     * returns the icon-group of this item (determines icon-effects).
     * @see setIcon
     */
    TDEIcon::Group iconGroup() const              { return m_group; }
    /**
     * returns the pixmap  of this item.
     */
    virtual const TQPixmap * pixmap() const      { return &m_pixmap; }

    /**
     * Makes this item a local or global one. This has only an effect
     * on persistent items of course.
     * @see isPersistent
     * @see applicationLocal
     */
    void setApplicationLocal( bool local );

    /**
     * returns whether this is a global item or a local one. KURLBar
     * can differentiate between global and local items (only for the current
     * application) for easy extensiblity.
     * @see setApplicationLocal
     */
    bool applicationLocal() const               { return m_appLocal; }

    /**
     * returns whether this item is persistent (via KURLBar::writeConfig()
     * and KURLBar::readConfig()) or not.
     * @since 3.2
     */
    bool isPersistent() const;

protected:
    virtual void paint( TQPainter *p );

private:
    int iconSize() const;
    void init( const TQString& icon, TDEIcon::Group group,
               const TQString& description, bool persistent );

    KURL m_url;
    TQString m_description;
    TQString m_icon;
    TQString m_toolTip;
    TQPixmap m_pixmap;
    TDEIcon::Group m_group;
    KURLBar *m_parent;
    bool m_appLocal :1;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KURLBarItemPrivate;
    KURLBarItemPrivate *d;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class KURLBarListBox;

/**
 * KURLBar is a widget that displays icons together with a description. They
 * can be arranged either horizontally or vertically. Clicking on an item
 * will cause the activated() signal to be emitted. The user can edit
 * existing items by choosing "Edit entry" in the contextmenu. He can also
 * remove or add new entries (via drag&drop or the context menu).
 *
 * KURLBar offers the methods readConfig() and writeConfig() to
 * read and write the configuration of all the entries. It can differentiate
 * between global and local entries -- global entries will be saved in the
 * global configuration (kdeglobals), while local entries will be saved in
 * your application's TDEConfig object.
 *
 * Due to the configurability, you usually only insert some default entries
 * once and then solely use the read and writeConfig methods to preserve the
 * user's configuration.
 *
 * The widget has a "current" item, that is visualized to differentiate it
 * from others.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @short A URL-bar widget, as used in the KFileDialog
 */
class TDEIO_EXPORT KURLBar : public TQFrame
{
    Q_OBJECT

public:
    /**
     * Constructs a KURLBar. Set @p useGlobalItems to true if you want to
     * allow global/local item separation.
     */
    KURLBar( bool useGlobalItems,
             TQWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    /**
     * Destroys the KURLBar.
     */
    ~KURLBar();

    /**
     * Inserts a new item into the KURLBar and returns the created
     * KURLBarItem.
     *
     * @p url the url of the item
     * @p description the description of the item (shown in the view)
     * @p applicationLocal whether this should be a global or a local item
     * @p icon an icon -- if empty, the default icon for the url will be used
     * @p group the icon-group for using icon-effects
     */
    virtual KURLBarItem * insertItem( const KURL& url,
                                      const TQString& description,
                                      bool applicationLocal = true,
                                      const TQString& icon = TQString::null,
                                      TDEIcon::Group group = TDEIcon::Panel );
    /**
     * Inserts a new dynamic item into the KURLBar and returns the created
     * KURLBarItem.
     *
     * @p url the url of the item
     * @p description the description of the item (shown in the view)
     * @p icon an icon -- if empty, the default icon for the url will be used
     * @p group the icon-group for using icon-effects
     * @since 3.2
     */
    virtual KURLBarItem * insertDynamicItem( const KURL& url,
                                             const TQString& description,
                                             const TQString& icon = TQString::null,
                                             TDEIcon::Group group = TDEIcon::Panel );
    /**
     * The items can be arranged either vertically in one column or
     * horizontally in one row.
     * @see orientation
     */
    virtual void setOrientation( Qt::Orientation orient );
    /**
     * @returns the current orientation mode.
     * @see setOrientation
     */
    Orientation orientation() const;

    /**
     * Allows to set a custom KURLBarListBox.
     * Note: The previous listbox will be deleted. Items of the previous
     * listbox will not be moved to the new box.
     * @see listBox
     */
    virtual void setListBox( KURLBarListBox * );
    /**
     * @returns the KURLBarListBox that is used.
     * @see setListBox
     */
    KURLBarListBox *listBox() const { return m_listBox; }

    /**
     * Sets the default iconsize to be used for items inserted with
     * insertItem. By default TDEIcon::SizeMedium.
     * @see iconsize
     */
    virtual void setIconSize( int size );
    /**
     * @returns the default iconsize used for items inserted with
     * insertItem. By default TDEIcon::SizeMedium
     * @see setIconSize
     */
    int iconSize() const { return m_iconSize; }

    /**
     * Clears the view, removes all items.
     */
    virtual void clear();

    /**
     * @returns a proper sizehint, depending on the orientation and the number
     * of items available.
     */
    virtual TQSize sizeHint() const;

    /**
     * @returns a proper minimum size (reimplemented)
     */
    virtual TQSize minimumSizeHint() const;

    /**
     * Call this method to read a saved configuration from @p config,
     * inside the group @p itemGroup. All items in there will be restored.
     * The reading of every item is delegated to the readItem() method.
     */
    virtual void readConfig(  TDEConfig *config, const TQString& itemGroup );
    /**
     * Call this method to save the current configuration into @p config,
     * inside the group @p iconGroup. The writeItem() method is used
     * to save each item.
     */
    virtual void writeConfig( TDEConfig *config, const TQString& itemGroup );

    /**
     * Called from readConfig() to read the i'th from @p config.
     * After reading a KURLBarItem is created and initialized with the read
     * values (as well as the given @p applicationLocal).
     */
    virtual void readItem( int i, TDEConfig *config, bool applicationLocal );
    /**
     * Called from writeConfig() to save the KURLBarItem @p item as the
     * i'th entry in the config-object.
     * @p global tell whether it should be saved in the global configuration
     * or not (using TDEConfig::writeEntry( key, value, true, global ) ).
     */
    virtual void writeItem( KURLBarItem *item, int i, TDEConfig *, bool global );

    /**
     * @returns the current KURLBarItem, or 0L if none.
     * @see setCurrentItem
     * @see currentURL
     */
    KURLBarItem * currentItem() const;
    /**
     * @returns the url of the current item or an invalid url, if there is
     * no current item.
     * @see currentItem
     * @see setCurrentItem
     */
    KURL currentURL() const;

    /**
     * @returns true when the urlbar was modified by the user (e.g. by
     * editing/adding/removing one or more entries). Will be reset to false
     * after calling writeConfig().
     */
    bool isModified() const             { return m_isModified; }

    /**
     * @returns true when the urlbar may not be modified by the user
     */
    bool isImmutable() const             { return m_isImmutable; }

    /**
     * @returns true if the bar is in vertical mode.
     */
    bool isVertical() const { return orientation() == Qt::Vertical; }

public slots:
    /**
     * Makes the item with the url @p url the current item. Does nothing
     * if no item with that url is available.
     * @see currentItem
     * @see currentURL
     */
    virtual void setCurrentItem( const KURL& url );

signals:
    /**
     * This signal is emitted when the user activated an item, e.g., by
     * clicking on it.
     */
    void activated( const KURL& url );

protected:
    /**
     * Pops up a KURLBarItemDialog to let the user add a new item.
     * Uses editItem() to do the job.
     * @returns false if the user aborted the dialog and no item is added.
     */
    virtual bool addNewItem();
    /**
     * Pops up a KURLBarItemDialog to let the user edit the properties
     * of @p item. Invoked e.g. by addNewItem(), when the user drops
     * a url  onto the bar or from the contextmenu.
     * @returns false if the user aborted the dialog and @p item is not
     * changed.
     */
    virtual bool editItem( KURLBarItem *item );

    virtual void resizeEvent( TQResizeEvent * );

    virtual void paletteChange( const TQPalette & );

    /**
     * The currently active item.
     */
    KURLBarItem * m_activeItem;
    /**
     * Whether we support global entries or just local ones.
     */
    bool m_useGlobal  :1;

    /**
     * Whether the urlbar was modified by the user (e.g. by
     * editing/adding/removing an item).
     */
    bool m_isModified :1;

    /**
     * Whether the urlbar may be modified by the user.
     * If immutable is true, the urlbar can not be modified.
     */
    bool m_isImmutable :1;

protected slots:
    /**
     * Reimplemented to show a contextmenu, allowing the user to add, edit
     * or remove items, or change the iconsize.
     */
    virtual void slotContextMenuRequested( TQListBoxItem *, const TQPoint& pos );
    /**
     * Called when an item has been selected. Emits the activated()
     * signal.
     */
    virtual void slotSelected( TQListBoxItem * );

    /**
     * Called when a url was dropped onto the bar to show a
     * KURLBarItemDialog.
     */
    virtual void slotDropped( TQDropEvent * );

private slots:
    void slotSelected( int button, TQListBoxItem * );

private:
    KURLBarListBox *m_listBox;
    int m_iconSize;


protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KURLBarPrivate;
    KURLBarPrivate *d;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class TQDragObject;
class KURLBarToolTip;

/**
 * This is the listbox used in KURLBar. It is a subclass of TDEListBox to support
 * drag & drop and to set up the row / column mode.
 *
 * The widget has just one row or one column, depending on orientation().
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEIO_EXPORT KURLBarListBox : public TDEListBox
{
    Q_OBJECT

public:
    /**
     * Constructs a KURLBarListBox.
     */
    KURLBarListBox( TQWidget *parent = 0, const char *name = 0 );
    /**
     * Destroys the box.
     */
    ~KURLBarListBox();

    /**
     * Sets the orientation of the widget. Horizontal means, all items are
     * arranged in one row. Vertical means, all items are arranged in one
     * column.
     * @see orientation
     */
    virtual void setOrientation( Qt::Orientation orient );
    /**
     * @returns the current orientation.
     * @see setOrientation
     */
    Qt::Orientation orientation() const { return m_orientation; }

    bool isVertical() const { return m_orientation == Qt::Vertical; }

signals:
    /**
     * Emitted when a drop-event happened.
     */
    void dropped( TQDropEvent *e );

protected:
    /**
     * @returns a suitable TQDragObject when an item is dragged.
     */
    virtual TQDragObject * dragObject();

    virtual void contentsDragEnterEvent( TQDragEnterEvent * );
    virtual void contentsDropEvent( TQDropEvent * );
    virtual void contextMenuEvent( TQContextMenuEvent * );
    virtual void paintEvent( TQPaintEvent* );

private:
    Qt::Orientation m_orientation;
    KURLBarToolTip *m_toolTip;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KURLBarListBoxPrivate;
    KURLBarListBoxPrivate *d;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class TQCheckBox;
class TDEIconButton;
class KLineEdit;
class KURLRequester;

/**
 * A dialog that allows editing entries of a KURLBar ( KURLBarItem).
 * The dialog offers to configure a given url, description and icon.
 * See the class-method getInformation() for easy usage.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEIO_EXPORT KURLBarItemDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * A convenience method to show the dialog and retrieve all the
     * properties via the given parameters. The parameters are used to
     * initialize the dialog and then return the user-configured values.
     *
     * See the KURLBarItem constructor for the parameter description.
     */
    static bool getInformation( bool allowGlobal, KURL& url,
                                TQString& description, TQString& icon,
                                bool& appLocal, int iconSize,
                                TQWidget *parent = 0 );

    /**
     * Constructs a KURLBarItemDialog.
     *
     * @p allowGlobal if you set this to true, the dialog will have a checkbox
     *                for the user to decide if he wants the entry to be
     *                available globally or just for the current application.
     * @p url the url of the item
     * @p description a short, translated description of the item
     * @p icon an icon for the item
     * @p appLocal tells whether the item should be local for this application
     *             or be available globally
     * @p iconSize determines the size of the icon that is shown/selectable
     * @p parent the parent-widget for the dialog
     *
     * If you leave the icon empty, the default icon for the given url will be
     * used (KMimeType::pixmapForURL()).
     */
    KURLBarItemDialog( bool allowGlobal, const KURL& url,
                       const TQString& description, TQString icon,
                       bool appLocal = true,
                       int iconSize = TDEIcon::SizeMedium,
                       TQWidget *parent = 0, const char *name = 0 );
    /**
     * Destroys the dialog.
     */
    ~KURLBarItemDialog();

    /**
     * @returns the configured url
     */
    KURL url() const;

    /**
     * @returns the configured description
     */
    TQString description() const;

    /**
     * @returns the configured icon
     */
    TQString icon() const;

    /**
     * @returns whether the item should be local to the application or global.
     * If allowGlobal was set to false in the constructor, this will always
     * return true.
     */
    bool applicationLocal() const;

protected:
    /**
     * The KURLRequester used for editing the url
     */
    KURLRequester * m_urlEdit;
    /**
     * The KLineEdit used for editing the description
     */
    KLineEdit     * m_edit;
    /**
     * The TDEIconButton to configure the icon
     */
    TDEIconButton   * m_iconButton;
    /**
     * The TQCheckBox to modify the local/global setting
     */
    TQCheckBox     * m_appLocal;

public slots:
    void urlChanged(const TQString & );

private:
    class KURLBarItemDialogPrivate;
    KURLBarItemDialogPrivate *d;
};


#endif // KURLBAR_H
