/*
 * Copyright (C) 2000 by Matthias Kalle Dalheimer <kalle@kde.org>
 *
 * Licensed under the Artistic License.
 */

#ifndef __KDCOPWINDOW_H__
#define __KDCOPWINDOW_H__

class DCOPClient;
class QListViewItem;
class KAction;
class KSelectAction;
class QWidgetStack;
class QLabel;
class KDCOPListView;

#include <kmainwindow.h>
#include "kdcoplistview.h"
#include "kdcopview.h"

class KDCOPWindow : public KMainWindow
{
  Q_OBJECT

  public:

    KDCOPWindow( QWidget* parent = 0, const char* name = 0 );

  protected slots:

    void slotCurrentChanged( QListViewItem* item );
    void slotCallFunction();
    void slotCallFunction( QListViewItem* item );
    void slotApplicationRegistered(const QCString &);
    void slotApplicationUnregistered(const QCString &);
    void slotFillApplications();
    void slotCopy();
    void slotMode();
    void slotReload();
  private:
    void fillObjects( DCOPBrowserItem*, const char* app );
    void fillFunctions( DCOPBrowserItem*, const char* app, const char* obj );

    bool getParameters
      (
       const QString  & unNormalisedSignature,
       QString        & normalisedSignature,
       QStringList    & types,
       QStringList    & names
      );

    bool demarshal(QCString & replyType, QDataStream & reply, QListBox *theList);

    DCOPClient    * dcopClient;
    KAction       * exeaction;
    KSelectAction * langmode;
    kdcopview	  * mainView;
    QVBoxLayout  * mainLayout;
};



#endif
