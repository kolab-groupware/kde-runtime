/*
  main.cpp - A KControl Application

  written 1998 by Matthias Hoelzer
  written 1999-2000 by Hans Petter Bieker <bieker@kde.org>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */

#include <stdio.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qobjectlist.h>
#include <qlayout.h>

#include <kconfig.h>
#include <kcmodule.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcharsets.h>

#include "locale.h"
#include "localenum.h"
#include "localemon.h"
#include "localetime.h"
#include "klocalesample.h"
#include "main.h"
#include "main.moc"

KLocale *locale;

#ifndef LAT
#define LAT QString::fromLatin1("1")
#endif

KLocaleApplication::KLocaleApplication(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  locale = new KLocale(QString::fromLatin1("kcmlocale"));

  QVBoxLayout *l = new QVBoxLayout(this, 5);

  tab = new QTabWidget( this, "section" );
  l->addWidget(tab);

  localemain = new KLocaleConfig( this, I18N_NOOP("&Locale") );
  tab->addTab( localemain, LAT);
  localenum = new KLocaleConfigNumber( this, I18N_NOOP("&Numbers") );
  tab->addTab( localenum, QString::fromLatin1("1") );
  localemon = new KLocaleConfigMoney( this, I18N_NOOP("&Money") );
  tab->addTab( localemon, QString::fromLatin1("1") ); 
  localetime = new KLocaleConfigTime( this, I18N_NOOP("&Time && dates") );
  tab->addTab( localetime, QString::fromLatin1("1") ); 

  connect(localemain, SIGNAL(resample()),       SLOT(update()));
  connect(localenum,  SIGNAL(resample()),       SLOT(update()));
  connect(localemon,  SIGNAL(resample()),       SLOT(update()));
  connect(localetime, SIGNAL(resample()),       SLOT(update()));
  connect(localemain, SIGNAL(countryChanged()), SLOT(reset()) );
  connect(localemain, SIGNAL(chsetChanged()),   SLOT(newChset()) );

  // Examples
  gbox = new QGroupBox(LAT, this, I18N_NOOP("Examples"));
  l->addWidget(gbox);
  sample = new KLocaleSample(gbox);

  update();
}

KLocaleApplication::~KLocaleApplication()
{
    delete locale;
}

void KLocaleApplication::load()
{
    localemain->load();
    localenum->load();
    localemon->load();
    localetime->load();
}

void KLocaleApplication::save()
{
    localemain->save();
    localenum->save();
    localemon->save();
    localetime->save();
}

void KLocaleApplication::defaults()
{
    localemain->defaults();
    localenum->defaults();
    localemon->defaults();
    localetime->defaults();
}

void KLocaleApplication::moduleChanged(bool state)
{
  emit changed(state);
}

void KLocaleApplication::updateSample()
{
    sample->update();
    //CT is this the right place?
    emit moduleChanged(true);
}

void KLocaleApplication::reTranslate()
{
    QObjectList it;
    it.append( this );
    reTranslate(it);

    localemain->reTranslate();
    localenum->reTranslate();
    localemon->reTranslate();
    localetime->reTranslate();
}

void KLocaleApplication::reTranslate(QObjectListIt it)
{
    QObject *wc;
    while( (wc = it.current()) != 0 ) {
      ++it;
      if (wc->children())
        reTranslate(QObjectListIt(*wc->children()));

      if ( !qstrcmp(wc->name(), "unnamed") )
         continue;
      if ( !wc->isWidgetType() )
         continue;

      if ( !qstrcmp(wc->className(), "QGroupBox"))
      {
        ((QGroupBox *)wc)->setTitle(locale->translate(wc->name()));
        ((QGroupBox *)wc)->setMinimumSize(((QGroupBox *)wc)->sizeHint());
      }
      else if ( !qstrcmp(wc->className(), "QLabel"))
      {
        ((QLabel *)wc)->setText(locale->translate(wc->name()));
        ((QLabel *)wc)->setMinimumSize(((QLabel *)wc)->sizeHint());
      }
      else if ( !qstrcmp(wc->className(), "QTabWidget"))
      {
          ((QTabWidget *)wc)->changeTab(localemain, locale->translate(localemain->name()));
          ((QTabWidget *)wc)->changeTab(localenum, locale->translate(localenum->name()));
          ((QTabWidget *)wc)->changeTab(localemon, locale->translate(localemon->name()));
          ((QTabWidget *)wc)->changeTab(localetime, locale->translate(localetime->name()));
      }
    }
}

void KLocaleApplication::reset()
{
  localenum->reset();
  localemon->reset();
  localetime->reset();
}

void KLocaleApplication::newChset()
{
  QFont *font = new QFont(QString::fromLatin1("helvetica"), 12, QFont::Normal);
  KGlobal::charsets()->setQFont(*font, locale->charset());
  KConfig *c = KGlobal::config();
  c->setGroup( QString::fromLatin1("General") );
  setFont(c->readFontEntry(QString::fromLatin1("font"), font));
}

extern "C" {
  KCModule *create_locale(QWidget *parent, const char* name) {
    return new KLocaleApplication(parent, name);
  }
}
