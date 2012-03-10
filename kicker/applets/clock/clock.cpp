/************************************************************

Copyright (c) 1996-2002 the kicker authors. See file AUTHORS.

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

#include <cstdlib>
#include <ctime>
#include <time.h>

#include <qcheckbox.h>
#include <qcursor.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qclipboard.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qcombobox.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kcolorbutton.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kprocess.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstringhandler.h>
#include <kfiledialog.h>
#include <kfontrequester.h>
#include <kglobalsettings.h>
#include <kconfigdialogmanager.h>
#include <kcalendarsystem.h>
#include <kicontheme.h>
#include <kiconloader.h>

#include <global.h> // libkickermain

#include "clock.h"
#include "datepicker.h"
#include "zone.h"
#include "analog.h"
#include "digital.h"
#include "fuzzy.h"
#include "prefs.h"

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalogue("clockapplet");
        KGlobal::locale()->insertCatalogue("timezones"); // For time zone translations
        return new ClockApplet(configFile, KPanelApplet::Normal,
                               KPanelApplet::Preferences, parent, "clockapplet");
    }
}

// Settings

KConfigDialogSingle::KConfigDialogSingle(Zone *zone, QWidget *parent,
                                         const char *name, Prefs * prefs,
                                         KDialogBase::DialogType dialogType,
                                         bool modal) :
    KConfigDialog(parent, name, prefs, dialogType,
                  KDialogBase::Default | KDialogBase::Ok |
                  KDialogBase::Apply | KDialogBase::Cancel,
                  KDialogBase::Ok,
                  modal), _prefs(prefs)
{
    // As a temporary mesure until the kicker applet's app name is set to the
    // applets name so KDialogBase gets the right info.
    setPlainCaption(i18n("Configure - Clock"));
    setIcon(SmallIcon("date"));

    settings = new SettingsWidgetImp(prefs, zone, 0, "General");
    connect(settings->kcfg_Type, SIGNAL(activated(int)), SLOT(selectPage(int)));

    settings->kcfg_PlainBackgroundColor->setDefaultColor(KApplication::palette().active().background());
    settings->kcfg_DateBackgroundColor->setDefaultColor(KApplication::palette().active().background());

    // Digital
    digitalPage = new DigitalWidget(0, "DigitalClock");
    settings->widgetStack->addWidget(digitalPage, 1);
    digitalPage->kcfg_DigitalBackgroundColor->setDefaultColor(KApplication::palette().active().background());

    // Analog
    analogPage = new AnalogWidget(0, "AnalogClock");
    settings->widgetStack->addWidget(analogPage, 2);
    analogPage->kcfg_AnalogBackgroundColor->setDefaultColor(KApplication::palette().active().background());

    // Fuzzy
    fuzzyPage = new FuzzyWidget(0, "FuzzyClock");
    settings->widgetStack->addWidget(fuzzyPage, 3);
    fuzzyPage->kcfg_FuzzyBackgroundColor->setDefaultColor(KApplication::palette().active().background());

    connect(settings->kcfg_PlainShowDate, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));
    connect(settings->kcfg_PlainShowDayOfWeek, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));
    connect(digitalPage->kcfg_DigitalShowDate, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));
    connect(digitalPage->kcfg_DigitalShowDayOfWeek, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));
    connect(digitalPage->kcfg_DigitalShowDate, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));
    connect(analogPage->kcfg_AnalogShowDate, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));
    connect(analogPage->kcfg_AnalogShowDayOfWeek, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));
    connect(fuzzyPage->kcfg_FuzzyShowDate, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));
    connect(fuzzyPage->kcfg_FuzzyShowDayOfWeek, SIGNAL(toggled(bool)),
            SLOT(dateToggled()));

    addPage(settings, i18n("General"), QString::fromLatin1("package_settings"));
}

void KConfigDialogSingle::updateSettings()
{
    settings->OkApply();
}

void KConfigDialogSingle::updateWidgets()
{
    selectPage( _prefs->type() );
}

void KConfigDialogSingle::updateWidgetsDefault()
{
    KConfigSkeletonItem *item = _prefs->findItem("Type");
    item->swapDefault();
    selectPage( _prefs->type() );
    item->swapDefault();
    // This is ugly, but kcfg_Type does not have its correct setting
    // at this point in time.
    QTimer::singleShot(0, this, SLOT(dateToggled()));
}

void KConfigDialogSingle::selectPage(int p)
{
    settings->widgetStack->raiseWidget( p );
    dateToggled();
}

void KConfigDialogSingle::dateToggled()
{
    bool showDate;
    switch( settings->kcfg_Type->currentItem() )
    {
      case Prefs::EnumType::Plain:
         showDate = settings->kcfg_PlainShowDate->isChecked() ||
                    settings->kcfg_PlainShowDayOfWeek->isChecked();
         break;
      case Prefs::EnumType::Digital:
         showDate = digitalPage->kcfg_DigitalShowDate->isChecked() ||
                    digitalPage->kcfg_DigitalShowDayOfWeek->isChecked();
         break;
      case Prefs::EnumType::Analog:
         showDate = analogPage->kcfg_AnalogShowDate->isChecked() ||
                    analogPage->kcfg_AnalogShowDayOfWeek->isChecked();
         break;
      case Prefs::EnumType::Fuzzy:
      default:
         showDate = fuzzyPage->kcfg_FuzzyShowDate->isChecked() ||
                    fuzzyPage->kcfg_FuzzyShowDayOfWeek->isChecked();
         break;
    }
    settings->dateBox->setEnabled(showDate);
}

SettingsWidgetImp::SettingsWidgetImp(Prefs *p, Zone *z, QWidget* parent, const char* name, WFlags fl) :
    SettingsWidget(parent, name, fl), prefs(p), zone(z)
{
    zone->readZoneList(tzListView);
}

void SettingsWidgetImp::OkApply()
{
    zone->getSelectedZonelist(tzListView);
    zone->writeSettings();
}

//************************************************************


ClockWidget::ClockWidget(ClockApplet *applet, Prefs* prefs)
    : _applet(applet), _prefs(prefs), _force(false)
{}


ClockWidget::~ClockWidget()
{}


//************************************************************


PlainClock::PlainClock(ClockApplet *applet, Prefs *prefs, QWidget *parent, const char *name)
    : QLabel(parent, name), ClockWidget(applet, prefs)
{
    setBackgroundOrigin(AncestorOrigin);
    loadSettings();
    updateClock();
}


int PlainClock::preferedWidthForHeight(int ) const
{
    QString maxLengthTime = KGlobal::locale()->formatTime( QTime( 23, 59 ), _prefs->plainShowSeconds());
    return fontMetrics().width( maxLengthTime+2 );
}


int PlainClock::preferedHeightForWidth(int /*w*/) const
{
    return fontMetrics().lineSpacing();
}


void PlainClock::updateClock()
{
    QString newStr = KGlobal::locale()->formatTime(_applet->clockGetTime(), _prefs->plainShowSeconds());

    if (_force || newStr != _timeStr) {
        _timeStr = newStr;
        setText(_timeStr);
    }
}

void PlainClock::loadSettings()
{
    setFrameStyle(_prefs->plainShowFrame() ? Panel | Sunken : NoFrame);
    setAlignment(AlignVCenter | AlignHCenter | SingleLine);

    setFont(_prefs->plainFont());
}

bool PlainClock::showDate()
{
    return _prefs->plainShowDate();
}

bool PlainClock::showDayOfWeek()
{
    return _prefs->plainShowDayOfWeek();
}

//************************************************************


DigitalClock::DigitalClock(ClockApplet *applet, Prefs *prefs, QWidget *parent, const char *name)
    : QLCDNumber(parent, name), ClockWidget(applet, prefs)
{
    setWFlags(WNoAutoErase);
    setBackgroundOrigin(AncestorOrigin);
    loadSettings();
    updateClock();
}


DigitalClock::~DigitalClock()
{
    delete _buffer;
}


int DigitalClock::preferedWidthForHeight(int h) const
{
    if (h > 29) h = 29;
    if (h < 0) h = 0;
    return (numDigits()*h*5/11)+2;
}


int DigitalClock::preferedHeightForWidth(int w) const
{
    if (w < 0) w = 0;
    return((w / numDigits() * 2) + 6);
}


void DigitalClock::updateClock()
{
    static bool colon = true;
    QString newStr;
    QTime t(_applet->clockGetTime());

    int h = t.hour();
    int m = t.minute();
    int s = t.second();

    QString format("%02d");

    QString sep(!colon && _prefs->digitalBlink() ? " " : ":");

    if (_prefs->digitalShowSeconds())
        format += sep + "%02d";

    if (KGlobal::locale()->use12Clock()) {
        if (h > 12)
            h -= 12;
        else if( h == 0)
            h = 12;

        format.prepend("%2d" + sep);
    } else
        format.prepend("%02d" + sep);


    if (_prefs->digitalShowSeconds())
        newStr.sprintf(format.latin1(), h, m, s);
    else
        newStr.sprintf(format.latin1(), h, m);

    if (_force || newStr != _timeStr)
    {
        _timeStr = newStr;
        setUpdatesEnabled( FALSE );
        display(_timeStr);
        setUpdatesEnabled( TRUE );
        update();
    }
    
    if (_prefs->digitalBlink())
        colon = !colon;
}

void DigitalClock::loadSettings()
{
    setFrameStyle(_prefs->digitalShowFrame() ? Panel | Sunken : NoFrame);
    setMargin( 4 );
    setSegmentStyle(QLCDNumber::Flat);

    if (_prefs->digitalLCDStyle())
        lcdPattern = KIconLoader("clockapplet").loadIcon("lcd", KIcon::User);

    setNumDigits(_prefs->digitalShowSeconds() ? 8:5);

    _buffer = new QPixmap(width(), height());
}

void DigitalClock::paintEvent(QPaintEvent*)
{
    QPainter p(_buffer);

    if (_prefs->digitalLCDStyle())
    {
        p.drawTiledPixmap(0, 0, width(), height(), lcdPattern);
    }
    else if (_prefs->digitalBackgroundColor() !=
             KApplication::palette().active().background())
    {
        p.fillRect(0, 0, width(), height(), _prefs->digitalBackgroundColor());
    }
    else if (paletteBackgroundPixmap())
    {
        QPoint offset = backgroundOffset();
        p.drawTiledPixmap(0, 0, width(), height(), *paletteBackgroundPixmap(), offset.x(), offset.y());
    }
    else
    {
        p.fillRect(0, 0, width(), height(), _prefs->digitalBackgroundColor());
    }

    drawContents(&p);
    if (_prefs->digitalShowFrame())
    {
        drawFrame(&p);
    }

    p.end();
    bitBlt(this, 0, 0, _buffer, 0, 0);
}


// yes, the colors for the lcd-lock are hardcoded,
// but other colors would break the lcd-lock anyway
void DigitalClock::drawContents( QPainter * p)
{
    setUpdatesEnabled( FALSE );
    QPalette pal = palette();
    if (_prefs->digitalLCDStyle())
        pal.setColor( QColorGroup::Foreground, QColor(128,128,128));
    else
        pal.setColor( QColorGroup::Foreground, _prefs->digitalShadowColor());
    setPalette( pal );
    p->translate( +1, +1 );
    QLCDNumber::drawContents( p );
    if (_prefs->digitalLCDStyle())
        pal.setColor( QColorGroup::Foreground, Qt::black);
    else
        pal.setColor( QColorGroup::Foreground, _prefs->digitalForegroundColor());
    setPalette( pal );
    p->translate( -2, -2 );
    setUpdatesEnabled( TRUE );
    QLCDNumber::drawContents( p );
    p->translate( +1, +1 );
}


// reallocate buffer pixmap
void DigitalClock::resizeEvent ( QResizeEvent *)
{
    delete _buffer;
    _buffer = new QPixmap( width(), height() );
}


bool DigitalClock::showDate()
{
    return _prefs->digitalShowDate();
}

bool DigitalClock::showDayOfWeek()
{
    return _prefs->digitalShowDayOfWeek();
}


//************************************************************


AnalogClock::AnalogClock(ClockApplet *applet, Prefs *prefs, QWidget *parent, const char *name)
    : QFrame(parent, name), ClockWidget(applet, prefs), _spPx(NULL)
{
    setWFlags(WNoAutoErase);
    setBackgroundOrigin(AncestorOrigin);
    loadSettings();
}


AnalogClock::~AnalogClock()
{
    delete _spPx;
}

void AnalogClock::initBackgroundPixmap()
{
    //if no antialiasing, use pixmap as-is
    if (_prefs->analogAntialias() == 0)
    {
      lcdPattern = KIconLoader("clockapplet").loadIcon("lcd",KIcon::User);
      _bgScale = 1;
    }
    else
    {
        //make a scaled pixmap -- so when image is reduced it'll look "OK".
        _bgScale = _prefs->analogAntialias()+1;
        QImage bgImage = KIconLoader("clockapplet").loadIcon("lcd", KIcon::User).convertToImage();
        lcdPattern = QPixmap(bgImage.scale(bgImage.width() * _bgScale,
                             bgImage.height() * _bgScale));

    }
}

void AnalogClock::updateClock()
{
    if (!_force)
    {
        if (!_prefs->analogShowSeconds() && (_time.minute() == _applet->clockGetTime().minute()))
            return;
    }
    
    _time = _applet->clockGetTime();
    update();
}

void AnalogClock::loadSettings()
{
    if (_prefs->analogLCDStyle())
    {
        initBackgroundPixmap();
    }
/*  this may prevent flicker, but it also prevents transparency
    else
    {
        setBackgroundMode(NoBackground);
    }*/

    setFrameStyle(_prefs->analogShowFrame() ? Panel | Sunken : NoFrame);
    _time = _applet->clockGetTime();
    _spPx = new QPixmap(size().width() * _prefs->analogAntialias()+1,
                        size().height() * _prefs->analogAntialias()+1);

    update();
}

void AnalogClock::paintEvent( QPaintEvent * )
{
    if ( !isVisible() )
        return;

    int aaFactor = _prefs->analogAntialias()+1;
    int spWidth = size().width() * aaFactor;
    int spHeight = size().height() * aaFactor;

    if ((spWidth != _spPx->size().width()) ||
        (spHeight != _spPx->size().height()))
    {
        delete _spPx;
        _spPx = new QPixmap(spWidth, spHeight);
    }

    QPainter paint;
    paint.begin(_spPx);

    if (_prefs->analogLCDStyle())
    {
        if (_bgScale != aaFactor)
        {
            //check to see if antialiasing has changed -- bg pixmap will need
            //to be re-created
            initBackgroundPixmap();
        }

        paint.drawTiledPixmap(0, 0, spWidth, spHeight, lcdPattern);
    }
    else if (_prefs->analogBackgroundColor() != KApplication::palette().active().background())
    {
        _spPx->fill(_prefs->analogBackgroundColor());
    }
    else if (paletteBackgroundPixmap())
    {
        QPixmap bg(width(), height());
        QPainter p(&bg);
        QPoint offset = backgroundOffset();
        p.drawTiledPixmap(0, 0, width(), height(), *paletteBackgroundPixmap(), offset.x(), offset.y());
        p.end();
        QImage bgImage = bg.convertToImage().scale(spWidth, spHeight);
        paint.drawImage(0, 0, bgImage);
    }
    else
    {
       _spPx->fill(_prefs->analogBackgroundColor());
    }

    QPointArray pts;
    QPoint cp(spWidth / 2, spHeight / 2);

    int d = KMIN(spWidth,spHeight) - (10 * aaFactor);

    if (_prefs->analogLCDStyle()) 
    {
        paint.setPen( QPen(QColor(100,100,100), aaFactor) );
        paint.setBrush( QColor(100,100,100) );
    }
    else
    {
        paint.setPen( QPen(_prefs->analogShadowColor(), aaFactor) );
        paint.setBrush( _prefs->analogShadowColor() );
    }

    paint.setViewport(2,2,spWidth,spHeight);

    for ( int c=0 ; c < 2 ; c++ ) {
        QWMatrix matrix;
        matrix.translate( cp.x(), cp.y());
        matrix.scale( d/1000.0F, d/1000.0F );

        // hour
        float h_angle = 30*(_time.hour()%12-3) + _time.minute()/2;
        matrix.rotate( h_angle );
        paint.setWorldMatrix( matrix );
        pts.setPoints( 4, -20,0,  0,-20, 300,0, 0,20 );
        paint.drawPolygon( pts );
        matrix.rotate( -h_angle );

        // minute
        float m_angle = (_time.minute()-15)*6;
        matrix.rotate( m_angle );
        paint.setWorldMatrix( matrix );
        pts.setPoints( 4, -10,0, 0,-10, 400,0, 0,10 );
        paint.drawPolygon( pts );
        matrix.rotate( -m_angle );

        if (_prefs->analogShowSeconds()) {   // second
            float s_angle = (_time.second()-15)*6;
            matrix.rotate( s_angle );
            paint.setWorldMatrix( matrix );
            pts.setPoints(4,0,0,0,0,400,0,0,0);
            paint.drawPolygon( pts );
            matrix.rotate( -s_angle );
        }

        QWMatrix matrix2;
        matrix2.translate( cp.x(), cp.y());
        matrix2.scale( d/1000.0F, d/1000.0F );

        // quadrante
        for ( int i=0 ; i < 12 ; i++ ) {
            paint.setWorldMatrix( matrix2 );
            paint.drawLine( 460,0, 500,0 ); // draw hour lines
            // paint.drawEllipse( 450, -15, 30, 30 );
            matrix2.rotate( 30 );
        }

        if (_prefs->analogLCDStyle()) {
            paint.setPen( QPen(Qt::black, aaFactor) );
            paint.setBrush( Qt::black );
        } else {
            paint.setPen( QPen(_prefs->analogForegroundColor(), aaFactor) );
            paint.setBrush( _prefs->analogForegroundColor() );
        }

        paint.setViewport(0,0,spWidth,spHeight);
    }
    paint.end();

    QPainter paintFinal;
    paintFinal.begin(this);

    if (aaFactor != 1)
    {
        QImage spImage = _spPx->convertToImage();
        QImage displayImage = spImage.smoothScale(size());

        paintFinal.drawImage(0, 0, displayImage);
    }
    else
    {
        paintFinal.drawPixmap(0, 0, *_spPx);
    }

    if (_prefs->analogShowFrame())
    {
        drawFrame(&paintFinal);
    }
}


// the background pixmap disappears during a style change
void AnalogClock::styleChange(QStyle &)
{
    if (_prefs->analogLCDStyle())
    {
       initBackgroundPixmap();
    }
}

bool AnalogClock::showDate()
{
    return _prefs->analogShowDate();
}

bool AnalogClock::showDayOfWeek()
{
    return _prefs->analogShowDayOfWeek();
}


//************************************************************


FuzzyClock::FuzzyClock(ClockApplet *applet, Prefs *prefs, QWidget *parent, const char *name)
    : QFrame(parent, name), ClockWidget(applet, prefs)
{
    setBackgroundOrigin(AncestorOrigin);
    loadSettings();
    hourNames   << i18n("hour","one") << i18n("hour","two")
                << i18n("hour","three") << i18n("hour","four") << i18n("hour","five")
                << i18n("hour","six") << i18n("hour","seven") << i18n("hour","eight")
                << i18n("hour","nine") << i18n("hour","ten") << i18n("hour","eleven")
                << i18n("hour","twelve");

    // xgettext:no-c-format
    normalFuzzy << i18n("%0 o'clock") // xgettext:no-c-format
                << i18n("five past %0") // xgettext:no-c-format
                << i18n("ten past %0") // xgettext:no-c-format
                << i18n("quarter past %0") // xgettext:no-c-format
                << i18n("twenty past %0") // xgettext:no-c-format
                << i18n("twenty five past %0") // xgettext:no-c-format
                << i18n("half past %0") // xgettext:no-c-format
                << i18n("twenty five to %1") // xgettext:no-c-format
                << i18n("twenty to %1") // xgettext:no-c-format
                << i18n("quarter to %1") // xgettext:no-c-format
                << i18n("ten to %1") // xgettext:no-c-format
                << i18n("five to %1") // xgettext:no-c-format
                << i18n("%1 o'clock");

    // xgettext:no-c-format
    normalFuzzyOne << i18n("one","%0 o'clock") // xgettext:no-c-format
                   << i18n("one","five past %0") // xgettext:no-c-format
                   << i18n("one","ten past %0") // xgettext:no-c-format
                   << i18n("one","quarter past %0") // xgettext:no-c-format
                   << i18n("one","twenty past %0") // xgettext:no-c-format
                   << i18n("one","twenty five past %0") // xgettext:no-c-format
                   << i18n("one","half past %0") // xgettext:no-c-format
                   << i18n("one","twenty five to %1") // xgettext:no-c-format
                   << i18n("one","twenty to %1") // xgettext:no-c-format
                   << i18n("one","quarter to %1") // xgettext:no-c-format
                   << i18n("one","ten to %1") // xgettext:no-c-format
                   << i18n("one","five to %1") // xgettext:no-c-format
                   << i18n("one","%1 o'clock");

    dayTime << i18n("Night")
            << i18n("Early morning") << i18n("Morning") << i18n("Almost noon")
            << i18n("Noon") << i18n("Afternoon") << i18n("Evening")
            << i18n("Late evening");

    _time = _applet->clockGetTime();
    alreadyDrawing=false;
    update();
}

void FuzzyClock::deleteMyself()
{
    if(alreadyDrawing) // try again later
        QTimer::singleShot(1000, this, SLOT(deleteMyself()));
    else
        delete this;
}


int FuzzyClock::preferedWidthForHeight(int ) const
{
    QFontMetrics fm(_prefs->fuzzyFont());
    return fm.width(_timeStr) + 8;
}


int FuzzyClock::preferedHeightForWidth(int ) const
{
    QFontMetrics fm(_prefs->fuzzyFont());
    return fm.width(_timeStr) + 8;
}


void FuzzyClock::updateClock()
{
    if (!_force)
    {
        if (_time.hour() == _applet->clockGetTime().hour() &&
            _time.minute() == _applet->clockGetTime().minute())
        return;
    }

    _time = _applet->clockGetTime();
    update();
}

void FuzzyClock::loadSettings()
{
    setFrameStyle(_prefs->fuzzyShowFrame() ? Panel | Sunken : 0);
}

void FuzzyClock::drawContents(QPainter *p)
{
    if (!isVisible())
        return;

    if(!_applet)
        return;

    alreadyDrawing = true;
    QString newTimeStr;

    if (_prefs->fuzzyness() == 1 || _prefs->fuzzyness() == 2) {
      int minute = _time.minute();
      int sector = 0;
      int realHour = 0;

      if (_prefs->fuzzyness() == 1) {
          if (minute > 2)
              sector = (minute - 3) / 5 + 1;
      } else {
          if (minute > 6)
              sector = ((minute - 7) / 15 + 1) * 3;
      }

      newTimeStr = normalFuzzy[sector];
      int phStart = newTimeStr.find("%");
      if (phStart >= 0) { // protect yourself from translations
          int phLength = newTimeStr.find(" ", phStart) - phStart;

          // larrosa: we want the exact length, in case the translation needs it,
          // in other case, we would cut off the end of the translation.
          if (phLength < 0) phLength = newTimeStr.length() - phStart;
          int deltaHour = newTimeStr.mid(phStart + 1, phLength - 1).toInt();

          if ((_time.hour() + deltaHour) % 12 > 0)
              realHour = (_time.hour() + deltaHour) % 12 - 1;
          else
              realHour = 12 - ((_time.hour() + deltaHour) % 12 + 1);
          if (realHour==0) {
              newTimeStr = normalFuzzyOne[sector];
              phStart = newTimeStr.find("%");
              // larrosa: Note that length is the same,
              // so we only have to update phStart
          }
          if (phStart >= 0)
              newTimeStr.replace(phStart, phLength, hourNames[realHour]);
          newTimeStr.replace(0, 1, QString(newTimeStr.at(0).upper()));
      }
    } else if (_prefs->fuzzyness() == 3) {
        newTimeStr = dayTime[_time.hour() / 3];
    } else {
        int dow = _applet->clockGetDate().dayOfWeek();

        if (dow == 1)
            newTimeStr = i18n("Start of week");
        else if (dow >= 2 && dow <= 4)
            newTimeStr = i18n("Middle of week");
        else if (dow == 5)
            newTimeStr = i18n("End of week");
        else
            newTimeStr = i18n("Weekend!");
    }

    if (_timeStr != newTimeStr) {
        _timeStr = newTimeStr;
        _applet->resizeRequest();
    }

    p->setFont(_prefs->fuzzyFont());
    p->setPen(_prefs->fuzzyForegroundColor());
    if (_applet->getOrientation() == Vertical) {
        p->rotate(90);
        p->drawText(4, -2, height() - 8, -(width()) + 2, AlignCenter, _timeStr);
    } else {
        p->drawText(4, 2, width() - 8, height() - 4, AlignCenter, _timeStr);
    }
    alreadyDrawing = false;
}

bool FuzzyClock::showDate()
{
    return _prefs->fuzzyShowDate();
}

bool FuzzyClock::showDayOfWeek()
{
    return _prefs->fuzzyShowDayOfWeek();
}


//************************************************************


ClockApplet::ClockApplet(const QString& configFile, Type t, int actions,
                         QWidget *parent, const char *name)
    : KPanelApplet(configFile, t, actions, parent, name),
      _calendar(0),
      _disableCalendar(false),
      _clock(0),
      _timer(new QTimer(this)),
      m_layoutTimer(new QTimer(this)),
      m_layoutDelay(0),
      m_followBackgroundSetting(true),
      m_dateFollowBackgroundSetting(true),
      TZoffset(0),
      _prefs(new Prefs(sharedConfig())),
      zone(new Zone(config())),
      menu(0),
      m_tooltip(this)
{
    DCOPObject::setObjId("ClockApplet");
    _prefs->readConfig();
    configFileName = configFile.latin1();
    setBackgroundOrigin(AncestorOrigin);

    _dayOfWeek = new QLabel(this);
    _dayOfWeek->setAlignment(AlignVCenter | AlignHCenter | WordBreak);
    _dayOfWeek->setBackgroundOrigin(AncestorOrigin);
    _dayOfWeek->installEventFilter(this);   // catch mouse clicks

    _date = new QLabel(this);
    _date->setAlignment(AlignVCenter | AlignHCenter | WordBreak);
    _date->setBackgroundOrigin(AncestorOrigin);
    _date->installEventFilter(this);   // catch mouse clicks

    connect(m_layoutTimer, SIGNAL(timeout()), this, SLOT(fixupLayout()));
    connect(_timer, SIGNAL(timeout()), SLOT(slotUpdate()));
    connect(kapp, SIGNAL(kdisplayPaletteChanged()), SLOT(globalPaletteChange()));

    reconfigure();    // initialize clock widget
    slotUpdate();

    if (kapp->authorizeKAction("kicker_rmb"))
    {
        menu = new KPopupMenu();
        connect(menu, SIGNAL(aboutToShow()), SLOT(aboutToShowContextMenu()));
        connect(menu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
        setCustomMenu(menu);
    }

    installEventFilter(KickerTip::the());
}


ClockApplet::~ClockApplet()
{
    //reverse for the moment
    KGlobal::locale()->removeCatalogue("clockapplet");
    KGlobal::locale()->removeCatalogue("timezones"); // For time zone translations

    if (_calendar)
    {
        // we have to take care of the calendar closing first before deleting
        // the prefs
        _calendar->close();
    }

    zone->writeSettings();

    delete _prefs; _prefs = 0;
    delete zone; zone = 0;
    delete menu; menu = 0;
    config()->sync();
}

int ClockApplet::widthForHeight(int h) const
{
    if (orientation() == Qt::Vertical)
    {
        return width();
    }

    int shareDateHeight = 0, shareDayOfWeekHeight = 0;
    bool dateToSide = (h < 32);
    bool mustShowDate = showDate || (zone->zoneIndex() != 0);
    if (mustShowDate)
    {
        _date->setAlignment(AlignVCenter | AlignHCenter);
        if (!dateToSide)
        {
            shareDateHeight = _date->sizeHint().height();
        }
    }
    if (showDayOfWeek)
    {
        _dayOfWeek->setAlignment(AlignVCenter | AlignHCenter);
        if (!dateToSide)
        {
            shareDayOfWeekHeight = _dayOfWeek->sizeHint().height();
        }
    }

    int clockWidth = _clock->preferedWidthForHeight(KMAX(0, h - shareDateHeight - shareDayOfWeekHeight));
    int w = clockWidth;
    if (!mustShowDate && !showDayOfWeek)
    {
        // resize the date widgets in case the are to the left of the clock
        _clock->widget()->setFixedSize(w, h);
        _clock->widget()->move(0,0);
        _dayOfWeek->move(clockWidth + 4, 0);
        _date->move(clockWidth + 4, 0);
    }
    else
    {
        int dateWidth = mustShowDate ? _date->sizeHint().width() + 4 : 0;
        int dayOfWeekWidth = showDayOfWeek ? _dayOfWeek->sizeHint().width() + 4 : 0;

        if (dateToSide)
        {
            w += dateWidth + dayOfWeekWidth;
            bool dateFirst = false;

            if (mustShowDate)
            {
                // if the date format STARTS with a year, assume it's in descending
                // order and should therefore PRECEED the date.
                QString dateFormat = KGlobal::locale()->dateFormatShort();
                dateFirst = dateFormat.at(1) == 'y' || dateFormat.at(1) == 'Y';
            }

            if (dateFirst)
            {
                _date->setFixedSize(dateWidth, h);
                _date->move(0, 0);

                if (showDayOfWeek)
                {
                    _dayOfWeek->setFixedSize(dayOfWeekWidth, h);
                    _dayOfWeek->move(dateWidth, 0);
                }

                _clock->widget()->setFixedSize(clockWidth, h);
                _clock->widget()->move(dateWidth + dayOfWeekWidth, 0);
            }
            else
            {
                _clock->widget()->setFixedSize(clockWidth, h);
                _clock->widget()->move(0,0);

                if (showDayOfWeek)
                {
                    _dayOfWeek->setFixedSize(dayOfWeekWidth, h);
                    _dayOfWeek->move(clockWidth, 0);
                }

                if (mustShowDate)
                {
                    _date->setFixedSize(dateWidth, h);
                    _date->move(clockWidth + dayOfWeekWidth, 0);
                }
            }
        }
        else
        {
            w = KMAX(KMAX(w, dateWidth), dayOfWeekWidth);

            _clock->widget()->setFixedSize(w, h - shareDateHeight - shareDayOfWeekHeight);
            _clock->widget()->setMinimumSize(w, h - shareDateHeight - shareDayOfWeekHeight);
            _clock->widget()->move(0, 0);
            if (showDayOfWeek)
            {
                _dayOfWeek->setFixedSize(w, _dayOfWeek->sizeHint().height());
                _dayOfWeek->move(0, _clock->widget()->height());
            }

            if (mustShowDate)
            {
                _date->setFixedSize(w, _date->sizeHint().height());
                _date->move(0, _clock->widget()->height() + shareDayOfWeekHeight);
            }
        }
    }

    return w;
}

int ClockApplet::heightForWidth(int w) const
{
    if (orientation() == Qt::Horizontal)
    {
        return height();
    }

    int clockHeight = _clock->preferedHeightForWidth(w);
    bool mustShowDate = showDate || (zone->zoneIndex() != 0);

    _clock->widget()->setFixedSize(w, clockHeight);

    // add 4 pixels in height for each of date+dayOfWeek, if visible
    if (showDayOfWeek)
    {
        if (_dayOfWeek->minimumSizeHint().width() > w)
        {
            _dayOfWeek->setAlignment(AlignVCenter | WordBreak);
        }
        else
        {
            _dayOfWeek->setAlignment(AlignVCenter | AlignHCenter | WordBreak);
        }

        _dayOfWeek->setFixedSize(w, _dayOfWeek->minimumSizeHint().height());
        _dayOfWeek->move(0, clockHeight);

        clockHeight += _dayOfWeek->height();
    }

    if (mustShowDate)
    {
        // yes, the const_cast is ugly, but this is to ensure that we
        // get a proper date label in the case that we munged it for
        // display on panel that is too narrow and then they made it wider
        const_cast<ClockApplet*>(this)->updateDateLabel(false);

        if (_date->minimumSizeHint().width() > w)
        {
            QString dateStr = _date->text();
            // if we're too wide to fit, replace the first non-digit from the end with a space
            int p = dateStr.findRev(QRegExp("[^0-9]"));
            if (p > 0)
            {
                _date->setText(dateStr.insert(p, '\n'));
            }
        }

        if (_date->minimumSizeHint().width() > w)
        {
            _date->setAlignment(AlignVCenter | WordBreak);
        }
        else
        {
            _date->setAlignment(AlignVCenter | AlignHCenter | WordBreak);
        }
        _date->setFixedSize(w, _date->heightForWidth(w));
        _date->move(0, clockHeight);

        clockHeight += _date->height();
    }

    return clockHeight;
}

void ClockApplet::preferences()
{
    preferences(false);
}

void ClockApplet::preferences(bool timezone)
{
  KConfigDialogSingle *dialog = dynamic_cast<KConfigDialogSingle*>(KConfigDialog::exists(configFileName));

  if (!dialog)
  {
    dialog = new KConfigDialogSingle(zone, this, configFileName, _prefs, KDialogBase::Swallow);
    connect(dialog, SIGNAL(settingsChanged()), this, SLOT(slotReconfigure()));
  }

  if (timezone)
  {
      dialog->settings->tabs->setCurrentPage(1);
  }

  dialog->show();
}

void ClockApplet::updateFollowBackground()
{
    QColor globalBgroundColor = KApplication::palette().active().background();
    QColor bgColor;
    
    switch (_prefs->type())
    {
        case Prefs::EnumType::Plain:
            bgColor = _prefs->plainBackgroundColor();
            break;
        case Prefs::EnumType::Analog:
            bgColor = _prefs->analogBackgroundColor();
            break;
        case Prefs::EnumType::Fuzzy:
            bgColor = _prefs->fuzzyBackgroundColor();
            break;
        case Prefs::EnumType::Digital:
        default:
            bgColor = _prefs->digitalBackgroundColor();
            break;
    }
    
    m_followBackgroundSetting = (bgColor == globalBgroundColor);
    
    bgColor = _prefs->dateBackgroundColor();
    m_dateFollowBackgroundSetting = (bgColor == globalBgroundColor);
}

// DCOP interface
void ClockApplet::reconfigure()
{
    _timer->stop();

    // ugly workaround for FuzzyClock: sometimes FuzzyClock
    // hasn't finished drawing when getting deleted, so we
    // ask FuzzyClock to delete itself appropriately
    if (_clock && _clock->widget()->inherits("FuzzyClock"))
    {
        FuzzyClock* f = static_cast<FuzzyClock*>(_clock);
        f->deleteMyself();
    }
    else
    {
        delete _clock;
    }

    int shortInterval = 500;
    int updateInterval = 0;
    
    switch (_prefs->type())
    {
        case Prefs::EnumType::Plain:
            _clock = new PlainClock(this, _prefs, this);
            if (_prefs->plainShowSeconds())
                updateInterval = shortInterval;
            break;
        case Prefs::EnumType::Analog:
            _clock = new AnalogClock(this, _prefs, this);
            if (_prefs->analogShowSeconds())
                updateInterval = shortInterval;
            break;
        case Prefs::EnumType::Fuzzy:
            _clock = new FuzzyClock(this, _prefs, this);
            break;
        case Prefs::EnumType::Digital:
        default:
            _clock = new DigitalClock(this, _prefs, this);
            if (_prefs->digitalShowSeconds() || _prefs->digitalBlink())
                updateInterval = shortInterval;
            break;
    }

    m_updateOnTheMinute = updateInterval != shortInterval;
    if (m_updateOnTheMinute)
    {
        connect(_timer, SIGNAL(timeout()), this, SLOT(setTimerTo60()));
        updateInterval = ((60 - clockGetTime().second()) * 1000) + 500;
    }
    else
    {
        // in case we reconfigure to show seconds but setTimerTo60 is going to be called
        // we need to make sure to disconnect this so we don't end up updating only once
        // a minute ;)
        disconnect(_timer, SIGNAL(timeout()), this, SLOT(setTimerTo60()));
    }

    _timer->start(updateInterval);

    // See if the clock wants to show the date.
    showDate = _clock->showDate();
    if (showDate)
    {
        TZoffset = zone->calc_TZ_offset(zone->zone(), true);
        updateDateLabel();
    }
    
    updateFollowBackground();
    setBackground();

    // FIXME: this means you can't have a transparent clock but a non-transparent
    //        date or day =/

    _clock->widget()->installEventFilter(this);   // catch mouse clicks
    _clock->widget()->show();

    _clock->forceUpdate(); /* force repaint */

    if (showDayOfWeek)
    {
        _dayOfWeek->show();
    }
    else
    {
        _dayOfWeek->hide();
    }

    if (showDate || (zone->zoneIndex() != 0))
    {
        _date->show();
    }
    else
    {
        _date->hide();
    }

    emit(updateLayout());

    showZone(zone->zoneIndex());
}

void ClockApplet::setTimerTo60()
{
//    kdDebug() << "setTimerTo60" << endl;
    disconnect(_timer, SIGNAL(timeout()), this, SLOT(setTimerTo60()));
    _timer->changeInterval(60000);
}

void ClockApplet::setBackground()
{
    QColor globalBgroundColor = KApplication::palette().active().background();
    QColor fgColor, bgColor;
    
    if (!_clock)
        return;
    
    switch (_prefs->type())
    {
        case Prefs::EnumType::Plain:
            bgColor = _prefs->plainBackgroundColor();
            fgColor = _prefs->plainForegroundColor();
            break;
        case Prefs::EnumType::Analog:
            bgColor = _prefs->analogBackgroundColor();
            fgColor = _prefs->analogForegroundColor();
            break;
        case Prefs::EnumType::Fuzzy:
            bgColor = _prefs->fuzzyBackgroundColor();
            fgColor = _prefs->fuzzyForegroundColor();
            break;
        case Prefs::EnumType::Digital:
        default:
            bgColor = _prefs->digitalBackgroundColor();
            fgColor = _prefs->digitalForegroundColor();
            break;
    }
    
    if (!m_followBackgroundSetting)
        _clock->widget()->setPaletteBackgroundColor(bgColor);
    else
        _clock->widget()->unsetPalette();
    _clock->widget()->setPaletteForegroundColor(fgColor);
    
    bgColor = _prefs->dateBackgroundColor();
        
    // See if the clock wants to show the day of week.
    // use same font/color as for date
    showDayOfWeek = _clock->showDayOfWeek();
    if (showDayOfWeek)
    {
        _dayOfWeek->setFont(_prefs->dateFont());
        
        if (!m_dateFollowBackgroundSetting)
            _dayOfWeek->setBackgroundColor(bgColor);
        else
            _dayOfWeek->unsetPalette();
        _dayOfWeek->setPaletteForegroundColor(_prefs->dateForegroundColor());
    }
    
    // See if the clock wants to show the date.
    showDate = _clock->showDate();
    _date->setFont(_prefs->dateFont());
    
    if (!m_dateFollowBackgroundSetting)
        _date->setPaletteBackgroundColor(bgColor);
    else
        _date->unsetPalette();
    _date->setPaletteForegroundColor(_prefs->dateForegroundColor());
}

void ClockApplet::globalPaletteChange()
{
    if (!m_dateFollowBackgroundSetting && !m_followBackgroundSetting)
        return;
    
    QColor globalBgroundColor = KApplication::palette().active().background();
    
    if (m_dateFollowBackgroundSetting)
        _prefs->setDateBackgroundColor(globalBgroundColor);
    
    if (m_followBackgroundSetting)
    {
        // we need to makes sure we have the background color synced!
        // otherwise when we switch color schemes again or restart kicker
        // it might come back non-transparent
        switch (_prefs->type())
        {
            case Prefs::EnumType::Plain:
                _prefs->setPlainBackgroundColor(globalBgroundColor);
                break;
            case Prefs::EnumType::Analog:
                _prefs->setAnalogBackgroundColor(globalBgroundColor);
                break;
            case Prefs::EnumType::Fuzzy:
                _prefs->setFuzzyBackgroundColor(globalBgroundColor);
                break;
            case Prefs::EnumType::Digital:
            default:
                _prefs->setDigitalBackgroundColor(globalBgroundColor);
                break;
        }
     }
     
    _prefs->writeConfig();
}

void ClockApplet::slotUpdate()
{
    if (_lastDate != clockGetDate())
    {
        updateDateLabel();
    }

    if (m_updateOnTheMinute)
    {
        // catch drift so we're never more than a few s out
        int seconds = clockGetTime().second();
//        kdDebug() << "checking for drift: " << seconds << endl;

        if (seconds > 2)
        {
            connect(_timer, SIGNAL(timeout()), this, SLOT(setTimerTo60()));
            _timer->changeInterval(((60 - seconds) * 1000) + 500);
        }
    }
    _clock->updateClock();
    KickerTip::Client::updateKickerTip();
}

void ClockApplet::slotCalendarDeleted()
{
    _calendar = 0L;
    // don't reopen the calendar immediately ...
    _disableCalendar = true;
    QTimer::singleShot(100, this, SLOT(slotEnableCalendar()));

    // we are free to show a tip know :)
    installEventFilter(KickerTip::the());
}


void ClockApplet::slotEnableCalendar()
{
    _disableCalendar = false;
}

void ClockApplet::toggleCalendar()
{
    if (_calendar && !_disableCalendar)
    {
        // calls slotCalendarDeleted which does the cleanup for us
        _calendar->close();
        return;
    }

    if (_calendar || _disableCalendar)
    {
        return;
    }

    KickerTip::the()->untipFor(this);
    removeEventFilter(KickerTip::the());

    _calendar = new DatePicker(this, _lastDate, _prefs);
    connect(_calendar, SIGNAL(destroyed()), SLOT(slotCalendarDeleted()));

    QSize size = _prefs->calendarSize();

    if (size != QSize())
    {
        _calendar->resize(size);
    }
    else
    {
        _calendar->adjustSize();
    }

    // make calendar fully visible
    QPoint popupAt = KickerLib::popupPosition(popupDirection(),
                                              _calendar,
                                              this);
    _calendar->move(popupAt);
    _calendar->show();
    _calendar->setFocus();
}


void ClockApplet::openContextMenu()
{
    if (!menu || !kapp->authorizeKAction("kicker_rmb"))
        return;

    menu->exec( QCursor::pos() );
}

void ClockApplet::contextMenuActivated(int result)
{
    if ((result >= 0) && (result < 100))
    {
        _prefs->setType(result);
        _prefs->writeConfig();
        reconfigure();
        return;
    };

    if ((result >= 500) && (result < 600))
    {
        showZone(result-500);
        zone->writeSettings();
        return;
    };

    KProcess proc;
    switch (result)
    {
        case 102:
            preferences();
            break;
        case 103:
            proc << locate("exe", "kdesu");
            proc << "--nonewdcop";
            proc << QString("%1 kde-clock.desktop --lang %2")
                .arg(locate("exe", "kcmshell"))
                .arg(KGlobal::locale()->language());
            proc.start(KProcess::DontCare);
            break;
        case 104:
            proc << locate("exe", "kcmshell");
            proc << "kde-language.desktop";
            proc.start(KProcess::DontCare);
            break;
        case 110:
            preferences(true);
            break;
    } /* switch() */
}

void ClockApplet::aboutToShowContextMenu()
{
    bool bImmutable = config()->isImmutable();

    menu->clear();
    menu->insertTitle( SmallIcon( "clock" ), i18n( "Clock" ) );

    KLocale *loc = KGlobal::locale();
    QDateTime dt = QDateTime::currentDateTime();
    dt = dt.addSecs(TZoffset);

    KPopupMenu *copyMenu = new KPopupMenu( menu );
    copyMenu->insertItem(loc->formatDateTime(dt), 201);
    copyMenu->insertItem(loc->formatDate(dt.date()), 202);
    copyMenu->insertItem(loc->formatDate(dt.date(), true), 203);
    copyMenu->insertItem(loc->formatTime(dt.time()), 204);
    copyMenu->insertItem(loc->formatTime(dt.time(), true), 205);
    copyMenu->insertItem(dt.date().toString(), 206);
    copyMenu->insertItem(dt.time().toString(), 207);
    copyMenu->insertItem(dt.toString(), 208);
    copyMenu->insertItem(dt.toString("yyyy-MM-dd hh:mm:ss"), 209);
    connect( copyMenu, SIGNAL( activated(int) ), this, SLOT( slotCopyMenuActivated(int) ) );

    if (!bImmutable)
    {
        KPopupMenu *zoneMenu = new KPopupMenu( menu );
        connect(zoneMenu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
        for (int i = 0; i <= zone->remoteZoneCount(); i++)
        {
            if (i == 0)
            {
                zoneMenu->insertItem(i18n("Local Timezone"), 500 + i);
            }
            else
            {
                zoneMenu->insertItem(i18n(zone->zone(i).utf8()).replace("_", " "), 500 + i);
            }
        }
        zoneMenu->setItemChecked(500 + zone->zoneIndex(),true);
        zoneMenu->insertSeparator();
        zoneMenu->insertItem(SmallIcon("configure"), i18n("&Configure Timezones..."), 110);

        KPopupMenu *type_menu = new KPopupMenu(menu);
        connect(type_menu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
        type_menu->insertItem(i18n("&Plain"), Prefs::EnumType::Plain, 1);
        type_menu->insertItem(i18n("&Digital"), Prefs::EnumType::Digital, 2);
        type_menu->insertItem(i18n("&Analog"), Prefs::EnumType::Analog, 3);
        type_menu->insertItem(i18n("&Fuzzy"), Prefs::EnumType::Fuzzy, 4);
        type_menu->setItemChecked(_prefs->type(),true);

        menu->insertItem(i18n("&Type"), type_menu, 101, 1);
        menu->insertItem(i18n("Show Time&zone"), zoneMenu, 110, 2);
        if (kapp->authorize("user/root"))
        {
            menu->insertItem(SmallIcon("date"), i18n("&Adjust Date && Time..."), 103, 4);
        }
        menu->insertItem(SmallIcon("kcontrol"), i18n("Date && Time &Format..."), 104, 5);
    }

    menu->insertItem(SmallIcon("editcopy"), i18n("C&opy to Clipboard"), copyMenu, 105, 6);
    if (!bImmutable)
    {
        menu->insertSeparator(7);
        menu->insertItem(SmallIcon("configure"), i18n("&Configure Clock..."), 102, 8);
    }
}


void ClockApplet::slotCopyMenuActivated( int id )
{
    QPopupMenu *m = (QPopupMenu *) sender();
    QString s = m->text(id);
    QApplication::clipboard()->setText(s);
}

QTime ClockApplet::clockGetTime()
{
    return QTime::currentTime().addSecs(TZoffset);
}

QDate ClockApplet::clockGetDate()
{
    return QDateTime::currentDateTime().addSecs(TZoffset).date();
}

void ClockApplet::showZone(int z)
{
    zone->setZone(z);
    TZoffset = zone->calc_TZ_offset( zone->zone() );
    updateDateLabel();
    _clock->forceUpdate(); /* force repaint */
}

void ClockApplet::nextZone()
{
    zone->nextZone();
    showZone(zone->zoneIndex());
}

void ClockApplet::prevZone()
{
    zone->prevZone();
    showZone(zone->zoneIndex());
}

void ClockApplet::mousePressEvent(QMouseEvent *ev)
{
    switch (ev->button()) 
    {
        case QMouseEvent::LeftButton:
            toggleCalendar();
            break;
        case QMouseEvent::RightButton:
            openContextMenu();
            break;
        case QMouseEvent::MidButton:
            nextZone();
            QToolTip::remove(_clock->widget());
            break;
        default:
            break;
    }
}

void ClockApplet::wheelEvent(QWheelEvent* e)
{
    if (e->delta() < 0)
    {
        prevZone();
    }
    else
    {
        nextZone();
    }

    QToolTip::remove(_clock->widget());
    KickerTip::Client::updateKickerTip();
}

// catch the mouse clicks of our child widgets
bool ClockApplet::eventFilter( QObject *o, QEvent *e )
{
    if (( o == _clock->widget() || o == _date || o == _dayOfWeek) &&
        e->type() == QEvent::MouseButtonPress )
    {
        mousePressEvent(static_cast<QMouseEvent*>(e) );
        return true;
    }

    return KPanelApplet::eventFilter(o, e);
}

void ClockApplet::positionChange(Position p)
{
    KPanelApplet::positionChange(p);
    reconfigure();
}

void ClockApplet::updateDateLabel(bool reLayout)
{
    _lastDate = clockGetDate();
    _dayOfWeek->setText(KGlobal::locale()->calendar()->weekDayName(_lastDate));

    if (zone->zoneIndex() != 0)
    {
        QString zone_s = i18n(zone->zone().utf8());
        _date->setText(zone_s.mid(zone_s.find('/') + 1).replace("_", " "));
        _date->setShown(true);
    }
    else
    {
        QString dateStr = KGlobal::locale()->formatDate(_lastDate, true);
        _date->setText(dateStr);
        _date->setShown(showDate);
    }

    if (reLayout)
    {
        if (_calendar && _lastDate != _calendar->date())
        {
            _calendar->setDate(_lastDate);
        }

        m_layoutTimer->stop();
        m_layoutTimer->start(m_layoutDelay, true);
    }
}

void ClockApplet::updateKickerTip(KickerTip::Data& data)
{
    int zoneCount = zone->remoteZoneCount();

    QString activeZone = zone->zone();
    if (zoneCount == 0)
    {
        QString _time = KGlobal::locale()->formatTime(clockGetTime(),
                                                    _prefs->plainShowSeconds());
        QString _date = KGlobal::locale()->formatDate(clockGetDate(), false);
        data.message = _time;
        data.subtext = _date;

        if (!activeZone.isEmpty())
        {
            activeZone = i18n(activeZone.utf8());
            data.subtext.append("<br>").append(activeZone.mid(activeZone.find('/') + 1).replace("_", " "));
        }
    }
    else
    {
        int activeIndex = zone->zoneIndex();

        for (int i = 0; i <= zone->remoteZoneCount(); i++)
        {
            QString m_zone = zone->zone(i);
            TZoffset = zone->calc_TZ_offset(m_zone);

            if (!m_zone.isEmpty())
            {
                m_zone = i18n(m_zone.utf8()); // ensure it gets translated
            }

            QString _time = KGlobal::locale()->formatTime(clockGetTime(),
                                                          _prefs->plainShowSeconds());
            QString _date = KGlobal::locale()->formatDate(clockGetDate(), false);

            if (activeIndex == i)
            {
                data.message = m_zone.mid(m_zone.find('/') + 1).replace("_", " ");
                data.message += "  " + _time + "<br>" + _date;
            }
            else
            {
                if (i == 0)
                {
                    data.subtext += "<b>" + i18n("Local Timezone") + "</b>";
                }
                else
                {
                    data.subtext += "<b>" + m_zone.mid(m_zone.find('/') + 1).replace("_", " ") + "</b>";
                }
                data.subtext += " " + _time + ", " + _date + "<br>";
            }
        }

        TZoffset = zone->calc_TZ_offset(activeZone);
    }

    data.icon = DesktopIcon("date", KIcon::SizeMedium);
    data.direction = popupDirection();
    data.duration = 4000;
}

void ClockApplet::fixupLayout()
{
    m_layoutDelay = 0;

    // ensure we have the right widget line up in horizontal mode
    // when we are showing date beside the clock
    // this fixes problems triggered by having the date first
    // because of the date format (e.g. YY/MM/DD) and then hiding
    // the date
    if (orientation() == Qt::Horizontal && height() < 32)
    {
        bool mustShowDate = showDate || (zone->zoneIndex() != 0);

        if (!mustShowDate && !showDayOfWeek)
        {
            _clock->widget()->move(0,0);
        }

        int dayWidth = 0;
        if (!showDayOfWeek)
        {
            _dayOfWeek->move(_clock->widget()->width() + 4, 0);
        }
        else
        {
            dayWidth = _dayOfWeek->width();
        }

        if (!showDate)
        {
            _date->move(_clock->widget()->width() + dayWidth + 4, 0);
        }
    }

    emit updateLayout();
}

int ClockApplet::type()
{
    return _prefs->type();
}

ClockAppletToolTip::ClockAppletToolTip( ClockApplet* clock )
    : QToolTip( clock ),
      m_clock( clock )
{
}

void ClockAppletToolTip::maybeTip( const QPoint & /*point*/ )
{
    QString tipText;
    if ( (m_clock->type() == Prefs::EnumType::Fuzzy) ||
         (m_clock->type() == Prefs::EnumType::Analog) )
    {
        // show full time (incl. hour) as tooltip for Fuzzy clock
        tipText = KGlobal::locale()->formatDateTime(QDateTime::currentDateTime().addSecs(m_clock->TZoffset));
    }
    else
    {
        tipText = KGlobal::locale()->formatDate(m_clock->clockGetDate());
    }

    if (m_clock->timezones() && m_clock->timezones()->zoneIndex() > 0)
    {
        tipText += "\n" + i18n("Showing time for %1").arg(i18n(m_clock->timezones()->zone().utf8()), false);
    }

    tip(m_clock->geometry(), tipText);
}

//************************************************************

#include "clock.moc"
