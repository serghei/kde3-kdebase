/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KIO_PRINT_H
#define KIO_PRINT_H

#include <kio/slavebase.h>
#include <kio/global.h>
#include <qstring.h>
#include <qbuffer.h>

class KMPrinter;
namespace KIO {
	class Job;
}

class KIO_Print : public QObject, public KIO::SlaveBase
{
	Q_OBJECT
public:
	KIO_Print(const QCString& pool, const QCString& app);

	void listDir(const KURL& url);
	void get(const KURL& url);
	void stat(const KURL& url);

protected slots:
	void slotResult( KIO::Job* );
	void slotData( KIO::Job*, const QByteArray& );
	void slotTotalSize( KIO::Job*, KIO::filesize_t );
	void slotProcessedSize( KIO::Job*, KIO::filesize_t );

private:
	void listRoot();
	void listDirDB( const KURL& );
	void statDB( const KURL& );
	bool getDBFile( const KURL& );
	void getDB( const KURL& );
	void showClassInfo(KMPrinter*);
	void showPrinterInfo(KMPrinter*);
	void showSpecialInfo(KMPrinter*);
	void showData(const QString&);
	QString locateData(const QString&);
	void showJobs(KMPrinter *p = 0, bool completed = false);
	void showDriver(KMPrinter*);

	bool loadTemplate(const QString& filename, QString& buffer);

	QBuffer m_httpBuffer;
	int     m_httpError;
	QString m_httpErrorTxt;
};

#endif
