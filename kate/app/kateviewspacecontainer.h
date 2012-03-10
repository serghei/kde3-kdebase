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

#ifndef __KATE_VIEWSPACE_CONTAINER_H__
#define __KATE_VIEWSPACE_CONTAINER_H__

#include "katemain.h"
#include "../interfaces/viewmanager.h"

#include <kate/view.h>
#include <kate/document.h>

#include "katemdi.h"

class KConfig;
class KateMainWindow;

class KateViewSpaceContainer: public QVBox
{
  Q_OBJECT

  friend class KateViewSpace;
  friend class KateVSStatusBar;

  public:
    KateViewSpaceContainer (QWidget *parent, KateViewManager *viewManager);

    ~KateViewSpaceContainer ();

    inline QPtrList<Kate::View> &viewList () { return m_viewList; };

  public:
    /* This will save the splitter configuration */
    void saveViewConfiguration(KConfig *config,const QString& group);

    /* restore it */
    void restoreViewConfiguration (KConfig *config,const QString& group);

  private:
    /**
     * create and activate a new view for doc, if doc == 0, then
     * create a new document
     */
    bool createView ( Kate::Document *doc =0L );

    bool deleteView ( Kate::View *view, bool delViewSpace = true);

    void moveViewtoSplit (Kate::View *view);
    void moveViewtoStack (Kate::View *view);

    /* Save the configuration of a single splitter.
     * If child splitters are found, it calls it self with those as the argument.
     * If a viewspace child is found, it is asked to save its filelist.
     */
    void saveSplitterConfig(KateMDI::Splitter* s, int idx=0, KConfig* config=0L, const QString& viewConfGrp="");

    /** Restore a single splitter.
     * This is all the work is done for @see saveSplitterConfig()
     */
    void restoreSplitter ( KConfig* config, const QString &group, QWidget* parent , const QString& viewConfGrp);

    void removeViewSpace (KateViewSpace *viewspace);

    bool showFullPath;

  public:
    Kate::View* activeView ();
    KateViewSpace* activeViewSpace ();

    uint viewCount ();
    uint viewSpaceCount ();

    bool isViewActivationBlocked(){return m_blockViewCreationAndActivation;};

  public:
    void closeViews(uint documentNumber);
    KateMainWindow *mainWindow();
  friend class KateViewManager;

  private slots:
    void activateView ( Kate::View *view );
    void activateSpace ( Kate::View* v );
    void slotViewChanged();
    void reactivateActiveView();
    void slotPendingDocumentNameChanged();

    void documentCreated (Kate::Document *doc);
    void documentDeleted (uint docNumber);

  public slots:
     /* Splits a KateViewSpace into two.
      * The operation is performed by creating a KateMDI::Splitter in the parent of the KateViewSpace to be split,
      * which is then moved to that splitter. Then a new KateViewSpace is created and added to the splitter,
      * and a KateView is created to populate the new viewspace. The new KateView is made the active one,
      * because createView() does that.
      * If no viewspace is provided, the result of activeViewSpace() is used.
      * The isHoriz, true pr default, decides the orientation of the splitting action.
      * If atTop is true, the new viewspace will be moved to the first position in the new splitter.
      * If a newViewUrl is provided, the new view will show the document in that URL if any, otherwise
      * the document of the current view in the viewspace to be split is used.
      */
    void splitViewSpace( KateViewSpace* vs=0L, bool isHoriz=true, bool atTop=false );

    bool getShowFullPath() const { return showFullPath; }

    void activateView ( uint documentNumber );
    void activateView ( int documentNumber ) { activateView((uint) documentNumber); };

    /** Splits the active viewspace horizontally */
    void slotSplitViewSpaceHoriz () { splitViewSpace(); }
    /** Splits the active viewspace vertically */
    void slotSplitViewSpaceVert () { splitViewSpace( 0L, false ); }

    void slotCloseCurrentViewSpace();

    void statusMsg ();

    void setActiveSpace ( KateViewSpace* vs );
    void setActiveView ( Kate::View* view );

    void setShowFullPath(bool enable);

    void activateNextView();
    void activatePrevView();

  signals:
    void statusChanged (Kate::View *, int, int, int, bool, int, const QString &);
    void statChanged ();
    void viewChanged ();

  private:
    KateViewManager *m_viewManager;
    QPtrList<KateViewSpace> m_viewSpaceList;
    QPtrList<Kate::View> m_viewList;

    bool m_blockViewCreationAndActivation;

    bool m_activeViewRunning;

    bool m_pendingViewCreation;
    QGuardedPtr<Kate::Document> m_pendingDocument;
};

#endif
