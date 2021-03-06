/**
 *  kcmhtmlsearch.cpp
 *
 *  Copyright (c) 2000 Matthias H�lzer-Kl�pfel <hoelzer@kde.org>
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

#include <qlayout.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kurlrequester.h>
#include <klineedit.h>

#include "htmlsearchconfig.h"
#include "htmlsearchconfig.moc"

namespace KHC {

HtmlSearchConfig::HtmlSearchConfig(QWidget *parent, const char *name) : QWidget(parent, name)
{
    QVBoxLayout *vbox = new QVBoxLayout(this, 5);


    QGroupBox *gb = new QGroupBox(i18n("ht://dig"), this);
    vbox->addWidget(gb);

    QGridLayout *grid = new QGridLayout(gb, 3, 2, 6, 6);

    grid->addRowSpacing(0, gb->fontMetrics().lineSpacing());

    QLabel *l = new QLabel(i18n("The fulltext search feature makes use of the "
                                "ht://dig HTML search engine. "
                                "You can get ht://dig at the"),
                           gb);
    l->setAlignment(QLabel::WordBreak);
    l->setMinimumSize(l->sizeHint());
    grid->addMultiCellWidget(l, 1, 1, 0, 1);
    QWhatsThis::add(gb, i18n("Information about where to get the ht://dig package."));

    KURLLabel *url = new KURLLabel(gb);
    url->setURL("http://www.htdig.org");
    url->setText(i18n("ht://dig home page"));
    url->setAlignment(QLabel::AlignHCenter);
    grid->addMultiCellWidget(url, 2, 2, 0, 1);
    connect(url, SIGNAL(leftClickedURL(const QString &)), this, SLOT(urlClicked(const QString &)));

    gb = new QGroupBox(i18n("Program Locations"), this);

    vbox->addWidget(gb);
    grid = new QGridLayout(gb, 4, 2, 6, 6);
    grid->addRowSpacing(0, gb->fontMetrics().lineSpacing());

    mHtsearchUrl = new KURLRequester(gb);
    l = new QLabel(mHtsearchUrl, i18n("htsearch:"), gb);
    l->setBuddy(mHtsearchUrl);
    grid->addWidget(l, 1, 0);
    grid->addWidget(mHtsearchUrl, 1, 1);
    connect(mHtsearchUrl->lineEdit(), SIGNAL(textChanged(const QString &)), SIGNAL(changed()));
    QString wtstr = i18n("Enter the URL of the htsearch CGI program.");
    QWhatsThis::add(mHtsearchUrl, wtstr);
    QWhatsThis::add(l, wtstr);

    mIndexerBin = new KURLRequester(gb);
    l = new QLabel(mIndexerBin, i18n("Indexer:"), gb);
    l->setBuddy(mIndexerBin);
    grid->addWidget(l, 2, 0);
    grid->addWidget(mIndexerBin, 2, 1);
    connect(mIndexerBin->lineEdit(), SIGNAL(textChanged(const QString &)), SIGNAL(changed()));
    wtstr = i18n("Enter the path to your htdig indexer program here.");
    QWhatsThis::add(mIndexerBin, wtstr);
    QWhatsThis::add(l, wtstr);

    mDbDir = new KURLRequester(gb);
    mDbDir->setMode(KFile::Directory | KFile::LocalOnly);
    l = new QLabel(mDbDir, i18n("htdig database:"), gb);
    l->setBuddy(mDbDir);
    grid->addWidget(l, 3, 0);
    grid->addWidget(mDbDir, 3, 1);
    connect(mDbDir->lineEdit(), SIGNAL(textChanged(const QString &)), SIGNAL(changed()));
    wtstr = i18n("Enter the path to the htdig database folder.");
    QWhatsThis::add(mDbDir, wtstr);
    QWhatsThis::add(l, wtstr);
}

HtmlSearchConfig::~HtmlSearchConfig()
{
    kdDebug() << "~HtmlSearchConfig()" << endl;
}

void HtmlSearchConfig::makeReadOnly()
{
    mHtsearchUrl->setEnabled(false);
    mIndexerBin->setEnabled(false);
    mDbDir->setEnabled(false);
}

void HtmlSearchConfig::load(KConfig *config)
{
    config->setGroup("htdig");

    mHtsearchUrl->lineEdit()->setText(config->readPathEntry("htsearch", kapp->dirs()->findExe("htsearch")));
    mIndexerBin->lineEdit()->setText(config->readPathEntry("indexer"));
    mDbDir->lineEdit()->setText(config->readPathEntry("dbdir", "/opt/www/htdig/db/"));
}

void HtmlSearchConfig::save(KConfig *config)
{
    config->setGroup("htdig");

    config->writePathEntry("htsearch", mHtsearchUrl->lineEdit()->text());
    config->writePathEntry("indexer", mIndexerBin->lineEdit()->text());
    config->writePathEntry("dbdir", mDbDir->lineEdit()->text());
}

void HtmlSearchConfig::defaults()
{
    mHtsearchUrl->lineEdit()->setText(kapp->dirs()->findExe("htsearch"));
    mIndexerBin->lineEdit()->setText("");
    mDbDir->lineEdit()->setText("/opt/www/htdig/db/");
}

void HtmlSearchConfig::urlClicked(const QString &url)
{
    kapp->invokeBrowser(url);
}

} // End namespace KHC
// vim:ts=2:sw=2:et
