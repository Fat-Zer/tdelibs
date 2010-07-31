#ifndef BUG_MAILER_H
#define BUG_MAILER_H "$Id$"

#include <tqobject.h>

class SMTP;

class BugMailer : public TQObject {
    Q_OBJECT
public:
    BugMailer(SMTP* s) : TQObject(0, "mailer"), sm(s) {}

public slots:
    void slotError(int);
    void slotSend();
private:
    SMTP *sm;
};

#endif
