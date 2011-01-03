/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kfile.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *           (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include "kicondialog.h"

#include <config.h>

#include <assert.h>

#include <kiconviewsearchline.h>

#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kprogress.h>
#include <kiconview.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>

#include <tqlayout.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqsortedlist.h>
#include <tqimage.h>
#include <tqpixmap.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqtimer.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>
#include <tqfileinfo.h>
#include <tqtoolbutton.h>
#include <tqwhatsthis.h>

#ifdef HAVE_LIBART
#include <svgicons/ksvgiconengine.h>
#include <svgicons/ksvgiconpainter.h>
#endif

class KIconCanvas::KIconCanvasPrivate
{
  public:
    KIconCanvasPrivate() { m_bLoading = false; }
    ~KIconCanvasPrivate() {}
    bool m_bLoading;
};

/**
 * Helper class for sorting icon paths by icon name
 */
class IconPath : public QString
{
protected:
 TQString m_iconName;

public:
 IconPath(const TQString &ip) : TQString (ip)
 {
   int n = tqfindRev('/');
   m_iconName = (n==-1) ? static_cast<TQString>(*this) : mid(n+1);
 }


 IconPath() : TQString ()
 { }

 bool operator== (const IconPath &ip) const
 { return m_iconName == ip.m_iconName; }

 bool operator< (const IconPath &ip) const
 { return m_iconName < ip.m_iconName; }

};

/*
 * KIconCanvas: Iconview for the iconloader dialog.
 */

KIconCanvas::KIconCanvas(TQWidget *parent, const char *name)
    : KIconView(parent, name)
{
    d = new KIconCanvasPrivate;
    mpTimer = new TQTimer(this);
    connect(mpTimer, TQT_SIGNAL(timeout()), TQT_SLOT(slotLoadFiles()));
    connect(this, TQT_SIGNAL(currentChanged(TQIconViewItem *)),
	    TQT_SLOT(slotCurrentChanged(TQIconViewItem *)));
    setGridX(80);
    setWordWrapIconText(false);
    setShowToolTips(true);
}

KIconCanvas::~KIconCanvas()
{
    delete mpTimer;
    delete d;
}

void KIconCanvas::loadFiles(const TQStringList& files)
{
    clear();
    mFiles = files;
    emit startLoading(mFiles.count());
    mpTimer->start(10, true); // #86680
    d->m_bLoading = false;
}

void KIconCanvas::slotLoadFiles()
{
    setResizeMode(Fixed);
    TQApplication::setOverrideCursor(waitCursor);

    // disable updates to not trigger paint events when adding child items
    setUpdatesEnabled( false );

#ifdef HAVE_LIBART
    KSVGIconEngine *svgEngine = new KSVGIconEngine();
#endif

    d->m_bLoading = true;
    int i;
    TQStringList::ConstIterator it;
    uint emitProgress = 10; // so we will emit it once in the beginning
    TQStringList::ConstIterator end(mFiles.end());
    for (it=mFiles.begin(), i=0; it!=end; ++it, i++)
    {
	// Calling kapp->processEvents() makes the iconview flicker like hell
	// (it's being tqrepainted once for every new item), so we don't do this.
	// Instead, we directly tqrepaint the progress bar without going through
	// the event-loop. We do that just once for every 10th item so that
	// the progress bar doesn't flicker in turn. (pfeiffer)
	if ( emitProgress >= 10 ) {
	    emit progress(i);
            emitProgress = 0;
        }

        emitProgress++;
//	kapp->processEvents();
        if ( !d->m_bLoading ) // user clicked on a button that will load another set of icons
            break;
	TQImage img;

	// Use the extension as the format. Works for XPM and PNG, but not for SVG
	TQString path= *it;
	TQString ext = path.right(3).upper();

	if (ext != "SVG" && ext != "VGZ")
	    img.load(*it);
#ifdef HAVE_LIBART
	else
	    if (svgEngine->load(60, 60, *it))
		img = *svgEngine->painter()->image();
#endif

	if (img.isNull())
	    continue;
	if (img.width() > 60 || img.height() > 60)
	{
	    if (img.width() > img.height())
	    {
		int height = (int) ((60.0 / img.width()) * img.height());
		img = img.smoothScale(60, height);
	    } else
	    {
		int width = (int) ((60.0 / img.height()) * img.width());
		img = img.smoothScale(width, 60);
	    }
	}
	TQPixmap pm;
	pm.convertFromImage(img);
	TQFileInfo fi(*it);
	TQIconViewItem *item = new TQIconViewItem(this, fi.baseName(), pm);
	item->setKey(*it);
	item->setDragEnabled(false);
	item->setDropEnabled(false);
    }

#ifdef HAVE_LIBART
    delete svgEngine;
#endif

    // enable updates since we have to draw the whole view now
    setUpdatesEnabled( true );

    TQApplication::restoreOverrideCursor();
    d->m_bLoading = false;
    emit finished();
    setResizeMode(Adjust);
}

TQString KIconCanvas::getCurrent() const
{
    if (!currentItem())
	return TQString::null;
    return currentItem()->key();
}

void KIconCanvas::stopLoading()
{
    d->m_bLoading = false;
}

void KIconCanvas::slotCurrentChanged(TQIconViewItem *item)
{
    emit nameChanged((item != 0L) ? item->text() : TQString::null);
}

class KIconDialog::KIconDialogPrivate
{
  public:
    KIconDialogPrivate() {
        m_bStrictIconSize = true;
	m_bLockUser = false;
	m_bLockCustomDir = false;
	searchLine = 0;
    }
    ~KIconDialogPrivate() {}
    bool m_bStrictIconSize, m_bLockUser, m_bLockCustomDir;
    TQString custom;
    TQString customLocation;
    KIconViewSearchLine *searchLine;
};

/*
 * KIconDialog: Dialog for selecting icons. Both system and user
 * specified icons can be chosen.
 */

KIconDialog::KIconDialog(TQWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Ok|Cancel, Ok)
{
    d = new KIconDialogPrivate;
    mpLoader = KGlobal::iconLoader();
    init();
}

KIconDialog::KIconDialog(KIconLoader *loader, TQWidget *parent,
	const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Ok|Cancel, Ok)
{
    d = new KIconDialogPrivate;
    mpLoader = loader;
    init();
}

void KIconDialog::init()
{
    mGroupOrSize = KIcon::Desktop;
    mContext = KIcon::Any;
    mType = 0;
    mFileList = KGlobal::dirs()->findAllResources("appicon", TQString::tqfromLatin1("*.png"));

    TQWidget *main = new TQWidget( this );
    setMainWidget(main);

    TQVBoxLayout *top = new TQVBoxLayout(main);
    top->setSpacing( spacingHint() );

    TQButtonGroup *bgroup = new TQButtonGroup(0, Qt::Vertical, i18n("Icon Source"), main);
    bgroup->tqlayout()->setSpacing(KDialog::spacingHint());
    bgroup->tqlayout()->setMargin(KDialog::marginHint());
    top->addWidget(bgroup);
    connect(bgroup, TQT_SIGNAL(clicked(int)), TQT_SLOT(slotButtonClicked(int)));
    TQGridLayout *grid = new TQGridLayout(bgroup->tqlayout(), 3, 2);
    mpRb1 = new TQRadioButton(i18n("S&ystem icons:"), bgroup);
    grid->addWidget(mpRb1, 1, 0);
    mpCombo = new TQComboBox(bgroup);
    connect(mpCombo, TQT_SIGNAL(activated(int)), TQT_SLOT(slotContext(int)));
    grid->addWidget(mpCombo, 1, 1);
    mpRb2 = new TQRadioButton(i18n("O&ther icons:"), bgroup);
    grid->addWidget(mpRb2, 2, 0);
    mpBrowseBut = new TQPushButton(i18n("&Browse..."), bgroup);
    grid->addWidget(mpBrowseBut, 2, 1);

    //
    // ADD SEARCHLINE
    //
    TQHBoxLayout *searchLayout = new TQHBoxLayout(0, 0, KDialog::spacingHint());
    top->addLayout(searchLayout);

    TQToolButton *clearSearch = new TQToolButton(main);
    clearSearch->setTextLabel(i18n("Clear Search"), true);
    clearSearch->setIconSet(SmallIconSet(TQApplication::reverseLayout() ? "clear_left" :"locationbar_erase"));
    searchLayout->addWidget(clearSearch);

    TQLabel *searchLabel = new TQLabel(i18n("&Search:"), main);
    searchLayout->addWidget(searchLabel);

    d->searchLine = new KIconViewSearchLine(main, "searchLine");
    searchLayout->addWidget(d->searchLine);
    searchLabel->setBuddy(d->searchLine);


    // signals and slots connections
    connect(clearSearch, TQT_SIGNAL(clicked()), d->searchLine, TQT_SLOT(clear()));

    TQString wtstr = i18n("Search interactively for icon names (e.g. folder).");
    TQWhatsThis::add(searchLabel, wtstr);
    TQWhatsThis::add(d->searchLine, wtstr);


    mpCanvas = new KIconCanvas(main);
    connect(mpCanvas, TQT_SIGNAL(executed(TQIconViewItem *)), TQT_SLOT(slotAcceptIcons()));
    connect(mpCanvas, TQT_SIGNAL(returnPressed(TQIconViewItem *)), TQT_SLOT(slotAcceptIcons()));
    mpCanvas->setMinimumSize(400, 125);
    top->addWidget(mpCanvas);
    d->searchLine->setIconView(mpCanvas);

    mpProgress = new KProgress(main);
    top->addWidget(mpProgress);
    connect(mpCanvas, TQT_SIGNAL(startLoading(int)), TQT_SLOT(slotStartLoading(int)));
    connect(mpCanvas, TQT_SIGNAL(progress(int)), TQT_SLOT(slotProgress(int)));
    connect(mpCanvas, TQT_SIGNAL(finished()), TQT_SLOT(slotFinished()));

    // When pressing Ok or Cancel, stop loading icons
    connect(this, TQT_SIGNAL(hidden()), mpCanvas, TQT_SLOT(stopLoading()));

    static const char* const context_text[] = {
        I18N_NOOP( "Actions" ),
        I18N_NOOP( "Animations" ),
        I18N_NOOP( "Applications" ),
        I18N_NOOP( "Categories" ),
        I18N_NOOP( "Devices" ),
        I18N_NOOP( "Emblems" ),
        I18N_NOOP( "Emotes" ),
        I18N_NOOP( "Filesystems" ),
        I18N_NOOP( "International" ),
        I18N_NOOP( "Mimetypes" ),
        I18N_NOOP( "Places" ),
        I18N_NOOP( "tqStatus" ) };
    static const KIcon::Context context_id[] = {
        KIcon::Action,
        KIcon::Animation,
        KIcon::Application,
        KIcon::Category,
        KIcon::Device,
        KIcon::Emblem,
        KIcon::Emote,
        KIcon::FileSystem,
        KIcon::International,
        KIcon::MimeType,
        KIcon::Place,
        KIcon::tqStatusIcon };
    mNumContext = 0;
    int cnt = sizeof( context_text ) / sizeof( context_text[ 0 ] );
    // check all 3 arrays have same sizes
    assert( cnt == sizeof( context_id ) / sizeof( context_id[ 0 ] )
            && cnt == sizeof( mContextMap ) / sizeof( mContextMap[ 0 ] ));
    for( int i = 0;
         i < cnt;
         ++i )
    {
        if( mpLoader->hasContext( context_id[ i ] ))
        {
            mpCombo->insertItem(i18n( context_text[ i ] ));
            mContextMap[ mNumContext++ ] = context_id[ i ];
        }
    }
    mpCombo->setFixedSize(mpCombo->tqsizeHint());

    mpBrowseBut->setFixedWidth(mpCombo->width());

    // Make the dialog a little taller
    incInitialSize(TQSize(0,100));
}


KIconDialog::~KIconDialog()
{
    delete d;
}

void KIconDialog::slotAcceptIcons()
{
  d->custom=TQString::null;
  slotOk();
}

void KIconDialog::showIcons()
{
    mpCanvas->clear();
    TQStringList filelist;
    if (mType == 0)
	if (d->m_bStrictIconSize)
            filelist=mpLoader->queryIcons(mGroupOrSize, mContext);
        else
            filelist=mpLoader->queryIconsByContext(mGroupOrSize, mContext);
    else if ( !d->customLocation.isNull() )
	filelist=mpLoader->queryIconsByDir( d->customLocation );
    else
	filelist=mFileList;

    TQSortedList <IconPath>iconlist;
    iconlist.setAutoDelete(true);
    TQStringList::Iterator it;
    for( it = filelist.begin(); it != filelist.end(); ++it )
       iconlist.append(new IconPath(*it));

    iconlist.sort();
    filelist.clear();

    for ( IconPath *ip=iconlist.first(); ip != 0; ip=iconlist.next() )
       filelist.append(*ip);

    d->searchLine->clear();
    mpCanvas->loadFiles(filelist);
}

void KIconDialog::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

bool KIconDialog::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void KIconDialog::setIconSize( int size )
{
    // see KIconLoader, if you think this is weird
    if ( size == 0 )
        mGroupOrSize = KIcon::Desktop; // default Group
    else
        mGroupOrSize = -size; // yes, KIconLoader::queryIconsByContext is weird
}

int KIconDialog::iconSize() const
{
    // 0 or any other value ==> mGroupOrSize is a group, so we return 0
    return (mGroupOrSize < 0) ? -mGroupOrSize : 0;
}

#ifndef KDE_NO_COMPAT
TQString KIconDialog::selectIcon(KIcon::Group group, KIcon::Context context, bool user)
{
    setup( group, context, false, 0, user );
    return openDialog();
}
#endif

void KIconDialog::setup(KIcon::Group group, KIcon::Context context,
                        bool strictIconSize, int iconSize, bool user )
{
    d->m_bStrictIconSize = strictIconSize;
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mType = user ? 1 : 0;
    mpRb1->setChecked(!user);
    mpRb2->setChecked(user);
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled(user);
    setContext( context );
}

void KIconDialog::setup(KIcon::Group group, KIcon::Context context,
                        bool strictIconSize, int iconSize, bool user,
                        bool lockUser, bool lockCustomDir )
{
    d->m_bStrictIconSize = strictIconSize;
    d->m_bLockUser = lockUser;
    d->m_bLockCustomDir = lockCustomDir;
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mType = user ? 1 : 0;
    mpRb1->setChecked(!user);
    mpRb1->setEnabled( !lockUser || !user );
    mpRb2->setChecked(user);
    mpRb2->setEnabled( !lockUser || user );
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled( user && !lockCustomDir );
    setContext( context );
}

void KIconDialog::setContext( KIcon::Context context )
{
    mContext = context;
    for( int i = 0;
         i < mNumContext;
         ++i )
        if( mContextMap[ i ] == context )
        {
            mpCombo->setCurrentItem( i );
            return;
        }
}

void KIconDialog::setCustomLocation( const TQString& location )
{
    d->customLocation = location;
}

TQString KIconDialog::openDialog()
{
    showIcons();

    if ( exec() == Accepted )
    {
        if (!d->custom.isNull())
            return d->custom;
	TQString name = mpCanvas->getCurrent();
	if (name.isEmpty() || (mType == 1))
	    return name;
	TQFileInfo fi(name);
	return fi.baseName();
    }
    return TQString::null;
}

void KIconDialog::showDialog()
{
    setModal(false);
    showIcons();
    show();
}

void KIconDialog::slotOk()
{
    TQString name;
    if (!d->custom.isNull())
    {
        name = d->custom;
    }
    else
    {
        name = mpCanvas->getCurrent();
        if (!name.isEmpty() && (mType != 1))
        {
            TQFileInfo fi(name);
            name = fi.baseName();
        }
    }

    emit newIconName(name);
    KDialogBase::slotOk();
}

TQString KIconDialog::getIcon(KIcon::Group group, KIcon::Context context,
                             bool strictIconSize, int iconSize, bool user,
                             TQWidget *parent, const TQString &caption)
{
    KIconDialog dlg(parent, "icon dialog");
    dlg.setup( group, context, strictIconSize, iconSize, user );
    if (!caption.isNull())
        dlg.setCaption(caption);

    return dlg.openDialog();
}

void KIconDialog::slotButtonClicked(int id)
{
    TQString file;

    switch (id)
    {
    case 0:
        if(mType!=0)
        {
            mType = 0;
            mpBrowseBut->setEnabled(false);
            mpCombo->setEnabled(true);
            showIcons();
        }
	break;

    case 1:
        if(mType!=1)
        {
            mType = 1;
            mpBrowseBut->setEnabled( !d->m_bLockCustomDir );
            mpCombo->setEnabled(false);
            showIcons();
        }
        break;
    case 2:
        {
            // Create a file dialog to select a PNG, XPM or SVG file,
            // with the image previewer shown.
            // KFileDialog::getImageOpenURL doesn't allow svg.
            KFileDialog dlg(TQString::null, i18n("*.png *.xpm *.svg *.svgz|Icon Files (*.png *.xpm *.svg *.svgz)"),
                            this, "filedialog", true);
            dlg.setOperationMode( KFileDialog::Opening );
            dlg.setCaption( i18n("Open") );
            dlg.setMode( KFile::File );

            KImageFilePreview *ip = new KImageFilePreview( &dlg );
            dlg.setPreviewWidget( ip );
            dlg.exec();

            file = dlg.selectedFile();
            if (!file.isEmpty())
            {
                d->custom = file;
                if ( mType == 1 )
                  d->customLocation = TQFileInfo( file ).dirPath( true );
                slotOk();
            }
        }
        break;
    }
}

void KIconDialog::slotContext(int id)
{
    mContext = static_cast<KIcon::Context>( mContextMap[ id ] );
    showIcons();
}

void KIconDialog::slotStartLoading(int steps)
{
    if (steps < 10)
	mpProgress->hide();
    else
    {
        mpProgress->setTotalSteps(steps);
        mpProgress->setProgress(0);
        mpProgress->show();
    }
}

void KIconDialog::slotProgress(int p)
{
    mpProgress->setProgress(p);
    // commented out the following since setProgress already paints ther
    // progress bar. ->tqrepaint() only makes it flicker
    //mpProgress->tqrepaint();
}

void KIconDialog::slotFinished()
{
    mpProgress->hide();
}

class KIconButton::KIconButtonPrivate
{
  public:
    KIconButtonPrivate() {
        m_bStrictIconSize = false;
        iconSize = 0; // let KIconLoader choose the default
    }
    ~KIconButtonPrivate() {}
    bool m_bStrictIconSize;
    int iconSize;
};


/*
 * KIconButton: A "choose icon" pushbutton.
 */

KIconButton::KIconButton(TQWidget *parent, const char *name)
    : TQPushButton(parent, name)
{
    init( KGlobal::iconLoader() );
}

KIconButton::KIconButton(KIconLoader *loader,
	TQWidget *parent, const char *name)
    : TQPushButton(parent, name)
{
    init( loader );
}

void KIconButton::init( KIconLoader *loader )
{
    d = new KIconButtonPrivate;
    mGroup = KIcon::Desktop;
    mContext = KIcon::Application;
    mbUser = false;

    mpLoader = loader;
    mpDialog = 0L;
    connect(this, TQT_SIGNAL(clicked()), TQT_SLOT(slotChangeIcon()));
}

KIconButton::~KIconButton()
{
    delete mpDialog;
    delete d;
}

void KIconButton::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

bool KIconButton::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void KIconButton::setIconSize( int size )
{
    d->iconSize = size;
}

int KIconButton::iconSize() const
{
    return d->iconSize;
}

void KIconButton::setIconType(KIcon::Group group, KIcon::Context context, bool user)
{
    mGroup = group;
    mContext = context;
    mbUser = user;
}

void KIconButton::setIcon(const TQString& icon)
{
    mIcon = icon;
    setIconSet(mpLoader->loadIconSet(mIcon, mGroup, d->iconSize));

    if (!mpDialog)
    {
        mpDialog = new KIconDialog(mpLoader, this);
        connect(mpDialog, TQT_SIGNAL(newIconName(const TQString&)), TQT_SLOT(newIconName(const TQString&)));
    }

    if ( mbUser )
      mpDialog->setCustomLocation( TQFileInfo( mpLoader->iconPath(mIcon, mGroup, true) ).dirPath( true ) );
}

void KIconButton::resetIcon()
{
    mIcon = TQString::null;
    setIconSet(TQIconSet());
}

void KIconButton::slotChangeIcon()
{
    if (!mpDialog)
    {
        mpDialog = new KIconDialog(mpLoader, this);
        connect(mpDialog, TQT_SIGNAL(newIconName(const TQString&)), TQT_SLOT(newIconName(const TQString&)));
    }

    mpDialog->setup( mGroup, mContext, d->m_bStrictIconSize, d->iconSize, mbUser );
    mpDialog->showDialog();
}

void KIconButton::newIconName(const TQString& name)
{
    if (name.isEmpty())
        return;

    TQIconSet iconset = mpLoader->loadIconSet(name, mGroup, d->iconSize);
    setIconSet(iconset);
    mIcon = name;

    if ( mbUser )
      mpDialog->setCustomLocation( TQFileInfo( mpLoader->iconPath(mIcon, mGroup, true) ).dirPath( true ) );

    emit iconChanged(name);
}

void KIconCanvas::virtual_hook( int id, void* data )
{ KIconView::virtual_hook( id, data ); }

void KIconDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kicondialog.moc"
