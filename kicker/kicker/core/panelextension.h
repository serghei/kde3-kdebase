/*****************************************************************

Copyright (c) 2000 Matthias Elter
              2004 Aaron J. Seigo <aseigo@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef _panelextension_h_
#define _panelextension_h_

#include <kpanelextension.h>
#include <dcopobject.h>

#include "appletinfo.h"

class AppletContainer;
class ContainerArea;
class QPopupMenu;
class QGridLayout;

// This is the KPanelExtension responsible for the main kicker panel
// Prior to KDE 3.4 it was the ChildPanelExtension

class PanelExtension : public KPanelExtension, virtual public  DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    PanelExtension(const QString& configFile, QWidget *parent = 0, const char *name = 0);
    virtual ~PanelExtension();

    QPopupMenu* opMenu();

k_dcop:
    int panelSize() { return sizeInPixels(); }
    int panelOrientation() { return static_cast<int>(orientation()); }
    int panelPosition() { return static_cast<int>(position()); }

    void setPanelSize(int size);
    void addKMenuButton();
    void addDesktopButton();
    void addWindowListButton();
    void addURLButton(const QString &url);
    void addBrowserButton(const QString &startDir);
    void addServiceButton(const QString &desktopEntry);
    void addServiceMenuButton(const QString &name, const QString& relPath);
    void addNonKDEAppButton(const QString &filePath, const QString &icon,
                            const QString &cmdLine, bool inTerm);
    void addNonKDEAppButton(const QString &title, const QString &description,
                            const QString &filePath, const QString &icon,
                            const QString &cmdLine, bool inTerm);

    void addApplet(const QString &desktopFile);
    void addAppletContainer(const QString &desktopFile); // KDE4: remove, useless

    bool insertApplet(const QString& desktopFile, int index);
    bool insertImmutableApplet(const QString& desktopFile, int index);
    QStringList listApplets();
    bool removeApplet(int index);

    void restart(); // KDE4: remove, moved to Kicker
    void configure(); // KDE4: remove, moved to Kikcker

public:
    QSize sizeHint(Position, QSize maxSize) const;
    Position preferedPosition() const { return Bottom; }
    bool eventFilter( QObject *, QEvent * );

protected:
    void positionChange(Position);

    ContainerArea    *_containerArea;

protected slots:
    void configurationChanged();
    void immutabilityChanged(bool);
    void slotBuildOpMenu();
    void showConfig();
    virtual void populateContainerArea();

private:
    QPopupMenu* _opMnu;
    QPopupMenu* m_panelAddMenu;
    QPopupMenu* m_removeMnu;
    QPopupMenu* m_addExtensionMenu;
    QPopupMenu* m_removeExtensionMenu;
    QString _configFile;
    bool m_opMenuBuilt;
};

class MenubarExtension : public PanelExtension
{
    Q_OBJECT

    public:
        MenubarExtension(const AppletInfo& info);
        virtual ~MenubarExtension();

    protected slots:
        virtual void populateContainerArea();

    private:
        MenubarExtension();

        AppletContainer* m_menubar;
};

#endif
