/*****************************************************************

Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>

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

#ifndef __taskbarcontainer_h__
#define __taskbarcontainer_h__

#include <qframe.h>

#include <dcopobject.h>
#include <kpanelapplet.h>
#include <kpanelextension.h>

class QBoxLayout;
class SimpleButton;
class KWindowListMenu;
class TaskBar;

class KDE_EXPORT TaskBarContainer : public QFrame, public DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    TaskBarContainer( bool enableFrame, QWidget* parent = 0, const char* name = 0 );
    ~TaskBarContainer();

    void orientationChange( Orientation );
    void popupDirectionChange( KPanelApplet::Direction );
    void preferences();
    //FIXME: don't use Position, use Orientation
    QSize sizeHint( KPanelExtension::Position, QSize maxSize ) const;
    void setBackground();

k_dcop:
    void configChanged();

signals:
    void containerCountChanged();

protected slots:
    void configure();
    void showWindowListMenu();
    void windowListMenuAboutToHide();
    void reconnectWindowListButton();

private:
    KPanelApplet::Direction      direction;
    bool                         showWindowListButton;
    QBoxLayout *                 layout;
    TaskBar *                    taskBar;
    SimpleButton *               windowListButton;
    KWindowListMenu *            windowListMenu;
};

#endif
