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

#include <qlayout.h>
#include <qtimer.h>
#include <qwmatrix.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <krootpixmap.h>
#include <kstandarddirs.h>

#include "global.h"
#include "kickerSettings.h"
#include "taskbarcontainer.h"

#include "taskbarextension.h"
#include "taskbarextension.moc"

extern "C"
{
    KDE_EXPORT KPanelExtension* init( QWidget *parent, const QString& configFile )
    {
        KGlobal::locale()->insertCatalogue( "taskbarextension" );
   	return new TaskBarExtension( configFile, KPanelExtension::Stretch,
				     KPanelExtension::Preferences, parent, "taskbarextension" );
    }
}

TaskBarExtension::TaskBarExtension(const QString& configFile, Type type,
                                   int actions, QWidget *parent, const char *name)
    : KPanelExtension(configFile, type, actions, parent, name),
      m_bgImage(0),
      m_bgFilename(0),
      m_rootPixmap(0)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    m_container = new TaskBarContainer(false, this);
    m_container->setBackgroundOrigin(AncestorOrigin);
    positionChange(position());
    layout->addWidget(m_container);

    connect(m_container, SIGNAL(containerCountChanged()),
            SIGNAL(updateLayout()));

    kapp->dcopClient()->setNotifications(true);
    connectDCOPSignal("kicker", "kicker", "configurationChanged()",
                      "configure()", false);

    connect(kapp, SIGNAL(kdisplayPaletteChanged()),
            SLOT(setBackgroundTheme()));

    QTimer::singleShot(0, this, SLOT(setBackgroundTheme()));
}

TaskBarExtension::~TaskBarExtension()
{
    KGlobal::locale()->removeCatalogue( "taskbarextension" );
}

void TaskBarExtension::positionChange( Position p )
{

    m_container->orientationChange(orientation());

    switch (p)
    {
    case Bottom:
        m_container->popupDirectionChange(KPanelApplet::Up);
        break;
    case Top:
        m_container->popupDirectionChange(KPanelApplet::Down);
        break;
    case Right:
        m_container->popupDirectionChange(KPanelApplet::Left);
        break;
    case Left:
        m_container->popupDirectionChange(KPanelApplet::Right);
        break;
    case Floating:
        if (orientation() == Horizontal)
        {
            m_container->popupDirectionChange(KPanelApplet::Down);
        }
        else if (QApplication::reverseLayout())
        {
            m_container->popupDirectionChange(KPanelApplet::Left);
        }
        else
        {
            m_container->popupDirectionChange(KPanelApplet::Right);
        }
        break;
    }
    setBackgroundTheme();
}

void TaskBarExtension::preferences()
{
    m_container->preferences();
}

QSize TaskBarExtension::sizeHint(Position p, QSize maxSize) const
{
    if (p == Left || p == Right)
        maxSize.setWidth(sizeInPixels());
    else
        maxSize.setHeight(sizeInPixels());

//    kdDebug(1210) << "TaskBarExtension::sizeHint( Position, QSize )" << endl;
//    kdDebug(1210) << " width: " << size.width() << endl;
//    kdDebug(1210) << "height: " << size.height() << endl;
    return m_container->sizeHint(p, maxSize);
}

void TaskBarExtension::configure()
{
    setBackgroundTheme();
    update();
}

void TaskBarExtension::setBackgroundTheme()
{
    if (KickerSettings::transparent())
    {
        if (!m_rootPixmap)
        {
            m_rootPixmap = new KRootPixmap(this);
            m_rootPixmap->setCustomPainting(true);
            connect(m_rootPixmap, SIGNAL(backgroundUpdated(const QPixmap&)),
                    SLOT(updateBackground(const QPixmap&)));
        }
        else
        {
            m_rootPixmap->repaint(true);
        }

        double tint = double(KickerSettings::tintValue()) / 100;
        m_rootPixmap->setFadeEffect(tint, KickerSettings::tintColor());
        m_rootPixmap->start();
        return;
    }
    else if (m_rootPixmap)
    {
        delete m_rootPixmap;
        m_rootPixmap = 0;
    }

    unsetPalette();
    m_container->unsetPalette();

    if (KickerSettings::useBackgroundTheme())
    {
        QString bgFilename = locate("appdata", KickerSettings::backgroundTheme());

        if (m_bgFilename != bgFilename)
        {
            m_bgFilename = bgFilename;
            m_bgImage.load(m_bgFilename);
        }

        if (!m_bgImage.isNull())
        {
            QImage bgImage = m_bgImage;

            if (orientation() == Vertical)
            {
                if (KickerSettings::rotateBackground())
                {
                    QWMatrix matrix;
                    matrix.rotate(position() == KPanelExtension::Left ? 90: 270);
                    bgImage = bgImage.xForm(matrix);
                }

                bgImage = bgImage.scaleWidth(size().width());
            }
            else
            {
                if (position() == KPanelExtension::Top &&
                    KickerSettings::rotateBackground())
                {
                    QWMatrix matrix;
                    matrix.rotate(180);
                    bgImage = bgImage.xForm(matrix);
                }

                bgImage = bgImage.scaleHeight(size().height());
            }

            if (KickerSettings::colorizeBackground())
            {
                KickerLib::colorize(bgImage);
            }
            setPaletteBackgroundPixmap(bgImage);
            m_container->setPaletteBackgroundPixmap(bgImage);
        }
    }
}

void TaskBarExtension::updateBackground(const QPixmap& bgImage)
{
    unsetPalette();
    setPaletteBackgroundPixmap(bgImage);
    m_container->unsetPalette();
    m_container->setPaletteBackgroundPixmap(bgImage);
}

void TaskBarExtension::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    setBackgroundTheme();
}

