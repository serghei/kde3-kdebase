/* This file is part of the KDE project
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KATE_MAILFILES_DIALOG_H_
#define _KATE_MAILFILES_DIALOG_H_

#include <kate/document.h>

#include <kdialogbase.h>
#include <kurl.h>
#include <qptrlist.h>

class QString;
class QStringList;
class KateMainWindow;

/**
    This is a dialog for choosing which of the open files to mail.
    The current file is selected by default, the dialog can be expanded
    to display all the files if required.
    
*/
class KateMailDialog : public KDialogBase {
  Q_OBJECT
  public:
    KateMailDialog( QWidget *parent=0,
                          KateMainWindow *mainwin=0 );
    ~KateMailDialog() {};

    /**
        @return a list of the selected docs.
    */
    QPtrList<Kate::Document> selectedDocs();
  private slots:
    void slotShowButton();
  private:
    class KListView *list;
    class QLabel *lInfo;
    KateMainWindow *mainWindow;
    class QVBox *mw;

};

#endif // _KATE_MAILFILES_DIALOG_H_
