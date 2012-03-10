/*
 *   kdeprintfax - a small fax utility
 *   Copyright (C) 2001  Michael Goffioul
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "faxctrl.h"
#include "kdeprintfax.h"
#include "defcmds.h"

#include <ktextedit.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <qregexp.h>
#include <kprinter.h>
#include <qsimplerichtext.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qvaluestack.h>
#include <qstylesheet.h>

#include <kprocess.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kwin.h>
#include <kemailsettings.h>
#include <kdebug.h>
#include <kstdguiitem.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <stdlib.h>
#include <stdarg.h>

#define quote(x) KProcess::quote(x)

/** \brief Return a string for the page size.
 *
 * @param size the page size, which is actually of type @c QPrinter::PageSize
 * @return a pointer to a static string for the name of the page size.
 */
char const* pageSizeText(int size)
{
	switch(size)
	{
	case QPrinter::A4:		return "a4";
	case QPrinter::Legal:	return "legal";
	case QPrinter::Letter:	return "letter";
	default:				return "letter";
	}
}

/** \brief Return the default page size.
 */
static QString pageSize()
{
	KConfig	*conf = KGlobal::config();
	conf->setGroup("Fax");
	return conf->readEntry("Page", pageSizeText(KGlobal::locale()->pageSize()));
}

static QString stripNumber( const QString& s )
{
	KConfig *conf = KGlobal::config();
	conf->setGroup( "Personal" );

	// removes any non-numeric character, except ('+','*','#') (hope it's supported by faxing tools)
	QString strip_s = s;
	strip_s.replace( QRegExp( "[^\\d+*#]" ), "" );
	if ( strip_s.find( '+' ) != -1 && conf->readBoolEntry( "ReplaceIntChar", false ) )
		strip_s.replace( "+", conf->readEntry( "ReplaceIntCharVal" ) );
	return strip_s;
}

static QString tagList( int n, ... )
{
	QString t;

	va_list ap;
	va_start( ap, n );
	for ( int i=0; i<n; i++ )
	{
		QString tag = va_arg( ap, const char* );
		tag.append( "(_(\\w|\\{[^\\}]*\\}))?" );
		if ( t.isEmpty() )
			t = tag;
		else
			t.append( "|" ).append( tag );
	}

	return t;
}

/** Process a @c %variable that has a boolean value.
 * The format is <tt>%name_{iftrue??iffalse}</tt>. Either
 * @em iftrue or @em iffalse can be empty (or both, but that would
 * be silly). For example, if the fax software uses the
 * option @c -l for low resolution, you might use <tt>%res_{??-l}</tt>,
 * and if it uses high resolution, use <tt>%res_{-h??}</tt>. As an
 * abbreviation, omit the trailing <tt>?</tt>, e.g., <tt>%res_{-h}</tt>.
 *
 * By the way, when using multiple, adjacent question marks,
 * always be careful about inadvertently using trigraphs.
 * Always escape the second and subsequent question marks, to be safe.
 * I suppose question marks are not the best string to use,
 * but I think they work well for the end-user.
 *
 * @param match the string to process
 * @param value the Boolean value to control the processing
 * @return the new value of the tag
 */
static QString processTag( const QString& match, bool value)
{
	QString v;
	int p = match.find( '_' );
	if ( p != -1 && match[ p+1 ] == '{' )
	{
		// Find the ?? that separates the iftrue from the iffalse parts.
		int q = match.find( "?\?", p+2 );
		if ( q == -1 )
		{
			// No iffalse part
			if (value)
				v = match.mid( p+2 );
			// else value is false, so leave v empty.
		}
		else if ( value )
			// Extract only the iftrue part
			v = match.mid( p+2, q-p-2 );
		else
			// Extract only the iffalse part
			v = match.mid( q+2, match.length()-q-3 );
	}
	// Else the string is malformed: no _ or no { after the _
	// In either case, there isn't much the program can do,
	// so just leave the result string empty.

	return v;
}

static QString processTag( const QString& match, const QString& value )
{
	QString v;
	int p = match.find( '_' );
	if ( p != -1 )
	{
		if ( value.isEmpty() )
			v = "";
		else
		{
			if ( match[ p+1 ] == '{' )
			{
				v = match.mid( p+2, match.length()-p-3 );
				v.replace( "@@", quote( value ) );
			}
			else
				v = ( "-" + match.mid( p+1 ) + " " + quote( value ) );
		}
	}
	else
		v = quote( value );
	return v;
}

static bool isTag( const QString& m, const QString& t )
{
	return ( m == t || m.startsWith( t+"_" ) );
}

static QString replaceTags( const QString& s, const QString& tags, KdeprintFax *fax = NULL, const KdeprintFax::FaxItem& item = KdeprintFax::FaxItem() )
{
	// unquote variables (they will be replaced with quoted values later)

	QValueStack<bool> stack;
	KConfig	*conf = KGlobal::config();

	QString cmd = s;

	bool issinglequote=false;
	bool isdoublequote=false;
	QRegExp re_noquote("(\\$\\(|\\)|\\(|\"|'|\\\\|`|"+tags+")");
	QRegExp re_singlequote("('|"+tags+")");
	QRegExp re_doublequote("(\\$\\(|\"|\\\\|`|"+tags+")");
	for	( int i = re_noquote.search(cmd);
		i != -1;
		i = (issinglequote?re_singlequote.search(cmd,i)
			:isdoublequote?re_doublequote.search(cmd,i)
			:re_noquote.search(cmd,i))
	)
	{
		if (cmd[i]=='(') // (...)
		{
			// assert(isdoublequote == false)
			stack.push(isdoublequote);
			i++;
		}
		else if (cmd[i]=='$') // $(...)
		{
			stack.push(isdoublequote);
			isdoublequote = false;
			i+=2;
		}
		else if (cmd[i]==')') // $(...) or (...)
		{
			if (!stack.isEmpty())
				isdoublequote = stack.pop();
			else
				qWarning("Parse error.");
			i++;
		}
		else if (cmd[i]=='\'')
		{
			issinglequote=!issinglequote;
			i++;
		}
		else if (cmd[i]=='"')
		{
			isdoublequote=!isdoublequote;
			i++;
		}
		else if (cmd[i]=='\\')
			i+=2;
		else if (cmd[i]=='`')
		{
			// Replace all `...` with safer $(...)
			cmd.replace (i, 1, "$(");
			QRegExp re_backticks("(`|\\\\`|\\\\\\\\|\\\\\\$)");
			for (	int i2=re_backticks.search(cmd,i+2);
				i2!=-1;
				i2=re_backticks.search(cmd,i2)
			)
			{
				if (cmd[i2] == '`')
				{
					cmd.replace (i2, 1, ")");
					i2=cmd.length(); // leave loop
				}
				else
				{	// remove backslash and ignore following character
					cmd.remove (i2, 1);
					i2++;
				}
			}
			// Leave i unchanged! We need to process "$("
		}
		else
		{
			QString match, v;

			// get match
			if (issinglequote)
				match=re_singlequote.cap();
			else if (isdoublequote)
				match=re_doublequote.cap();
			else
				match=re_noquote.cap();

			// substitute %variables
			// settings
			if ( isTag( match, "%dev" ) )
			{
				conf->setGroup("Fax");
				v = processTag( match, conf->readEntry("Device", "modem") );

			}
			else if (isTag( match, "%server" ))
			{
				conf->setGroup( "Fax" );
				v = conf->readEntry("Server");
				if (v.isEmpty())
					v = getenv("FAXSERVER");
				if (v.isEmpty())
					v = QString::fromLatin1("localhost");
				v = processTag( match, v );
			}
			else if (isTag( match, "%page" ))
			{
				conf->setGroup( "Fax" );
				v = processTag( match, pageSize() );
			}
			else if (isTag( match, "%res" ))
			{
				conf->setGroup( "Fax" );
				v = processTag(match, conf->readEntry("Resolution", "High") == "High");
			}
			else if (isTag( match, "%user" ))
			{
				conf->setGroup("Personal");
				v = processTag(match, conf->readEntry("Name", getenv("USER")));
			}
			else if (isTag( match, "%from" ))
			{
				conf->setGroup( "Personal" );
				v = processTag(match, conf->readEntry("Number"));
			}
			else if (isTag( match, "%email" ))
			{
				KEMailSettings	e;
				v = processTag(match, e.getSetting(KEMailSettings::EmailAddress));
			}
			// arguments
			else if (isTag( match, "%number" ))
				v = processTag( match, stripNumber( item.number) );
			else if (isTag( match, "%rawnumber" ))
				v = processTag( match, item.number );
			else if (isTag( match, "%name" ))
				v = processTag(match, item.name);
			else if (isTag( match, "%comment" ))
				v = processTag(match, fax->comment());
			else if (isTag( match, "%enterprise" ))
				v = processTag(match, item.enterprise);
			else if ( isTag( match, "%time" ) )
				v = processTag( match, fax->time() );
			else if ( isTag( match, "%subject" ) )
				v = processTag( match, fax->subject() );
			else if (isTag( match, "%cover" ))
				v = processTag(match, fax->cover());

			// %variable inside of a quote?
			if (isdoublequote)
				v='"'+v+'"';
			else if (issinglequote)
				v="'"+v+"'";

			cmd.replace (i, match.length(), v);
			i+=v.length();
		}
	}

	return cmd;
}

FaxCtrl::FaxCtrl(QWidget *parent, const char *name)
: QObject(parent, name)
{
	m_process = new KProcess();
	m_process->setUseShell(true);
	connect(m_process, SIGNAL(receivedStdout(KProcess*,char*,int)), SLOT(slotReceivedStdout(KProcess*,char*,int)));
	connect(m_process, SIGNAL(receivedStderr(KProcess*,char*,int)), SLOT(slotReceivedStdout(KProcess*,char*,int)));
	connect(m_process, SIGNAL(processExited(KProcess*)), SLOT(slotProcessExited(KProcess*)));
	connect(this, SIGNAL(faxSent(bool)), SLOT(cleanTempFiles()));
	m_logview = 0;
}

FaxCtrl::~FaxCtrl()
{
	slotCloseLog();
	delete m_process;
}

bool FaxCtrl::send(KdeprintFax *f)
{
	m_command = faxCommand();
	if (m_command.isEmpty())
		return false;

	// replace tags common to all fax "operations"
	m_command = replaceTags( m_command, tagList( 11, "%dev", "%server", "%page", "%res", "%user", "%from", "%email", "%comment", "%time", "%subject", "%cover" ), f );

	m_log = QString::null;
	m_filteredfiles.clear();
	cleanTempFiles();
	m_files = f->files();
	m_faxlist = f->faxList();

	addLogTitle( i18n( "Converting input files to PostScript" ) );
	filter();

	return true;
}

void FaxCtrl::slotReceivedStdout(KProcess*, char *buffer, int len)
{
	QCString	str(buffer, len);
	kdDebug() << "Received stdout: " << str << endl;
	addLog(QString(str));
}

void FaxCtrl::slotProcessExited(KProcess*)
{
	// we exited a process: if there's still entries in m_files, this was a filter
	// process, else this was the fax process
	bool	ok = (m_process->normalExit() && ((m_process->exitStatus() & (m_files.count() > 0 ? 0x1 : 0xFFFFFFFF)) == 0));
	if ( ok )
	{
		if ( m_files.count() > 0 )
		{
			// remove first element
			m_files.remove(m_files.begin());
			if (m_files.count() > 0)
				filter();
			else
				sendFax();
		}
		else if ( !m_faxlist.isEmpty() )
			sendFax();
		else
			faxSent( true );
	}
	else
	{
		emit faxSent(false);
	}
}

QString FaxCtrl::faxCommand()
{
	KConfig	*conf = KGlobal::config();
	conf->setGroup("System");
	QString	sys = conf->readPathEntry("System", "efax");
	QString cmd;
	if (sys == "hylafax")
		cmd = conf->readPathEntry("HylaFax", hylafax_default_cmd);
	else if (sys == "mgetty")
		cmd = conf->readPathEntry("Mgetty", mgetty_default_cmd);
	else if ( sys == "other" )
		cmd = conf->readPathEntry( "Other", QString::null );
	else
		cmd = conf->readPathEntry("EFax", efax_default_cmd);
	if (cmd.startsWith("%exe_"))
		cmd = defaultCommand(cmd);
	return cmd;
}

void FaxCtrl::sendFax()
{
	if ( m_command.find( "%files" ) != -1 )
	{
		// replace %files tag
		QString	filestr;
		for (QStringList::ConstIterator it=m_filteredfiles.begin(); it!=m_filteredfiles.end(); ++it)
			filestr += (quote(*it)+" ");
		m_command.replace("%files", filestr);
	}

	if ( !m_faxlist.isEmpty() )
	{
		KdeprintFax::FaxItem item = m_faxlist.first();
		m_faxlist.remove(m_faxlist.begin());

		addLogTitle( i18n( "Sending fax to %1 (%2)" ).arg( item.number ).arg( item.name ) );

		QString cmd = replaceTags( m_command, tagList( 4, "%number", "%name", "%enterprise", "%rawnumber" ), NULL, item );
		m_process->clearArguments();
		*m_process << cmd;
		addLog(i18n("Sending to fax using: %1").arg(cmd));
		if (!m_process->start(KProcess::NotifyOnExit, KProcess::AllOutput))
			emit faxSent(false);
		else
			emit message(i18n("Sending fax to %1...").arg( item.number ));
	}
}

void FaxCtrl::filter()
{
	if (m_files.count() > 0)
	{
		QString	mimeType = KMimeType::findByURL(KURL(m_files[0]), 0, true)->name();
		if (mimeType == "application/postscript" || mimeType == "image/tiff")
		{
			emit message(i18n("Skipping %1...").arg(m_files[0]));
			m_filteredfiles.prepend(m_files[0]);
			m_files.remove(m_files.begin());
			filter();
		}
		else
		{
			QString	tmp = locateLocal("tmp","kdeprintfax_") + kapp->randomString(8);
			m_filteredfiles.prepend(tmp);
			m_tempfiles.append(tmp);
			m_process->clearArguments();
			*m_process << locate("data","kdeprintfax/anytops") << "-m" << KProcess::quote(locate("data","kdeprintfax/faxfilters"))
				<< QString::fromLatin1("--mime=%1").arg(mimeType)
				<< "-p" << pageSize()
				<<  KProcess::quote(m_files[0]) << KProcess::quote(tmp);
			if (!m_process->start(KProcess::NotifyOnExit, KProcess::AllOutput))
				emit faxSent(false);
			else
				emit message(i18n("Filtering %1...").arg(m_files[0]));
		}
	}
	else
	{
		sendFax();
	}
}

bool FaxCtrl::abort()
{
	if (m_process->isRunning())
		return m_process->kill();
	else
		return false;
}

void FaxCtrl::viewLog(QWidget *)
{
	if (!m_logview)
	{
		QWidget	*topView = new QWidget(0, "LogView", WType_TopLevel|WStyle_DialogBorder|WDestructiveClose);
		m_logview = new KTextEdit(topView);
		m_logview->setTextFormat( Qt::LogText );
		m_logview->setWordWrap( QTextEdit::WidgetWidth );
		m_logview->setPaper( Qt::white );
		//m_logview->setReadOnly(true);
		//m_logview->setWordWrap(QTextEdit::NoWrap);
		QPushButton	*m_clear = new KPushButton(KStdGuiItem::clear(), topView);
		QPushButton	*m_close = new KPushButton(KStdGuiItem::close(), topView);
		QPushButton *m_print = new KPushButton( KStdGuiItem::print(), topView );
		QPushButton *m_save = new KPushButton( KStdGuiItem::saveAs(), topView );
		m_close->setDefault(true);
		connect(m_clear, SIGNAL(clicked()), SLOT(slotClearLog()));
		connect(m_close, SIGNAL(clicked()), SLOT(slotCloseLog()));
		connect(m_logview, SIGNAL(destroyed()), SLOT(slotCloseLog()));
		connect( m_print, SIGNAL( clicked() ), SLOT( slotPrintLog() ) );
		connect( m_save, SIGNAL( clicked() ), SLOT( slotSaveLog() ) );

		QVBoxLayout	*l0 = new QVBoxLayout(topView, 10, 10);
		l0->addWidget(m_logview);
		QHBoxLayout	*l1 = new QHBoxLayout(0, 0, 10);
		l0->addLayout(l1);
		l1->addStretch(1);
		l1->addWidget( m_save );
		l1->addWidget( m_print );
		l1->addWidget(m_clear);
		l1->addWidget(m_close);

		m_logview->setText(m_log);

		topView->resize(450, 350);
		topView->show();
	}
	else
	{
		KWin::activateWindow(m_logview->parentWidget()->winId());
	}
}

void FaxCtrl::addLogTitle( const QString& s )
{
	QString t( s );
	t.prepend( '\n' ).append( '\n' );
	addLog( t, true );
}

void FaxCtrl::addLog(const QString& s, bool isTitle)
{
	QString t = QStyleSheet::escape(s);
	if ( isTitle )
		t.prepend( "<font color=red><b>" ).append( "</b></font>" );
	m_log.append( t + '\n' );
	if (m_logview)
		m_logview->append(t);
}

QString FaxCtrl::faxSystem()
{
	KConfig	*conf = KGlobal::config();
	conf->setGroup("System");
	QString	s = conf->readEntry("System", "efax");
	s[0] = s[0].upper();
	return s;
}

void FaxCtrl::cleanTempFiles()
{
	for (QStringList::ConstIterator it=m_tempfiles.begin(); it!=m_tempfiles.end(); ++it)
		QFile::remove(*it);
	m_tempfiles.clear();
}

void FaxCtrl::slotClearLog()
{
	m_log = QString::null;
	if (m_logview)
		m_logview->clear();
}

void FaxCtrl::slotCloseLog()
{
	const QObject	*obj = sender();
	if (m_logview)
	{
		QTextEdit	*view = m_logview;
		m_logview = 0;
		if (obj && obj->inherits("QPushButton"))
			delete view->parentWidget();
kdDebug() << "slotClose()" << endl;
	}
}

void FaxCtrl::slotPrintLog()
{
	if ( m_logview )
	{
		KPrinter printer;
		printer.setDocName( i18n( "Fax log" ) );
		printer.setDocFileName( "faxlog" );
		if ( printer.setup( m_logview->topLevelWidget(), i18n( "Fax Log" ) ) )
		{
			QPainter painter( &printer );
			QPaintDeviceMetrics metric( &printer );
			QRect body( 0, 0, metric.width(), metric.height() ), view( body );
			//QString txt = m_logview->text();
			QString txt = m_log;

			txt.replace( '\n', "<br>" );
			txt.prepend( "<h2>" + i18n( "KDEPrint Fax Tool Log" ) + "</h2>" );

			kdDebug() << "Log: " << txt << endl;
			QSimpleRichText richText( txt, m_logview->font() );

			richText.setWidth( &painter, body.width() );
			do
			{
				richText.draw( &painter, body.left(), body.top(), view, m_logview->colorGroup() );
				view.moveBy( 0, body.height() );
				painter.translate( 0, -body.height() );
				if ( view.top() >= richText.height() )
					break;
				printer.newPage();
			} while ( true );
		}
	}
}

void FaxCtrl::slotSaveLog()
{
	if ( m_logview )
	{
		QString filename = KFileDialog::getSaveFileName( QString::null, QString::null, m_logview );
		if ( !filename.isEmpty() )
		{
			QFile f( filename );
			if ( f.open( IO_WriteOnly ) )
			{
				QTextStream t( &f );
				t << i18n( "KDEPrint Fax Tool Log" ) << endl;
				t << m_logview->text() << endl;
				f.close();
			}
			else
				KMessageBox::error( m_logview, i18n( "Cannot open file for writing." ) );
		}
	}
}

#include "faxctrl.moc"
