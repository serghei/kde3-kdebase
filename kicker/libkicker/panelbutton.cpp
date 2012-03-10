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

#include <qcursor.h>
#include <qfile.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qstylesheet.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <kipc.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "global.h"

#include "kshadowengine.h"
#include "kshadowsettings.h"

#include "kickerSettings.h"
#include "panelbutton.h"
#include "panelbutton.moc"

// init static variable
KShadowEngine* PanelButton::s_textShadowEngine = 0L;

PanelButton::PanelButton( QWidget* parent, const char* name )
    : QButton(parent, name),
      m_valid(true),
      m_isLeftMouseButtonDown(false),
      m_drawArrow(false),
      m_highlight(false),
      m_changeCursorOverItem(true),
      m_hasAcceptedDrag(false),
      m_arrowDirection(KPanelExtension::Bottom),
      m_popupDirection(KPanelApplet::Up),
      m_orientation(Horizontal),
      m_size((KIcon::StdSizes)-1),
      m_fontPercent(0.40)
{
    setBackgroundOrigin(AncestorOrigin);
    setWFlags(WNoAutoErase);
    KGlobal::locale()->insertCatalogue("libkicker");
    calculateIconSize();
    setAcceptDrops(true);

    m_textColor = KGlobalSettings::textColor();

    updateSettings(KApplication::SETTINGS_MOUSE);

    kapp->addKipcEventMask(KIPC::SettingsChanged | KIPC::IconChanged);

    installEventFilter(KickerTip::the());

    connect(kapp, SIGNAL(settingsChanged(int)), SLOT(updateSettings(int)));
    connect(kapp, SIGNAL(iconChanged(int)), SLOT(updateIcon(int)));
}

void PanelButton::configure()
{
    QString name = tileName();
    if( name.isEmpty() )
        return;

    if (!KickerSettings::enableTileBackground())
    {
        setTile(QString::null);
        return;
    }

    KConfigGroup tilesGroup( KGlobal::config(), "button_tiles" );
    if( !tilesGroup.readBoolEntry( "Enable" + name + "Tiles", true ) ) {
            setTile( QString::null );
        return;
    }

    QString tile = tilesGroup.readEntry( name + "Tile" );
    QColor color = QColor();

    if (tile == "Colorize")
    {
        color = tilesGroup.readColorEntry( name + "TileColor" );
        tile = QString::null;
    }

    setTile( tile, color );
}

void PanelButton::setTile(const QString& tile, const QColor& color)
{
    if (tile == m_tile && m_tileColor == color)
    {
        return;
    }

    m_tile = tile;
    m_tileColor = color;
    loadTiles();
    update();
}

void PanelButton::setDrawArrow(bool drawArrow)
{
    if (m_drawArrow == drawArrow)
    {
        return;
    }

    m_drawArrow = drawArrow;
    update();
}

QImage PanelButton::loadTile(const QString& tile,
                             const QSize& size,
                             const QString& state)
{
    QString name = tile;

    if (size.height() < 42)
    {
        name += "_tiny_";
    }
    else if (size.height() < 54)
    {
        name += "_normal_";
    }
    else
    {
        name += "_large_";
    }

    name += state + ".png";

    QImage tileImg(KGlobal::dirs()->findResource("tiles", name));

    // scale if size does not match exactly
    if (!tileImg.isNull() && tileImg.size() != size)
    {
        tileImg = tileImg.smoothScale(size);
    }

    return tileImg;
}

void PanelButton::setEnabled(bool enable)
{
    QButton::setEnabled(enable);
    loadIcons();
    update();
}

void PanelButton::setPopupDirection(KPanelApplet::Direction d)
{
    m_popupDirection = d;
    setArrowDirection(KickerLib::directionToPopupPosition(d));
}

void PanelButton::setOrientation(Orientation o)
{
    m_orientation = o;
}

void PanelButton::updateIcon(int group)
{
    if (group != KIcon::Panel)
    {
        return;
    }

    loadIcons();
    update();
}

void PanelButton::updateSettings(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
    {
        return;
    }

    m_changeCursorOverItem = KGlobalSettings::changeCursorOverIcon();

    if (m_changeCursorOverItem)
    {
        setCursor(KCursor::handCursor());
    }
    else
    {
        unsetCursor();
    }
}

void PanelButton::checkForDeletion(const QString& path)
{
    if (path == m_backingFile)
    {
        setEnabled(false);
        QTimer::singleShot(1000, this, SLOT(scheduleForRemoval()));
    }
}

bool PanelButton::checkForBackingFile()
{
    return QFile::exists(m_backingFile);
}

void PanelButton::scheduleForRemoval()
{
    static int timelapse = 1000;
    if (checkForBackingFile())
    {
        setEnabled(true);
        timelapse = 1000;
        emit hideme(false);
        return;
    }
    else if (KickerSettings::removeButtonsWhenBroken())
    {
        if (timelapse > 255*1000) // we'v given it ~8.5 minutes by this point
        {
            emit removeme();
            return;
        }

        if (timelapse > 3000 && isVisible())
        {
            emit hideme(true);
        }

        timelapse *= 2;
        QTimer::singleShot(timelapse, this, SLOT(scheduleForRemoval()));
    }
}

// return the dimension that the button wants to be for a given panel dimension (panelDim)
int PanelButton::preferredDimension(int panelDim) const
{
    // determine the upper limit on the size.  Normally, this is panelDim,
    // but if conserveSpace() is true, we restrict size to comfortably fit the icon
    if (KickerSettings::conserveSpace())
    {
        int newSize = preferredIconSize(panelDim);
        if (newSize > 0)
        {
            return QMIN(panelDim, newSize + (KDialog::spacingHint() * 2));
        }
    }

    return panelDim;
}

int PanelButton::widthForHeight(int height) const
{
    int rc = preferredDimension(height);

    // we only paint the text when horizontal, so make sure we're horizontal
    // before adding the text in here
    if (orientation() == Horizontal && !m_buttonText.isEmpty())
    {
        QFont f(font());
        f.setPixelSize(KMIN(height, KMAX(int(float(height) * m_fontPercent), 16)));
        QFontMetrics fm(f);

        rc += fm.width(m_buttonText) + KMIN(25, KMAX(5, fm.width('m') / 2));
    }

    return rc;
}

int PanelButton::heightForWidth(int width) const
{
    return preferredDimension(width);
}

const QPixmap& PanelButton::labelIcon() const
{
    return m_highlight ? m_iconh : m_icon;
}

const QPixmap& PanelButton::zoomIcon() const
{
    return m_iconz;
}

bool PanelButton::isValid() const
{
    return m_valid;
}

void PanelButton::setTitle(const QString& t)
{
    m_title = t;
}

void PanelButton::setIcon(const QString& icon)
{
    if (icon == m_iconName)
    {
        return;
    }

    m_iconName = icon;
    loadIcons();
    update();
    emit iconChanged();
}

QString PanelButton::icon() const
{
    return m_iconName;
}

bool PanelButton::hasText() const
{
    return !m_buttonText.isEmpty();
}

void PanelButton::setButtonText(const QString& text)
{
    m_buttonText = text;
    update();
}

QString PanelButton::buttonText() const
{
    return m_buttonText;
}

void PanelButton::setTextColor(const QColor& c)
{
    m_textColor = c;
}

QColor PanelButton::textColor() const
{
    return m_textColor;
}

void PanelButton::setFontPercent(double p)
{
    m_fontPercent = p;
}

double PanelButton::fontPercent() const
{
    return m_fontPercent;
}

KPanelExtension::Orientation PanelButton::orientation() const
{
    return m_orientation;
}

KPanelApplet::Direction PanelButton::popupDirection() const
{
    return m_popupDirection;
}

QPoint PanelButton::center() const
{
    return mapToGlobal(rect().center());
}

QString PanelButton::title() const
{
    return m_title;
}

void PanelButton::triggerDrag()
{
    setDown(false);

    startDrag();
}

void PanelButton::startDrag()
{
    emit dragme(m_icon);
}

void PanelButton::enterEvent(QEvent* e)
{
    if (!m_highlight)
    {
        m_highlight = true;
        repaint(false);
    }

    QButton::enterEvent(e);
}

void PanelButton::leaveEvent(QEvent* e)
{
    if (m_highlight)
    {
        m_highlight = false;
        repaint(false);
    }

    QButton::leaveEvent(e);
}

void PanelButton::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->isAccepted())
    {
        m_hasAcceptedDrag = true;
    }

    update();
    QButton::dragEnterEvent( e );
}

void PanelButton::dragLeaveEvent(QDragLeaveEvent* e)
{
    m_hasAcceptedDrag = false;
    update();
    QButton::dragLeaveEvent( e );
}

void PanelButton::dropEvent(QDropEvent* e)
{
    m_hasAcceptedDrag = false;
    update();
    QButton::dropEvent( e );
}

void PanelButton::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_isLeftMouseButtonDown || (e->state() & LeftButton) == 0)
    {
        return;
    }

    QPoint p(e->pos() - m_lastLeftMouseButtonPress);
    if (p.manhattanLength() <= 16)
    {
        // KGlobalSettings::dndEventDelay() is not enough!
        return;
    }

    m_isLeftMouseButtonDown = false;
    triggerDrag();
}

void PanelButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == LeftButton)
    {
        m_lastLeftMouseButtonPress = e->pos();
        m_isLeftMouseButtonDown = true;
    }
    QButton::mousePressEvent(e);
}

void PanelButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == LeftButton)
    {
        m_isLeftMouseButtonDown = false;
    }
    QButton::mouseReleaseEvent(e);
}

void PanelButton::resizeEvent(QResizeEvent*)
{
    loadTiles();

    if (calculateIconSize())
    {
        loadIcons();
    }
}

void PanelButton::drawButton(QPainter *p)
{
    const QPixmap& tile = (isDown() || isOn()) ? m_down : m_up;
    
    if (m_tileColor.isValid())
    {
        p->fillRect(rect(), m_tileColor);
        style().drawPrimitive(QStyle::PE_Panel, p, rect(), colorGroup());
    }
    else if (paletteBackgroundPixmap())
    {
        // Draw the background. This is always needed, even when using tiles,
        // because they don't have to cover the entire button.
        QPoint offset = backgroundOffset();
        int ox = offset.x();
        int oy = offset.y();
        p->drawTiledPixmap( 0, 0, width(), height(),*paletteBackgroundPixmap(), ox, oy);
    }

    if (!tile.isNull())
    {
        // Draw the tile.
        p->drawPixmap(0, 0, tile);
    }
    else if (isDown() || isOn())
    {
        // Draw shapes to indicate the down state.
        style().drawPrimitive(QStyle::PE_Panel, p, rect(), colorGroup(), QStyle::Style_Sunken);
    }

    drawButtonLabel(p);

    if (hasFocus() || m_hasAcceptedDrag)
    {
        int x1, y1, x2, y2;
        rect().coords(&x1, &y1, &x2, &y2);
        QRect r(x1+2, y1+2, x2-x1-3, y2-y1-3);
        style().drawPrimitive(QStyle::PE_FocusRect, p, r, colorGroup(),
        QStyle::Style_Default, colorGroup().button());
    }
}

void PanelButton::drawButtonLabel(QPainter *p)
{
    QPixmap icon = labelIcon();
    bool active = isDown() || isOn();

    if (active)
    {
        icon = icon.convertToImage().smoothScale(icon.width() - 2,
                                                 icon.height() - 2);
    }

    if (!m_buttonText.isEmpty() && orientation() == Horizontal)
    {
        int h = height();
        int w = width();
        int y = (h - icon.height())/2;
        p->save();
        QFont f = font();

        double fontPercent = m_fontPercent;
        if (active)
        {
            fontPercent *= .8;
        }
        f.setPixelSize(KMIN(h, KMAX(int(float(h) * m_fontPercent), 16)));
        QFontMetrics fm(f);
        p->setFont(f);

        /* Draw shadowed text */
        bool reverse = QApplication::reverseLayout();
        QPainter::TextDirection rtl = reverse ? QPainter::RTL : QPainter::LTR;

        if (!reverse && !icon.isNull())
        {
            /* Draw icon */
            p->drawPixmap(3, y, icon);
        }

        int tX = reverse ? 3 : icon.width() + KMIN(25, KMAX(5, fm.width('m') / 2));
        int tY = fm.ascent() + ((h - fm.height()) / 2);

        QColor shadCol = KickerLib::shadowColor(m_textColor);

        // get a transparent pixmap
        QPainter pixPainter;
        QPixmap textPixmap(w, h);

        textPixmap.fill(QColor(0,0,0));
        textPixmap.setMask(textPixmap.createHeuristicMask(true));

        // draw text
        pixPainter.begin(&textPixmap);
        pixPainter.setPen(m_textColor);
        pixPainter.setFont(p->font()); // get the font from the root painter
        pixPainter.drawText(tX, tY, m_buttonText, -1, rtl);
        pixPainter.end();

        if (!s_textShadowEngine)
        {
            KShadowSettings* shadset = new KShadowSettings();
            shadset->setOffsetX(0);
            shadset->setOffsetY(0);
            shadset->setThickness(1);
            shadset->setMaxOpacity(96);
            s_textShadowEngine = new KShadowEngine(shadset);
        }

        // draw shadow
        QImage img = s_textShadowEngine->makeShadow(textPixmap, shadCol);
        p->drawImage(0, 0, img);
        p->save();
        p->setPen(m_textColor);
        p->drawText(tX, tY, m_buttonText, -1, rtl);
        p->restore();

        if (reverse && !icon.isNull())
        {
            p->drawPixmap(w - icon.width() - 3, y, icon);
        }

        p->restore();
    }
    else if (!icon.isNull())
    {
        int y = (height() - icon.height()) / 2;
        int x = (width()  - icon.width()) / 2;
        p->drawPixmap(x, y, icon);
    }

    if (m_drawArrow && (m_highlight || active))
    {
        QStyle::PrimitiveElement e = QStyle::PE_ArrowUp;
        int arrowSize = style().pixelMetric(QStyle::PM_MenuButtonIndicator);
        QRect r((width() - arrowSize)/2, 0, arrowSize, arrowSize);

        switch (m_arrowDirection)
        {
            case KPanelExtension::Top:
                e = QStyle::PE_ArrowUp;
                break;
            case KPanelExtension::Bottom:
                e = QStyle::PE_ArrowDown;
                r.moveBy(0, height() - arrowSize);
                break;
            case KPanelExtension::Right:
                e = QStyle::PE_ArrowRight;
                r = QRect(width() - arrowSize, (height() - arrowSize)/2, arrowSize, arrowSize);
                break;
            case KPanelExtension::Left:
                e = QStyle::PE_ArrowLeft;
                r = QRect(0, (height() - arrowSize)/2, arrowSize, arrowSize);
                break;
            case KPanelExtension::Floating:
                if (orientation() == Horizontal)
                {
                    e = QStyle::PE_ArrowDown;
                    r.moveBy(0, height() - arrowSize);
                }
                else if (QApplication::reverseLayout())
                {
                    e = QStyle::PE_ArrowLeft;
                    r = QRect(0, (height() - arrowSize)/2, arrowSize, arrowSize);
                }
                else
                {
                    e = QStyle::PE_ArrowRight;
                    r = QRect(width() - arrowSize, (height() - arrowSize)/2, arrowSize, arrowSize);
                }
                break;
        }

        int flags = QStyle::Style_Enabled;
        if (active)
        {
            flags |= QStyle::Style_Down;
        }
        style().drawPrimitive(e, p, r, colorGroup(), flags);
    }
}

// return the icon size that would be used if the panel were proposed_size
// if proposed_size==-1, use the current panel size instead
int PanelButton::preferredIconSize(int proposed_size) const
{
    // (re)calculates the icon sizes and report true if they have changed.
    // Get sizes from icontheme. We assume they are sorted.
    KIconTheme *ith = KGlobal::iconLoader()->theme();

    if (!ith)
    {
        return -1; // unknown icon size
    }

    QValueList<int> sizes = ith->querySizes(KIcon::Panel);

    int sz = ith->defaultSize(KIcon::Panel);

    if (proposed_size < 0)
    {
        proposed_size = (orientation() == Horizontal) ? height() : width();
    }

    // determine the upper limit on the size.  Normally, this is panelSize,
    // but if conserve space is requested, the max button size is used instead.
    int upperLimit = proposed_size;
    if (proposed_size > KickerLib::maxButtonDim() &&
        KickerSettings::conserveSpace())
    {
        upperLimit = KickerLib::maxButtonDim();
    }

    //kdDebug()<<endl<<endl<<flush;
    QValueListConstIterator<int> i = sizes.constBegin();
    while (i != sizes.constEnd())
    {
        if ((*i) + (2 * KickerSettings::iconMargin()) > upperLimit)
        {
            break;
        }
        sz = *i;   // get the largest size under the limit
        ++i;
    }

    //kdDebug()<<"Using icon sizes: "<<sz<<"  "<<zoom_sz<<endl<<flush;
    return sz;
}

void PanelButton::backedByFile(const QString& localFilePath)
{
    m_backingFile = localFilePath;

    if (m_backingFile.isEmpty())
    {
        return;
    }

    // avoid multiple connections
    disconnect(KDirWatch::self(), SIGNAL(deleted(const QString&)),
               this, SLOT(checkForDeletion(const QString&)));

    if (!KDirWatch::self()->contains(m_backingFile))
    {
        KDirWatch::self()->addFile(m_backingFile);
    }

    connect(KDirWatch::self(), SIGNAL(deleted(const QString&)),
            this, SLOT(checkForDeletion(const QString&)));

}

void PanelButton::setArrowDirection(KPanelExtension::Position dir)
{
    if (m_arrowDirection != dir)
    {
        m_arrowDirection = dir;
        update();
    }
}

void PanelButton::loadTiles()
{
    if (m_tileColor.isValid())
    {
        setBackgroundOrigin(WidgetOrigin);
        m_up = m_down = QPixmap();
    }
    else if (m_tile.isNull())
    {
        setBackgroundOrigin(AncestorOrigin);
        m_up = m_down = QPixmap();
    }
    else
    {
        setBackgroundOrigin(WidgetOrigin);
        // If only the tiles were named a bit smarter we wouldn't have
        // to pass the up or down argument.
        m_up   = QPixmap(loadTile(m_tile, size(), "up"));
        m_down = QPixmap(loadTile(m_tile, size(), "down"));
    }
}

void PanelButton::loadIcons()
{
    KIconLoader * ldr = KGlobal::iconLoader();
    QString nm = m_iconName;
    KIcon::States defaultState = isEnabled() ? KIcon::DefaultState :
                                               KIcon::DisabledState;
    m_icon = ldr->loadIcon(nm, KIcon::Panel, m_size, defaultState, 0L, true);

    if (m_icon.isNull())
    {
        nm = defaultIcon();
        m_icon = ldr->loadIcon(nm, KIcon::Panel, m_size, defaultState);
    }

    if (!isEnabled())
    {
        m_iconh = m_icon;
    }
    else
    {
        m_iconh = ldr->loadIcon(nm, KIcon::Panel, m_size,
                                KIcon::ActiveState, 0L, true);
    }

    m_iconz = ldr->loadIcon(nm, KIcon::Panel, KIcon::SizeHuge,
                            defaultState, 0L, true );
}

// (re)calculates the icon sizes and report true if they have changed.
//      (false if we don't know, because theme couldn't be loaded?)
bool PanelButton::calculateIconSize()
{
    int size = preferredIconSize();

    if (size < 0)
    {
        // size unknown
        return false;
    }

    if (m_size != size)
    {
        // Size has changed, update
        m_size = size;
        return true;
    }

    return false;
}

void PanelButton::updateKickerTip(KickerTip::Data& data)
{
    data.message = QStyleSheet::escape(title());
    data.subtext = QStyleSheet::escape(QToolTip::textFor(this));
    data.icon = zoomIcon();
    data.direction = popupDirection();
}

//
// PanelPopupButton class
//

PanelPopupButton::PanelPopupButton(QWidget *parent, const char *name)
  : PanelButton(parent, name),
    m_popup(0),
    m_pressedDuringPopup(false),
    m_initialized(false)
{
    connect(this, SIGNAL(pressed()), SLOT(slotExecMenu()));
}

void PanelPopupButton::setPopup(QPopupMenu *popup)
{
    if (m_popup)
    {
        m_popup->removeEventFilter(this);
        disconnect(m_popup, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
    }

    m_popup = popup;
    setDrawArrow(m_popup != 0);

    if (m_popup)
    {
        m_popup->installEventFilter(this);
        connect(m_popup, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
    }
}

QPopupMenu *PanelPopupButton::popup() const
{
    return m_popup;
}

bool PanelPopupButton::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::MouseMove)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (rect().contains(mapFromGlobal(me->globalPos())) &&
            ((me->state() & ControlButton) != 0 ||
             (me->state() & ShiftButton) != 0))
        {
            PanelButton::mouseMoveEvent(me);
            return true;
        }
    }
    else if (e->type() == QEvent::MouseButtonPress ||
             e->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (rect().contains(mapFromGlobal(me->globalPos())))
        {
            m_pressedDuringPopup = true;
            return true;
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (rect().contains(mapFromGlobal(me->globalPos())))
        {
            if (m_pressedDuringPopup && m_popup)
            {
                m_popup->hide();
            }
            return true;
        }
    }
    return false;
}

void PanelPopupButton::showMenu()
{
    if (isDown())
    {
        if (m_popup)
        {
            m_popup->hide();
        }

        setDown(false);
        return;
    }

    setDown(true);
    update();
    slotExecMenu();
}

void PanelPopupButton::slotExecMenu()
{
    if (!m_popup)
    {
        return;
    }

    m_pressedDuringPopup = false;
    KickerTip::enableTipping(false);
    kapp->syncX();
    kapp->processEvents();

    if (!m_initialized)
    {
        initPopup();
    }

    m_popup->adjustSize();
    m_popup->exec(KickerLib::popupPosition(popupDirection(), m_popup, this));
}

void PanelPopupButton::menuAboutToHide()
{
    if (!m_popup)
    {
        return;
    }

    setDown(false);
    KickerTip::enableTipping(true);
}

void PanelPopupButton::triggerDrag()
{
    if (m_popup)
    {
        m_popup->hide();
    }

    PanelButton::triggerDrag();
}

void PanelPopupButton::setInitialized(bool initialized)
{
    m_initialized = initialized;
}

