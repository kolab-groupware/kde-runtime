/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)
                 2000 Charles Samuels (charles@kde.org)
                 2000 Stefan Schimanski (1Stein@gmx.de)
                 2000 Matthias Ettrich (ettrich@kde.org)
                 2000 Waldo Bastian <bastian@kde.org>
                 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>
                 2005 Allan Sandfeld Jensen <kde@carewolf.com>
                 2005-2006 by Olivier Goffart <ogoffart at kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "notifybysound.h"
#include "knotifyconfig.h"


// QT headers
#include <QHash>
#include <QSignalMapper>
#include <QFileInfo>

// KDE headers
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>
#include <kurl.h>
#include <config-runtime.h>
#include <kcomponentdata.h>

// Phonon headers
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/audiooutput.h>


class NotifyBySound::Private
{
	public:
		enum { NoSound, UsePhonon, ExternalPlayer } playerMode;
		QString externalPlayer;

		QHash<int, KProcess *> processes;
		QHash<int, Phonon::MediaObject*> mediaobjects;
		QSignalMapper *signalmapper;
		Phonon::AudioOutput *audiooutput;

		int volume;

};


NotifyBySound::NotifyBySound(QObject *parent) : KNotifyPlugin(parent),d(new Private)
{
	d->signalmapper = new QSignalMapper(this);
	connect(d->signalmapper, SIGNAL(mapped(int)), this, SLOT(slotSoundFinished(int)));

	d->audiooutput = new Phonon::AudioOutput( Phonon::NotificationCategory, this );

	loadConfig();
}


NotifyBySound::~NotifyBySound()
{
	delete d;
}


void NotifyBySound::loadConfig()
{
    // load external player settings
	KSharedConfig::Ptr kc = KGlobal::config();
	KConfigGroup cg(kc, "Sounds");

	d->playerMode = Private::UsePhonon;
	if(cg.readEntry( "Use external player", false ))
	{
		d->playerMode = Private::ExternalPlayer;
		d->externalPlayer = cg.readPathEntry("External player");
		// try to locate a suitable player if none is configured
		if ( d->externalPlayer.isEmpty() ) {
			QStringList players;
			players << "wavplay" << "aplay" << "auplay" << "artsplay" << "akodeplay";
			QStringList::Iterator it = players.begin();
			while ( d->externalPlayer.isEmpty() && it != players.end() ) {
				d->externalPlayer = KStandardDirs::findExe( *it );
				++it;
			}
		}
	}
	else if(cg.readEntry( "No sound" , false ))
	{
		d->playerMode = Private::NoSound;
	}
	// load default volume
	setVolume( cg.readEntry( "Volume", 100 ) );
}




void NotifyBySound::notify( int eventId, KNotifyConfig * config )
{
	if(d->mediaobjects.contains(eventId)  || d->processes.contains(eventId) )
	{
		//a sound is already playing for this notification,  we don't support playing two sounds.
		finish( eventId );
		return;
	}

	KUrl soundFileURL = config->readEntry( "Sound" , true );
	QString soundFile = soundFileURL.toLocalFile();

	if (soundFile.isEmpty())
	{
		finish( eventId );
		return;
	}
	
    // get file name
	if ( KUrl::isRelativeUrl(soundFile) )
	{
		QString search = QString("%1/sounds/%2").arg(config->appname).arg(soundFile);
		search = KGlobal::mainComponent().dirs()->findResource("data", search);
		if ( search.isEmpty() )
			soundFile = KStandardDirs::locate( "sound", soundFile );
		else
			soundFile = search;
	}
	if ( soundFile.isEmpty() )
	{
		finish( eventId );
		return;
	}

	kDebug(300) << " going to play " << soundFile;

	if(d->playerMode == Private::UsePhonon)
	{
		Phonon::MediaObject *media = new Phonon::MediaObject( this );
		connect( media, SIGNAL( finished() ), d->signalmapper, SLOT(map()));
		d->signalmapper->setMapping( media , eventId );

                Phonon::createPath(media, d->audiooutput);
		media->setCurrentSource(soundFile);
		media->play();
		d->mediaobjects.insert(eventId , media);
	}
	else if (d->playerMode == Private::ExternalPlayer && !d->externalPlayer.isEmpty())
	{
        // use an external player to play the sound
		KProcess *proc = new KProcess( this );
		connect( proc, SIGNAL(finished(int, QProcess::ExitStatus)),
		         d->signalmapper,  SLOT(map()) );
		d->signalmapper->setMapping( proc , eventId );

		(*proc) << d->externalPlayer << soundFile;
		proc->start();
		
		
		
		return;
	}
}


void NotifyBySound::setVolume( int volume )
{
	if ( volume<0 ) volume=0;
	if ( volume>=100 ) volume=100;
	d->volume = volume;
	d->audiooutput->setVolume(  d->volume / 100.0 );
}


void NotifyBySound::slotSoundFinished(int id)
{
	kDebug(300) << id;
	if(d->mediaobjects.contains(id))
	{
		d->mediaobjects[id]->deleteLater();
		d->mediaobjects.remove(id);
	}
	if(d->processes.contains(id))
	{
		d->processes[id]->deleteLater();
		d->processes.remove(id);
	}
	finish(id);
}


void NotifyBySound::close(int id)
{
	if(d->mediaobjects.contains(id))
	{
		d->mediaobjects[id]->stop();
		d->mediaobjects[id]->deleteLater();
		d->mediaobjects.remove(id);
	}
	if(d->processes.contains(id))
	{
		d->processes[id]->kill();
		d->processes[id]->deleteLater();
		d->processes.remove(id);
	}
}

#include "notifybysound.moc"
