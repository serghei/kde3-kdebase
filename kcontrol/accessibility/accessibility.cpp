/****************************************************************************
  accessibility.cpp
  KDE Control Accessibility module to control Bell, Keyboard and ?Mouse?
  -------------------
  Copyright : (c) 2000 Matthias Hölzer-Klüpfel
  -------------------
  Original Author: Matthias Hölzer-Klüpfel
  Contributors: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 ****************************************************************************/

/****************************************************************************
 *                                                                          *
 *   This program is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation; either version 2 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 ****************************************************************************/

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>

#include <kaboutdata.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include "accessibility.moc"

typedef KGenericFactory<AccessibilityConfig, QWidget> AccessibilityFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_accessibility, AccessibilityFactory("kcmaccessibility") )

/**
 * This function checks if the kaccess daemon needs to be run
 * This function will be deprecated since the kaccess daemon will be part of kded
 */
// static bool needToRunKAccessDaemon( KConfig *config ){
//    KConfigGroup group( config, "Bell" );
//
//    if(!group.readBoolEntry("SystemBell", true)){
//       return true;
//    }
//    if(group.readBoolEntry("ArtsBell", false)){
//       return true;
//    }
//    if(group.readBoolEntry("VisibleBell", false)){
//       return true;
//    }
//    return false; // don't need it
// }

AccessibilityConfig::AccessibilityConfig(QWidget *parent, const char *name, const QStringList &)
  : AccessibilityConfigWidget( parent, name){

   KAboutData *about =
   new KAboutData(I18N_NOOP("kcmaccessiblity"), I18N_NOOP("KDE Accessibility Tool"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 2000, Matthias Hoelzer-Kluepfel"));

   about->addAuthor("Matthias Hoelzer-Kluepfel", I18N_NOOP("Author") , "hoelzer@kde.org");
   about->addAuthor("José Pablo Ezequiel Fernández", I18N_NOOP("Author") , "pupeno@kde.org");
   setAboutData( about );

   kdDebug() << "Running: AccessibilityConfig::AccessibilityConfig(QWidget *parent, const char *name, const QStringList &)" << endl;
   // TODO: set the KURL Dialog to open just audio files
   connect( mainTab, SIGNAL(currentChanged(QWidget*)), this, SIGNAL(quickHelpChanged()) );
   load();
}


AccessibilityConfig::~AccessibilityConfig(){
   kdDebug() << "Running: AccessibilityConfig::~AccessibilityConfig()" << endl;
}

void AccessibilityConfig::load()
{
   load( false );
}

void AccessibilityConfig::load( bool useDefaults )
{
   kdDebug() << "Running: AccessibilityConfig::load()" << endl;
   
   KConfig *bell = new KConfig("bellrc", true);
  
   bell->setReadDefaults( useDefaults );

   bell->setGroup("General");
   systemBell->setChecked(bell->readBoolEntry("SystemBell", false));
   customBell->setChecked(bell->readBoolEntry("CustomBell", false));
   visibleBell->setChecked(bell->readBoolEntry("VisibleBell", false));
   
   bell->setGroup("CustomBell");
   soundToPlay->setURL(bell->readPathEntry("Sound", ""));

   bell->setGroup("Visible");
   invertScreen->setChecked(bell->readBoolEntry("Invert", true));
   flashScreen->setChecked(bell->readBoolEntry("Flash", false));
   // TODO: There has to be a cleaner way.
   QColor *redColor = new QColor(Qt::red);
   flashScreenColor->setColor(bell->readColorEntry("FlashColor", redColor));
   delete redColor;
   visibleBellDuration->setValue(bell->readNumEntry("Duration", 500));
  
   delete bell;
   emit changed( useDefaults );
}


void AccessibilityConfig::save(){
   kdDebug() << "Running: AccessibilityConfig::save()" << endl;
   
   KConfig *bell = new KConfig("bellrc");
   
   bell->setGroup("General");
   bell->writeEntry("SystemBell", systemBell->isChecked());
   bell->writeEntry("CustomBell", customBell->isChecked());
   bell->writeEntry("VisibleBell", visibleBell->isChecked());
   
   bell->setGroup("CustomBell");
   bell->writePathEntry("Sound", soundToPlay->url());

   bell->setGroup("Visible");
   bell->writeEntry("Invert", invertScreen->isChecked());
   bell->writeEntry("Flash", flashScreen->isChecked());
   bell->writeEntry("FlashColor", flashScreenColor->color());
   bell->writeEntry("Duration", visibleBellDuration->value());
   
   bell->sync();
   delete bell;
}


void AccessibilityConfig::defaults()
{
   load( true );
}
