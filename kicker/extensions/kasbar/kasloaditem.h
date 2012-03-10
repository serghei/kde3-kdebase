// -*- c++ -*-


#ifndef KASLOADITEM_H
#define KASLOADITEM_H

#include "kasitem.h"

#include <kdemacros.h>

/**
 * An item that displays the system load.
 */
class KDE_EXPORT KasLoadItem : public KasItem
{
    Q_OBJECT

public:
    KasLoadItem( KasBar *parent );
    virtual ~KasLoadItem();

    void paint( QPainter *p );

public slots:
    void updateDisplay();
    void showMenuAt( QMouseEvent *ev );
    void showMenuAt( QPoint p );

private:
    QValueList<double> valuesOne;
    QValueList<double> valuesFive;
    QValueList<double> valuesFifteen;
};

#endif // KASLOADITEM_H

