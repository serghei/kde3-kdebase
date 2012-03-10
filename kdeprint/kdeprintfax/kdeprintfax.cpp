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

#include "kdeprintfax.h"
#include "faxab.h"
#include "faxctrl.h"
#include "configdlg.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qdatetimeedit.h>
#include <qcombobox.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kstdaction.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kaction.h>
#include <klistbox.h>
#include <klistview.h>
#include <qheader.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmimetype.h>
#include <kseparator.h>
#include <ksystemtray.h>
#include <kstatusbar.h>
#include <ksqueezedtextlabel.h>
#include <krun.h>
#include <kopenwith.h>
#include <kpushbutton.h>
#include <kurldrag.h>
#include <kdebug.h>

KdeprintFax::KdeprintFax(QWidget *parent, const char *name)
: KMainWindow(parent, name)
{
	m_faxctrl = new FaxCtrl(this);
	m_quitAfterSend = false;
	connect(m_faxctrl, SIGNAL(message(const QString&)), SLOT(slotMessage(const QString&)));
	connect(m_faxctrl, SIGNAL(faxSent(bool)), SLOT(slotFaxSent(bool)));

	QWidget	*mainw = new QWidget(this);
	setCentralWidget(mainw);
	m_files = new KListBox(mainw);
	connect( m_files, SIGNAL( currentChanged( QListBoxItem* ) ), SLOT( slotCurrentChanged() ) );
	m_upbtn = new KPushButton( mainw );
	m_upbtn->setIconSet( SmallIconSet( "up" ) );
	QToolTip::add( m_upbtn, i18n( "Move up" ) );
	connect( m_upbtn, SIGNAL( clicked() ), SLOT( slotMoveUp() ) );
	m_upbtn->setEnabled( false );
	m_downbtn = new KPushButton( mainw );
	m_downbtn->setIconSet( SmallIconSet( "down" ) );
	QToolTip::add( m_downbtn, i18n( "Move down" ) );
	connect( m_downbtn, SIGNAL( clicked() ), SLOT( slotMoveDown() ) );
	m_downbtn->setEnabled( false );
	QLabel	*m_filelabel = new QLabel(i18n("F&iles:"), mainw);
        m_filelabel->setBuddy(m_files);
        KSeparator*m_line = new KSeparator( KSeparator::HLine, mainw);
		KSeparator *m_line2 = new KSeparator( KSeparator::HLine, mainw );
	m_numbers = new KListView( mainw );
	m_numbers->addColumn( i18n("Fax Number") );
	m_numbers->addColumn( i18n("Name") );
	m_numbers->addColumn( i18n("Enterprise") );
	m_numbers->header()->setStretchEnabled( true );
	m_numbers->setSelectionMode( QListView::Extended );
	connect( m_numbers, SIGNAL( selectionChanged() ), SLOT( slotFaxSelectionChanged() ) );
	connect( m_numbers, SIGNAL( executed( QListViewItem* ) ), SLOT( slotFaxExecuted( QListViewItem* ) ) );
	m_newbtn = new KPushButton( mainw );
	m_newbtn->setPixmap( SmallIcon( "edit" ) );
	QToolTip::add( m_newbtn, i18n( "Add fax number" ) );
	connect( m_newbtn, SIGNAL( clicked() ), SLOT( slotFaxAdd() ) );
	m_abbtn = new KPushButton( mainw );
	m_abbtn->setPixmap( SmallIcon( "kaddressbook" ) );
	QToolTip::add( m_abbtn, i18n( "Add fax number from addressbook" ) );
	connect( m_abbtn, SIGNAL( clicked() ), SLOT( slotKab() ) );
	m_delbtn = new KPushButton( mainw );
	m_delbtn->setIconSet( SmallIconSet( "edittrash" ) );
	QToolTip::add( m_delbtn, i18n( "Remove fax number" ) );
	m_delbtn->setEnabled( false );
	connect( m_delbtn, SIGNAL( clicked() ), SLOT( slotFaxRemove() ) );
	QLabel	*m_commentlabel = new QLabel(i18n("&Comment:"), mainw);
	KSystemTray	*m_tray = new KSystemTray(this);
	m_tray->setPixmap(SmallIcon("kdeprintfax"));
	m_tray->show();
	m_comment = new QTextEdit(mainw);
// I don't understand why anyone would want to turn off word wrap. It makes
// the text hard to read and write. It provides no benefit. Therefore,
// I commented out the next line. [Ray Lischner]
//	m_comment->setWordWrap(QTextEdit::NoWrap);
	m_comment->setLineWidth(1);
	m_commentlabel->setBuddy(m_comment);
	QLabel	*m_timelabel = new QLabel(i18n("Sched&ule:"), mainw);
	m_timecombo = new QComboBox(mainw);
	m_timecombo->insertItem(i18n("Now"));
	m_timecombo->insertItem(i18n("At Specified Time"));
	m_timecombo->setCurrentItem(0);
	m_timelabel->setBuddy(m_timecombo);
	m_time = new QTimeEdit(mainw);
	m_time->setTime(QTime::currentTime());
	m_time->setEnabled(false);
	connect(m_timecombo, SIGNAL(activated(int)), SLOT(slotTimeComboActivated(int)));
	m_cover = new QCheckBox(i18n("Send Co&ver Sheet"), mainw);
	connect(m_cover, SIGNAL(toggled(bool)), SLOT(slotCoverToggled(bool)));
	m_subject = new QLineEdit( mainw );
	QLabel *m_subjectlabel = new QLabel( i18n( "Su&bject:" ), mainw );
	m_subjectlabel->setBuddy( m_subject );

	QGridLayout	*l0 = new QGridLayout(mainw, 10, 2, 10, 5);
	l0->setColStretch(1,1);
	l0->addWidget(m_filelabel, 0, 0, Qt::AlignLeft|Qt::AlignTop);
	QHBoxLayout *l2 = new QHBoxLayout( 0, 0, 10 );
	QVBoxLayout *l3 = new QVBoxLayout( 0, 0, 5 );
	l0->addLayout( l2, 0, 1 );
	l2->addWidget( m_files );
	l2->addLayout( l3 );
	//l3->addStretch( 1 );
	l3->addWidget( m_upbtn );
	l3->addWidget( m_downbtn );
	l3->addStretch( 1 );
	l0->addMultiCellWidget(m_line, 1, 1, 0, 1);
	l0->addRowSpacing(1, 10);
	QHBoxLayout *l5 = new QHBoxLayout( 0, 0, 10 );
	QVBoxLayout *l6 = new QVBoxLayout( 0, 0, 5 );
	l0->addMultiCellLayout( l5, 2, 4, 0, 1 );
	l5->addWidget( m_numbers );
	l5->addLayout( l6 );
	l6->addWidget( m_newbtn );
	l6->addWidget( m_delbtn );
	l6->addWidget( m_abbtn );
	l6->addStretch( 1 );
	l0->addMultiCellWidget( m_line2, 5, 5, 0, 1 );
	l0->addRowSpacing( 5, 10 );
	l0->addWidget( m_cover, 6, 1 );
	l0->addWidget( m_subjectlabel, 7, 0 );
	l0->addWidget( m_subject, 7, 1 );
	l0->addWidget(m_commentlabel, 8, 0, Qt::AlignTop|Qt::AlignLeft);
	l0->addWidget(m_comment, 8, 1);
	l0->addWidget(m_timelabel, 9, 0);
	QHBoxLayout	*l1 = new QHBoxLayout(0, 0, 5);
	l0->addLayout(l1, 9, 1);
	l1->addWidget(m_timecombo, 1);
	l1->addWidget(m_time, 0);

	m_msglabel = new KSqueezedTextLabel(statusBar());
	statusBar()->addWidget(m_msglabel, 1);
	statusBar()->insertFixedItem(i18n("Processing..."), 1);
	statusBar()->changeItem(i18n("Idle"), 1);
	statusBar()->insertFixedItem("hylafax/efax", 2);
	initActions();
	setAcceptDrops(true);
	setCaption(i18n("Send to Fax"));
	updateState();

	resize(550,500);
	QWidget	*d = kapp->desktop();
	move((d->width()-width())/2, (d->height()-height())/2);
}

KdeprintFax::~KdeprintFax()
{
}

void KdeprintFax::initActions()
{
	new KAction(i18n("&Add File..."), "filenew", Qt::Key_Insert, this, SLOT(slotAdd()), actionCollection(), "file_add");
	new KAction(i18n("&Remove File"), "remove", Qt::Key_Delete, this, SLOT(slotRemove()), actionCollection(), "file_remove");
	new KAction(i18n("&Send Fax"), "connect_established", Qt::Key_Return, this, SLOT(slotFax()), actionCollection(), "fax_send");
	new KAction(i18n("A&bort"), "stop", Qt::Key_Escape, this, SLOT(slotAbort()), actionCollection(), "fax_stop");
	new KAction(i18n("A&ddress Book"), "kaddressbook", Qt::CTRL+Qt::Key_A, this, SLOT(slotKab()), actionCollection(), "fax_ab");
	new KAction(i18n("V&iew Log"), "contents", Qt::CTRL+Qt::Key_L, this, SLOT(slotViewLog()), actionCollection(), "fax_log");
	new KAction(i18n("Vi&ew File"), "filefind", Qt::CTRL+Qt::Key_O, this, SLOT(slotView()), actionCollection(), "file_view");
	new KAction( i18n( "&New Fax Recipient..." ), "edit", Qt::CTRL+Qt::Key_N, this, SLOT( slotFaxAdd() ), actionCollection(), "fax_add" );

	KStdAction::quit(this, SLOT(slotQuit()), actionCollection());
	setStandardToolBarMenuEnabled(true);
	KStdAction::showMenubar(this, SLOT(slotToggleMenuBar()), actionCollection());
	KStdAction::preferences(this, SLOT(slotConfigure()), actionCollection());
        KStdAction::keyBindings(guiFactory(), SLOT(configureShortcuts()), 
actionCollection());
	actionCollection()->action("fax_stop")->setEnabled(false);
	connect(actionCollection()->action("file_remove"), SIGNAL(enabled(bool)), actionCollection()->action("file_view"), SLOT(setEnabled(bool)));
	actionCollection()->action("file_remove")->setEnabled(false);

	createGUI();
}

void KdeprintFax::slotToggleMenuBar()
{
	if (menuBar()->isVisible()) menuBar()->hide();
	else menuBar()->show();
}

void KdeprintFax::slotAdd()
{
	KURL	url = KFileDialog::getOpenURL(QString::null, QString::null, this);
	if (!url.isEmpty())
		addURL(url);
}

void KdeprintFax::slotRemove()
{
	if (m_files->currentItem() >= 0)
		m_files->removeItem(m_files->currentItem());
	if (m_files->count() == 0)
		actionCollection()->action("file_remove")->setEnabled(false);
}

void KdeprintFax::slotView()
{
	if (m_files->currentItem() >= 0)
	{
		new KRun(KURL( m_files->currentText() ));
	}
}

void KdeprintFax::slotFax()
{
	if (m_files->count() == 0)
		KMessageBox::error(this, i18n("No file to fax."));
	else if ( m_numbers->childCount() == 0 )
		KMessageBox::error(this, i18n("No fax number specified."));
	else if (m_faxctrl->send(this))
	{
		actionCollection()->action("fax_send")->setEnabled(false);
		actionCollection()->action("fax_stop")->setEnabled(true);
		statusBar()->changeItem(i18n("Processing..."), 1);
	}
	else
		KMessageBox::error(this, i18n("Unable to start Fax process."));
}

void KdeprintFax::slotAbort()
{
	if (!m_faxctrl->abort())
		KMessageBox::error(this, i18n("Unable to stop Fax process."));
}

void KdeprintFax::slotKab()
{
	QStringList	number, name, enterprise;
	if (FaxAB::getEntry(number, name, enterprise, this))
	{
		for ( unsigned int i = 0; i<number.count(); i++ )
			new QListViewItem( m_numbers, number[ i ], name[ i ], enterprise[ i ] );
	}
}

void KdeprintFax::addURL(KURL url)
{
	QString	target;
	if (KIO::NetAccess::download(url,target,this))
	{
		m_files->insertItem(KMimeType::pixmapForURL(url,0,KIcon::Small),target);
		actionCollection()->action("file_remove")->setEnabled(true);
		slotCurrentChanged();
	}
	else
		KMessageBox::error(this, i18n("Unable to retrieve %1.").arg(url.prettyURL()));
}

void KdeprintFax::setPhone(QString phone)
{
	QString name, enterprise;
	FaxAB::getEntryByNumber(phone, name, enterprise);
	new QListViewItem( m_numbers, phone, name, enterprise );
}

void KdeprintFax::sendFax( bool quitAfterSend )
{
	slotFax();
	m_quitAfterSend = quitAfterSend;
}

void KdeprintFax::dragEnterEvent(QDragEnterEvent *e)
{
	e->accept(KURLDrag::canDecode(e));
}

void KdeprintFax::dropEvent(QDropEvent *e)
{
	KURL::List l;
	if (KURLDrag::decode(e, l))
	{
		for (KURL::List::ConstIterator it = l.begin(); it != l.end(); ++it)
			addURL(*it);
	}
}

QStringList KdeprintFax::files()
{
	QStringList	l;
	for (uint i=0; i<m_files->count(); i++)
		l.append(m_files->text(i));
	return l;
}


int KdeprintFax::faxCount() const
{
	return m_numbers->childCount();
}

/*
QListViewItem* KdeprintFax::faxItem( int i ) const
{
	QListViewItem *item = m_numbers->firstChild();
	while ( i && item && item->nextSibling() )
	{
		item = item->nextSibling();
		i--;
	}
	if ( i || !item )
		kdError() << "KdeprintFax::faxItem(" << i << ") => fax item index out of bound" << endl;
	return item;
}

QString KdeprintFax::number( int i ) const
{
	QListViewItem *item = faxItem( i );
	return ( item ? item->text( 0 ) : QString::null );
}

QString KdeprintFax::name( int i ) const
{
	QListViewItem *item = faxItem( i );
	return ( item ? item->text( 1 ) : QString::null );
}

QString KdeprintFax::enterprise( int i ) const
{
	QListViewItem *item = faxItem( i );
	return ( item ? item->text( 2 ) : QString::null );
}
*/

KdeprintFax::FaxItemList KdeprintFax::faxList() const
{
	FaxItemList list;
	QListViewItemIterator it( m_numbers );
	while ( it.current() )
	{
		FaxItem item;
		item.number = it.current()->text( 0 );
		item.name = it.current()->text( 1 );
		item.enterprise = it.current()->text( 2 );
		list << item;
		++it;
	}
	return list;
}

QString KdeprintFax::comment() const
{
	return m_comment->text();
}

bool KdeprintFax::cover() const
{
	return m_cover->isChecked();
}

QString KdeprintFax::subject() const
{
	return m_subject->text();
}

void KdeprintFax::slotMessage(const QString& msg)
{
	m_msglabel->setText(msg);
}

void KdeprintFax::slotFaxSent(bool status)
{
	actionCollection()->action("fax_send")->setEnabled(true);
	actionCollection()->action("fax_stop")->setEnabled(false);
	statusBar()->changeItem(i18n("Idle"), 1);

 	if( m_quitAfterSend ) {
		slotQuit();
	}
	else {
		if (!status)
			KMessageBox::error(this, i18n("Fax error: see log message for more information."));
		slotMessage(QString::null);
	}
}

void KdeprintFax::slotViewLog()
{
	m_faxctrl->viewLog(this);
}

void KdeprintFax::slotConfigure()
{
	if (ConfigDlg::configure(this))
		updateState();
}

void KdeprintFax::updateState()
{
	QString	cmd = m_faxctrl->faxCommand();
	m_cover->setEnabled(cmd.find("%cover") != -1);
	if ( !m_cover->isEnabled() )
		m_cover->setChecked(false);
	m_comment->setEnabled(cmd.find("%comment") != -1 && m_cover->isChecked());
	//m_comment->setPaper(m_comment->isEnabled() ? colorGroup().brush(QColorGroup::Base) : colorGroup().brush(QColorGroup::Background));
	if (!m_comment->isEnabled())
	{
		m_comment->setText("");
		m_comment->setPaper( colorGroup().background() );
	}
	else
		m_comment->setPaper( colorGroup().base() );
	/*
	m_enterprise->setEnabled(cmd.find("%enterprise") != -1);
	if (!m_enterprise->isEnabled())
		m_enterprise->setText("");
	*/
	if (cmd.find("%time") == -1)
	{
		m_timecombo->setCurrentItem(0);
		m_timecombo->setEnabled(false);
		slotTimeComboActivated(0);
	}
	else
		m_timecombo->setEnabled( true );
	/*m_name->setEnabled( cmd.find( "%name" ) != -1 );*/
	m_subject->setEnabled( cmd.find( "%subject" ) != -1 && m_cover->isChecked() );
	statusBar()->changeItem(m_faxctrl->faxSystem(), 2);
}

void KdeprintFax::slotQuit()
{
	close(true);
}

void KdeprintFax::slotTimeComboActivated(int ID)
{
	m_time->setEnabled(ID == 1);
}

QString KdeprintFax::time() const
{
	if (!m_time->isEnabled())
		return QString::null;
	return m_time->time().toString("hh:mm");
}

void KdeprintFax::slotMoveUp()
{
	int index = m_files->currentItem();
	if ( index > 0 )
	{
		QListBoxItem *item = m_files->item( index );
		m_files->takeItem( item );
		m_files->insertItem( item, index-1 );
		m_files->setCurrentItem( index-1 );
	}
}

void KdeprintFax::slotMoveDown()
{
	int index = m_files->currentItem();
	if ( index >= 0 && index < ( int )m_files->count()-1 )
	{
		QListBoxItem *item = m_files->item( index );
		m_files->takeItem( item );
		m_files->insertItem( item, index+1 );
		m_files->setCurrentItem( index+1 );
	}
}

/** The user or program toggled the "Cover Sheet" check box.
 * Update the state of the other controls to reflect the
 * new status.
 */
void KdeprintFax::slotCoverToggled(bool)
{
	updateState();
}

void KdeprintFax::slotCurrentChanged()
{
	int index = m_files->currentItem();
	m_upbtn->setEnabled( index > 0 );
	m_downbtn->setEnabled( index >=0 && index < ( int )m_files->count()-1 );
}

void KdeprintFax::slotFaxSelectionChanged()
{
	QListViewItemIterator it( m_numbers, QListViewItemIterator::Selected );
	m_delbtn->setEnabled( it.current() != NULL );
}

void KdeprintFax::slotFaxRemove()
{
	QListViewItemIterator it( m_numbers, QListViewItemIterator::Selected );
	QPtrList<QListViewItem> items;
	items.setAutoDelete( true );
	while ( it.current() )
	{
		items.append( it.current() );
		++it;
	}
	items.clear();
	/* force this slot to be called, to update buttons state */
	slotFaxSelectionChanged();
}

void KdeprintFax::slotFaxAdd()
{
	QString number, name, enterprise;
	if ( manualFaxDialog( number, name, enterprise ) )
	{
		new QListViewItem( m_numbers, number, name, enterprise );
	}
}

void KdeprintFax::slotFaxExecuted( QListViewItem *item )
{
	if ( item )
	{
		QString number = item->text( 0 ), name = item->text( 1 ), enterprise = item->text( 2 );
		if ( manualFaxDialog( number, name, enterprise ) )
		{
			item->setText( 0, number );
			item->setText( 1, name );
			item->setText( 2, enterprise );
		}
	}
}

bool KdeprintFax::manualFaxDialog( QString& number, QString& name, QString& enterprise )
{
	/* dialog construction */
	KDialogBase dlg( this, "manualFaxDialog", true, i18n( "Fax Number" ), KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );
	QWidget *mainw = new QWidget( &dlg );
	QLabel *lab0 = new QLabel( i18n( "Enter recipient fax properties." ), mainw );
	QLabel *lab1 = new QLabel( i18n( "&Number:" ), mainw );
	QLabel *lab2 = new QLabel( i18n( "N&ame:" ), mainw );
	QLabel *lab3 = new QLabel( i18n( "&Enterprise:" ), mainw );
	QLineEdit *edit_number = new QLineEdit( number, mainw );
	QLineEdit *edit_name = new QLineEdit( name, mainw );
	QLineEdit *edit_enterprise = new QLineEdit( enterprise, mainw );
	lab1->setBuddy( edit_number );
	lab2->setBuddy( edit_name );
	lab3->setBuddy( edit_enterprise );
	QGridLayout *l0 = new QGridLayout( mainw, 5, 2, 0, 5 );
	l0->setColStretch( 1, 1 );
	l0->addMultiCellWidget( lab0, 0, 0, 0, 1 );
	l0->setRowSpacing( 1, 10 );
	l0->addWidget( lab1, 2, 0 );
	l0->addWidget( lab2, 3, 0 );
	l0->addWidget( lab3, 4, 0 );
	l0->addWidget( edit_number, 2, 1 );
	l0->addWidget( edit_name, 3, 1 );
	l0->addWidget( edit_enterprise, 4, 1 );
	dlg.setMainWidget( mainw );
	dlg.resize( 300, 10 );

	/* dialog execution */
	while ( 1 )
		if ( dlg.exec() )
		{
			if ( edit_number->text().isEmpty() )
			{
				KMessageBox::error( this, i18n( "Invalid fax number." ) );
			}
			else
			{
				number = edit_number->text();
				name = edit_name->text();
				enterprise = edit_enterprise->text();
				return true;
			}
		}
		else
			return false;
}

#include "kdeprintfax.moc"
