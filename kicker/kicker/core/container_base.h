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

#ifndef __container_base_h__
#define __container_base_h__

#include <qwidget.h>
#include <qpoint.h>
#include <qvaluelist.h>

#include <kpanelextension.h>
#include <kpanelapplet.h>

class KConfigGroup;
class QPopupMenu;

class BaseContainer : public QWidget
{
    Q_OBJECT

public:
    typedef QValueList<BaseContainer*> List;
    typedef QValueListIterator<BaseContainer*> Iterator;
    typedef QValueListConstIterator<BaseContainer*> ConstIterator;

    BaseContainer( QPopupMenu* appletOpMenu, QWidget* parent = 0, const char * name = 0 );
    ~BaseContainer();

    virtual void reparent(QWidget * parent, WFlags f, const QPoint & p, bool showIt = false);

    virtual int widthForHeight(int height) const = 0;
    virtual int heightForWidth(int width)  const = 0;

    virtual bool isStretch() const { return false; }

    virtual void completeMoveOperation() {}
    virtual void about() {}
    virtual void help() {}
    virtual void preferences() {}
    virtual void reportBug() {}

    virtual bool isValid() const { return true; }
    bool isImmutable() const;
    virtual void setImmutable(bool immutable);

    double freeSpace() const { return _fspace; }
    void setFreeSpace(double f) { _fspace = f; }

    QString appletId() const { return _aid; }
    void setAppletId(const QString& s) { _aid = s; }

    virtual int actions() const { return _actions; }

    KPanelApplet::Direction popupDirection() const { return _dir; }
    KPanelExtension::Orientation orientation() const { return _orient; }
    KPanelExtension::Alignment alignment() const { return _alignment; }

    virtual void setBackground() {}

    QPopupMenu* opMenu();
    void clearOpMenu();

    void loadConfiguration( KConfigGroup& );
    void saveConfiguration( KConfigGroup&, bool layoutOnly = false ) const;

    void configure(KPanelExtension::Orientation, KPanelApplet::Direction);
    virtual void configure() {}

    QPoint moveOffset() const { return _moveOffset; }

    virtual QString appletType() const = 0;
    virtual QString icon() const { return "unknown"; }
    virtual QString visibleName() const = 0;

public slots:
    virtual void slotRemoved(KConfig* config);
    virtual void setPopupDirection(KPanelApplet::Direction d) { _dir = d; }
    virtual void setOrientation(KPanelExtension::Orientation o) { _orient = o; }

    void setAlignment(KPanelExtension::Alignment a);

signals:
    void removeme(BaseContainer*);
    void takeme(BaseContainer*);
    void moveme(BaseContainer*);
    void maintainFocus(bool);
    void requestSave();
    void focusReqested(bool);

protected:
    virtual void doLoadConfiguration( KConfigGroup& ) {}
    virtual void doSaveConfiguration( KConfigGroup&,
                                      bool /* layoutOnly */ ) const {}
    virtual void alignmentChange(KPanelExtension::Alignment) {}

    virtual QPopupMenu* createOpMenu() = 0;
    QPopupMenu *appletOpMenu() const { return _appletOpMnu; }

    KPanelApplet::Direction _dir;
    KPanelExtension::Orientation _orient;
    KPanelExtension::Alignment _alignment;
    double             _fspace;
    QPoint             _moveOffset;
    QString            _aid;
    int                _actions;
    bool               m_immutable;

private:
    QPopupMenu        *_opMnu;
    QPopupMenu        *_appletOpMnu;
};

#endif

