/*

  This is an encapsulation of the  Netscape plugin API.

  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
                     Stefan Schimanski <1Stein@gmx.de>
  Copyright (c) 2003-2005 George Staikos <staikos@kde.org>

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


#ifndef __NS_PLUGIN_H__
#define __NS_PLUGIN_H__


#include <dcopobject.h>
#include "NSPluginClassIface.h"
#include "NSPluginCallbackIface_stub.h"


#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qptrqueue.h>
#include <qdict.h>
#include <qmap.h>
#include <qintdict.h>
#include <qguardedptr.h>

#include <kparts/browserextension.h> // for URLArgs
#include <kio/job.h>


#define XP_UNIX
#define MOZ_X11
#include "sdk/npupp.h"

typedef char *NP_GetMIMEDescriptionUPP(void);
typedef NPError NP_InitializeUPP(NPNetscapeFuncs *, NPPluginFuncs *);
typedef NPError NP_ShutdownUPP(void);


#include <X11/Intrinsic.h>


void quitXt();

class KLibrary;
class QTimer;


class NSPluginStreamBase : public QObject {
    Q_OBJECT
    friend class NSPluginInstance;

public:
    NSPluginStreamBase(class NSPluginInstance *instance);
    ~NSPluginStreamBase();

    KURL url()
    {
        return _url;
    }
    int pos()
    {
        return _pos;
    }
    void stop();

signals:
    void finished(NSPluginStreamBase *strm);

protected:
    void finish(bool err);
    bool pump();
    bool error()
    {
        return _error;
    }
    void queue(const QByteArray &data);
    bool create(const QString &url, const QString &mimeType, void *notify, bool forceNotify = false);
    int tries()
    {
        return _tries;
    }
    void inform();
    void updateURL(const KURL &newURL);

    class NSPluginInstance *_instance;
    uint16 _streamType;
    NPStream *_stream;
    void *_notifyData;
    KURL _url;
    QString _fileURL;
    QString _mimeType;
    QByteArray _data;
    class KTempFile *_tempFile;

private:
    int process(const QByteArray &data, int start);

    unsigned int _pos;
    QByteArray _queue;
    unsigned int _queuePos;
    int _tries;
    bool _onlyAsFile;
    bool _error;
    bool _informed;
    bool _forceNotify;
};


class NSPluginStream : public NSPluginStreamBase {
    Q_OBJECT

public:
    NSPluginStream(class NSPluginInstance *instance);
    ~NSPluginStream();

    bool get(const QString &url, const QString &mimeType, void *notifyData, bool reload = false);
    bool post(const QString &url, const QByteArray &data, const QString &mimeType, void *notifyData, const KParts::URLArgs &args);

protected slots:
    void data(KIO::Job *job, const QByteArray &data);
    void totalSize(KIO::Job *job, KIO::filesize_t size);
    void mimetype(KIO::Job *job, const QString &mimeType);
    void result(KIO::Job *job);
    void redirection(KIO::Job *job, const KURL &url);
    void resume();

protected:
    QGuardedPtr< KIO::TransferJob > _job;
    QTimer *_resumeTimer;
};


class NSPluginBufStream : public NSPluginStreamBase {
    Q_OBJECT

public:
    NSPluginBufStream(class NSPluginInstance *instance);
    ~NSPluginBufStream();

    bool get(const QString &url, const QString &mimeType, const QByteArray &buf, void *notifyData, bool singleShot = false);

protected slots:
    void timer();

protected:
    QTimer *_timer;
    bool _singleShot;
};


class NSPluginInstance : public QObject, public virtual NSPluginInstanceIface {
    Q_OBJECT

public:
    // constructor, destructor
    NSPluginInstance(NPP privateData, NPPluginFuncs *pluginFuncs, KLibrary *handle, int width, int height, QString src, QString mime, QString appId,
                     QString callbackId, bool embed, WId xembed, QObject *parent, const char *name = 0);
    ~NSPluginInstance();

    // DCOP functions
    void shutdown();
    int winId()
    {
        return _form != 0 ? XtWindow(_form) : 0;
    }
    int setWindow(Q_INT8 remove = 0);
    void resizePlugin(Q_INT32 w, Q_INT32 h);
    void javascriptResult(Q_INT32 id, QString result);
    void displayPlugin();
    void gotFocusIn();
    void gotFocusOut();

    // value handling
    NPError NPGetValue(NPPVariable variable, void *value);
    NPError NPSetValue(NPNVariable variable, void *value);

    // window handling
    NPError NPSetWindow(NPWindow *window);

    // stream functions
    NPError NPDestroyStream(NPStream *stream, NPReason reason);
    NPError NPNewStream(NPMIMEType type, NPStream *stream, NPBool seekable, uint16 *stype);
    void NPStreamAsFile(NPStream *stream, const char *fname);
    int32 NPWrite(NPStream *stream, int32 offset, int32 len, void *buf);
    int32 NPWriteReady(NPStream *stream);

    // URL functions
    void NPURLNotify(QString url, NPReason reason, void *notifyData);

    // Event handling
    uint16 HandleEvent(void *event);

    // signal emitters
    void emitStatus(const QString &message);
    void requestURL(const QString &url, const QString &mime, const QString &target, void *notify, bool forceNotify = false, bool reload = false);
    void postURL(const QString &url, const QByteArray &data, const QString &mime, const QString &target, void *notify, const KParts::URLArgs &args,
                 bool forceNotify = false);

    QString normalizedURL(const QString &url) const;

public slots:
    void streamFinished(NSPluginStreamBase *strm);

private slots:
    void timer();

private:
    friend class NSPluginStreamBase;

    static void forwarder(Widget, XtPointer, XEvent *, Boolean *);

    void destroy();

    bool _destroyed;
    bool _visible;
    void addTempFile(KTempFile *tmpFile);
    QPtrList< KTempFile > _tempFiles;
    NSPluginCallbackIface_stub *_callback;
    QPtrList< NSPluginStreamBase > _streams;
    KLibrary *_handle;
    QTimer *_timer;

    NPP _npp;
    NPPluginFuncs _pluginFuncs;

    Widget _area, _form, _toplevel;
    WId _xembed_window;
    QString _baseURL;
    int _width, _height;

    struct Request
    {
        // A GET request
        Request(const QString &_url, const QString &_mime, const QString &_target, void *_notify, bool _forceNotify = false, bool _reload = false)
        {
            url = _url;
            mime = _mime;
            target = _target;
            notify = _notify;
            post = false;
            forceNotify = _forceNotify;
            reload = _reload;
        }

        // A POST request
        Request(const QString &_url, const QByteArray &_data, const QString &_mime, const QString &_target, void *_notify,
                const KParts::URLArgs &_args, bool _forceNotify = false)
        {
            url = _url;
            mime = _mime;
            target = _target;
            notify = _notify;
            post = true;
            data = _data;
            args = _args;
            forceNotify = _forceNotify;
        }

        QString url;
        QString mime;
        QString target;
        QByteArray data;
        bool post;
        bool forceNotify;
        bool reload;
        void *notify;
        KParts::URLArgs args;
    };

    NPWindow _win;
    NPSetWindowCallbackStruct _win_info;
    QPtrQueue< Request > _waitingRequests;
    QMap< int, Request * > _jsrequests;
};


class NSPluginClass : public QObject, virtual public NSPluginClassIface {
    Q_OBJECT
public:
    NSPluginClass(const QString &library, QObject *parent, const char *name = 0);
    ~NSPluginClass();

    QString getMIMEDescription();
    DCOPRef newInstance(QString url, QString mimeType, Q_INT8 embed, QStringList argn, QStringList argv, QString appId, QString callbackId,
                        Q_INT8 reload, Q_INT8 post, QByteArray postData, Q_UINT32 xembed);
    void destroyInstance(NSPluginInstance *inst);
    bool error()
    {
        return _error;
    }

    void setApp(const QCString &app)
    {
        _app = app;
    }
    const QCString &app() const
    {
        return _app;
    }

protected slots:
    void timer();

private:
    int initialize();
    void shutdown();

    KLibrary *_handle;
    QString _libname;
    bool _constructed;
    bool _error;
    QTimer *_timer;

    NP_GetMIMEDescriptionUPP *_NP_GetMIMEDescription;
    NP_InitializeUPP *_NP_Initialize;
    NP_ShutdownUPP *_NP_Shutdown;

    NPPluginFuncs _pluginFuncs;
    NPNetscapeFuncs _nsFuncs;

    QPtrList< NSPluginInstance > _instances;
    QPtrList< NSPluginInstance > _trash;

    QCString _app;
};


class NSPluginViewer : public QObject, virtual public NSPluginViewerIface {
    Q_OBJECT
public:
    NSPluginViewer(QCString dcopId, QObject *parent, const char *name = 0);
    virtual ~NSPluginViewer();

    void shutdown();
    DCOPRef newClass(QString plugin);

private slots:
    void appUnregistered(const QCString &id);

private:
    QDict< NSPluginClass > _classes;
};


#endif
