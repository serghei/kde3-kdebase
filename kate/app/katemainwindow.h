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

#ifndef __KATE_MAINWINDOW_H__
#define __KATE_MAINWINDOW_H__

#include "katemain.h"
#include "katemdi.h"

#include <kate/view.h>
#include <kate/document.h>

#include <kparts/part.h>

#include <kaction.h>

class KateTabWidget;
class GrepTool;

namespace Kate {
  class MainWindow;
  class ToolViewManager;
}

class KFileItem;
class KRecentFilesAction;
class DCOPObject;

class KateExternalToolsMenuAction;

class KateMainWindow : public KateMDI::MainWindow, virtual public KParts::PartBase
{
  Q_OBJECT

  friend class KateConfigDialog;
  friend class KateViewManager;

  public:
    /**
     * Construct the window and restore it's state from given config if any
     * @param sconfig session config for this window, 0 if none
     * @param sgroup session config group to use
     */
    KateMainWindow (KConfig *sconfig, const QString &sgroup);

    /**
     * Destruct the nice window
     */
    ~KateMainWindow();

  /**
   * Accessor methodes for interface and child objects
   */
  public:
    Kate::MainWindow *mainWindow () { return m_mainWindow; }
    Kate::ToolViewManager *toolViewManager () { return m_toolViewManager; }

    KateViewManager *viewManager () { return m_viewManager; }

    DCOPObject *dcopObject () { return m_dcop; }

  /**
   * various methodes to get some little info out of this
   */
  public:
    /** Returns the URL of the current document.
     * anders: I add this for use from the file selector. */
    KURL activeDocumentUrl();

    uint mainWindowNumber () const { return myID; }

    /**
     * Prompts the user for what to do with files that are modified on disk if any.
     * This is optionally run when the window receives focus, and when the last
     * window is closed.
     * @return true if no documents are modified on disk, or all documents were
     * handled by the dialog; otherwise (the dialog was canceled) false.
     */
    bool showModOnDiskPrompt();

    /**
     * central tabwidget ;)
     * @return tab widget
     */
    KateTabWidget *tabWidget ();

  public:
    void readProperties(KConfig *config);
    void saveProperties(KConfig *config);
    void saveGlobalProperties( KConfig* sessionConfig );

  public:
    bool queryClose_internal();

  private:
    void setupMainWindow();
    void setupActions();
    bool queryClose();

    /**
     * read some global options from katerc
     */
    void readOptions();

    /**
     * save some global options to katerc
     */
    void saveOptions();

    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );

  /**
   * slots used for actions in the menus/toolbars
   * or internal signal connections
   */
  private slots:
    void newWindow ();

    void slotConfigure();

    void slotOpenWithMenuAction(int idx);

    void slotGrepToolItemSelected ( const QString &filename, int linenumber );
    void slotMail();

    void slotFileQuit();
    void slotEditToolbars();
    void slotNewToolbarConfig();
    void slotWindowActivated ();
    void slotUpdateOpenWith();
    void documentMenuAboutToShow();
    void slotDropEvent(QDropEvent *);
    void editKeys();
    void mSlotFixOpenWithMenu();

    void fileSelected(const KFileItem *file);

    void tipOfTheDay();

    /* to update the caption */
    void slotDocumentCreated (Kate::Document *doc);
    void updateCaption (Kate::Document *doc);

    void pluginHelp ();
    void slotFullScreen(bool);

  public:
    void openURL (const QString &name=0L);

  private slots:
    void updateGrepDir (bool visible);

  protected:
    bool event( QEvent * );

  private slots:
    void slotDocumentCloseAll();

  private:
    static uint uniqueID;
    uint myID;

    Kate::MainWindow *m_mainWindow;
    Kate::ToolViewManager *m_toolViewManager;

    bool syncKonsole;
    bool modNotification;

    DCOPObject *m_dcop;

    // console
    KateConsole *console;

    // management items
    KateViewManager *m_viewManager;

    KRecentFilesAction *fileOpenRecent;

    KateFileList *filelist;
    KateFileSelector *fileselector;

    KActionMenu* documentOpenWith;

    QPopupMenu *documentMenu;

    KToggleAction* settingsShowFilelist;
    KToggleAction* settingsShowFileselector;

    KateExternalToolsMenuAction *externalTools;
    GrepTool * greptool;
    bool m_modignore, m_grrr;

    KateTabWidget *m_tabWidget;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
