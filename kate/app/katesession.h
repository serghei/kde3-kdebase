/* This file is part of the KDE project
   Copyright (C) 2005 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __KATE_SESSION_H__
#define __KATE_SESSION_H__

#include "katemain.h"

#include <kdialogbase.h>
#include <ksimpleconfig.h>
#include <ksharedptr.h>
#include <kaction.h>

#include <qobject.h>
#include <qvaluelist.h>

class KateSessionManager;

class KDirWatch;
class KListView;
class KPushButton;

class QCheckBox;

class KateSession  : public KShared
{
  public:
    /**
     * Define a Shared-Pointer type
     */
    typedef KSharedPtr<KateSession> Ptr;

  public:
    /**
     * create a session from given file
     * @param fileName session filename, relative
     * @param name session name
     * @param manager pointer to the manager
     */
    KateSession (KateSessionManager *manager, const QString &fileName, const QString &name);

    /**
     * init the session object, after construction or create
     */
    void init ();

    /**
     * destruct me
     */
    ~KateSession ();

    /**
     * session filename, absolute, calculated out of relative filename + session dir
     * @return absolute path to session file
     */
    QString sessionFile () const;

    /**
     * relative session filename
     * @return relative filename for this session
     */
    const QString &sessionFileRelative () const { return m_sessionFileRel; }

    /**
     * session name
     * @return name for this session
     */
    const QString &sessionName () const { return m_sessionName; }

    /**
     * is this a valid session? if not, don't use any session if this is
     * the active one
     */
    bool isNew () const { return m_sessionName.isEmpty(); }

    /**
     * create the session file, if not existing
     * @param name name for this session
     * @param force force to create new file
     * @return true if created, false if no creation needed
     */
    bool create (const QString &name, bool force = false);

    /**
     * rename this session
     * @param name new name
     * @return success
     */
    bool rename (const QString &name);

    /**
     * config to read
     * on first access, will create the config object, delete will be done automagic
     * return 0 if we have no file to read config from atm
     * @return config to read from
     */
    KConfig *configRead ();

    /**
     * config to write
     * on first access, will create the config object, delete will be done automagic
     * return 0 if we have no file to write config to atm
     * @return config to write from
     */
    KConfig *configWrite ();

    /**
     * count of documents in this session
     * @return documents count
     */
    unsigned int documents () const { return m_documents; }

  private:
    /**
     * session filename, in local location we can write to
     * relative filename to the session dirs :)
     */
    QString m_sessionFileRel;

    /**
     * session name, extracted from the file, to display to the user
     */
    QString m_sessionName;

    /**
     * number of document of this session
     */
    unsigned int m_documents;

    /**
     * KateSessionMananger
     */
    KateSessionManager *m_manager;

    /**
     * simpleconfig to read from
     */
    KSimpleConfig *m_readConfig;

    /**
     * simpleconfig to write to
     */
    KSimpleConfig *m_writeConfig;
};

typedef QValueList<KateSession::Ptr> KateSessionList;

class KateSessionManager : public QObject
{
  Q_OBJECT

  public:
    KateSessionManager(QObject *parent);
    ~KateSessionManager();

    /**
     * allow access to this :)
     * @return instance of the session manager
     */
    static KateSessionManager *self();

    /**
     * allow access to the session list
     * kept up to date by watching the dir
     */
    inline KateSessionList & sessionList () { updateSessionList (); return m_sessionList; }

    /**
     * activate a session
     * first, it will look if a session with this name exists in list
     * if yes, it will use this session, else it will create a new session file
     * @param session session to activate
     * @param closeLast try to close last session or not?
     * @param saveLast try to save last session or not?
     * @param loadNew load new session stuff?
     */
    void activateSession (KateSession::Ptr session, bool closeLast = true, bool saveLast = true, bool loadNew = true);

    /**
     * create a new session
     * @param name session name
     */
    KateSession::Ptr createSession (const QString &name);

    /**
     * return session with given name
     * if no existing session matches, create new one with this name
     * @param name session name
     */
    KateSession::Ptr giveSession (const QString &name);

    /**
     * save current session
     * for sessions without filename: save nothing
     * @param tryAsk should we ask user if needed?
     * @param rememberAsLast remember this session as last used?
     * @return success
     */
    bool saveActiveSession (bool tryAsk = false, bool rememberAsLast = false);

    /**
     * return the current active session
     * sessionFile == empty means we have no session around for this instance of kate
     * @return session active atm
     */
    inline KateSession::Ptr activeSession () { return m_activeSession; }

    /**
     * session dir
     * @return global session dir
     */
    inline const QString &sessionsDir () const { return m_sessionsDir; }

    /**
     * initial session chooser, on app start
     * @return success, if false, app should exit
     */
    bool chooseSession ();

  public slots:
    /**
     * try to start a new session
     * asks user first for name
     */
    void sessionNew ();

    /**
     * try to open a existing session
     */
    void sessionOpen ();

    /**
     * try to save current session
     */
    void sessionSave ();

    /**
     * try to save as current session
     */
    void sessionSaveAs ();

    /**
     * show dialog to manage our sessions
     */
    void sessionManage ();

  private slots:
    void dirty (const QString &path);

  public:
    /**
     * trigger update of session list
     */
    void updateSessionList ();

  private:
    /**
     * absolute path to dir in home dir where to store the sessions
     */
    QString m_sessionsDir;

    /**
     * list of current available sessions
     */
    KateSessionList m_sessionList;

    /**
     * current active session
     */
    KateSession::Ptr m_activeSession;
};

class KateSessionChooser : public KDialogBase
{
  Q_OBJECT

  public:
    KateSessionChooser (QWidget *parent, const QString &lastSession);
    ~KateSessionChooser ();

    KateSession::Ptr selectedSession ();

    bool reopenLastSession ();

    enum {
      resultQuit = QDialog::Rejected,
      resultOpen,
      resultNew,
      resultNone
    };

  protected slots:
    /**
     * open session
     */
    void slotUser1 ();

    /**
     * new session
     */
    void slotUser2 ();

    /**
     * quit kate
     */
    void slotUser3 ();

    /**
     * selection has changed
     */
    void selectionChanged ();

  private:
    KListView *m_sessions;
    QCheckBox *m_useLast;
};

class KateSessionOpenDialog : public KDialogBase
{
  Q_OBJECT

  public:
    KateSessionOpenDialog (QWidget *parent);
    ~KateSessionOpenDialog ();

    KateSession::Ptr selectedSession ();

    enum {
      resultOk,
      resultCancel
    };

  protected slots:
    /**
     * cancel pressed
     */
    void slotUser1 ();

    /**
     * ok pressed
     */
    void slotUser2 ();

  private:
    KListView *m_sessions;
};

class KateSessionManageDialog : public KDialogBase
{
  Q_OBJECT

  public:
    KateSessionManageDialog (QWidget *parent);
    ~KateSessionManageDialog ();

  protected slots:
    /**
     * close pressed
     */
    void slotUser1 ();

    /**
     * selection has changed
     */
    void selectionChanged ();

    /**
     * try to rename session
     */
    void rename ();

    /**
     * try to delete session
     */
    void del ();

  private:
    /**
     * update our list
     */
    void updateSessionList ();

  private:
    KListView *m_sessions;
    KPushButton *m_rename;
    KPushButton *m_del;
};

class KateSessionsAction : public KActionMenu
{
  Q_OBJECT

  public:
    KateSessionsAction(const QString& text, QObject* parent = 0, const char* name = 0);
    ~KateSessionsAction (){;};

  public  slots:
    void slotAboutToShow();

    void openSession (int i);
};

#endif
