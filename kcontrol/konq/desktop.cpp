// -*- c-basic-offset: 2 -*-
/**
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qslider.h>

#include <kapplication.h>
#include <kglobal.h>
#include <dcopclient.h>
#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kconfig.h>

#include <netwm.h>

#include "desktop.h"
#include "desktop.moc"

extern "C"
{
  KDE_EXPORT KCModule *create_virtualdesktops(QWidget *parent, const char * /*name*/)
  {
    return new KDesktopConfig(parent, "kcmkonq");
  }
}

// I'm using lineedits by intention as it makes sence to be able
// to see all desktop names at the same time. It also makes sense to
// be able to TAB through those line edits fast. So don't send me mails
// asking why I did not implement a more intelligent/smaller GUI.

KDesktopConfig::KDesktopConfig(QWidget *parent, const char * /*name*/)
  : KCModule(parent, "kcmkonq")
{

  setQuickHelp( i18n("<h1>Multiple Desktops</h1>In this module, you can configure how many virtual desktops you want and how these should be labeled."));

  Q_ASSERT(maxDesktops % 2 == 0);

  QVBoxLayout *layout = new QVBoxLayout(this, 0, KDialog::spacingHint());

  // number group
  QGroupBox *number_group = new QGroupBox(this);

  QHBoxLayout *lay = new QHBoxLayout(number_group,
                     KDialog::marginHint(),
                     KDialog::spacingHint());

  QLabel *label = new QLabel(i18n("N&umber of desktops: "), number_group);
  _numInput = new KIntNumInput(4, number_group);
  _numInput->setRange(1, maxDesktops, 1, true);
  connect(_numInput, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)));
  connect(_numInput, SIGNAL(valueChanged(int)),  SLOT( changed() ));
  label->setBuddy( _numInput );
  QString wtstr = i18n( "Here you can set how many virtual desktops you want on your KDE desktop. Move the slider to change the value." );
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( _numInput, wtstr );

  lay->addWidget(label);
  lay->addWidget(_numInput);
  lay->setStretchFactor( _numInput, 2 );

  layout->addWidget(number_group);

  // name group
  QGroupBox *name_group = new QGroupBox(i18n("Desktop &Names"), this);

  name_group->setColumnLayout(4, Horizontal);

  for(int i = 0; i < (maxDesktops/2); i++)
    {
      _nameLabel[i] = new QLabel(i18n("Desktop %1:").arg(i+1), name_group);
      _nameInput[i] = new KLineEdit(name_group);
      _nameLabel[i+(maxDesktops/2)] = new QLabel(i18n("Desktop %1:").arg(i+(maxDesktops/2)+1), name_group);
      _nameInput[i+(maxDesktops/2)] = new KLineEdit(name_group);
      QWhatsThis::add( _nameLabel[i], i18n( "Here you can enter the name for desktop %1" ).arg( i+1 ) );
      QWhatsThis::add( _nameInput[i], i18n( "Here you can enter the name for desktop %1" ).arg( i+1 ) );
      QWhatsThis::add( _nameLabel[i+(maxDesktops/2)], i18n( "Here you can enter the name for desktop %1" ).arg( i+(maxDesktops/2)+1 ) );
      QWhatsThis::add( _nameInput[i+(maxDesktops/2)], i18n( "Here you can enter the name for desktop %1" ).arg( i+(maxDesktops/2)+1 ) );

      connect(_nameInput[i], SIGNAL(textChanged(const QString&)),
           SLOT( changed() ));
      connect(_nameInput[i+(maxDesktops/2)], SIGNAL(textChanged(const QString&)),
           SLOT( changed() ));
    }

  for(int i = 1; i < maxDesktops; i++)
      setTabOrder( _nameInput[i-1], _nameInput[i] );

  layout->addWidget(name_group);

  _wheelOption = new QCheckBox(i18n("Mouse wheel over desktop background switches desktop"), this);
  connect(_wheelOption,SIGNAL(toggled(bool)), SLOT( changed() ));

  layout->addWidget(_wheelOption);
  layout->addStretch(1);

  load();
}

void KDesktopConfig::load()
{
	load( false );
}
	
void KDesktopConfig::load( bool useDefaults )
{
  // get number of desktops
  NETRootInfo info( qt_xdisplay(), NET::NumberOfDesktops | NET::DesktopNames );
  int n = info.numberOfDesktops();

  int konq_screen_number = 0;
  if (qt_xdisplay())
     konq_screen_number = DefaultScreen(qt_xdisplay());

  QCString groupname;
  if (konq_screen_number == 0)
     groupname = "Desktops";
  else
     groupname.sprintf("Desktops-screen-%d", konq_screen_number);

  KConfig * kwinconfig = new KConfig("kwinrc");

  kwinconfig->setReadDefaults( useDefaults );

  kwinconfig->setGroup("Desktops");
  for(int i = 1; i <= maxDesktops; i++)
  {
    QString key_name(QString("Name_") + QString::number(i));
    QString name = QString::fromUtf8(info.desktopName(i));
    if (name.isEmpty()) // Get name from configuration if none is set in the WM.
    {
        name = kwinconfig->readEntry(key_name, i18n("Desktop %1").arg(i));
    }
    _nameInput[i-1]->setText(name);

    // Is this entry immutable or not in the range of configured desktops?
    _labelImmutable[i - 1] = kwinconfig->entryIsImmutable(key_name);
    _nameInput[i-1]->setEnabled(i <= n && !_labelImmutable[i - 1]);
  }

  _numInput->setEnabled(!kwinconfig->entryIsImmutable("Number"));

  delete kwinconfig;
  kwinconfig = 0;

  QString configfile;
  if (konq_screen_number == 0)
      configfile = "kdesktoprc";
  else
      configfile.sprintf("kdesktop-screen-%drc", konq_screen_number);

  KConfig *config = new KConfig(configfile, false, false);

  config->setReadDefaults( useDefaults );

  config->setGroup("Mouse Buttons");
  _wheelOption->setChecked(config->readBoolEntry("WheelSwitchesWorkspace",false));

  _wheelOptionImmutable = config->entryIsImmutable("WheelSwitchesWorkspace");

  if (_wheelOptionImmutable || n<2)
     _wheelOption->setEnabled( false );

  delete config;
  config = 0;

  _numInput->setValue(n);
  emit changed( useDefaults );
}

void KDesktopConfig::save()
{
  NETRootInfo info( qt_xdisplay(), NET::NumberOfDesktops | NET::DesktopNames );
  // set desktop names
  for(int i = 1; i <= maxDesktops; i++)
  {
    info.setDesktopName(i, (_nameInput[i-1]->text()).utf8());
    info.activate();
  }
  // set number of desktops
  info.setNumberOfDesktops(_numInput->value());
  info.activate();

  XSync(qt_xdisplay(), FALSE);

  int konq_screen_number = 0;
  if (qt_xdisplay())
     konq_screen_number = DefaultScreen(qt_xdisplay());

  QCString appname;
  if (konq_screen_number == 0)
      appname = "kdesktop";
  else
      appname.sprintf("kdesktop-screen-%d", konq_screen_number);

  KConfig *config = new KConfig(appname + "rc");
  config->setGroup("Mouse Buttons");
  config->writeEntry("WheelSwitchesWorkspace", _wheelOption->isChecked());
  delete config;

  // Tell kdesktop about the new config file
  if ( !kapp->dcopClient()->isAttached() )
     kapp->dcopClient()->attach();
  QByteArray data;

  kapp->dcopClient()->send( appname, "KDesktopIface", "configure()", data );

  emit changed(false);
}

void KDesktopConfig::defaults()
{
	load( true );
}

void KDesktopConfig::slotValueChanged(int n)
{
  for(int i = 0; i < maxDesktops; i++)
  { _nameInput[i]->setEnabled(i < n && !_labelImmutable[i]); }
  if (!_wheelOptionImmutable)
  { _wheelOption->setEnabled(n>1); }
  emit changed(true);
}
