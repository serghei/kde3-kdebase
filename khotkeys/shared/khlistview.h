/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2002 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef _KHLISTVIEW_H_
#define _KHLISTVIEW_H_

#include <qtimer.h>

#include <klistview.h>
#include <kdemacros.h>

namespace KHotKeys {

class KDE_EXPORT KHListView : public KListView {
    Q_OBJECT
    Q_PROPERTY(bool forceSelect READ forceSelect WRITE setForceSelect)
public:
    KHListView(QWidget *parent_P, const char *name_P = NULL);
    virtual void clear();
    virtual void insertItem(QListViewItem *item_P);
    virtual void clearSelection();
    bool forceSelect() const;
    void setForceSelect(bool force_P);
signals:
    void current_changed(QListViewItem *item_P);

protected:
    virtual void contentsDropEvent(QDropEvent *);
private slots:
    void slot_selection_changed(QListViewItem *item_P);
    void slot_selection_changed();
    void slot_current_changed(QListViewItem *item_P);
    void slot_insert_select();

private:
    QListViewItem *saved_current_item;
    bool in_clear;
    bool ignore;
    bool force_select;
    QTimer insert_select_timer;
};

//***************************************************************************
// Inline
//***************************************************************************

inline void KHListView::setForceSelect(bool force_P)
{
    force_select = force_P;
}

inline bool KHListView::forceSelect() const
{
    return force_select;
}

} // namespace KHotKeys

#endif
