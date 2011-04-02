/*

  Copyright (c) 2000 KDE Project

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

 */

#include "klegacystyle.h"
#include "klegacystyle.moc"
#include <klocale.h>
#include <kiconloader.h>

#define INCLUDE_MENUITEM_DEF
#include <tqapplication.h>
#include <tqbitmap.h>
#include <tqbuttongroup.h>
#include <tqcanvas.h>
#include <tqcheckbox.h>
#include <tqcolor.h>
#include <tqcolordialog.h>
#include <tqcombobox.h>
#include <tqdial.h>
#include <tqdialog.h>
#include <tqdict.h>
#include <tqfile.h>
#include <tqfiledialog.h>
#include <tqfileinfo.h>
#include <tqfont.h>
#include <tqfontdialog.h>
#include <tqframe.h>
#include <tqguardedptr.h>
#include <tqgrid.h>
#include <tqgroupbox.h>
#include <tqhbox.h>
#include <tqhbuttongroup.h>
#include <tqheader.h>
#include <tqhgroupbox.h>
#include <tqiconview.h>
#include <tqimage.h>
#include <tqinputdialog.h>
#include <tqintdict.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlcdnumber.h>
#include <tqlineedit.h>
#include <tqptrlist.h>
#include <tqlistbox.h>
#include <tqlistview.h>
#include <tqmainwindow.h>
#include <tqmenubar.h>
#include <tqmenudata.h>
#include <tqmessagebox.h>
#include <tqmultilineedit.h>
#include <tqobjectlist.h>
#include <tqpainter.h>
#include <tqpixmap.h>
#include <tqpixmapcache.h>
#include <tqpopupmenu.h>
#include <tqprintdialog.h>
#include <tqprogressbar.h>
#include <tqprogressdialog.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqregexp.h>
#include <tqscrollbar.h>
#include <tqscrollview.h>
#include <tqsemimodal.h>
#include <tqsizegrip.h>
#include <tqslider.h>
#include <tqspinbox.h>
#include <tqsplitter.h>
#include <tqstatusbar.h>
#include <tqstring.h>
#include <tqtabbar.h>
#include <tqtabdialog.h>
#include <qtableview.h>
#include <tqtabwidget.h>
#include <tqtextbrowser.h>
#include <tqtextstream.h>
#include <tqtextview.h>
#include <tqtoolbar.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>
#include <tqvbox.h>
#include <tqvbuttongroup.h>
#include <tqvgroupbox.h>
#include <tqwidget.h>
#include <tqwidgetstack.h>
#include <tqwizard.h>
#include <tqworkspace.h>

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>

// forward declaration of classes
class KLegacyBorder;
class KLegacyStyleData;
class KLegacyImageData;
class GtkObject;

// declaration of hidden functions
typedef void (TQStyle::*QDrawMenuBarItemImpl) (TQPainter *, int, int, int, int,
					      TQMenuItem *, TQColorGroup &, bool, bool);
extern QDrawMenuBarItemImpl qt_set_draw_menu_bar_impl(QDrawMenuBarItemImpl impl);

// the addresses of the integers are used to place things in the
// style data dict
static int listviewitem_ptr   = 0;
static int listboxitem_ptr    = 1;
static int menuitem_ptr       = 2;
static int separator_ptr      = 3;
static int arrow_ptr          = 4;
static int whatsthis_ptr      = 5;
static int checkmenuitem_ptr  = 6;
static int radiomenuitem_ptr  = 7;
static int eventbox_ptr       = 8;

// a TQImage cache, since we need to resize some images to different sizes, we
// will cache them, to save the overhead of loading the image from disk each
// time it's needed
static const int imageCacheSize = 61;
static TQDict<TQImage> *imageCache = 0;


class KLegacy {
public:
    enum Function { Box = 1, FlatBox, Extension, Check, Option,
		    HLine, VLine, BoxGap, Slider, Tab, Arrow, Handle, FShadow, Focus };
    enum State    { Normal = 1, Prelight, Active, Insensitive, Selected };
    enum Shadow   { NoShadow = 0, In, Out, EtchedIn, EtchedOut };
    enum GapSide  { Left = 1, Right, Top, Bottom };
};


class KLegacyBorder : public KLegacy {
private:
    int l, r, t, b;


public:
    KLegacyBorder(int ll = 0, int rr = 0, int tt = 0, int bb = 0)
	: l(ll), r(rr), t(tt), b(bb)
    { }

    KLegacyBorder(const KLegacyBorder &br)
	: l(br.l), r(br.r), t(br.t), b(br.b)
    { }

    inline int left(void) const
    { return l; }
    inline int right(void) const
    { return r; }
    inline int top(void) const
    { return t; }
    inline int bottom(void) const
    { return b; }

    inline void setLeft(int ll)
    { l = ll; }
    inline void setRight(int rr)
    { r = rr; }
    inline void setTop(int tt)
    { t = tt; }
    inline void setBottom(int bb)
    { b = bb; }
};


struct KLegacyImageDataKeyField {
    TQ_INT8 function       : 8;
    TQ_INT8 state          : 8;
    TQ_INT8 shadow         : 4;
    TQ_INT8 orientation    : 4;
    TQ_INT8 arrowDirection : 4;
    TQ_INT8 gapSide        : 4;
};


union KLegacyImageDataKey {
    KLegacyImageDataKeyField data;
    long cachekey;
};


class KLegacyImageData : public KLegacy {
public:
    KLegacyImageDataKey key;

    TQString file;
    TQString detail;
    TQString overlayFile;
    TQString gapFile;
    TQString gapStartFile;
    TQString gapEndFile;

    KLegacyBorder border;
    KLegacyBorder overlayBorder;
    KLegacyBorder gapBorder;
    KLegacyBorder gapStartBorder;
    KLegacyBorder gapEndBorder;

    bool recolorable;
    bool stretch;
    bool overlayStretch;

    KLegacyImageData()
	: recolorable(false),
	  stretch(false),
	  overlayStretch(false)
    { key.cachekey = 0; }
};


class KLegacyStyleData : public KLegacy {
public:
    // name of this style
    TQString name;

    // font to use
    TQFont *fn;

    // list of image datas (which tell us how to draw things)
    QList<KLegacyImageData> imageList;

    // background, foreground and base colors for the 5 widget
    //states that Gtk defines
    TQColor back[5], fore[5], base[5];

    // reference count
    int ref;

    KLegacyStyleData()
	: fn(0), ref(0)
    {
	// have the imageList delete the items it holds when it's deleted
	imageList.setAutoDelete(true);
    }
};


class GtkObject : public TQObject {
private:
    KLegacyStyleData *d;

    friend class KLegacyStylePrivate;


public:
    GtkObject(GtkObject *parent, const char *name)
	: TQObject(parent, name)
    { d = 0; }

    GtkObject *find(TQRegExp &) const;

    TQColor backColor(KLegacy::State);
    TQColor baseColor(KLegacy::State);
    TQColor foreColor(KLegacy::State);

    TQFont *font();

    inline TQString styleName()
    { return styleData()->name; }

    KLegacyStyleData *styleData();
    KLegacyImageData *getImageData(KLegacyImageDataKey,
				const TQString & = TQString::null);

    TQPixmap *draw(KLegacyImageDataKey, int, int, const TQString & = TQString::null);
    TQPixmap *draw(KLegacyImageData *, int, int);
};


static TQPixmap *drawImage(TQImage *image, int width, int height,
			  KLegacyBorder border, bool scale)
{
    if ((! image) || (image->isNull()) || (width < 1) || (height < 1)) {
	return (TQPixmap *) 0;
    }

    TQPixmap *pixmap = new TQPixmap(width, height);

    if (scale) {
	if (width < 2) width = 2;
	if (height < 2) height = 2;

	int x[3], y[3], w[3], h[3], x2[3], y2[3], w2[3], h2[3];

	// left
	x[0] = x2[0] = 0;
	w[0] = (border.left() < 1) ? 1 : border.left();

	// middle
	x[1] = border.left();
	w[1] = image->width() - border.left() - border.right();
	if (w[1] < 1) w[1] = 1;

	// right
	x[2] = image->width() - border.right();
	w[2] = (border.right() < 1) ? 1 : border.right();
	if (x[2] < 0) x[2] = 0;

	if ((border.left() + border.right()) > width) {
	    // left
	    x2[0] = 0;
	    w2[0] = (width / 2) + 1;

	    // middle
	    x2[1] = w2[0] - 1;
	    w2[1] = 1;

	    // right
	    x2[2] = x2[1];
	    w2[2] = w2[0];
	} else {
	    // left
	    x2[0] = 0;
	    w2[0] = border.left();

	    // middle
	    x2[1] = w2[0];
	    w2[1] = width - border.left() - border.right() + 1;

	    // right
	    x2[2] = width - border.right();
	    w2[2] = border.right();
	}

	// top
	y[0] = 0;
	h[0] = (border.top() < 1) ? 1 : border.top();

	// middle
	y[1] = border.top();
	h[1] = image->height() - border.top() - border.bottom();
	if (h[1] < 1) h[1] = 1;

	// bottom
	y[2] = image->height() - border.bottom();
	h[2] = (border.bottom() < 1) ? 1 : border.bottom();
	if (y[2] < 0) y[2] = 0;

	if ((border.top() + border.bottom()) > height) {
	    // left
	    y2[0] = 0;
	    h2[0] = height / 2;

	    // middle
	    y2[1] = h2[0];
	    h2[1] = 1;

	    // right
	    y2[2] = y2[1];
	    h2[2] = h2[0];
	} else {
	    // left
	    y2[0] = 0;
	    h2[0] = border.top();

	    // middle
	    y2[1] = h2[0];
	    h2[1] = height - border.top() - border.bottom() + 1;

	    // bottom
	    y2[2] = height - border.bottom();
	    h2[2] = border.bottom();
	}

	// draw the image
	bool mask = image->hasAlphaBuffer();
	TQBitmap bm(width, height);
	bm.fill(Qt::color1);

	TQImage nimage[3][3];
	int xx = -1, yy = -1;
	while (++yy < 3) {
	    xx = -1;
	    while (++xx < 3) {
		nimage[yy][xx] = image->copy(x[xx], y[yy], w[xx], h[yy]);

		if (nimage[yy][xx].isNull()) continue;

		if ((w[xx] != w2[xx]) || (h[yy] != h2[yy]))
		    nimage[yy][xx] = nimage[yy][xx].smoothScale(w2[xx], h2[yy]);

		if (nimage[yy][xx].isNull()) continue;

		bitBlt(pixmap, x2[xx], y2[yy], &nimage[yy][xx],
		       0, 0, w2[xx], h2[yy], Qt::CopyROP);

		if (mask) {
                    TQImage am = nimage[yy][xx].createAlphaMask();
		    bitBlt(&bm, x2[xx], y2[yy], &am,
			   0, 0, w2[xx], h2[yy], Qt::CopyROP);
                }
	    }
	}

	if (mask)
	    pixmap->setMask(bm);
    } else {
	for (int y = 0; y < height; y += image->height())
	    for (int x = 0; x < width; x += image->width())
		bitBlt(pixmap, x, y, image, 0, 0, -1, -1, Qt::CopyROP);

	if (image->hasAlphaBuffer()) {
	    TQImage mask = image->createAlphaMask();

	    if (! mask.isNull() && mask.depth() == 1) {
		TQBitmap bm(width, height);
		bm.fill(Qt::color1);
		bm = mask;
		pixmap->setMask(bm);
	    }
	}
    }

    return pixmap;
}


// Generate an object tree for all the known Gtk widgets...
// returns a pointer to the bottom of the tree
static GtkObject *initialize(TQPtrDict<GtkObject> &dict) {
    //
    // auto generated stuff from :
    // --
    // #!/usr/bin/perl -w
    //
    // foreach $line ( <STDIN> ) {
    //     chomp $line;
    //     $line =~ s/[^\sa-zA-Z0-9]/ /g;
    //     $line =~ /^(\s*)(\S*)/;
    //     $prefixlength = length $1;
    //     $classname = $2;
    //     $class{$prefixlength} = $classname;
    //     $prefixlength--;
    //     while( $prefixlength >= 0 && !defined($class{$prefixlength}) ) {
    //       $prefixlength--;
    //     }
    //     $parent = $class{$prefixlength};
    //     $parent = "0" if ( $parent eq $classname );
    //
    //     # for GtkBin:
    //     # myGtkBin = new GtkObject( myGtkWidget, "GtkBin" );
    //
    //     print "GtkObject * my$classname =
    //                new GtkObject( my$parent, \"$classname\" );\n";
    // }
    // --

    GtkObject * myGtkObject =
	new GtkObject( 0, "GtkObject" );
    GtkObject * myGtkWidget =
	new GtkObject( myGtkObject, "GtkWidget" );
    GtkObject * myGtkMisc =
	new GtkObject( myGtkWidget, "GtkMisc" );
    GtkObject * myGtkLabel =
	new GtkObject( myGtkMisc, "GtkLabel" );
    // GtkObject * myGtkAccelLabel =
    //  new GtkObject( myGtkLabel, "GtkAccelLabel" );
    GtkObject * myGtkTipsQuery =
	new GtkObject( myGtkLabel, "GtkTipsQuery" );
    GtkObject * myGtkArrow =
	new GtkObject( myGtkMisc, "GtkArrow" );
    // GtkObject * myGtkImage =
    //  new GtkObject( myGtkMisc, "GtkImage" );
    // GtkObject * myGtkPixmap =
    //  new GtkObject( myGtkMisc, "GtkPixmap" );
    GtkObject * myGtkContainer =
	new GtkObject( myGtkWidget, "GtkContainer" );
    GtkObject * myGtkBin =
	new GtkObject( myGtkContainer, "GtkBin" );
    // GtkObject * myGtkAlignment =
    //  new GtkObject( myGtkBin, "GtkAlignment" );
    GtkObject * myGtkFrame =
	new GtkObject( myGtkBin, "GtkFrame" );
    // GtkObject * myGtkAspectFrame =
    //  new GtkObject( myGtkFrame, "GtkAspectFrame" );
    GtkObject * myGtkButton =
	new GtkObject( myGtkBin, "GtkButton" );
    GtkObject * myGtkToggleButton =
	new GtkObject( myGtkButton, "GtkToggleButton" );
    GtkObject * myGtkCheckButton =
	new GtkObject( myGtkToggleButton, "GtkCheckButton" );
    GtkObject * myGtkRadioButton =
	new GtkObject( myGtkCheckButton, "GtkRadioButton" );
    GtkObject * myGtkOptionMenu =
	new GtkObject( myGtkButton, "GtkOptionMenu" );
    GtkObject * myGtkItem =
	new GtkObject( myGtkBin, "GtkItem" );
    GtkObject * myGtkMenuItem =
	new GtkObject( myGtkItem, "GtkMenuItem" );
    GtkObject * myGtkCheckMenuItem =
	new GtkObject( myGtkMenuItem, "GtkCheckMenuItem" );
    GtkObject * myGtkRadioMenuItem =
	new GtkObject( myGtkCheckMenuItem, "GtkRadioMenuItem" );
    // GtkObject * myGtkTearoffMenuItem =
    //  new GtkObject( myGtkMenuItem, "GtkTearoffMenuItem" );
    GtkObject * myGtkListItem =
	new GtkObject( myGtkItem, "GtkListItem" );
    GtkObject * myGtkTreeItem =
	new GtkObject( myGtkItem, "GtkTreeItem" );
    GtkObject * myGtkWindow =
	new GtkObject( myGtkBin, "GtkWindow" );
    GtkObject * myGtkColorSelectionDialog =
	new GtkObject( myGtkWindow, "GtkColorSelectionDialog" );
    GtkObject * myGtkDialog =
	new GtkObject( myGtkWindow, "GtkDialog" );
    GtkObject * myGtkInputDialog =
	new GtkObject( myGtkDialog, "GtkInputDialog" );
    // GtkObject * myGtkDrawWindow =
    //  new GtkObject( myGtkWindow, "GtkDrawWindow" );
    GtkObject * myGtkFileSelection =
	new GtkObject( myGtkWindow, "GtkFileSelection" );
    GtkObject * myGtkFontSelectionDialog =
	new GtkObject( myGtkWindow, "GtkFontSelectionDialog" );
    // GtkObject * myGtkPlug =
    //  new GtkObject( myGtkWindow, "GtkPlug" );
    GtkObject * myGtkEventBox =
	new GtkObject( myGtkBin, "GtkEventBox" );
    // GtkObject * myGtkHandleBox =
    //  new GtkObject( myGtkBin, "GtkHandleBox" );
    // GtkObject * myGtkScrolledWindow =
    //  new GtkObject( myGtkBin, "GtkScrolledWindow" );
    GtkObject * myGtkViewport =
	new GtkObject( myGtkBin, "GtkViewport" );
    GtkObject * myGtkBox =
	new GtkObject( myGtkContainer, "GtkBox" );
    GtkObject * myGtkButtonBox =
	new GtkObject( myGtkBox, "GtkButtonBox" );
    GtkObject * myGtkHButtonBox =
	new GtkObject( myGtkButtonBox, "GtkHButtonBox" );
    GtkObject * myGtkVButtonBox =
	new GtkObject( myGtkButtonBox, "GtkVButtonBox" );
    GtkObject * myGtkVBox =
	new GtkObject( myGtkBox, "GtkVBox" );
    // GtkObject * myGtkColorSelection =
    //  new GtkObject( myGtkVBox, "GtkColorSelection" );
    // GtkObject * myGtkGammaCurve =
    //  new GtkObject( myGtkVBox, "GtkGammaCurve" );
    GtkObject * myGtkHBox =
	new GtkObject( myGtkBox, "GtkHBox" );


    // CHANGED!  It seems that the gtk optionmenu and gtk combobox aren't related,
    // but in Qt they are the same class... so we have changed gth GtkCombo to inherit
    // from GtkOptionMenu (so that Qt comboboxes look like the optionmenus by default)
    GtkObject * myGtkCombo =
	new GtkObject( myGtkOptionMenu, "GtkCombo" );


    GtkObject * myGtkStatusbar =
	new GtkObject( myGtkHBox, "GtkStatusbar" );
    GtkObject * myGtkCList =
	new GtkObject( myGtkContainer, "GtkCList" );
    GtkObject * myGtkCTree =
	new GtkObject( myGtkCList, "GtkCTree" );
    // GtkObject * myGtkFixed =
    //  new GtkObject( myGtkContainer, "GtkFixed" );
    GtkObject * myGtkNotebook =
	new GtkObject( myGtkContainer, "GtkNotebook" );
    // GtkObject * myGtkFontSelection =
    //  new GtkObject( myGtkNotebook, "GtkFontSelection" );
    GtkObject * myGtkPaned =
	new GtkObject( myGtkContainer, "GtkPaned" );
    // GtkObject * myGtkHPaned =
    //  new GtkObject( myGtkPaned, "GtkHPaned" );
    // GtkObject * myGtkVPaned =
    // new GtkObject( myGtkPaned, "GtkVPaned" );
    // GtkObject * myGtkLayout =
    //  new GtkObject( myGtkContainer, "GtkLayout" );
    // GtkObject * myGtkList =
    //  new GtkObject( myGtkContainer, "GtkList" );
    GtkObject * myGtkMenuShell =
	new GtkObject( myGtkContainer, "GtkMenuShell" );
    GtkObject * myGtkMenuBar =
	new GtkObject( myGtkMenuShell, "GtkMenuBar" );
    GtkObject * myGtkMenu =
	new GtkObject( myGtkMenuShell, "GtkMenu" );
    // GtkObject * myGtkPacker =
    //  new GtkObject( myGtkContainer, "GtkPacker" );
    // GtkObject * myGtkSocket =
    //  new GtkObject( myGtkContainer, "GtkSocket" );
    GtkObject * myGtkTable =
	new GtkObject( myGtkContainer, "GtkTable" );
    GtkObject * myGtkToolbar =
	new GtkObject( myGtkContainer, "GtkToolbar" );
    // GtkObject * myGtkTree =
    // new GtkObject( myGtkContainer, "GtkTree" );
    // GtkObject * myGtkCalendar =
    //  new GtkObject( myGtkWidget, "GtkCalendar" );
    GtkObject * myGtkDrawingArea =
	new GtkObject( myGtkWidget, "GtkDrawingArea");
    // GtkObject * myGtkCurve =
    // new GtkObject( myGtkDrawingArea, "GtkCurve" );
    GtkObject * myGtkEditable =
	new GtkObject( myGtkWidget, "GtkEditable" );
    GtkObject * myGtkEntry =
	new GtkObject( myGtkEditable, "GtkEntry" );
    GtkObject * myGtkSpinButton =
	new GtkObject( myGtkEntry, "GtkSpinButton" );
    GtkObject * myGtkText =
	new GtkObject( myGtkEditable, "GtkText" );
    GtkObject * myGtkRuler =
	new GtkObject( myGtkWidget, "GtkRuler" );
    // GtkObject * myGtkHRuler =
    //  new GtkObject( myGtkRuler, "GtkHRuler" );
    // GtkObject * myGtkVRuler =
    //  new GtkObject( myGtkRuler, "GtkVRuler" );
    GtkObject * myGtkRange =
	new GtkObject( myGtkWidget, "GtkRange" );
    GtkObject * myGtkScale =
	new GtkObject( myGtkRange, "GtkScale" );
    // GtkObject * myGtkHScale =
    //  new GtkObject( myGtkScale, "GtkHScale" );
    // GtkObject * myGtkVScale =
    //  new GtkObject( myGtkScale, "GtkVScale" );
    GtkObject * myGtkScrollbar =
	new GtkObject( myGtkRange, "GtkScrollbar" );
    // GtkObject * myGtkHScrollbar =
    //  new GtkObject( myGtkScrollbar, "GtkHScrollbar" );
    // GtkObject * myGtkVScrollbar =
    //  new GtkObject( myGtkScrollbar, "GtkVScrollbar" );
    GtkObject * myGtkSeparator =
	new GtkObject( myGtkWidget, "GtkSeparator" );
    // GtkObject * myGtkHSeparator =
    //  new GtkObject( myGtkSeparator, "GtkHSeparator" );
    // GtkObject * myGtkVSeparator =
    //  new GtkObject( myGtkSeparator, "GtkVSeparator" );
    // GtkObject * myGtkPreview =
    //  new GtkObject( myGtkWidget, "GtkPreview" );
    GtkObject * myGtkProgress =
	new GtkObject( myGtkWidget, "GtkProgress" );
    GtkObject * myGtkProgressBar =
	new GtkObject( myGtkProgress, "GtkProgressBar" );
    //GtkObject * myGtkData =
    // new GtkObject( myGtkObject, "GtkData" );
    // GtkObject * myGtkAdjustment =
    //	new GtkObject( myGtkData, "GtkAdjustment" );
    // GtkObject * myGtkTooltips =
    //  new GtkObject( myGtkData, "GtkTooltips" );
    // GtkObject * myGtkItemFactory =
    //  new GtkObject( myGtkObject, "GtkItemFactory" );

    // Insert the above Gtk widgets into a dict, using meta data pointers for
    // the different widgets in Qt
    //
    // verify with:
    // --
    // egrep "::staticMetaObject\(\)$" **/*.cpp | fmt -1 | grep :: |
    //    sort | uniq > meta
    //--

    dict.insert(TQButton::staticMetaObject(), myGtkButton);
    dict.insert(TQButtonGroup::staticMetaObject(), myGtkButtonBox);
    dict.insert(TQCanvas::staticMetaObject(), myGtkDrawingArea);
    dict.insert(TQCanvasView::staticMetaObject(), myGtkDrawingArea);
    dict.insert(TQCheckBox::staticMetaObject(), myGtkCheckButton);
    dict.insert(QColorDialog::staticMetaObject(), myGtkColorSelectionDialog);
    dict.insert(TQComboBox::staticMetaObject(), myGtkCombo);
    dict.insert(TQDial::staticMetaObject(), myGtkWidget);
    dict.insert(TQDialog::staticMetaObject(), myGtkDialog);
    dict.insert(TQFileDialog::staticMetaObject(), myGtkFileSelection);
    dict.insert(QFontDialog::staticMetaObject(), myGtkFontSelectionDialog);
    dict.insert(TQFrame::staticMetaObject(), myGtkFrame);
    dict.insert(TQGrid::staticMetaObject(), myGtkFrame);
    dict.insert(TQGroupBox::staticMetaObject(), myGtkBox);
    dict.insert(TQHBox::staticMetaObject(), myGtkHBox);
    dict.insert(TQHButtonGroup::staticMetaObject(), myGtkHButtonBox);
    dict.insert(TQHGroupBox::staticMetaObject(), myGtkHBox);
    dict.insert(TQHeader::staticMetaObject(), myGtkRuler);
    dict.insert(TQIconView::staticMetaObject(), myGtkCTree);
    dict.insert(QInputDialog::staticMetaObject(), myGtkInputDialog);
    dict.insert(TQLCDNumber::staticMetaObject(), myGtkFrame);
    dict.insert(TQLabel::staticMetaObject(), myGtkLabel);
    dict.insert(TQLineEdit::staticMetaObject(), myGtkEntry);
    dict.insert(TQListBox::staticMetaObject(), myGtkCList);
    dict.insert(TQListView::staticMetaObject(), myGtkCTree);
    dict.insert(TQMainWindow::staticMetaObject(), myGtkWindow);
    dict.insert(TQMenuBar::staticMetaObject(), myGtkMenuBar);
    dict.insert(TQMessageBox::staticMetaObject(), myGtkDialog);
    dict.insert(TQMultiLineEdit::staticMetaObject(), myGtkText);
    dict.insert(TQPopupMenu::staticMetaObject(), myGtkMenu);
    dict.insert(TQPrintDialog::staticMetaObject(), myGtkDialog);
    dict.insert(TQProgressBar::staticMetaObject(), myGtkProgressBar);
    dict.insert(TQProgressDialog::staticMetaObject(), myGtkDialog);
    dict.insert(TQPushButton::staticMetaObject(), myGtkButton);
    dict.insert(TQRadioButton::staticMetaObject(), myGtkRadioButton);
    dict.insert(TQScrollBar::staticMetaObject(), myGtkScrollbar);
    dict.insert(TQScrollView::staticMetaObject(), myGtkViewport);
    dict.insert(TQSemiModal::staticMetaObject(), myGtkDialog);
    dict.insert(TQSizeGrip::staticMetaObject(), myGtkWidget);
    dict.insert(TQSlider::staticMetaObject(), myGtkScale);
    dict.insert(TQSpinBox::staticMetaObject(), myGtkSpinButton);
    dict.insert(TQSplitter::staticMetaObject(), myGtkPaned);
    dict.insert(TQStatusBar::staticMetaObject(), myGtkStatusbar);
    dict.insert(TQTabBar::staticMetaObject(), myGtkNotebook);
    dict.insert(TQTabDialog::staticMetaObject(), myGtkNotebook);
    dict.insert(TQTabWidget::staticMetaObject(), myGtkNotebook);
    dict.insert(QTableView::staticMetaObject(), myGtkTable);
    dict.insert(TQTextBrowser::staticMetaObject(), myGtkText);
    dict.insert(TQTextView::staticMetaObject(), myGtkText);
    dict.insert(TQToolBar::staticMetaObject(), myGtkToolbar);
    dict.insert(TQToolButton::staticMetaObject(), myGtkButton);
    dict.insert(TQVBox::staticMetaObject(), myGtkVBox);
    dict.insert(TQVButtonGroup::staticMetaObject(), myGtkVButtonBox);
    dict.insert(TQVGroupBox::staticMetaObject(), myGtkVBox);
    dict.insert(TQWidget::staticMetaObject(), myGtkWidget);
    dict.insert(TQWidgetStack::staticMetaObject(), myGtkWidget);
    dict.insert(TQWizard::staticMetaObject(), myGtkWindow);
    dict.insert(TQWorkspace::staticMetaObject(), myGtkWindow);

    // stuff that we don't have meta data for, but want to know about
    dict.insert(&listboxitem_ptr, myGtkListItem);
    dict.insert(&listviewitem_ptr, myGtkTreeItem);
    dict.insert(&menuitem_ptr, myGtkMenuItem);
    dict.insert(&separator_ptr, myGtkSeparator);
    dict.insert(&arrow_ptr, myGtkArrow);
    dict.insert(&whatsthis_ptr, myGtkTipsQuery);
    dict.insert(&checkmenuitem_ptr, myGtkCheckMenuItem);
    dict.insert(&radiomenuitem_ptr, myGtkRadioMenuItem);
    dict.insert(&eventbox_ptr, myGtkEventBox);

    return myGtkObject;
}


KLegacyImageData *GtkObject::getImageData(KLegacyImageDataKey key, const TQString &detail) {
    KLegacyImageData *imagedata = 0;

    if (styleData()) {
	QListIterator<KLegacyImageData> it(styleData()->imageList);

	while ((imagedata = it.current()) != 0) {
	    ++it;

	    if ((((imagedata->key.data.function != 0) &&
		  (imagedata->key.data.function == key.data.function)) ||
		 (imagedata->key.data.function == 0)) &&

		(((imagedata->key.data.state != 0) &&
		  (imagedata->key.data.state == key.data.state)) ||
		 (imagedata->key.data.state == 0)) &&

		(((imagedata->key.data.shadow != 0) &&
		  (imagedata->key.data.shadow == key.data.shadow)) ||
		 (imagedata->key.data.shadow == 0)) &&

		(((imagedata->key.data.orientation != 0) &&
		  (imagedata->key.data.orientation == key.data.orientation)) ||
		 (imagedata->key.data.orientation == 0)) &&

		(((imagedata->key.data.arrowDirection != 0) &&
		  (imagedata->key.data.arrowDirection == key.data.arrowDirection)) ||
		 (imagedata->key.data.arrowDirection == 0)) &&

		(((imagedata->key.data.gapSide != 0) &&
		  (imagedata->key.data.gapSide == key.data.gapSide)) ||
		 (imagedata->key.data.gapSide == 0)) &&

		(((!imagedata->detail.isNull()) &&
		  (detail == imagedata->detail)) ||
		 (imagedata->detail.isNull()))) {
		// we have a winner
		break;
	    }
	}
    }

    if ((! imagedata) && (parent())) {
	imagedata = ((GtkObject *) parent())->getImageData(key, detail);
    }

    return imagedata;
}


KLegacyStyleData *GtkObject::styleData() {
    if ((! d) && parent()) {
	d = ((GtkObject *) parent())->styleData();
    }

    return d;
}


TQColor GtkObject::backColor(KLegacy::State s) {
    if ((!  styleData()->back[s].isValid()) && parent()) {
	return ((GtkObject *) parent())->backColor(s);
    }

    if (styleData()->back[s].isValid())
	return  styleData()->back[s];

    return white;
}


TQColor GtkObject::baseColor(KLegacy::State s) {
    if ((! styleData()->base[s].isValid()) && parent()) {
	return ((GtkObject *) parent())->baseColor(s);
    }

    if (styleData()->base[s].isValid())
	return styleData()->base[s];

    return white;
}


TQColor GtkObject::foreColor(KLegacy::State s) {
    if ((! styleData()->fore[s].isValid()) && parent()) {
	return ((GtkObject *) parent())->foreColor(s);
    }

    if (styleData()->fore[s].isValid())
	return styleData()->fore[s];

    return black;
}


TQFont *GtkObject::font() {
    if ((! styleData()->fn) && parent()) {
	return ((GtkObject *) parent())->font();
    }

    return styleData()->fn;
}


GtkObject *GtkObject::find(TQRegExp &r) const {
    // if the regular expression matches the name of this widget, return
    if (r.match(name()) != -1) {
	return (GtkObject *) this;
    }

    // regex doesn't match us, and we have no children, return 0
    if (! children()) return 0;

    TQObject *o;
    GtkObject *obj, *gobj;

    TQObjectListIt ot(*children());

    // search our children to see if any match the regex
    while ((o = ot.current()) != 0) {
	++ot;

	// this would be nice if tqmoc could parse this file :/
	//
	// if (o->className() != "GtkObject") {
	//     qDebug("object is not a GtkObject (className = '%s')",
	// 	      o->className());
	//     continue;
	// }

	obj = (GtkObject *) o;

	// use obj->find(r) instead of r.match(obj->name()) so that this child's
	// children will be searched as well... this allows us to search the entire
	// object tree
	if ((gobj = obj->find(r)) != 0) {
	    // found something!
	    return (GtkObject *) gobj;
	}
    }

    // found nothing
    return 0;
}


TQPixmap *GtkObject::draw(KLegacyImageDataKey key, int width, int height,
			 const TQString &detail)
{
    KLegacyImageData *imagedata = getImageData(key, detail);
    if (! imagedata) {
	return 0;
    }

    return draw(imagedata, width, height);
}


TQPixmap *GtkObject::draw(KLegacyImageData *imagedata, int width, int height) {
    TQString pixmapKey;
    TQTextOStream(&pixmapKey) << "$KLegacy_Image_" << styleData()->name << "_" <<
	className() << "_" << width << "x" << height << "_" <<
	imagedata->key.cachekey << "_" << (uint) imagedata->recolorable <<
	(uint) imagedata->stretch << (uint) imagedata->overlayStretch;

    TQPixmap *pixmap = TQPixmapCache::find(pixmapKey);
    if (pixmap) {
	return pixmap;
    }

    TQPixmap *main = 0, *overlay = 0;

    if (! imagedata->file.isNull()) {
	TQImage *image = imageCache->find(imagedata->file);
	bool found = true;

	if (! image) {
	    image = new TQImage(imagedata->file);

	    if (! image || image->isNull()) {
		found = false;
	    } else {
		imageCache->insert(imagedata->file, image);
	    }
	}

	if (found) {
	    int w = ((imagedata->stretch) ? width : image->width()),
		h = ((imagedata->stretch) ? height : image->height());
	    main = drawImage(image, w, h, imagedata->border, imagedata->stretch);
	}
    }

    if (! imagedata->overlayFile.isNull()) {
	TQImage *image = imageCache->find(imagedata->overlayFile);
	bool found = true;

	if (! image) {
	    image = new TQImage(imagedata->overlayFile);

	    if (! image || image->isNull()) {
		found = false;
	    } else {
		imageCache->insert(imagedata->overlayFile, image);
	    }
	}

	if (found) {
	    int w = ((imagedata->overlayStretch) ? width : image->width()),
		h = ((imagedata->overlayStretch) ? height : image->height());
	    overlay = drawImage(image, w, h, imagedata->overlayBorder,
				imagedata->overlayStretch);
	}
    }

    TQSize sz;
    if (main) {
	sz = sz.expandedTo(main->size());
    }

    if (overlay) {
	sz = sz.expandedTo(overlay->size());
    }

    if (sz.isEmpty()) {
	return (TQPixmap *) 0;
    }

    pixmap = new TQPixmap(sz);
    pixmap->fill(TQColor(192,192,176));
    TQPainter p(pixmap);

    if (main && (! main->isNull())) {
	p.drawTiledPixmap(0, 0, sz.width(), sz.height(), *main);
    }

    if (overlay && (! overlay->isNull())) {
	TQPoint pt((sz.width() - overlay->width()) / 2,
		  (sz.height() - overlay->height()) / 2);
	p.drawPixmap(pt, *overlay);
    }

    p.end();

    if (main) {
	if (main->mask() && (! main->mask()->isNull())) {
	    TQBitmap bm(sz);
	    TQPainter m(&bm);
	    TQRect r(0, 0, width, height);

	    m.drawTiledPixmap(r, *(main->mask()));
	    m.end();

	    pixmap->setMask(bm);
	}
    } else if (overlay) {
	if (overlay->mask() && (! overlay->mask()->isNull())) {
	    TQBitmap bm(sz);
	    TQPainter m(&bm);
	    TQRect r((sz.width() - overlay->width()) / 2,
		    (sz.height() - overlay->height()) / 2,
		    sz.width(), sz.height());
	    m.drawTiledPixmap(r, *(overlay->mask()));
	    m.end();

	    pixmap->setMask(bm);
	}
    }

    if (! TQPixmapCache::insert(pixmapKey, pixmap)) {
	delete pixmap;
	pixmap = (TQPixmap *) 0;
    }

    return pixmap;
}


class KLegacyStylePrivate : public KLegacy {
private:
    TQDict<KLegacyStyleData> styleDict;
    TQStringList pixmapPath;
    TQTextStream filestream;

    TQFont oldfont;
    TQPalette oldpalette;

    // pointer to the widget under the pointer
    TQGuardedPtr<TQWidget> lastWidget;

    // current position of the mouse
    TQPoint mousePos;
    bool hovering;

    TQPtrDict<GtkObject> gtkDict;
    GtkObject *gtktree;

    friend class KLegacyStyle;


public:
    KLegacyStylePrivate();
    ~KLegacyStylePrivate();

    // parse the filename passed
    bool parse(const TQString &filename);

    bool parseClass();
    bool parseEngine(KLegacyStyleData *);
    bool parseImage(KLegacyStyleData *);
    bool parsePixmapPath();
    bool parseStyle();
};


KLegacyStylePrivate::KLegacyStylePrivate()
    : lastWidget(0), mousePos(-1, -1), hovering(false), gtktree(0)
{
    TQPixmapCache::setCacheLimit(8192);

    if (! imageCache) {
	imageCache = new TQDict<TQImage>(imageCacheSize);
	CHECK_PTR(imageCache);

	imageCache->setAutoDelete(true);
    }

    styleDict.setAutoDelete(true);

    gtktree = initialize(gtkDict);
    CHECK_PTR(gtktree);

    if (! gtktree->d) {
	gtktree->d = new KLegacyStyleData;
 	gtktree->d->name = "Default";
    }

    // get the path to this users .gtkrc
    TQString gtkrcFilename = getenv("HOME");
    gtkrcFilename += "/.gtkrc";

    TQFile gtkrc(gtkrcFilename);

    if (gtkrc.open(IO_ReadOnly)) {
	filestream.setDevice(&gtkrc);

	while (! filestream.atEnd()) {
	    TQString next;
	    filestream >> next;

	    if (next.isNull()) continue;

	    // skip comments
	    if (next[0] == '#') { filestream.readLine(); continue; }

	    if (next == "class" || next == "widget" || next == "widget_class") {
		if (! parseClass())
		    qWarning("\"class\" parse error");
	    } else if (next == "pixmap_path") {
		if (! parsePixmapPath())
		    qWarning("\"pixmap_path\" parse error");
	    } else if (next == "style") {
		if (! parseStyle())
		    qWarning("\"style\" parse error");
	    }
	}

	gtkrc.close();
    } else
	qWarning("%s: failed to open", gtkrcFilename.latin1());
}


KLegacyStylePrivate::~KLegacyStylePrivate() {
    if (imageCache) {
	delete imageCache;
	imageCache = 0;
    }

    if (gtktree) {
	delete gtktree;
	gtktree = 0;
    }
}


bool KLegacyStylePrivate::parseClass() {
    if (filestream.atEnd()) return false;

    TQString classname, keyword, stylename;
    filestream >> classname;
    filestream >> keyword;
    filestream >> stylename;

    if (classname.isNull() || keyword.isNull() || stylename.isNull() ||
	keyword != "style" ||
	classname[0] != '\"' || classname[classname.length() - 1] != '\"' ||
	stylename[0] != '\"' || stylename[stylename.length() - 1] != '\"')
	return false;

    classname = classname.mid(1, classname.length() - 2);
    stylename = stylename.mid(1, stylename.length() - 2);

    TQRegExp r(classname);
    r.setWildcard(true);
    GtkObject *obj = gtktree->find(r);

    if (! obj) {
	qWarning("unknown object '%s'", classname.latin1());
	return false;
    }

    KLegacyStyleData *styledata = styleDict.tqfind(stylename);

    if (! styledata) {
	qWarning("no such style '%s' for class '%s' (%p)", stylename.latin1(),
		 classname.latin1(), styledata);
	return false;
    }

    obj->d = styledata;
    styledata->ref++;

    return true;
}


bool KLegacyStylePrivate::parseImage(KLegacyStyleData *styledata) {
    if (filestream.atEnd()) {
	qWarning("parseImage: premature end of stream");
	return false;
    }

    TQString next, equals, parameter;
    filestream >> next;

    // skip comments
    while (next[0] == '#') {
	filestream.readLine();
	filestream >> next;
    }

    if (next.isNull() || next != "{") {
	qWarning("parseImage: expected '{' after 'image'\n"
		 "  in style '%s', after processing %d previous images\n",
		 styledata->name.latin1(), styledata->imageList.count());
	return false;
    }

    KLegacyImageData *imagedata = new KLegacyImageData;

    int paren_count = 1;
    while (paren_count) {
	filestream >> next;
	if (next.isNull()) continue;

	// skip comments
	if (next[0] == '#') {filestream.readLine(); continue; }

	if (next == "arrow_direction") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "UP")
		imagedata->key.data.arrowDirection = Qt::UpArrow + 1;
	    else if (parameter == "DOWN")
		imagedata->key.data.arrowDirection = Qt::DownArrow + 1;
	    else if (parameter == "LEFT")
		imagedata->key.data.arrowDirection = Qt::LeftArrow + 1;
	    else if (parameter == "RIGHT")
		imagedata->key.data.arrowDirection = Qt::RightArrow + 1;
	} else if (next == "border") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter != "{")
		continue;
	    TQString border =filestream.readLine();

	    int lp, rp, tp, bp, l, r, t, b;
	    lp = border.tqfind(',');
	    rp = border.tqfind(',', lp + 1);
	    tp = border.tqfind(',', rp + 1);
	    bp = border.tqfind('}', tp + 1);

	    l = border.left(lp).toUInt();
	    r = border.mid(lp + 1, rp - lp - 1).toUInt();
	    t = border.mid(rp + 1, tp - rp - 1).toUInt();
	    b = border.mid(tp + 1, bp - tp - 1).toUInt();

	    imagedata->border.setLeft(l);
	    imagedata->border.setRight(r);
	    imagedata->border.setTop(t);
	    imagedata->border.setBottom(b);
	} else if (next == "detail") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter[0] != '\"' || parameter[parameter.length() - 1] != '\"')
		continue;

	    parameter = parameter.mid(1, parameter.length() - 2);
	    imagedata->detail = parameter;
	} else if (next == "file") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter[0] != '\"' || parameter[parameter.length() - 1] != '\"') {
		qWarning("image: file parameter malformed");
		continue;
	    }

	    parameter = parameter.mid(1, parameter.length() - 2);

	    TQStringList::Iterator it;
	    for (it = pixmapPath.begin(); it != pixmapPath.end(); ++it) {
		TQFileInfo fileinfo((*it) + parameter);

		if (fileinfo.exists()) {
		    imagedata->file = fileinfo.filePath();
		    break;
		}
	    }
	} else if (next == "function") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "BOX")
		imagedata->key.data.function = Box;
	    else if (parameter == "FLAT_BOX")
		imagedata->key.data.function = FlatBox;
	    else if (parameter == "EXTENSION")
		imagedata->key.data.function = Extension;
	    else if (parameter == "CHECK")
		imagedata->key.data.function = Check;
	    else if (parameter == "OPTION")
		imagedata->key.data.function = Option;
	    else if (parameter == "HLINE")
		imagedata->key.data.function = HLine;
	    else if (parameter == "VLINE")
		imagedata->key.data.function = VLine;
	    else if (parameter == "BOX_GAP")
		imagedata->key.data.function = BoxGap;
	    else if (parameter == "SLIDER")
		imagedata->key.data.function = Slider;
	    else if (parameter == "TAB")
		imagedata->key.data.function = Tab;
	    else if (parameter == "ARROW")
		imagedata->key.data.function = Arrow;
	    else if (parameter == "HANDLE")
		imagedata->key.data.function = Handle;
	    else if (parameter == "SHADOW")
		imagedata->key.data.function = FShadow;
	    else if (parameter == "FOCUS")
		imagedata->key.data.function = Focus;
	} else if (next == "gap_side" ) {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "TOP")
		imagedata->key.data.gapSide = KLegacy::Top;
	    else if (parameter == "BOTTOM")
		imagedata->key.data.gapSide = KLegacy::Bottom;
	} else if (next == "orientation") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "VERTICAL")
		imagedata->key.data.orientation = Qt::Vertical + 1;
	    else if (parameter == "HORIZONTAL")
		imagedata->key.data.orientation = Qt::Horizontal + 1;
	} else if (next == "overlay_border") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter != "{")
		continue;
	    TQString border = filestream.readLine();

	    int lp, rp, tp, bp, l, r, t, b;
	    lp = border.tqfind(',');
	    rp = border.tqfind(',', lp + 1);
	    tp = border.tqfind(',', rp + 1);
	    bp = border.tqfind('}', tp + 1);

	    l = border.left(lp).toUInt();
	    r = border.mid(lp + 1, rp - lp - 1).toUInt();
	    t = border.mid(rp + 1, tp - rp - 1).toUInt();
	    b = border.mid(tp + 1, bp - tp - 1).toUInt();

	    imagedata->overlayBorder.setLeft(l);
	    imagedata->overlayBorder.setRight(r);
	    imagedata->overlayBorder.setTop(t);
	    imagedata->overlayBorder.setBottom(b);
	} else if (next == "overlay_file") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter[0] != '\"' || parameter[parameter.length() - 1] != '\"') {
		qWarning("image: overlay_file parameter malformed");
		continue;
	    }

	    parameter = parameter.mid(1, parameter.length() - 2);

	    TQStringList::Iterator it;
	    for (it = pixmapPath.begin(); it != pixmapPath.end(); ++it) {
		TQFileInfo fileinfo((*it) + parameter);

		if (fileinfo.exists()) {
		    imagedata->overlayFile = fileinfo.filePath();
		    break;
		}
	    }
	} else if (next == "overlay_stretch") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "TRUE")
		imagedata->overlayStretch = true;
	    else
		imagedata->overlayStretch = false;
	} else if (next == "stretch") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "TRUE")
		imagedata->stretch = true;
	    else
		imagedata->stretch = false;
	} else if (next == "shadow") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "NONE")
		imagedata->key.data.shadow = NoShadow;
	    else if (parameter == "IN")
		imagedata->key.data.shadow = In;
	    else if (parameter == "OUT")
		imagedata->key.data.shadow = Out;
	    else if (parameter == "ETCHED_IN")
		imagedata->key.data.shadow = EtchedIn;
	    else if (parameter == "ETCHED_OUT")
		imagedata->key.data.shadow = EtchedOut;
	} else if (next == "state") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "NORMAL")
		imagedata->key.data.state = Normal;
	    else if (parameter == "PRELIGHT")
		imagedata->key.data.state = Prelight;
	    else if (parameter == "ACTIVE")
		imagedata->key.data.state = Active;
	    else if (parameter == "INSENSITIVE")
		imagedata->key.data.state = Insensitive;
	    else if (parameter == "SELECTED")
		imagedata->key.data.state = Selected;
	} else if (next == "{") paren_count++;
	else if (next == "}") paren_count--;
    }

    styledata->imageList.append(imagedata);

    return true;
}


bool KLegacyStylePrivate::parseEngine(KLegacyStyleData *styledata) {
    if (filestream.atEnd()) return false;

    TQString enginename, paren;
    filestream >> enginename;
    filestream >> paren;

    if (enginename.isNull() || paren.isNull() ||
	enginename[0] != '\"' || enginename[enginename.length() - 1] != '\"' ||
	paren != "{") {
	return false;
    }

    TQString next;
    int paren_count = 1;
    while (paren_count) {
	filestream >> next;

	// skip comments
	if (next[0] == '#') {
	    filestream.readLine();
	    continue;
	}

	if (next == "image") {
	    if (! parseImage(styledata)) {
		qWarning("image parse error");
	    }
	} else if (next == "{") {
	    paren_count++;
	} else if (next == "}") {
	    paren_count--;
	}
    }

    return true;
}


bool KLegacyStylePrivate::parsePixmapPath() {
    if (filestream.atEnd()) {
	return false;
    }

    TQString next;
    filestream >> next;

    if (next.isNull() || next[0] != '\"' || next[next.length() - 1] != '\"') {
	return false;
    }

    next = next.mid(1, next.length() - 2);

    int start = 0, end = next.tqfind(":");
    while (end != -1) {
	TQString path(next.mid(start, end - start));

	if (path[path.length() - 1] != '/') {
	    path += '/';
	}

	TQFileInfo fileinfo(path);
	if (fileinfo.exists() && fileinfo.isDir()) {
	    pixmapPath.append(path);
	}

	start = end + 1;
	end = next.tqfind(":", start);
    }

    // get the straggler
    end = next.length();
    TQString path(next.mid(start, end - start));

    if (path[path.length() - 1] != '/') {
	path += '/';
    }

    TQFileInfo fileinfo(path);
    if (fileinfo.exists() && fileinfo.isDir()) {
	pixmapPath.append(path);
    }

    return true;
}


bool KLegacyStylePrivate::parseStyle() {
    if (filestream.atEnd()) return false;

    TQString stylename, paren;
    filestream >> stylename;
    filestream >> paren;

    if (stylename.isNull() || paren.isNull() ||
	stylename[0] != '\"' || stylename[stylename.length() - 1] != '\"')
	return false;

    stylename = stylename.mid(1, stylename.length() - 2);

    if (paren == "=") {
	TQString newstylename;
	filestream >> newstylename;

	if (newstylename.isNull() ||
	    newstylename[0] != '\"' || newstylename[newstylename.length() - 1] != '\"')
	    return false;

	newstylename = newstylename.mid(1, newstylename.length() - 2);

	KLegacyStyleData *styledata = styleDict.tqfind(stylename);

	if (! styledata) return false;

	KLegacyStyleData *newstyledata = new KLegacyStyleData(*styledata);
	newstyledata->name = newstylename;
	styleDict.insert(newstylename, newstyledata);

	return true;
    } else if (paren != "{") {
	qWarning("parseStyle: expected '{' while parsing style %s",
		 stylename.latin1());
	return false;
    }

    KLegacyStyleData *styledata = new KLegacyStyleData;
    styledata->name = stylename;

    TQString next, parameter;
    int paren_count = 1;
    while (paren_count) {
	filestream >> next;

	// skip comments
	if (next[0] == '#') {
	    filestream.readLine();
	    continue;
	}

	if (next.left(5) == "base[") {
	    int l = next.tqfind('['), r = next.tqfind(']'), state;

	    if (l < 1 || r < 1 || r < l) continue;

	    TQString mode = next.mid(l + 1, r - l - 1);
	    if (mode == "ACTIVE")
		state = Active;
	    else if (mode == "NORMAL")
		state = Normal;
	    else if (mode == "INSENSITIVE")
		state = Insensitive;
	    else if (mode == "PRELIGHT")
		state = Prelight;
	    else if (mode == "SELECTED")
		state = Selected;

	    filestream >> next;
	    filestream >> parameter;

	    if (next.isNull() || parameter.isNull() || next != "=") continue;

	    if (parameter[0] == '\"') { // assume color of the form "#rrggbb"
		TQString colorname = parameter.mid(1, parameter.length() - 2);
		if (colorname.isNull()) continue;

		styledata->base[state].setNamedColor(colorname);
	    } else if (parameter == "{") { // assume color of the form  { ri, gi, bi }
		TQString color =filestream.readLine();

		int rp, gp, bp;
		float ri, gi, bi;

		rp = color.tqfind(',');
		gp = color.tqfind(',', rp + 1);
		bp = color.tqfind('}', gp + 1);

		ri = color.left(rp).toFloat();
		gi = color.mid(rp + 1, gp - rp - 1).toFloat();
		bi = color.mid(gp + 1, bp - gp - 1).toFloat();

		int red   = (int) (255 * ri);
		int green = (int) (255 * gi);
		int blue  = (int) (255 * bi);
		styledata->base[state].setRgb(red, green, blue);
	    }
	} else if (next.left(3) == "bg[") {
	    int l = next.tqfind('['), r = next.tqfind(']'), state;

	    if (l < 1 || r < 1 || r < l) continue;

	    TQString mode = next.mid(l + 1, r - l - 1);
	    if (mode == "ACTIVE")
		state = Active;
	    else if (mode == "NORMAL")
		state = Normal;
	    else if (mode == "INSENSITIVE")
		state = Insensitive;
	    else if (mode == "PRELIGHT")
		state = Prelight;
	    else if (mode == "SELECTED")
		state = Selected;

	    filestream >> next;
	    filestream >> parameter;

	    if (next.isNull() || parameter.isNull() || next != "=") continue;

	    if (parameter[0] == '\"') { // assume color of the form "#rrggbb"
		TQString colorname = parameter.mid(1, parameter.length() - 2);
		if (colorname.isNull()) continue;

		styledata->back[state].setNamedColor(colorname);
	    } else if (parameter == "{") { // assume color of the form  { ri, gi, bi }
		TQString color =filestream.readLine();

		int rp, gp, bp;
		float ri, gi, bi;

		rp = color.tqfind(',');
		gp = color.tqfind(',', rp + 1);
		bp = color.tqfind('}', gp + 1);

		ri = color.left(rp).toFloat();
		gi = color.mid(rp + 1, gp - rp - 1).toFloat();
		bi = color.mid(gp + 1, bp - gp - 1).toFloat();

		int red   = (int) (255 * ri);
		int green = (int) (255 * gi);
		int blue  = (int) (255 * bi);
		styledata->back[state].setRgb(red, green, blue);
	    }
	} else if (next == "engine") {
	    if (! parseEngine(styledata))
		fprintf(stderr, "engine parse error\n");
	} else if (next.left(3) == "fg[") {
	    int l = next.tqfind('['), r = next.tqfind(']'), state;

	    if (l < 1 || r < 1 || r < l) continue;

	    TQString mode = next.mid(l + 1, r - l - 1);
	    if (mode == "ACTIVE")
		state = Active;
	    else if (mode == "NORMAL")
		state = Normal;
	    else if (mode == "INSENSITIVE")
		state = Insensitive;
	    else if (mode == "PRELIGHT")
		state = Prelight;
	    else if (mode == "SELECTED")
		state = Selected;

	    filestream >> next;
	    filestream >> parameter;

	    if (next.isNull() || parameter.isNull() || next != "=") continue;

	    if (parameter[0] == '\"') { // assume color of the form "#rrggbb"
		TQString colorname = parameter.mid(1, parameter.length() - 2);
		if (colorname.isNull()) continue;

		styledata->fore[state].setNamedColor(colorname);
	    } else if (parameter == "{") { // assume color of the form  { ri, gi, bi }
		TQString color = filestream.readLine();

		int rp, gp, bp;
		float ri, gi, bi;

		rp = color.tqfind(',');
		gp = color.tqfind(',', rp + 1);
		bp = color.tqfind('}', gp + 1);

		ri = color.left(rp).toFloat();
		gi = color.mid(rp + 1, gp - rp - 1).toFloat();
		bi = color.mid(gp + 1, bp - gp - 1).toFloat();

		int red   = (int) (255 * ri);
		int green = (int) (255 * gi);
		int blue  = (int) (255 * bi);
		styledata->fore[state].setRgb(red, green, blue);
	    }
	} else if (next == "font") {
	    filestream >> next;
	    filestream >> parameter;

	    if (next.isNull() || parameter.isNull() || next != "=" ||
		parameter[0] != '\"' || parameter[parameter.length() - 1] != '\"') {
		qWarning("font parameter malformed '%s'", parameter.latin1());
		continue;
	    }

	    parameter = parameter.mid(1, parameter.length() - 2);

	    if (! styledata->fn) {
		styledata->fn = new TQFont;
	    }

	    styledata->fn->setRawName(parameter);
	} else if (next == "{") {
	    paren_count++;
	} else if (next == "}") {
	    paren_count--;
	}
    }

    styleDict.insert(styledata->name, styledata);

    return true;
}


KLegacyStyle::KLegacyStyle(void) : KStyle() {
    setButtonDefaultIndicatorWidth(6);
    setScrollBarExtent(15, 15);
    setButtonMargin(3);
    setSliderThickness(15);

    priv = new KLegacyStylePrivate;
}


KLegacyStyle::~KLegacyStyle(void) {
    delete priv;
}


int KLegacyStyle::defaultFrameWidth() const {
    return 2;
}


void KLegacyStyle::polish(TQApplication *app) {
    priv->oldfont = app->font();
    priv->oldpalette = app->palette();

    GtkObject *gobj = priv->gtkDict.tqfind(TQMainWindow::staticMetaObject());

    if (gobj) {
	if (gobj->font()) {
	    app->setFont(*gobj->font(), true);
	}

	TQPalette pal = app->palette();
	TQBrush brush;

	// background
	brush = pal.brush(TQPalette::Active, TQColorGroup::Background);
	brush.setColor(gobj->backColor(KLegacy::Active));
	pal.setBrush(TQPalette::Active, TQColorGroup::Background, brush);

	brush = pal.brush(TQPalette::Inactive, TQColorGroup::Background);
	brush.setColor(gobj->backColor(KLegacy::Normal));
	pal.setBrush(TQPalette::Inactive, TQColorGroup::Background, brush);

	brush = pal.brush(TQPalette::Disabled, TQColorGroup::Background);
	brush.setColor(gobj->backColor(KLegacy::Insensitive));
	pal.setBrush(TQPalette::Disabled, TQColorGroup::Background, brush);

	// foreground
	brush = pal.brush(TQPalette::Active, TQColorGroup::Foreground);
	brush.setColor(gobj->foreColor(KLegacy::Active));
	pal.setBrush(TQPalette::Active, TQColorGroup::Foreground, brush);

	brush = pal.brush(TQPalette::Inactive, TQColorGroup::Foreground);
	brush.setColor(gobj->foreColor(KLegacy::Normal));
	pal.setBrush(TQPalette::Inactive, TQColorGroup::Foreground, brush);

	brush = pal.brush(TQPalette::Disabled, TQColorGroup::Foreground);
	brush.setColor(gobj->foreColor(KLegacy::Insensitive));
	pal.setBrush(TQPalette::Disabled, TQColorGroup::Foreground, brush);

	// base
	brush = pal.brush(TQPalette::Active, TQColorGroup::Base);
	brush.setColor(gobj->baseColor(KLegacy::Normal));
	pal.setBrush(TQPalette::Active, TQColorGroup::Base, brush);

	brush = pal.brush(TQPalette::Inactive, TQColorGroup::Base);
	brush.setColor(gobj->baseColor(KLegacy::Normal));
	pal.setBrush(TQPalette::Inactive, TQColorGroup::Base, brush);

	brush = pal.brush(TQPalette::Disabled, TQColorGroup::Base);
	brush.setColor(gobj->baseColor(KLegacy::Normal));
	pal.setBrush(TQPalette::Disabled, TQColorGroup::Base, brush);

	// button
	brush = pal.brush(TQPalette::Active, TQColorGroup::Button);
	brush.setColor(gobj->backColor(KLegacy::Active));
	pal.setBrush(TQPalette::Active, TQColorGroup::Button, brush);

	brush = pal.brush(TQPalette::Normal, TQColorGroup::Button);
	brush.setColor(gobj->backColor(KLegacy::Normal));
	pal.setBrush(TQPalette::Normal, TQColorGroup::Button, brush);

	brush = pal.brush(TQPalette::Disabled, TQColorGroup::Button);
	brush.setColor(gobj->backColor(KLegacy::Insensitive));
	pal.setBrush(TQPalette::Disabled, TQColorGroup::Button, brush);

	// text
	brush = pal.brush(TQPalette::Active, TQColorGroup::Text);
	brush.setColor(gobj->foreColor(KLegacy::Active));
	pal.setBrush(TQPalette::Active, TQColorGroup::Text, brush);

	brush = pal.brush(TQPalette::Inactive, TQColorGroup::Text);
	brush.setColor(gobj->foreColor(KLegacy::Normal));
	pal.setBrush(TQPalette::Inactive, TQColorGroup::Text, brush);

	brush = pal.brush(TQPalette::Disabled, TQColorGroup::Text);
	brush.setColor(gobj->foreColor(KLegacy::Insensitive));
	pal.setBrush(TQPalette::Disabled, TQColorGroup::Text, brush);

	// highlight
	brush = pal.brush(TQPalette::Active, TQColorGroup::Highlight);
	brush.setColor(gobj->backColor(KLegacy::Selected));
	pal.setBrush(TQPalette::Active, TQColorGroup::Highlight, brush);

	brush = pal.brush(TQPalette::Active, TQColorGroup::Highlight);
	brush.setColor(gobj->backColor(KLegacy::Selected));
	pal.setBrush(TQPalette::Active, TQColorGroup::Highlight, brush);

	brush = pal.brush(TQPalette::Active, TQColorGroup::Highlight);
	brush.setColor(gobj->backColor(KLegacy::Selected));
	pal.setBrush(TQPalette::Active, TQColorGroup::Highlight, brush);

	// highlight text
	brush = pal.brush(TQPalette::Active, TQColorGroup::HighlightedText);
	brush.setColor(gobj->foreColor(KLegacy::Selected));
	pal.setBrush(TQPalette::Active, TQColorGroup::HighlightedText, brush);

	brush = pal.brush(TQPalette::Active, TQColorGroup::HighlightedText);
	brush.setColor(gobj->foreColor(KLegacy::Active));
	pal.setBrush(TQPalette::Active, TQColorGroup::HighlightedText, brush);

	brush = pal.brush(TQPalette::Active, TQColorGroup::HighlightedText);
	brush.setColor(gobj->foreColor(KLegacy::Active));
	pal.setBrush(TQPalette::Active, TQColorGroup::HighlightedText, brush);

	app->setPalette(pal, true);
    }

    qt_set_draw_menu_bar_impl((QDrawMenuBarItemImpl) &KLegacyStyle::drawMenuBarItem);

    KStyle::polish(app);
}


void KLegacyStyle::polish(TQWidget *widget) {
    if (qstrcmp(widget->name(), "qt_viewport") == 0 ||
	widget->testWFlags(WType_Popup) ||
	widget->inherits("KDesktop"))
	return;

    if (widget->backgroundMode() == TQWidget::PaletteBackground ||
	widget->backgroundMode() == TQWidget::PaletteButton &&
	(! widget->ownPalette()))
	widget->setBackgroundMode(TQWidget::X11ParentRelative);

    TQMetaObject *metaobject = 0;
    TQString detail;
    KLegacyImageDataKey key;
    key.cachekey = 0;

    bool eventFilter = false;
    bool mouseTrack = false;
    bool immediateRender = false;
    bool bgPixmap = false;

    if (widget->inherits(TQBUTTON_OBJECT_NAME_STRING)) {
	metaobject = TQButton::staticMetaObject();
	eventFilter = true;
    }

    if (widget->inherits(TQCOMBOBOX_OBJECT_NAME_STRING)) {
	metaobject = TQComboBox::staticMetaObject();
	eventFilter = true;
    }

    if (widget->inherits(TQSCROLLBAR_OBJECT_NAME_STRING)) {
	metaobject = TQScrollBar::staticMetaObject();
	eventFilter = true;
	mouseTrack = true;
    }

    if (widget->inherits(TQMENUBAR_OBJECT_NAME_STRING)) {
	eventFilter = true;
	immediateRender = true;

	metaobject = TQMenuBar::staticMetaObject();

	detail = "menubar";
	key.data.function = KLegacy::Box;
	key.data.shadow = KLegacy::Out;
	key.data.state = KLegacy::Normal;

	((TQMenuBar *) widget)->setFrameShape(TQFrame::StyledPanel);
	((TQMenuBar *) widget)->setLineWidth(0);
	widget->setBackgroundMode(TQWidget::PaletteBackground);
    }

    if (widget->inherits(TQTOOLBAR_OBJECT_NAME_STRING)) {
	metaobject = TQToolBar::staticMetaObject();

	eventFilter = true;
	immediateRender = true;

	detail = "menubar";
	key.data.function = KLegacy::Box;
	key.data.shadow = KLegacy::Out;
	key.data.state = KLegacy::Normal;

	widget->setBackgroundMode(TQWidget::PaletteBackground);
    }

    if (widget->inherits(TQLINEEDIT_OBJECT_NAME_STRING)) {
	metaobject = TQLineEdit::staticMetaObject();

	eventFilter = true;
	immediateRender = true;

	detail = "entry_bg";
	key.data.function = KLegacy::FlatBox;
	key.data.shadow = KLegacy::NoShadow;
	key.data.state = (widget->isEnabled()) ? KLegacy::Normal : KLegacy::Insensitive;

	widget->setBackgroundMode(TQWidget::PaletteBase);
    }

    if (widget->isTopLevel() || widget->inherits("QWorkspaceChild")) {
	immediateRender = true;

	bgPixmap = true;
	metaobject = TQMainWindow::staticMetaObject();
	key.cachekey = 0;
	key.data.function = KLegacy::FlatBox;
	detail = "base";
    }

    if (widget->inherits(TQPOPUPMENU_OBJECT_NAME_STRING)) {
	qDebug("polishing popup '%s'", widget->className());
	metaobject = TQPopupMenu::staticMetaObject();
	widget->setBackgroundMode(TQWidget::PaletteBackground);
    }

    GtkObject *gobj = gobj = priv->gtkDict.tqfind(((metaobject) ? metaobject :
						 widget->tqmetaObject()));

    if (gobj) {
	if (gobj->font() && (*gobj->font() != TQApplication::font()))
	    widget->setFont(*gobj->font());

	if (immediateRender) {
	    TQPixmap *pix = gobj->draw(key, widget->width(), widget->height(), detail);

	    if (pix && ! pix->isNull()) {
		if (! bgPixmap) {
		    TQPalette pal = widget->palette();
		    TQBrush brush;

		    // base
		    // active
		    brush = pal.brush(TQPalette::Active,
				      TQColorGroup::Base);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Active,
				 TQColorGroup::Base, brush);

		    // inactive
		    brush = pal.brush(TQPalette::Inactive,
				      TQColorGroup::Base);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Inactive,
				 TQColorGroup::Base, brush);

		    // disabled
		    brush = pal.brush(TQPalette::Disabled,
				      TQColorGroup::Base);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Disabled,
				 TQColorGroup::Base, brush);

		    // background - button
		    // active
		    brush = pal.brush(TQPalette::Active,
				      TQColorGroup::Background);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Active,
				 TQColorGroup::Background, brush);

		    brush = pal.brush(TQPalette::Active,
				      TQColorGroup::Button);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Active,
				 TQColorGroup::Button, brush);

		    // inactive
		    brush = pal.brush(TQPalette::Inactive,
				      TQColorGroup::Background);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Inactive,
				 TQColorGroup::Background, brush);

		    brush = pal.brush(TQPalette::Inactive,
				      TQColorGroup::Button);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Inactive,
				 TQColorGroup::Button, brush);

		    // disabled
		    brush = pal.brush(TQPalette::Disabled,
				      TQColorGroup::Background);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Disabled,
				 TQColorGroup::Background, brush);

		    brush = pal.brush(TQPalette::Disabled,
				      TQColorGroup::Button);
		    brush.setPixmap(*pix);
		    pal.setBrush(TQPalette::Disabled,
				 TQColorGroup::Button, brush);

		    widget->setPalette(pal);
		} else
		    widget->setBackgroundPixmap(*pix);
	    }
	}
    }

    if (eventFilter) {
	widget->installEventFilter(this);
    }

    if (mouseTrack) {
 	widget->setMouseTracking(mouseTrack);
    }

    KStyle::polish(widget);
}


void KLegacyStyle::polishPopupMenu(TQPopupMenu *popup) {
    KStyle::polishPopupMenu(popup);

    popup->setMouseTracking(true);
    popup->setCheckable(true);

    popup->installEventFilter(this);
}


void KLegacyStyle::unPolish(TQWidget *widget) {
    if (widget->inherits("KDesktop"))
	return;

    widget->setBackgroundOrigin(TQWidget::WidgetOrigin);
    widget->setBackgroundPixmap(TQPixmap());
    widget->removeEventFilter(this);
    widget->unsetPalette();
    widget->setAutoMask(false);
    KStyle::unPolish(widget);
}


void KLegacyStyle::unPolish(TQApplication *app) {
    app->setFont(priv->oldfont, true);
    app->setPalette(priv->oldpalette, true);

    qt_set_draw_menu_bar_impl(0);

    KStyle::unPolish(app);
}


void KLegacyStyle::drawKMenuItem(TQPainter *p, int x, int y, int w, int h, const TQColorGroup &g,
				 bool active, TQMenuItem *mi, TQBrush *)
{
    drawMenuBarItem(p, x, y, w, h, mi, (TQColorGroup &) g,
		    (mi) ? mi->isEnabled() : false, active);
}


void KLegacyStyle::drawMenuBarItem(TQPainter *p, int x, int y, int w, int h, TQMenuItem *mi,
				   TQColorGroup &g, bool enabled, bool active)
{
    if (enabled && active) {
	GtkObject *gobj = priv->gtkDict.tqfind(&menuitem_ptr);

	if (gobj) {
	    KLegacyImageDataKey key;
	    key.cachekey = 0;
	    key.data.function = KLegacy::Box;
	    key.data.state = KLegacy::Prelight;
	    key.data.shadow = KLegacy::Out;

	    TQPixmap *pix = gobj->draw(key, w, h, "menuitem");

	    if (pix && ! pix->isNull())
		p->drawPixmap(x, y, *pix);
	}
    }

    drawItem(p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
	     g, enabled, mi->pixmap(), mi->text(), -1, &g.buttonText());
}


void KLegacyStyle::drawButton(TQPainter *p, int x, int y , int w, int h,
			      const TQColorGroup &g, bool sunken, const TQBrush *fill)
{
    drawBevelButton(p, x, y, w, h, g, sunken, fill);
}


void KLegacyStyle::drawBevelButton(TQPainter *p, int x, int y, int w, int h,
				const TQColorGroup & g, bool sunken,
				const TQBrush *fill)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQButton::staticMetaObject());

    if (! gobj) {
	KStyle::drawBevelButton(p, x, y, w, h, g, sunken, fill);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    key.data.shadow = (sunken) ? KLegacy::In : KLegacy::Out;
    key.data.state = (sunken) ? KLegacy::Active : KLegacy::Normal;

    TQPixmap *pix = gobj->draw(key, w, h, "button");

    if (pix && (! pix->isNull()))
	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawBevelButton(p, x, y, w, h, g, sunken, fill);
}


void KLegacyStyle::drawPushButton(TQPushButton *btn, TQPainter *p) {
    GtkObject *gobj = priv->gtkDict.tqfind(TQPushButton::staticMetaObject());

    if (! gobj) {
	KStyle::drawPushButton(btn, p);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;

    TQColorGroup g = btn->tqcolorGroup();
    TQBrush fill = g.brush(TQColorGroup::Button);
    int x1, y1, x2, y2;
    btn->rect().coords(&x1, &y1, &x2, &y2);

    if (btn->isDefault()) {
	// draw default button
	key.data.state = (btn->isEnabled()) ? KLegacy::Normal : KLegacy::Insensitive;
	key.data.shadow = KLegacy::In;

	TQPixmap *pix = gobj->draw(key, x2 -x1 + 1, y2 - y1 + 1, "buttondefault");

	if (! pix)
	    pix = gobj->draw(key, x2 - x1 + 1, y2 - y1 + 1, "button");

	if (pix)
	    p->drawPixmap(x1, y1, *pix);
        else
	    KStyle::drawBevelButton(p, x1, y1, x2 - x1 + 1, y2 - y1 + 1,
				    g, true, &fill);
    }

    int diw = buttonDefaultIndicatorWidth();
    if (btn->isDefault() || btn->autoDefault()) {
	x1 += diw;
	y1 += diw;
	x2 -= diw;
	y2 -= diw;
    }

    if (btn->isOn() || btn->isDown()) {
	key.data.state = KLegacy::Active;
	key.data.shadow = KLegacy::In;
    } else {
	key.data.state = ((btn->isEnabled()) ?
			  ((static_cast<TQWidget *>( btn ) == priv->lastWidget) ? KLegacy::Prelight : KLegacy::Normal) :
			  KLegacy::Insensitive);
	key.data.shadow = ((btn->isOn() || btn->isDown()) ?
			   KLegacy::In : KLegacy::Out);
    }

    TQPixmap *pix = gobj->draw(key, x2 - x1 + 1, y2 - y1 + 1, "button");

    if (pix && ! pix->isNull())
	p->drawPixmap(x1, y1, *pix);
    else {
	KStyle::drawBevelButton(p, x1, y1, x2 - x1 + 1, y2 - y1 + 1,
				g, btn->isOn() || btn->isDown(), &fill);
    }
}


void KLegacyStyle::drawIndicator(TQPainter *p, int x, int y, int w, int h,
			      const TQColorGroup &g, int state,
			      bool down, bool enabled)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQCheckBox::staticMetaObject());

    if (! gobj) {
	KStyle::drawIndicator(p, x, y, w, h, g, state, down, enabled);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Check;
    key.data.state = KLegacy::Normal;
    key.data.shadow = ((state != TQButton::Off) || down) ? KLegacy::In : KLegacy::Out;

    TQPixmap *pix = gobj->draw(key, w, h, "checkbutton");

    if (pix && (! pix->isNull()))
     	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawIndicator(p, x, y, w, h, g, state, down, enabled);
}


void KLegacyStyle::drawIndicatorMask(TQPainter *p, int x, int y, int w, int h, int state) {
    GtkObject *gobj = priv->gtkDict.tqfind(TQCheckBox::staticMetaObject());

    if (! gobj) {
	KStyle::drawIndicatorMask(p, x, y, w, h, state);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Check;
    key.data.state = KLegacy::Normal;
    key.data.shadow = (state != TQButton::Off) ? KLegacy::In : KLegacy::Out;

    TQPixmap *pix = gobj->draw(key, w, h, "checkbutton");

    if (pix && pix->mask() && ! pix->mask()->isNull())
	p->drawPixmap(x, y, *(pix->mask()));
    else
	KStyle::drawIndicatorMask(p, x, y, w, h, state);
}


TQSize KLegacyStyle::indicatorSize(void) const {
    GtkObject *gobj = priv->gtkDict.tqfind(TQCheckBox::staticMetaObject());

    if (! gobj)	return KStyle::indicatorSize();

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Check;
    key.data.shadow = KLegacy::Out;
    KLegacyImageData *id = gobj->getImageData(key, "checkbutton");

    if (! id) return KStyle::indicatorSize();

    TQString filename;
    if (! id->file.isNull())
	filename = id->file;
    else if (! id->overlayFile.isNull())
	filename = id->overlayFile;
    else
	return KStyle::indicatorSize();

    TQImage *image = imageCache->find(filename);
    if (! image) {
        image = new TQImage(filename);

        if (! image) return KStyle::indicatorSize();

        imageCache->insert(filename, image);
    }

    return TQSize(image->width(), image->height());
}


void KLegacyStyle::drawExclusiveIndicator(TQPainter *p, int x, int y, int w, int h,
				      const TQColorGroup &g, bool on,
				       bool down, bool enabled)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQRadioButton::staticMetaObject());

    if (! gobj) {
	drawExclusiveIndicator(p, x, y, w, h, g, on, down, enabled);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Option;
    key.data.state = KLegacy::Normal;
    key.data.shadow = (on || down) ? KLegacy::In : KLegacy::Out;

    TQPixmap *pix = gobj->draw(key, w, h, "radiobutton");

    if (pix && (! pix->isNull()))
	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawExclusiveIndicator(p, x, y, w, h, g, down, enabled);
}


void KLegacyStyle::drawExclusiveIndicatorMask(TQPainter *p, int x, int y, int w, int h,
					   bool on)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQRadioButton::staticMetaObject());

    if (! gobj) {
	KStyle::drawExclusiveIndicatorMask(p, x, y, w, h, on);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Option;
    key.data.state = KLegacy::Normal;
    key.data.shadow = (on) ? KLegacy::In : KLegacy::Out;

    TQPixmap *pix = gobj->draw(key, w, h, "radiobutton");

    if (pix && pix->mask() && ! pix->mask()->isNull())
	p->drawPixmap(x, y, *(pix->mask()));
    else
	KStyle::drawExclusiveIndicatorMask(p, x, y, w, h, on);
}


TQSize KLegacyStyle::exclusiveIndicatorSize(void) const {
    GtkObject *gobj = priv->gtkDict.tqfind(TQRadioButton::staticMetaObject());

    if (! gobj) {
	return KStyle::indicatorSize();
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Option;
    key.data.shadow = KLegacy::Out;
    KLegacyImageData *id = gobj->getImageData(key, "radiobutton");

    if (! id) return KStyle::indicatorSize();

    TQString filename;
    if (! id->file.isNull()) {
	filename = id->file;
    } else if (! id->overlayFile.isNull()) {
	filename = id->overlayFile;
    } else {
	return KStyle::indicatorSize();
    }

    TQImage *image = imageCache->find(filename);
    if (! image) {
        image = new TQImage(filename);

        if (! image) {
	    return KStyle::indicatorSize();
	}

        imageCache->insert(filename, image);
    }

    return TQSize(image->width(), image->height());
}


void KLegacyStyle::drawPopupMenuItem(TQPainter *p, bool checkable, int maxpmw, int tab,
				  TQMenuItem *mi, const TQPalette &pal, bool act,
				  bool enabled, int x, int y, int w, int h)
{
    const TQColorGroup & g = pal.active();
    TQColorGroup itemg = (! enabled) ? pal.disabled() : pal.active();

    if (checkable)
	maxpmw = QMAX(maxpmw, 15);

    int checkcol = maxpmw;

    if (mi && mi->isSeparator()) {
	p->setPen( g.dark() );
	p->drawLine( x, y, x+w, y );
	p->setPen( g.light() );
	p->drawLine( x, y+1, x+w, y+1 );
	return;
    }

    if ( act && enabled ) {
	GtkObject *gobj = priv->gtkDict.tqfind(&menuitem_ptr);

	if (gobj) {
	    KLegacyImageDataKey key;
	    key.cachekey = 0;
	    key.data.function = KLegacy::Box;
	    key.data.state = KLegacy::Prelight;
	    key.data.shadow = KLegacy::Out;

	    TQPixmap *pix = gobj->draw(key, w, h, "menuitem");

	    if (pix && ! pix->isNull())
		p->drawPixmap(x, y, *pix);
	}
    } else
	p->fillRect(x, y, w, h, g.brush( TQColorGroup::Button ));

    if ( !mi )
	return;

    if ( mi->isChecked() ) {
	if ( mi->iconSet() ) {
	    qDrawShadePanel( p, x+2, y+2, checkcol, h-2*2,
			     g, true, 1, &g.brush( TQColorGroup::Midlight ) );
	}
    } else if ( !act ) {
	p->fillRect(x+2, y+2, checkcol, h-2*2,
		    g.brush( TQColorGroup::Button ));
    }

    if ( mi->iconSet() ) {		// draw iconset
	TQIconSet::Mode mode = (enabled) ? TQIconSet::Normal : TQIconSet::Disabled;

	if (act && enabled)
	    mode = TQIconSet::Active;

	TQPixmap pixmap = mi->iconSet()->pixmap(TQIconSet::Small, mode);

	int pixw = pixmap.width();
	int pixh = pixmap.height();

	TQRect cr( x + 2, y+2, checkcol, h-2*2 );
	TQRect pmr( 0, 0, pixw, pixh );

	pmr.moveCenter(cr.center());

	p->setPen( itemg.text() );
	p->drawPixmap( pmr.topLeft(), pixmap );

    } else  if (checkable) {
	int mw = checkcol;
	int mh = h - 4;

	if (mi->isChecked())
	    drawCheckMark(p, x+2, y+2, mw, mh, itemg, act, ! enabled);
    }

    p->setPen( g.buttonText() );

    TQColor discol;
    if (! enabled) {
	discol = itemg.text();
	p->setPen( discol );
    }

    if (mi->custom()) {
	p->save();
	mi->custom()->paint(p, itemg, act, enabled, x + checkcol + 4, y + 2,
			    w - checkcol - tab - 3, h - 4);
	p->restore();
    }

    TQString s = mi->text();
    if ( !s.isNull() ) {			// draw text
	int t = s.tqfind( '\t' );
	int m = 2;
	const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
	if ( t >= 0 ) {				// draw tab text
	    p->drawText( x+w-tab-2-2,
			 y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
	}
	p->drawText(x + checkcol + 4, y + 2, w - checkcol -tab - 3, h - 4,
		    text_flags, s, t);
    } else if (mi->pixmap()) {
	TQPixmap *pixmap = mi->pixmap();

	if (pixmap->depth() == 1) p->setBackgroundMode(OpaqueMode);
	p->drawPixmap(x + checkcol + 2, y + 2, *pixmap);
	if (pixmap->depth() == 1) p->setBackgroundMode(TransparentMode);
    }

    if (mi->popup()) {
	int hh = h / 2;

	drawMenuArrow(p, RightArrow, (act) ? mi->isEnabled() : false,
		      x + w - hh - 6, y + (hh / 2), hh, hh, g, mi->isEnabled());
    }
}


void KLegacyStyle::drawComboButton(TQPainter *p, int x, int y, int w, int h,
				const TQColorGroup &g, bool sunken, bool editable,
				bool enabled, const TQBrush *b)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQComboBox::staticMetaObject());

    if (! gobj) {
	KStyle::drawComboButton(p, x, y, w, h, g, sunken, editable, enabled, b);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    key.data.state = KLegacy::Normal;
    key.data.shadow = KLegacy::Out;

    if (priv->lastWidget && priv->lastWidget->inherits(TQCOMBOBOX_OBJECT_NAME_STRING))
	key.data.state = KLegacy::Prelight;

    TQPixmap *pix = gobj->draw(key, w, h, "optionmenu");

    if (pix && ! pix->isNull()) {
	p->drawPixmap(x, y, *pix);
    } else {
	KStyle::drawComboButton(p, x, y, w, h, g, sunken, editable, enabled, b);
	return;
    }

    TQRect rect = comboButtonRect(x, y, w, h);
    int tw = w - rect.width() - rect.right() - rect.left();
    int th = rect.height();

    key.data.function = KLegacy::Tab;
    key.data.state = KLegacy::Normal;
    pix = gobj->draw(key, tw, th, "optionmenutab");

    if (pix && ! pix->isNull())
	p->drawPixmap(x + rect.width() + rect.left() + ((18 - pix->width()) / 2),
		      y + rect.y() + ((rect.height() - pix->height()) / 2), *pix);
}


TQRect KLegacyStyle::comboButtonRect(int x, int y, int w, int h) {
    GtkObject *gobj = priv->gtkDict.tqfind(TQComboBox::staticMetaObject());

    if (! gobj) {
	return KStyle::comboButtonRect(x, y, w, h);
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    KLegacyImageData *id = gobj->getImageData(key, "optionmenu");

    if (! id) {
	return KStyle::comboButtonRect(x, y, w, h);
    }

    return TQRect(x + id->border.left() + 1, y + id->border.top() + 1,
		 w - id->border.left() - id->border.right() - 18,
		 h - id->border.top() - id->border.bottom() - 2);
}


TQRect KLegacyStyle::comboButtonFocusRect(int x, int  y, int w, int h) {
    return comboButtonRect(x, y, w, h);
}


TQStyle::ScrollControl KLegacyStyle::scrollBarPointOver(const TQScrollBar *scrollbar,
						       int sliderStart, const TQPoint &p)
{
    return TQCommonStyle::scrollBarPointOver(scrollbar, sliderStart, p);
}


void KLegacyStyle::scrollBarMetrics(const TQScrollBar *scrollbar, int &sliderMin,
				 int &sliderMax, int &sliderLength, int &buttonDim)
{
    int maxLength;
    int b = defaultFrameWidth();
    int length = ((scrollbar->orientation() == TQScrollBar::Horizontal) ?
		  scrollbar->width() : scrollbar->height());
    int extent = ((scrollbar->orientation() == TQScrollBar::Horizontal) ?
		  scrollbar->height() : scrollbar->width());

    if (length > ((extent - (b * 2) - 1) * 2) + (b * 2))
	buttonDim = extent - (b * 2);
    else
	buttonDim = ((length - (b * 2)) / 2) - 1;

    sliderMin = b + buttonDim;
    maxLength = length - (b * 2) - (buttonDim * 2);

    if (scrollbar->maxValue() == scrollbar->minValue()) {
	sliderLength = maxLength - 2;
    } else {
	uint range = scrollbar->maxValue() - scrollbar->minValue();

	sliderLength = (scrollbar->pageStep() * maxLength) /
		       (range + scrollbar->pageStep());

	if (sliderLength < buttonDim || range > (INT_MAX / 2))
	    sliderLength = buttonDim;
	if (sliderLength >= maxLength)
	    sliderLength = maxLength - 2;
    }

    sliderMax = sliderMin + maxLength - sliderLength;

    sliderMin += 1;
    sliderMax -= 1;
}


void KLegacyStyle::drawScrollBarControls(TQPainter *p, const TQScrollBar *scrollbar,
				     int start, uint controls, uint active)
{
    if (! scrollbar->isVisible()) return;

    GtkObject *gobj = priv->gtkDict.tqfind(TQScrollBar::staticMetaObject());

    if (! gobj) {
	KStyle::drawScrollBarControls(p, scrollbar, start, controls, active);
	return;
    }

    KLegacyImageDataKey gkey;
    gkey.cachekey = 0;
    gkey.data.function = KLegacy::Box;
    gkey.data.orientation = scrollbar->orientation() + 1;

    KLegacyImageData *groove_id = gobj->getImageData(gkey, "trough");

    if (! groove_id) {
	KStyle::drawScrollBarControls(p, scrollbar, start, controls, active);
	return;
    }

    int sliderMin;
    int sliderMax;
    int sliderLen;
    int buttonDim;
    scrollBarMetrics(scrollbar, sliderMin, sliderMax, sliderLen, buttonDim);

    // the rectangle for the slider
    TQRect slider(
		 // x
		 ((scrollbar->orientation() == Vertical) ?
		  defaultFrameWidth() : start),

		 // y
		 ((scrollbar->orientation() == Vertical) ?
		  start : defaultFrameWidth()),

		 // w
		 ((scrollbar->orientation() == Vertical) ?
		  buttonDim : sliderLen),

		 // h
		 ((scrollbar->orientation() == Vertical) ?
		  sliderLen : buttonDim));

    KLegacyImageDataKey skey;
    skey.cachekey = 0;
    skey.data.function = KLegacy::Box;
    skey.data.orientation = scrollbar->orientation() + 1;

    if ((active & Slider) || (priv->hovering && slider.contains(priv->mousePos)))
	skey.data.state = KLegacy::Prelight;
    else
	skey.data.state = KLegacy::Normal;

    KLegacyImageData *slider_id = gobj->getImageData(skey, "slider");

    if (! slider_id) {
	KStyle::drawScrollBarControls(p, scrollbar, start, controls, active);
	return;
    }

    TQPixmap *groove_pm = gobj->draw(groove_id, scrollbar->width(), scrollbar->height());

    if ((! groove_pm) || (groove_pm->isNull())) {
	groove_pm = 0;
    }

    TQPixmap *slider_pm = gobj->draw(slider_id, slider.width(), slider.height());

    if ((! slider_pm) || (slider_pm->isNull())) {
	slider_pm = 0;
    }

    TQPixmap buf(scrollbar->size());
    {
	TQPainter p2(&buf);

	if (groove_pm) {
	    p2.drawTiledPixmap(scrollbar->rect(), *groove_pm);
	}

	if (slider_pm) {
	    p2.drawTiledPixmap(slider, *slider_pm);
	}

	// arrows
	int x, y;
	x = y = defaultFrameWidth();

	drawArrow(&p2, ((scrollbar->orientation() == Vertical) ?
			UpArrow : LeftArrow),
		  (active & SubLine), x, y,
		  buttonDim,
		  buttonDim,
		  scrollbar->tqcolorGroup(), true);

	if  (scrollbar->orientation() == Vertical)
	    y = scrollbar->height() - buttonDim - defaultFrameWidth();
	else
	    x = scrollbar->width()  - buttonDim - defaultFrameWidth();

	drawArrow(&p2, ((scrollbar->orientation() == Vertical) ?
			DownArrow : RightArrow),
		  (active & AddLine), x, y,
		  buttonDim,
		  buttonDim,
		  scrollbar->tqcolorGroup(), true);
    }
    p->drawPixmap(0, 0, buf);
}


void KLegacyStyle::drawSlider(TQPainter *p, int x, int y, int w, int h, const TQColorGroup &g,
			   Orientation orientation, bool tickAbove, bool tickBelow)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQSlider::staticMetaObject());

    if (! gobj) {
	KStyle::drawSlider(p, x, y, w, h, g, orientation, tickAbove, tickBelow);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    key.data.shadow = KLegacy::Out;
    key.data.state = KLegacy::Normal;
    key.data.orientation = orientation + 1;

    TQPixmap *pix = gobj->draw(key, w, h, "slider");

    if (pix && ! pix->isNull())
	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawSlider(p, x, y, w, h, g, orientation, tickAbove, tickBelow);
}



void KLegacyStyle::drawSliderGroove(TQPainter *p, int x, int y, int w, int h,
				const TQColorGroup &g, QCOORD c, Orientation o)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQSlider::staticMetaObject());

    if (! gobj) {
	KStyle::drawSliderGroove(p, x, y, w, h, g, c, o);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    key.data.shadow = KLegacy::In;
    key.data.state = KLegacy::Active;
    key.data.orientation = o + 1;

    TQPixmap *pix = gobj->draw(key, w, h, "trough");

    if (pix && ! pix->isNull())
	p->drawPixmap(x, y, *pix);
    else
        KStyle::drawSliderGroove(p, x, y, w, h, g, c, o);
}


void KLegacyStyle::drawArrow(TQPainter *p, ArrowType type, bool down,
			 int x, int y, int w, int h,
			 const TQColorGroup &g, bool enabled, const TQBrush *b)
{
    GtkObject *gobj = priv->gtkDict.tqfind(&arrow_ptr);

    if (! gobj) {
	KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, b);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Arrow;
    key.data.state = (down) ? KLegacy::Active : KLegacy::Normal;
    key.data.shadow = (down) ? KLegacy::In : KLegacy::NoShadow;
    key.data.arrowDirection = type + 1;

    if ((! down) && priv->hovering &&
	TQRect(x, y, w, h).contains(priv->mousePos)) {
	key.data.state = KLegacy::Prelight;
    }

    TQPixmap *pix = gobj->draw(key, w, h, "arrow");

    if (pix && ! pix->isNull())
	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, b);
}


void KLegacyStyle::drawMenuArrow(TQPainter *p, ArrowType type, bool down,
			 int x, int y, int w, int h,
			 const TQColorGroup &g, bool enabled, const TQBrush *b)
{
    GtkObject *gobj = priv->gtkDict.tqfind(&menuitem_ptr);

    if (! gobj) {
	KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, b);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Arrow;
    key.data.state = (down) ? KLegacy::Active : KLegacy::Normal;
    key.data.shadow = (down) ? KLegacy::In : KLegacy::NoShadow;
    key.data.arrowDirection = type + 1;

    TQPixmap *pix = gobj->draw(key, w, h, "arrow");

    if (pix && ! pix->isNull())
    	p->drawPixmap(x + ((w - pix->width()) / 2),
	              y + ((h - pix->height()) / 2), *pix);
    else
	KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, b);
}


void KLegacyStyle::drawPanel(TQPainter *p, int x, int y, int w, int h,
			  const TQColorGroup &g, bool sunken, int, const TQBrush *brush)
{
    KStyle::drawPanel(p, x, y, w, h, g, sunken, 1, brush);
}


void KLegacyStyle::drawPopupPanel(TQPainter *p, int x, int y, int w, int h,
				  const TQColorGroup &g, int, const TQBrush *fill)
{
    TQBrush brush = (fill) ? *fill : g.brush(TQColorGroup::Background);

    p->fillRect(x, y, w, h, brush);
}


void KLegacyStyle::drawCheckMark(TQPainter *p, int x, int y, int w, int h,
			      const TQColorGroup &g, bool activated, bool disabled)
{
    GtkObject *gobj = priv->gtkDict.tqfind(&checkmenuitem_ptr);

    if (! gobj) {
	KStyle::drawCheckMark(p, x, y, w, h, g, activated, disabled);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Check;
    key.data.shadow = (disabled) ? KLegacy::Out : KLegacy::In;

    TQPixmap *pix = gobj->draw(key, w, h);

    if (pix && (! pix->isNull())) {
	x += (w - pix->width()) / 2;
	y += (h - pix->height()) / 2;
	p->drawPixmap(x, y, *pix);
    } else {
	KStyle::drawCheckMark(p, x, y, w, h, g, activated, disabled);
    }
}


void KLegacyStyle::drawSplitter(TQPainter *p, int x, int y, int w, int h,
			     const TQColorGroup &g, Orientation orientation)
{
    if (orientation == Horizontal) {
	int xpos = x + (w / 2);
	int kpos = 10;
	int ksize = splitterWidth() - 2;

	qDrawShadeLine(p, xpos, kpos + ksize - 1, xpos, h, g);
	drawBevelButton(p, xpos - (splitterWidth() / 2) + 1, kpos, ksize, ksize,
			g, false, &g.brush(TQColorGroup::Button));
	qDrawShadeLine(p, xpos, 0, xpos, kpos, g);
    } else {
	int ypos = y + (h / 2);
	int kpos = w - 10 - splitterWidth();
	int ksize = splitterWidth() - 2;

	qDrawShadeLine(p, 0, ypos, kpos, ypos, g);
	drawBevelButton(p, kpos, ypos - (splitterWidth() / 2) + 1, ksize, ksize,
			g, false, &g.brush(TQColorGroup::Button));
	qDrawShadeLine(p, kpos + ksize - 1, ypos, w, ypos, g);
    }
}


void KLegacyStyle::drawTab(TQPainter *p, const TQTabBar *tabbar, TQTab *tab, bool selected)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQTabBar::staticMetaObject());

    if (! gobj) {
	KStyle::drawTab(p, tabbar, tab, selected);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Extension;
    key.data.state = (! selected) ? KLegacy::Active : KLegacy::Normal;
    key.data.shadow = KLegacy::Out;
    key.data.gapSide = (tabbar->tqshape() == TQTabBar::RoundedAbove ||
			tabbar->tqshape() == TQTabBar::TriangularAbove) ?
		       KLegacy::Bottom : KLegacy::Top;

    int ry = tab->r.top(), rh = tab->r.height();

    if (! selected) {
	rh -= 2;

	if (tabbar->tqshape() == TQTabBar::RoundedAbove ||
	    tabbar->tqshape() == TQTabBar::TriangularAbove)
	    ry += 2;
    }

    TQPixmap *pix = gobj->draw(key, tab->r.width(), rh, "tab");


    if (pix && ! pix->isNull())
	p->drawPixmap(tab->r.left(), ry, *pix);
    else
	KStyle::drawTab(p, tabbar, tab, selected);
}


void KLegacyStyle::drawKBarHandle(TQPainter *p, int x, int y, int w, int h,
				  const TQColorGroup &g, KToolBarPos type, TQBrush *fill)
{
    GtkObject *gobj = priv->gtkDict.tqfind(TQToolBar::staticMetaObject());

    if (! gobj) {
	KStyle::drawKBarHandle(p, x, y, w, h, g, type, fill);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Handle;
    key.data.state = KLegacy::Normal;
    key.data.shadow = KLegacy::Out;
    key.data.orientation = (type == Left || type == Right) ?
			   Vertical + 1: Horizontal + 1;

    TQPixmap *pix = gobj->draw(key, w, h, "handle");

    if (pix && ! pix->isNull())
	p->drawPixmap(x, y, *pix);
}


void KLegacyStyle::drawKickerHandle(TQPainter *p, int x, int y, int w, int h,
				    const TQColorGroup &g, TQBrush *fill)
{
    drawKBarHandle(p, x, y, w, h, g, Left, fill);
}


void KLegacyStyle::drawKickerAppletHandle(TQPainter *p, int x, int y, int w, int h,
					  const TQColorGroup &g, TQBrush *fill)
{
    drawKBarHandle(p, x, y, w, h, g, Left, fill);
}


void KLegacyStyle::drawKickerTaskButton(TQPainter *p, int x, int y, int w, int h,
					const TQColorGroup &g, const TQString &title,
					bool active, TQPixmap *icon, TQBrush *fill)
{
    drawBevelButton(p, x, y, w, h, g, active, fill);

    const int pxWidth = 20;
    int textPos = pxWidth;

    TQRect br(buttonRect(x, y, w, h));

    if (active)
        p->translate(1,1);

    if (icon && ! icon->isNull()) {
	int dx = (pxWidth - icon->width()) / 2;
	int dy = (h - icon->height()) / 2;

        p->drawPixmap(br.x() + dx, dy, *icon);
    }

    TQString s(title);

    static const TQString &modStr = KGlobal::staticQString(
        TQString::fromUtf8("[") + i18n("modified") + TQString::fromUtf8("]"));

    int modStrPos = s.tqfind(modStr);

    if (modStrPos != -1) {
        s.remove(modStrPos, modStr.length()+1);

	TQPixmap modPixmap = SmallIcon("modified");

	int dx = (pxWidth - modPixmap.width()) / 2;
	int dy = (h - modPixmap.height()) / 2;

	p->drawPixmap(br.x() + textPos + dx, dy, modPixmap);

	textPos += pxWidth;
    }

    if (! s.isEmpty()) {
	if (p->fontMetrics().width(s) > br.width() - textPos) {
	    int maxLen = br.width() - textPos - p->fontMetrics().width("...");

	    while ( (! s.isEmpty()) && (p->fontMetrics().width(s) > maxLen))
		s.truncate(s.length() - 1);

	    s.append("...");
	}

	p->setPen((active) ? g.foreground() : g.buttonText());

	p->drawText(br.x() + textPos, -1, w - textPos, h, AlignVCenter | AlignLeft, s);
    }
}


bool KLegacyStyle::eventFilter(TQObject *obj, TQEvent *e) {
    switch (e->type()) {
    case TQEvent::Resize:
	{
	    TQWidget *w = (TQWidget *) obj;

	    if (w->inherits(TQPOPUPMENU_OBJECT_NAME_STRING) && w->width() < 700) {
		GtkObject *gobj = priv->gtkDict.tqfind(TQPopupMenu::staticMetaObject());

		if (gobj) {
		    KLegacyImageDataKey key;
		    key.cachekey = 0;
		    key.data.function = KLegacy::Box;
		    key.data.state = KLegacy::Normal;
		    key.data.shadow = KLegacy::Out;

		    TQPixmap *pix = gobj->draw(key, w->width(), w->height(), "menu");

		    if (pix && ! pix->isNull()) {
			TQPalette pal = w->palette();

			// active
			TQBrush brush = pal.brush(TQPalette::Active,
						 TQColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Active,
				     TQColorGroup::Background, brush);

			brush = pal.brush(TQPalette::Active,
					  TQColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Active,
				     TQColorGroup::Button, brush);

			// inactive
			brush = pal.brush(TQPalette::Inactive,
					  TQColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Inactive,
				     TQColorGroup::Background, brush);

			brush = pal.brush(TQPalette::Inactive,
					  TQColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Inactive,
				     TQColorGroup::Button, brush);

			// disabled
			brush = pal.brush(TQPalette::Disabled,
					  TQColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Disabled,
				     TQColorGroup::Background, brush);

			brush = pal.brush(TQPalette::Disabled,
					  TQColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Disabled,
				     TQColorGroup::Button, brush);

			w->setPalette(pal);
		    }
		}
	    } else if (w->isTopLevel() || w->inherits("QWorkspaceChild")) {
		GtkObject *gobj = priv->gtkDict.tqfind(TQMainWindow::staticMetaObject());

		if (gobj) {
		    KLegacyImageDataKey key;
		    key.cachekey = 0;
		    key.data.function = KLegacy::FlatBox;

		    TQPixmap *p = gobj->draw(key, w->width(), w->height(), "base");

		    if (p && (! p->isNull()))
			w->setBackgroundPixmap(*p);
		}
	    } else if (w->inherits(TQLINEEDIT_OBJECT_NAME_STRING)) {
		GtkObject *gobj = priv->gtkDict.tqfind(TQLineEdit::staticMetaObject());

		if (gobj) {
		    KLegacyImageDataKey key;
		    key.cachekey = 0;
		    key.data.function = KLegacy::FlatBox;
		    key.data.shadow = KLegacy::NoShadow;
		    key.data.state = (w->isEnabled()) ? KLegacy::Normal : KLegacy::Insensitive;

		    TQPixmap *pix = gobj->draw(key, w->width(), w->height(),
					      "entry_bg");

		    if (pix && (! pix->isNull())) {
			TQPalette pal = w->palette();

			// active
			TQBrush brush = pal.brush(TQPalette::Active,
						 TQColorGroup::Base);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Active,
				     TQColorGroup::Base, brush);

			// inactive
			brush = pal.brush(TQPalette::Inactive,
					  TQColorGroup::Base);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Inactive,
				     TQColorGroup::Base, brush);

			// disabled
			brush = pal.brush(TQPalette::Disabled,
					  TQColorGroup::Base);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Disabled,
				     TQColorGroup::Base, brush);

			w->setPalette(pal);
		    }
		}
	    } else if (w->inherits(TQMENUBAR_OBJECT_NAME_STRING) ||
		       w->inherits(TQTOOLBAR_OBJECT_NAME_STRING)) {
		GtkObject *gobj = priv->gtkDict.tqfind(TQMenuBar::staticMetaObject());

		if (gobj) {
		    KLegacyImageDataKey key;
		    key.cachekey = 0;
		    key.data.function = KLegacy::Box;
		    key.data.state = KLegacy::Normal;
		    key.data.shadow = KLegacy::Out;

		    TQPixmap *pix = gobj->draw(key, w->width(), w->height(),
					      "menubar");
		    if (pix && (! pix->isNull())) {
			TQPalette pal = w->palette();

			// active
			TQBrush brush = pal.brush(TQPalette::Active,
						 TQColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Active,
				     TQColorGroup::Background, brush);

			brush = pal.brush(TQPalette::Active,
					  TQColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Active,
				     TQColorGroup::Button, brush);

			// inactive
			brush = pal.brush(TQPalette::Inactive,
					  TQColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Inactive,
				     TQColorGroup::Background, brush);

			brush = pal.brush(TQPalette::Inactive,
					  TQColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Inactive,
				     TQColorGroup::Button, brush);

			// disabled
			brush = pal.brush(TQPalette::Disabled,
					  TQColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Disabled,
				     TQColorGroup::Background, brush);

			brush = pal.brush(TQPalette::Disabled,
					  TQColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(TQPalette::Disabled,
				     TQColorGroup::Button, brush);

			w->setPalette(pal);
		    }
		}
	    }

	    break;
	}

    case TQEvent::Enter:
	{
	    if (obj->inherits(TQPUSHBUTTON_OBJECT_NAME_STRING) ||
		obj->inherits(TQCOMBOBOX_OBJECT_NAME_STRING) ||
		obj->inherits(TQSLIDER_OBJECT_NAME_STRING) ||
		obj->inherits(TQSCROLLBAR_OBJECT_NAME_STRING)) {
		priv->lastWidget = (TQWidget *) obj;
		priv->lastWidget->tqrepaint(false);
	    } else if (obj->inherits(TQRADIOBUTTON_OBJECT_NAME_STRING)) {
		TQWidget *w = (TQWidget *) obj;

		if (! w->isTopLevel() && w->isEnabled()) {
		    GtkObject *gobj = priv->gtkDict.tqfind(TQRadioButton::staticMetaObject());

		    if (gobj) {
			KLegacyImageDataKey key;
			key.cachekey = 0;
			key.data.function = KLegacy::FlatBox;

			TQPixmap *pix = gobj->draw(key, w->width(), w->height());

			if (pix && (! pix->isNull())) {
			    TQPalette pal = w->palette();
			    TQBrush brush = pal.brush(TQPalette::Normal,
						     TQColorGroup::Background);

			    brush.setPixmap(*pix);
			    pal.setBrush(TQPalette::Normal,
					 TQColorGroup::Background, brush);

			    w->setPalette(pal);
			    w->setBackgroundMode(TQWidget::PaletteBackground);
			    w->setBackgroundOrigin(TQWidget::WidgetOrigin);
			}
		    }
		}
	    } else if (obj->inherits(TQCHECKBOX_OBJECT_NAME_STRING)) {
		TQWidget *w = (TQWidget *) obj;

		if (! w->isTopLevel() && w->isEnabled()) {
		    GtkObject *gobj = priv->gtkDict.tqfind(TQCheckBox::staticMetaObject());

		    if (gobj) {
			KLegacyImageDataKey key;
			key.cachekey = 0;
			key.data.function = KLegacy::FlatBox;

			TQPixmap *pix = gobj->draw(key, w->width(), w->height());

			if (pix && (! pix->isNull())) {
			    TQPalette pal = w->palette();
			    TQBrush brush = pal.brush(TQPalette::Normal,
						     TQColorGroup::Background);

			    brush.setPixmap(*pix);
			    pal.setBrush(TQPalette::Normal,
					 TQColorGroup::Background, brush);

			    w->setPalette(pal);
			    w->setBackgroundMode(TQWidget::PaletteBackground);
			    w->setBackgroundOrigin(TQWidget::WidgetOrigin);
			}
		    }
		}
	    }

	    break;
	}

    case TQEvent::Leave:
	{
	    if (obj == priv->lastWidget) {
		priv->lastWidget = 0;
		((TQWidget *) obj)->tqrepaint(false);
	    } else if (obj->inherits(TQRADIOBUTTON_OBJECT_NAME_STRING) ||
		       obj->inherits(TQCHECKBOX_OBJECT_NAME_STRING)) {
		TQWidget *w = (TQWidget *) obj;

		if (! w->isTopLevel()) {
		    w->setBackgroundMode(TQWidget::X11ParentRelative);
		    w->setBackgroundOrigin(TQWidget::WidgetOrigin);
		    w->tqrepaint(true);
		}
	    }

	    break;
	}

    case TQEvent::MouseMove:
	{
	    TQMouseEvent *me = (TQMouseEvent *) e;
	    priv->mousePos = me->pos();
	    if (obj->inherits(TQSCROLLBAR_OBJECT_NAME_STRING) &&
		(! (me->state() & (LeftButton | MidButton | RightButton)))) {
		priv->hovering = true;
		((TQWidget *) obj)->tqrepaint(false);
		priv->hovering = false;
	    }

	    break;
	}

    default:
	{
	    break;
	}
    }

    return KStyle::eventFilter(obj, e);
}
