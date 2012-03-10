/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "documentmanager.h"
#include "documentmanager.moc"

#include "plugin.h"
#include "viewmanager.h"
#include "toolviewmanager.h"
#include "pluginmanager.h"

#include "application.h"

#include "../app/katedocmanager.h"

namespace Kate
{

class PrivateDocumentManager
  {
  public:
    PrivateDocumentManager ()
    {
    }

    ~PrivateDocumentManager ()
    {
    }

    KateDocManager *docMan;
  };

DocumentManager::DocumentManager (void *documentManager) : QObject ((KateDocManager*) documentManager)
{
  d = new PrivateDocumentManager ();
  d->docMan = (KateDocManager*) documentManager;
}

DocumentManager::~DocumentManager ()
{
  delete d;
}

Document *DocumentManager::document (uint n)
{
  return d->docMan->document (n);
}

Document *DocumentManager::activeDocument ()
{
  return d->docMan->activeDocument ();
}

Document *DocumentManager::documentWithID (uint id)
{
  return d->docMan->documentWithID (id);
}

int DocumentManager::findDocument (const KURL &url)
{
  return d->docMan->findDocument (url);
}

bool DocumentManager::isOpen (const KURL &url)
{
  return d->docMan->isOpen (url);
}

uint DocumentManager::documents ()
{
  return d->docMan->documents ();
}

Document *DocumentManager::openURL(const KURL&url,const QString &encoding,uint *id)
{
  return d->docMan->openURL (url, encoding, id);
}

bool DocumentManager::closeDocument(Document *document)
{
  return d->docMan->closeDocument (document);
}

bool DocumentManager::closeDocument(uint n)
{
  return d->docMan->closeDocument (n);
}

bool DocumentManager::closeDocumentWithID(uint id)
{
  return d->docMan->closeDocument (id);
}

bool DocumentManager::closeAllDocuments()
{
  return d->docMan->closeAllDocuments ();
}

DocumentManager *documentManager ()
{
  return application()->documentManager ();
}

}

