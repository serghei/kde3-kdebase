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

#ifndef __CLOCK_H
#define __CLOCK_H

#include <qlcdnumber.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qguardedptr.h>
#include <qdatetime.h>
#include <qvbox.h>
#include <qstringlist.h>
#include <qtooltip.h>
#include <qevent.h>

#include <dcopobject.h>
#include <kpanelapplet.h>
#include <kdirwatch.h>
#include <kconfigdialog.h>

#include <kickertip.h>
#include "settings.h"

class QTimer;
class QBoxLayout;
class DatePicker;
class QPixmap;
class Zone;
class KPopupMenu;
class Prefs;
class ClockApplet;

namespace KIO
{
  class Job;
}

class DigitalWidget;
class AnalogWidget;
class FuzzyWidget;
class ClockApplet;
class KConfigDialogManager;
class SettingsWidgetImp;

class SettingsWidgetImp : public SettingsWidget
{
    Q_OBJECT
    
    public:
        SettingsWidgetImp(Prefs *p=0,
                Zone *z=0,
                QWidget* parent=0,
                const char* name=0,
                WFlags fl=0);
    public slots:
        void OkApply();
    
    private:
        Prefs *prefs;
        Zone *zone;
};

class KConfigDialogSingle : public KConfigDialog
{
    Q_OBJECT
    
    public:
        KConfigDialogSingle(Zone *zone,
                QWidget *parent=0,
                const char *name=0,
                Prefs *prefs=0,
                KDialogBase::DialogType dialogType = KDialogBase::IconList,
                bool modal=false);
    
        SettingsWidgetImp* settings;
    
        void updateSettings();
        void updateWidgets();
        void updateWidgetsDefault();

    protected slots:
        void selectPage(int p);
        void dateToggled();
    
    private:
        DigitalWidget *digitalPage;
        AnalogWidget *analogPage;
        FuzzyWidget *fuzzyPage;
        Prefs *_prefs;
};

/**
 * Base class for all clock types
 */
class ClockWidget
{
    public:
        ClockWidget(ClockApplet *applet, Prefs *prefs);
        virtual ~ClockWidget();
        
        virtual QWidget* widget()=0;
        virtual int preferedWidthForHeight(int h) const =0;
        virtual int preferedHeightForWidth(int w) const =0;
        virtual void updateClock()=0;
        virtual void forceUpdate() { _force = true; updateClock(); }
        virtual void loadSettings()=0;
        virtual bool showDate()=0;
        virtual bool showDayOfWeek()=0;
    
    protected:
        ClockApplet *_applet;
        Prefs *_prefs;
        QTime _time;
        bool _force;
};


class PlainClock : public QLabel, public ClockWidget
{
    Q_OBJECT

    public:
        PlainClock(ClockApplet *applet, Prefs *prefs, QWidget *parent=0, const char *name=0);
        
        QWidget* widget()    { return this; }
        int preferedWidthForHeight(int h) const;
        int preferedHeightForWidth(int w) const;
        void updateClock();
        void loadSettings();
        bool showDate();
        bool showDayOfWeek();
    
    protected:
        QString _timeStr;
};


class DigitalClock : public QLCDNumber, public ClockWidget
{
    Q_OBJECT

    public:
        DigitalClock(ClockApplet *applet, Prefs *prefs, QWidget *parent=0, const char *name=0);
        ~DigitalClock();
    
        QWidget* widget()    { return this; }
        int preferedWidthForHeight(int h) const;
        int preferedHeightForWidth(int w) const;
        void updateClock();
        void loadSettings();
        bool showDate();
        bool showDayOfWeek();
    
    protected:
        void paintEvent( QPaintEvent*);
        void drawContents( QPainter * p);
        void resizeEvent ( QResizeEvent *ev);
        
        QPixmap *_buffer;
        QString _timeStr;
        QPixmap lcdPattern;
};


class AnalogClock : public QFrame, public ClockWidget
{
    Q_OBJECT

    public:
        AnalogClock(ClockApplet *applet, Prefs *prefs, QWidget *parent=0, const char *name=0);
        ~AnalogClock();
    
        QWidget* widget()                        { return this; }
        int preferedWidthForHeight(int h) const  { return h; }
        int preferedHeightForWidth(int w) const  { return w; }
        void updateClock();
        void loadSettings();
        bool showDate();
        bool showDayOfWeek();
    
    protected:
        virtual void paintEvent(QPaintEvent *);
        void styleChange(QStyle&);
        void initBackgroundPixmap();
        
        QPixmap *_spPx;
        QPixmap lcdPattern;
        int _bgScale;
};


class FuzzyClock : public QFrame, public ClockWidget
{
    Q_OBJECT

    public:
        FuzzyClock(ClockApplet *applet, Prefs* prefs, QWidget *parent=0, const char *name=0);
        
        QWidget* widget()    { return this; }
        int preferedWidthForHeight(int h) const;
        int preferedHeightForWidth(int w) const;
        void updateClock();
        void loadSettings();
        bool showDate();
        bool showDayOfWeek();

    public slots:
        void deleteMyself();

    protected:
        virtual void drawContents(QPainter *p);
        
        QStringList hourNames;
        QStringList normalFuzzy;
        QStringList normalFuzzyOne;
        QStringList dayTime;
        
        QString _timeStr;

    private:
        bool alreadyDrawing;
};

class ClockAppletToolTip : public QToolTip
{
    public:
        ClockAppletToolTip( ClockApplet* clock );

    protected:
        virtual void maybeTip( const QPoint & );

    private:
        ClockApplet *m_clock;
};

class ClockApplet : public KPanelApplet, public KickerTip::Client, public DCOPObject
{
  Q_OBJECT
  K_DCOP

  friend class ClockAppletToolTip;

    public:
        ClockApplet(const QString& configFile, Type t = Normal, int actions = 0,
                    QWidget *parent = 0, const char *name = 0);
        ~ClockApplet();
        
        int widthForHeight(int h) const;
        int heightForWidth(int w) const;
        void preferences();
        void preferences(bool timezone);
        int type();
        Orientation getOrientation()    { return orientation(); }
        void resizeRequest()            { emit(updateLayout()); }
        const Zone* timezones()            { return zone; }
        
        QTime clockGetTime();
        QDate clockGetDate();

        virtual void updateKickerTip(KickerTip::Data&);

    k_dcop:
        void reconfigure();

    protected slots:
        void slotReconfigure() { reconfigure(); }
        void slotUpdate();
        void slotCalendarDeleted();
        void slotEnableCalendar();
        void slotCopyMenuActivated( int id );
        void contextMenuActivated(int result);
        void aboutToShowContextMenu();
        void fixupLayout();
        void globalPaletteChange();
        void setTimerTo60();

    protected:
        void toggleCalendar();
        void openContextMenu();
        void updateDateLabel(bool reLayout = true);
        void showZone(int z);
        void nextZone();
        void prevZone();
        void updateFollowBackground();
        
        void paletteChange(const QPalette &) { setBackground(); }
        void setBackground();
        void mousePressEvent(QMouseEvent *ev);
        void wheelEvent(QWheelEvent* e);
        bool eventFilter(QObject *, QEvent *);

        virtual void positionChange(Position p);

        QCString configFileName;
        DatePicker *_calendar;
        bool _disableCalendar;
        ClockWidget *_clock;
        QLabel *_date;
        QLabel *_dayOfWeek;
        QDate _lastDate;
        QTimer *_timer;
        QTimer *m_layoutTimer;
        int m_layoutDelay;
        bool m_followBackgroundSetting;
        bool m_dateFollowBackgroundSetting;
        int TZoffset;

        // settings
        Prefs *_prefs;
        Zone *zone;
        bool showDate;
        bool showDayOfWeek;
        bool m_updateOnTheMinute;
        QStringList _remotezonelist;
        KPopupMenu* menu;
        ClockAppletToolTip m_tooltip;
};


#endif
