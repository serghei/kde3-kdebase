/*
    This file is part of KSysGuard.
    Copyright ( C ) 2002 Nadeem Hasan ( nhasan@kde.org )

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or ( at your option ) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kaccelmanager.h>
#include <klocale.h>

#include "KSGAppletSettings.h"

KSGAppletSettings::KSGAppletSettings( QWidget *parent, const char *name )
    : KDialogBase( parent, name, false, QString::null, Ok|Apply|Cancel, 
      Ok, true )
{
  setCaption( i18n( "System Guard Settings" ) );

  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *topLayout = new QGridLayout( page, 3, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() ); 

  QLabel *label = new QLabel( i18n( "Number of displays:" ), page );
  topLayout->addWidget( label, 0, 0 );

  mNumDisplay = new QSpinBox( 1, 32, 1, page );
  mNumDisplay->setValue(2);
  topLayout->addWidget( mNumDisplay, 0, 1 );
  label->setBuddy( mNumDisplay );

  label = new QLabel( i18n( "Size ratio:" ), page );
  topLayout->addWidget( label, 1, 0 );

  mSizeRatio = new QSpinBox( 50, 500, 50, page );
  mSizeRatio->setSuffix( i18n( "%" ) );
  mSizeRatio->setValue(100);
  topLayout->addWidget( mSizeRatio, 1, 1 );
  label->setBuddy( mSizeRatio );

  label = new QLabel( i18n( "Update interval:" ), page );
  topLayout->addWidget( label, 2, 0 );

  mInterval = new QSpinBox( 1, 300, 1, page );
  mInterval->setValue(2);
  mInterval->setSuffix( i18n( " sec" ) );
  topLayout->addWidget( mInterval, 2, 1 );
  label->setBuddy( mInterval );

  resize( QSize( 250, 130 ).expandedTo( minimumSizeHint() ) );

  KAcceleratorManager::manage( page );
}

KSGAppletSettings::~KSGAppletSettings()
{
}

int KSGAppletSettings::numDisplay() const
{
  return mNumDisplay->value();
}

void KSGAppletSettings::setNumDisplay( int value )
{
  mNumDisplay->setValue( value );
}

int KSGAppletSettings::sizeRatio() const
{
  return mSizeRatio->value();
}

void KSGAppletSettings::setSizeRatio( int value )
{
  mSizeRatio->setValue( value );
}

int KSGAppletSettings::updateInterval() const
{
  return mInterval->value();
}

void KSGAppletSettings::setUpdateInterval( int value )
{
  mInterval->setValue( value );
}

