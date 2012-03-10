/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

#ifndef __KATE_DOCMANAGER_H__
#define __KATE_DOCMANAGER_H__

#include "katemain.h"
#include "../interfaces/documentmanager.h"

#include <kate/document.h>

#include <qguardedptr.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qptrdict.h>
#include <qintdict.h>
#include <qmap.h>
#include <qpair.h>

namespace KParts { class Factory; }

class KConfig;
class DCOPObject;

class KateDocumentInfo
{
  public:
    KateDocumentInfo ()
     : modifiedOnDisc (false),
       modifiedOnDiscReason (0)
    {
    }

    bool modifiedOnDisc;
    unsigned char modifiedOnDiscReason;
};

typedef QPair<KURL,QDateTime> TPair;

class KateDocManager : public QObject
{
  Q_OBJECT

  public:
    KateDocManager (QObject *parent);
    ~KateDocManager ();

    static KateDocManager *self ();

    Kate::DocumentManager *documentManager () { return m_documentManager; };

    Kate::Document *createDoc ();
    void deleteDoc (Kate::Document *doc);

    Kate::Document *document (uint n);

    Kate::Document *activeDocument ();
    void setActiveDocument (Kate::Document *doc);

    Kate::Document *firstDocument ();
    Kate::Document *nextDocument ();

    // search document with right documentNumber()
    Kate::Document *documentWithID (uint id);

    const KateDocumentInfo *documentInfo (Kate::Document *doc);

    int findDocument (Kate::Document *doc);
    /** Returns the documentNumber of the doc with url URL or -1 if no such doc is found */
    int findDocument (KURL url);
    // Anders: The above is not currently stable ?
    Kate::Document *findDocumentByUrl( KURL url );

    bool isOpen(KURL url);

    uint documents ();

    QPtrList<Kate::Document> &documentList () { return m_docList; };

    Kate::Document *openURL(const KURL&,const QString &encoding=QString::null,uint *id =0,bool isTempFile=false);

    bool closeDocument(class Kate::Document *,bool closeURL=true);
    bool closeDocument(uint);
    bool closeDocumentWithID(uint);
    bool closeAllDocuments(bool closeURL=true);

    QPtrList<Kate::Document> modifiedDocumentList();
    bool queryCloseDocuments(KateMainWindow *w);

    void saveDocumentList (class KConfig *config);
    void restoreDocumentList (class KConfig *config);

    DCOPObject *dcopObject () { return m_dcop; };

    inline bool getSaveMetaInfos() { return m_saveMetaInfos; };
    inline void setSaveMetaInfos(bool b) { m_saveMetaInfos = b; };

    inline int getDaysMetaInfos() { return m_daysMetaInfos; };
    inline void setDaysMetaInfos(int i) { m_daysMetaInfos = i; };

  public slots:
    /**
     * saves all documents that has at least one view.
     * documents with no views are ignored :P
     */
    void saveAll();

  signals:
    void documentCreated (Kate::Document *doc);
    void documentDeleted (uint documentNumber);
    void documentChanged ();
    void initialDocumentReplaced ();

  private slots:
    void slotModifiedOnDisc (Kate::Document *doc, bool b, unsigned char reason);
    void slotModChanged(Kate::Document *doc);

  private:
    bool loadMetaInfos(Kate::Document *doc, const KURL &url);
    void saveMetaInfos(Kate::Document *doc);
    bool computeUrlMD5(const KURL &url, QCString &result);

    Kate::DocumentManager *m_documentManager;
    QPtrList<Kate::Document> m_docList;
    QIntDict<Kate::Document> m_docDict;
    QPtrDict<KateDocumentInfo> m_docInfos;
    QMap<uint,TPair> m_tempFiles;
    QGuardedPtr<Kate::Document> m_currentDoc;
    KConfig *m_metaInfos;
    bool m_saveMetaInfos;
    int m_daysMetaInfos;

    DCOPObject *m_dcop;

    KParts::Factory *m_factory;

};

#endif
// kate: space-indent on; indent-width 2; replace-tabs on;
