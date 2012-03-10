/* This file is part of the KDE project
   Copyright (C) 2003 Ian Reinhart Geiser <geiseri@kde.org>

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

#ifndef _katedocmanager_Iface_h_
#define _katedocmanager_Iface_h_

#include <dcopobject.h>
#include <dcopref.h>

#include <kurl.h>

class KateDocManager;

class KateDocManagerDCOPIface : public DCOPObject
{
  K_DCOP

  public:
    KateDocManagerDCOPIface (KateDocManager *dm);

  k_dcop:
    DCOPRef document (uint n);

    DCOPRef activeDocument ();
    
    uint activeDocumentNumber ();

    DCOPRef documentWithID (uint id);

    int findDocument (KURL url);

    bool isOpen (KURL url);

    uint documents ();

    DCOPRef openURL (KURL url, QString encoding);

    bool closeDocument (uint n);

    bool closeDocumentWithID (uint id);

    bool closeAllDocuments ();

  private:
    KateDocManager *m_dm;
};
#endif
