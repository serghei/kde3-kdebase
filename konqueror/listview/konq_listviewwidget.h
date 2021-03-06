/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2004 Michael Brade <brade@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __konq_listviewwidget_h__
#define __konq_listviewwidget_h__

#include <qvaluelist.h>
#include <qvaluevector.h>

#include <kurl.h>
#include <kfileitem.h>
#include <klistview.h>
#include <kparts/browserextension.h>
#include <konq_propsview.h>
#include "konq_listviewitems.h"

namespace KIO {
class Job;
}

class QCursor;
class QRect;
class KDirLister;
class KonqFMSettings;
class ListViewPropertiesExtension;
class KToggleAction;
class KonqListView;
class KonqFileTip;
class ListViewBrowserExtension;
class QTimer;
class QFocusEvent;
class QDragMoveEvent;
class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QPaintEvent;
class QResizeEvent;
class QMouseEvent;

class ColumnInfo {
public:
    ColumnInfo();
    void setData(const QString &n, const QString &desktopName, int kioUds, KToggleAction *someAction, int theWith = -1);
    void setData(const QString &n, const QString &desktopName, int kioUds /* UDS_EXTRA */, QVariant::Type type, KToggleAction *someAction,
                 int theWith = -1);
    int displayInColumn;
    QString name;
    QString desktopFileName;
    int udsId;
    QVariant::Type type; // only used if udsId == UDS_EXTRA
    bool displayThisOne;
    KToggleAction *toggleThisOne;
    int width;
};

/**
 * The tree view widget (based on KListView).
 * Most of the functionality is here.
 */
class KonqBaseListViewWidget : public KListView {
    friend class KonqBaseListViewItem;
    friend class KonqListView;
    friend class ListViewBrowserExtension;

    Q_OBJECT
public:
    KonqBaseListViewWidget(KonqListView *parent, QWidget *parentWidget);
    virtual ~KonqBaseListViewWidget();
    unsigned int NumberOfAtoms;

    virtual void stop();
    const KURL &url();

    struct iterator
    {
        KonqBaseListViewItem *m_p;

        iterator() : m_p(0L)
        {
        }
        iterator(KonqBaseListViewItem *_b) : m_p(_b)
        {
        }
        iterator(const iterator &_it) : m_p(_it.m_p)
        {
        }

        KonqBaseListViewItem &operator*()
        {
            return *m_p;
        }
        KonqBaseListViewItem *operator->()
        {
            return m_p;
        }
        bool operator==(const iterator &_it)
        {
            return (m_p == _it.m_p);
        }
        bool operator!=(const iterator &_it)
        {
            return (m_p != _it.m_p);
        }
        iterator &operator++();
        iterator operator++(int);
    };
    iterator begin()
    {
        iterator it((KonqBaseListViewItem *)firstChild());
        return it;
    }
    iterator end()
    {
        iterator it;
        return it;
    }

    virtual bool openURL(const KURL &url);

    void selectedItems(QPtrList< KonqBaseListViewItem > *_list);
    KFileItemList visibleFileItems();
    KFileItemList selectedFileItems();
    KURL::List selectedUrls(bool mostLocal = false);

    /** @return the KonqListViewDir which handles the directory _url */
    // virtual KonqListViewDir *findDir ( const QString & _url );

    /**
     * @return the Properties instance for this view. Used by the items.
     */
    KonqPropsView *props() const;

    // QPtrList<ColumnInfo> *columnConfigInfo() { return &confColumns; };
    QValueVector< ColumnInfo > &columnConfigInfo()
    {
        return confColumns;
    };
    QString sortedByColumn;

    virtual void setShowIcons(bool enable)
    {
        m_showIcons = enable;
    }
    virtual bool showIcons()
    {
        return m_showIcons;
    }

    void setItemFont(const QFont &f)
    {
        m_itemFont = f;
    }
    QFont itemFont() const
    {
        return m_itemFont;
    }
    void setItemColor(const QColor &c)
    {
        m_itemColor = c;
    }
    QColor itemColor() const
    {
        return m_itemColor;
    }
    int iconSize() const
    {
        return props()->iconSize();
    }

    void setAscending(bool b)
    {
        m_bAscending = b;
    }
    bool ascending() const
    {
        return m_bAscending;
    }
    bool caseInsensitiveSort() const;

    virtual void paintEmptyArea(QPainter *p, const QRect &r);

    virtual void saveState(QDataStream &);
    virtual void restoreState(QDataStream &);

    virtual void disableIcons(const KURL::List &lst);

    KonqListView *m_pBrowserView;
    KonqFMSettings *m_pSettings;

signals:
    void viewportAdjusted();

public slots:
    // virtual void slotOnItem( KonqBaseListViewItem* _item );
    // The '2' was added to differentiate it from KListView::slotMouseButtonClicked()
    void slotMouseButtonClicked2(int _button, QListViewItem *_item, const QPoint &pos, int);
    virtual void slotExecuted(QListViewItem *_item);
    void slotItemRenamed(QListViewItem *, const QString &, int);

protected slots:
    void slotAutoScroll();

    // from QListView
    virtual void slotReturnPressed(QListViewItem *_item);
    virtual void slotCurrentChanged(QListViewItem *_item)
    {
        slotOnItem(_item);
    }

    // slots connected to the directory lister
    virtual void slotStarted();
    virtual void slotCompleted();
    virtual void slotCanceled();
    virtual void slotClear();
    virtual void slotNewItems(const KFileItemList &);
    virtual void slotDeleteItem(KFileItem *);
    virtual void slotRefreshItems(const KFileItemList &);
    virtual void slotRedirection(const KURL &);
    void slotPopupMenu(QListViewItem *, const QPoint &, int);

    // forces a repaint on column size changes / branch expansion
    // when there is a background pixmap
    void slotUpdateBackground();

    // Notifies the browser view of the currently selected items
    void slotSelectionChanged();
    virtual void reportItemCounts();

protected:
    // creates the listview columns according to confColumns
    virtual void createColumns();
    // reads the configuration for the columns of the current
    // protocol, it is called when the protocol changes
    // it checks/unchecks the menu items and sets confColumns
    void readProtocolConfig(const KURL &url);
    // calls updateContents of every ListViewItem, called after
    // the columns changed
    void updateListContents();

    // this is called in the constructor, so virtual would be nonsense
    void initConfig();

    virtual void startDrag();
    virtual void viewportDragMoveEvent(QDragMoveEvent *_ev);
    virtual void viewportDragEnterEvent(QDragEnterEvent *_ev);
    virtual void viewportDragLeaveEvent(QDragLeaveEvent *_ev);
    virtual void viewportDropEvent(QDropEvent *_ev);
    virtual void viewportPaintEvent(QPaintEvent *e);
    virtual void viewportResizeEvent(QResizeEvent *e);

    virtual void drawRubber();
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void contentsWheelEvent(QWheelEvent *e);

    virtual void leaveEvent(QEvent *e);

    /** Common method for slotCompleted and slotCanceled */
    virtual void setComplete();

    // the second parameter is set to true when the menu shortcut is pressed,
    // so the position of the mouse pointer doesn't matter when using keyboard, aleXXX
    virtual void popupMenu(const QPoint &_global, bool alwaysForSelectedFiles = false);

    // this one is called only by KListView, and this is friend anyways (Alex)
    // KDirLister *dirLister() const { return m_dirLister; }

protected:
    int executeArea(QListViewItem *_item);

    /** The directory lister for this URL */
    KDirLister *m_dirLister;

    // QPtrList<ColumnInfo> confColumns;
    // IMO there is really no need for an advanced data structure
    // we have a fixed number of members,
    // it consumes less memory and access should be faster (Alex)
    // This might not be the case for ever... we should introduce custom fields in kio (David)
    QValueVector< ColumnInfo > confColumns;

    KonqBaseListViewItem *m_dragOverItem;
    KonqBaseListViewItem *m_activeItem;
    QPtrList< KonqBaseListViewItem > *m_selected;
    QTimer *m_scrollTimer;

    QFont m_itemFont;
    QColor m_itemColor;

    QRect *m_rubber;

    bool m_bTopLevelComplete : 1;
    bool m_showIcons : 1;
    bool m_bCaseInsensitive : 1;
    bool m_bUpdateContentsPosAfterListing : 1;
    bool m_bAscending : 1;
    bool m_itemFound : 1;
    bool m_restored : 1;

    int m_filenameColumn;
    int m_filenameColumnWidth;

    KURL m_url;

    QString m_itemToGoTo;
    QStringList m_itemsToSelect;
    QTimer *m_backgroundTimer;

    KonqFileTip *m_fileTip;
};

#endif
