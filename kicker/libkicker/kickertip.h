/*****************************************************************

Copyright (c) 2004 Zack Rusin <zrusin@kde.org>
                   Sami Kyostil <skyostil@kempele.fi>
                   Aaron J. Seigo <aseigo@kde.org>

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

#ifndef KICKER_TIP_H
#define KICKER_TIP_H

#include <qbitmap.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qwidget.h>

#include <kpanelapplet.h>

class QMimeSourceFactory;
class QPaintEvent;
class QSimpleRichText;
class QTimer;

class KDE_EXPORT KickerTip : public QWidget
{
    Q_OBJECT

public:
    enum MaskEffect { Plain, Dissolve };

    struct Data
    {
            QString message;
            QString subtext;
            QPixmap icon;
            KickerTip::MaskEffect maskEffect;
            int duration;
            KPanelApplet::Direction direction;

            // do NOT delete this in the client!
            QMimeSourceFactory* mimeFactory;
    };

    class KDE_EXPORT Client
    {
        public:
            virtual void updateKickerTip(KickerTip::Data&) = 0;
            void updateKickerTip() const;
    };

    static KickerTip* the();
    static void enableTipping(bool tip);
    static bool tippingEnabled();

    void untipFor(const QWidget* w);
    bool eventFilter(QObject *o, QEvent *e);

protected:
    KickerTip(QWidget * parent);
    ~KickerTip();

    void paintEvent(QPaintEvent * e);
    void mousePressEvent(QMouseEvent * e);

    void plainMask();
    void dissolveMask();

    void displayInternal();
    void hide();

    void tipFor(const QWidget* w);
    bool isTippingFor(const QWidget* w) const;

protected slots:
    void tipperDestroyed(QObject* o);
    void internalUpdate();
    void display();

private:
    QBitmap m_mask;
    QPixmap m_pixmap;
    QPixmap m_icon;
    MaskEffect m_maskEffect;
    QSimpleRichText* m_richText;
    QMimeSourceFactory* m_mimeFactory;

    int m_dissolveSize;
    int m_dissolveDelta;
    KPanelApplet::Direction m_direction;

    QTimer m_timer;
    QTimer m_frameTimer;
    bool m_dirty;
    bool m_toolTipsEnabled;

    const QWidget* m_tippingFor;

    static KickerTip* m_self;
    static int m_tippingEnabled;

    friend class KickerTip::Client;
};

#endif
