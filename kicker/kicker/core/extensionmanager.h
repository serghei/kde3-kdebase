/*****************************************************************

Copyright (c) 2000 Matthias Elter

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

#ifndef __extensionmanager_h__
#define __extensionmanager_h__

#include <qvaluelist.h>
#include <qobject.h>

#include "container_extension.h"

const int XineramaAllScreens = -2;

class ExtensionManager : public QObject
{
    Q_OBJECT

public:
    static ExtensionManager* the();

    void configureMenubar(bool duringInit);
    void addExtension( const QString &desktopFile );
    bool isMainPanel(const QWidget* panel) const;
    bool isMenuBar(const QWidget* panel) const;
    void addContainer( ExtensionContainer* );
    void removeAllContainers();
    ExtensionList containers() const { return _containers; }

    KPanelExtension::Position initialPanelPosition(KPanelExtension::Position preferred);
    QRect workArea(int XineramaScreen, const ExtensionContainer* container);
    int nextPanelOrder();

    // return the space available for all icons on the desktop
    // subtracts all panels from XineramaScreen's geometry
    QRect desktopIconsArea(int xineramaScreen) const;

public slots:
    void removeContainer( ExtensionContainer* );
    void initialize();
    void extensionSizeChanged(ExtensionContainer *);

signals:
    void desktopIconsAreaChanged(const QRect &, int xineramaScreen);

protected:
    friend class Kicker;

    ExtensionManager();
    ~ExtensionManager();
    QString uniqueId();
    void saveContainerConfig();
    bool shouldExclude(int XineramaScreen,
                       const ExtensionContainer* container,
                       const ExtensionContainer* exclude) const;

protected slots:
    void configurationChanged();
    void updateMenubar();

private:
    void migrateMenubar();
    void reduceArea(QRect &area, const ExtensionContainer *panel) const;

    ExtensionList _containers;
    ExtensionContainer* m_menubarPanel;
    ExtensionContainer* m_mainPanel;
    int m_panelCounter;
    static ExtensionManager* m_self;
};

#endif

