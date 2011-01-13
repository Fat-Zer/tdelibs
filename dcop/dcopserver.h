/*
Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef DCOPSERVER_H
#define DCOPSERVER_H "$Id$"

#include <tqobject.h>

#include <tqstring.h>
#include <tqsocketnotifier.h>
#include <tqptrlist.h>
#include <tqvaluelist.h>
#include <tqcstring.h>
#include <tqdict.h>
#include <tqptrdict.h>
#include <tqintdict.h>
#include <tqapplication.h>

#define INT32 QINT32
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#endif
#include <KDE-ICE/ICElib.h>
extern "C" {
#include <KDE-ICE/ICEutil.h>
#include <KDE-ICE/ICEmsg.h>
#include <KDE-ICE/ICEproto.h>
}

class DCOPConnection;
class DCOPListener;
class DCOPSignalConnectionList;
class DCOPSignals;
class TQTimer;

// If you enable the following define DCOP will create
// $HOME/.dcop.log file which will list all signals passing
// through it.
//#define DCOP_LOG
#ifdef DCOP_LOG
class TQTextStream;
class TQFile;
#endif

typedef TQValueList<TQCString> QCStringList;

/**
 * @internal
 */
class DCOPConnection : public TQSocketNotifier
{
public:
    DCOPConnection( IceConn conn );
    ~DCOPConnection();

    DCOPSignalConnectionList *signalConnectionList();

    // Add the data from offset @p start in @p _data to the output
    // buffer and schedule it for later transmission.
    void waitForOutputReady(const TQByteArray &_data, int start);

    // Called from DCOPServer::slotOutputReady()
    // Flush the output buffer.
    void slotOutputReady();

    TQCString appId;
    TQCString plainAppId;
    IceConn iceConn;
    int notifyRegister;
    /**
     * When client A has called client B then for the duration of the call:
     * A->waitingOnReply contains B
     *   and either
     * B->waitingForReply contains A
     *   or
     * B->waitingForDelayedReply contains A
     *
     * This allows us to do proper bookkeeping in case client A, client B
     * or both unregister during the call.
     */
    TQPtrList <_IceConn> waitingOnReply;
    TQPtrList <_IceConn> waitingForReply;
    TQPtrList <_IceConn> waitingForDelayedReply;
    DCOPSignalConnectionList *_signalConnectionList;
    bool daemon;
    bool outputBlocked;
    TQValueList <TQByteArray> outputBuffer;
    unsigned long outputBufferStart;
    TQSocketNotifier *outputBufferNotifier;
};


/**
 * @internal
 */
class DCOPServer : public TQObject
{
    Q_OBJECT
public:
    DCOPServer(bool _suicide);
    ~DCOPServer();

    void* watchConnection( IceConn iceConn );
    void removeConnection( void* data );
    void processMessage( IceConn iceConn, int opcode, unsigned long length, Bool swap);
    void ioError( IceConn iceConn );

    bool receive(const TQCString &app, const TQCString &obj,
                 const TQCString &fun, const TQByteArray& data,
                 TQCString& replyType, TQByteArray &replyData, IceConn iceConn);

    DCOPConnection *findApp(const TQCString &appId);
    DCOPConnection *findConn(IceConn iceConn)
       { return clients.tqfind(iceConn); }

    void sendMessage(DCOPConnection *conn, const TQCString &sApp,
                     const TQCString &rApp, const TQCString &rObj,
                     const TQCString &rFun, const TQByteArray &data);

private slots:
    void newClient( int socket );
    void processData( int socket );
    void slotTerminate();
    void slotSuicide();
    void slotShutdown();
    void slotExit();
    void slotCleanDeadConnections();
    void slotOutputReady(int socket );

#ifdef Q_OS_WIN
public:
    static BOOL WINAPI dcopServerConsoleProc(DWORD dwCtrlType);
private:
    static DWORD WINAPI TerminatorThread(void * pParam);
#endif
private:
    void broadcastApplicationRegistration( DCOPConnection* conn, const TQCString type,
        const TQCString& data );
    bool suicide;
    bool shutdown;
    int majorOpcode;
    int currentClientNumber;
    CARD32 serverKey;
    DCOPSignals *dcopSignals;
    TQTimer *m_timer;
    TQTimer *m_deadConnectionTimer;
    TQPtrList<DCOPListener> listener;
    TQAsciiDict<DCOPConnection> appIds; // index on app id
    TQPtrDict<DCOPConnection> clients; // index on iceConn
    TQIntDict<DCOPConnection> fd_clients; // index on fd
    TQPtrList<_IceConn> deadConnections;

#ifdef Q_OS_WIN
    HANDLE m_evTerminate;
    HANDLE m_hTerminateThread;
    DWORD m_dwTerminateThreadId;
#endif

#ifdef DCOP_LOG
    TQTextStream *m_stream;
    TQFile *m_logger;
#endif
};

extern DCOPServer* the_server;

#endif
