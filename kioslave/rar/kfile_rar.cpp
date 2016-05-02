/***************************************************************************
 *   Copyright (C) 2005-2006 by Raul Fernandes                             *
 *   rgfbr@yahoo.com.br                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <config.h>
#include "kfile_rar.h"
#include "krar.h"

#include <kgenericfactory.h>
#include <kio/global.h>

typedef KGenericFactory< KRarPlugin > KRarFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_rar, KRarFactory("kfile_rar"))

KRarPlugin::KRarPlugin(QObject *parent, const char *name, const QStringList &args) : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo *info = addMimeTypeInfo("application/x-rar-compressed");

    KFileMimeTypeInfo::GroupInfo *group = 0L;

    group = addGroupInfo(info, "RarInfo", i18n("Rar Information"));

    KFileMimeTypeInfo::ItemInfo *item;

    item = addItemInfo(group, "Directories", i18n("Directories"), QVariant::UInt);
    item = addItemInfo(group, "Files", i18n("Files"), QVariant::UInt);
    item = addItemInfo(group, "Unpacked", i18n("Unpacked"), QVariant::ULongLong);
    setUnit(item, KFileMimeTypeInfo::Bytes);
    item = addItemInfo(group, "Packed", i18n("Packed"), QVariant::ULongLong);
    setUnit(item, KFileMimeTypeInfo::Bytes);
    item = addItemInfo(group, "Ratio", i18n("Ratio"), QVariant::String);
    item = addItemInfo(group, "ArchiveVolume", i18n("Archive Volume"), QVariant::String);
    item = addItemInfo(group, "Lock", i18n("Lock"), QVariant::String);
    item = addItemInfo(group, "Solid", i18n("Solid"), QVariant::String);
    item = addItemInfo(group, "Auth Info", i18n("Auth Info"), QVariant::String);
}

bool KRarPlugin::readInfo(KFileMetaInfo &info, uint /*what*/)
{
    KFileMetaInfoGroup group = appendGroup(info, "RarInfo");

    if(info.url().protocol() != "file")
        return false;

    KRar rar(info.path());
    if(!rar.open(IO_ReadOnly))
    {
        rar.close();
        return false;
    }

    appendItem(group, "Directories", rar.dirs());
    appendItem(group, "Files", rar.files());
    appendItem(group, "Unpacked", rar.uncompressed());
    appendItem(group, "Packed", rar.compressed());
    appendItem(group, "Ratio", QString("%1 %").arg(rar.ratio()));
    if(rar.isArchiveVolume())
        appendItem(group, "ArchiveVolume", rar.volNum());
    else
        appendItem(group, "ArchiveVolume", i18n("None"));
    appendItem(group, "Lock", (rar.isLocked() ? i18n("Yes") : i18n("No")));
    appendItem(group, "Solid", (rar.isSolid() ? i18n("Yes") : i18n("No")));
    appendItem(group, "Auth Info", (rar.hasAuthInfo() ? i18n("Yes") : i18n("No")));

    rar.close();

    return true;
}

#include "kfile_rar.moc"
