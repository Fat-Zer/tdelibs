/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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
 **/

#include "kpfilterpage.h"
#include "kmfactory.h"
#include "kxmlcommand.h"

#include <tqtoolbutton.h>
#include <tqheader.h>
#include <tqtooltip.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <klistview.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kactivelabel.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kdialog.h>

KPFilterPage::KPFilterPage(TQWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	TQString whatsThisAddFilterButton = i18n(  " <qt> <b>Add Filter button</b>"
						" <p>This button calls a little dialog to let you"
						" select a filter here. </p>"
						" <p><b>Note 1:</b> You can chain different filters as long as you make "
						" sure that the output of one fits as input of the next. (KDEPrint "
						" checks your filtering chain and will warn you if you fail to do so.</p> "
						" <p><b>Note 2:</b> The filters you define here are applied to your jobfile "
						" <em><b>before</b></em> it is handed downstream to your spooler and print "
						" subsystem (e.g. CUPS, LPRng, LPD).</p>"
						" </ul>"
						" </qt>" );

	TQString whatsThisRemoveFilterButton = i18n(" <qt> <b>Remove Filter button</b>"
						" <p>This button removes the highlighted filter from the"
						" list of filters."
						" </qt>" );

	TQString whatsThisMoveFilterUpButton = i18n(" <qt> <b>Move Filter Up button</b>"
						" <p>This button moves the highlighted filter up in the list"
						" of filters, towards the front of the filtering chain. </p>"
						" </qt>" );

	TQString whatsThisMoveFilterDownButton = i18n(" <qt> <b>Move Filter Down button</b>"
						" <p>This button moves the highlighted filter down in the list"
						" of filters, towards the end of the filtering chain..</p>"
						" </qt>" );

	TQString whatsThisConfigureFilterButton = i18n( " <qt> <b>Configure Filter button</b>"
						" <p>This button lets you configure the currently highlighted filter."
						" It opens a separate dialog. "
						" </p>"
						" </qt>" );

	TQString whatsThisFilterInfoPane = i18n( " <qt> <b>Filter Info Pane</b>"
						" <p>This field shows some general info about the selected filter. "
						" Amongst them are: "
						" <ul> "
						" <li>the <em>filter name</em> (as displayed in the KDEPrint user interface); </li> "
						" <li>the <em>filter requirements</em> (that is the external program that needs "
						" to present and executable on this system); </li> "
						" <li>the <em>filter input format</em> (in the form of one or several <em>MIME types</em>"
						" accepted by the filter); </li> "
						" <li>the <em>filter output format</em> (in the form of a <em>MIME type</em> "
						" generated by the filter); </li> "
						" <li>a more or less verbose text describing the filter's operation.</li> "
						" </ul> "
						" </p>"
						" </qt>" );

	TQString whatsThisFilterchainListView = i18n( " <qt> <b>Filtering Chain</b> (if enabled, is run <em>before</em> actual "
						" job submission to print system)"
						" <p>This field shows which filters are currently selected to act as 'pre-filters' "
						" for KDEPrint. Pre-filters are processing the print files <em>before</em> they are "
						" send downstream to your real print subsystem. </p> "
						" <p>The list shown in this field may be empty (default). </p> "
						" <p>The pre-filters act on the printjob in the order they are listed (from top to bottom). "
						" This is done by acting as a <em>filtering chain</em> where the output of one filter "
						" acts as input to the next. By putting the filters into the wrong order, you can make " 
						" the filtering chain fail. For example: if your file is ASCII text, and you want the "
						" output being processed by the 'Multipage per Sheet' filter, the first filter must be "
						" one that processes ASCII into PostScript. </p> "
						" <p>KDEPrint can utilize <em>any</em> external filtering program which you may find useful "
						" through this interface. </p> "
						" <p>KDEPrint ships preconfigured with support for a selection of common filters. These "
						" filters however need to be "
						" installed independently from KDEPrint. These pre-filters work <em>for all</em> print subsystems"
						" supported by KDEPrint (such as CUPS, LPRng and LPD), because they are not depending on these.</p> "
						".<p> Amongst the pre-configured filters shipping with KDEPrint are: </p> "
						" <ul> "
						" <li>the <em>Enscript text filter</em>  </li> "
						" <li>a <em>Multiple Pages per Sheet filter</em </li> "
						" <li>a <em>PostScript to PDF converter</em>.</li> "
						" <li>a <em>Page Selection/Ordering filter</em>.</li> "
						" <li>a <em>Poster Printing filter</em>.</li> "
						" <li>and some more..</li> "
						" </ul> "
						" To insert a filter into this list, simply click on the <em>funnel</em> icon (topmost on "
						" the right icon column group) and proceed. </p>"
						" <p>Please click on the other elements of this dialog to learn more about the KDEPrint "
						" pre-filters. "
						" </p>"
						" </qt>" );

	setTitle(i18n("Filters"));
	m_activefilters.setAutoDelete(true);
	m_valid = true;

	m_view = new KListView(this);
	  TQWhatsThis::add(m_view, whatsThisFilterchainListView);
	m_view->addColumn("");
	m_view->setFrameStyle(TQFrame::WinPanel|TQFrame::Sunken);
	m_view->setLineWidth(1);
	m_view->setSorting(-1);
	m_view->header()->hide();
	connect(m_view,TQT_SIGNAL(selectionChanged(TQListViewItem*)),TQT_SLOT(slotItemSelected(TQListViewItem*)));

	m_add = new TQToolButton(this);
	  TQWhatsThis::add(m_add, whatsThisAddFilterButton);
	m_add->setIconSet(BarIconSet("filter"));
	TQToolTip::add(m_add, i18n("Add filter"));

	m_remove = new TQToolButton(this);
	  TQWhatsThis::add(m_remove, whatsThisRemoveFilterButton);
	m_remove->setIconSet(BarIconSet("remove"));
	TQToolTip::add(m_remove, i18n("Remove filter"));

	m_up = new TQToolButton(this);
	  TQWhatsThis::add(m_up, whatsThisMoveFilterUpButton);
	m_up->setIconSet(BarIconSet("up"));
	TQToolTip::add(m_up, i18n("Move filter up"));

	m_down = new TQToolButton(this);
	  TQWhatsThis::add(m_down, whatsThisMoveFilterDownButton);
	m_down->setIconSet(BarIconSet("down"));
	TQToolTip::add(m_down, i18n("Move filter down"));

	m_configure = new TQToolButton(this);
	  TQWhatsThis::add(m_configure, whatsThisConfigureFilterButton);
	m_configure->setIconSet(BarIconSet("configure"));
	TQToolTip::add(m_configure, i18n("Configure filter"));

	connect(m_add,TQT_SIGNAL(clicked()),TQT_SLOT(slotAddClicked()));
	connect(m_remove,TQT_SIGNAL(clicked()),TQT_SLOT(slotRemoveClicked()));
	connect(m_up,TQT_SIGNAL(clicked()),TQT_SLOT(slotUpClicked()));
	connect(m_down,TQT_SIGNAL(clicked()),TQT_SLOT(slotDownClicked()));
	connect(m_configure,TQT_SIGNAL(clicked()),TQT_SLOT(slotConfigureClicked()));
	connect(m_view,TQT_SIGNAL(doubleClicked(TQListViewItem*)),TQT_SLOT(slotConfigureClicked()));

	m_info = new KActiveLabel(this);
	  TQWhatsThis::add(m_info, whatsThisFilterInfoPane);
	m_info->setVScrollBarMode( TQScrollView::Auto );
	m_info->setHScrollBarMode( TQScrollView::Auto );
	m_info->setFrameStyle( TQFrame::Panel|TQFrame::Sunken );
	m_info->setMinimumSize( TQSize( 240, 100 ) );

	TQGridLayout	*l1 = new TQGridLayout(this, 2, 2, 0, KDialog::spacingHint());
	l1->setColStretch(0, 1);
	TQVBoxLayout	*l2 = new TQVBoxLayout(0, 0, 1);
	l1->addWidget(m_view, 0, 0);
	l1->addLayout(l2, 0, 1);
	l2->addWidget(m_add);
	l2->addWidget(m_remove);
	l2->addSpacing(5);
	l2->addWidget(m_up);
	l2->addWidget(m_down);
	l2->addSpacing(5);
	l2->addWidget(m_configure);
	l2->addStretch(1);
	l1->addMultiCellWidget(m_info, 1, 1, 0, 1);
	slotItemSelected(0);

	resize(100,50);
}

KPFilterPage::~KPFilterPage()
{
}

void KPFilterPage::updateButton()
{
/*  TQListViewItem	*item = m_view->currentItem();
  bool state=(item!=0);
  m_remove->setEnabled(state);
  m_up->setEnabled((state && item->itemAbove() != 0));
  m_down->setEnabled((state && item->itemBelow() != 0));
  m_configure->setEnabled(state);*/
}

void KPFilterPage::slotAddClicked()
{
	bool	ok;
	TQString choice = KXmlCommandManager::self()->selectCommand( this );
	ok = !choice.isEmpty();
	if (ok)
	{
		KXmlCommand	*cmd = KXmlCommandManager::self()->loadCommand(choice);
		if (!cmd) return; // Error
		TQStringList	filters = activeList();
		int		pos = KXmlCommandManager::self()->insertCommand(filters, cmd->name());
		TQListViewItem	*prev(0);
		if (pos > 0)
		{
			prev = m_view->firstChild();
			for (int i=1;prev && i<pos;i++)
				prev = prev->nextSibling();
		}
		m_activefilters.insert(cmd->name(), cmd);
		TQListViewItem	*item = new TQListViewItem(m_view, prev, cmd->description(), cmd->name());
		item->setPixmap(0, SmallIcon("filter"));
		checkFilterChain();
	}
}

void KPFilterPage::slotRemoveClicked()
{
	if (m_view->selectedItem())
	{
		TQString	idname = m_view->selectedItem()->text(1);
		delete m_view->selectedItem();
		m_activefilters.remove(idname);
		checkFilterChain();
		if (m_view->currentItem())
			m_view->setSelected(m_view->currentItem(), true);
		slotItemSelected(m_view->currentItem());
	}
}

void KPFilterPage::slotUpClicked()
{
	TQListViewItem	*item = m_view->selectedItem();
	if (item && item->itemAbove())
	{
		TQListViewItem	*clone = new TQListViewItem(m_view,item->itemAbove()->itemAbove(),item->text(0),item->text(1));
		clone->setPixmap(0, SmallIcon("filter"));
		delete item;
		m_view->setSelected(clone, true);
		checkFilterChain();
	}
}

void KPFilterPage::slotDownClicked()
{
	TQListViewItem	*item = m_view->selectedItem();
	if (item && item->itemBelow())
	{
		TQListViewItem	*clone = new TQListViewItem(m_view,item->itemBelow(),item->text(0),item->text(1));
		clone->setPixmap(0, SmallIcon("filter"));
		delete item;
		m_view->setSelected(clone, true);
		checkFilterChain();
	}
}

void KPFilterPage::slotConfigureClicked()
{
	KXmlCommand	*filter = currentFilter();
	if (!filter || !KXmlCommandManager::self()->configure(filter, this))
		KMessageBox::error(this,i18n("Internal error: unable to load filter."));
}

void KPFilterPage::slotItemSelected(TQListViewItem *item)
{
	m_remove->setEnabled((item != 0));
	m_up->setEnabled((item != 0 && item->itemAbove() != 0));
	m_down->setEnabled((item != 0 && item->itemBelow() != 0));
	m_configure->setEnabled((item != 0));
	updateInfo();
}

void KPFilterPage::setOptions(const TQMap<TQString,TQString>& opts)
{
	TQStringList	filters = TQStringList::split(',',opts["_kde-filters"],false);
	// remove unneeded filters
	TQDictIterator<KXmlCommand>	dit(m_activefilters);
	for (;dit.current();)
	{
		if (filters.find(dit.currentKey()) == filters.end())
			m_activefilters.remove(dit.currentKey());
		else
		{
			dit.current()->setOptions(opts);
			++dit;
		}
	}
	// add needed filters
	m_view->clear();
	TQListViewItem	*item(0);
	for (TQStringList::ConstIterator sit=filters.begin(); sit!=filters.end(); ++sit)
	{
		KXmlCommand	*f(0);
		if ((f=m_activefilters.find(*sit)) == 0)
		{
			f = KXmlCommandManager::self()->loadCommand(*sit);
			if (f)
			{
				m_activefilters.insert(*sit,f);
				f->setOptions(opts);
			}
		}
		if (f)
			item = new TQListViewItem(m_view,item,f->description(),f->name());
	}
	checkFilterChain();
}

void KPFilterPage::getOptions(TQMap<TQString,TQString>& opts, bool incldef)
{
	TQStringList	filters = activeList();
	for (TQStringList::ConstIterator it=filters.begin(); it!=filters.end(); ++it)
	{
		KXmlCommand	*f = m_activefilters.find(*it);
		if (f)
			f->getOptions(opts, incldef);
	}
	if (filters.count() > 0 || incldef)
	{
		opts["_kde-filters"] = filters.join(",");
	}
}

TQStringList KPFilterPage::activeList()
{
	TQStringList	list;
	TQListViewItem	*item = m_view->firstChild();
	while (item)
	{
		list.append(item->text(1));
		item = item->nextSibling();
	}
	return list;
}

KXmlCommand* KPFilterPage::currentFilter()
{
	KXmlCommand	*filter(0);
	if (m_view->selectedItem())
		filter = m_activefilters.find(m_view->selectedItem()->text(1));
	return filter;
}

void KPFilterPage::checkFilterChain()
{
	TQListViewItem	*item = m_view->firstChild();
	bool		ok(true);
	m_valid = true;
	while (item)
	{
		item->setPixmap(0, (ok ? SmallIcon("filter") : SmallIcon("filterstop")));
		KXmlCommand	*f1 = m_activefilters.find(item->text(1));
		if (f1 && item->nextSibling())
		{
			KXmlCommand	*f2 = m_activefilters.find(item->nextSibling()->text(1));
			if (f2)
			{
				if (!f2->acceptMimeType(f1->mimeType()))
				{
					item->setPixmap(0, SmallIcon("filterstop"));
					ok = false;
					m_valid = false;
				}
				else
					ok = true;
			}
		}
		item = item->nextSibling();
	}
}

bool KPFilterPage::isValid(TQString& msg)
{
	if (!m_valid)
	{
		msg = i18n("<p>The filter chain is wrong. The output format of at least one filter is not supported by its follower. See <b>Filters</b> tab for more information.</p>");
	}
	return m_valid;
}

void KPFilterPage::updateInfo()
{
	TQString	txt;
	KXmlCommand	*f = currentFilter();
	if (f)
	{
		TQString	templ("<b>%1:</b> %2<br>");
		txt.append(templ.arg(i18n("Name")).arg(f->name()));
		txt.append(templ.arg(i18n("Requirements")).arg(f->requirements().join(", ")));
		txt.append(templ.arg(i18n("Input")).arg(f->inputMimeTypes().join(", ")));
		txt.append(templ.arg(i18n("Output")).arg(f->mimeType()));
		if ( !f->comment().isEmpty() )
			txt.append( "<br>" ).append( f->comment() );
	}
	m_info->setText(txt);
}

#include "kpfilterpage.moc"
