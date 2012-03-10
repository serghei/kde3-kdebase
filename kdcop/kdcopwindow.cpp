/*
 * Copyright (C) 2000 by Matthias Kalle Dalheimer <kalle@kde.org>
 *               2004 by Olivier Goffart  <ogoffart @ tiscalinet.be>
 *
 * Licensed under the Artistic License.
 */

#include "kdcopwindow.h"
#include "kdcoplistview.h"

#include <dcopclient.h>
#include <klocale.h>
#include <kdatastream.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <keditlistbox.h>
#include <klistbox.h>
#include <kdialogbase.h>
#include <kstdaccel.h>
#include <kcolorbutton.h>
#include <klistviewsearchline.h>

#include <qtimer.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qsize.h>
#include <qrect.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <dcopref.h>
#include <qvbox.h>
#include <qimage.h>
#include <qheader.h>

#include <kdebug.h>
#include <kkeydialog.h>
#include <stdio.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <kurlrequester.h>

class DCOPBrowserApplicationItem;
class DCOPBrowserInterfaceItem;
class DCOPBrowserFunctionItem;

//------------------------------

class KMultiIntEdit : public QVBox
{
public:
	KMultiIntEdit(QWidget *parent , const char * name=0) : QVBox(parent,name) {}
	void addField(int key, const QString & caption  )
	{
		QHBox *l=new QHBox(this);
		new QLabel(caption + ": ", l);
		KLineEdit* e = new KLineEdit( l );
		m_widgets.insert(key, e ) ;
        	e->setValidator( new QIntValidator( e ) );
	}
	int field(int key)
	{
		KLineEdit *e=m_widgets[key];
		if(!e) return 0;
		return e->text().toInt();
	}

private:
	QMap<int,KLineEdit*> m_widgets;
};

//------------------------------

DCOPBrowserItem::DCOPBrowserItem
(QListView * parent, DCOPBrowserItem::Type type)
  : QListViewItem(parent),
    type_(type)
{
}

DCOPBrowserItem::DCOPBrowserItem
(QListViewItem * parent, DCOPBrowserItem::Type type)
  :  QListViewItem(parent),
     type_(type)
{
}

  DCOPBrowserItem::Type
DCOPBrowserItem::type() const
{
  return type_;
}

// ------------------------------------------------------------------------

DCOPBrowserApplicationItem::DCOPBrowserApplicationItem
(QListView * parent, const QCString & app)
  : DCOPBrowserItem(parent, Application),
    app_(app)
{
  setExpandable(true);
  setText(0, QString::fromUtf8(app_));
  setPixmap(0,  KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "exec" ), KIcon::Small ));


	/* Get the icon:  we use the icon from a mainwindow in that class.
	    a lot of applications has a app-mainwindow#1 object, but others can still have
	    a main window with another name. In that case, we search for a main window with the qt object.
	 * Why don't search with qt dirrectly?
	    simply because some application that have a 'mainwindow#1' doesn't have a qt object.  And, for
	    reason i don't know, some application return stanges result. Some konqueror instance are returning
	    konqueror-mainwindow#3 while only the #1 exists,  I already seen the same problem with konsole
	 * All calls are async to don't block the GUI if the clients does not reply immediatly
	 */

	QRegExp rx( "([^\\-]+)");  // remove the possible processus id
	rx.search(app_);           //   konqueror-123  => konqueror-mainwindow#1
	QString mainWindowName= rx.cap(1) + "-mainwindow#1" ;

	QByteArray data;
	int callId=kapp->dcopClient()->callAsync( app_, mainWindowName.utf8(), "icon()", data, this, SLOT(retreiveIcon(int, const QCString&, const QByteArray&)));

	if(!callId)
	{
		//maybe there is another mainwindow registered with another name.
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << QCString( "MainWindow" );

		kapp->dcopClient()->callAsync( app_, "qt", "find(QCString)", data, this, SLOT(slotGotWindowName(int, const QCString&, const QByteArray& )));
	}
}

  void
DCOPBrowserApplicationItem::setOpen(bool o)
{
  DCOPBrowserItem::setOpen(o);

  if (0 == firstChild())
    populate();
}

  void
DCOPBrowserApplicationItem::populate()
{
  KApplication::setOverrideCursor(waitCursor);

  bool ok = false;
  bool isDefault = false;

  QCStringList objs = kapp->dcopClient()->remoteObjects(app_, &ok);

  for (QCStringList::ConstIterator it = objs.begin(); it != objs.end(); ++it)
  {
    if (*it == "default")
    {
      isDefault = true;
      continue;
    }
    new DCOPBrowserInterfaceItem(this, app_, *it, isDefault);
    isDefault = false;
  }

  KApplication::restoreOverrideCursor();
}

void DCOPBrowserApplicationItem::slotGotWindowName(int /*callId*/, const QCString& /*replyType*/, const QByteArray &replyData)
{
	QDataStream reply(replyData, IO_ReadOnly);
	QCStringList mainswindows;
	reply >> mainswindows;
	QStringList sl=QStringList::split("/",mainswindows.first() );
	if(sl.count() >= 1)
	{
		QString mainWindowName=sl[1];
		if(!mainWindowName.isEmpty())
		{
			QByteArray data;
			kapp->dcopClient()->callAsync( app_, mainWindowName.utf8(), "icon()", data,
				this, SLOT(retreiveIcon(int, const QCString&, const QByteArray&)));
		}
	}
}

void DCOPBrowserApplicationItem::retreiveIcon(int /*callId*/,const QCString& /*replyType*/, const QByteArray &replyData)
{
	QDataStream reply(replyData, IO_ReadOnly);
	QPixmap returnQPixmap;
	reply >> returnQPixmap;
	if(!returnQPixmap.isNull())
		setPixmap(0, QPixmap(returnQPixmap.convertToImage().smoothScale(16,16)) );
	else
		kdDebug() << "Unable to retreive the icon" << endl;
}

// ------------------------------------------------------------------------

DCOPBrowserInterfaceItem::DCOPBrowserInterfaceItem
(
 DCOPBrowserApplicationItem * parent,
 const QCString & app,
 const QCString & object,
 bool def
)
  : DCOPBrowserItem(parent, Interface),
    app_(app),
    object_(object)
{
  setExpandable(true);

  if (def)
    setText(0, i18n("%1 (default)").arg(QString::fromUtf8(object_)));
  else
    setText(0, QString::fromUtf8(object_));
}

  void
DCOPBrowserInterfaceItem::setOpen(bool o)
{
  DCOPBrowserItem::setOpen(o);

  if (0 == firstChild())
    populate();
}

  void
DCOPBrowserInterfaceItem::populate()
{
  KApplication::setOverrideCursor(waitCursor);

  bool ok = false;

  QCStringList funcs = kapp->dcopClient()->remoteFunctions(app_, object_, &ok);

  for (QCStringList::ConstIterator it = funcs.begin(); it != funcs.end(); ++it)
    if ((*it) != "QCStringList functions()")
      new DCOPBrowserFunctionItem(this, app_, object_, *it);

  KApplication::restoreOverrideCursor();
}

// ------------------------------------------------------------------------

DCOPBrowserFunctionItem::DCOPBrowserFunctionItem
(
 DCOPBrowserInterfaceItem * parent,
 const QCString & app,
 const QCString & object,
 const QCString & function
)
  : DCOPBrowserItem(parent, Function),
    app_(app),
    object_(object),
    function_(function)
{
  setExpandable(false);
  setText(0, QString::fromUtf8(function_));
}

  void
DCOPBrowserFunctionItem::setOpen(bool o)
{
  DCOPBrowserItem::setOpen(o);
}

// ------------------------------------------------------------------------

KDCOPWindow::KDCOPWindow(QWidget *parent, const char * name)
  : KMainWindow(parent, name)
{
  dcopClient = kapp->dcopClient();
  dcopClient->attach();
  resize( 377, 480 );
  statusBar()->message(i18n("Welcome to the KDE DCOP browser"));

	mainView = new kdcopview(this, "KDCOP");
        mainView->kListViewSearchLine1->setListView( mainView->lv );
	setCentralWidget(mainView);
	mainView->lv->addColumn(i18n("Application"));
	mainView->lv->header()->setStretchEnabled(true, 0);
//	mainView->lv->addColumn(i18n("Interface"));
//	mainView->lv->addColumn(i18n("Function"));
	mainView->lv->setDragAutoScroll( FALSE );
	mainView->lv->setRootIsDecorated( TRUE );
  connect
    (
     mainView->lv,
     SIGNAL(doubleClicked(QListViewItem *)),
     SLOT(slotCallFunction(QListViewItem *))
    );

  connect
    (
     mainView->lv,
     SIGNAL(currentChanged(QListViewItem *)),
     SLOT(slotCurrentChanged(QListViewItem *))
    );


  // set up the actions
  KStdAction::quit( this, SLOT( close() ), actionCollection() );
  KStdAction::copy( this, SLOT( slotCopy()), actionCollection() );
  KStdAction::keyBindings( guiFactory(), SLOT( configureShortcuts() ), actionCollection() );


  (void) new KAction( i18n( "&Reload" ), "reload", KStdAccel::shortcut(KStdAccel::Reload), this, SLOT( slotReload() ), actionCollection(), "reload" );

  exeaction =
    new KAction
    (
     i18n("&Execute"),
      "exec",
     CTRL + Key_E,
     this,
     SLOT(slotCallFunction()),
     actionCollection(),
     "execute"
    );

  exeaction->setEnabled(false);
  exeaction->setToolTip(i18n("Execute the selected DCOP call."));

  langmode = new KSelectAction ( i18n("Language Mode"),
  		CTRL + Key_M,
		this,
		SLOT(slotMode()),
		actionCollection(),
		"langmode");
  langmode->setEditable(false);
  langmode->setItems(QStringList::split(",", "Shell,C++,Python"));
  langmode->setToolTip(i18n("Set the current language export."));
  langmode->setCurrentItem(0);
  slotMode();
  connect
    (
     dcopClient,
     SIGNAL(applicationRegistered(const QCString &)),
     SLOT(slotApplicationRegistered(const QCString &))
    );

  connect
    (
     dcopClient,
     SIGNAL(applicationRemoved(const QCString &)),
     SLOT(slotApplicationUnregistered(const QCString &))
    );

  dcopClient->setNotifications(true);
  createGUI();
  setCaption(i18n("DCOP Browser"));
	mainView->lb_replyData->hide();
  QTimer::singleShot(0, this, SLOT(slotFillApplications()));
}


void KDCOPWindow::slotCurrentChanged( QListViewItem* i )
{
  DCOPBrowserItem* item = (DCOPBrowserItem*)i;

  if( item->type() == DCOPBrowserItem::Function )
    exeaction->setEnabled( true );
  else
    exeaction->setEnabled( false );
}


void KDCOPWindow::slotCallFunction()
{
  slotCallFunction( mainView->lv->currentItem() );
}

void KDCOPWindow::slotReload()
{
  slotFillApplications();
}

void KDCOPWindow::slotCallFunction( QListViewItem* it )
{
  if(it == 0)
    return;
  DCOPBrowserItem * item = static_cast<DCOPBrowserItem *>(it);

  if (item->type() != DCOPBrowserItem::Function)
    return;

  DCOPBrowserFunctionItem * fitem =
    static_cast<DCOPBrowserFunctionItem *>(item);

  QString unNormalisedSignature = QString::fromUtf8(fitem->function());
  QString normalisedSignature;
  QStringList types;
  QStringList names;

  if (!getParameters(unNormalisedSignature, normalisedSignature, types, names))
  {
    KMessageBox::error
      (this, i18n("No parameters found."), i18n("DCOP Browser Error"));

    return;
  }

  QByteArray data;
  QByteArray replyData;

  QCString replyType;

  QDataStream arg(data, IO_WriteOnly);

  KDialogBase mydialog( this, "KDCOP Parameter Entry", true,
    QString::null, KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );

  mydialog.setCaption
    ( i18n("Call Function %1").arg( fitem->function() ) );

  QFrame *frame = mydialog.makeMainWidget();

  QLabel* h1 = new QLabel( i18n( "Name" ), frame );
  QLabel* h2 = new QLabel( i18n( "Type" ), frame );
  QLabel* h3 = new QLabel( i18n( "Value" ), frame );

  QGridLayout* grid = new QGridLayout( frame, types.count() + 2, 3,
    0, KDialog::spacingHint() );

  grid->addWidget( h1, 0, 0 );
  grid->addWidget( h2, 0, 1 );
  grid->addWidget( h3, 0, 2 );

  // Build up a dialog for parameter entry if there are any parameters.

  if (types.count())
  {
    int i = 0;

    QPtrList<QWidget> wl;

    for (QStringList::ConstIterator it = types.begin(); it != types.end(); ++it)
    {
      i++;

      const QString type = *it;

      const QString name = i-1 < (int)names.count() ? names[i-1] : QString::null;

      if( type == "int" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "int", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
        e->setValidator( new QIntValidator( e ) );
      }
      else if ( type == "unsigned"  || type == "uint" || type == "unsigned int"
             || type == "Q_UINT32" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "unsigned int", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );

        QIntValidator* iv = new QIntValidator( e );
        iv->setBottom( 0 );
        e->setValidator( iv );
      }
      else if ( type == "long" || type == "long int" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "long", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
        e->setValidator( new QIntValidator( e ) );
      }
      else if ( type == "ulong" || type == "unsigned long" || type == "unsigned long int"
             || type == "Q_UINT64" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "unsigned long", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
        e->setValidator( new QIntValidator( e ) );
      }
      else if ( type == "short" || type == "short int" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "long", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
        e->setValidator( new QIntValidator( e ) );
      }
      else if ( type == "ushort" || type == "unsigned short" || type == "unsigned short int"  )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "unsigned short", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
        e->setValidator( new QIntValidator( e ) );
      }
      else if ( type == "float" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "float", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
        e->setValidator( new QDoubleValidator( e ) );
      }
      else if ( type == "double" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "double", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
        e->setValidator( new QDoubleValidator( e ) );
      }
      else if ( type == "bool" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "bool", frame );
        grid->addWidget( l, i, 1 );
        QCheckBox* c = new QCheckBox( frame );
        grid->addWidget( c, i, 2 );
        wl.append( c );
      }
      else if ( type == "QString" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QString", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if ( type == "QCString" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QString", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if ( type == "QStringList" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QStringList", frame );
        grid->addWidget( l, i, 1 );
        KEditListBox* e = new KEditListBox ( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if ( type == "QValueList<QCString>" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QValueList<QCString>", frame );
        grid->addWidget( l, i, 1 );
        KEditListBox* e = new KEditListBox ( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if ( type == "KURL" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "KURL", frame );
        grid->addWidget( l, i, 1 );
        KLineEdit* e = new KLineEdit( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if ( type == "QColor" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QColor", frame );
        grid->addWidget( l, i, 1 );
        KColorButton* e = new KColorButton( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if ( type == "QSize" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QSize", frame );
        grid->addWidget( l, i, 1 );
        KMultiIntEdit* e = new KMultiIntEdit( frame );
        e->addField( 1, i18n("Width") );
        e->addField( 2, i18n("Height") );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if ( type == "QPoint" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QPoint", frame );
        grid->addWidget( l, i, 1 );
        KMultiIntEdit* e = new KMultiIntEdit( frame );
        e->addField( 1, i18n("X") );
        e->addField( 2, i18n("Y") );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if ( type == "QRect" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QRect", frame );
        grid->addWidget( l, i, 1 );
        KMultiIntEdit* e = new KMultiIntEdit( frame );
        e->addField( 1, i18n("Left") );
        e->addField( 2, i18n("Top") );
        e->addField( 3, i18n("Width") );
        e->addField( 4, i18n("Height") );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else if( type == "QPixmap" )
      {
        QLabel* n = new QLabel( name, frame );
        grid->addWidget( n, i, 0 );
        QLabel* l = new QLabel( "QPixmap", frame );
        grid->addWidget( l, i, 1 );
        KURLRequester* e = new KURLRequester( frame );
        grid->addWidget( e, i, 2 );
        wl.append( e );
      }
      else
      {
        KMessageBox::sorry(this, i18n("Cannot handle datatype %1").arg(type));
        return;
      }
    }

    if (!wl.isEmpty())
      wl.at(0)->setFocus();

    i++;

    int ret = mydialog.exec();

    if (QDialog::Accepted != ret)
      return;

    // extract the arguments

    i = 0;

    for (QStringList::ConstIterator it = types.begin(); it != types.end(); ++it)
    {
      QString type = *it;

      if ( type == "int" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text().toInt();
      }
      else if ( type == "unsigned" || type == "uint" || type == "unsigned int"
             || type == "Q_UINT32" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text().toUInt();
      }
      else if( type == "long" || type == "long int" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text().toLong();
      }
      else if( type == "ulong" || type == "unsigned long" || type == "unsigned long int" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text().toULong();
      }
      else if( type == "short" || type == "short int" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text().toShort();
      }
      else if( type == "ushort" || type == "unsigned short" || type == "unsigned short int" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text().toUShort();
      }
      else if ( type == "Q_UINT64" )
      {
        KLineEdit* e = ( KLineEdit* )wl.at( i );
        arg << e->text().toULongLong();
      }
      else if( type == "float" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text().toFloat();
      }
      else if( type == "double" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text().toDouble();
      }
      else if( type == "bool" )
      {
        QCheckBox* c = (QCheckBox*)wl.at( i );
        arg << c->isChecked();
      }
      else if( type == "QCString" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << QCString( e->text().local8Bit() );
      }
      else if( type == "QString" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << e->text();
      }
      else if( type == "QStringList" )
      {
        KEditListBox* e = (KEditListBox*)wl.at( i );
        arg << e->items();
      }
      else if( type == "QValueList<QCString>" )
      {
        KEditListBox* e = (KEditListBox*)wl.at( i );
        for (int i = 0; i < e->count(); i++)
          arg << QCString( e->text(i).local8Bit() );
      }
      else if( type == "KURL" )
      {
        KLineEdit* e = (KLineEdit*)wl.at( i );
        arg << KURL( e->text() );
      }
      else if( type == "QColor" )
      {
       KColorButton* e = (KColorButton*)wl.at( i );
       arg << e->color();
      }
      else if( type == "QSize" )
      {
       KMultiIntEdit* e = (KMultiIntEdit*)wl.at( i );
       arg << QSize(e->field(1) , e->field(2)) ;
      }
      else if( type == "QPoint" )
      {
       KMultiIntEdit* e = (KMultiIntEdit*)wl.at( i );
       arg << QPoint(e->field(1) , e->field(2)) ;
      }
      else if( type == "QRect" )
      {
       KMultiIntEdit* e = (KMultiIntEdit*)wl.at( i );
       arg << QRect(e->field(1) , e->field(2) , e->field(3) , e->field(4)) ;
      }
      else if( type == "QPixmap" )
      {
       KURLRequester* e= (KURLRequester*)wl.at( i );
       arg << QPixmap(e->url());
      }
      else
      {
        KMessageBox::sorry(this, i18n("Cannot handle datatype %1").arg(type));
        return;
      }

      i++;
    }
  }

  DCOPRef( fitem->app(), "MainApplication-Interface" ).call( "updateUserTimestamp", kapp->userTimestamp());

  // Now do the DCOP call

  bool callOk =
    dcopClient->call
    (
     fitem->app(),
     fitem->object(),
     normalisedSignature.utf8(),
     data,
     replyType,
     replyData
    );

  if (!callOk)
  {
    kdDebug()
      << "call failed( "
      << fitem->app().data()
      << ", "
      << fitem->object().data()
      << ", "
      << normalisedSignature
      << " )"
      << endl;

    statusBar()->message(i18n("DCOP call failed"));

    QString msg = i18n("<p>DCOP call failed.</p>%1");

    bool appRegistered = dcopClient->isApplicationRegistered(fitem->app());

    if (appRegistered)
    {
      msg =
        msg.arg
        (
         i18n
         (
          "<p>Application is still registered with DCOP;"
          " I do not know why this call failed.</p>"
         )
        );
    }
    else
    {
      msg =
        msg.arg
        (
         i18n
         (
          "<p>The application appears to have unregistered with DCOP.</p>"
         )
        );
    }

    KMessageBox::information(this, msg);
  }
  else
  {
    QString coolSignature =
      QString::fromUtf8(fitem->app())
      + "."
      + QString::fromUtf8(fitem->object())
      + "."
      + normalisedSignature ;

    statusBar()->message(i18n("DCOP call %1 executed").arg(coolSignature));

    if (replyType != "void" && replyType != "ASYNC" && !replyType.isEmpty() )
    {
      QDataStream reply(replyData, IO_ReadOnly);
      if (demarshal(replyType, reply, mainView->lb_replyData))
	{
      mainView->l_replyType->setText
        (
         i18n("<strong>%1</strong>")
         .arg(QString::fromUtf8(replyType))
        );
	mainView->lb_replyData->show();
	}
	else
	{
	        mainView->l_replyType->setText(i18n("Unknown type %1.").arg(QString::fromUtf8(replyType)));
      		mainView->lb_replyData->hide();
	}
    }
    else
    {
      mainView->l_replyType->setText(i18n("No returned values"));
      mainView->lb_replyData->hide();
    }
  }
}


void KDCOPWindow::slotFillApplications()
{
  KApplication::setOverrideCursor(waitCursor);

  QCStringList apps = dcopClient->registeredApplications();
  QCString appId = dcopClient->appId();

  mainView->lv->clear();

  for (QCStringList::ConstIterator it = apps.begin(); it != apps.end(); ++it)
  {
    if ((*it) != appId && (*it).left(9) != "anonymous")
    {
      new DCOPBrowserApplicationItem(mainView->lv, *it);
    }
  }

  KApplication::restoreOverrideCursor();
}

bool KDCOPWindow::demarshal
(
 QCString &   replyType,
 QDataStream & reply,
 QListBox	*theList
)
{
  QStringList ret;
  QPixmap pret;
  bool isValid = true;
  theList->clear();
  ret.clear();

  if ( replyType == "QVariant" )
  {
    // read data type from stream
    Q_INT32 type;
    reply >> type;

    // change replyType to real typename
    replyType = QVariant::typeToName( (QVariant::Type)type );

    // demarshal data with a recursive call
    return demarshal(replyType, reply, theList);
  }
  else if ( replyType == "int" )
  {
    int i;
    reply >> i;
    ret << QString::number(i);
  }
  else if ( replyType == "uint" || replyType == "unsigned int"
         || replyType == "Q_UINT32" )
  {
    uint i;
    reply >> i;
    ret << QString::number(i);
  }
  else if ( replyType == "long" || replyType == "long int" )
  {
    long l;
    reply >> l;
    ret << QString::number(l);
  }
  else if ( replyType == "ulong" || replyType == "unsigned long" || replyType == "unsigned long int" )
  {
    ulong l;
    reply >> l;
    ret << QString::number(l);
  }
  else if ( replyType == "Q_UINT64" )
  {
    Q_UINT64 i;
    reply >> i;
    ret << QString::number(i);
  }
  else if ( replyType == "float" )
  {
    float f;
    reply >> f;
    ret << QString::number(f);
  }
  else if ( replyType == "double" )
  {
    double d;
    reply >> d;
    ret << QString::number(d);
  }
  else if (replyType == "bool")
  {
    bool b;
    reply >> b;
    ret << (b ? QString::fromUtf8("true") : QString::fromUtf8("false"));
  }
  else if (replyType == "QString")
  {
    QString s;
    reply >> s;
    ret << s;
  }
  else if (replyType == "QStringList")
  {
    reply >> ret;
  }
  else if (replyType == "QCString")
  {
    QCString r;
    reply >> r;
    ret << QString::fromUtf8(r);
  }
  else if (replyType == "QCStringList")
  {
    QCStringList lst;
    reply >> lst;

    for (QCStringList::ConstIterator it(lst.begin()); it != lst.end(); ++it)
      ret << *it;
  }
  else if (replyType == "KURL")
  {
    KURL r;
    reply >> r;
    ret << r.prettyURL();
  }
  else if (replyType == "QSize")
  {
    QSize r;
    reply >> r;
    ret << QString::number(r.width()) + "x" + QString::number(r.height());
  }
  else if (replyType == "QPoint")
  {
    QPoint r;
    reply >> r;
    ret << "(" + QString::number(r.x()) + "," + QString::number(r.y()) + ")";
  }
  else if (replyType == "QRect")
  {
    QRect r;
    reply >> r;
    ret << QString::number(r.x()) + "x" + QString::number(r.y()) + "+" + QString::number(r.height()) + "+" + QString::number(r.width());
  }
  else if (replyType == "QFont")
  {
	QFont r;
	reply >> r;
	ret << r.rawName();
  }
  else if (replyType == "QCursor")
  {
	QCursor r;
	reply >> r;
	//theList->insertItem(r, 1);
	ret << "Cursor #" + QString::number(r.shape());
  }
  else if (replyType == "QPixmap")
  {
	QPixmap r;
	reply >> r;
	theList->insertItem(r, 1);
  }
  else if (replyType == "QColor")
  {
	QColor r;
	reply >> r;
	QString color = r.name();
	QPixmap p(15,15);
	p.fill(r);
	theList->insertItem(p,color, 1);
  }
  else if (replyType == "QDateTime")
  {
  	QDateTime r;
	reply >> r;
	ret << r.toString();
  }
  else if (replyType == "QDate")
  {
  	QDate r;
	reply >> r;
	ret << r.toString();
  }
  else if (replyType == "QTime")
  {
  	QTime r;
	reply >> r;
	ret << r.toString();
  }
  else if (replyType == "DCOPRef")
  {
	DCOPRef r;
	reply >> r;
	if (!r.app().isEmpty() && !r.obj().isEmpty())
	  ret << QString("DCOPRef(%1, %2)").arg(r.app(), r.obj());
  }
  else
  {
    ret <<
      i18n("Do not know how to demarshal %1").arg(QString::fromUtf8(replyType));
	isValid = false;
  }

      if (!ret.isEmpty())
      	theList->insertStringList(ret);
	return isValid;
}

  void
KDCOPWindow::slotApplicationRegistered(const QCString & appName)
{
  QListViewItemIterator it(mainView->lv);

  for (; it.current(); ++it)
  {
    DCOPBrowserApplicationItem * item =
      static_cast<DCOPBrowserApplicationItem *>(it.current());

    if (item->app() == appName)
      return;
  }

  QCString appId = dcopClient->appId();

  if (appName != appId && appName.left(9) != "anonymous")
  {
    new DCOPBrowserApplicationItem(mainView->lv, appName);
  }
}

  void
KDCOPWindow::slotApplicationUnregistered(const QCString & appName)
{
  QListViewItemIterator it(mainView->lv);

  for (; it.current(); ++it)
  {
    DCOPBrowserApplicationItem * item =
      static_cast<DCOPBrowserApplicationItem *>(it.current());

    if (item->app() == appName)
    {
      delete item;
      return;
    }
  }
}

  bool
KDCOPWindow::getParameters
(
 const QString  & _unNormalisedSignature,
 QString        & normalisedSignature,
 QStringList    & types,
 QStringList    & names
)
{
  QString unNormalisedSignature(_unNormalisedSignature);

  int s = unNormalisedSignature.find(' ');

  if ( s < 0 )
    s = 0;
  else
    s++;

  unNormalisedSignature = unNormalisedSignature.mid(s);

  int left  = unNormalisedSignature.find('(');
  int right = unNormalisedSignature.findRev(')');

  if (-1 == left)
  {
    // Fucked up function signature.
    return false;
  }

  QStringList intTypes;
  intTypes << "int" << "unsigned" << "long" << "bool" ;

  if (left > 0 && left + 1 < right - 1)
  {
    types =
      QStringList::split
      (',', unNormalisedSignature.mid(left + 1, right - left - 1));

    for (QStringList::Iterator it = types.begin(); it != types.end(); ++it)
    {
      (*it) = (*it).simplifyWhiteSpace();

      int s = (*it).findRev(' ');

      if (-1 != s && !intTypes.contains((*it).mid(s + 1)))
      {
        names.append((*it).mid(s + 1));

        (*it) = (*it).left(s);
      }
    }
  }

  normalisedSignature =
    unNormalisedSignature.left(left) + "(" + types.join(",") + ")";

  return true;
}
void KDCOPWindow::slotCopy()
{
	// Copy pixmap and text to the clipboard from the
	// below list view.  If there is nothing selected from
	// the below menu then tell the tree to copy its current
	// selection as text.
	QClipboard *clipboard = QApplication::clipboard();
	if (mainView->lb_replyData->count()!= 0)
	{

		//if (!mainView->lb_replyData->pixmap(mainView->lb_replyData->currentItem())->isNull())
		//{
			kdDebug() << "Is pixmap" << endl;
		//	QPixmap p;
		// 	p = *mainView->lb_replyData->pixmap(mainView->lb_replyData->currentItem());
		//	clipboard->setPixmap(p);
		//}
		QString t = mainView->lb_replyData->text(mainView->lb_replyData->currentItem());
		if (!t.isNull())
			clipboard->setText(t);
	}
}

void KDCOPWindow::slotMode()
{
	kdDebug () << "Going to mode " << langmode->currentText() << endl;
	// Tell lv what the current mode is here...
	mainView->lv->setMode(langmode->currentText());
}

#include "kdcopwindow.moc"
