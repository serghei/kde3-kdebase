/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __KATE_FILELIST_H__
#define __KATE_FILELIST_H__

#include "katemain.h"

#include <kate/document.h>

#include <klistview.h>

#include <qtooltip.h>
#include <qcolor.h>
#include <qptrlist.h>

#define RTTI_KateFileListItem 1001

class KateMainWindow;

class KAction;
class KSelectAction;

class KateFileListItem : public QListViewItem
{
  public:
    KateFileListItem( QListView *lv,
		      Kate::Document *doc );
    ~KateFileListItem();

    inline uint documentNumber () { return m_docNumber; }
    inline Kate::Document * document() { return doc; }

    int rtti() const { return RTTI_KateFileListItem; }

    /**
     * Sets the view history position.
     */
    void setViewHistPos( int p ) {  m_viewhistpos = p; }
    /**
     * Sets the edit history position.
     */
    void setEditHistPos( int p ) { m_edithistpos = p; }

  protected:
    virtual const QPixmap *pixmap ( int column ) const;
    void paintCell( QPainter *painter, const QColorGroup & cg, int column, int width, int align );
    /**
     * Reimplemented so we can sort by a number of different document properties.
     */
    int compare ( QListViewItem * i, int col, bool ascending ) const;

  private:
    Kate::Document *doc;
    int m_viewhistpos; ///< this gets set by the list as needed
    int m_edithistpos; ///< this gets set by the list as needed
    uint m_docNumber;
};

class KateFileList : public KListView
{
  Q_OBJECT

  friend class KFLConfigPage;

  public:
    KateFileList (KateMainWindow *main, KateViewManager *_viewManager, QWidget * parent = 0, const char * name = 0 );
    ~KateFileList ();

    int sortType () const { return m_sort; };
    void updateSort ();

    enum sorting {
      sortByID = 0,
      sortByName = 1,
      sortByURL = 2
    };

    QString tooltip( QListViewItem *item, int );

    uint histCount() const { return m_viewHistory.count(); }
    uint editHistCount() const { return m_editHistory.count(); }
    QColor editShade() const { return m_editShade; }
    QColor viewShade() const { return m_viewShade; }
    bool shadingEnabled() { return m_enableBgShading; }

    void readConfig( class KConfig *config, const QString &group );
    void writeConfig( class KConfig *config, const QString &group );

    /**
     * reimplemented to remove the item from the history stacks
     */
    void takeItem( QListViewItem * );

  public slots:
    void setSortType (int s);
    void slotNextDocument();
    void slotPrevDocument();

  private slots:
    void slotDocumentCreated (Kate::Document *doc);
    void slotDocumentDeleted (uint documentNumber);
    void slotActivateView( QListViewItem *item );
    void slotModChanged (Kate::Document *doc);
    void slotModifiedOnDisc (Kate::Document *doc, bool b, unsigned char reason);
    void slotNameChanged (Kate::Document *doc);
    void slotViewChanged ();
    void slotMenu ( QListViewItem *item, const QPoint &p, int col );

  protected:
    virtual void keyPressEvent( QKeyEvent *e );
    /**
     * Reimplemented to force Single mode for real:
     * don't let a mouse click outside items deselect.
     */
    virtual void contentsMousePressEvent( QMouseEvent *e );
    /**
     * Reimplemented to make sure the first (and only) column is at least
     * the width of the viewport
     */
    virtual void resizeEvent( QResizeEvent *e );

  private:
    void setupActions ();
    void updateActions ();

  private:
    KateMainWindow *m_main;
    KateViewManager *viewManager;

    int m_sort;
    bool notify;

    KAction* windowNext;
    KAction* windowPrev;
    KSelectAction* sortAction;

    QPtrList<KateFileListItem> m_viewHistory;
    QPtrList<KateFileListItem> m_editHistory;

    QColor m_viewShade, m_editShade;
    bool m_enableBgShading;

    class ToolTip *m_tooltip;
};

class KFLConfigPage : public Kate::ConfigPage {
  Q_OBJECT
  public:
    KFLConfigPage( QWidget* parent=0, const char *name=0, KateFileList *fl=0 );
    virtual ~KFLConfigPage() {};

    virtual void apply();
    virtual void reload();

  public slots:
    void slotEnableChanged();

  private slots:
    void slotMyChanged();

  private:
    class QCheckBox *cbEnableShading;
    class KColorButton *kcbViewShade, *kcbEditShade;
    class QLabel *lEditShade, *lViewShade, *lSort;
    class QComboBox *cmbSort;
    KateFileList *m_filelist;

    bool m_changed;
};


#endif
// kate: space-indent on; indent-width 2; replace-tabs on;
