/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2000 Charles Samuels <charles@kde.org>
   Copyright (C) 2000 Peter Putzer <putzer@kde.org>

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
#ifndef KLISTVIEW_H
#define KLISTVIEW_H

#include <tqlistview.h>

#include <tqptrlist.h>
#include <kdelibs_export.h>

class QDragObject;
class KConfig;
class KLineEdit;
/**
 * This Widget extends the functionality of TQListView to honor the system
 * wide settings for Single Click/Double Click mode, AutoSelection and
 * ChangeCursorOverLink (TM).
 *
 * There is a new signal executed(). It gets connected to either
 * TQListView::clicked() or TQListView::doubleClicked() depending on the KDE
 * wide Single Click/Double Click settings. It is strongly recommended that
 * you use this signal instead of the above mentioned. This way you don�t
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the
 * TQListView::selectionChanged() signal.
 *
 * Drag-and-Drop is supported with the signal dropped(), just setAcceptDrops(true)
 * and connect it to a suitable slot.
 * To see where you are dropping, setDropVisualizer(true).
 * And also you'll need acceptDrag(TQDropEvent*)
 *
 * KListView is drag-enabled, too: to benefit from that you have to derive from it.
 * Reimplement dragObject() and (possibly) startDrag(),
 * and setDragEnabled(true).
 */
class KDEUI_EXPORT KListView : public QListView
{
  friend class KListViewItem;

  Q_OBJECT
  Q_ENUMS( SelectionModeExt )
  Q_PROPERTY( bool fullWidth READ fullWidth WRITE setFullWidth )
  Q_PROPERTY( bool itemsMovable READ itemsMovable WRITE setItemsMovable )
  Q_PROPERTY( bool itemsRenameable READ itemsRenameable WRITE setItemsRenameable )
  Q_PROPERTY( bool dragEnabled READ dragEnabled  WRITE  setDragEnabled )
  Q_PROPERTY( bool autoOpen READ autoOpen WRITE setAutoOpen )
  Q_PROPERTY( bool dropVisualizer READ dropVisualizer WRITE setDropVisualizer )
  Q_PROPERTY( int tooltipColumn READ tooltipColumn WRITE  setTooltipColumn )
  Q_PROPERTY( int dropVisualizerWidth READ dropVisualizerWidth WRITE  setDropVisualizerWidth )
  Q_PROPERTY( TQColor alternateBackground READ alternateBackground WRITE  setAlternateBackground )
  Q_PROPERTY( bool shadeSortColumn READ shadeSortColumn WRITE setShadeSortColumn )

  Q_OVERRIDE( SelectionModeExt selectionMode READ selectionModeExt WRITE setSelectionModeExt )

public:
  /**
   * Possible selection modes.
   *
   * The first four correspond directly to TQListView::SelectionMode, while
   * the FileManager selection mode is defined as follows:
   *   @li home: move to the first
   *   @li end: move to the last
   *   @li PgUp/PgDn: move one page up/down
   *   @li up/down: move one item up/down
   *   @li insert: toggle selection of current and move to the next
   *   @li space: toggle selection of the current
   *   @li CTRL+up: move to the previous item and toggle selection of this one
   *   @li CTRL+down: toggle selection of the current item and move to the next
   *   @li CTRL+end: toggle selection from (including) the current
   *   item to (including) the last item
   *   @li CTRL+home: toggle selection from (including) the current
   *   item to the (including) the first item
   *   @li CTRL+PgDn: toggle selection from (including) the current
   *   item to (excluding) the item one page down
   *   @li CTRL+PgUp: toggle selection from (excluding) the current
   *   item to (including) the item one page up
   *
   *    The combinations work the same with SHIFT instead of CTRL, except
   *    that if you start selecting something using SHIFT everything selected
   *    before will be deselected first.
   *
   *    Additionally the current item is always selected automatically when
   *    navigating using the keyboard, except other items were selected explicitly.
   *
   *   This way e.g. SHIFT+up/PgUp then SHIFT+down/PgDn leaves no item selected
   */
  enum SelectionModeExt {
	Single = TQListView::Single,
	Multi = TQListView::Multi,
	Extended = TQListView::Extended,
	NoSelection = TQListView::NoSelection,
	FileManager
  };

  /**
   * Constructor.
   *
   * The parameters @p parent and @p name are handled by
   * TQListView, as usual.
   */
  KListView (TQWidget *parent = 0, const char *name = 0);

  /**
   * Destructor.
   */
  virtual ~KListView();

 /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void setAcceptDrops (bool);

  /**
   * This function determines whether the given coordinates are within the
   * execute area. The execute area is the part of a TQListViewItem where mouse
   * clicks or double clicks respectively generate a executed() signal.
   * Depending on TQListView::allColumnsShowFocus() this is either the
   * whole item or only the first column.
   * @return true if point is inside execute area of an item, false in all
   * other cases including the case that it is over the viewport.
   */
  virtual bool isExecuteArea( const TQPoint& point );

  /**
   * Same thing, but from an x coordinate only. This only checks if x is in
   * the first column (if all columns don't show focus), without testing if
   * the y coordinate is over an item or not.
   */
  bool isExecuteArea( int x );

  /**
   * @return a list containing the currently selected items.
   *
   * @deprecated
   */
  TQPtrList<TQListViewItem> selectedItems() const;

  /**
   * @return a list containing the currently selected items.
   *
   * @param includeHiddenItems Set to true to return all items regardless of
   * visibility. Set to false to only return items that are isVisible().
   *
   * @return a list of all selected listview items
   *
   * @since 3.4
   */
  TQPtrList<TQListViewItem> selectedItems(bool includeHiddenItems) const; // ### BIC: KDE 4: use an implicitly shared class! (QValutList?) and merge with above, default to true

  /**
   * Arbitrarily move @p item to @p parent, positioned immediately after item @p after.
   * If after is 0, @p item is made the first child of @p parent.
   *
   * (Un-deprecated in kde-3.4)
   */
  void moveItem(TQListViewItem *item, TQListViewItem *parent, TQListViewItem *after);

  /**
   * @return the last item (not child!) of this listview.
   *
   * @see lastChild()
   */
  TQListViewItem *lastItem() const;

  /**
   * @return the last child of this listview.
   *
   * @see lastItem()
   */
  TQListViewItem* lastChild () const;

  /**
   * @return the lineedit used for inline renaming.
   * Use that to setup a KCompletion or TQValidator for the lineedit
   *
   * @since 3.2
   */
  KLineEdit* renameLineEdit() const;

  /**
   * @returns if it is legal to move items in the list view.  True by default.
   *
   * @see setDragEnabled()
   * @see setItemsMovable()
   */
  bool itemsMovable() const;

  /**
   * @return whether inplace-renaming has been enabled.  False by default.
   *
   * @see setItemsRenameable()
   */
  bool itemsRenameable() const;

  /**
   * @return whether dragging is enabled.  False by default.
   *
   * @see setDragEnabled()
   */
  bool dragEnabled() const;

  /**
   * @return true if AutoOpen is enabled (not implemented currently).
   *
   * @see setAutoOpen()
   */
  bool autoOpen() const;

  /**
   * @return true if @p column is renamable.
   *
   * @see setRenameable()
   */
  bool isRenameable (int column) const;

  /**
   * @return true if drawing of the drop-visualizer has been enabled.  True by default.
   *
   * @see setDropVisualizer()
   */
  bool dropVisualizer() const;

  /**
   * @return the column for which tooltips are displayed (or -1 if none set).
   *
   * @see setTooltipColumn()
   */
  int tooltipColumn() const;

  /**
   * For future expansions.
   *
   * Do not use.
   * @deprecated
   */
  bool createChildren() const KDE_DEPRECATED;

  /**
   * @return true if drawing of the drop-highlighter has been enabled.  False by default.
   *
   * @see setDropHighlighter()
   */
  bool dropHighlighter() const;

  /**
   * The dropVisualizerWidth defaults to 4.
   *
   * @see setDropVisualizerWidth()
   * @return the current width of the drop-visualizer.
   */
  int dropVisualizerWidth () const;

  /**
   * @return the "extended" selection mode of this listview.
   *
   * @see SelectionModeExt
   * @see setSelectionModeExt
   */
  SelectionModeExt selectionModeExt () const;

  /**
   * Returns the index of @p item within the item tree or -1 if
   * @p item doesn't exist in this list view. This function takes
   * all items into account not only the visible ones.
   */
  int itemIndex( const TQListViewItem *item ) const;

  /**
   * Returns the item of @p index within the item tree or 0 if
   * @p index doesn't exist in this list view. This function takes
   * all items into account not only the visible ones.
   */
  TQListViewItem* itemAtIndex(int index);

  /**
   * @deprecated
   * @see setFullWidth()
   */
  void setFullWidth() KDE_DEPRECATED;

  /**
   * Let the last column fit exactly all the available width.
   *
   * @see fullWidth()
   */
  void setFullWidth(bool fullWidth);

  /**
   * Returns whether the last column is set to fit the available width.
   *
   * @see setFullWidth()
   */
  bool fullWidth() const;

  /**
   * Reimplemented for full width support
   *
   * @see removeColumn()
   */
  virtual int addColumn(const TQString& label, int width = -1);
  /**
   * Reimplemented for full width support
   */
  virtual int addColumn(const TQIconSet& iconset, const TQString& label, int width = -1);
  /**
   * Reimplemented for full width support
   *
   * @see addColumn()
   */
  virtual void removeColumn(int index);

  /**
   * sets the alternate background background color.
   * This only has an effect if the items are KListViewItems
   *
   * @param c the color to use for every other item. Set to an invalid
   *        color to disable alternate colors.
   *
   * @see alternateBackground()
   **/
  void setAlternateBackground(const TQColor &c);
  /**
   * @return the alternate background color
   *
   * @see setAlternateBackground()
   */
  const TQColor &alternateBackground() const;

  /**
   * Saves the list view's layout (column widtsh, column order, sort column)
   * to a KConfig group
   *
   * @param config the KConfig object to write to
   * @param group the config group to use
   */
  void saveLayout(KConfig *config, const TQString &group) const;
  /**
   * Reads the list view's layout from a KConfig group as stored with
   * saveLayout
   *
   * @param config the KConfig object to read from
   * @param group the config group to use
   */
  void restoreLayout(KConfig *config, const TQString &group);
  /**
   * Reimplemented to remember the current sort column and order.
   * @param column is the column to be sorted, or -1 to sort in order of
   * insertion
   * @param ascending whether to sort ascending (or descending)
   */
  virtual void setSorting(int column, bool ascending = true);

  /**
   * @return the currently sorted column, or -1 if none is sorted
   */
  int columnSorted(void) const;

  /**
   * @return whether the current sort is ascending (or descending)
   */
  bool ascendingSort(void) const;

  /**
   * Reimplemented for internal reasons.
   */
  virtual void takeItem(TQListViewItem *i);

  /**
   * Set to true if the currently sorted column should be drawn shaded. Defaults to true
   * @param shadeSortColumn True if sort column should be shaded.
   *
   * @since 3.4
   */
  void setShadeSortColumn(bool shadeSortColumn);

  /**
   * See if the sort column should be drawn shaded
   * @return true if the sort column should be shaded
   *
   * @since 3.4
   */
  bool shadeSortColumn(void) const;
signals:

  /**
   * This signal is emitted whenever the user executes an listview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listview item.
   *
   * Note that you may not delete any TQListViewItem objects in slots
   * connected to this signal.
   */
  void executed( TQListViewItem *item );

  /**
   * This signal is emitted whenever the user executes an listview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listview item.
   * @param pos is the position where the user has clicked
   * @param c is the column into which the user clicked.
   *
   * Note that you may not delete any TQListViewItem objects in slots
   * connected to this signal.
   */
  void executed( TQListViewItem *item, const TQPoint &pos, int c );

  /**
   * This signal gets emitted whenever something acceptable is
   * dropped onto the listview.
   *
   * @param e is the drop event itself (it has already been accepted)
   * @param after is the item after which the drop occurred (or 0L, if
   * the drop was above all items)
   *
   * @see acceptDrop()
   */
  void dropped (TQDropEvent * e, TQListViewItem *after);

  /**
   * This signal gets emitted whenever something acceptable is
   * dropped onto the listview.
   *
   * This is an overloaded version of the above (provided to simplify
   * processing drops outside of the class).
   *
   * @param list is the listview
   * @param e is the drop event itself (it has already been accepted)
   * @param after is the item after which the drop occurred (or 0L, if
   * the drop was above all items
   */
  void dropped (KListView* list, TQDropEvent* e, TQListViewItem* after);

  /**
   * This signal gets emitted whenever something acceptable is
   * dropped onto the listview.
   *
   * This function also provides a parent, in the event that your listview
   * is a tree
   * @param list is the listview
   * @param e is the drop event itself (it has already been accepted)
   * @param parent the item that is to be the parent of the new item
   * @param after is the item after which the drop occurred (or 0L, if
   * the drop was above all items
   */
  void dropped (KListView* list, TQDropEvent* e, TQListViewItem* parent, TQListViewItem* after);

  /**
   * This signal gets emitted whenever something acceptable is
   * dropped onto the listview.
   *
   * This function also provides a parent, in the event that your listview
   * is a tree
   * @param e is the drop event itself (it has already been accepted)
   * @param parent the item that is to be the parent of the new item
   * @param after is the item after which the drop occurred (or 0L, if
   * the drop was above all items
   */
  void dropped (TQDropEvent* e, TQListViewItem* parent, TQListViewItem* after);

  /**
   * This signal is emitted when ever the user moves an item in the list via
   * DnD.
   * If more than one item is moved at the same time, this signal is only emitted
   * once.
   */
  void moved();

  /**
   * Connect to this signal if you want to do some preprocessing before
   * a move is made, for example, to disable sorting
   *
   * This is sent only once per each groups of moves.  That is, for each
   * drop that is a move this will be emitted once, before KListView calls
   * @see moveItem()
   */
  void aboutToMove();

  /**
   * This signal is emitted when ever the user moves an item in the list via
   * DnD.
   * If more than one item is moved at the same time, @p afterFirst and
   * @p afterNow will reflect what was true before the move.
   * This differs from moved(), so be careful. All the items will have been
   * moved before moved() is emitted, which is not true in this method. // FIXME
   * @param item the item that was moved
   * @param afterFirst the item that parameter item was in before the move, in the list
   * @param afterNow the item it's currently after.
   */
  void moved (TQListViewItem *item, TQListViewItem *afterFirst, TQListViewItem *afterNow);


  /**
   * This signal is emitted after all the items have been moved. It reports info for
   * each and every item moved, in order.  The first element in @p items associates
   * with the first of afterFirst and afterNow.
   */
  void moved(TQPtrList<TQListViewItem> &items, TQPtrList<TQListViewItem> &afterFirst, TQPtrList<TQListViewItem> &afterNow);

  /**
   * This signal gets emitted when an item is renamed via in-place renaming.
   *
   * @param item is the renamed item.
   * @param str is the new value of column @p col.
   * @param col is the renamed column.
   */
  void itemRenamed(TQListViewItem* item, const TQString &str, int col);

  /**
   * Same as above, but without the extra information.
   */
  void itemRenamed(TQListViewItem* item);

  /**
   * This signal is emitted when the shortcut key for popup-menus is pressed.
   *
   * Normally you should not use this, just connect a slot to signal
   * contextMenu (KListView*, TQListViewItem*, const TQPoint&) to correctly
   * handle showing context menus regardless of settings.
   *
   * @param list is this listview.
   * @param item is the currentItem() at the time the key was pressed. May be 0L.
   */
  void menuShortCutPressed (KListView* list, TQListViewItem* item);

  /**
   * This signal is emitted whenever a context-menu should be shown for item @p i.
   * It automatically adjusts for all settings involved (Menu key, showMenuOnPress/Click).
   *
   * @param l is this listview.
   * @param i is the item for which the menu should be shown. May be 0L.
   * @param p is the point at which the menu should be shown.
   */
  void contextMenu (KListView* l, TQListViewItem* i, const TQPoint& p);

  void itemAdded(TQListViewItem *item);
  void itemRemoved(TQListViewItem *item);

public slots:
  /**
   * Rename column @p c of @p item.
   */
  virtual void rename(TQListViewItem *item, int c);

  /**
   * By default, if you called setItemsRenameable(true),
   * only the first column is renameable.
   * Use this function to enable the feature on other columns.
   *
   * If you want more intelligent (dynamic) selection,
   * you'll have to derive from KListView,
   * and override rename() and call only call it
   * if you want the item to be renamed.
   */
  void setRenameable (int column, bool yesno=true);

  /**
   * Set whether items in the list view can be moved.
   * It is enabled by default.
   *
   * @see itemsMovable()
   */
  virtual void setItemsMovable(bool b);

  /**
   * Enables inplace-renaming of items.
   * It is disabled by default.
   *
   * @see itemsRenameable()
   * @see setRenameable()
   */
  virtual void setItemsRenameable(bool b);

  /**
   * Enable/Disable the dragging of items.
   * It is disabled by default.
   */
  virtual void setDragEnabled(bool b);

  /**
   * Enable/Disable AutoOpen (not implemented currently).
   */
  virtual void setAutoOpen(bool b);

  /**
   * Enable/Disable the drawing of a drop-visualizer
   * (a bar that shows where a dropped item would be inserted).
   * It is enabled by default, if dragging is enabled
   */
  virtual void setDropVisualizer(bool b);

  /**
   * Set the width of the (default) drop-visualizer.
   * If you don't call this method, the width is set to 4.
   */
  void setDropVisualizerWidth (int w);

  /**
   * Set which column should be used for automatic tooltips.
   *
   * @param column is the column for which tooltips will be shown.
   * Set -1 to disable this feature.
   */
  virtual void setTooltipColumn(int column);

  /**
   * Enable/Disable the drawing of a drop-highlighter
   * (a rectangle around the item under the mouse cursor).
   * It is disabled by default.
   */
  virtual void setDropHighlighter(bool b);

  /**
   * For future expansions.
   *
   * Do not use.
   * @deprecated
   */
  virtual void setCreateChildren(bool b) KDE_DEPRECATED;

  /**
   * Set the selection mode.
   *
   * A different name was chosen to avoid API-clashes with TQListView::setSelectionMode().
   */
  void setSelectionModeExt (SelectionModeExt mode);

  /**
   * Enable/disable tabbing between editable cells
   * @since 3.1
   */
  void setTabOrderedRenaming(bool b);

  /**
   * Returns whether tab ordered renaming is enabled
   * @since 3.1
   */
  bool tabOrderedRenaming() const;

protected:
  /**
   * Determine whether a drop on position @p p would count as
   * being above or below the TQRect @p rect.
   *
   * @param rect is the rectangle we examine.
   * @param p is the point located in the rectangle, p is assumed to be in
   * viewport coordinates.
   */
  inline bool below (const TQRect& rect, const TQPoint& p)
  {
	return (p.y() > (rect.top() + (rect.bottom() - rect.top())/2));
  }

  /**
   * An overloaded version of below(const TQRect&, const TQPoint&).
   *
   * It differs from the above only in what arguments it takes.
   *
   * @param i the item whose rect() is passed to the above function.
   * @param p is translated from contents coordinates to viewport coordinates
   * before being passed to the above function.
   */
  inline bool below (TQListViewItem* i, const TQPoint& p)
  {
	return below (itemRect(i), contentsToViewport(p));
  }

  /**
   * Reimplemented to reload the alternate background in palette changes.
   * @internal
   */
  virtual bool event( TQEvent * );

  /**
   * Emit signal executed.
   * @internal
   */
  void emitExecute( TQListViewItem *item, const TQPoint &pos, int c );

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
   virtual void focusInEvent(TQFocusEvent* fe);

   /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void focusOutEvent( TQFocusEvent *fe );

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void leaveEvent( TQEvent *e );

  /**
   * @return the tooltip for @p column of @p item.
   */
  virtual TQString tooltip(TQListViewItem* item, int column) const;

  /**
   * @return whether the tooltip for @p column of @p item shall be shown at point @p pos.
   */
  virtual bool showTooltip(TQListViewItem *item, const TQPoint &pos, int column) const;

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void contentsDragMoveEvent (TQDragMoveEvent *event);

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void contentsMousePressEvent( TQMouseEvent *e );

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void contentsMouseMoveEvent( TQMouseEvent *e );

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void contentsMouseDoubleClickEvent ( TQMouseEvent *e );

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void contentsDragLeaveEvent (TQDragLeaveEvent *event);

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void contentsMouseReleaseEvent (TQMouseEvent*);

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void contentsDropEvent (TQDropEvent*);

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void contentsDragEnterEvent (TQDragEnterEvent *);

  /**
   * @return a dragobject encoding the current selection.
   *
   * @see setDragEnabled()
   */
  virtual TQDragObject *dragObject();

  /**
   * @return true if the @p event provides some acceptable
   * format.
   * A common mistake is to forget the "const" in your reimplementation
   */
  virtual bool acceptDrag (TQDropEvent* event) const;

  /**
   * Paint the drag line. If painter is null, don't try to :)
   *
   * If after == 0 then the marker should be drawn at the top.
   *
   * @return the rectangle that you painted to.
   */
  virtual TQRect drawDropVisualizer (TQPainter *p, TQListViewItem *parent, TQListViewItem *after);

  /**
   * Paint the drag rectangle. If painter is null, don't try to :)
   *
   *
   * @return the rectangle that you painted to.
   */
  virtual TQRect drawItemHighlighter(TQPainter *painter, TQListViewItem *item);

  /**
   * This method calls dragObject() and starts the drag.
   *
   * Reimplement it to do fancy stuff like setting a pixmap or
   * using a non-default DragMode
   */
  virtual void startDrag();

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void keyPressEvent (TQKeyEvent*);

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void viewportPaintEvent(TQPaintEvent*);

  /**
   * In FileManager selection mode: explicitly activate the mode
   * in which the current item is automatically selected.
   */
  void activateAutomaticSelection();
  /**
   * In FileManager selection mode: explicitly deactivate the mode
   * in which the current item is automatically selected.
   */
  void deactivateAutomaticSelection();
  /**
   * In FileManager selection mode: return whether it is currently in the mode
   * where the current item is selected automatically.
   * Returns false if items were selected explicitly, e.g. using the mouse.
   */
  bool automaticSelection() const;

  /**
   * Reimplemented for setFullWidth()
   */
  virtual void viewportResizeEvent(TQResizeEvent* e);

  /**
   * Disable AutoSelection. This overrides the system wide setting for
   * AutoSelection. Please don't call this unless you have good reasons to
   * override the system wide setting for AutoSelection.
   * @see resetAutoSelection()
   * @since 3.2
   */
  void disableAutoSelection();

  /**
   * Reset AutoSelection to the system wide setting.
   * @see disableAutoSelection()
   * @since 3.2
   */
  void resetAutoSelection();

  /**
   * @deprecated This is just here for binary compatibility.  Use the signal
   * in TQListView instead.
   */
  // KDE 4: remove
  void doubleClicked( TQListViewItem *item, const TQPoint &pos, int c );

protected slots:
  /**
   * Update internal settings whenever the global ones change.
   * @internal
   */
  void slotSettingsChanged(int);

  void slotMouseButtonClicked( int btn, TQListViewItem *item, const TQPoint &pos, int c );
  void doneEditing(TQListViewItem *item, int row);

  /**
   * Repaint the rect where I was drawing the drop line.
   */
  void cleanDropVisualizer();

  /**
   * Repaint the rect where I was drawing the drop rectangle.
   */
  void cleanItemHighlighter();

  /**
   * Emit the contextMenu signal. This slot is for mouse actions.
   */
  void emitContextMenu (TQListViewItem*, const TQPoint&, int);

  /**
   * Emit the contextMenu signal. This slot is for key presses.
   */
  void emitContextMenu (KListView*, TQListViewItem*);

  /**
   * Accessory slot for AutoSelect
   * @internal
   */
  void slotOnItem( TQListViewItem *item );

  /**
   * Accessory slot for AutoSelect/ChangeCursorOverItem
   * @internal
   */
  void slotOnViewport();

  /**
   * Process AutoSelection.
   * @internal
   */
  void slotAutoSelect();

  void slotDragExpand();

  /**
   * Reacts to header changes in full width mode
   * @internal
   */
  void slotHeaderChanged();

protected:
  /**
   * Handle dropEvent when itemsMovable() is set to true.
   */
  virtual void movableDropEvent (TQListViewItem* parent, TQListViewItem* afterme);

  /**
   * Where is the nearest TQListViewItem that I'm going to drop?
   *
   * FIXME KDE 4.0: Make this method const so it can be called from an
   * acceptDrag method without ugly casts
   */
  virtual void findDrop(const TQPoint &pos, TQListViewItem *&parent, TQListViewItem *&after);

  /**
   * A special keyPressEvent (for FileManager selection mode).
   */
  void fileManagerKeyPressEvent (TQKeyEvent*);

  /**
   * Convert the depth of an item into its indentation in pixels
   */
  int depthToPixels( int depth );

private:
  class Tooltip;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KListViewPrivate;
  KListViewPrivate* const d;
  bool isExecuteArea( int x, TQListViewItem* item );
};

/**
 * A listview item with support for alternate background colors. It is
 * a drop-in replacement for QListViewItem
 *
 * @short listview item with alternate background color support
 */
class KDEUI_EXPORT KListViewItem : public QListViewItem
{
  friend class KListView;
public:
  /**
   * constructors. The semantics remain as in TQListViewItem.
   * Although they accept a TQListViewItem as parent, please
   * don't mix KListViewItem (or subclasses) with QListViewItem
   * (or subclasses).
   */
  KListViewItem(TQListView *parent);
  KListViewItem(TQListViewItem *parent);
  KListViewItem(TQListView *parent, TQListViewItem *after);
  KListViewItem(TQListViewItem *parent, TQListViewItem *after);

  KListViewItem(TQListView *parent,
    TQString, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null);

  KListViewItem(TQListViewItem *parent,
    TQString, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null);

  KListViewItem(TQListView *parent, TQListViewItem *after,
    TQString, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null);

  KListViewItem(TQListViewItem *parent, TQListViewItem *after,
    TQString, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null,
    TQString = TQString::null, TQString = TQString::null);

  virtual ~KListViewItem();

  virtual void insertItem(TQListViewItem *item);
  virtual void takeItem(TQListViewItem *item);
  /**
   * returns true if this item is to be drawn with the alternate background
   */
  bool isAlternate();
  /**
   * returns the background color for this item
   */
  const TQColor &backgroundColor() KDE_DEPRECATED;  // #### should be removed in 4.0; use below instead

  /**
   * returns the background color for this item at given column
   * This can be different in the column which is sorted due to shading
   * ### could be merged with above (column = -1) to be source compatible
   * ### but will only work if sort-shading is not used or the listView has
   * ### only 1 column
   * @since 3.4
   */
  TQColor backgroundColor(int column);

  virtual void paintCell(TQPainter *p, const TQColorGroup &cg,
    int column, int width, int alignment);

private:
  void init();

private:
  uint m_odd : 1;
  uint m_known : 1;
  uint m_unused : 30;
};

#endif

// vim: ts=2 sw=2 et
