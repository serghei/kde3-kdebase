/*
   This file is part of the Kate text editor of the KDE project.
   It describes a "external tools" action for kate and provides a dialog
   page to configure that.

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

   ---
   Copyright (C) 2004, Anders Lund <anders@alweb.dk>
*/

#ifndef _KATE_EXTERNAL_TOOLS_H_
#define _KATE_EXTERNAL_TOOLS_H_

#include <kaction.h>
#include <kdialogbase.h>
#include <kate/document.h>
#include <kmacroexpander.h>
#include <qpixmap.h>

/**
 * The external tools action
 * This action creates a menu, in which each item will launch a process
 * with the provided arguments, which may include the following macros:
 * - %URLS: the URLs of all open documents.
 * - %URL: The URL of the active document.
 * - %filedir: The directory of the current document, if that is a local file.
 * - %selection: The selection of the active document.
 * - %text: The text of the active document.
 * - %line: The line number of the cursor in the active view.
 * - %column: The column of the cursor in the active view.
 *
 * Each item has the following properties:
 * - Name: The friendly text for the menu
 * - Exec: The command to execute, including arguments.
 * - TryExec: the name of the executable, if not available, the
 *       item will not be displayed.
 * - MimeTypes: An optional list of mimetypes. The item will be disabled or
 *       hidden if the current file is not of one of the indicated types.
 *
 */
class KateExternalToolsMenuAction : public KActionMenu
{
  friend class KateExternalToolAction;

  Q_OBJECT
  public:
    KateExternalToolsMenuAction( const QString &text=QString::null, QObject *parent=0, const char* name=0, class KateMainWindow *mw=0 );
    ~KateExternalToolsMenuAction() {};

    /**
     * This will load all the confiured services.
     */
    void reload();

    class KActionCollection *actionCollection() { return m_actionCollection; }

  private slots:
    void slotDocumentChanged();

  private:
    class KActionCollection *m_actionCollection;
    class KateMainWindow *mainwindow; // for the actions to access view/doc managers
};

/**
 * This Action contains a KateExternalTool
 */
class KateExternalToolAction : public KAction, public KWordMacroExpander
{
  Q_OBJECT
  public:
    KateExternalToolAction( QObject *parent, const char *name, class KateExternalTool *t );
    ~KateExternalToolAction();
  protected:
    virtual bool expandMacro( const QString &str, QStringList &ret );

  private slots:
    void slotRun();

  public:
    class KateExternalTool *tool;
};

/**
 * This class defines a single external tool.
 */
class KateExternalTool
{
  public:
    KateExternalTool( const QString &name=QString::null,
                      const QString &command=QString::null,
                      const QString &icon=QString::null,
                      const QString &tryexec=QString::null,
                      const QStringList &mimetypes=QStringList(),
                      const QString &acname=QString::null,
                      const QString &cmdname=QString::null,
                      int save=0 );
    ~KateExternalTool() {};

    QString name; ///< The name used in the menu.
    QString command; ///< The command to execute.
    QString icon; ///< the icon to use in the menu.
    QString tryexec; ///< The name or path of the executable.
    QStringList mimetypes; ///< Optional list of mimetypes for which this action is valid.
    bool hasexec; ///< This is set by the constructor by calling checkExec(), if a value is present.
    QString acname; ///< The name for the action. This is generated first time the action is is created.
    QString cmdname; ///< The name for the commandline.
    int save; ///< We can save documents prior to activating the tool command: 0 = nothing, 1 = current document, 2 = all documents.

    /**
     * @return true if mimetypes is empty, or the @p mimetype matches.
     */
    bool valid( const QString &mimetype ) const;
    /**
     * @return true if "tryexec" exists and has the executable bit set, or is
     * empty.
     * This is run at least once, and the tool is disabled if it fails.
     */
    bool checkExec();

  private:
    QString m_exec; ///< The fully qualified path of the executable.
};

/**
 * The config widget.
 * The config widget allows the user to view a list of services of the type
 * "Kate/ExternalTool" and add, remove or edit them.
 */
class KateExternalToolsConfigWidget : public Kate::ConfigPage
{
  Q_OBJECT
  public:
    KateExternalToolsConfigWidget( QWidget *parent, const char* name);
    virtual ~KateExternalToolsConfigWidget();

    virtual void apply();
    virtual void reload();
    virtual void reset() { reload(); } // sigh
    virtual void defaults() { reload(); } // double sigh

  private slots:
    void slotNew();
    void slotEdit();
    void slotRemove();
    void slotInsertSeparator();

    void slotMoveUp();
    void slotMoveDown();

    void slotSelectionChanged();

  private:
    QPixmap blankIcon();

    QStringList m_removed;

    class KListBox *lbTools;
    class QPushButton *btnNew, *btnRemove, *btnEdit, *btnMoveUp, *btnMoveDwn;

    class KConfig *config;

    bool m_changed;
};

/**
 * A Singleton class for invoking external tools with the view command line
 */
 class KateExternalToolsCommand : public Kate::Command {
 public:
    KateExternalToolsCommand ();
    virtual ~KateExternalToolsCommand () {};
    static KateExternalToolsCommand *self();
    void reload();
  public:
    virtual QStringList cmds ();
    virtual bool exec (Kate::View *view, const QString &cmd, QString &msg);
    virtual bool help (Kate::View *view, const QString &cmd, QString &msg);
  private:
    static KateExternalToolsCommand *s_self;
    QStringList m_list;
    QMap<QString,QString> m_map;
    bool m_inited;
 };

/**
 * A Dialog to edit a single KateExternalTool object
 */
class KateExternalToolServiceEditor : public KDialogBase
{
  Q_OBJECT

  public:

    KateExternalToolServiceEditor( KateExternalTool *tool=0,
    				   QWidget *parent=0, const char *name=0 );

    class QLineEdit *leName, *leExecutable, *leMimetypes,*leCmdLine;
    class QTextEdit *teCommand;
    class KIconButton *btnIcon;
    class QComboBox *cmbSave;

  private slots:
    /**
     * Run when the OK button is clicked, to ensure critical values are provided
     */
    void slotOk();
    /**
     * show a mimetype chooser dialog
     */
    void showMTDlg();

  private:
    KateExternalTool *tool;
};
#endif //_KATE_EXTERNAL_TOOLS_H_
