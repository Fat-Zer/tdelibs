/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __KIconLoader_p_h_Included__
#define __KIconLoader_p_h_Included__

#include <tqobject.h>
#include <tqstringlist.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <tqdict.h>

class KIconThemeNode
{
public:
    KIconThemeNode(KIconTheme *_theme);
    ~KIconThemeNode();

    void queryIcons(TQStringList *lst, int size, KIcon::Context context) const;
    void queryIconsByContext(TQStringList *lst, int size, KIcon::Context context) const;
    KIcon findIcon(const TQString& name, int size, KIcon::MatchType match) const;
    void printTree(TQString& dbgString) const;

    KIconTheme *theme;
};

class KIconLoaderPrivate : public QObject
{
    Q_OBJECT
public:
    TQStringList mThemesInTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    KStandardDirs *mpDirs;
    KIconLoader *q;
    KIconEffect mpEffect;
    TQDict<TQImage> imgDict;
    TQImage lastImage; // last loaded image without effect applied
    TQString lastImageKey; // key for icon without effect
    TQString appname;
    int lastIconType; // see KIcon::type
    int lastIconThreshold; // see KIcon::threshold
    TQPtrList<KIconThemeNode> links;
    bool extraDesktopIconsLoaded;
    bool delayedLoading;

public slots:
    void reconfigure();
};

#endif // __KIconLoader_p_h_Included__
