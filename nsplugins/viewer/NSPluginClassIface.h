/*

  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
                     Stefan Schimanski <1Stein@gmx.de>

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



#ifndef __NSPluginClassIface_h__
#define __NSPluginClassIface_h__


#include <qstringlist.h>
#include <qcstring.h>
#include <dcopobject.h>
#include <dcopref.h>


class NSPluginViewerIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:
  virtual void shutdown() = 0;
  virtual DCOPRef newClass(QString plugin) = 0;
};


class NSPluginClassIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:

  virtual DCOPRef newInstance(QString url, QString mimeType, Q_INT8 embed,
                              QStringList argn, QStringList argv,
                              QString appId, QString callbackId, Q_INT8 reload,
                              Q_INT8 doPost, QByteArray postData, Q_UINT32 xembed) = 0;
  virtual QString getMIMEDescription() = 0;

};


class NSPluginInstanceIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:

  virtual void shutdown() = 0;

  virtual int winId() = 0;

  virtual int setWindow(Q_INT8 remove=0) = 0;

  virtual void resizePlugin(Q_INT32 w, Q_INT32 h) = 0;

  virtual void javascriptResult(Q_INT32 id, QString result) = 0;

  virtual void displayPlugin() = 0;
  
  virtual void gotFocusIn() = 0;
  virtual void gotFocusOut() = 0;
};


#endif

