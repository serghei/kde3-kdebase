/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

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

#ifndef __taskbar_h__
#define __taskbar_h__

#include <kpanelextension.h>
#include <taskmanager.h>

#include "taskcontainer.h"
#include "panner.h"
#include "kshadowengine.h"

#define WINDOWLISTBUTTON_SIZE 15
#define BUTTON_MAX_WIDTH 200
#define BUTTON_MIN_WIDTH 20

class Startup;
class Task;
class KGlobalAccel;

class TaskBar : public Panner
{
    Q_OBJECT

public:
    TaskBar( QWidget *parent = 0, const char *name = 0 );
    ~TaskBar();

    QSize sizeHint() const;
    QSize sizeHint( KPanelExtension::Position, QSize maxSize ) const;

    void setOrientation( Orientation );
    void setArrowType( Qt::ArrowType at );

    int containerCount() const;
    int taskCount() const;
    int showScreen() const;

    bool showIcon() const { return m_showIcon; }
    bool sortByDesktop() const { return m_sortByDesktop; }
    bool showAllWindows() const { return m_showAllWindows; }

    QImage* blendGradient(const QSize& size);
    
    void setBackground();
    
    KTextShadowEngine *textShadowEngine();

public slots:
    void configure();

signals:
    void containerCountChanged();

protected slots:
    void add(Task::Ptr);
    void add(Startup::Ptr);
    void showTaskContainer(TaskContainer*);
    void remove(Task::Ptr task, TaskContainer *container = 0);
    void remove(Startup::Ptr startup, TaskContainer *container = 0);

    void desktopChanged( int );
    void windowChanged(Task::Ptr);
    void windowChangedGeometry(Task::Ptr);

    void publishIconGeometry();

    void activateNextTask( bool forward );
    void slotActivateNextTask();
    void slotActivatePreviousTask();
    void slotSettingsChanged(int);
    void reLayout();

protected:
    void reLayoutEventually();
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseDoubleClickEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void viewportResizeEvent( QResizeEvent * );
    void wheelEvent(QWheelEvent*);
    void propagateMouseEvent( QMouseEvent* );
    void resizeEvent( QResizeEvent* );
    void moveEvent( QMoveEvent* );
    bool idMatch( const QString& id1, const QString& id2 );
    TaskContainer::List filteredContainers();

private:
    void sortContainersByDesktop(TaskContainer::List& list);
    void setViewportBackground();

    bool			blocklayout;
    bool			m_showAllWindows;
    // The screen to show, -1 for all screens
    int 			m_currentScreen;
    bool			m_showOnlyCurrentScreen;
    bool			m_sortByDesktop;
    bool			m_showIcon;
    bool			m_showOnlyIconified;
    ArrowType 			arrowType;
    TaskContainer::List		containers;
    TaskContainer::List		m_hiddenContainers;
    TaskContainer::List		m_deletableContainers;
    PixmapList			frames;
    int                         maximumButtonsWithoutShrinking() const;
    bool                        shouldGroup() const;
    bool                        isGrouping;
    void                        reGroup();
    KGlobalAccel*               keys;
    KTextShadowEngine* m_textShadowEngine;
    QTimer m_relayoutTimer;
    bool m_ignoreUpdates;
    QImage m_blendGradient;
};

#endif
