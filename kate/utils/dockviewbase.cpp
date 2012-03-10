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

#include "dockviewbase.h"
#include "dockviewbase.moc"

#include <qlabel.h>
#include <qlayout.h>

//#include <kdebug.h>

namespace Kate {

// data storage
class DockViewBasePrivate {
  public:
  QWidget *header;
  QLabel *lTitle;
  QLabel *lPrefix;
};

}

Kate::DockViewBase::DockViewBase( QWidget* parent, const char* name )
  : QVBox( parent, name ),
    d ( new Kate::DockViewBasePrivate )
{
  init( QString::null, QString::null );
}

Kate::DockViewBase::DockViewBase( const QString &prefix, const QString &title, QWidget* parent, const char* name )
  : QVBox( parent, name ),
    d ( new Kate::DockViewBasePrivate )
{
  init( prefix, title );
}

Kate::DockViewBase::~DockViewBase()
{
  delete d;
}

void Kate::DockViewBase::setTitlePrefix( const QString &prefix )
{
    d->lPrefix->setText( prefix );
    d->lPrefix->show();
}

QString Kate::DockViewBase::titlePrefix() const
{
  return d->lPrefix->text();
}

void Kate::DockViewBase::setTitle( const QString &title )
{
  d->lTitle->setText( title );
  d->lTitle->show();
}

QString Kate::DockViewBase::title() const
{
  return d->lTitle->text();
}

void Kate::DockViewBase::setTitle( const QString &prefix, const QString &title )
{
  setTitlePrefix( prefix );
  setTitle( title );
}

void Kate::DockViewBase::init( const QString &prefix, const QString &title )
{
  setSpacing( 4 );
  d->header = new QWidget( this );
  d->header->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed, true ) );
  QHBoxLayout *lo = new QHBoxLayout( d->header );
  lo->setSpacing( 6 );
  lo->insertSpacing( 0, 6 ); 
  d->lPrefix = new QLabel( title, d->header );
  lo->addWidget( d->lPrefix );
  d->lTitle = new QLabel( title, d->header );
  lo->addWidget( d->lTitle );
  lo->setStretchFactor( d->lTitle, 1 );
  lo->insertSpacing( -1, 6 );
  if ( prefix.isEmpty() ) d->lPrefix->hide();
  if ( title.isEmpty() ) d->lTitle->hide();
}
