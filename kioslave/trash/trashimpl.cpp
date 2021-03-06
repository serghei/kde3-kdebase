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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "trashimpl.h"
#include <klocale.h>
#include <klargefile.h>
#include <kio/global.h>
#include <kio/renamedlg.h>
#include <kio/job.h>
#include <kdebug.h>
#include <kurl.h>
#include <kdirnotify_stub.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kmountpoint.h>
#include <kfileitem.h>
#include <kio/chmodjob.h>

#include <dcopref.h>

#include <qapplication.h>
#include <qeventloop.h>
#include <qfile.h>
#include <qdir.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

TrashImpl::TrashImpl()
    : QObject()
    , m_lastErrorCode(0)
    , m_initStatus(InitToBeDone)
    , m_lastId(0)
    , m_homeDevice(0)
    , m_trashDirectoriesScanned(false)
    , m_mibEnum(KGlobal::locale()->fileEncodingMib())
    ,
    // not using kio_trashrc since KIO uses that one already for kio_trash
    // so better have a separate one, for faster parsing by e.g. kmimetype.cpp
    m_config("trashrc")
{
    KDE_struct_stat buff;
    if(KDE_lstat(QFile::encodeName(QDir::homeDirPath()), &buff) == 0)
    {
        m_homeDevice = buff.st_dev;
    }
    else
    {
        kdError() << "Should never happen: couldn't stat $HOME " << strerror(errno) << endl;
    }
}

/**
 * Test if a directory exists, create otherwise
 * @param _name full path of the directory
 * @return errorcode, or 0 if the dir was created or existed already
 * Warning, don't use return value like a bool
 */
int TrashImpl::testDir(const QString &_name) const
{
    DIR *dp = opendir(QFile::encodeName(_name));
    if(dp == NULL)
    {
        QString name = _name;
        if(name.endsWith("/"))
            name.truncate(name.length() - 1);
        QCString path = QFile::encodeName(name);

        bool ok = ::mkdir(path, S_IRWXU) == 0;
        if(!ok && errno == EEXIST)
        {
#if 0 // this would require to use SlaveBase's method to ask the question
        //int ret = KMessageBox::warningYesNo( 0, i18n("%1 is a file, but KDE needs it to be a directory. Move it to %2.orig and create directory?").arg(name).arg(name) );
        //if ( ret == KMessageBox::Yes ) {
#endif
            if(::rename(path, path + ".orig") == 0)
            {
                ok = ::mkdir(path, S_IRWXU) == 0;
            }
            else
            { // foo.orig existed already. How likely is that?
                ok = false;
            }
            if(!ok)
            {
                return KIO::ERR_DIR_ALREADY_EXIST;
            }
#if 0
        //} else {
        //    return 0;
        //}
#endif
        }
        if(!ok)
        {
            // KMessageBox::sorry( 0, i18n( "Couldn't create directory %1. Check for permissions." ).arg( name ) );
            kdWarning() << "could not create " << name << endl;
            return KIO::ERR_COULD_NOT_MKDIR;
        }
        else
        {
            kdDebug() << name << " created." << endl;
        }
    }
    else // exists already
    {
        closedir(dp);
    }
    return 0; // success
}

bool TrashImpl::init()
{
    if(m_initStatus == InitOK)
        return true;
    if(m_initStatus == InitError)
        return false;

    // Check the trash directory and its info and files subdirs
    // see also kdesktop/init.cc for first time initialization
    m_initStatus = InitError;
    // $XDG_DATA_HOME/Trash, i.e. ~/.local/share/Trash by default.
    const QString xdgDataDir = KGlobal::dirs()->localxdgdatadir();
    if(!KStandardDirs::makeDir(xdgDataDir, 0700))
    {
        kdWarning() << "failed to create " << xdgDataDir << endl;
        return false;
    }

    const QString trashDir = xdgDataDir + "Trash";
    int err;
    if((err = testDir(trashDir)))
    {
        error(err, trashDir);
        return false;
    }
    if((err = testDir(trashDir + "/info")))
    {
        error(err, trashDir + "/info");
        return false;
    }
    if((err = testDir(trashDir + "/files")))
    {
        error(err, trashDir + "/files");
        return false;
    }
    m_trashDirectories.insert(0, trashDir);
    m_initStatus = InitOK;
    kdDebug() << k_funcinfo << "initialization OK, home trash dir: " << trashDir << endl;
    return true;
}

void TrashImpl::migrateOldTrash()
{
    kdDebug() << k_funcinfo << endl;
    const QString oldTrashDir = KGlobalSettings::trashPath();
    const QStrList entries = listDir(oldTrashDir);
    bool allOK = true;
    QStrListIterator entryIt(entries);
    for(; entryIt.current(); ++entryIt)
    {
        QString srcPath = QFile::decodeName(*entryIt);
        if(srcPath == "." || srcPath == ".." || srcPath == ".directory")
            continue;
        srcPath.prepend(oldTrashDir); // make absolute
        int trashId;
        QString fileId;
        if(!createInfo(srcPath, trashId, fileId))
        {
            kdWarning() << "Trash migration: failed to create info for " << srcPath << endl;
            allOK = false;
        }
        else
        {
            bool ok = moveToTrash(srcPath, trashId, fileId);
            if(!ok)
            {
                (void)deleteInfo(trashId, fileId);
                kdWarning() << "Trash migration: failed to create info for " << srcPath << endl;
                allOK = false;
            }
            else
            {
                kdDebug() << "Trash migration: moved " << srcPath << endl;
            }
        }
    }
    if(allOK)
    {
        // We need to remove the old one, otherwise the desktop will have two trashcans...
        kdDebug() << "Trash migration: all OK, removing old trash directory" << endl;
        synchronousDel(oldTrashDir, false, true);
    }
}

bool TrashImpl::createInfo(const QString &origPath, int &trashId, QString &fileId)
{
    kdDebug() << k_funcinfo << origPath << endl;
    // Check source
    const QCString origPath_c(QFile::encodeName(origPath));
    KDE_struct_stat buff_src;
    if(KDE_lstat(origPath_c.data(), &buff_src) == -1)
    {
        if(errno == EACCES)
            error(KIO::ERR_ACCESS_DENIED, origPath);
        else
            error(KIO::ERR_DOES_NOT_EXIST, origPath);
        return false;
    }

    // Choose destination trash
    trashId = findTrashDirectory(origPath);
    if(trashId < 0)
    {
        kdWarning() << "OUCH - internal error, TrashImpl::findTrashDirectory returned " << trashId << endl;
        return false; // ### error() needed?
    }
    kdDebug() << k_funcinfo << "trashing to " << trashId << endl;

    // Grab original filename
    KURL url;
    url.setPath(origPath);
    const QString origFileName = url.fileName();

    // Make destination file in info/
    url.setPath(infoPath(trashId, origFileName)); // we first try with origFileName
    KURL baseDirectory;
    baseDirectory.setPath(url.directory());
    // Here we need to use O_EXCL to avoid race conditions with other kioslave processes
    int fd = 0;
    do
    {
        kdDebug() << k_funcinfo << "trying to create " << url.path() << endl;
        fd = ::open(QFile::encodeName(url.path()), O_WRONLY | O_CREAT | O_EXCL, 0600);
        if(fd < 0)
        {
            if(errno == EEXIST)
            {
                url.setFileName(KIO::RenameDlg::suggestName(baseDirectory, url.fileName()));
                // and try again on the next iteration
            }
            else
            {
                error(KIO::ERR_COULD_NOT_WRITE, url.path());
                return false;
            }
        }
    } while(fd < 0);
    const QString infoPath = url.path();
    fileId = url.fileName();
    Q_ASSERT(fileId.endsWith(".trashinfo"));
    fileId.truncate(fileId.length() - 10); // remove .trashinfo from fileId

    FILE *file = ::fdopen(fd, "w");
    if(!file)
    { // can't see how this would happen
        error(KIO::ERR_COULD_NOT_WRITE, infoPath);
        return false;
    }

    // Contents of the info file. We could use KSimpleConfig, but that would
    // mean closing and reopening fd, i.e. opening a race condition...
    QCString info = "[Trash Info]\n";
    info += "Path=";
    // Escape filenames according to the way they are encoded on the filesystem
    // All this to basically get back to the raw 8-bit representation of the filename...
    if(trashId == 0) // home trash: absolute path
        info += KURL::encode_string(origPath, m_mibEnum).latin1();
    else
        info += KURL::encode_string(makeRelativePath(topDirectoryPath(trashId), origPath), m_mibEnum).latin1();
    info += "\n";
    info += "DeletionDate=";
    info += QDateTime::currentDateTime().toString(Qt::ISODate).latin1();
    info += "\n";
    size_t sz = info.size() - 1; // avoid trailing 0 from QCString

    size_t written = ::fwrite(info.data(), 1, sz, file);
    if(written != sz)
    {
        ::fclose(file);
        QFile::remove(infoPath);
        error(KIO::ERR_DISK_FULL, infoPath);
        return false;
    }

    ::fclose(file);

    kdDebug() << k_funcinfo << "info file created in trashId=" << trashId << " : " << fileId << endl;
    return true;
}

QString TrashImpl::makeRelativePath(const QString &topdir, const QString &path)
{
    const QString realPath = KStandardDirs::realFilePath(path);
    // topdir ends with '/'
    if(realPath.startsWith(topdir))
    {
        const QString rel = realPath.mid(topdir.length());
        Q_ASSERT(rel[0] != '/');
        return rel;
    }
    else
    { // shouldn't happen...
        kdWarning() << "Couldn't make relative path for " << realPath << " (" << path << "), with topdir=" << topdir << endl;
        return realPath;
    }
}

QString TrashImpl::infoPath(int trashId, const QString &fileId) const
{
    QString trashPath = trashDirectoryPath(trashId);
    trashPath += "/info/";
    trashPath += fileId;
    trashPath += ".trashinfo";
    return trashPath;
}

QString TrashImpl::filesPath(int trashId, const QString &fileId) const
{
    QString trashPath = trashDirectoryPath(trashId);
    trashPath += "/files/";
    trashPath += fileId;
    return trashPath;
}

bool TrashImpl::deleteInfo(int trashId, const QString &fileId)
{
    bool ok = QFile::remove(infoPath(trashId, fileId));
    if(ok)
        fileRemoved();
    return ok;
}

bool TrashImpl::moveToTrash(const QString &origPath, int trashId, const QString &fileId)
{
    kdDebug() << k_funcinfo << endl;
    const QString dest = filesPath(trashId, fileId);
    if(!move(origPath, dest))
    {
        // Maybe the move failed due to no permissions to delete source.
        // In that case, delete dest to keep things consistent, since KIO doesn't do it.
        if(QFileInfo(dest).isFile())
            QFile::remove(dest);
        else
            synchronousDel(dest, false, true);
        return false;
    }
    fileAdded();
    return true;
}

bool TrashImpl::moveFromTrash(const QString &dest, int trashId, const QString &fileId, const QString &relativePath)
{
    QString src = filesPath(trashId, fileId);
    if(!relativePath.isEmpty())
    {
        src += '/';
        src += relativePath;
    }
    if(!move(src, dest))
        return false;
    return true;
}

bool TrashImpl::move(const QString &src, const QString &dest)
{
    if(directRename(src, dest))
    {
        // This notification is done by KIO::moveAs when using the code below
        // But if we do a direct rename we need to do the notification ourselves
        KDirNotify_stub allDirNotify("*", "KDirNotify*");
        KURL urlDest;
        urlDest.setPath(dest);
        urlDest.setPath(urlDest.directory());
        allDirNotify.FilesAdded(urlDest);
        return true;
    }
    if(m_lastErrorCode != KIO::ERR_UNSUPPORTED_ACTION)
        return false;

    KURL urlSrc, urlDest;
    urlSrc.setPath(src);
    urlDest.setPath(dest);
    kdDebug() << k_funcinfo << urlSrc << " -> " << urlDest << endl;
    KIO::CopyJob *job = KIO::moveAs(urlSrc, urlDest, false);
#ifdef KIO_COPYJOB_HAS_SETINTERACTIVE
    job->setInteractive(false);
#endif
    connect(job, SIGNAL(result(KIO::Job *)), this, SLOT(jobFinished(KIO::Job *)));
    qApp->eventLoop()->enterLoop();

    return m_lastErrorCode == 0;
}

void TrashImpl::jobFinished(KIO::Job *job)
{
    kdDebug() << k_funcinfo << " error=" << job->error() << endl;
    error(job->error(), job->errorText());
    qApp->eventLoop()->exitLoop();
}

bool TrashImpl::copyToTrash(const QString &origPath, int trashId, const QString &fileId)
{
    kdDebug() << k_funcinfo << endl;
    const QString dest = filesPath(trashId, fileId);
    if(!copy(origPath, dest))
        return false;
    fileAdded();
    return true;
}

bool TrashImpl::copyFromTrash(const QString &dest, int trashId, const QString &fileId, const QString &relativePath)
{
    QString src = filesPath(trashId, fileId);
    if(!relativePath.isEmpty())
    {
        src += '/';
        src += relativePath;
    }
    return copy(src, dest);
}

bool TrashImpl::copy(const QString &src, const QString &dest)
{
    // kio_file's copy() method is quite complex (in order to be fast), let's just call it...
    m_lastErrorCode = 0;
    KURL urlSrc;
    urlSrc.setPath(src);
    KURL urlDest;
    urlDest.setPath(dest);
    kdDebug() << k_funcinfo << "copying " << src << " to " << dest << endl;
    KIO::CopyJob *job = KIO::copyAs(urlSrc, urlDest, false);
#ifdef KIO_COPYJOB_HAS_SETINTERACTIVE
    job->setInteractive(false);
#endif
    connect(job, SIGNAL(result(KIO::Job *)), this, SLOT(jobFinished(KIO::Job *)));
    qApp->eventLoop()->enterLoop();

    return m_lastErrorCode == 0;
}

bool TrashImpl::directRename(const QString &src, const QString &dest)
{
    kdDebug() << k_funcinfo << src << " -> " << dest << endl;
    if(::rename(QFile::encodeName(src), QFile::encodeName(dest)) != 0)
    {
        if(errno == EXDEV)
        {
            error(KIO::ERR_UNSUPPORTED_ACTION, QString::fromLatin1("rename"));
        }
        else
        {
            if((errno == EACCES) || (errno == EPERM))
            {
                error(KIO::ERR_ACCESS_DENIED, dest);
            }
            else if(errno == EROFS)
            { // The file is on a read-only filesystem
                error(KIO::ERR_CANNOT_DELETE, src);
            }
            else
            {
                error(KIO::ERR_CANNOT_RENAME, src);
            }
        }
        return false;
    }
    return true;
}

#if 0
bool TrashImpl::mkdir( int trashId, const QString& fileId, int permissions )
{
    const QString path = filesPath( trashId, fileId );
    if ( ::mkdir( QFile::encodeName( path ), permissions ) != 0 ) {
        if ( errno == EACCES ) {
            error( KIO::ERR_ACCESS_DENIED, path );
            return false;
        } else if ( errno == ENOSPC ) {
            error( KIO::ERR_DISK_FULL, path );
            return false;
        } else {
            error( KIO::ERR_COULD_NOT_MKDIR, path );
            return false;
        }
    } else {
        if ( permissions != -1 )
            ::chmod( QFile::encodeName( path ), permissions );
    }
    return true;
}
#endif

bool TrashImpl::del(int trashId, const QString &fileId)
{
    QString info = infoPath(trashId, fileId);
    QString file = filesPath(trashId, fileId);

    QCString info_c = QFile::encodeName(info);

    KDE_struct_stat buff;
    if(KDE_lstat(info_c.data(), &buff) == -1)
    {
        if(errno == EACCES)
            error(KIO::ERR_ACCESS_DENIED, file);
        else
            error(KIO::ERR_DOES_NOT_EXIST, file);
        return false;
    }

    if(!synchronousDel(file, true, QFileInfo(file).isDir()))
        return false;

    QFile::remove(info);
    fileRemoved();
    return true;
}

bool TrashImpl::synchronousDel(const QString &path, bool setLastErrorCode, bool isDir)
{
    const int oldErrorCode = m_lastErrorCode;
    const QString oldErrorMsg = m_lastErrorMessage;
    KURL url;
    url.setPath(path);

    // First ensure that all dirs have u+w permissions,
    // otherwise we won't be able to delete files in them (#130780).
    if(isDir)
    {
        kdDebug() << k_funcinfo << "chmod'ing " << url << endl;
        KFileItem fileItem(url, "inode/directory", KFileItem::Unknown);
        KFileItemList fileItemList;
        fileItemList.append(&fileItem);
        KIO::ChmodJob *chmodJob = KIO::chmod(fileItemList, 0200, 0200, QString::null, QString::null, true /*recursive*/, false /*showProgressInfo*/);
        connect(chmodJob, SIGNAL(result(KIO::Job *)), this, SLOT(jobFinished(KIO::Job *)));
        qApp->eventLoop()->enterLoop();
    }

    kdDebug() << k_funcinfo << "deleting " << url << endl;
    KIO::DeleteJob *job = KIO::del(url, false, false);
    connect(job, SIGNAL(result(KIO::Job *)), this, SLOT(jobFinished(KIO::Job *)));
    qApp->eventLoop()->enterLoop();
    bool ok = m_lastErrorCode == 0;
    if(!setLastErrorCode)
    {
        m_lastErrorCode = oldErrorCode;
        m_lastErrorMessage = oldErrorMsg;
    }
    return ok;
}

bool TrashImpl::emptyTrash()
{
    kdDebug() << k_funcinfo << endl;
    // The naive implementation "delete info and files in every trash directory"
    // breaks when deleted directories contain files owned by other users.
    // We need to ensure that the .trashinfo file is only removed when the
    // corresponding files could indeed be removed.

    const TrashedFileInfoList fileInfoList = list();

    TrashedFileInfoList::const_iterator it = fileInfoList.begin();
    const TrashedFileInfoList::const_iterator end = fileInfoList.end();
    for(; it != end; ++it)
    {
        const TrashedFileInfo &info = *it;
        const QString filesPath = info.physicalPath;
        if(synchronousDel(filesPath, true, true))
        {
            QFile::remove(infoPath(info.trashId, info.fileId));
        } // else error code is set
    }
    fileRemoved();

    return m_lastErrorCode == 0;
}

TrashImpl::TrashedFileInfoList TrashImpl::list()
{
    // Here we scan for trash directories unconditionally. This allows
    // noticing plugged-in [e.g. removeable] devices, or new mounts etc.
    scanTrashDirectories();

    TrashedFileInfoList lst;
    // For each known trash directory...
    TrashDirMap::const_iterator it = m_trashDirectories.begin();
    for(; it != m_trashDirectories.end(); ++it)
    {
        const int trashId = it.key();
        QString infoPath = it.data();
        infoPath += "/info";
        // Code taken from kio_file
        QStrList entryNames = listDir(infoPath);
        // char path_buffer[PATH_MAX];
        // getcwd(path_buffer, PATH_MAX - 1);
        // if ( chdir( infoPathEnc ) )
        //    continue;
        QStrListIterator entryIt(entryNames);
        for(; entryIt.current(); ++entryIt)
        {
            QString fileName = QFile::decodeName(*entryIt);
            if(fileName == "." || fileName == "..")
                continue;
            if(!fileName.endsWith(".trashinfo"))
            {
                kdWarning() << "Invalid info file found in " << infoPath << " : " << fileName << endl;
                continue;
            }
            fileName.truncate(fileName.length() - 10);

            TrashedFileInfo info;
            if(infoForFile(trashId, fileName, info))
                lst << info;
        }
    }
    return lst;
}

// Returns the entries in a given directory - including "." and ".."
QStrList TrashImpl::listDir(const QString &physicalPath)
{
    const QCString physicalPathEnc = QFile::encodeName(physicalPath);
    kdDebug() << k_funcinfo << "listing " << physicalPath << endl;
    QStrList entryNames;
    DIR *dp = opendir(physicalPathEnc);
    if(dp == 0)
        return entryNames;
    KDE_struct_dirent *ep;
    while((ep = KDE_readdir(dp)) != 0L)
        entryNames.append(ep->d_name);
    closedir(dp);
    return entryNames;
}

bool TrashImpl::infoForFile(int trashId, const QString &fileId, TrashedFileInfo &info)
{
    kdDebug() << k_funcinfo << trashId << " " << fileId << endl;
    info.trashId = trashId; // easy :)
    info.fileId = fileId;   // equally easy
    info.physicalPath = filesPath(trashId, fileId);
    return readInfoFile(infoPath(trashId, fileId), info, trashId);
}

bool TrashImpl::readInfoFile(const QString &infoPath, TrashedFileInfo &info, int trashId)
{
    KSimpleConfig cfg(infoPath, true);
    if(!cfg.hasGroup("Trash Info"))
    {
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, infoPath);
        return false;
    }
    cfg.setGroup("Trash Info");
    info.origPath = KURL::decode_string(cfg.readEntry("Path"), m_mibEnum);
    if(info.origPath.isEmpty())
        return false; // path is mandatory...
    if(trashId == 0)
        Q_ASSERT(info.origPath[0] == '/');
    else
    {
        const QString topdir = topDirectoryPath(trashId); // includes trailing slash
        info.origPath.prepend(topdir);
    }
    QString line = cfg.readEntry("DeletionDate");
    if(!line.isEmpty())
    {
        info.deletionDate = QDateTime::fromString(line, Qt::ISODate);
    }
    return true;
}

QString TrashImpl::physicalPath(int trashId, const QString &fileId, const QString &relativePath)
{
    QString filePath = filesPath(trashId, fileId);
    if(!relativePath.isEmpty())
    {
        filePath += "/";
        filePath += relativePath;
    }
    return filePath;
}

void TrashImpl::error(int e, const QString &s)
{
    if(e)
        kdDebug() << k_funcinfo << e << " " << s << endl;
    m_lastErrorCode = e;
    m_lastErrorMessage = s;
}

bool TrashImpl::isEmpty() const
{
    // For each known trash directory...
    if(!m_trashDirectoriesScanned)
        scanTrashDirectories();
    TrashDirMap::const_iterator it = m_trashDirectories.begin();
    for(; it != m_trashDirectories.end(); ++it)
    {
        QString infoPath = it.data();
        infoPath += "/info";

        DIR *dp = opendir(QFile::encodeName(infoPath));
        if(dp)
        {
            struct dirent *ep;
            ep = readdir(dp);
            ep = readdir(dp); // ignore '.' and '..' dirent
            ep = readdir(dp); // look for third file
            closedir(dp);
            if(ep != 0)
            {
                // kdDebug() << ep->d_name << " in " << infoPath << " -> not empty" << endl;
                return false; // not empty
            }
        }
    }
    return true;
}

void TrashImpl::fileAdded()
{
    m_config.setGroup("Status");
    if(m_config.readBoolEntry("Empty", true) == true)
    {
        m_config.writeEntry("Empty", false);
        m_config.sync();
    }
    // The apps showing the trash (e.g. kdesktop) will be notified
    // of this change when KDirNotify::FilesAdded("trash:/") is emitted,
    // which will be done by the job soon after this.
}

void TrashImpl::fileRemoved()
{
    if(isEmpty())
    {
        m_config.setGroup("Status");
        m_config.writeEntry("Empty", true);
        m_config.sync();
    }
    // The apps showing the trash (e.g. kdesktop) will be notified
    // of this change when KDirNotify::FilesRemoved(...) is emitted,
    // which will be done by the job soon after this.
}

int TrashImpl::findTrashDirectory(const QString &origPath)
{
    kdDebug() << k_funcinfo << origPath << endl;
    // First check if same device as $HOME, then we use the home trash right away.
    KDE_struct_stat buff;
    if(KDE_lstat(QFile::encodeName(origPath), &buff) == 0 && buff.st_dev == m_homeDevice)
        return 0;

    QString mountPoint = KIO::findPathMountPoint(origPath);
    const QString trashDir = trashForMountPoint(mountPoint, true);
    kdDebug() << "mountPoint=" << mountPoint << " trashDir=" << trashDir << endl;
    if(trashDir.isEmpty())
        return 0; // no trash available on partition
    int id = idForTrashDirectory(trashDir);
    if(id > -1)
    {
        kdDebug() << " known with id " << id << endl;
        return id;
    }
// new trash dir found, register it
// but we need stability in the trash IDs, so that restoring or asking
// for properties works even kio_trash gets killed because idle.
#if 0
    kdDebug() << k_funcinfo << "found " << trashDir << endl;
    m_trashDirectories.insert( ++m_lastId, trashDir );
    if ( !mountPoint.endsWith( "/" ) )
        mountPoint += '/';
    m_topDirectories.insert( m_lastId, mountPoint );
    return m_lastId;
#endif
    scanTrashDirectories();
    return idForTrashDirectory(trashDir);
}

void TrashImpl::scanTrashDirectories() const
{
    const KMountPoint::List lst = KMountPoint::currentMountPoints();
    for(KMountPoint::List::ConstIterator it = lst.begin(); it != lst.end(); ++it)
    {
        const QCString str = (*it)->mountType().latin1();
        // Skip pseudo-filesystems, there's no chance we'll find a .Trash on them :)
        // ## Maybe we should also skip readonly filesystems
        if(str != "proc" && str != "devfs" && str != "usbdevfs" && str != "sysfs" && str != "devpts" && str != "subfs" /* #96259 */
           && str != "autofs" /* #101116 */)
        {
            QString topdir = (*it)->mountPoint();
            QString trashDir = trashForMountPoint(topdir, false);
            if(!trashDir.isEmpty())
            {
                // OK, trashDir is a valid trash directory. Ensure it's registered.
                int trashId = idForTrashDirectory(trashDir);
                if(trashId == -1)
                {
                    // new trash dir found, register it
                    m_trashDirectories.insert(++m_lastId, trashDir);
                    kdDebug() << k_funcinfo << "found " << trashDir << " gave it id " << m_lastId << endl;
                    if(!topdir.endsWith("/"))
                        topdir += '/';
                    m_topDirectories.insert(m_lastId, topdir);
                }
            }
        }
    }
    m_trashDirectoriesScanned = true;
}

TrashImpl::TrashDirMap TrashImpl::trashDirectories() const
{
    if(!m_trashDirectoriesScanned)
        scanTrashDirectories();
    return m_trashDirectories;
}

TrashImpl::TrashDirMap TrashImpl::topDirectories() const
{
    if(!m_trashDirectoriesScanned)
        scanTrashDirectories();
    return m_topDirectories;
}

QString TrashImpl::trashForMountPoint(const QString &topdir, bool createIfNeeded) const
{
    // (1) Administrator-created $topdir/.Trash directory

    const QString rootTrashDir = topdir + "/.Trash";
    const QCString rootTrashDir_c = QFile::encodeName(rootTrashDir);
    // Can't use QFileInfo here since we need to test for the sticky bit
    uid_t uid = getuid();
    KDE_struct_stat buff;
    const uint requiredBits = S_ISVTX; // Sticky bit required
    if(KDE_lstat(rootTrashDir_c, &buff) == 0)
    {
        if((S_ISDIR(buff.st_mode))     // must be a dir
           && (!S_ISLNK(buff.st_mode)) // not a symlink
           && ((buff.st_mode & requiredBits) == requiredBits) && (::access(rootTrashDir_c, W_OK)))
        {
            const QString trashDir = rootTrashDir + "/" + QString::number(uid);
            const QCString trashDir_c = QFile::encodeName(trashDir);
            if(KDE_lstat(trashDir_c, &buff) == 0)
            {
                if((buff.st_uid == uid)        // must be owned by user
                   && (S_ISDIR(buff.st_mode))  // must be a dir
                   && (!S_ISLNK(buff.st_mode)) // not a symlink
                   && (buff.st_mode & 0777) == 0700)
                { // rwx for user
                    return trashDir;
                }
                kdDebug() << "Directory " << trashDir << " exists but didn't pass the security checks, can't use it" << endl;
            }
            else if(createIfNeeded && initTrashDirectory(trashDir_c))
            {
                return trashDir;
            }
        }
        else
        {
            kdDebug() << "Root trash dir " << rootTrashDir << " exists but didn't pass the security checks, can't use it" << endl;
        }
    }

    // (2) $topdir/.Trash-$uid
    const QString trashDir = topdir + "/.Trash-" + QString::number(uid);
    const QCString trashDir_c = QFile::encodeName(trashDir);
    if(KDE_lstat(trashDir_c, &buff) == 0)
    {
        if((buff.st_uid == uid)        // must be owned by user
           && (S_ISDIR(buff.st_mode))  // must be a dir
           && (!S_ISLNK(buff.st_mode)) // not a symlink
           && ((buff.st_mode & 0777) == 0700))
        { // rwx for user, ------ for group and others

            if(checkTrashSubdirs(trashDir_c))
                return trashDir;
        }
        kdDebug() << "Directory " << trashDir << " exists but didn't pass the security checks, can't use it" << endl;
        // Exists, but not useable
        return QString::null;
    }
    if(createIfNeeded && initTrashDirectory(trashDir_c))
    {
        return trashDir;
    }
    return QString::null;
}

int TrashImpl::idForTrashDirectory(const QString &trashDir) const
{
    // If this is too slow we can always use a reverse map...
    TrashDirMap::ConstIterator it = m_trashDirectories.begin();
    for(; it != m_trashDirectories.end(); ++it)
    {
        if(it.data() == trashDir)
        {
            return it.key();
        }
    }
    return -1;
}

bool TrashImpl::initTrashDirectory(const QCString &trashDir_c) const
{
    if(::mkdir(trashDir_c, 0700) != 0)
        return false;
    // This trash dir will be useable only if the directory is owned by user.
    // In theory this is the case, but not on e.g. USB keys...
    uid_t uid = getuid();
    KDE_struct_stat buff;
    if(KDE_lstat(trashDir_c, &buff) != 0)
        return false;       // huh?
    if((buff.st_uid == uid) // must be owned by user
       && ((buff.st_mode & 0777) == 0700))
    { // rwx for user, --- for group and others

        return checkTrashSubdirs(trashDir_c);
    }
    else
    {
        kdDebug() << trashDir_c << " just created, by it doesn't have the right permissions, must be a FAT partition. Removing it again." << endl;
        // Not good, e.g. USB key. Delete again.
        // I'm paranoid, it would be better to find a solution that allows
        // to trash directly onto the USB key, but I don't see how that would
        // pass the security checks. It would also make the USB key appears as
        // empty when it's in fact full...
        ::rmdir(trashDir_c);
        return false;
    }
    return true;
}

bool TrashImpl::checkTrashSubdirs(const QCString &trashDir_c) const
{
    // testDir currently works with a QString - ## optimize
    QString trashDir = QFile::decodeName(trashDir_c);
    const QString info = trashDir + "/info";
    if(testDir(info) != 0)
        return false;
    const QString files = trashDir + "/files";
    if(testDir(files) != 0)
        return false;
    return true;
}

QString TrashImpl::trashDirectoryPath(int trashId) const
{
    // Never scanned for trash dirs? (This can happen after killing kio_trash
    // and reusing a directory listing from the earlier instance.)
    if(!m_trashDirectoriesScanned)
        scanTrashDirectories();
    Q_ASSERT(m_trashDirectories.contains(trashId));
    return m_trashDirectories[trashId];
}

QString TrashImpl::topDirectoryPath(int trashId) const
{
    if(!m_trashDirectoriesScanned)
        scanTrashDirectories();
    assert(trashId != 0);
    Q_ASSERT(m_topDirectories.contains(trashId));
    return m_topDirectories[trashId];
}

// Helper method. Creates a URL with the format trash:/trashid-fileid or
// trash:/trashid-fileid/relativePath/To/File for a file inside a trashed directory.
KURL TrashImpl::makeURL(int trashId, const QString &fileId, const QString &relativePath)
{
    KURL url;
    url.setProtocol("trash");
    QString path = "/";
    path += QString::number(trashId);
    path += '-';
    path += fileId;
    if(!relativePath.isEmpty())
    {
        path += '/';
        path += relativePath;
    }
    url.setPath(path);
    return url;
}

// Helper method. Parses a trash URL with the URL scheme defined in makeURL.
// The trash:/ URL itself isn't parsed here, must be caught by the caller before hand.
bool TrashImpl::parseURL(const KURL &url, int &trashId, QString &fileId, QString &relativePath)
{
    if(url.protocol() != "trash")
        return false;
    const QString path = url.path();
    int start = 0;
    if(path[0] == '/') // always true I hope
        start = 1;
    int slashPos = path.find('-', 0); // don't match leading slash
    if(slashPos <= 0)
        return false;
    bool ok = false;
    trashId = path.mid(start, slashPos - start).toInt(&ok);
    Q_ASSERT(ok);
    if(!ok)
        return false;
    start = slashPos + 1;
    slashPos = path.find('/', start);
    if(slashPos <= 0)
    {
        fileId = path.mid(start);
        relativePath = QString::null;
        return true;
    }
    fileId = path.mid(start, slashPos - start);
    relativePath = path.mid(slashPos + 1);
    return true;
}

#include "trashimpl.moc"
