/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <qwhatsthis.h>

#include <klocale.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kapplication.h>
#include <krun.h>

#include "global.h"
#include "helpwidget.h"

HelpWidget::HelpWidget(QWidget *parent) : QWhatsThis(parent)
{
  setBaseText();
}

void HelpWidget::setText(const QString& docPath, const QString& text)
{
  docpath = docPath;
  if (text.isEmpty() && docPath.isEmpty())
    setBaseText();
  else if (docPath.isEmpty())
    helptext = text;
  else
    helptext = (text + i18n("<p>Use the \"What's This?\" (Shift+F1) to get help on specific options.</p><p>To read the full manual click <a href=\"%1\">here</a>.</p>")
		      .arg(docPath.local8Bit()));
}

void HelpWidget::setBaseText()
{
  if (KCGlobal::isInfoCenter())
     helptext = (i18n("<h1>KDE Info Center</h1>"
			 "There is no quick help available for the active info module."
			 "<br><br>"
			 "Click <a href = \"kinfocenter/index.html\">here</a> to read the general Info Center manual.") );
  else
     helptext = (i18n("<h1>KDE Control Center</h1>"
			 "There is no quick help available for the active control module."
			 "<br><br>"
			 "Click <a href = \"kcontrol/index.html\">here</a> to read the general Control Center manual.") );
}

QString HelpWidget::text() const
{
    return helptext;
}

bool HelpWidget::clicked(const QString & _url)
{
    if ( _url.isNull() )
        return true;

    if ( _url.find('@') > -1 ) {
        kapp->invokeMailer(_url);
        return true;
    }

    KProcess process;
    KURL url(KURL("help:/"), _url);

    if (url.protocol() == "help" || url.protocol() == "man" || url.protocol() == "info") {
        process << "khelpcenter"
                << url.url();
        process.start(KProcess::DontCare);
    } else {
        new KRun(url);
    }
    return true;
}

void HelpWidget::handbookRequest()
{
    if (docpath.isEmpty())
        kdWarning() << "No handbook defined" << endl;
 
    clicked(docpath);
}
