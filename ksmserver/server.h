/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SERVER_H
#define SERVER_H

// needed to avoid clash with INT8 defined in X11/Xmd.h on solaris
#define QT_CLEAN_NAMESPACE 1
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qsocketnotifier.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qcstring.h>
#include <qdict.h>
#include <qptrqueue.h>
#include <qptrdict.h>
#include <kapplication.h>
#include <qtimer.h>
#include <dcopobject.h>

#include "server2.h"

#include "KSMServerInterface.h"

#define SESSION_PREVIOUS_LOGOUT "saved at previous logout"
#define SESSION_BY_USER  "saved by user"

typedef QValueList<QCString> QCStringList;
class KSMListener;
class KSMConnection;
class KSMClient;

enum SMType { SM_ERROR, SM_WMCOMMAND, SM_WMSAVEYOURSELF };
struct SMData
    {
    SMType type;
    QStringList wmCommand;
    QString wmClientMachine;
    QString wmclass1, wmclass2;
    };
typedef QMap<WId,SMData> WindowMap;

class KSMServer : public QObject, public KSMServerInterface
{
Q_OBJECT
K_DCOP
k_dcop:
    void notifySlot(QString,QString,QString,QString,QString,int,int,int,int);
    void logoutSoundFinished(int,int);
    void autoStart0Done();
    void autoStart1Done();
    void autoStart2Done();
    void kcmPhase1Done();
    void kcmPhase2Done();
public:
    KSMServer( const QString& windowManager, bool only_local );
    ~KSMServer();

    static KSMServer* self();

    void* watchConnection( IceConn iceConn );
    void removeConnection( KSMConnection* conn );

    KSMClient* newClient( SmsConn );
    void  deleteClient( KSMClient* client );

    // callbacks
    void saveYourselfDone( KSMClient* client, bool success );
    void interactRequest( KSMClient* client, int dialogType );
    void interactDone( KSMClient* client, bool cancelShutdown );
    void phase2Request( KSMClient* client );

    // error handling
    void ioError( IceConn iceConn );

    // notification
    void clientSetProgram( KSMClient* client );
    void clientRegistered( const char* previousId );

    // public API
    void restoreSession( QString sessionName );
    void startDefaultSession();
    void shutdown( KApplication::ShutdownConfirm confirm,
                   KApplication::ShutdownType sdtype,
                   KApplication::ShutdownMode sdmode );

    virtual void suspendStartup( QCString app );
    virtual void resumeStartup( QCString app );

public slots:
    void cleanUp();

private slots:
    void newConnection( int socket );
    void processData( int socket );
    void restoreSessionInternal();
    void restoreSessionDoneInternal();

    void protectionTimeout();
    void timeoutQuit();
    void timeoutWMQuit();
    void knotifyTimeout();
    void kcmPhase1Timeout();
    void kcmPhase2Timeout();
    void pendingShutdownTimeout();

    void autoStart0();
    void autoStart1();
    void autoStart2();
    void tryRestoreNext();
    void startupSuspendTimeout();

private:
    void handlePendingInteractions();
    void completeShutdownOrCheckpoint();
    void startKilling();
    void performStandardKilling();
    void completeKilling();
    void killWM();
    void completeKillingWM();
    void cancelShutdown( KSMClient* c );
    void killingCompleted();

    void discardSession();
    void storeSession();

    void startProtection();
    void endProtection();

    void startApplication( QStringList command,
        const QString& clientMachine = QString::null,
        const QString& userId = QString::null );
    void executeCommand( const QStringList& command );
    
    bool isWM( const KSMClient* client ) const;
    bool isWM( const QString& program ) const;
    bool defaultSession() const; // empty session
    void setupXIOErrorHandler();

    void performLegacySessionSave();
    void storeLegacySession( KConfig* config );
    void restoreLegacySession( KConfig* config );
    void restoreLegacySessionInternal( KConfig* config, char sep = ',' );
    QStringList windowWmCommand(WId w);
    QString windowWmClientMachine(WId w);
    WId windowWmClientLeader(WId w);
    QCString windowSessionId(WId w, WId leader);
    
    bool checkStartupSuspend();
    void finishStartup();
    void resumeStartupInternal();

    // public dcop interface
    void logout( int, int, int );
    QStringList sessionList();
    QString currentSession();
    void saveCurrentSession();
    void saveCurrentSessionAs( QString );

 private:
    QPtrList<KSMListener> listener;
    QPtrList<KSMClient> clients;

    enum State
        {
        Idle,
        LaunchingWM, AutoStart0, KcmInitPhase1, AutoStart1, Restoring, FinishingStartup, // startup
        Shutdown, Checkpoint, Killing, KillingWM, WaitingForKNotify // shutdown
        };
    State state;
    bool dialogActive;
    bool saveSession;
    int wmPhase1WaitingCount;
    int saveType;
    QMap< QCString, int > startupSuspendCount;

    KApplication::ShutdownType shutdownType;
    KApplication::ShutdownMode shutdownMode;
    QString bootOption;

    bool clean;
    KSMClient* clientInteracting;
    QString wm;
    QString sessionGroup;
    QString sessionName;
    QCString launcher;
    QTimer protectionTimer;
    QTimer restoreTimer;
    QString xonCommand;
    int logoutSoundEvent;
    QTimer knotifyTimeoutTimer;
    QTimer startupSuspendTimeoutTimer;
    bool waitAutoStart2;
    bool waitKcmInit2;
    QTimer pendingShutdown;
    KApplication::ShutdownConfirm pendingShutdown_confirm;
    KApplication::ShutdownType pendingShutdown_sdtype;
    KApplication::ShutdownMode pendingShutdown_sdmode;

    // ksplash interface
    void upAndRunning( const QString& msg );
    void publishProgress( int progress, bool max  = false  );

    // sequential startup
    int appsToStart;
    int lastAppStarted;
    QString lastIdStarted;
    
    QStringList excludeApps;

    WindowMap legacyWindows;
};

#endif
