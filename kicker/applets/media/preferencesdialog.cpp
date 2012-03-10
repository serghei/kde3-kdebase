/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "preferencesdialog.h"

#include <klocale.h>
#include <kmimetype.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <klistview.h>
#include <kfiledetailview.h>

class MediumTypeItem : public QCheckListItem
{
public:
	MediumTypeItem(QListView *parent, const QString name,
	               const QString mimetype)
		: QCheckListItem(parent, name, CheckBox),
		  mMimeType(mimetype) { }

	const QString &mimeType() const { return mMimeType; }

private:
	QString mMimeType;
};

class MediumItem : public QCheckListItem
{
public:
	MediumItem(QListView *parent, const QString name,
	           const KFileItem medium)
		: QCheckListItem(parent, name, CheckBox),
		  mMedium(medium) { }

	const QString itemURL() const { return mMedium.url().url(); }

private:
	KFileItem mMedium;
};



PreferencesDialog::PreferencesDialog(KFileItemList media, QWidget *parent,
                                     const char *name)
	: KDialogBase(Tabbed, i18n("Media Applet Preferences"), Ok|Cancel|Default,
	              Ok, parent, name, true),
	  mMedia(media)
{
	QVBox *types_page = addVBoxPage( i18n("Medium Types") );
	mpMediumTypesListView = new KListView(types_page);

	//mpMediumTypesListView->setFullWidth(true);
	mpMediumTypesListView->addColumn( i18n("Types to Display") );
	QWhatsThis::add(mpMediumTypesListView, i18n("Deselect the medium types which you do not want to see in the applet"));



	QVBox *media_page = addVBoxPage( i18n("Media") );
	mpMediaListView = new KListView(media_page);

	//mpMediaListView->setFullWidth(true);
	mpMediaListView->addColumn( i18n("Media to Display") );
	QWhatsThis::add(mpMediaListView, i18n("Deselect the media which you do not want to see in the applet"));

	slotDefault();
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::slotDefault()
{
	QStringList defaultExclude;

	defaultExclude << "media/hdd_mounted";
	defaultExclude << "media/hdd_unmounted";
	defaultExclude << "media/nfs_mounted";
	defaultExclude << "media/nfs_unmounted";
	defaultExclude << "media/smb_mounted";
	defaultExclude << "media/smb_unmounted";

	setExcludedMediumTypes(defaultExclude);
	setExcludedMedia(QStringList());
}

QStringList PreferencesDialog::excludedMediumTypes()
{
	QStringList excludedTypes;

	for(MediumTypeItem *it=static_cast<MediumTypeItem *>(mpMediumTypesListView->firstChild());
	    it; it=static_cast<MediumTypeItem *>(it->nextSibling()))
	{
		if(!it->isOn()) excludedTypes << it->mimeType();
	}

	return excludedTypes;
}

void PreferencesDialog::setExcludedMediumTypes(QStringList excludedTypesList)
{
	mpMediumTypesListView->clear();
	mpMediumTypesListView->setRootIsDecorated(false);
	KMimeType::List mimetypes = KMimeType::allMimeTypes();

	QValueListIterator<KMimeType::Ptr> it(mimetypes.begin());

	for(; it != mimetypes.end(); ++it)
	{
		if ((*it)->name().startsWith("media/"))
		{
			bool ok=excludedTypesList.contains((*it)->name())==0;
			MediumTypeItem *item = new MediumTypeItem(mpMediumTypesListView, (*it)->comment(), (*it)->name());
			item->setOn(ok);
		}
	}
}

QStringList PreferencesDialog::excludedMedia()
{
	QStringList excluded;

	for(MediumItem *it=static_cast<MediumItem *>(mpMediaListView->firstChild());
	    it; it=static_cast<MediumItem *>(it->nextSibling()))
	{
		if(!it->isOn()) excluded << it->itemURL();
	}

	return excluded;
}

void PreferencesDialog::setExcludedMedia(QStringList excludedList)
{
	mpMediaListView->clear();
	mpMediaListView->setRootIsDecorated(false);

	KFileItemListIterator it( mMedia );
	KFileItem *file;
	while ( (file = it.current()) != 0 )
	{
		++it;

		bool ok = excludedList.contains(file->url().url())==0;
		MediumItem *item = new MediumItem(mpMediaListView,
		                                  file->text(), *file);
		item->setOn(ok);
	}
}


#include "preferencesdialog.moc"
