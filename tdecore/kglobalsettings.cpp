/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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
#include "config.h"
#include "kglobalsettings.h"

#include <tqdir.h>
#include <tqpixmap.h>
#include <tqfontdatabase.h>
#include <tqcursor.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kapplication.h>

#include <kipc.h>

#ifdef Q_WS_WIN
#include <windows.h>
#include "qt_windows.h"
#include <win32_utils.h>
static QRgb qt_colorref2qrgb(COLORREF col)
{
    return tqRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}
#endif

#include <kdebug.h>
#include <kglobal.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <kcharsets.h>
#include <kaccel.h>
#include <klocale.h>
#include <tqfontinfo.h>
#include <stdlib.h>
#include <kprotocolinfo.h>

#include <tqtextcodec.h>
#include <tqtextstream.h>
#include <tqfile.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

TQString* KGlobalSettings::s_desktopPath = 0;
TQString* KGlobalSettings::s_autostartPath = 0;
TQString* KGlobalSettings::s_trashPath = 0;
TQString* KGlobalSettings::s_documentPath = 0;
TQFont *KGlobalSettings::_generalFont = 0;
TQFont *KGlobalSettings::_fixedFont = 0;
TQFont *KGlobalSettings::_toolBarFont = 0;
TQFont *KGlobalSettings::_menuFont = 0;
TQFont *KGlobalSettings::_windowTitleFont = 0;
TQFont *KGlobalSettings::_taskbarFont = 0;
TQFont *KGlobalSettings::_largeFont = 0;
TQColor *KGlobalSettings::_trinity4Blue = 0;
TQColor *KGlobalSettings::_inactiveBackground = 0;
TQColor *KGlobalSettings::_inactiveForeground = 0;
TQColor *KGlobalSettings::_activeBackground = 0;
TQColor *KGlobalSettings::_buttonBackground = 0;
TQColor *KGlobalSettings::_selectBackground = 0;
TQColor *KGlobalSettings::_linkColor = 0;
TQColor *KGlobalSettings::_visitedLinkColor = 0;
TQColor *KGlobalSettings::alternateColor = 0;

KGlobalSettings::KMouseSettings *KGlobalSettings::s_mouseSettings = 0;

// helper function for reading xdg user dirs: it is required in order to take 
// care of locale stuff
void readXdgUserDirs(TQString *desktop, TQString *documents)
{
	TQFile f( TQDir::homeDirPath() + "/.config/user-dirs.dirs" );

	if (!f.open(IO_ReadOnly))
		return;

	// set the codec for the current locale
	TQTextStream s(&f);
	s.setCodec( TQTextCodec::codecForLocale() );

	TQString line = s.readLine();
	while (!line.isNull())
	{
		if (line.startsWith("XDG_DESKTOP_DIR="))
			*desktop = line.remove("XDG_DESKTOP_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
		else if (line.startsWith("XDG_DOCUMENTS_DIR="))
			*documents = line.remove("XDG_DOCUMENTS_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());

		line = s.readLine();
	}
}

int KGlobalSettings::dndEventDelay()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readNumEntry("StartDragDist", TQApplication::startDragDistance());
}

bool KGlobalSettings::singleClick()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
}

bool KGlobalSettings::iconUseRoundedRect()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("IconUseRoundedRect", KDE_DEFAULT_ICONTEXTROUNDED);
}

KGlobalSettings::TearOffHandle KGlobalSettings::insertTearOffHandle()
{
    int tearoff;
    bool effectsenabled;
    KConfigGroup g( KGlobal::config(), "KDE" );
    effectsenabled = g.readBoolEntry( "EffectsEnabled", false);
    tearoff = g.readNumEntry("InsertTearOffHandle", KDE_DEFAULT_INSERTTEAROFFHANDLES);
    return effectsenabled ? (TearOffHandle) tearoff : Disable;
}

bool KGlobalSettings::changeCursorOverIcon()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

bool KGlobalSettings::visualActivate()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("VisualActivate", KDE_DEFAULT_VISUAL_ACTIVATE);
}

unsigned int KGlobalSettings::visualActivateSpeed()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return
        g.readNumEntry(
            "VisualActivateSpeed",
            KDE_DEFAULT_VISUAL_ACTIVATE_SPEED
        );
}



int KGlobalSettings::autoSelectDelay()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readNumEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
}

KGlobalSettings::Completion KGlobalSettings::completionMode()
{
    int completion;
    KConfigGroup g( KGlobal::config(), "General" );
    completion = g.readNumEntry("completionMode", -1);
    if ((completion < (int) CompletionNone) ||
        (completion > (int) CompletionPopupAuto))
      {
        completion = (int) CompletionPopup; // Default
      }
  return (Completion) completion;
}

bool KGlobalSettings::showContextMenusOnPress ()
{
    KConfigGroup g(KGlobal::config(), "ContextMenus");
    return g.readBoolEntry("ShowOnPress", true);
}

int KGlobalSettings::contextMenuKey ()
{
    KConfigGroup g(KGlobal::config(), "Shortcuts");
    KShortcut cut (g.readEntry ("PopupMenuContext", "Menu"));
    return cut.keyCodeQt();
}

TQColor KGlobalSettings::toolBarHighlightColor()
{
    initColors();
    KConfigGroup g( KGlobal::config(), "Toolbar style" );
    return g.readColorEntry("HighlightColor", _trinity4Blue);
}

TQColor KGlobalSettings::inactiveTitleColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION));
#else
    if (!_inactiveBackground)
        _inactiveBackground = new TQColor(157, 170, 186);
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readColorEntry( "inactiveBackground", _inactiveBackground );
#endif
}

TQColor KGlobalSettings::inactiveTextColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
#else
    if (!_inactiveForeground)
       _inactiveForeground = new TQColor(221,221,221);
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readColorEntry( "inactiveForeground", _inactiveForeground );
#endif
}

TQColor KGlobalSettings::activeTitleColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION));
#else
    initColors();
    if (!_activeBackground)
      _activeBackground = new TQColor(65,142,220);
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readColorEntry( "activeBackground", _activeBackground);
#endif
}

TQColor KGlobalSettings::activeTextColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readColorEntry( "activeForeground", tqwhiteptr );
#endif
}

int KGlobalSettings::contrast()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readNumEntry( "contrast", 7 );
}

TQColor KGlobalSettings::buttonBackground()
{
    if (!_buttonBackground)
      _buttonBackground = new TQColor(221,223,228);
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "buttonBackground", _buttonBackground );
}

TQColor KGlobalSettings::buttonTextColor()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "buttonForeground", tqblackptr );
}

// IMPORTANT:
//  This function should be kept in sync with
//   KApplication::kdisplaySetPalette()
TQColor KGlobalSettings::baseColor()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "windowBackground", tqwhiteptr );
}

// IMPORTANT:
//  This function should be kept in sync with
//   KApplication::kdisplaySetPalette()
TQColor KGlobalSettings::textColor()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "windowForeground", tqblackptr );
}

// IMPORTANT:
//  This function should be kept in sync with
//   KApplication::kdisplaySetPalette()
TQColor KGlobalSettings::highlightedTextColor()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "selectForeground", tqwhiteptr );
}

// IMPORTANT:
//  This function should be kept in sync with
//   KApplication::kdisplaySetPalette()
TQColor KGlobalSettings::highlightColor()
{
    initColors();
    if (!_selectBackground)
        _selectBackground = new TQColor(103,141,178);
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "selectBackground", _selectBackground );
}

TQColor KGlobalSettings::alternateBackgroundColor()
{
    initColors();
    KConfigGroup g( KGlobal::config(), "General" );
    *alternateColor = calculateAlternateBackgroundColor( baseColor() );
    return g.readColorEntry( "alternateBackground", alternateColor );
}

TQColor KGlobalSettings::calculateAlternateBackgroundColor(const TQColor& base)
{
    if (base == Qt::white)
        return TQColor(238,246,255);
    else
    {
        int h, s, v;
        base.hsv( &h, &s, &v );
        if (v > 128)
            return base.dark(106);
        else if (base != Qt::black)
            return base.light(110);

        return TQColor(32,32,32);
    }
}

bool KGlobalSettings::shadeSortColumn()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readBoolEntry( "shadeSortColumn", KDE_DEFAULT_SHADE_SORT_COLUMN );
}

TQColor KGlobalSettings::linkColor()
{
    initColors();
    if (!_linkColor)
        _linkColor = new TQColor(0,0,238);
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "linkColor", _linkColor );
}

TQColor KGlobalSettings::visitedLinkColor()
{
    if (!_visitedLinkColor)
        _visitedLinkColor = new TQColor(82,24,139);
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "visitedLinkColor", _visitedLinkColor );
}

TQFont KGlobalSettings::generalFont()
{
    if (_generalFont)
        return *_generalFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _generalFont = new TQFont("Sans Serif", 10);
    _generalFont->setPointSize(10);
    _generalFont->setStyleHint(TQFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_generalFont = g.readFontEntry("font", _generalFont);

    return *_generalFont;
}

TQFont KGlobalSettings::fixedFont()
{
    if (_fixedFont)
        return *_fixedFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _fixedFont = new TQFont("Monospace", 10);
    _fixedFont->setPointSize(10);
    _fixedFont->setStyleHint(TQFont::TypeWriter);

    KConfigGroup g( KGlobal::config(), "General" );
    *_fixedFont = g.readFontEntry("fixed", _fixedFont);

    return *_fixedFont;
}

TQFont KGlobalSettings::toolBarFont()
{
    if(_toolBarFont)
        return *_toolBarFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _toolBarFont = new TQFont("Sans Serif", 10);
    _toolBarFont->setPointSize(10);
    _toolBarFont->setStyleHint(TQFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_toolBarFont = g.readFontEntry("toolBarFont", _toolBarFont);

    return *_toolBarFont;
}

TQFont KGlobalSettings::menuFont()
{
    if(_menuFont)
        return *_menuFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _menuFont = new TQFont("Sans Serif", 10);
    _menuFont->setPointSize(10);
    _menuFont->setStyleHint(TQFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_menuFont = g.readFontEntry("menuFont", _menuFont);

    return *_menuFont;
}

TQFont KGlobalSettings::windowTitleFont()
{
    if(_windowTitleFont)
        return *_windowTitleFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _windowTitleFont = new TQFont("Sans Serif", 9, TQFont::Bold);
    _windowTitleFont->setPointSize(10);
    _windowTitleFont->setStyleHint(TQFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "WM" );
    *_windowTitleFont = g.readFontEntry("activeFont", _windowTitleFont); // inconsistency

    return *_windowTitleFont;
}

TQFont KGlobalSettings::taskbarFont()
{
    if(_taskbarFont)
        return *_taskbarFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _taskbarFont = new TQFont("Sans Serif", 10);
    _taskbarFont->setPointSize(10);
    _taskbarFont->setStyleHint(TQFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_taskbarFont = g.readFontEntry("taskbarFont", _taskbarFont);

    return *_taskbarFont;
}


TQFont KGlobalSettings::largeFont(const TQString &text)
{
    TQFontDatabase db;
    TQStringList fam = db.tqfamilies();

    // Move a bunch of preferred fonts to the front.
    if (fam.remove("Arial"))
       fam.prepend("Arial");
    if (fam.remove("Verdana"))
       fam.prepend("Verdana");
    if (fam.remove("Tahoma"))
       fam.prepend("Tahoma");
    if (fam.remove("Lucida Sans"))
       fam.prepend("Lucida Sans");
    if (fam.remove("Lucidux Sans"))
       fam.prepend("Lucidux Sans");
    if (fam.remove("Nimbus Sans"))
       fam.prepend("Nimbus Sans");
    if (fam.remove("Gothic I"))
       fam.prepend("Gothic I");

    if (_largeFont)
        fam.prepend(_largeFont->family());

    for(TQStringList::ConstIterator it = fam.begin();
        it != fam.end(); ++it)
    {
        if (db.isSmoothlyScalable(*it) && !db.isFixedPitch(*it))
        {
            TQFont font(*it);
            font.setPixelSize(75);
            TQFontMetrics metrics(font);
            int h = metrics.height();
            if ((h < 60) || ( h > 90))
                continue;

            bool ok = true;
            for(unsigned int i = 0; i < text.length(); i++)
            {
                if (!metrics.inFont(text[i]))
                {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;

            font.setPointSize(48);
            _largeFont = new TQFont(font);
            return *_largeFont;
        }
    }
    _largeFont = new TQFont(KGlobalSettings::generalFont());
    _largeFont->setPointSize(48);
    return *_largeFont;
}

void KGlobalSettings::initStatic() // should be called initPaths(). Don't put anything else here.
{
    if ( s_desktopPath != 0 )
        return;

    s_desktopPath = new TQString();
    s_autostartPath = new TQString();
    s_trashPath = new TQString();
    s_documentPath = new TQString();

    KConfigGroup g( KGlobal::config(), "Paths" );

	// Read desktop and documents path using XDG_USER_DIRS
	readXdgUserDirs(s_desktopPath, s_documentPath);
	
    if (s_desktopPath->isEmpty() == true) {
      *s_desktopPath = TQDir::homeDirPath() + "/Desktop/";
    }
    *s_desktopPath = TQDir::cleanDirPath( *s_desktopPath );
    if ( !s_desktopPath->endsWith("/") )
      s_desktopPath->append('/');

    if (s_documentPath->isEmpty() == true) {
#ifdef Q_WS_WIN
      *s_documentPath = getWin32ShellFoldersPath("Personal");
#else
      *s_documentPath = TQDir::homeDirPath() + "/Documents/";
#endif
    }
    *s_documentPath = TQDir::cleanDirPath( *s_documentPath );
    if ( !s_documentPath->endsWith("/"))
      s_documentPath->append('/');

    // Trash Path - TODO remove in KDE4 (kio_trash can't use it for interoperability reasons)
    *s_trashPath = *s_desktopPath + i18n("Trash") + "/";
    *s_trashPath = g.readPathEntry( "Trash" , *s_trashPath);
    *s_trashPath = TQDir::cleanDirPath( *s_trashPath );
    if ( !s_trashPath->endsWith("/") )
      s_trashPath->append('/');
    // We need to save it in any case, in case the language changes later on,
    if ( !g.hasKey( "Trash" ) )
    {
      g.writePathEntry( "Trash", *s_trashPath, true, true );
      g.sync();
    }

    // Autostart Path
    *s_autostartPath = KGlobal::dirs()->localtdedir() + "Autostart/";
    *s_autostartPath = g.readPathEntry( "Autostart" , *s_autostartPath);
    *s_autostartPath = TQDir::cleanDirPath( *s_autostartPath );
    if ( !s_autostartPath->endsWith("/") )
      s_autostartPath->append('/');

    // Make sure this app gets the notifications about those paths
    if (kapp)
        kapp->addKipcEventMask(KIPC::SettingsChanged);
}

void KGlobalSettings::initColors()
{
    if (!_trinity4Blue) {
      if (TQPixmap::defaultDepth() > 8)
        _trinity4Blue = new TQColor(103,141,178);
      else
        _trinity4Blue = new TQColor(0, 0, 192);
    }
    if (!alternateColor)
      alternateColor = new TQColor(237, 244, 249);
}

void KGlobalSettings::rereadFontSettings()
{
    delete _generalFont;
    _generalFont = 0L;
    delete _fixedFont;
    _fixedFont = 0L;
    delete _menuFont;
    _menuFont = 0L;
    delete _toolBarFont;
    _toolBarFont = 0L;
    delete _windowTitleFont;
    _windowTitleFont = 0L;
    delete _taskbarFont;
    _taskbarFont = 0L;
}

void KGlobalSettings::rereadPathSettings()
{
    kdDebug() << "KGlobalSettings::rereadPathSettings" << endl;
    delete s_autostartPath;
    s_autostartPath = 0L;
    delete s_trashPath;
    s_trashPath = 0L;
    delete s_desktopPath;
    s_desktopPath = 0L;
    delete s_documentPath;
    s_documentPath = 0L;
}

KGlobalSettings::KMouseSettings & KGlobalSettings::mouseSettings()
{
    if ( ! s_mouseSettings )
    {
        s_mouseSettings = new KMouseSettings;
        KMouseSettings & s = *s_mouseSettings; // for convenience

#ifndef Q_WS_WIN
        KConfigGroup g( KGlobal::config(), "Mouse" );
        TQString setting = g.readEntry("MouseButtonMapping");
        if (setting == "RightHanded")
            s.handed = KMouseSettings::RightHanded;
        else if (setting == "LeftHanded")
            s.handed = KMouseSettings::LeftHanded;
        else
        {
#ifdef Q_WS_X11
            // get settings from X server
            // This is a simplified version of the code in input/mouse.cpp
            // Keep in sync !
            s.handed = KMouseSettings::RightHanded;
            unsigned char map[20];
            int num_buttons = XGetPointerMapping(kapp->getDisplay(), map, 20);
            if( num_buttons == 2 )
            {
                if ( (int)map[0] == 1 && (int)map[1] == 2 )
                    s.handed = KMouseSettings::RightHanded;
                else if ( (int)map[0] == 2 && (int)map[1] == 1 )
                    s.handed = KMouseSettings::LeftHanded;
            }
            else if( num_buttons >= 3 )
            {
                if ( (int)map[0] == 1 && (int)map[2] == 3 )
                    s.handed = KMouseSettings::RightHanded;
                else if ( (int)map[0] == 3 && (int)map[2] == 1 )
                    s.handed = KMouseSettings::LeftHanded;
            }
#else
        // FIXME(E): Implement in Qt Embedded
#endif
        }
#endif //Q_WS_WIN
    }
#ifdef Q_WS_WIN
    //not cached
    s_mouseSettings->handed = (GetSystemMetrics(SM_SWAPBUTTON) ? KMouseSettings::LeftHanded : KMouseSettings::RightHanded);
#endif
    return *s_mouseSettings;
}

void KGlobalSettings::rereadMouseSettings()
{
#ifndef Q_WS_WIN
    delete s_mouseSettings;
    s_mouseSettings = 0L;
#endif
}

bool KGlobalSettings::isMultiHead()
{
#ifdef Q_WS_WIN
    return GetSystemMetrics(SM_CMONITORS) > 1;
#else
    TQCString multiHead = getenv("TDE_MULTIHEAD");
    if (!multiHead.isEmpty()) {
        return (multiHead.lower() == "true");
    }
    return false;
#endif
}

bool KGlobalSettings::wheelMouseZooms()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry( "WheelMouseZooms", KDE_DEFAULT_WHEEL_ZOOM );
}

TQRect KGlobalSettings::splashScreenDesktopGeometry()
{
    TQDesktopWidget *dw = TQApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        int scr = group.readNumEntry("Unmanaged", -3);
        if (group.readBoolEntry("XineramaEnabled", true) && scr != -2) {
            if (scr == -3)
                scr = dw->screenNumber(TQCursor::pos());
            return dw->screenGeometry(scr);
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

TQRect KGlobalSettings::desktopGeometry(const TQPoint& point)
{
    TQDesktopWidget *dw = TQApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        if (group.readBoolEntry("XineramaEnabled", true) &&
            group.readBoolEntry("XineramaPlacementEnabled", true)) {
            return dw->screenGeometry(dw->screenNumber(point));
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

TQRect KGlobalSettings::desktopGeometry(TQWidget* w)
{
    TQDesktopWidget *dw = TQApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        if (group.readBoolEntry("XineramaEnabled", true) &&
            group.readBoolEntry("XineramaPlacementEnabled", true)) {
            if (w)
                return dw->screenGeometry(dw->screenNumber(w));
            else return dw->screenGeometry(-1);
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

bool KGlobalSettings::showIconsOnPushButtons()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("ShowIconsOnPushButtons",
        KDE_DEFAULT_ICON_ON_PUSHBUTTON);
}

bool KGlobalSettings::showFilePreview(const KURL &url)
{
    KConfigGroup g(KGlobal::config(), "PreviewSettings");
    TQString protocol = url.protocol();
    bool defaultSetting = KProtocolInfo::showFilePreview( protocol );
    return g.readBoolEntry(protocol, defaultSetting );
}

bool KGlobalSettings::showKonqIconActivationEffect()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("ShowKonqIconActivationEffect",
        KDE_DEFAULT_KONQ_ACTIVATION_EFFECT);
}

bool KGlobalSettings::opaqueResize()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("OpaqueResize",
        KDE_DEFAULT_OPAQUE_RESIZE);
}

int KGlobalSettings::buttonLayout()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readNumEntry("ButtonLayout",
        KDE_DEFAULT_BUTTON_LAYOUT);
}
