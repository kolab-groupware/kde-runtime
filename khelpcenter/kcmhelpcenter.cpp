/*
  This file is part of KHelpcenter.

  Copyright (C) 2002 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include <unistd.h>
#include <sys/types.h>

#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qprogressdialog.h>
#include <qtabwidget.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include "htmlsearchconfig.h"

#include "docmetainfo.h"
#include "scopeitem.h"

#include "kcmhelpcenter.h"
#include "kcmhelpcenter.moc"

extern "C"
{
  KCModule *create_helpcenter( QWidget *parent, const char * )
  {
    KGlobal::locale()->insertCatalogue( "kcmhelpcenter" );
    return new KCMHelpCenter( parent, "kcmhelpcenter" );
  }
}

using namespace KHC;

KCMHelpCenter::KCMHelpCenter(QWidget *parent, const char *name)
  : KCModule(parent, name), mProgressDialog( 0 )
{
//  setButtons(Help);

  QVBoxLayout *tabLayout = new QVBoxLayout( this );

  QTabWidget *tabWidget = new QTabWidget( this );
  tabLayout->addWidget( tabWidget );

  QWidget *scopeTab = createScopeTab( tabWidget );
  tabWidget->addTab( scopeTab, i18n( "Index" ) );

  mHtmlSearchTab = new KHC::HtmlSearchConfig( tabWidget );
  connect( mHtmlSearchTab, SIGNAL( changed( bool ) ),
           SIGNAL( changed( bool ) ) );
  tabWidget->addTab( mHtmlSearchTab, i18n("HTML Search") );

#if 0
  if ( getuid() != 0 ) {
    mBuildButton->setEnabled( false );    
  }
#endif

  mConfig = new KConfig("khelpcenterrc");

  delete DocMetaInfo::self();
  DocMetaInfo::self()->scanMetaInfo( KGlobal::locale()->languagesTwoAlpha() );

  load();
}

KCMHelpCenter::~KCMHelpCenter()
{
  delete mConfig;
}

QWidget *KCMHelpCenter::createScopeTab( QWidget *parent )
{
  QWidget *scopeTab = new QWidget( parent );

  QVBoxLayout *topLayout = new QVBoxLayout( scopeTab );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  mListView = new QListView( scopeTab );
  mListView->addColumn( i18n("Search Scope") );
  mListView->addColumn( i18n("Status") );
  mListView->setColumnAlignment( 1, AlignCenter );
  topLayout->addWidget( mListView );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );

  buttonLayout->addStretch( 1 );
  
  mBuildButton = new QPushButton( i18n("Build Index"), scopeTab );
  buttonLayout->addWidget( mBuildButton );
  connect( mBuildButton, SIGNAL( clicked() ), SLOT( buildIndex() ) );

  return scopeTab;
}

void KCMHelpCenter::defaults()
{
}

void KCMHelpCenter::save()
{
  kdDebug() << "KCMHelpCenter::save()" << endl;

  mHtmlSearchTab->save( mConfig );

  mConfig->sync();
}

void KCMHelpCenter::load()
{
  mHtmlSearchTab->load( mConfig );

  mListView->clear();

  DocEntry::List entries = DocMetaInfo::self()->docEntries();
  DocEntry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
//    kdDebug() << "Entry: " << (*it)->name() << " Indexer: '"
//              << (*it)->indexer() << "'" << endl;
    if ( !(*it)->indexer().isEmpty() ) {
      ScopeItem *item = new ScopeItem( mListView, *it );
      item->setOn( (*it)->searchEnabled() );
    }
  }
  
  updateStatus();
}

void KCMHelpCenter::updateStatus()
{
  QListViewItemIterator it( mListView );
  while ( it.current() != 0 ) {
    ScopeItem *item = static_cast<ScopeItem *>( it.current() );
    QString status;
    if ( item->entry()->indexExists() ) {
      status = i18n("Ok");
    } else {
      status = i18n("Missing");
    }
    item->setText( 1, status );
    
    ++it;
  }
}

void KCMHelpCenter::buildIndex()
{
  kdDebug() << "Build Index" << endl;

  QListViewItemIterator it( mListView );
  while ( it.current() != 0 ) {
    ScopeItem *item = static_cast<ScopeItem *>( it.current() );
    if ( item->isOn() ) {
      mIndexQueue.append( item->entry()->indexer() );
    }
    ++it;
  }

  if ( mIndexQueue.isEmpty() ) return;

  if ( !mProgressDialog ) {
    mProgressDialog = new QProgressDialog( i18n("Build Search Indices"),
                                           i18n("Cancel"), 1, this,
                                           "mProgressDialog", true );
    connect( mProgressDialog, SIGNAL( cancelled() ),
             SLOT( cancelBuildIndex() ) );
  }
  mProgressDialog->setTotalSteps( mIndexQueue.count() );
  mProgressDialog->setProgress( 0 );
  mProgressDialog->show();
  
  processIndexQueue();
}

void KCMHelpCenter::cancelBuildIndex()
{
  mProgressDialog->hide();
  mIndexQueue.clear();
}

void KCMHelpCenter::processIndexQueue()
{
  QStringList::Iterator it = mIndexQueue.begin();

  if ( it == mIndexQueue.end() ) {
    mProgressDialog->hide();
    return;
  }

  KProcess *proc = new KProcess;
  
  QStringList args = QStringList::split( ' ', *it );
  *proc << args;

  kdDebug() << "KCMHelpCenter::processIndexQueue(): " << (*it) << endl;

  connect( proc, SIGNAL( processExited( KProcess * ) ),
           SLOT( slotIndexFinished( KProcess * ) ) ); 

  proc->start();

  mIndexQueue.remove( it );
}

void KCMHelpCenter::slotIndexFinished( KProcess *proc )
{
  delete proc;

  mProgressDialog->setProgress( mProgressDialog->progress() + 1 );

  updateStatus();

  processIndexQueue();
}

QString KCMHelpCenter::quickHelp() const
{
  return i18n("<h1>Help Center</h1> With this control module you configure "
              "and build the index files required by the search function of "
              "the help center.");
}

const KAboutData* KCMHelpCenter::aboutData() const
{
  KAboutData *about =
    new KAboutData( I18N_NOOP("KCMHelpCenter"),
                    I18N_NOOP("Help Center Control Module"),
                    0, 0, KAboutData::License_GPL,
                    I18N_NOOP("(c) 2002 Cornelius Schumacher") );

  about->addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  return about;
}
