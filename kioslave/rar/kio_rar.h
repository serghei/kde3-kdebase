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

#ifndef _kio_rar_H_
#define _kio_rar_H_

#include <kio/global.h>
#include <kio/slavebase.h>

class KShellProcess;
class KProcess;
class KProcIO;
class KURL;
class QCString;
class KRar;

class kio_rarProtocol : public QObject, public KIO::SlaveBase {
    Q_OBJECT
public:
    kio_rarProtocol(const QCString &pool_socket, const QCString &app_socket);
    virtual ~kio_rarProtocol();
    // virtual void mimetype(const KURL& url);
    virtual void get(const KURL &url);
    virtual void listDir(const KURL &url);
    virtual void stat(const KURL &url);
    virtual void del(const KURL &url, bool isFile);
    //    virtual void put( const KURL &url, int permissions, bool overwrite, bool resume );
    //    virtual void rename( const KURL &src, const KURL &dest, bool _overwrite );
private:
    QString rarProgram;
    KIO::filesize_t processed;
    KProcIO *proc;
    KShellProcess *procShell;
    time_t m_archiveTime;
    KRar *m_archiveFile;
    int checkFile(const KURL &url, KURL &fileUrl);
public slots:
    void receivedData(KProcess *, char *buffer, int len);
};

#endif /* _kio_rar_H_ */
