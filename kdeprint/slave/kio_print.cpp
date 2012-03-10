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

#include "kio_print.h"
#include <kdeprint/kmprinter.h>
#include <kdeprint/kmmanager.h>
#include <kdeprint/kmjobmanager.h>
#include <kdeprint/kmjob.h>
#include <kdeprint/driver.h>

#include <qfile.h>
#include <qtextstream.h>
#include <klocale.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kio/global.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kio/job.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <qfile.h>
#include <qdom.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define	PRINT_DEBUG	kdDebug(7019) << "kio_print: "

extern "C"
{
	int KDE_EXPORT kdemain(int argc, char **argv);
}

void addAtom(KIO::UDSEntry& entry, unsigned int ID, long l, const QString& s = QString::null)
{
	KIO::UDSAtom	atom;
	atom.m_uds = ID;
	atom.m_long = l;
	atom.m_str = s;
	entry.append(atom);
}

static void createDirEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime)
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, name);
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_ACCESS, 0500);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
	addAtom(entry, KIO::UDS_URL, 0, url);
	PRINT_DEBUG << "creating dir entry url=" << url << " mimetype=" << mime << endl;
	addAtom(entry, KIO::UDS_SIZE, 0);
	//addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "application/octet-stream");
}

static void createFileEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime)
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, name);
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFREG);
	addAtom(entry, KIO::UDS_URL, 0, url);
	addAtom(entry, KIO::UDS_ACCESS, 0400);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
	addAtom(entry, KIO::UDS_SIZE, 0);
	addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "application/octet-stream");
}

QString buildMenu(const QStringList& items, const QStringList& links, int active)
{
	if (items.count() == 0 || items.count() != links.count())
		return QString("<td height=20 class=\"menu\">&nbsp;</td>");

	QString	s;
	int	index = 0;
	for (QStringList::ConstIterator it1=items.begin(), it2=links.begin(); it1!=items.end() && it2!=links.end(); ++it1, ++it2, index++)
	{
		if (index == active)
			s.append("<td height=20 class=\"menuactive\">&nbsp; ").append(*it1).append("&nbsp;</td>");
		else
			s.append("<td height=20 class=\"menu\">&nbsp; <a class=\"menu\" href=\"").append(*it2).append("\">").append(*it1).append("</a>&nbsp;</td>");
		if ((unsigned int)index < items.count()-1)
			s.append("<td height=20 class=\"menu\">|</td>");
	}
	return s;
}

QString buildOptionRow(DrBase *opt, bool f)
{
	QString	s("<tr class=\"%1\"><td width=\"41%\">%1</td><td width=\"59%\">%1</td></tr>\n");
	s = s.arg(f ? "contentwhite" : "contentyellow").arg(opt->get("text")).arg(opt->prettyText());
	return s;
}

QString buildGroupTable(DrGroup *grp, bool showHeader = true)
{
	QString	s("<tr class=\"top\"><td colspan=\"2\">%1</td></tr>\n");
	if (showHeader)
		s = s.arg(grp->get("text"));
	else
		s = QString::null;

	QPtrListIterator<DrBase>	oit(grp->options());
	bool	f(false);
	for (; oit.current(); ++oit, f = !f)
		s.append(buildOptionRow(oit.current(), f));

	QPtrListIterator<DrGroup>	git(grp->groups());
	for (; git.current(); ++git)
		s.append(buildGroupTable(git.current()));

	return s;
}

int kdemain(int argc, char **argv)
{
	KInstance  instance("kio_print");

	PRINT_DEBUG << "starting ioslave" << endl;
	if (argc != 4)
	{
		fprintf(stderr, "Usage: kio_print protocol domain-socket1 domain-socket2\n");
		exit(-1);
	}

	/* create fake KApplicatiom object, needed for job stuffs */
	KAboutData about( "kio_print", "kio_print", "fake_version", 
			"KDEPrint IO slave", KAboutData::License_GPL, "(c) 2003, Michael Goffioul" );
	KCmdLineArgs::init( &about );
	KApplication app;

	KIO_Print	 slave(argv[2], argv[3]);
	slave.dispatchLoop();

	PRINT_DEBUG << "done" << endl;
	return 0;
}

KIO_Print::KIO_Print(const QCString& pool, const QCString& app)
: KIO::SlaveBase("print", pool, app)
{
}

void KIO_Print::listDir(const KURL& url)
{
	if ( url.protocol() == "printdb" )
	{
		listDirDB( url );
		return;
	}

	QStringList	path = QStringList::split('/', url.path(), false);

	PRINT_DEBUG << "listing " << url.path() << endl;
	QString	group = path[0].lower();
	if (path.count() == 0)
		listRoot();
	else if (path.count() == 1 && group != "manager" && group != "jobs")
	{
		PRINT_DEBUG << "listing group " << path[0] << endl;

		int	mask;
		QString	mimeType;
		KIO::UDSEntry	entry;

		if (group == "printers")
		{
			mask = KMPrinter::Printer;
			mimeType = "print/printer";
		}
		else if (group == "classes")
		{
			mask = KMPrinter::Class | KMPrinter::Implicit;
			mimeType = "print/class";
		}
		else if (group == "specials")
		{
			mask = KMPrinter::Special;
			mimeType = "print/printer";
		}
		else
		{
			error(KIO::ERR_DOES_NOT_EXIST, url.url());
			return;
		}

		QPtrListIterator<KMPrinter>	it(*(KMManager::self()->printerList()));
		for (;it.current();++it)
		{
			if (!(it.current()->type() & mask) || !it.current()->instanceName().isEmpty())
			{
				PRINT_DEBUG << "rejecting " << it.current()->name() << endl;
				continue;
			}

			//createFileEntry(entry, it.current()->name(), ("print:/"+path[0]+"/"+it.current()->name()), mimeType, "text/html", S_IFDIR);
			createDirEntry(entry, it.current()->name(), ("print:/"+group+"/"+KURL::encode_string_no_slash(it.current()->name())), mimeType);
			PRINT_DEBUG << "accepting " << it.current()->name() << endl;
			listEntry(entry, false);
		}

		listEntry(KIO::UDSEntry(), true);
		finished();
	}
	else
	{
		//error(KIO::ERR_UNSUPPORTED_ACTION, i18n("Unsupported path %1").arg(url.path()));
		// better do nothing
		listEntry(KIO::UDSEntry(), true);
		totalSize(0);
		finished();
	}
}

void KIO_Print::listRoot()
{
	PRINT_DEBUG << "listing root entry" << endl;

	KIO::UDSEntry	entry;

	// Classes entry
	createDirEntry(entry, i18n("Classes"), "print:/classes", "print/folder");
	listEntry(entry, false);

	// Printers entry
	createDirEntry(entry, i18n("Printers"), "print:/printers", "print/folder");
	listEntry(entry, false);

	// Specials entry
	createDirEntry(entry, i18n("Specials"), "print:/specials", "print/folder");
	listEntry(entry, false);

	// Management entry
	//createFileEntry(entry, i18n("Manager"), "print:/manager", "print/manager", QString::null, S_IFDIR);
	createDirEntry(entry, i18n("Manager"), "print:/manager", "print/manager");
	listEntry(entry, false);

	// Jobs entry
	createDirEntry(entry, i18n("Jobs"), "print:/jobs", "print/jobs");
	listEntry(entry, false);

	// finish
	totalSize(4);
	listEntry(entry, true);
	finished();
}

void KIO_Print::listDirDB( const KURL& url )
{
	PRINT_DEBUG << "listDirDB: " << url << endl;

	QStringList pathComps = QStringList::split( '/', url.path(), false );
	KURL remUrl;

	remUrl.setProtocol( "http" );
	remUrl.setHost( url.host() );
	remUrl.setPort( url.port() );
	remUrl.setPath( "/list-data.cgi" );
	switch ( pathComps.size() )
	{
		case 0: /* list manufacturers */
			remUrl.addQueryItem( "type", "makes" );
			break;
		case 1: /* list printers for the given manufacturer */
			remUrl.addQueryItem( "type", "printers" );
			remUrl.addQueryItem( "make", pathComps[ 0 ] );
			break;
		case 2: /* list drivers for given printer */
			remUrl.addQueryItem( "type", "drivers" );
			remUrl.addQueryItem( "printer", pathComps[ 1 ] );
			break;
		default:
			error( KIO::ERR_UNSUPPORTED_ACTION, "Not implemented" );
			return;
	}
	remUrl.addQueryItem( "format", "xml" );

	if ( getDBFile( remUrl ) )
	{
		QDomDocument doc;
		if ( doc.setContent( &m_httpBuffer, false ) )
		{
			QDomNodeList l;
			KIO::UDSEntry entry;
			switch ( pathComps.size() )
			{
				case 0:
					l = doc.documentElement().elementsByTagName( "make" );
					for ( unsigned int i=0; i<l.count(); i++ )
					{
						QString make = l.item( i ).toElement().text();
						KURL makeUrl = url;
						makeUrl.addPath( "/" + make );
						createDirEntry( entry, make, makeUrl.url(), "print/folder" );
						listEntry( entry, false );
						PRINT_DEBUG << "make: " << make << endl;
					}
					break;
				case 1:
					l = doc.documentElement().elementsByTagName( "printer" );
					for ( unsigned int i=0; i<l.count(); i++ )
					{
						QString ID, name;
						for ( QDomNode n=l.item( i ).firstChild(); !n.isNull(); n=n.nextSibling() )
						{
							QDomElement e = n.toElement();
							if ( e.tagName() == "id" )
								ID = e.text();
							else if ( e.tagName() == "model" )
								name = e.text();
						}
						if ( !ID.isEmpty() && !name.isEmpty() )
						{
							KURL printerUrl = url;
							printerUrl.addPath( "/" + ID );
							createDirEntry( entry, name, printerUrl.url(), "print/printermodel" );
							listEntry( entry, false );
							PRINT_DEBUG << "printer: " << ID << endl;
						}
					}
					break;
				case 2:
					l = doc.documentElement().elementsByTagName( "driver" );
					for ( unsigned int i=0; i<l.count(); i++ )
					{
						QString driver = l.item( i ).toElement().text();
						KURL driverUrl = url;
						driverUrl.addPath( "/" + driver );
						createFileEntry( entry, driver, driverUrl.url(), "print/driver" );
						listEntry( entry, false );
						PRINT_DEBUG << "driver: " << driver << endl;
					}
					break;
				default:
					error( KIO::ERR_UNSUPPORTED_ACTION, "Not implemented" );
					return;
			}
			listEntry( KIO::UDSEntry(), true );
			finished();
		}
		else
		{
			if ( m_httpBuffer.buffer().size() == 0 )
				error( KIO::ERR_INTERNAL, i18n( "Empty data received (%1)." ).arg( url.host() ) );
			else
				error( KIO::ERR_INTERNAL, i18n( "Corrupted/incomplete data or server error (%1)." ).arg( url.host() ) );
		}
	}
	/*
	 * If error occured while downloading, error has been called by
	 * getDBFile. No need for a "else" statement.
	 */
}

void KIO_Print::stat(const KURL& url)
{
	if ( url.protocol() == "printdb" )
	{
		statDB( url );
		return;
	}

	PRINT_DEBUG << "stat: " << url.url() << endl;
	QStringList	path = QStringList::split('/', url.encodedPathAndQuery(-1), false);
	KIO::UDSEntry	entry;
	QString	mime;
	bool err(false);

	PRINT_DEBUG << "path components: " << path.join(", ") << endl;

	switch (path.count())
	{
		case 0:
			createDirEntry(entry, i18n("Print System"), "print:/", "print/folder");
			break;
		case 1:
			if (path[0].lower() == "classes")
				createDirEntry(entry, i18n("Classes"), "print:/classes", "print/folder");
			else if (path[0].lower() == "printers")
				createDirEntry(entry, i18n("Printers"), "print:/printers", "print/folder");
			else if (path[0].lower() == "specials")
				createDirEntry(entry, i18n("Specials"), "print:/specials", "print/folder");
			else if (path[0].lower() == "manager")
				createDirEntry(entry, i18n("Manager"), "print:/manager", "print/manager");
			else if (path[0].lower().startsWith("jobs"))
				createFileEntry(entry, i18n("Jobs"), url.url(), "text/html");
			else
				err = true;
			break;
		case 2:
			if (path[0].lower() == "printers")
				mime = "print/printer";
			else if (path[0].lower() == "classes")
				mime = "print/class";
			else if (path[0].lower() == "specials")
				mime = "print/printer";
			else
				err = true;
			createFileEntry(entry, path[1], "print:/"+path[0]+"/"+path[1], "text/html");
			break;
	}

	if (!err)
	{
		statEntry(entry);
		finished();
	}
	else
		error(KIO::ERR_DOES_NOT_EXIST, url.path());
}

void KIO_Print::statDB( const KURL& url )
{
	PRINT_DEBUG << "statDB: " << url << endl;
	KIO::UDSEntry entry;
	QStringList pathComps = QStringList::split( '/', url.path(), false );
	if ( pathComps.size() == 3 )
		createFileEntry( entry, i18n( "Printer driver" ), url.url(), "print/driver" );
	else
		createDirEntry( entry, i18n( "On-line printer driver database" ), url.url(), "inode/directory" );
	statEntry( entry );
	finished();
}

bool KIO_Print::getDBFile( const KURL& src )
{
	PRINT_DEBUG << "downloading " << src.url() << endl;

	/* re-initialize the internal buffer */
	if ( m_httpBuffer.isOpen() )
		m_httpBuffer.close();
	m_httpError = 0;
	m_httpBuffer.open( IO_WriteOnly|IO_Truncate ); // be sure to erase the existing data

	/* start the transfer job */
	KIO::TransferJob *job = KIO::get( src, false, false );
	connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( slotResult( KIO::Job* ) ) );
	connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ), SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
	connect( job, SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ), SLOT( slotTotalSize( KIO::Job*, KIO::filesize_t ) ) );
	connect( job, SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ), SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
	kapp->enter_loop();
	m_httpBuffer.close();

	/* return the result */
	if ( m_httpError != 0 )
		error( m_httpError, m_httpErrorTxt );
	return ( m_httpError == 0 );
}

void KIO_Print::getDB( const KURL& url )
{
	PRINT_DEBUG << "downloading PPD file for " << url.url() << endl;

	QStringList pathComps = QStringList::split( '/', url.path(), false );
	if ( pathComps.size() != 3 )
		error( KIO::ERR_MALFORMED_URL, url.url() );
	else
	{
		KURL remUrl;

		remUrl.setProtocol( "http" );
		remUrl.setHost( url.host() );
		remUrl.setPath( "/ppd-o-matic.cgi" );
		remUrl.addQueryItem( "driver", pathComps[ 2 ] );
		remUrl.addQueryItem( "printer", pathComps[ 1 ] );

		if ( getDBFile( remUrl ) )
		{
			mimeType( "text/plain" );
			data( m_httpBuffer.buffer() );
			finished();
		}
		/*
		 * no "else" statement needed, the error has
		 * already been emitted by the getDBFile function
		 */
	}
}

void KIO_Print::slotResult( KIO::Job *j )
{
	/*
	 * store slave results for later user (job gets deleted 
	 * after this function). Store only if no other error
	 * occured previously (when writing to the buffer).
	 */
	if ( m_httpError == 0 )
	{
		m_httpError = j->error();
		m_httpErrorTxt = j->errorText();
	}
	kapp->exit_loop();
}

void KIO_Print::slotData( KIO::Job *j, const QByteArray& d )
{
	PRINT_DEBUG << "HTTP data received (size=" << d.size() << ")" << endl;
	if ( d.size() > 0 )
	{
		int len = m_httpBuffer.writeBlock( d );
		if ( len == -1 || len != ( int )d.size() )
		{
			m_httpError = KIO::ERR_INTERNAL;
			m_httpErrorTxt = "Unable to write to the internal buffer.";
			j->kill( false );
		}
	}
}

void KIO_Print::slotTotalSize( KIO::Job*, KIO::filesize_t sz )
{
	totalSize( sz );
}

void KIO_Print::slotProcessedSize( KIO::Job*, KIO::filesize_t sz )
{
	processedSize( sz );
}

void KIO_Print::get(const KURL& url)
{
	if ( url.protocol() == "printdb" )
	{
		getDB( url );
		return;
	}

	QStringList	elems = QStringList::split('/', url.encodedPathAndQuery(), false);
	QString		group(elems[0].lower()), printer(KURL::decode_string(elems[1])), path, query;
	KMPrinter	*mprinter(0);

	if (group == "manager")
	{
		PRINT_DEBUG << "opening print management part" << endl;

		mimeType("print/manager");
		finished();
		return;
	}

	PRINT_DEBUG << "getting " << url.url() << endl;

	if (group.startsWith("jobs"))
	{
		int	p = group.find('?');
		if (p != -1)
			query = group.mid(p+1);
		if (!query.isEmpty() && query != "jobs" && query != "completed_jobs")
		{
			error(KIO::ERR_MALFORMED_URL, QString::null);
			return;
		}
		PRINT_DEBUG << "listing jobs for all printers" << endl;
		showJobs(0, query == "completed_jobs");
		return;
	}

	int	p = printer.find('?');
	if (p != -1)
	{
		query = printer.mid(p+1);
		printer = printer.left(p);
	}

	PRINT_DEBUG << "opening " << url.url() << endl;
	PRINT_DEBUG << "extracted printer name = " << printer << endl;

	KMManager::self()->printerList(false);
	mprinter = KMManager::self()->findPrinter(printer);
	if (!mprinter)
		path = locateData(printer.isEmpty() ? group : printer);

	if (elems.count() > 2 || (path.isEmpty() && group != "printers" && group != "classes" && group != "specials")
	    || (mprinter == 0 && path.isEmpty()))
	{
		error(KIO::ERR_DOES_NOT_EXIST, url.path());
		return;
	}

	if (mprinter != 0)
	{
		if (!query.isEmpty() && query != "general")
		{
			if (query == "jobs")
				showJobs(mprinter, false);
			else if (query == "completed_jobs")
				showJobs(mprinter, true);
			else if (query == "driver")
				showDriver(mprinter);
			else
				error(KIO::ERR_MALFORMED_URL, KURL::decode_string(elems[1]));
		}
		else if (group == "printers" && mprinter->isPrinter())
			showPrinterInfo(mprinter);
		else if (group == "classes" && mprinter->isClass(true))
			showClassInfo(mprinter);
		else if (group == "specials" && mprinter->isSpecial())
			showSpecialInfo(mprinter);
		else
			error(KIO::ERR_INTERNAL, i18n("Unable to determine object type for %1.").arg(printer));
	}
	else if (!path.isEmpty())
		showData(path);
	else
		error(KIO::ERR_INTERNAL, i18n("Unable to determine source type for %1.").arg(printer));
}

void KIO_Print::showPrinterInfo(KMPrinter *printer)
{
	if (!KMManager::self()->completePrinter(printer))
		error(KIO::ERR_INTERNAL, i18n("Unable to retrieve printer information for %1.").arg(printer->name()));
	else
	{
		mimeType("text/html");

		QString	content;
		if (!loadTemplate(QString::fromLatin1("printer.template"), content))
		{
			error(KIO::ERR_INTERNAL, i18n("Unable to load template %1").arg("printer.template"));
			return;
		}

		content = content
				 .arg(i18n("Properties of %1").arg(printer->printerName()))
				 .arg(i18n("Properties of %1").arg(printer->printerName()))
				 .arg(buildMenu(QStringList::split('|', i18n("General|Driver|Active jobs|Completed jobs"), false),
							 QStringList::split('|', "?general|?driver|?jobs|?completed_jobs", true),
							 0))
				 .arg(QString::null)
				 .arg(printer->pixmap())
				 .arg(printer->name())
				 .arg(i18n("General Properties"))
				 .arg(i18n("Type")).arg(printer->isRemote() ? i18n("Remote") : i18n("Local"))
				 .arg(i18n("State")).arg(printer->stateString())
				 .arg(i18n("Location")).arg(printer->location())
				 .arg(i18n("Description")).arg(printer->description())
				 .arg(i18n("URI")).arg(printer->uri().prettyURL())
				 .arg(i18n("Interface (Backend)")).arg(printer->device())
				 .arg(i18n("Driver"))
				 .arg(i18n("Manufacturer")).arg(printer->manufacturer())
				 .arg(i18n("Model")).arg(printer->model())
				 .arg(i18n("Driver Information")).arg(printer->driverInfo());

		data(content.local8Bit());
		finished();
	}
}

void KIO_Print::showClassInfo(KMPrinter *printer)
{
	if (!KMManager::self()->completePrinter(printer))
		error(KIO::ERR_INTERNAL, i18n("Unable to retrieve class information for %1.").arg(printer->name()));
	else
	{
		mimeType("text/html");

		QString	content;
		if (!loadTemplate(QString::fromLatin1("class.template"), content))
		{
			error(KIO::ERR_INTERNAL, i18n("Unable to load template %1").arg("class.template"));
			return;
		}

		QString		memberContent("<ul>\n");
		QStringList	members(printer->members());
		for (QStringList::ConstIterator it=members.begin(); it!=members.end(); ++it)
		{
			memberContent.append(QString::fromLatin1("<li><a href=\"print:/printers/%1\">%2</a></li>\n").arg(*it).arg(*it));
		}
		memberContent.append("</ul>\n");

		QString		typeContent = (printer->isImplicit() ? i18n("Implicit") : (printer->isRemote() ? i18n("Remote") : i18n("Local")));

		content = content
				 .arg(i18n("Properties of %1").arg(printer->printerName()))
				 .arg(i18n("Properties of %1").arg(printer->printerName()))
				 .arg(buildMenu(QStringList::split('|', i18n("General|Active jobs|Completed jobs"), false),
							 QStringList::split('|', "?general|?jobs|?completed_jobs", true),
							 0))
				 .arg(QString::null)
				 .arg(printer->pixmap())
				 .arg(printer->name())
				 .arg(i18n("General Properties"))
				 .arg(i18n("Type")).arg(typeContent)
				 .arg(i18n("State")).arg(printer->stateString())
				 .arg(i18n("Location")).arg(printer->location())
				 .arg(i18n("Description")).arg(printer->description())
				 .arg(i18n("URI")).arg(printer->uri().prettyURL())
				 .arg(i18n("Members")).arg(memberContent);

		data(content.local8Bit());
		finished();
	}
}

void KIO_Print::showSpecialInfo(KMPrinter *printer)
{
	mimeType("text/html");

	QString	content;
	if (!loadTemplate(QString::fromLatin1("pseudo.template"), content))
	{
		error(KIO::ERR_INTERNAL, i18n("Unable to load template %1").arg("pseudo.template"));
		return;
	}

	QString	reqContent("<ul>\n");
	QStringList	requirements = QStringList::split(",", printer->option("kde-special-require"), false);
	for (QStringList::ConstIterator it=requirements.begin(); it!=requirements.end(); ++it)
		reqContent += ("<li>" + (*it) + "</li>\n");
	reqContent.append("</ul>\n");

	content = content
			 .arg(i18n("Properties of %1").arg(printer->printerName()))
			 .arg(i18n("Properties of %1").arg(printer->printerName()))
			 .arg(buildMenu(QStringList::split('|', i18n("General"), false),
						 QStringList::split('|', "?general", true),
						 0))
			 .arg(QString::null)
			 .arg(printer->pixmap())
			 .arg(printer->name())
			 .arg(i18n("General Properties"))
			 .arg(i18n("Location")).arg(printer->location())
			 .arg(i18n("Description")).arg(printer->description())
			 .arg(i18n("Requirements")).arg(reqContent)
			 .arg(i18n("Command Properties"))
			 .arg(i18n("Command")).arg("<tt>"+printer->option("kde-special-command")+"</tt>")
			 .arg(i18n("Use Output File")).arg(printer->option("kde-special-file") == "1" ? i18n("Yes") : i18n("No"))
			 .arg(i18n("Default Extension")).arg(printer->option("kde-special-extension"));

	data(content.local8Bit());
	finished();
}

bool KIO_Print::loadTemplate(const QString& filename, QString& buffer)
{
	QFile	f(locate("data", QString::fromLatin1("kdeprint/template/")+filename));
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		buffer = t.read();
		return true;
	}
	else
	{
		buffer = QString::null;
		return false;
	}
}

void KIO_Print::showData(const QString& pathname)
{
	PRINT_DEBUG << "sending data: " << pathname << endl;
	QFile	f(pathname);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QByteArray	arr(f.readAll());
		mimeType(KMimeType::findByURL(KURL(pathname), 0, true, true)->name());
		data(arr);
		finished();
	}
	else
	{
		PRINT_DEBUG << "file not found" << endl;
		error(KIO::ERR_DOES_NOT_EXIST, pathname);
	}
}

/**
 * Locate a data in this order:
 *	- $KDEDIR/share/apps/kdeprint/template/
 *	- as a desktop icon
 */
QString KIO_Print::locateData(const QString& item)
{
	QString	path = locate("data", "kdeprint/template/"+item);
	if (path.isEmpty())
		path = KGlobal::iconLoader()->iconPath(item, KIcon::Desktop, true);
	return path;
}

void KIO_Print::showJobs(KMPrinter *prt, bool completed)
{
	mimeType("text/html");

	// Add the printer to the current list in the job manager
	KMJobManager::JobType	type = (completed ? KMJobManager::CompletedJobs : KMJobManager::ActiveJobs);
	KMJobManager	*mgr = KMJobManager::self();
	if (prt)
		mgr->addPrinter(prt->printerName(), type);
	else
	{
		QPtrListIterator<KMPrinter>	pit(*(KMManager::self()->printerList()));
		for (; pit.current(); ++pit)
			if (pit.current()->isVirtual() || pit.current()->isSpecial())
				continue;
			else
				mgr->addPrinter(pit.current()->printerName(), type);
	}

	QString	content;
	if (!loadTemplate(QString::fromLatin1("jobs.template"), content))
	{
		error(KIO::ERR_INTERNAL, i18n("Unable to load template %1").arg("pseudo.template"));
		return;
	}

	if (prt)
	{
		content = content
				 .arg(i18n("Jobs of %1").arg(prt->printerName()))
				 .arg(i18n("Jobs of %1").arg(prt->printerName()))
				 .arg(prt->isClass () ?
						 buildMenu(QStringList::split('|', i18n("General|Active jobs|Completed jobs"), false),
							 QStringList::split('|', "?general|?jobs|?completed_jobs", true),
							 (completed ? 2 : 1)) :
						 buildMenu(QStringList::split('|', i18n("General|Driver|Active jobs|Completed jobs"), false),
							 QStringList::split('|', "?general|?driver|?jobs|?completed_jobs", true),
							 (completed ? 3 : 2)))
				 .arg(QString::null)
				 .arg(prt->pixmap())
				 .arg(prt->printerName());
	}
	else
	{
		content = content
				 .arg(i18n("All jobs"))
				 .arg(buildMenu(QStringList::split('|', i18n("Active jobs|Completed jobs"), false),
							 QStringList::split('|', "?jobs|?completed_jobs", true),
							 (completed ? 1 : 0)))
				 .arg("fileprint")
				 .arg(i18n("All jobs"));
	}
	content = content.arg(i18n("ID")).arg(i18n("Owner")).arg(i18n("Printer")).arg(i18n("Name")).arg(i18n("State"));

	QString	jobContent, cellContent("<td>%1</td>\n");
	QPtrListIterator<KMJob>	it(mgr->jobList());
	bool	flag(true);
	for (; it.current(); ++it, flag = !flag)
	{
		jobContent.append("<tr class=\"").append(flag ? "contentyellow" : "contentwhite").append("\">\n");
		jobContent.append(cellContent.arg(it.current()->id()));
		jobContent.append(cellContent.arg(it.current()->owner()));
		jobContent.append(cellContent.arg(it.current()->printer()));
		jobContent.append(cellContent.arg(it.current()->name()));
		jobContent.append(cellContent.arg(it.current()->stateString()));
		jobContent.append("</tr>\n");
	}
	content = content.arg(jobContent);

	// remove the current printer to the current list in the job manager
	if (prt)
		mgr->removePrinter(prt->printerName(), type);
	else
	{
		QPtrListIterator<KMPrinter>	pit(*(KMManager::self()->printerList()));
		for (; pit.current(); ++pit)
			if (pit.current()->isVirtual() || pit.current()->isSpecial())
				continue;
			else
				mgr->removePrinter(pit.current()->printerName(), type);
	}

	data(content.local8Bit());
	finished();
}

void KIO_Print::showDriver(KMPrinter *prt)
{
	mimeType("text/html");

	QString	content;
	if (!loadTemplate(QString::fromLatin1("driver.template"), content))
	{
		error(KIO::ERR_INTERNAL, i18n("Unable to load template %1").arg("pseudo.template"));
		return;
	}

	DrMain	*driver = KMManager::self()->loadPrinterDriver(prt, true);
	content = content
			 .arg(i18n("Driver of %1").arg(prt->printerName()))
			 .arg(i18n("Driver of %1").arg(prt->printerName()))
			 .arg(buildMenu(QStringList::split('|', i18n("General|Driver|Active jobs|Completed jobs"), false),
						 QStringList::split('|', "?general|?driver|?jobs|?completed_jobs", true),
						 1))
			 .arg(QString::null)
			 .arg(prt->pixmap())
			 .arg(prt->printerName() + "&nbsp;(" + (driver ? driver->get("text") : i18n("No driver found")) + ")");

	if (driver)
		content = content.arg(buildGroupTable(driver, false));
	else
		content = content.arg(QString::null);

	data(content.local8Bit());
	finished();
}

#include "kio_print.moc"
