/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


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

#ifndef NOTIFYBYPOPUP_H
#define NOTIFYBYPOPUP_H

#include "knotifyplugin.h"
#include <QMap>

class KPassivePopup;

class NotifyByPopup : public KNotifyPlugin
{ Q_OBJECT
	public:
		NotifyByPopup(QObject *parent=0l);
		virtual ~NotifyByPopup();
		
		virtual QString optionName() { return "Popup"; }
		virtual void notify(int id , KNotifyConfig *config);
		virtual void close( int id );
		virtual void update(int id, KNotifyConfig *config);
	private:
		QMap<int, KPassivePopup * > m_popups;
		// the y coordinate of the next position popup should appears
		int m_nextPosition;
		void fillPopup(KPassivePopup *,int id,KNotifyConfig *config);
		
	private Q_SLOTS:
		void slotPopupDestroyed();
		void slotLinkClicked(const QString & );

};

#endif
