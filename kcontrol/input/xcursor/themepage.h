/*
 * Copyright (C) 2003 Fredrik H�glund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __THEMEPAGE_H
#define __THEMEPAGE_H

#include <qdict.h>


class KListView;
class QString;
class PreviewWidget;
class QStringList;
class QListViewItem;
class QPushButton;

struct ThemeInfo;


class ThemePage : public QWidget {
    Q_OBJECT

public:
    ThemePage(QWidget *parent = 0, const char *name = 0);
    ~ThemePage();

    // Called by the KCM
    void save();
    void load();
    void load(bool useDefaults);
    void defaults();

signals:
    void changed(bool);

private slots:
    void selectionChanged(QListViewItem *);
    void installClicked();
    void removeClicked();

private:
    bool installThemes(const QString &file);
    void insertTheme(const QString &);
    const QStringList getThemeBaseDirs() const;
    bool isCursorTheme(const QString &theme, const int depth = 0) const;
    void insertThemes();
    QPixmap createIcon(const QString &, const QString &) const;

    KListView *listview;
    PreviewWidget *preview;
    QPushButton *installButton, *removeButton;
    QString selectedTheme;
    QString currentTheme;
    QStringList themeDirs;
    QDict< ThemeInfo > themeInfo;
};

#endif // __THEMEPAGE_H

// vim: set noet ts=4 sw=4:
