/*
    KAppfinder, the KDE application finder

    Copyright (c) 2002-2003 Tobias Koenig <tokoe@kde.org>

    Based on code written by Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <kaccelmanager.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kservice.h>
#include <kservicegroup.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <kstartupinfo.h>

#include <qaccel.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qregexp.h>

#include "toplevel.h"

TopLevel::TopLevel( const QString &destDir, QWidget *parent, const char *name )
  : KDialog( parent, name, true )
{
  setCaption( i18n( "KAppfinder" ) );
  QVBoxLayout *layout = new QVBoxLayout( this, marginHint() );

  QLabel *label = new QLabel( i18n( "The application finder looks for non-KDE "
                                    "applications on your system and adds "
                                    "them to the KDE menu system. "
                                    "Click 'Scan' to begin, select the desired applications and then click 'Apply'."), this);
  label->setAlignment( AlignAuto | WordBreak );
  layout->addWidget( label );

  layout->addSpacing( 5 );

  mListView = new QListView( this );
  mListView->addColumn( i18n( "Application" ) );
  mListView->addColumn( i18n( "Description" ) );
  mListView->addColumn( i18n( "Command" ) );
  mListView->setMinimumSize( 300, 300 );
  mListView->setRootIsDecorated( true );
  mListView->setAllColumnsShowFocus( true );
  mListView->setSelectionMode(QListView::NoSelection);
  layout->addWidget( mListView );

  mProgress = new KProgress( this );
  mProgress->setPercentageVisible( false );
  layout->addWidget( mProgress );

  mSummary = new QLabel( i18n( "Summary:" ), this );
  layout->addWidget( mSummary );

  KButtonBox* bbox = new KButtonBox( this );
  mScanButton = bbox->addButton( KGuiItem( i18n( "Scan" ), "find"), this, SLOT( slotScan() ) );
  bbox->addStretch( 5 );
  mSelectButton = bbox->addButton( i18n( "Select All" ), this,
                                   SLOT( slotSelectAll() ) );
  mSelectButton->setEnabled( false );
  mUnSelectButton = bbox->addButton( i18n( "Unselect All" ), this,
                                     SLOT( slotUnselectAll() ) );
  mUnSelectButton->setEnabled( false );
  bbox->addStretch( 5 );
  mApplyButton = bbox->addButton( KStdGuiItem::apply(), this, SLOT( slotCreate() ) );
  mApplyButton->setEnabled( false );
  bbox->addButton( KStdGuiItem::close(), kapp, SLOT( quit() ) );
  bbox->layout();

  layout->addWidget( bbox );

	connect( kapp, SIGNAL( lastWindowClosed() ), kapp, SLOT( quit() ) );

  mAppCache.setAutoDelete( true );

  adjustSize();

  mDestDir = destDir;
  mDestDir = mDestDir.replace( QRegExp( "^~/" ), QDir::homeDirPath() + "/" );
	
  KStartupInfo::appStarted();

  QAccel *accel = new QAccel( this );
  accel->connectItem( accel->insertItem( Key_Q + CTRL ), kapp, SLOT( quit() ) );

  KAcceleratorManager::manage( this );
}


TopLevel::~TopLevel()
{
  mAppCache.clear();
}

QListViewItem* TopLevel::addGroupItem( QListViewItem *parent, const QString &relPath,
                                       const QString &name )
{
  KServiceGroup::Ptr root = KServiceGroup::group( relPath );
  if( !root )
		  return 0L;
  KServiceGroup::List list = root->entries();

  KServiceGroup::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    KSycocaEntry *p = (*it);
    if ( p->isType( KST_KServiceGroup ) ) {
      KServiceGroup* serviceGroup = static_cast<KServiceGroup*>( p );
      if ( QString( "%1%2/" ).arg( relPath ).arg( name ) == serviceGroup->relPath() ) {
        QListViewItem* retval;
        if ( parent )
          retval = parent->firstChild();
        else
          retval = mListView->firstChild();

        while ( retval ) {
          if ( retval->text( 0 ) == serviceGroup->caption() )
            return retval;

          retval = retval->nextSibling();
        }

        QListViewItem *item;
        if ( parent )
          item = new QListViewItem( parent, serviceGroup->caption() );
        else
          item = new QListViewItem( mListView, serviceGroup->caption() );

        item->setPixmap( 0, SmallIcon( serviceGroup->icon() ) );
        item->setOpen( true );
        return item;
      }
    }
  }

  return 0;
}

void TopLevel::slotScan()
{
  KIconLoader* loader = KGlobal::iconLoader();

  mTemplates = KGlobal::dirs()->findAllResources( "data", "kappfinder/apps/*.desktop", true );

  mAppCache.clear();

  mFound = 0;
  int count = mTemplates.count();

  mScanButton->setEnabled( false );
  mProgress->setPercentageVisible( true );
  mProgress->setTotalSteps( count );
  mProgress->setValue( 0 );

  mListView->clear();

  QStringList::Iterator it;
  for ( it = mTemplates.begin(); it != mTemplates.end(); ++it ) {
    // eye candy
    mProgress->setProgress( mProgress->progress() + 1 );

    QString desktopName = *it;
    int i = desktopName.findRev('/');
    desktopName = desktopName.mid(i+1);
    i = desktopName.findRev('.');
    if (i != -1)
       desktopName = desktopName.left(i);

    bool found;
    found = KService::serviceByDesktopName(desktopName);
    if (found)
       continue;

    found = KService::serviceByMenuId("kde-"+desktopName+".desktop");
    if (found)
       continue; 

    found = KService::serviceByMenuId("gnome-"+desktopName+".desktop");
    if (found)
       continue; 

    KDesktopFile desktop( *it, true );

    // copy over the desktop file, if exists
    if ( scanDesktopFile( mAppCache, *it, mDestDir ) ) {
      QString relPath = *it;
      int pos = relPath.find( "kappfinder/apps/" );
      relPath = relPath.mid( pos + strlen( "kappfinder/apps/" ) );
      relPath = relPath.left( relPath.findRev( '/' ) + 1 );
      QStringList dirList = QStringList::split( '/', relPath );

      QListViewItem *dirItem = 0;
      QString tmpRelPath = QString::null;

      QStringList::Iterator tmpIt;
      for ( tmpIt = dirList.begin(); tmpIt != dirList.end(); ++tmpIt ) {
        dirItem = addGroupItem( dirItem, tmpRelPath, *tmpIt );
        tmpRelPath += *tmpIt + '/';
      }

      mFound++;

      QCheckListItem *item;
      if ( dirItem )
        item = new QCheckListItem( dirItem, desktop.readName(), QCheckListItem::CheckBox );
      else
        item = new QCheckListItem( mListView, desktop.readName(), QCheckListItem::CheckBox );

      item->setPixmap( 0, loader->loadIcon( desktop.readIcon(), KIcon::Small ) );
      item->setText( 1, desktop.readGenericName() );
      item->setText( 2, desktop.readPathEntry( "Exec" ) );
      if ( desktop.readBoolEntry( "X-StandardInstall" ) )
        item->setOn( true );

      AppLnkCache* cache = mAppCache.last();
      if ( cache )
        cache->item = item;
    }

    // update summary
    QString sum( i18n( "Summary: found %n application",
                       "Summary: found %n applications", mFound ) );
    mSummary->setText( sum );
  }

  // stop scanning
  mProgress->setValue( 0 );
  mProgress->setPercentageVisible( false );

  mScanButton->setEnabled( true );

  if ( mFound > 0 ) {
    mApplyButton->setEnabled( true );
    mSelectButton->setEnabled( true );
    mUnSelectButton->setEnabled( true );
  }
}

void TopLevel::slotSelectAll()
{
  AppLnkCache* cache;
  for ( cache = mAppCache.first(); cache; cache = mAppCache.next() )
    cache->item->setOn( true );
}

void TopLevel::slotUnselectAll()
{
  AppLnkCache* cache;
  for ( cache = mAppCache.first(); cache; cache = mAppCache.next() )
    cache->item->setOn( false );
}

void TopLevel::slotCreate()
{
  // copy template files
  mAdded = 0;
  createDesktopFiles( mAppCache, mAdded );

  // decorate directories
  decorateDirs( mDestDir );

  KService::rebuildKSycoca(this);

  QString message( i18n( "%n application was added to the KDE menu system.",
                         "%n applications were added to the KDE menu system.", mAdded ) );
  KMessageBox::information( this, message, QString::null, "ShowInformation" );
}

#include "toplevel.moc"
