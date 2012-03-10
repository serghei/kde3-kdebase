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

#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <kdialog.h>

#include <qstringlist.h>

#include "common.h"

class KProgress;
class QLabel;
class QListView;
class QPushButton;

class TopLevel : public KDialog
{
  Q_OBJECT

  public:
    TopLevel( const QString &destDir, QWidget *parent = 0, const char *name = 0 );
    ~TopLevel();

  public slots:
    void slotScan();
    void slotCreate();
    void slotSelectAll();
    void slotUnselectAll();

  private:
    QListViewItem *addGroupItem( QListViewItem *parent, const QString &relPath,
                                 const QString &name );

    KProgress *mProgress;
    QLabel *mSummary;
    QListView *mListView;
    QPushButton *mApplyButton;
    QPushButton *mQuitButton;
    QPushButton *mScanButton;
    QPushButton *mSelectButton;
    QPushButton *mUnSelectButton;
    QString mDestDir;
    QStringList mTemplates;

    QPtrList<AppLnkCache> mAppCache;

    int mFound, mAdded;
};

#endif
