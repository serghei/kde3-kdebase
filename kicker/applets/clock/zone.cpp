/************************************************************

Copyright (c) 1996-2002 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "zone.h"

#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <klocale.h>

#include <qfile.h>
#include <qtooltip.h>
#include <klistview.h>

#include <time.h>
#include <stdlib.h> // for getenv()

Zone::Zone(KConfig* conf):
    config(conf),
    _zoneIndex(0)
{
    _defaultTZ = ::getenv("TZ");
    tzset();

    config->setGroup("General");

    /* default displayable timezones */
    QString tzList = config->readEntry("RemoteZones");
    _remotezonelist = QStringList::split(",", tzList);
    setZone(config->readNumEntry("Initial_TZ", 0));
}

Zone::~Zone()
{
    writeSettings();
}

void Zone::setZone(int z)
{
    if (_zoneIndex > _remotezonelist.count())
        z = 0;

    _zoneIndex = z;
}

QString Zone::zone(int z) const
{
    return (z == 0 ? _defaultTZ : _remotezonelist[z-1]);
}

int Zone::calc_TZ_offset(const QString& zone, bool /* reset */)
{
  const KTimezone *z = zone.isEmpty() ? m_zoneDb.local() : m_zoneDb.zone(zone);

  if (!z)
  {
    z = m_zoneDb.local();
  }

  if (z)
  {
    return -z->offset(Qt::LocalTime);
  }

  return 0;
}

void Zone::readZoneList(KListView *listView )
{
  const KTimezones::ZoneMap zones = m_zoneDb.allZones();
  QMap<QString, QListViewItem*> KontinentMap;

  listView->setRootIsDecorated(true);
  for (KTimezones::ZoneMap::ConstIterator it = zones.begin(); it != zones.end(); ++it)
  {
    const KTimezone *zone = it.data();
    QString tzName = zone->name();
    QString comment = zone->comment();
    if (!comment.isEmpty())
      comment = i18n(comment.utf8());

    const QStringList KontCity = QStringList::split("/", i18n(tzName.utf8()).replace("_", " "));
    QListViewItem* Kontinent = KontinentMap[KontCity[0]];
    if (!Kontinent) {
	KontinentMap[KontCity[0]] = new QListViewItem(listView, KontCity[0]);
	Kontinent = KontinentMap[KontCity[0]];
	Kontinent->setExpandable(true);
    }

    QCheckListItem *li = new QCheckListItem(Kontinent, KontCity[1], QCheckListItem::CheckBox);
    li->setText(1, comment);
    li->setText(2, tzName); /* store complete path in ListView */

    if (_remotezonelist.findIndex(tzName) != -1)
       li->setOn(true);

    // locate the flag from /l10n/%1/flag.png
    // if not available select default "C" flag
    QString flag = locate( "locale", QString("l10n/%1/flag.png").arg(zone->countryCode().lower()) );
    if (!QFile::exists(flag))
       flag = locate( "locale", "l10n/C/flag.png" );
    if (QFile::exists(flag))
       li->setPixmap(0, QPixmap(flag));
  }
}

void Zone::getSelectedZonelist(KListView *listView)
{
     _remotezonelist.clear();

     /* loop through all entries */
     QListViewItem *root = listView->firstChild();
     while (root) {
	if (root->firstChild()) {
		root = root->firstChild();
		continue;
	}

	QCheckListItem *cl = (QCheckListItem*) root;
	if (cl->isOn()) {
		_remotezonelist.append(cl->text(2));
	}

	if (root->nextSibling()) {
		root = root->nextSibling();
		continue;
	}
	root = root->parent();
	if (root)
		root = root->nextSibling();
     }
}

void Zone::nextZone()
{
    if (++_zoneIndex > _remotezonelist.count())
	_zoneIndex = 0;
}

void Zone::prevZone()
{
    if (_zoneIndex == 0)
	_zoneIndex = _remotezonelist.count();
    else
	--_zoneIndex;
}

void Zone::writeSettings()
{
    config->setGroup("General");
    config->writeEntry("RemoteZones", _remotezonelist.join(","));
    config->writeEntry("Initial_TZ",_zoneIndex);
    config->sync();
}
