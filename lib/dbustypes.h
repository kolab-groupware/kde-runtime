/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DBUSTYPES_H
#define DBUSTYPES_H

#include <QtCore/QMetaType>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtDBus/QDBusVariant>

#include "simpleresourcegraph.h"

typedef QHash<QString, QDBusVariant> NepomukDBusVariantHash;
Q_DECLARE_METATYPE(NepomukDBusVariantHash)

namespace Nepomuk {
    namespace DBus {
        QString convertUri(const QUrl& uri);
        QStringList convertUriList(const QList<QUrl>& uris);
        QHash<QString, QDBusVariant> convertMetadataHash(const QHash<QUrl, QVariant>& metadata);
        QList<QDBusVariant> convertVariantList(const QVariantList& vl);
    }
}

#endif // DBUSTYPES_H
