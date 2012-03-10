/* This file is part of the KDE project
   Copyright (C) 2003 Aaron J. Seigo <aseigo@olympusproject.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <iostream>
using namespace std;

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

#include <qdir.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstyle.h>
#include <qfile.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kpixmap.h>
#include <krecentdocument.h>
#include <kservice.h>
#include <kservicegroup.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <ksycocaentry.h>

#include "menuinfo.h"
#include "service_mnu.h"
#include "kicker.h"
#include "tom.h"

const int configureMenuID = 1000;
const int contextMenuTitleID = 10000;
const int destMenuTitleID = 10001;

extern "C"
{
    KDE_EXPORT void* init_kickermenu_tom()
    {
        KGlobal::locale()->insertCatalogue("libkickermenu_tom");
        return new TOMFactory;
    }
};

TOMFactory::TOMFactory(QObject *parent, const char *name)
: KLibFactory(parent, name)
{
}

QObject* TOMFactory::createObject(QObject *parent, const char *name, const char*, const QStringList&)
{
    return new TOM((QWidget*)parent, name);
}

#include <qmenudata.h>
/*
 * TODO: switch the backgroundmode when translucency turns on/off
 * TODO: catch font changes too?
 */
class runMenuWidget : public QWidget, public QMenuItem
{
    public:
        runMenuWidget(KPopupMenu* parent, int index)
            : QWidget (parent),
              m_menu(parent),
              m_index(index)
        {
            setFocusPolicy(StrongFocus);

            QHBoxLayout* runLayout = new QHBoxLayout(this);
            textRect = fontMetrics().boundingRect(i18n("Run:"));
            runLayout->setSpacing(KDialog::spacingHint());
            runLayout->addSpacing((KDialog::spacingHint() * 3) + KIcon::SizeMedium + textRect.width());
            icon = DesktopIcon("run", KIcon::SizeMedium);
            /*QLabel* l1 = new QLabel(this);
            QPixmap foo = DesktopIcon("run", KIcon::SizeMedium);
            cout << "foo is: " << foo.width() << " by " << foo.height() << endl;
            l1->setPixmap(foo);
            runLayout->addWidget(l1);*/
            /*QLabel* l2 = new QLabel(i18n("&Run: "), this);
            l2->setBackgroundMode(Qt::X11ParentRelative, Qt::X11ParentRelative);
            l2->setBuddy(this);
            runLayout->addWidget(l2);*/
            m_runEdit = new KHistoryCombo(this);
            m_runEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            runLayout->addWidget(m_runEdit, 10);
            runLayout->addSpacing(KDialog::spacingHint());

            QSettings settings;
            if (settings.readEntry("/KStyle/Settings/MenuTransparencyEngine", "Disabled") != "Disabled")
            {
                setBackgroundMode(Qt::X11ParentRelative, Qt::X11ParentRelative);
                //l1->setBackgroundMode(Qt::X11ParentRelative, Qt::X11ParentRelative);
                //l2->setBackgroundMode(Qt::X11ParentRelative, Qt::X11ParentRelative);
                m_runEdit->setBackgroundMode(Qt::X11ParentRelative, Qt::X11ParentRelative);
            }
            else
            {
                /*setBackgroundMode(Qt::NoBackground, Qt::NoBackground);
                l1->setBackgroundMode(Qt::NoBackground, Qt::NoBackground);
                l2->setBackgroundMode(Qt::NoBackground, Qt::NoBackground);
                m_runEdit->setBackgroundMode(Qt::NoBackground, Qt::NoBackground);*/
                //l1->setAutoMask(true);
                //l1->setBackgroundMode(Qt::NoBackground, Qt::NoBackground);
                //l2->setBackgroundMode(Qt::X11ParentRelative, Qt::X11ParentRelative);
                //m_runEdit->setBackgroundMode(Qt::X11ParentRelative, Qt::X11ParentRelative);
            }

            setMinimumHeight(KIcon::SizeMedium + 2);
        }
        ~runMenuWidget() {}


        void paintEvent(QPaintEvent * /*e*/)
        {
            QPainter p(this);
            QRect r(rect());
            // ew, nasty hack. may result in coredumps due to horrid C-style cast???
            kapp->style().drawControl(QStyle::CE_PopupMenuItem, &p, m_menu, r, palette().active(), QStyle::Style_Enabled,
                                      QStyleOption(static_cast<QMenuItem*>(this), 0, KIcon::SizeMedium ));
            p.drawPixmap(KDialog::spacingHint(), 1, icon);
            p.drawText((KDialog::spacingHint() * 2) + KIcon::SizeMedium, textRect.height() + ((height() - textRect.height()) / 2), i18n("Run:"));
        }

        void focusInEvent (QFocusEvent* e)
        {
            if (!e || e->gotFocus())
            {
                m_menu->setActiveItem(m_index);
                m_runEdit->setFocus();
            }
        }

        void enterEvent(QEvent*)
        {
            focusInEvent(0);
        }

    private:
        KPopupMenu* m_menu;
        KHistoryCombo* m_runEdit;
        QPixmap icon;
        QRect textRect;
        int m_index;
};

TOM::TOM(QWidget *parent, const char *name)
    : KPanelMenu(parent, name),
      m_maxIndex(0)
{
    disableAutoClear();
    m_submenus.setAutoDelete(true);
    setCaption(i18n("Task-Oriented Menu"));

    // KMenu legacy: support some menu config options
    KConfig* config = KGlobal::config();
    config->setGroup("menus");
    m_detailedTaskEntries = config->readBoolEntry("DetailedMenuEntries", false);
    if (m_detailedTaskEntries)
    {
        m_detailedNamesFirst = config->readBoolEntry("DetailedEntriesNamesFirst", false);
    }

    m_isImmutable = Kicker::kicker()->isImmutable() || config->groupIsImmutable("TOM");
    config->setGroup("TOM");
    m_largerFont = font();
    m_largerFont = config->readFontEntry("Font", &m_largerFont);
}

TOM::~TOM()
{
    slotClear();
}

void TOM::initializeRecentApps(QPopupMenu* menu)
{
    /*
     * TODO: make this real
     *       add a KDE-wide "commands run" registry
     */

    if (!m_isImmutable)
    {
        menu->insertSeparator();
        menu->insertItem(i18n("Configure This Menu"), configureMenuID);
    }
}

void TOM::initializeRecentDocs()
{
    m_recentDocsMenu->clear();
    m_recentDocsMenu->insertItem(SmallIconSet("history_clear"), i18n("Clear History"),
                                 this, SLOT(clearRecentDocHistory()));
    m_recentDocsMenu->insertSeparator();

    m_recentDocURLs = KRecentDocument::recentDocuments();

    if (m_recentDocURLs.isEmpty())
    {
        setItemEnabled(m_recentDocsMenu->insertItem(i18n("No Entries")), false);
        return;
    }

    int id = 0;
    for (QStringList::ConstIterator it = m_recentDocURLs.begin();
         it != m_recentDocURLs.end();
         ++it)
    {
        /*
         * TODO: make the number of visible items configurable?
         */

        KDesktopFile f(*it, true /* read only */);
        m_recentDocsMenu->insertItem(DesktopIcon(f.readIcon(), KIcon::SizeMedium),
                                     f.readName().replace('&', "&&"), id);
        ++id;
    }
}

int TOM::appendTaskGroup(KConfig& config, bool inSubMenu)
{
    if (!config.hasGroup("General"))
    {
        return 0;
    }

    config.setGroup("General");

    if (config.readBoolEntry("Hidden", false))
    {
        return 0;
    }

    QString name = config.readEntry("Name", i18n("Unknown"));
    QString icon = config.readEntry("Icon");
    int numTasks = config.readNumEntry("NumTasks", 0);

    if (numTasks < 1)
    {
        return 0;
    }

    KPopupMenu* taskGroup;
    if( inSubMenu )
    {
        taskGroup = new KPopupMenu(this);

        if (icon != QString::null)
        {
            insertItem(DesktopIcon(icon, KIcon::SizeMedium), name, taskGroup);
        }
        else
        {
            insertItem(name, taskGroup);
        }
    }
    else
    {
        taskGroup = this;
    }

    int foundTasks = 0;
    for (int i = 0; i < numTasks; ++i)
    {
        QString groupName = QString("Task%1").arg(i);

        if (config.hasGroup(groupName))
        {
            config.setGroup(groupName);

            if (config.readBoolEntry("Hidden", false))
            {
                continue;
            }

            QString name = config.readEntry("Name");
            // in case the name contains an ampersand, double 'em up
            name.replace("&", "&&");
            QString desktopfile = config.readPathEntry("DesktopFile");
            KService::Ptr pService = KService::serviceByDesktopPath(desktopfile);

            if (!pService)
            {
                pService = KService::serviceByDesktopName(desktopfile);

                if (!pService)
                {
                    continue;
                }
            }

            QString execName = pService->name();
            QString icon = pService->icon();

            if (m_detailedTaskEntries && !execName.isEmpty())
            {
                QString tmp = i18n("%1 (%2)");

                if (m_detailedNamesFirst)
                {
                    name = tmp.arg(execName).arg(name);
                }
                else
                {
                    name = tmp.arg(name).arg(execName);
                }
            }

            ++m_maxIndex;
            if (icon.isEmpty())
            {
                taskGroup->insertItem(name, m_maxIndex);
            }
            else
            {
       
		QIconSet iconset = BarIconSet(icon, 22);
		if (iconset.isNull())
		    taskGroup->insertItem(name, m_maxIndex);
		else
		    taskGroup->insertItem(iconset, name, m_maxIndex);
	
            }

            m_tasks.insert(m_maxIndex, pService);
            ++foundTasks;
        }
    }

    // if we end up with an empty task group, get rid of the menu
    if (foundTasks == 0)
    {
        if (inSubMenu)
        {
            delete taskGroup;
        }

        return 0;
    }

    connect(taskGroup, SIGNAL(activated(int)), this, SLOT(runTask(int)));
    // so we have an actual task group menu with tasks, let's add it

    if (inSubMenu)
    {
        QObject::connect(taskGroup, SIGNAL(aboutToShowContextMenu(KPopupMenu*, int, QPopupMenu*)),
                         this, SLOT(contextualizeRMBmenu(KPopupMenu*, int, QPopupMenu*)));

        m_submenus.append(taskGroup);

        taskGroup->setFont(m_largerFont);
        taskGroup->setKeyboardShortcutsEnabled(true);

        if (!m_isImmutable && !config.isImmutable())
        {
            taskGroup->insertSeparator();
            taskGroup->insertItem("Modify These Tasks", configureMenuID);
            QPopupMenu* rmbMenu = taskGroup->contextMenu();
            rmbMenu->setFont(m_largerFont);
            KPopupTitle* title = new KPopupTitle();
            title->setText(i18n("%1 Menu Editor").arg(name));
            rmbMenu->insertItem(title, contextMenuTitleID);
            rmbMenu->insertItem(i18n("Add This Task to Panel"));
            rmbMenu->insertItem(i18n("Modify This Task..."));
            rmbMenu->insertItem(i18n("Remove This Task..."), this, SLOT(removeTask()));
            rmbMenu->insertItem(i18n("Insert New Task..."));
        }
    }

    return foundTasks;
}
/*
 * TODO: track configuration file changes.
void TOM::configChanged()
{
    KPanelMenu::configChanged();
    setInitialized(false);
    initialize();
}
*/
void TOM::initialize()
{
    if (initialized())
    {
        return;
    }

    setInitialized(true);
    m_submenus.clear();
    m_tasks.clear();
    clear();

    /* hack to make items larger. should use something better?
    m_largerFont = font();
    if (m_largerFont.pointSize() < 18)
    {
        m_largerFont.setPointSize(18);
    }
    setFont(m_largerFont);*/


    /*if (!loadSidePixmap())
    {
      m_sidePixmap = m_sideTilePixmap = QPixmap();
    }
    else
    {
      connect(kapp, SIGNAL(kdisplayPaletteChanged()), SLOT(paletteChanged()));
    }*/

    // TASKS
    insertTitle(i18n("Tasks"), contextMenuTitleID);

    QStringList dirs = KGlobal::dirs()->findDirs("data", "kicker/tom/");
    QStringList::ConstIterator dIt = dirs.begin();
    QStringList::ConstIterator dEnd = dirs.end();

    for (; dIt != dEnd; ++dIt )
    {
        QDir dir(*dIt);

        QStringList entries = dir.entryList("*rc", QDir::Files);
        QStringList::ConstIterator eIt = entries.begin();
        QStringList::ConstIterator eEnd = entries.end();

        for (; eIt != eEnd; ++eIt )
        {
            KConfig config(*dIt + *eIt);
            appendTaskGroup(config);
        }
    }

    PanelServiceMenu* moreApps = new PanelServiceMenu(QString::null, QString::null, this, "More Applications");
    moreApps->setFont(m_largerFont);
    insertItem(DesktopIcon("misc", KIcon::SizeMedium), i18n("More Applications"), moreApps);
    m_submenus.append(moreApps);

    if (!m_isImmutable)
    {
        insertSeparator();
        // TODO: connect to taskgroup edits
        insertItem("Modify The Above Tasks");
    }

    // DESTINATIONS
    insertTitle(i18n("Destinations"), destMenuTitleID);
    int numDests = 0;
    QString destinationsConfig = locate("appdata", "tom/destinations");

    if (!destinationsConfig.isEmpty() && QFile::exists(destinationsConfig))
    {
        KConfig config(destinationsConfig);
        numDests = appendTaskGroup(config, false);
    }

    if (numDests == 0)
    {
        removeItem(destMenuTitleID);
    }
    else if (kapp->authorize("run_command"))
    {
        insertItem(DesktopIcon("run", KIcon::SizeMedium), i18n("Run Command..."), this, SLOT(runCommand()));
    }

    // RECENTLY USED ITEMS
    insertTitle(i18n("Recently Used Items"), contextMenuTitleID);

    m_recentDocsMenu = new KPopupMenu(this, "recentDocs");
    m_recentDocsMenu->setFont(m_largerFont);
    connect(m_recentDocsMenu, SIGNAL(aboutToShow()), this, SLOT(initializeRecentDocs()));
    connect(m_recentDocsMenu, SIGNAL(activated(int)), this, SLOT(openRecentDocument(int)));
    insertItem(DesktopIcon("document", KIcon::SizeMedium), i18n("Recent Documents"), m_recentDocsMenu);
    m_submenus.append(m_recentDocsMenu);

    KPopupMenu* recentApps = new KPopupMenu(this, "recentApps");
    recentApps->setFont(m_largerFont);
    recentApps->setKeyboardShortcutsEnabled(true);
    initializeRecentApps(recentApps);
    insertItem(i18n("Recent Applications"), recentApps);
    m_submenus.append(recentApps);

    // SPECIAL ITEMS
    insertTitle(i18n("Special Items"), contextMenuTitleID);

    // if we have no destinations, put the run command here
    if (numDests == 0 && kapp->authorize("run_command"))
    {
        insertItem(DesktopIcon("run", KIcon::SizeMedium), i18n("Run Command..."), this, SLOT(runCommand()));
    }


    KConfig* config = KGlobal::config();
    QStringList menu_ext = config->readListEntry("Extensions");
    if (!menu_ext.isEmpty())
    {
        bool needSeparator = false;
        for (QStringList::ConstIterator it = menu_ext.begin(); it != menu_ext.end(); ++it)
        {
            MenuInfo info(*it);
            if (!info.isValid())
            {
                continue;
            }

            KPanelMenu *menu = info.load();
            if (menu)
            {
                ++m_maxIndex;
                insertItem(DesktopIcon(info.icon(), KIcon::SizeMedium), info.name(), menu, m_maxIndex);
                m_submenus.append(menu);
                needSeparator = true;
            }
        }

        if (needSeparator)
        {
            insertSeparator();
        }
    }


    QString username;
    struct passwd *userInfo = getpwuid(getuid());
    if (userInfo)
    {
        username = QString::fromLocal8Bit(userInfo->pw_gecos);
        if (username.find(',') != -1)
        {
            // Remove everything from and including first comma
            username.truncate(username.find(','));
        }

        if (username.isEmpty())
        {
            username = QString::fromLocal8Bit(userInfo->pw_name);
        }
    }

    insertItem(DesktopIcon("exit", KIcon::SizeMedium),
               i18n("Logout %1").arg(username), this, SLOT(logout()));
}

void TOM::reload()
{
    setInitialized(false);
    initialize();
}

void TOM::contextualizeRMBmenu(KPopupMenu* menu, int menuItem, QPopupMenu* ctxMenu)
{
    if (menuItem == configureMenuID)
    {
        menu->hideContextMenu();
        return;
    }

    ctxMenu->removeItem(contextMenuTitleID);
    QString text = menu->text(menuItem);
    int parens = text.find('(') - 1;
    if (parens > 0)
    {
        text = text.left(parens);
    }
    KPopupTitle* title = new KPopupTitle();
    title->setText(i18n("The \"%2\" Task").arg(text));
    ctxMenu->insertItem(title, contextMenuTitleID, 0);
}

void TOM::slotClear()
{
    KPanelMenu::slotClear();
    m_submenus.clear();
    m_tasks.clear();
}

void TOM::slotExec(int /* id */)
{
    // Reimplemented because we have to
}

void TOM::removeTask()
{
    // TODO: write this change out to the appropriate config file
    QString task = KPopupMenu::contextMenuFocus()->text(KPopupMenu::contextMenuFocusItem());
    if (KMessageBox::warningContinueCancel(this,
                                  i18n("<qt>Are you sure you want to remove the <strong>%1</strong> task?<p>"
                                        "<em>Tip: You can restore the task after it has been removed by selecting the &quot;Modify These Tasks&quot; entry</em></qt>").arg(task),
                                  i18n("Remove Task?"),KStdGuiItem::del()) == KMessageBox::Continue)
    {
        m_tasks.remove(KPopupMenu::contextMenuFocusItem());
        KPopupMenu::contextMenuFocus()->removeItem(KPopupMenu::contextMenuFocusItem());
    }
}

// TODO: should we have a side graphic like the K Menu? personally, i don't
// think it helps usability. it's nice for branding, but that's it. perhaps a
// top image, or something blended with the actual menu background? deffinitely
// a job for a graphic artist.
/*
 * if this goes in, it should be shared w/the kmenu
 *
bool TOM::loadSidePixmap()
{
  KConfig *config = KGlobal::config();
  QColor color = palette().active().highlight();
  QImage image;

  config->setGroup("WM");
  QColor activeTitle = config->readColorEntry("activeBackground", &color);
  QColor inactiveTitle = config->readColorEntry("inactiveBackground", &color);

  config->setGroup("KMenu");
  if (!config->readBoolEntry("Usem_sidePixmap", true))
      return false;

  // figure out which color is most suitable for recoloring to
  int h1, s1, v1, h2, s2, v2, h3, s3, v3;
  activeTitle.hsv(&h1, &s1, &v1);
  inactiveTitle.hsv(&h2, &s2, &v2);
  palette().active().background().hsv(&h3, &s3, &v3);

  if ( (abs(h1-h3)+abs(s1-s3)+abs(v1-v3) < abs(h2-h3)+abs(s2-s3)+abs(v2-v3)) &&
       ((abs(h1-h3)+abs(s1-s3)+abs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
    color = inactiveTitle;
  else
    color = activeTitle;

  // limit max/min brightness
  int r, g, b;
  color.rgb(&r, &g, &b);
  int gray = qGray(r, g, b);
  if (gray > 180) {
    r = (r - (gray - 180) < 0 ? 0 : r - (gray - 180));
    g = (g - (gray - 180) < 0 ? 0 : g - (gray - 180));
    b = (b - (gray - 180) < 0 ? 0 : b - (gray - 180));
  } else if (gray < 76) {
    r = (r + (76 - gray) > 255 ? 255 : r + (76 - gray));
    g = (g + (76 - gray) > 255 ? 255 : g + (76 - gray));
    b = (b + (76 - gray) > 255 ? 255 : b + (76 - gray));
  }
  color.setRgb(r, g, b);

  QString sideName = config->readEntry("SideName", "kside.png");
  QString sideTileName = config->readEntry("SideTileName", "kside_tile.png");

  image.load(locate("data", "kicker/pics/" + sideName));

  if (image.isNull())
  {
    return false;
  }

  KIconEffect::colorize(image, color, 1.0);
  m_sidePixmap.convertFromImage(image);

  image.load(locate("data", "kicker/pics/" + sideTileName));

  if (image.isNull())
  {
    return false;
  }

  KIconEffect::colorize(image, color, 1.0);
  m_sideTilePixmap.convertFromImage(image);

  if (m_sidePixmap.width() != m_sideTilePixmap.width())
  {
    return false;
  }

  // pretile the pixmap to a height of at least 100 pixels
  if (m_sideTilePixmap.height() < 100)
  {
    int tiles = (int)(100 / m_sideTilePixmap.height()) + 1;
    QPixmap preTiledPixmap(m_sideTilePixmap.width(), m_sideTilePixmap.height() * tiles);
    QPainter p(&preTiledPixmap);
    p.drawTiledPixmap(preTiledPixmap.rect(), m_sideTilePixmap);
    m_sideTilePixmap = preTiledPixmap;
  }

  return true;
}

void TOM::paletteChanged()
{
    if (!loadSidePixmap())
        m_sidePixmap = m_sideTilePixmap = QPixmap();
}

void TOM::setMinimumSize(const QSize & s)
{
    KPanelMenu::setMinimumSize(s.width() + m_sidePixmap.width(), s.height());
}

void TOM::setMaximumSize(const QSize & s)
{
    KPanelMenu::setMaximumSize(s.width() + m_sidePixmap.width(), s.height());
}

void TOM::setMinimumSize(int w, int h)
{
    KPanelMenu::setMinimumSize(w + m_sidePixmap.width(), h);
}

void TOM::setMaximumSize(int w, int h)
{
    KPanelMenu::setMaximumSize(w + m_sidePixmap.width(), h);
}

QRect TOM::sideImageRect()
{
    return QStyle::visualRect( QRect( frameWidth(), frameWidth(), m_sidePixmap.width(),
                                      height() - 2*frameWidth() ), this );
}

void TOM::resizeEvent(QResizeEvent * e)
{
    setFrameRect( QStyle::visualRect( QRect( m_sidePixmap.width(), 0,
                                      width() - m_sidePixmap.width(), height() ), this ) );
}

void TOM::paintEvent(QPaintEvent * e)
{
    if (m_sidePixmap.isNull()) {
        KPanelMenu::paintEvent(e);
        return;
    }

    QPainter p(this);

    style().drawPrimitive( QStyle::PE_PanelPopup, &p,
                           QRect( 0, 0, width(), height() ),
                           colorGroup(), QStyle::Style_Default,
                           QStyleOption( frameWidth(), 0 ) );

    QRect r = sideImageRect();
    r.setBottom( r.bottom() - m_sidePixmap.height() );
    p.drawTiledPixmap( r, m_sideTilePixmap );

    r = sideImageRect();
    r.setTop( r.bottom() - m_sidePixmap.height() );
    p.drawPixmap( r, m_sidePixmap );

    drawContents( &p );
}

QMouseEvent TOM::translateMouseEvent( QMouseEvent* e )
{
    QRect side = sideImageRect();

    if ( !side.contains( e->pos() ) )
        return *e;

    QPoint newpos( e->pos() );
    QApplication::reverseLayout() ?
        newpos.setX( newpos.x() - side.width() ) :
        newpos.setX( newpos.x() + side.width() );
    QPoint newglobal( e->globalPos() );
    QApplication::reverseLayout() ?
        newglobal.setX( newpos.x() - side.width() ) :
        newglobal.setX( newpos.x() + side.width() );

    return QMouseEvent( e->type(), newpos, newglobal, e->button(), e->state() );
}

void TOM::mousePressEvent(QMouseEvent * e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    KPanelMenu::mousePressEvent( &newEvent );
}

void TOM::mouseReleaseEvent(QMouseEvent *e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    KPanelMenu::mouseReleaseEvent( &newEvent );
}

void TOM::mouseMoveEvent(QMouseEvent *e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    KPanelMenu::mouseMoveEvent( &newEvent );
}*/

extern int kicker_screen_number;

void TOM::runCommand()
{
    QByteArray data;
    QCString appname( "kdesktop" );
    if ( kicker_screen_number )
        appname.sprintf("kdesktop-screen-%d", kicker_screen_number);

    kapp->updateRemoteUserTimestamp( appname );
    kapp->dcopClient()->send( appname, "KDesktopIface",
                              "popupExecuteCommand()", data );
}

void TOM::runTask(int id)
{
    if (!m_tasks.contains(id)) return;

    kapp->propagateSessionManager();
    KApplication::startServiceByDesktopPath(m_tasks[id]->desktopEntryPath(),
      QStringList(), 0, 0, 0, "", true);
}

void TOM::clearRecentDocHistory()
{
    KRecentDocument::clear();
}

void TOM::openRecentDocument(int id)
{
    if (id >= 0)
    {
        kapp->propagateSessionManager();
        KURL u;
        u.setPath(m_recentDocURLs[id]);
        KDEDesktopMimeType::run(u, true);
    }
}

void TOM::logout()
{
    kapp->requestShutDown();
}

#include "tom.moc"
