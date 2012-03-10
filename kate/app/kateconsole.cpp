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

#include "kateconsole.h"
#include "kateconsole.moc"

#include "katemain.h"
#include "katemdi.h"
#include "katemainwindow.h"
#include "kateviewmanager.h"

#include <kate/view.h>
#include <kate/document.h>

#include <kde_terminal_interface.h>

#include <kparts/part.h>

#include <kurl.h>
#include <klibloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

KateConsole::KateConsole (KateMainWindow *mw, KateMDI::ToolView* parent)
 : QVBox (parent)
 , m_part (0)
 , m_mw (mw)
 , m_toolView (parent)
{
}

KateConsole::~KateConsole ()
{
  if (m_part)
    disconnect ( m_part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
}

void KateConsole::loadConsoleIfNeeded()
{
  if (m_part) return;

  if (!topLevelWidget() || !parentWidget()) return;
  if (!topLevelWidget() || !isVisibleTo(topLevelWidget())) return;

  KLibFactory *factory = KLibLoader::self()->factory("libkonsolepart");

  if (!factory) return;

  m_part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,"libkonsolepart", "KParts::ReadOnlyPart"));

  if (!m_part) return;

  setFocusProxy(m_part->widget());

  KGlobal::locale()->insertCatalogue("konsole");

  m_part->widget()->show();

  connect ( m_part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );

  if (m_mw->viewManager()->activeView())
    if (m_mw->viewManager()->activeView()->getDoc()->url().isValid())
      cd(KURL( m_mw->viewManager()->activeView()->getDoc()->url().path() ));
}

void KateConsole::slotDestroyed ()
{
  m_part = 0;

  // hide the dockwidget
  if (parentWidget())
  {
    m_mw->hideToolView (m_toolView);
    m_mw->centralWidget()->setFocus ();
  }
}

void KateConsole::showEvent(QShowEvent *)
{
  if (m_part) return;

  loadConsoleIfNeeded();
}

void KateConsole::cd (const KURL &url)
{
  loadConsoleIfNeeded();

  if (!m_part) return;

  m_part->openURL (url);
}

void KateConsole::sendInput( const QString& text )
{
  loadConsoleIfNeeded();

  if (!m_part) return;

  TerminalInterface *t = static_cast<TerminalInterface*>( m_part->qt_cast( "TerminalInterface" ) );

  if (!t) return;

  t->sendInput (text);
}

void KateConsole::slotPipeToConsole ()
{
  if (KMessageBox::warningContinueCancel
      (m_mw
       , i18n ("Do you really want to pipe the text to the console? This will execute any contained commands with your user rights.")
       , i18n ("Pipe to Console?")
       , i18n("Pipe to Console"), "Pipe To Console Warning") != KMessageBox::Continue)
    return;

  Kate::View *v = m_mw->viewManager()->activeView();

  if (!v)
    return;

  if (v->getDoc()->hasSelection ())
    sendInput (v->getDoc()->selection());
  else
    sendInput (v->getDoc()->text());
}
