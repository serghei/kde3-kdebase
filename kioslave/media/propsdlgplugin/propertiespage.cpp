/*
  Copyright (c) 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <qcheckbox.h>
#include <qtooltip.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qregexp.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <kdebug.h>
#include "propertiespage.h"
#include <dcopref.h>

// keep in sync with .ui and kded module
const char *short_names[] = {"lower", "win95", "winnt", "mixed", 0 };
const char *journales[] = {"data", "ordered", "writeback", 0 };

PropertiesPage::PropertiesPage(QWidget* parent, const QString &_id)
  : PropertiesPageGUI(parent), id(_id)
{
  kdDebug() << "props page " << id << endl;
  DCOPRef mediamanager("kded", "mediamanager");
  DCOPReply reply = mediamanager.call( "mountoptions", id);

  QStringList list;

  if (reply.isValid())
    list = reply;

  if (list.size()) {
    kdDebug() << "list " << list << endl;
    
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
      {
	QString key = (*it).left((*it).find('='));
	QString value = (*it).mid((*it).find('=') + 1);
	kdDebug() << "key '" << key << "' value '" << value << "'\n";
	options[key] = value;
      }

    if (!options.contains("ro")) 
      option_ro->hide();
    else
      option_ro->setChecked(options["ro"] == "true");
    connect( option_ro, SIGNAL( stateChanged(int) ), SIGNAL( changed() ) );

    if (!options.contains("quiet")) 
      option_quiet->hide();
    else
      option_quiet->setChecked(options["quiet"] == "true");
    connect( option_quiet, SIGNAL( stateChanged(int) ), SIGNAL( changed() ) );

    if (!options.contains("sync")) 
      option_sync->hide();
    else
      option_sync->setChecked(options["sync"] == "true");
    connect( option_sync, SIGNAL( stateChanged(int) ), SIGNAL( changed() ) );

    if (!options.contains("atime")) 
      option_atime->hide();
    else
      option_atime->setChecked(options["atime"] == "true");
    connect( option_atime, SIGNAL( stateChanged(int) ), SIGNAL( changed() ) );

    if (!options.contains("flush")) 
      option_flush->hide();
    else
      option_flush->setChecked(options["flush"] == "true");
    connect( option_flush, SIGNAL( stateChanged(int) ), SIGNAL( changed() ) );

    if (!options.contains("utf8")) 
      option_utf8->hide();
    else
      option_utf8->setChecked(options["utf8"] == "true");
    connect( option_utf8, SIGNAL( stateChanged(int) ), SIGNAL( changed() ) );

    if (!options.contains("uid")) 
      option_uid->hide();
    else
      option_uid->setChecked(options["uid"] == "true");
    connect( option_uid, SIGNAL( stateChanged(int) ), SIGNAL( changed() ) );

    if (!options.contains("shortname")) 
      {
	option_shortname->hide();
	text_shortname->hide();
      }
    else 
      {
	for (int index = 0; short_names[index]; ++index)
	  if (options["shortname"] == short_names[index]) 
	    {
	      option_shortname->setCurrentItem(index);
	      break;
	    }
	connect( option_shortname, SIGNAL( activated(int) ), SIGNAL( changed() ) );
      }

    if (!options.contains("journaling")) 
      {
	text_journaling->hide();
	option_journaling->hide();
      } 
    else
      {
	for (int index = 0; journales[index]; ++index)
	  if (options["journaling"] == journales[index]) 
	    {
	      option_journaling->setCurrentItem(index);
	      break;
	    }
	connect( option_journaling, SIGNAL( activated(int) ), SIGNAL( changed() ) );
      }

    label_filesystem->setText(i18n("Filesystem: %1").arg(options["filesystem"]));
    option_mountpoint->setText(options["mountpoint"]);
    connect( option_mountpoint, SIGNAL( textChanged( const QString &) ), SIGNAL( changed() ) );
    option_automount->setChecked(options["automount"] == "true");
    connect( option_automount, SIGNAL( stateChanged(int) ), SIGNAL( changed() ) );

    if (!options.contains("journaling") &&
	!options.contains("shortname") &&
	!options.contains("uid") &&
	!options.contains("utf8") &&
	!options.contains("flush"))
      groupbox_specific->hide();

  } else {
    
    groupbox_generic->setEnabled(false);
    groupbox_specific->setEnabled(false);
    label_filesystem->hide();
  }
}

PropertiesPage::~PropertiesPage() 
{
}

bool PropertiesPage::save() 
{
  QStringList result;

  if (options.contains("ro")) 
    result << QString("ro=%1").arg(option_ro->isChecked() ? "true" : "false");

  if (options.contains("quiet")) 
    result << QString("quiet=%1").arg(option_quiet->isChecked() ? "true" : "false");

  if (options.contains("sync"))
    result << QString("sync=%1").arg(option_sync->isChecked() ? "true" : "false");

  if (options.contains("atime"))
    result << QString("atime=%1").arg(option_atime->isChecked() ? "true" : "false");

  if (options.contains("flush"))
    result << QString("flush=%1").arg(option_flush->isChecked() ? "true" : "false");

  if (options.contains("utf8"))
    result << QString("utf8=%1").arg(option_utf8->isChecked() ? "true" : "false");

  if (options.contains("uid"))
    result << QString("uid=%1").arg(option_uid->isChecked() ? "true" : "false");

  if (options.contains("shortname")) 
    result << QString("shortname=%1").arg(short_names[option_shortname->currentItem()]);

  if (options.contains("journaling")) 
    result << QString("journaling=%1").arg(journales[option_journaling->currentItem()]);

  QString mp = option_mountpoint->text();
  if (!mp.startsWith("/media/"))
    {
      KMessageBox::sorry(this, i18n("Mountpoint has to be below /media"));
      return false;
    }
  result << QString("mountpoint=%1").arg(mp);
  result << QString("automount=%1").arg(option_automount->isChecked() ? "true" : "false");

  kdDebug() << result << endl;

  DCOPRef mediamanager("kded", "mediamanager");
  DCOPReply reply = mediamanager.call( "setMountoptions", id, result);
  
  if (reply.isValid())
    return (bool)reply;
  else {
    KMessageBox::sorry(this,
		       i18n("Saving the changes failed"));
    
    return false;
  }
}  

#include "propertiespage.moc"
