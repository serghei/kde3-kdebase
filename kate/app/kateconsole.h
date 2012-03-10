/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
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

#ifndef __KATE_CONSOLE_H__
#define __KATE_CONSOLE_H__

#include "katemain.h"

#include <kurl.h>

#include <qvbox.h>

namespace KParts {
  class ReadOnlyPart;
}

namespace KateMDI {
  class ToolView;
}

class KateMainWindow;

/**
 * KateConsole
 * This class is used for the internal terminal emulator
 * It uses internally the konsole part, thx to konsole devs :)
 */
class KateConsole : public QVBox
{
  Q_OBJECT

  public:
    /**
     * construct us
     * @param mw main window
     * @param parent toolview
     */
    KateConsole (KateMainWindow *mw, KateMDI::ToolView* parent);

    /**
     * destruct us
     */
    ~KateConsole ();

    /**
     * cd to dir
     * @param url given dir
     */
    void cd (const KURL &url);

    /**
     * send given text to console
     * @param text commands for console
     */
    void sendInput( const QString& text );

  public slots:
    /**
     * pipe current document to console
     */
    void slotPipeToConsole ();

  private slots:
    /**
     * the konsole exited ;)
     * handle that, hide the dock
     */
    void slotDestroyed ();

    /**
     * construct console if needed
     */
    void loadConsoleIfNeeded();

  protected:
    /**
     * the konsole get shown
     * @param ev show event
     */
    void showEvent(QShowEvent *ev);

  private:
    /**
     * console part
     */
    KParts::ReadOnlyPart *m_part;

    /**
     * main window of this console
     */
    KateMainWindow *m_mw;

    /**
     * toolview for this console
     */
    KateMDI::ToolView *m_toolView;
};

#endif
