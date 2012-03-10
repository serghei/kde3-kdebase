#ifndef KSWIDGET_H
#define KSWIDGET_H

#include <qxembed.h>
#include <X11/X.h>

class KSWidget : public QXEmbed
{
    Q_OBJECT
public:
    KSWidget( QWidget *parent = NULL, const char* name = NULL, int flags = 0 );
    virtual ~KSWidget();
private:
    Colormap colormap;
};

#endif
