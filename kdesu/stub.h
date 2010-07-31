/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __Stub_h_Included__
#define __Stub_h_Included__

#include <tqcstring.h>
#include <tqvaluelist.h>

#include "process.h"
#include "kcookie.h"

#include <kdelibs_export.h>

typedef TQValueList<TQCString> QCStringList;

/**
 * Chat with kdesu_stub.
 *
 * StubProcess extends PtyProcess with functionality to chat with kdesu_stub.
 */

class KDESU_EXPORT StubProcess: public PtyProcess
{
public:
    StubProcess();
    ~StubProcess();

    /**
     * Specify dcop transport
     */
    void setDcopTransport(const TQCString &dcopTransport) 
       { m_pCookie->setDcopTransport(dcopTransport); }

    /**
     * Set the command.
     */
    void setCommand(const TQCString &command) { m_Command = command; }

    /**
     * Set the target user.
     */
    void setUser(const TQCString &user) { m_User = user; }

    /**
     * Set to "X only mode": Sycoca is not built and kdeinit is not launched.
     */
    void setXOnly(bool xonly) { m_bXOnly = xonly; }

    /**
     * Enable DCOP forwarding.
     */
    void setDCOPForwarding(bool dcopForwarding) { m_bDCOPForwarding = dcopForwarding; }

    /**
     * Set the priority of the process. The priority value must be between 0
     * and 100, 0 being the lowest priority. This value is mapped to the
     * scheduler and system dependant priority range of the OS.
     */
    void setPriority(int prio);

    /**
     * Different schedulers. SchedNormal is the normal Unix timesharing
     * scheduler, while SchedRealtime is a POSIX.1b realtime scheduler.
     */
    enum Scheduler { SchedNormal, SchedRealtime };

    /**
     * Set the scheduler type. 
     */
    void setScheduler(int sched) { m_Scheduler = sched; }

protected:

    /**
     * Exchange all parameters with kdesu_stub.
     */
    int ConverseStub(int check);

    /**
     * Notify the taskbar that a new application has been started.
     * @obsolete
     */
    // KDE4 remove
    void notifyTaskbar(const TQString &suffix);

    /** 
     * This virtual function can be overloaded when special behavior is
     * desired. By default, it returns the value returned by KCookie.
     */
    virtual TQCString display() { return m_pCookie->display(); }
#ifdef Q_WS_X11
    /**
     * See display.
     */
    virtual TQCString displayAuth() { return m_pCookie->displayAuth(); }
#endif
    /**
     * See display.
     */
    virtual TQCString dcopServer() { return m_pCookie->dcopServer(); }
    /**
     * See display.
     */
    virtual TQCString dcopAuth() { return m_pCookie->dcopAuth(); }
    /**
     * See display.
     */
    virtual TQCString iceAuth() { return m_pCookie->iceAuth(); }

    bool m_bXOnly;
    bool m_bDCOPForwarding;
    int m_Priority;
    int  m_Scheduler;
    TQCString m_dcopTransport;
    TQCString m_Command;
    TQCString m_User;
    KCookie *m_pCookie;
    
private:
    TQCString commaSeparatedList(QCStringList);

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class StubProcessPrivate;
    StubProcessPrivate *d;
};

#endif // __Stub_h_Included__
