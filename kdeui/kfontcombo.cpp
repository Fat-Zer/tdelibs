/* This file is part of the KDE libraries
   Copyright (c) 2001 Malte Starostik <malte@kde.org>

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


#include <tqfontdatabase.h>
#include <tqlistbox.h>
#include <tqpainter.h>
#include <tqregexp.h>

#include <kcharsets.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kfontdialog.h>

#include "kfontcombo.h"
#include "kfontcombo.moc"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xft/Xft.h>


struct KFontComboPrivate
{
    KFontComboPrivate()
        : bold(false),
          italic(false),
          underline(false),
          strikeOut(false),
	  modified(false),
          size(0),
          lineSpacing(0)
    {
    }

    bool bold : 1;
    bool italic : 1;
    bool underline : 1;
    bool strikeOut : 1;
    bool displayFonts : 1;
    bool modified : 1;
    int size;
    int lineSpacing;
    TQString defaultFamily;
};

class KFontListItem : public QListBoxItem
{
public:
    KFontListItem(const TQString &fontName, KFontCombo *combo);
    virtual ~KFontListItem();

    virtual int width(const TQListBox *) const;
    virtual int height(const TQListBox *) const;

    void updateFont();

protected:
    virtual void paint(TQPainter *p);

private:
    void createFont();

private:
    KFontCombo *m_combo;
    TQString m_fontName;
    TQFont *m_font;
    bool m_canPaintName;
};

KFontListItem::KFontListItem(const TQString &fontName, KFontCombo *combo)
    : TQListBoxItem(combo->listBox()),
      m_combo(combo),
      m_fontName(fontName),
      m_font(0),
      m_canPaintName(true)
{
    setText(fontName);
}

KFontListItem::~KFontListItem()
{
    delete m_font;
}

int KFontListItem::width(const TQListBox *lb) const
{
    if (m_font)
       return TQFontMetrics(*m_font).width(text()) + 6;
    return lb->fontMetrics().width(text()) + 6;
}

int KFontListItem::height(const TQListBox *lb) const
{
    if (m_combo->d->displayFonts)
        return m_combo->d->lineSpacing + 2;
    TQFontMetrics fm(lb->fontMetrics());
    return fm.lineSpacing() + 2;
}

void KFontListItem::paint(TQPainter *p)
{
    if (m_combo->d->displayFonts)
    {
        if (!m_font)
            createFont();

        TQString t = m_fontName;
        if (p->device() != m_combo)
        {
            if (m_canPaintName)
                p->setFont(*m_font);
            else
                t = TQString::fromLatin1("(%1)").arg(m_fontName);
        }
        TQFontMetrics fm(p->fontMetrics());
        p->drawText(3, (m_combo->d->lineSpacing + fm.ascent() + fm.leading() / 2) / 2, t);
    }
    else
    {
        TQFontMetrics fm(p->fontMetrics());
        p->drawText(3, fm.ascent() + fm.leading() / 2, m_fontName);
    }
}

void KFontListItem::updateFont()
{
    if (!m_font)
        return;

    m_font->setBold(m_combo->d->bold);
    m_font->setItalic(m_combo->d->italic);
    m_font->setUnderline(m_combo->d->underline);
    m_font->setStrikeOut(m_combo->d->strikeOut);
    m_font->setPointSize(m_combo->d->size);
}

void KFontListItem::createFont()
{
    if (m_font)
        return;

    m_font = new TQFont(m_fontName);
    TQFontMetrics fm(*m_font);
    for (unsigned int i = 0; i < m_fontName.length(); ++i)
        if (!fm.inFont(m_fontName[i]))
        {
            m_canPaintName = false;
            break;
        }
    updateFont();
}

KFontCombo::KFontCombo(TQWidget *parent, const char *name)
    : KComboBox(true, parent, name)
{
    init();
    TQStringList families;
    KFontChooser::getFontList(families, 0);
    setFonts(families);
}

KFontCombo::KFontCombo(const TQStringList &fonts, TQWidget *parent, const char *name)
    : KComboBox(true, parent, name)
{
    init();
    setFonts(fonts);
}

void KFontCombo::setFonts(const TQStringList &fonts)
{
    clear();
    for (TQStringList::ConstIterator it = fonts.begin(); it != fonts.end(); ++it)
        new KFontListItem(*it, this);
}

/*
 * Maintenance note: Keep in sync with KFontAction::setFont()
 */
void KFontCombo::setCurrentFont(const TQString &family)
{
    TQString lowerName = family.lower();
    int c = count();
    for(int i = 0; i < c; i++)
    {
       if (text(i).lower() == lowerName)
       {
          setCurrentItem(i);
          d->defaultFamily = text(i);
	  d->modified = false;
          return;
       }
    }
    int x = lowerName.find(" [");
    if (x>-1)
    {
       lowerName = lowerName.left(x);
       for(int i = 0; i < c; i++)
       {
          if (text(i).lower() == lowerName)
          {
             setCurrentItem(i);
             d->defaultFamily = text(i);
	     d->modified = false;
             return;
          }
       }
    }

    lowerName += " [";
    for(int i = 0; i < c; i++)
    {
       if (text(i).lower().startsWith(lowerName))
       {
          setCurrentItem(i);
          d->defaultFamily = text(i);
	  d->modified = false;
          return;
       }
    }

    // nothing matched yet, try a fontconfig reverse lookup and
    // check again to solve an alias
    FcPattern *pattern = NULL;
    FcConfig *config = NULL;
    TQString realFamily;
    TQRegExp regExp("[-:]");
    pattern = FcNameParse( (unsigned char*) family.ascii() );
    FcDefaultSubstitute(pattern);
    FcConfigSubstitute (config, pattern, FcMatchPattern);
    pattern = FcFontMatch(NULL, pattern, NULL);
    realFamily = (char*)FcNameUnparse(pattern);
    realFamily.remove(realFamily.find(regExp), realFamily.length());

    if ( !realFamily.isEmpty() && realFamily != family )
       setCurrentFont( realFamily );
}

void KFontCombo::slotModified( int )
{
   d->modified = 1;
}

TQString KFontCombo::currentFont() const
{
   if (d->modified)
      return currentText();
   return d->defaultFamily;
}

void KFontCombo::setCurrentItem(int i)
{
    d->modified = true;
    TQComboBox::setCurrentItem(i);
}

void KFontCombo::init()
{
    d = new KFontComboPrivate;
    d->displayFonts = displayFonts();
    setInsertionPolicy(NoInsertion);
    setAutoCompletion(true);
    setSize(12);
    connect( this, TQT_SIGNAL(highlighted(int)), TQT_SLOT(slotModified(int)));
}

KFontCombo::~KFontCombo()
{
    delete d;
}

void KFontCombo::setBold(bool bold)
{
    if (d->bold == bold)
        return;
    d->bold = bold;
    updateFonts();
}

bool KFontCombo::bold() const
{
    return d->bold;
}

void KFontCombo::setItalic(bool italic)
{
    if (d->italic == italic)
        return;
    d->italic = italic;
    updateFonts();
}

bool KFontCombo::italic() const
{
    return d->italic;
}

void KFontCombo::setUnderline(bool underline)
{
    if (d->underline == underline)
        return;
    d->underline = underline;
    updateFonts();
}

bool KFontCombo::underline() const
{
    return d->underline;
}

void KFontCombo::setStrikeOut(bool strikeOut)
{
    if (d->strikeOut == strikeOut)
        return;
    d->strikeOut = strikeOut;
    updateFonts();
}

bool KFontCombo::strikeOut() const
{
    return d->strikeOut;
}

void KFontCombo::setSize(int size)
{
    if (d->size == size)
        return;
    d->size = size;
    TQFont f;
    f.setPointSize(size);
    TQFontMetrics fm(f);
    d->lineSpacing = fm.lineSpacing();
    updateFonts();
}

int KFontCombo::size() const
{
    return d->size;
}

void KFontCombo::updateFonts()
{
    if (!d->displayFonts)
        return;

    for (unsigned int i = 0; i < listBox()->count(); ++i)
    {
        KFontListItem *item = static_cast<KFontListItem *>(listBox()->item(i));
        item->updateFont();
    }
}

bool KFontCombo::displayFonts()
{
    KConfigGroupSaver saver(KGlobal::config(), "KDE");
    return KGlobal::config()->readBoolEntry("DisplayFontItems", true);
}

void KFontCombo::virtual_hook( int id, void* data )
{ KComboBox::virtual_hook( id, data ); }

