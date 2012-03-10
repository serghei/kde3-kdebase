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

#include "messageview.h"
#include "messageview.moc"

#include <qtextbrowser.h>

Kate::MessageView::MessageView( QWidget *parent, const char *name )
  : Kate::DockViewBase( parent, name )
{
  m_view = new QTextBrowser( this );
  // m_view->setFormat( Qt::richText ); // should be!!
  connect( m_view, SIGNAL( linkClicked( const QString & ) ), 
           SIGNAL( linkClicked( const QString & ) ) );
}

Kate::MessageView::~MessageView()
{
}

void Kate::MessageView::addMessage( const QString &msg )
{
  m_view->append( msg );
  m_view->scrollToBottom();
}

void Kate::MessageView::clear()
{
  m_view->clear();
}
