/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

#ifndef __KATE_APP_H__
#define __KATE_APP_H__

#include "katemain.h"

#include <kapplication.h>

#include <qvaluelist.h>

class KateSessionManager;
class KateAppDCOPIface;

namespace Kate {
class Application;
}

class KCmdLineArgs;

/**
 * Kate Application
 * This class represents the core kate application object
 */
class KDE_EXPORT KateApp : public KApplication {
    Q_OBJECT

    /**
     * constructors & accessor to app object + plugin interface for it
     */
public:
    /**
     * application constructor
     * @param args parsed command line args
     */
    KateApp(KCmdLineArgs *args);

    /**
     * application destructor
     */
    ~KateApp();

    /**
     * static accessor to avoid casting ;)
     * @return app instance
     */
    static KateApp *self();

    /**
     * accessor to the Kate::Application plugin interface
     * @return application plugin interface
     */
    Kate::Application *application();

    /**
     * Returns the current Kate version (X.Y) or (X.Y.Z)
     * @param fullVersion should full version be returned?
     * @return Kate version
     */
    static QString kateVersion(bool fullVersion = true);

    /**
     * kate init
     */
private:
    /**
     * restore a old kate session
     */
    void restoreKate();

    /**
     * try to start kate
     * @return success, if false, kate should exit
     */
    bool startupKate();

    /**
     * kate shutdown
     */
public:
    /**
     * shutdown kate application
     * @param win mainwindow which is used for dialogs
     */
    void shutdownKate(KateMainWindow *win);

    /**
     * application should exit
     * @return should we exit?
     */
    bool shouldExit()
    {
        return m_shouldExit;
    }

    /**
     * other accessors for global unique instances
     */
public:
    /**
     * accessor to plugin manager
     * @return plugin manager instance
     */
    KatePluginManager *pluginManager();

    /**
     * accessor to document manager
     * @return document manager instance
     */
    KateDocManager *documentManager();

    /**
     * accessor to session manager
     * @return session manager instance
     */
    KateSessionManager *sessionManager();

    /**
     * window management
     */
public:
    /**
     * create a new main window, use given config if any for restore
     * @param sconfig session config object
     * @param sgroup session group for this window
     * @return new constructed main window
     */
    KateMainWindow *newMainWindow(KConfig *sconfig = 0, const QString &sgroup = "");

    /**
     * removes the mainwindow given, DOES NOT DELETE IT
     * @param mainWindow window to remove
     */
    void removeMainWindow(KateMainWindow *mainWindow);

    /**
     * give back current active main window
     * can only be 0 at app start or exit
     * @return current active main window
     */
    KateMainWindow *activeMainWindow();

    /**
     * give back number of existing main windows
     * @return number of main windows
     */
    uint mainWindows() const;

    /**
     * give back the window you want
     * @param n window index
     * @return requested main window
     */
    KateMainWindow *mainWindow(uint n);

    /**
     * some stuff for the dcop API
     */
public:
    /**
     * open url with given encoding
     * used by kate if --use given
     * @param url filename
     * @param encoding encoding name
     * @param isTempFile if set to true and the file is a local file, it will be deleted when the document is closed.
     * @return success
     */
    bool openURL(const KURL &url, const QString &encoding, bool isTempFile);

    /**
     * position cursor in current active view
     * @param line line to set
     * @param column column to set
     * @return success
     */
    bool setCursor(int line, int column);

    /**
     * helper to handle stdin input
     * open a new document/view, fill it with the text given
     * @param text text to fill in the new doc/view
     * @return success
     */
    bool openInput(const QString &text);

private:
    /**
     * kate's command line args
     */
    KCmdLineArgs *m_args;

    /**
     * plugin interface
     */
    Kate::Application *m_application;

    /**
     * document manager
     */
    KateDocManager *m_docManager;

    /**
     * plugin manager
     */
    KatePluginManager *m_pluginManager;

    /**
     * session manager
     */
    KateSessionManager *m_sessionManager;

    /**
     * known main windows
     */
    QValueList< KateMainWindow * > m_mainWindows;

    /**
     * dcop interface
     */
    KateAppDCOPIface *m_obj;

    /**
     * should exit flag
     */
    bool m_shouldExit;
};

#endif
