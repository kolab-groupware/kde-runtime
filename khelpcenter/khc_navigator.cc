/*
 *  khc_navigator.cc - part of the KDE Help Center
 *
 *  Copyright (C) 1999 Matthias Elter (me@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "khc_navigator.h"
#include "khc_navigatoritem.h"
#include "khc_indexwidget.h"
#include "khc_searchwidget.h"

#include <qdir.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qtabbar.h>

#include <kapp.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>	
#include <kglobal.h>
#include <klocale.h>

khcNavigator::khcNavigator(QWidget *parent, const char *name)
    : QWidget(parent,name)
{
    tabBar = new QTabBar(this);
        
    setupContentsTab();
    setupIndexTab();
    setupSearchTab();

    connect(tabBar, SIGNAL(selected(int)),this,
	    SLOT(slotTabSelected(int)));

    buildTree();
}

khcNavigator::~khcNavigator()
{
    delete tree;
    delete search;
    delete index;
    delete tabBar;
}

void khcNavigator::resizeEvent(QResizeEvent *)
{
    tabBar->setGeometry(0, 0, width(), 28);
    tree->setGeometry(0, 28, width(), height()-28);
    search->setGeometry(0, 28, width(), height()-28);
    index->setGeometry(0, 28, width(), height()-28);
}

void khcNavigator::setupContentsTab()
{
    tree = new KTreeList(this);

    tree->setSmoothScrolling(true);
    tree->setTreeDrawing(false);
    tree->setExpandButtonDrawing(false);
    tree->setIndentSpacing(0);

    connect(tree, SIGNAL(highlighted(int)),this,
	    SLOT(slotItemSelected(int)));
    connect(tree, SIGNAL(selected(int)),this,
	    SLOT(slotItemSelected(int)));

    QTab *newTab = new QTab;
    newTab->label = "Contents";
    tabBar->addTab(newTab);
    tree->show();
}

void khcNavigator::setupIndexTab()
{
    index = new IndexWidget(this);
    index->hide();
 
    QTab *newTab = new QTab;
    newTab->label = "Man/Info";
    tabBar->addTab(newTab);
}

void khcNavigator::setupSearchTab()
{
    search = new SearchWidget(this);
    search->hide();
  
    connect(search, SIGNAL(matchSelected(QString)),this,
	    SLOT(slotURLSelected(QString)));

    QTab *newTab = new QTab;
    newTab->label = "Search";
    tabBar->addTab(newTab);
}

void khcNavigator::buildTree()
{
    // introduction document
    khcNavigatorItem *ti_intro = new khcNavigatorItem(i18n("Introduction"), "helpdoc.xpm");
    ti_intro->setURL(QString("file:" + locate("html", "default/khelpcenter/main.html")));
    ti_intro->insertInTree(tree, 0);
    staticItems.append(ti_intro);
    
    // KDE quickstart guide
    khcNavigatorItem *ti_qs = new khcNavigatorItem(i18n("Introduction to KDE"), "helpdoc.xpm");
    ti_qs->setURL(QString("file:" + locate("html", "default/khelpcenter/quickstart/index.html")));
    ti_qs->insertInTree(tree, 0);
    staticItems.append(ti_qs);

    // KDE user's manual
    khcNavigatorItem *ti_um = new khcNavigatorItem(i18n("KDE user's manual"), "helpdoc.xpm");
    ti_um->setURL(QString("file:" + locate("html", "default/khelpcenter/userguide/index.html")));
    ti_um->insertInTree(tree, 0);
    staticItems.append(ti_um);
  
    // set the introduction page as current document
    tree->setCurrentItem(tree->itemIndex(ti_intro));

    // application manuals
    khcNavigatorItem *ti_manual = new khcNavigatorItem(i18n("Application manuals"), "helpbook.xpm");
    ti_manual->insertInTree(tree, 0);
    staticItems.append(ti_manual);

    // fill the application manual subtree
    buildManualSubTree(ti_manual);

    // unix man pages
    khcNavigatorItem *ti_man = new khcNavigatorItem(i18n("Unix manual pages"), "helpbook.xpm");
    //ti_man->setURL(QString("man:/(index)"));
    ti_man->insertInTree(tree, 0);
    staticItems.append(ti_man);

    // fill the man pages subtree
    buildManSubTree(ti_man);

    // info browser 
    khcNavigatorItem *ti_info = new khcNavigatorItem(i18n("Browse info pages"), "helpdoc.xpm");
    ti_info->setURL(QString("file:/cgi-bin/info2html"));
    ti_info->insertInTree(tree,0);
    staticItems.append(ti_info);

    // scan plugin dir for plugins
    insertPlugins();

    // KDE FAQ
    khcNavigatorItem *ti_faq = new khcNavigatorItem(i18n("The KDE FAQ"), "helpdoc.xpm");
    ti_faq->setURL(QString("file:" + locate("html", "default/khelpcenter/faq/index.html")));
    ti_faq->insertInTree(tree, 0);
    staticItems.append(ti_faq);

    // kde links
    khcNavigatorItem *ti_links = new khcNavigatorItem(i18n("KDE on the web"), "helpdoc.xpm");
    ti_links->setURL(QString("file:" + locate("html", "default/khelpcenter/links.html")));
    ti_links->insertInTree(tree,0);
    staticItems.append(ti_links);

    // kde contacts
    khcNavigatorItem *ti_contact = new khcNavigatorItem(i18n("Contact Information"), "helpdoc.xpm");
    ti_contact->setURL(QString("file:" + locate("html", "default/khelpcenter/contact.html")));
    ti_contact->insertInTree(tree,0);
    staticItems.append(ti_contact);  
}

void khcNavigator::clearTree()
{
    tree->clear();

    while(!staticItems.isEmpty())
	staticItems.removeFirst();

    while(!manualItems.isEmpty())
	manualItems.removeFirst();

    while(!pluginItems.isEmpty())
	pluginItems.removeFirst();
}

void khcNavigator::buildManSubTree(khcNavigatorItem *parent)
{
    // man (1)
    khcNavigatorItem *ti_man_s1 = new khcNavigatorItem("(1) User commands", "helpdoc.xpm");
    ti_man_s1->setURL(QString("man:/(1)"));
    ti_man_s1->insertInTree(tree, parent);
    staticItems.append(ti_man_s1);
  
    // man(2)
    khcNavigatorItem *ti_man_s2 = new khcNavigatorItem("(2) System calls", "helpdoc.xpm");
    ti_man_s2->setURL(QString("man:/(2)"));
    ti_man_s2->insertInTree(tree, parent);
    staticItems.append(ti_man_s2);

    // man(3)
    khcNavigatorItem *ti_man_s3 = new khcNavigatorItem("(3) Subroutines", "helpdoc.xpm");
    ti_man_s3->setURL(QString("man:/(3)"));
    ti_man_s3->insertInTree(tree, parent);
    staticItems.append(ti_man_s3);

    // man(4)
    khcNavigatorItem *ti_man_s4 = new khcNavigatorItem("(4) Devices", "helpdoc.xpm");
    ti_man_s4->setURL(QString("man:/(4)"));
    ti_man_s4->insertInTree(tree, parent);
    staticItems.append(ti_man_s4);

    // man(5)
    khcNavigatorItem *ti_man_s5 = new khcNavigatorItem("(5) File Formats", "helpdoc.xpm");
    ti_man_s5->setURL(QString("man:/(5)"));
    ti_man_s5->insertInTree(tree, parent);
    staticItems.append(ti_man_s5);

    // man(6)
    khcNavigatorItem *ti_man_s6 = new khcNavigatorItem("(6) Games", "helpdoc.xpm");
    ti_man_s6->setURL(QString("man:/(6)"));
    ti_man_s6->insertInTree(tree, parent);
    staticItems.append(ti_man_s6);

    // man(7)
    khcNavigatorItem *ti_man_s7 = new khcNavigatorItem("(7) Miscellaneous", "helpdoc.xpm");
    ti_man_s7->setURL(QString("man:/(7)"));
    ti_man_s7->insertInTree(tree, parent);
    staticItems.append(ti_man_s7);

    // man(8)
    khcNavigatorItem *ti_man_s8 = new khcNavigatorItem("(8) Sys. Administration", "helpdoc.xpm");
    ti_man_s8->setURL(QString("man:/(8)"));
    ti_man_s8->insertInTree(tree, parent);
    staticItems.append(ti_man_s8);

    // man(9)
    khcNavigatorItem *ti_man_s9 = new khcNavigatorItem("(9) Kernel", "helpdoc.xpm");
    ti_man_s9->setURL(QString("man:/(9)"));
    ti_man_s9->insertInTree(tree, parent);
    staticItems.append(ti_man_s9);

    // man(n)
    khcNavigatorItem *ti_man_sn = new khcNavigatorItem("(n) New", "helpdoc.xpm");
    ti_man_sn->setURL(QString("man:/(n)"));
    ti_man_sn->insertInTree(tree, parent);
    staticItems.append(ti_man_sn);
}

void khcNavigator::buildManualSubTree(khcNavigatorItem *parent)
{
    // System applications
    QString appPath = kapp->kde_appsdir();
    processDir(appPath, parent, &manualItems);
    appendEntries(appPath, parent, &manualItems);

    /*
      // User applications
      appPath = KApplication::localkdedir() + "/share/applnk";
      processDir(appPath, userManualTop);
      appendEntries(appPath, userManualTop);
    */
}

void khcNavigator::insertPlugins()
{
    // Scan plugin dir
    QString path = kapp->kde_datadir() + "/khelpcenter/plugins";
    processDir(path, 0, &pluginItems);
    appendEntries(path, 0, &pluginItems);
}

void khcNavigator::slotReloadTree()
{
    emit setBussy(true);
    clearTree();
    buildTree();
    emit setBussy(false);
}

void khcNavigator::slotTabSelected(int id)
{
    if (id == 0)
    {
	tree->show();
	index->hide();
	search->hide();
    }
    else if (id == 1)
    {
	tree->hide();
	index->show();
	search->hide();
	index->tabSelected();
    }
    else if (id == 2)
    {
	tree->hide();
	index->hide();
	search->show();
	search->tabSelected();
    }
}

void khcNavigator::slotURLSelected(QString url)
{
    emit itemSelected(url);
}

void khcNavigator::slotItemSelected(int)
{
    khcNavigatorItem *item;
    KTreeListItem *currentItem;

    // get a pointer to the highlighted item
    currentItem = tree->getCurrentItem();

    // expand currentItem and collapse all items that are no parents of currentItemms
    tree->expandItem(tree->itemIndex(currentItem));
    tree->repaint(true);
    
    // build a list of parents
    QList<KTreeListItem> parentList;
    parentList.append(currentItem);

    KTreeListItem *pitem = currentItem;
    while (pitem->hasParent())
    {
	pitem = pitem->getParent();
	if (pitem == 0)
	    break;
	parentList.append(pitem);
    }

    // collapse all visible items that are not in parentList
    for (int i = 0; i <= tree->visibleCount(); i++)
    {
	bool isParent = false;

	for (pitem = parentList.first(); pitem != 0; pitem = parentList.next())
	{
	    if (tree->itemAt(i) == pitem)
	    {
		isParent = true;
		break;
	    }
	}
	if (isParent)
	    continue;

	tree->collapseItem(i);
    }
    
    // set currentItem
    tree->setCurrentItem(tree->itemIndex(currentItem));
  
    // now find the highlighted item in our lists
    for (item = staticItems.first(); item != 0; item = staticItems.next())
    {
	if (item == tree->getCurrentItem())
	{
	    if (item->getURL() != "")
		emit itemSelected(item->getURL());
	    return;
	}
    }
    for (item = manualItems.first(); item != 0; item = manualItems.next())
    {
	if (item == tree->getCurrentItem())
	{
	    if (item->getURL() != "")
		emit itemSelected(item->getURL());
	    return;
	}
    }
    for (item = pluginItems.first(); item != 0; item = pluginItems.next())
    {
	if (item == tree->getCurrentItem())
	{
	    if (item->getURL() != "")
		emit itemSelected(item->getURL());
	    return;
	}
    }
}

bool khcNavigator::appendEntries(const char *dirName, khcNavigatorItem *parent, QList<khcNavigatorItem> *appendList)
{
    QDir fileDir(dirName, "*.desktop", 0, QDir::Files | QDir::Hidden | QDir::Readable);

    if (!fileDir.exists())
	return false;

    QStringList fileList = fileDir.entryList();
    QStringList::Iterator itFile;

    for ( itFile = fileList.begin(); !(*itFile).isNull(); ++itFile )
    {
	QString filename = dirName;
	filename += "/";
	filename += *itFile;

	khcNavigatorItem *entry = new khcNavigatorItem;

	if (entry->readKDElnk(filename))
	{
	    entry->insertInTree(tree, parent);
	    appendList->append(entry);
	}
	else
	    delete entry;
    }
  
    return true;
}


bool khcNavigator::processDir( const char *dirName, khcNavigatorItem *parent,  QList<khcNavigatorItem> *appendList)
{
    QString folderName;

    QDir dirDir( dirName, "*", 0, QDir::Dirs );

    if (!dirDir.exists()) return false;
  
    QStringList dirList = dirDir.entryList();
    QStringList::Iterator itDir;
  
    for ( itDir = dirList.begin(); !(*itDir).isNull(); ++itDir )
    {
	if ( (*itDir)[0] == '.' )
	    continue;


	QString filename = dirDir.path();
	filename += "/";
	filename += *itDir;

	if (!containsDocuments(filename))
	    continue;

	QString dirFile = filename;
	dirFile += "/.directory";
	QString icon;
	  
	if ( QFile::exists( dirFile ) )
	{
	    KSimpleConfig sc( dirFile, true );
            sc.setDesktopGroup();
	    folderName = sc.readEntry("Name");

	    icon = //sc.readEntry("MiniIcon");
		//if (icon.isEmpty())
		icon = "helpbook.xpm";
	}
	else
	{
	    folderName = *itDir;
	    icon = "helpbook.xpm";
	}
	  
	khcNavigatorItem *dirItem = new khcNavigatorItem(folderName, icon);
	dirItem->insertInTree(tree, parent);
	appendList->append(dirItem);
	  
	  
	// read and append child items
	appendEntries(filename, dirItem, appendList);	
	processDir(filename, dirItem, appendList);
    }
    return true;
}

bool khcNavigator::containsDocuments(QString dir)
{
    QDir fileDir(dir, "*.desktop", 0, QDir::Files | QDir::Hidden | QDir::Readable);

    if (!fileDir.exists())
	return false;

    // does dir contain files
    if (fileDir.count() > 0)
    {
	// does at least one kdelnk contain a docPath
	QStringList fileList = fileDir.entryList();
	QStringList::Iterator itFile;
	for ( itFile = fileList.begin(); !(*itFile).isNull(); ++itFile )
	{
	    QString filename = dir;
	    filename += "/";
	    filename += *itFile;

	    KSimpleConfig sc( filename, true );
	    sc.setDesktopGroup();
	    QString docpath = sc.readEntry("DocPath");
		  
	    if (!docpath.isEmpty())
		return true;
	}
    }

    // does it contain subdirs
    QDir dirDir( dir, "*", 0, QDir::Dirs );
    if (dirDir.count() < 1)
	return false;

    // go through subdirs and search for files
    QStringList dirList = dirDir.entryList();
    QStringList::Iterator itDir;
  
    for (itDir = dirList.begin(); !(*itDir).isNull(); ++itDir)
    {
	if ( (*itDir).at(0) == '.' )
	    continue;

	if (containsDocuments(dir + "/" + *itDir))
	    return true;
    }
    return false;
}

#include "khc_navigator.moc"
