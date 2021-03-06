/*
 *  This file is part of the KDE Help Center
 *
 *  Copyright (C) 1999 Matthias Elter (me@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __navigator_h__
#define __navigator_h__

#include "glossary.h"

#include <klistview.h>
#include <kurl.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qptrlist.h>
#include <qtabwidget.h>
#include <qlistview.h>
#include <qdict.h>

class QPushButton;

class KListView;
class KService;
class KProcess;
class KProcIO;

class KCMHelpCenter;

namespace KHC {

class NavigatorItem;
class Navigator;
class View;
class SearchEngine;
class SearchWidget;
class Formatter;

class Navigator : public QWidget {
    Q_OBJECT
public:
    Navigator(View *, QWidget *parent = 0, const char *name = 0);
    virtual ~Navigator();

    KURL homeURL();

    SearchEngine *searchEngine() const;
    Formatter *formatter() const;

    const GlossaryEntry &glossEntry(const QString &term) const
    {
        return mGlossaryTree->entry(term);
    }

    void insertParentAppDocs(const QString &name, NavigatorItem *parent);
    void insertAppletDocs(NavigatorItem *parent);
    NavigatorItem *insertScrollKeeperDocs(NavigatorItem *parentItem, NavigatorItem *after);
    void insertInfoDocs(NavigatorItem *parentItem);
    void insertIOSlaveDocs(const QString &, NavigatorItem *parent);

    void createItemFromDesktopFile(NavigatorItem *item, const QString &name);

    bool showMissingDocs() const;

    void clearSelection();

    void showOverview(NavigatorItem *item, const KURL &url);

    void readConfig();
    void writeConfig();

public slots:
    void openInternalUrl(const KURL &url);
    void slotItemSelected(QListViewItem *index);
    void slotSearch();
    void slotShowSearchResult(const QString &);
    void slotSelectGlossEntry(const QString &id);
    void selectItem(const KURL &url);
    void showIndexDialog();

signals:
    void itemSelected(const QString &itemURL);
    void glossSelected(const GlossaryEntry &entry);

protected slots:
    void slotSearchFinished();
    void slotTabChanged(QWidget *);
    void checkSearchButton();

    bool checkSearchIndex();

    void clearSearch();

protected:
    QString createChildrenList(QListViewItem *child);

private:
    void setupContentsTab();
    void setupIndexTab();
    void setupSearchTab();
    void setupGlossaryTab();

    void insertPlugins();
    void hideSearch();

    KListView *mContentsTree;
    Glossary *mGlossaryTree;

    SearchWidget *mSearchWidget;
    KCMHelpCenter *mIndexDialog;

    QTabWidget *mTabWidget;

    QFrame *mSearchFrame;
    QLineEdit *mSearchEdit;
    QPushButton *mSearchButton;

    QPtrList< NavigatorItem > manualItems, pluginItems;

    bool mShowMissingDocs;

    SearchEngine *mSearchEngine;

    View *mView;

    KURL mHomeUrl;

    bool mSelected;

    KURL mLastUrl;

    int mDirLevel;
};
}

#endif
// vim:ts=2:sw=2:et
