// -*- c++ -*-
#ifndef _SPEED_H
#define _SPEED_H

#include <kio/global.h>
#include <kurl.h>

namespace TDEIO {
    class Job;
}

class SpeedTest : public TQObject {
    Q_OBJECT

public:
    SpeedTest(const KURL & url);

private slots:
    void entries( TDEIO::Job *, const TDEIO::UDSEntryList& );
    void finished( TDEIO::Job *job );

};

#endif
