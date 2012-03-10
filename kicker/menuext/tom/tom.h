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

#ifndef __tom_h_
#define __tom_h_

#include <qpixmap.h>

#include <kpanelmenu.h>
#include <klibloader.h>

class KPopupMenu;
class QPopupMenu;

typedef QPtrList<QPopupMenu> PopupMenuList;
typedef QMap<int, KService::Ptr> TaskMap;

class TOM : public KPanelMenu
{
    Q_OBJECT

    public:
        TOM(QWidget *parent = 0, const char *name = 0);
        ~TOM();

        // for the side image
        /*void setMinimumSize(const QSize &);
        void setMaximumSize(const QSize &);
        void setMinimumSize(int, int);
        void setMaximumSize(int, int);  */

    protected slots:
        void slotClear();
        void slotExec(int);
        //void configChanged();
        void initialize();
        void contextualizeRMBmenu(KPopupMenu* menu, int menuItem, QPopupMenu* ctxMenu);
        //void paletteChanged();
        void clearRecentDocHistory();
        void runCommand();
        void runTask(int id);
        void initializeRecentDocs();
        void openRecentDocument(int id);
        void logout();

        /*
         * slots for the RMB menu on task group
         */
        void removeTask();

    protected:
        void reload();

        int  appendTaskGroup(KConfig& config, bool inSubMenu = true );
        void initializeRecentApps(QPopupMenu* menu);
        //int  insertTOMTitle(QPopupMenu* menu, const QString &text, int id = -1, int index = -1);

        /*
         * this stuff should be shared w/the kmenu

        QRect sideImageRect();
        QMouseEvent translateMouseEvent( QMouseEvent* e );
        void resizeEvent(QResizeEvent *);
        void paintEvent(QPaintEvent *);
        void mousePressEvent(QMouseEvent *);
        void mouseReleaseEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *);
        bool loadSidePixmap();

        QPixmap m_sidePixmap;
        QPixmap m_sideTilePixmap;*/
        PopupMenuList m_submenus;
        QFont m_largerFont;
        int m_maxIndex;
        bool m_isImmutable;
        bool m_detailedTaskEntries;
        bool m_detailedNamesFirst;
        TaskMap m_tasks;
        KPopupMenu* m_recentDocsMenu;
        QStringList m_recentDocURLs;
};

class TOMFactory : public KLibFactory
{
    public:
        TOMFactory(QObject *parent = 0, const char *name = 0);

    protected:
        QObject* createObject(QObject *parent = 0, const char *name = 0,
                              const char *classname = "QObject",
                              const QStringList& args = QStringList());
};


#endif
