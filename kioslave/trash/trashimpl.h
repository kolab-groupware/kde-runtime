/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef TRASHIMPL_H
#define TRASHIMPL_H

#include <kio/jobclasses.h>

#include <qstring.h>
#include <qdatetime.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qstrlist.h>

/**
 * Implementation of all low-level operations done by kio_trash
 * The structure of the trash directory follows the freedesktop.org standard <TODO URL>
 */
class TrashImpl : public QObject
{
    Q_OBJECT
public:
    TrashImpl();

    /// Check .Trash directory in $HOME
    /// This MUST be called before doing anything else
    bool init();

    /// Create info for a file to be trashed
    /// Returns trashId and fileId
    /// The caller is then responsible for actually trashing the file
    bool createInfo( const QString& origPath, int& trashId, QString& fileId );

    /// Delete info file for a file to be trashed
    /// Usually used for undoing what createInfo did if trashing failed
    bool deleteInfo( int trashId, const QString& fileId );

    /// Try moving a file as a trashed file. The ids come from createInfo.
    bool tryRename( const QString& origPath, int trashId, const QString& fileId );

    /// Create a top-level trashed directory
    //bool mkdir( int trashId, const QString& fileId, int permissions );

    /// Get rid of a trashed file
    bool del( int trashId, const QString& fileId );

    /// Restore a trashed file
    bool restore( int trashId, const QString& fileId );

    /// Empty trash, i.e. delete all trashed files
    bool emptyTrash();

    struct TrashedFileInfo {
        int trashId; // for the url
        QString fileId; // for the url
        QString physicalPath; // for stat'ing etc.
        QString origPath; // from info file
        QDateTime deletionDate; // from info file
    };
    /// List trashed files
    typedef QValueList<TrashedFileInfo> TrashedFileInfoList;
    TrashedFileInfoList list();

    /// Return the info for a given trashed file
    bool infoForFile( int trashId, const QString& fileId, TrashedFileInfo& info );

    /// KIO error code
    int lastErrorCode() const { return m_lastErrorCode; }
    QString lastErrorMessage() const { return m_lastErrorMessage; }

    QStrList listDir( const QString& physicalPath );
    QString infoPath( int trashId, const QString& fileId ) const;
    QString filesPath( int trashId, const QString& fileId ) const;

private:
    /// Helper method. Tries to call ::rename(src,dest) and does error handling.
    bool directRename( const QString& src, const QString& dest );

    bool testDir( const QString& name );
    void error( int e, const QString& s );

    bool readInfoFile( const QString& infoPath, TrashedFileInfo& info );

    /// Find the trash dir to use for a given file to delete, based on original path
    int findTrashDirectory( const QString& origPath );
    /// Check .Trash directory in another partition
    bool initTrashDirectory( const QString& origPath );
    QString trashDirectoryPath( int trashId ) const {
        return m_trashDirectories[trashId];
    }

private slots:
    void delJobFinished(KIO::Job *job);
    void moveJobFinished(KIO::Job *job);

private:
    /// Last error code stored in class to simplify API.
    /// Note that this means almost no method can be const.
    int m_lastErrorCode;
    QString m_lastErrorMessage;

    enum { InitToBeDone, InitOK, InitError } m_initStatus;

    // A "trash directory" is a physical directory on disk,
    // e.g. $HOME/.Trash/$uid or /mnt/foo/.Trash/$uid
    // It has an id (number) and a path.
    // The home trash has id 0.
    typedef QMap<int, QString> TrashDirMap;
    TrashDirMap m_trashDirectories; // id -> path
    int m_lastId;

    // We don't cache any data related to the trashed files.
    // Another kioslave could change that behind our feet.
    // If we want to start caching data - and avoiding some race conditions -,
    // we should turn this class into a kded module and use DCOP to talk to it
    // from the kioslave.
};

#endif
