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
#include <qstring.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>

#include <kgenericfactory.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kfileshare.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kdialog.h>
#include <kglobal.h>
#include <dcopref.h>

#include "propertiespage.h"
#include "propsdlgshareplugin.h"
#include "../libmediacommon/medium.h"

typedef KGenericFactory<PropsDlgSharePlugin, KPropertiesDialog> PropsDlgSharePluginFactory;

K_EXPORT_COMPONENT_FACTORY( media_propsdlgplugin,
                            PropsDlgSharePluginFactory("media_propsdlgplugin") )

class PropsDlgSharePlugin::Private                            
{
  public:
    PropertiesPage* page; 
};
                            
PropsDlgSharePlugin::PropsDlgSharePlugin( KPropertiesDialog *dlg,
					  const char *, const QStringList & )
  : KPropsDlgPlugin(dlg), d(0)
{
  if (properties->items().count() != 1)
    return;

  KFileItem *item = properties->items().first();

  DCOPRef mediamanager("kded", "mediamanager");
  kdDebug() << "properties " << item->url() << endl;
  DCOPReply reply = mediamanager.call( "properties", item->url().url() );
  
  if ( !reply.isValid() )
    return;
  
  QVBox* vbox = properties->addVBoxPage(i18n("&Mounting"));
  
  d = new Private();
  
  d->page = new PropertiesPage(vbox, Medium::create(reply).id());
  connect(d->page, SIGNAL(changed()),
	  SLOT(slotChanged()));

  //  QTimer::singleShot(100, this, SLOT(slotChanged()));
  
}                            

void PropsDlgSharePlugin::slotChanged()
{
  kdDebug() << "slotChanged()\n";
  setDirty(true);
}

PropsDlgSharePlugin::~PropsDlgSharePlugin()
{
  delete d;
}

void PropsDlgSharePlugin::applyChanges() 
{
  kdDebug() << "applychanges\n";
  if (!d->page->save()) {
    properties->abortApplying();
  }
}


#include "propsdlgshareplugin.moc"

