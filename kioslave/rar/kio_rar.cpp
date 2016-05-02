/***************************************************************************
 *   Copyright (C) 2004-2006 by Raul Fernandes                             *
 *   rgfbr@yahoo.com.br                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qcstring.h>
#include <qstringlist.h>
#include <qbitarray.h>
#include <qfile.h>

#include <kdebug.h>
#include <kinstance.h>
#include <klocale.h>
#include <kurl.h>
#include <kprocio.h>
#include <kprocess.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <sys/stat.h>
#include <stdlib.h>
#include "kio_rar.h"
#include "kio_rar.moc"
#include "krar.h"

using namespace KIO;

extern "C" {
int kdemain(int argc, char **argv)
{
    KInstance instance("kio_rar");

    kdDebug(7101) << "*** Starting kio_rar " << endl;

    if(argc != 4)
    {
        kdDebug(7101) << "Usage: kio_rar  protocol domain-socket1 domain-socket2" << endl;
        exit(-1);
    }

    kio_rarProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kdDebug(7101) << "*** kio_rar Done" << endl;
    return 0;
}
}


kio_rarProtocol::kio_rarProtocol(const QCString &pool_socket, const QCString &app_socket) : SlaveBase("kio_rar", pool_socket, app_socket)
{
    kdDebug(7101) << "kio_rarProtocol::kio_rarProtocol()" << endl;
    rarProgram = KGlobal::dirs()->findExe("rar");
    if(rarProgram.isNull()) // Check if rar binary is available
    {
        rarProgram = KGlobal::dirs()->findExe("unrar");
        if(rarProgram.isNull())
        {
            error(KIO::ERR_SLAVE_DEFINED,
                  i18n("Neither rar nor unrar was not found in PATH. You should install one of them to work with this kioslave"));
        }
    }
    m_archiveTime = 0L;
    m_archiveFile = 0L;
}


kio_rarProtocol::~kio_rarProtocol()
{
    kdDebug(7101) << "kio_rarProtocol::~kio_rarProtocol()" << endl;
}


void kio_rarProtocol::get(const KURL &url)
{
    kdDebug(7101) << "kio_rar::get(const KURL& url)" << endl;

    KURL fileUrl;
    int errorNum;

    errorNum = checkFile(url, fileUrl);
    if(errorNum == 1)
    {
        // cannot open archive
        return;
    }

    if(errorNum == 2)
    {
        // it is not a RAR archive
        // send the path to konqueror to redirect to file: kioslave
        redirection(url.path());
        finished();
        return;
    }

    if(!m_archiveFile->directory()->entry(fileUrl.path()))
    {
        error(KIO::ERR_DOES_NOT_EXIST, fileUrl.path());
        return;
    }

    procShell = new KShellProcess();
    processed = 0;
    connect(procShell, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(receivedData(KProcess *, char *, int)));

    procShell->setEnvironment("LC_ALL", KGlobal::locale()->language());
    //-ierr -idp
    *procShell << rarProgram << "p -inul -c- -y"
               << "\"" + m_archiveFile->fileName() + "\""
               << "\"" + fileUrl.path().remove(0, 1) + "\"";

    infoMessage(i18n("Unpacking file..."));

    procShell->start(KProcess::Block, KProcess::Stdout);

    if(procShell->normalExit())
    {
        // WARNING Non fatal error(s) occurred
        if(procShell->exitStatus() == 1)
            warning(i18n("Non fatal error(s) occurred"));
        // FATAL ERROR A fatal error occurred
        if(procShell->exitStatus() == 2)
            error(KIO::ERR_SLAVE_DIED, url.url());
        // CRC ERROR A CRC error occurred when unpacking
        if(procShell->exitStatus() == 3)
            error(KIO::ERR_SLAVE_DEFINED, i18n("CRC failed for file %1").arg(url.url()));
        // WRITE ERROR Write to disk error
        // for this error we should use ERR_COULD_NOT_WRITE or ERR_DISK_FULL ?
        if(procShell->exitStatus() == 5)
            error(KIO::ERR_DISK_FULL, url.url());
        // OPEN ERROR Open file error
        if(procShell->exitStatus() == 6)
            error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.url());
        // MEMORY ERROR Not enough memory for operation
        if(procShell->exitStatus() == 8)
            error(KIO::ERR_OUT_OF_MEMORY, url.url());
    }

    data(QByteArray());
    finished();

    delete procShell;
    procShell = 0;
}


/*!
    \fn kio_rarProtocol::listDir( const KURL & url )
 */
void kio_rarProtocol::listDir(const KURL &url)
{
    kdDebug(7101) << "kio_rar::listDir(const KURL& url)" << endl;
    KURL fileUrl;
    int errorNum;

    errorNum = checkFile(url, fileUrl);
    if(errorNum == 1)
    {
        // cannot open archive
        return;
    }

    if(errorNum == 2)
    {
        // it is not a RAR archive
        // send the path to konqueror to redirect to file: kioslave
        redirection(url.path());
        finished();
        return;
    }

    infoMessage(i18n("Listing directory..."));

    const KArchiveDirectory *directory;
    if(fileUrl.path().isEmpty())
        directory = m_archiveFile->directory();
    else
        directory = ((const KArchiveDirectory *)(m_archiveFile->directory()->entry(fileUrl.path())));
    QStringList list = directory->entries();

    UDSEntry entry;
    UDSAtom atom;
    const KArchiveEntry *archiveEntry;

    if(list.count() != 0)
        for(QStringList::Iterator it = list.begin(); it != list.end(); ++it)
        {
            archiveEntry = directory->entry(*it);
            entry.clear();

            atom.m_uds = KIO::UDS_NAME;
            atom.m_str = archiveEntry->name();
            entry.append(atom);

            atom.m_uds = KIO::UDS_FILE_TYPE;
            if(archiveEntry->isDirectory())
                atom.m_long = S_IFDIR;
            else
                atom.m_long = S_IFREG;
            entry.append(atom);

            atom.m_uds = KIO::UDS_ACCESS;
            atom.m_long = archiveEntry->permissions();
            entry.append(atom);

            atom.m_uds = UDS_SIZE;
            atom.m_long = archiveEntry->isDirectory() ? 0L : ((KArchiveFile *)archiveEntry)->size();
            entry.append(atom);

            atom.m_uds = KIO::UDS_MODIFICATION_TIME;
            atom.m_long = archiveEntry->datetime().toTime_t();
            entry.append(atom);

            listEntry(entry, false);
        }

    listEntry(entry, true);
    finished();
}


/*!
    \fn kio_rarProtocol::stat( const KURL & url )
 */
void kio_rarProtocol::stat(const KURL &url)
{
    kdDebug(7101) << "kio_rar::stat(const KURL& url)" << endl;
    KURL fileUrl;
    int errorNum;

    errorNum = checkFile(url, fileUrl);
    if(errorNum == 1)
    {
        // cannot open archive
        return;
    }

    if(errorNum == 2)
    {
        // it is not a RAR archive
        // send the path to konqueror to redirect to file: kioslave
        redirection(url.path());
        finished();
        return;
    }

    UDSEntry entry;
    UDSAtom atom;

    // check if it is root directory
    if(fileUrl.path(-1) == "/")
    {
        atom.m_uds = KIO::UDS_NAME;
        atom.m_str = "/";
        entry.append(atom);
        atom.m_uds = KIO::UDS_FILE_TYPE;
        atom.m_long = S_IFDIR;
        entry.append(atom);
        statEntry(entry);
        finished();
        return;
    }

    const KArchiveEntry *archiveEntry = m_archiveFile->directory()->entry(fileUrl.path());

    if(!archiveEntry)
    {
        error(KIO::ERR_DOES_NOT_EXIST, fileUrl.path());
        return;
    }

    entry.clear();

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = archiveEntry->name();
    entry.append(atom);

    atom.m_uds = KIO::UDS_FILE_TYPE;
    if(archiveEntry->isDirectory())
        atom.m_long = S_IFDIR;
    else
        atom.m_long = S_IFREG;
    entry.append(atom);

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = archiveEntry->permissions();
    entry.append(atom);

    atom.m_uds = UDS_SIZE;
    atom.m_long = archiveEntry->isDirectory() ? 0L : ((KArchiveFile *)archiveEntry)->size();
    entry.append(atom);

    atom.m_uds = KIO::UDS_MODIFICATION_TIME;
    atom.m_long = archiveEntry->datetime().toTime_t();
    entry.append(atom);

    statEntry(entry);
    finished();
}


/*!
    \fn kio_rarProtocol::checkFile( const KURL &url, KURL &fileUrl )
 */
int kio_rarProtocol::checkFile(const KURL &url, KURL &fileUrl)
{
    kdDebug(7101) << "kio_rar::checkFile(const KURL& url, KURL &fileUrl)" << endl;
    QString archiveUrl;
    struct stat statbuf;

    if(m_archiveFile && url.path().startsWith(m_archiveFile->fileName()))
    {
        fileUrl = url.path().section(m_archiveFile->fileName(), 1);
        // Has it changed ?
        if(::stat(QFile::encodeName(m_archiveFile->fileName()), &statbuf) == 0)
        {
            if(m_archiveTime == statbuf.st_mtime)
                return 0;
        }
        archiveUrl = m_archiveFile->fileName();
    }
    else
    {
        if(url.path().find(".rar", false) == -1)
            return 2;
        archiveUrl = url.path().section(".rar", 0, 0) + ".rar";
        if(url.path().endsWith(".rar"))
            fileUrl = "/";
        else
            fileUrl = url.path().section(".rar", 1);
    }

    if(m_archiveFile)
    {
        m_archiveFile->close();
        delete m_archiveFile;
    }
    m_archiveFile = new KRar(archiveUrl);
    if(!m_archiveFile->open(IO_ReadOnly))
    {
        error(ERR_CANNOT_OPEN_FOR_READING, archiveUrl);
        return 1;
    }
    ::stat(QFile::encodeName(m_archiveFile->fileName()), &statbuf);
    m_archiveTime = statbuf.st_mtime;
    return 0;
}


/*!
    \fn kio_rarProtocol::receivedData( KProcess *, char* buffer, int len )
 */
void kio_rarProtocol::receivedData(KProcess *, char *buffer, int len)
{
    QByteArray d(len);
    d.setRawData(buffer, len);
    data(d);
    d.resetRawData(buffer, len);
    processed += len;
    processedSize(processed);
}


/*!
    \fn kio_rarProtocol::del( const KURL &url, bool isFile )
 */
void kio_rarProtocol::del(const KURL &url, bool /*isFile*/)
{
    kdDebug(7101) << "kio_rar::del(const KURL &url, bool isFile)" << endl;
    KURL fileUrl;
    int errorNum;

    errorNum = checkFile(url, fileUrl);
    if(errorNum == 1)
    {
        // cannot open archive
        return;
    }

    if(errorNum == 2)
    {
        // it is not a RAR archive
        // send the path to konqueror to redirect to file: kioslave
        redirection(url.path());
        finished();
        return;
    }

    if(!rarProgram.endsWith("/rar"))
    {
        error(ERR_UNSUPPORTED_ACTION, i18n("You need to have the rar binary in $PATH to use this action"));
        return;
    }

    infoMessage(i18n("Deleting file..."));

    procShell = new KShellProcess();
    procShell->setEnvironment("LC_ALL", KGlobal::locale()->language());
    *procShell << rarProgram << "d"
               << "\"" + m_archiveFile->fileName() + "\""
               << "\"" + fileUrl.path(-1).remove(0, 1) + "\"";
    procShell->start(KProcess::Block, KProcess::AllOutput);
    if(!procShell->normalExit())
        error(KIO::ERR_CANNOT_LAUNCH_PROCESS, url.path());

    finished();
}


/*!
    \fn kio_rarProtocol::put( const KURL &url, int permissions, bool overwrite, bool resume )
 */
// void kio_rarProtocol::put( const KURL &/*url*/, int /*permissions*/, bool /*overwrite*/, bool /*resume*/ )
/*{
    kdDebug(7101) << "kio_rar::put(const KURL &url, int permissions, bool overwrite, bool resume)" << endl ;
    KURL fileUrl;
    if( !checkName( url, fileUrl ) )
    {
      // it is not a rar archive
      // return the error that cannot enter the directory
      error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
      return;
    }
    if( !rarProgram.endsWith( "/rar" ) )
    {
      error( ERR_UNSUPPORTED_ACTION, i18n( "You need to have the rar binary in $PATH to use this action" ) );
      return;
    }

    //TODO: check if the archive exits, if not, create a new rar archive
    // QFile.exits();

    infoMessage( i18n( "Adding file..." ) );

    // receive the data
    QByteArray buffer;
    int length;
    QFile file( "/tmp/" + fileUrl.fileName() );
    file.open( IO_WriteOnly );
    do
    {
      dataReq();
      length = readData( buffer );
      file.writeBlock( buffer );
    }while( length > 0 );
    file.close();

    procShell = new KShellProcess();
    procShell->setEnvironment( "LC_ALL", KGlobal::locale()->language() );
    //New -st[name] switch to compress data from stdin (standard input).
    *procShell << rarProgram << "a -inul -ep -ap" + fileUrl.directory().remove( 0, 1 ) << "\"" + m_archiveFile.fileName() + "\"" << "\"" + file.name()
+ "\"";
    procShell->start( KProcess::Block, KProcess::NoCommunication );
    //if( !proc->normalExit() ) error( KIO::ERR_CANNOT_LAUNCH_PROCESS, url.path() );

    //delete the temporary file
    file.remove();

    delete procShell;
    procShell = 0;

    finished();
}*/


/*!
    \fn kio_rarProtocol::rename( const KURL &src, const KURL &dest, bool overwrite )
 */
// void kio_rarProtocol::rename( const KURL &/*src*/, const KURL &/*dest*/, bool /*overwrite*/ )
/*{
    kdDebug(7101) << "kio_rar::rename(const KURL &src, const KURL &dest, bool overwrite)" << endl ;
    KURL fileUrl;
    if( !checkFile( src, fileUrl ) )
    {
      // it is not a rar archive
      // return the error that cannot enter the directory
      error( ERR_CANNOT_ENTER_DIRECTORY, src.path() );
      return;
    }

    KURL destArchive;
    KURL destUrl;
    checkName( dest, destArchive, destUrl );
    if( archiveUrl != destArchive )
    {
      // we can not rename files in diferent archives
      error( ERR_CANNOT_ENTER_DIRECTORY, src.path() );
      return;
    }
    if( !rarProgram.endsWith( "/rar" ) )
    {
      error( ERR_UNSUPPORTED_ACTION, i18n( "You need to have the rar binary in $PATH to use this action" ) );
      return;
    }

    infoMessage( i18n( "Renaming file..." ) );

    procShell = new KShellProcess();
    procShell->setEnvironment( "LC_ALL", KGlobal::locale()->language() );
    *procShell << rarProgram << "rn" << "\"" + m_archiveFile.fileName() + "\"" << "\"" + fileUrl.path( -1 ).remove( 0, 1 ) + "\"" << "\"" +
destUrl.path( -1 ).remove( 0, 1 ) + "\"";
    procShell->start( KProcess::Block );
    //if( !proc->normalExit() ) error( KIO::ERR_CANNOT_LAUNCH_PROCESS, url.path() );

    finished();
}*/
