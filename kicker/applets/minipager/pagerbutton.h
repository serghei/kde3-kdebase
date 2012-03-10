/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#ifndef __MINIPAGERBUTTON_H
#define __MINIPAGERBUTTON_H

#include <qbutton.h>

#include "taskmanager.h"
#include "kickertip.h"

class KPixmap;
class KWinModule;
class KMiniPager;
class KSharedPixmap;
class QLineEdit;

class KMiniPagerButton : public QButton, public KickerTip::Client
{
    Q_OBJECT
public:
    KMiniPagerButton(int desk, bool useViewports, const QPoint& viewport,
            KMiniPager *parent=0, const char *name=0);
    ~KMiniPagerButton();

    int desktop() { return m_desktop; }

    QString desktopName() { return m_desktopName; }
    void setDesktopName( QString name ) { m_desktopName = name; }

    void rename();
    void backgroundChanged();
    void windowsChanged();

    bool shouldPaintWindow( KWin::WindowInfo *info ) const;

signals:
    void buttonSelected( int desk );
    void showMenu( const QPoint&, int );

protected:
    void drawButton(QPainter *);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void resizeEvent(QResizeEvent *ev);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent* e);
    void dragLeaveEvent(QDragLeaveEvent* e);
    void enabledChange( bool oldEnabled );
    void dropEvent(QDropEvent* e);

    bool eventFilter(QObject*, QEvent*);
    void updateKickerTip(KickerTip::Data &data);

private slots:
    void slotToggled(bool);
    void slotClicked();
    void slotDragSwitch();

    void backgroundLoaded( bool loaded );

private:
    void loadBgPixmap();
    QRect mapGeometryToViewport(const KWin::WindowInfo&) const;
    QPoint mapPointToViewport(const QPoint&) const;

    KMiniPager* m_pager;
    int m_desktop;
    bool m_useViewports;
    QString m_desktopName;
    QPoint m_viewport;

    QTimer m_updateCompressor;
    QTimer m_dragSwitchTimer;
    Task::Ptr m_dragging;

    QLineEdit* m_lineEdit;

    KSharedPixmap *m_sharedPixmap;
    KPixmap *m_bgPixmap;
    static KSharedPixmap *s_commonSharedPixmap;
    static KPixmap *s_commonBgPixmap;
    bool m_isCommon;

    Task::Ptr m_currentWindow;
    bool m_inside;
};

#endif
