/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben+kdelibs at meyerhome dot net)
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kconfigdialogmanager.h"

#include <tqbuttongroup.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqmetaobject.h>
#include <tqobjectlist.h>
#include <tqsqlpropertymap.h>
#include <tqtimer.h>
#include <tqwhatsthis.h>

#include <kapplication.h>
#include <kconfigskeleton.h>
#include <kdebug.h>
#include <kglobal.h>

#include <assert.h>

class KConfigDialogManager::Private {

public:
  Private() : insideGroupBox(false) { }

public:
  TQDict<TQWidget> knownWidget;
  TQDict<TQWidget> buddyWidget;
  bool insideGroupBox;
};

KConfigDialogManager::KConfigDialogManager(TQWidget *parent, KConfigSkeleton *conf, const char *name)
 : TQObject(parent, name), m_conf(conf), m_dialog(parent)
{
  d = new Private();

  kapp->installKDEPropertyMap();
  propertyMap = TQSqlPropertyMap::defaultMap();

  init(true);
}

KConfigDialogManager::~KConfigDialogManager()
{
  delete d;
}

void KConfigDialogManager::init(bool trackChanges)
{
  if(trackChanges)
  {
    // QT
    changedMap.insert("QButton", TQT_SIGNAL(stateChanged(int)));
    changedMap.insert("QCheckBox", TQT_SIGNAL(stateChanged(int)));
    changedMap.insert("QPushButton", TQT_SIGNAL(stateChanged(int)));
    changedMap.insert("QRadioButton", TQT_SIGNAL(stateChanged(int)));
    // We can only store one thing, so you can't have
    // a ButtonGroup that is checkable.
    changedMap.insert("QButtonGroup", TQT_SIGNAL(clicked(int)));
    changedMap.insert("QGroupBox", TQT_SIGNAL(toggled(bool)));
    changedMap.insert("QComboBox", TQT_SIGNAL(activated (int)));
    //qsqlproperty map doesn't store the text, but the value!
    //changedMap.insert("QComboBox", TQT_SIGNAL(textChanged(const TQString &)));
    changedMap.insert("QDateEdit", TQT_SIGNAL(valueChanged(const TQDate &)));
    changedMap.insert("QDateTimeEdit", TQT_SIGNAL(valueChanged(const TQDateTime &)));
    changedMap.insert("QDial", TQT_SIGNAL(valueChanged (int)));
    changedMap.insert("QLineEdit", TQT_SIGNAL(textChanged(const TQString &)));
    changedMap.insert("QSlider", TQT_SIGNAL(valueChanged(int)));
    changedMap.insert("QSpinBox", TQT_SIGNAL(valueChanged(int)));
    changedMap.insert("QTimeEdit", TQT_SIGNAL(valueChanged(const TQTime &)));
    changedMap.insert("QTextEdit", TQT_SIGNAL(textChanged()));
    changedMap.insert("QTextBrowser", TQT_SIGNAL(sourceChanged(const TQString &)));
    changedMap.insert("QMultiLineEdit", TQT_SIGNAL(textChanged()));
    changedMap.insert("QListBox", TQT_SIGNAL(selectionChanged()));
    changedMap.insert("QTabWidget", TQT_SIGNAL(currentChanged(TQWidget *)));

    // KDE
    changedMap.insert( "KComboBox", TQT_SIGNAL(activated (int)));
    changedMap.insert( "KFontCombo", TQT_SIGNAL(activated (int)));
    changedMap.insert( "KFontRequester", TQT_SIGNAL(fontSelected(const TQFont &)));
    changedMap.insert( "KFontChooser",  TQT_SIGNAL(fontSelected(const TQFont &)));
    changedMap.insert( "KHistoryCombo", TQT_SIGNAL(activated (int)));

    changedMap.insert( "KColorButton", TQT_SIGNAL(changed(const TQColor &)));
    changedMap.insert( "KDatePicker", TQT_SIGNAL(dateSelected (TQDate)));
    changedMap.insert( "KDateWidget", TQT_SIGNAL(changed (TQDate)));
    changedMap.insert( "KDateTimeWidget", TQT_SIGNAL(valueChanged (const TQDateTime &)));
    changedMap.insert( "KEditListBox", TQT_SIGNAL(changed()));
    changedMap.insert( "KListBox", TQT_SIGNAL(selectionChanged()));
    changedMap.insert( "KLineEdit", TQT_SIGNAL(textChanged(const TQString &)));
    changedMap.insert( "KPasswordEdit", TQT_SIGNAL(textChanged(const TQString &)));
    changedMap.insert( "KRestrictedLine", TQT_SIGNAL(textChanged(const TQString &)));
    changedMap.insert( "KTextBrowser", TQT_SIGNAL(sourceChanged(const TQString &)));
    changedMap.insert( "KTextEdit", TQT_SIGNAL(textChanged()));
    changedMap.insert( "KURLRequester",  TQT_SIGNAL(textChanged (const TQString& )));
    changedMap.insert( "KIntNumInput", TQT_SIGNAL(valueChanged (int)));
    changedMap.insert( "KIntSpinBox", TQT_SIGNAL(valueChanged (int)));
    changedMap.insert( "KDoubleNumInput", TQT_SIGNAL(valueChanged (double)));
  }

  // Go through all of the children of the widgets and find all known widgets
  (void) parseChildren(m_dialog, trackChanges);
}

void KConfigDialogManager::addWidget(TQWidget *widget)
{
  (void) parseChildren(widget, true);
}

void KConfigDialogManager::setupWidget(TQWidget *widget, KConfigSkeletonItem *item)
{
  TQVariant minValue = item->minValue();
  if (minValue.isValid())
  {
    if (widget->metaObject()->findProperty("minValue", true) != -1)
       widget->setProperty("minValue", minValue);
  }
  TQVariant maxValue = item->maxValue();
  if (maxValue.isValid())
  {
    if (widget->metaObject()->findProperty("maxValue", true) != -1)
       widget->setProperty("maxValue", maxValue);
  }
  if (TQWhatsThis::textFor( widget ).isEmpty())
  {
    TQString whatsThis = item->whatsThis();
    if ( !whatsThis.isEmpty() )
    {
      TQWhatsThis::add( widget, whatsThis );
    }
  }
}

bool KConfigDialogManager::parseChildren(const TQWidget *widget, bool trackChanges)
{
  bool valueChanged = false;
  const TQObjectList *listOfChildren = widget->children();
  if(!listOfChildren)
    return valueChanged;

  TQObject *object;
  for( TQObjectListIterator it( *listOfChildren );
       (object = it.current()); ++it )
  {
    if(!object->isWidgetType())
      continue; // Skip non-widgets

    TQWidget *childWidget = (TQWidget *)object;

    const char *widgetName = childWidget->name(0);
    bool bParseChildren = true;
    bool bSaveInsideGroupBox = d->insideGroupBox;

    if (widgetName && (strncmp(widgetName, "kcfg_", 5) == 0))
    {
      // This is one of our widgets!
      TQString configId = widgetName+5;
      KConfigSkeletonItem *item = m_conf->findItem(configId);
      if (item)
      {
        d->knownWidget.insert(configId, childWidget);

        setupWidget(childWidget, item);

        TQMap<TQString, TQCString>::const_iterator changedIt = changedMap.find(childWidget->className());

        if (changedIt == changedMap.end())
        {
		   // If the class name of the widget wasn't in the monitored widgets map, then look for 
		   // it again using the super class name. This fixes a problem with using QtRuby/Korundum 
		   // widgets with KConfigXT where 'Qt::Widget' wasn't being seen a the real deal, even 
		   // though it was a 'QWidget'.
          changedIt = changedMap.find(childWidget->metaObject()->superClassName());
        }

        if (changedIt == changedMap.end())
        {
          kdWarning(178) << "Don't know how to monitor widget '" << childWidget->className() << "' for changes!" << endl;
        }
        else
        {
          connect(childWidget, *changedIt,
                  this, TQT_SIGNAL(widgetModified()));

          TQGroupBox *gb = dynamic_cast<TQGroupBox *>(childWidget);
          if (!gb)
            bParseChildren = false;
          else
            d->insideGroupBox = true;

          TQComboBox *cb = dynamic_cast<TQComboBox *>(childWidget);
          if (cb && cb->editable())
            connect(cb, TQT_SIGNAL(textChanged(const TQString &)),
                    this, TQT_SIGNAL(widgetModified()));
        }
      }
      else
      {
        kdWarning(178) << "A widget named '" << widgetName << "' was found but there is no setting named '" << configId << "'" << endl;
        assert(false);
      }
    }
    else if (childWidget->inherits("QLabel"))
    {
      TQLabel *label = static_cast<TQLabel *>(childWidget);
      TQWidget *buddy = label->buddy();
      if (!buddy)
        continue;
      const char *buddyName = buddy->name(0);
      if (buddyName && (strncmp(buddyName, "kcfg_", 5) == 0))
      {
        // This is one of our widgets!
        TQString configId = buddyName+5;
        d->buddyWidget.insert(configId, childWidget);
      }
    }
#ifndef NDEBUG
    else if (widgetName)
    {
      TQMap<TQString, TQCString>::const_iterator changedIt = changedMap.find(childWidget->className());
      if (changedIt != changedMap.end())
      {
        if ((!d->insideGroupBox || !childWidget->inherits("QRadioButton")) && 
            !childWidget->inherits("QGroupBox"))
          kdDebug(178) << "Widget '" << widgetName << "' (" << childWidget->className() << ") remains unmanaged." << endl;
      }        
    }
#endif

    if(bParseChildren)
    {
      // this widget is not known as something we can store.
      // Maybe we can store one of its children.
      valueChanged |= parseChildren(childWidget, trackChanges);
    }
    d->insideGroupBox = bSaveInsideGroupBox;
  }
  return valueChanged;
}

void KConfigDialogManager::updateWidgets()
{
  bool changed = false;
  bool bSignalsBlocked = signalsBlocked();
  blockSignals(true);

  TQWidget *widget;
  for( TQDictIterator<TQWidget> it( d->knownWidget );
       (widget = it.current()); ++it )
  {
     KConfigSkeletonItem *item = m_conf->findItem(it.currentKey());
     if (!item)
     {
        kdWarning(178) << "The setting '" << it.currentKey() << "' has disappeared!" << endl;
        continue;
     }

     TQVariant p = item->property();
     if (p != property(widget))
     {
        setProperty(widget, p);
//        kdDebug(178) << "The setting '" << it.currentKey() << "' [" << widget->className() << "] has changed" << endl;
        changed = true;
     }
     if (item->isImmutable())
     {
        widget->setEnabled(false);
        TQWidget *buddy = d->buddyWidget.find(it.currentKey());
        if (buddy)
           buddy->setEnabled(false);
     }
  }
  blockSignals(bSignalsBlocked);

  if (changed)
    TQTimer::singleShot(0, this, TQT_SIGNAL(widgetModified()));
}

void KConfigDialogManager::updateWidgetsDefault()
{
  bool bUseDefaults = m_conf->useDefaults(true);
  updateWidgets();
  m_conf->useDefaults(bUseDefaults);
}

void KConfigDialogManager::updateSettings()
{
  bool changed = false;

  TQWidget *widget;
  for( TQDictIterator<TQWidget> it( d->knownWidget );
       (widget = it.current()); ++it )
  {
     KConfigSkeletonItem *item = m_conf->findItem(it.currentKey());
     if (!item)
     {
        kdWarning(178) << "The setting '" << it.currentKey() << "' has disappeared!" << endl;
        continue;
     }

     TQVariant p = property(widget);
     if (p != item->property())
     {
        item->setProperty(p);
        changed = true;
     }
  }
  if (changed)
  {
     m_conf->writeConfig();
     emit settingsChanged();
  }
}

void KConfigDialogManager::setProperty(TQWidget *w, const TQVariant &v)
{
  TQButtonGroup *bg = dynamic_cast<TQButtonGroup *>(w);
  if (bg)
  {
    bg->setButton(v.toInt());
    return;
  }

  TQComboBox *cb = dynamic_cast<TQComboBox *>(w);
  if (cb && cb->editable())
  {
    cb->setCurrentText(v.toString());
    return;
  }

  propertyMap->setProperty(w, v);
}

TQVariant KConfigDialogManager::property(TQWidget *w)
{
  TQButtonGroup *bg = dynamic_cast<TQButtonGroup *>(w);
  if (bg)
    return TQVariant(bg->selectedId());

  TQComboBox *cb = dynamic_cast<TQComboBox *>(w);
  if (cb && cb->editable())
      return TQVariant(cb->currentText());

  return propertyMap->property(w);
}

bool KConfigDialogManager::hasChanged()
{

  TQWidget *widget;
  for( TQDictIterator<TQWidget> it( d->knownWidget );
       (widget = it.current()); ++it )
  {
     KConfigSkeletonItem *item = m_conf->findItem(it.currentKey());
     if (!item)
     {
        kdWarning(178) << "The setting '" << it.currentKey() << "' has disappeared!" << endl;
        continue;
     }

     TQVariant p = property(widget);
     if (p != item->property())
     {
//        kdDebug(178) << "Widget for '" << it.currentKey() << "' has changed." << endl;
        return true;
     }
  }
  return false;
}

bool KConfigDialogManager::isDefault()
{
  bool bUseDefaults = m_conf->useDefaults(true);
  bool result = !hasChanged();
  m_conf->useDefaults(bUseDefaults);
  return result;
}

#include "kconfigdialogmanager.moc"

