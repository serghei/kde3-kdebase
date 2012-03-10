/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>

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

#ifndef __taskcontainer_h__
#define __taskcontainer_h__

#include <qpixmap.h>
#include <qtimer.h>
#include <qtoolbutton.h>

#include "kickertip.h"
#include "taskmanager.h"

class TaskBar;

typedef QValueList<QPixmap*> PixmapList;

class TaskContainer : public QToolButton, public KickerTip::Client
{
    Q_OBJECT

public:
    typedef QValueList<TaskContainer*> List;
    typedef QValueList<TaskContainer*>::iterator Iterator;

    TaskContainer(Task::Ptr, TaskBar*, QWidget *parent = 0, const char *name = 0);
    TaskContainer(Startup::Ptr, PixmapList&, TaskBar*,
                  QWidget *parent = 0, const char *name = 0);
    virtual ~TaskContainer();

    void setArrowType( Qt::ArrowType at );

    void init();

    void add(Task::Ptr);
    void remove(Task::Ptr);
    void remove(Startup::Ptr);

    bool contains(Task::Ptr);
    bool contains(Startup::Ptr);
    bool contains(WId);

    bool isEmpty();
    bool onCurrentDesktop();
    bool isIconified();
    bool isOnScreen();

    QString id();
    int desktop();
    QString name();

    virtual QSizePolicy sizePolicy () const;

    void publishIconGeometry( QPoint );
    void desktopChanged( int );
    void windowChanged(Task::Ptr);
    void settingsChanged();
    bool eventFilter( QObject *o, QEvent *e );

    int taskCount() const { return tasks.count(); }
    int filteredTaskCount() const { return m_filteredTasks.count(); }

    bool activateNextTask( bool forward, bool& forcenext );

    void updateKickerTip(KickerTip::Data&);

    void finish();
    
    void setBackground();
    
public slots:
    void updateNow();

signals:
    void showMe(TaskContainer*);

protected:
    void paintEvent(QPaintEvent*);
    void drawButton(QPainter*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dragLeaveEvent(QDragLeaveEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    bool startDrag(const QPoint& pos);
    void stopTimers();

    void performAction(int);
    void popupMenu(int);

    void updateFilteredTaskList();

protected slots:
    void animationTimerFired();
    void attentionTimerFired();
    void dragSwitch();
    void iconChanged();
    void setLastActivated();
    void taskChanged(bool geometryChangeOnly);
    void showMe();

private:
    void checkAttention(const Task::Ptr changed_task = NULL);
    QString                     sid;
    QTimer                      animationTimer;
    QTimer                      dragSwitchTimer;
    QTimer                      attentionTimer;
    QTimer                      m_paintEventCompressionTimer;
    int                         currentFrame;
    PixmapList                  frames;
    int                         attentionState;
    QRect                       iconRect;
    QPixmap                     animBg;
    Task::List                  tasks;
    Task::List                  m_filteredTasks;
    Task::Ptr                   lastActivated;
    QPopupMenu*                 m_menu;
    Startup::Ptr                m_startup;
    ArrowType                   arrowType;
    TaskBar*                    taskBar;
    bool                        discardNextMouseEvent;
    bool                        aboutToActivate;
    bool                        m_mouseOver;
    bool                        m_paintEventCompression;
    enum                        { ATTENTION_BLINK_TIMEOUT = 4 };
    QPoint                      m_dragStartPos;
};

#endif
