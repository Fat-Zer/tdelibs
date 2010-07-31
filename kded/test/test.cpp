#include "test.h"

class TestObject : public KShared
{
public:
   TestObject(const TQCString &_app) : app(_app)
     { qWarning("Creating TestObject belonging to '%s'", app.data()); }
   ~TestObject() 
     { qWarning("Destructing TestObject belonging to '%s'", app.data()); }
protected:
   TQCString app;
};

TestModule::TestModule(const TQCString &obj) : KDEDModule(obj)
{
  // Do stuff here
  setIdleTimeout(15); // 15 seconds idle timeout.
}

TQString TestModule::world()
{
  return "Hello World!";  
}

void TestModule::idle()
{
   qWarning("TestModule is idle.");
}

void TestModule::registerMe(const TQCString &app)
{
   insert(app, "test", new TestObject(app));
   // When 'app' unregisters with DCOP, the TestObject will get deleted.
}

extern "C" { 
  KDE_EXPORT KDEDModule *create_test(const TQCString &obj)
  {
     return new TestModule(obj);
  }
};

#include "test.moc"
