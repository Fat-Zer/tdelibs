/*****************************************************************

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
 
******************************************************************
*/

#include <testdcop.h>
#include <tqtimer.h>

#include <assert.h>

DCOPClientTransaction *countDownAction = 0;
int countDownCount = 0;

DCOPClientTransaction *countDownAction2 = 0;
int countDownCount2 = 0;
DCOPClient *client = 0;

bool MyDCOPObject::process(const TQCString &fun, const TQByteArray &data,
			   TQCString& replyType, TQByteArray &replyData)
{
  qDebug("in MyDCOPObject::process, fun = %s", fun.data());
  
  // note "fun" is normlized here (i.e. whitespace clean)
  if (fun == "aFunction(TQString,int)") {
    TQDataStream args(data, IO_ReadOnly);
    TQString arg1;
    int arg2;
    args >> arg1 >> arg2;
    function(arg1, arg2);
    replyType = "void";
    return true;
  }
  if (fun == "canLaunchRockets(TQRect)") {
    TQDataStream args(data, IO_ReadOnly);
    TQRect arg1;
    args >> arg1;

    printf("Rect x = %d, y = %d, w = %d, h = %d\n", arg1.x(), arg1.y(), arg1.width(), arg1.height());

    replyType = "TQRect";
    TQDataStream reply( replyData, IO_WriteOnly );
    TQRect r(10,20,100,200);
    reply << r;
    return true;
  }
  if (fun == "isAliveSlot(int)") {
    
    qDebug("isAliveSlot(int)");
    bool connectResult = client->disconnectDCOPSignal("", objId(), "", objId(), "" );
    qDebug("disconnectDCOPSignal returns %s", connectResult ? "true" : "false");
    return true;
  }
  if (fun == "countDown()") {
qDebug("countDown() countDownAction = %p", countDownAction);
    if (countDownAction2)
    {
       replyType = "TQString";
       TQDataStream reply( replyData, IO_WriteOnly );
       reply << TQString("Hey");
       return true;
    }

    if (countDownAction == 0)
    {
       countDownCount = 10;       
       countDownAction = client->beginTransaction();
       TQTimer::singleShot(1000, this, TQT_SLOT(slotTimeout()));
    }
    else
    {
       countDownCount2 = 10;       
       countDownAction2 = client->beginTransaction();
       TQTimer::singleShot(1000, this, TQT_SLOT(slotTimeout2()));
    }
    return true;
  }

  return DCOPObject::process(fun, data, replyType, replyData);
}

void MyDCOPObject::slotTimeout()
{
  qDebug("MyDCOPObject::slotTimeout() %d", countDownCount);
  countDownCount--;
  if (countDownCount == 0)
  {
     TQCString replyType = "TQString";
     TQByteArray replyData;
     TQDataStream reply( replyData, IO_WriteOnly );
     reply << TQString("Hello World");
     client->endTransaction(countDownAction, replyType, replyData);
     countDownAction = 0;
  }
  else
  {
     TQTimer::singleShot(1000, this, TQT_SLOT(slotTimeout()));
  }
}

void MyDCOPObject::slotTimeout2()
{
  qDebug("MyDCOPObject::slotTimeout2() %d", countDownCount2);
  countDownCount2--;
  if (countDownCount2 == 0)
  {
     TQCString replyType = "TQString";
     TQByteArray replyData;
     TQDataStream reply( replyData, IO_WriteOnly );
     reply << TQString("Hello World");
     client->endTransaction(countDownAction2, replyType, replyData);
     countDownAction2 = 0;
  }
  else
  {
     TQTimer::singleShot(1000, this, TQT_SLOT(slotTimeout2()));
  }
}

QCStringList MyDCOPObject::functions()
{
   QCStringList result = DCOPObject::functions();
   result << "TQRect canLaunchRockets(TQRect)";
   return result;
}

TestObject::TestObject(const TQCString& app)
 :  m_app(app)
{
   TQTimer::singleShot(2500, this, TQT_SLOT(slotTimeout()));
}

void TestObject::slotTimeout()
{
   TQCString replyType;
   TQByteArray data, reply;
   qWarning("#3 Calling countDown");

   if (!client->call(m_app, "object1", "countDown()", data, replyType, reply))
      qDebug("#3 I couldn't call countDown");
   else
      qDebug("#3 countDown() return type was '%s'", replyType.data() ); 
   
}

void TestObject::slotCallBack(int callId, const TQCString &replyType, const TQByteArray &replyData)
{
   qWarning("Call Back! callId = %d", callId);
   qWarning("Type = %s", replyType.data());
   
   TQDataStream args(replyData, IO_ReadOnly);
   TQString arg1;
   args >> arg1;
   
   qWarning("Value = %s", arg1.latin1());
}

#ifdef Q_OS_WIN
# define main kdemain
#endif

int main(int argc, char **argv)
{
  TQApplication app(argc, argv, "testdcop");

  TQCString replyType;
  TQByteArray data, reply;
  client = new DCOPClient();

  if (argc == 2)
  {
      TQCString appId = argv[1];
      TestObject obj(appId);
      qWarning("#1 Calling countDown");
      int result = client->callAsync(appId, "object1", "countDown()", data, &obj, TQT_SLOT(slotCallBack(int, const TQCString&, const TQByteArray&)));
      qDebug("#1 countDown() call id = %d", result);
      qWarning("#2 Calling countDown");
      result = client->callAsync(appId, "object1", "countDown()", data, &obj, TQT_SLOT(slotCallBack(int, const TQCString&, const TQByteArray&)));
      qDebug("#2 countDown() call id = %d", result);
      app.exec();

      return 0;
  }

//  client->attach(); // attach to the server, now we can use DCOP service

  client->registerAs( app.name(), false ); // register at the server, now others can call us.
  qDebug("I registered as '%s'", client->appId().data() );

  if ( client->isApplicationRegistered( app.name() ) )
      qDebug("indeed, we are registered!");

  TQDataStream dataStream( data, IO_WriteOnly );
  dataStream << (int) 43;
  client->emitDCOPSignal("alive(int,TQCString)", data);

  MyDCOPObject *obj1 = new MyDCOPObject("object1");

  bool connectResult = client->connectDCOPSignal("", "alive(int , TQCString)", "object1", "isAliveSlot(int)", false);
  qDebug("connectDCOPSignal returns %s", connectResult ? "true" : "false");

  TQDataStream ds(data, IO_WriteOnly);
  ds << TQString("fourty-two") << 42;
  if (!client->call(app.name(), "object1", "aFunction(TQString,int)", data, replyType, reply)) {
    qDebug("I couldn't call myself");
    assert( 0 );
  }
  else {
    qDebug("return type was '%s'", replyType.data() ); 
    assert( replyType == "void" );
  }

  client->send(app.name(), "object1", "aFunction(TQString,int)", data );

  int n = client->registeredApplications().count();
  qDebug("number of attached applications = %d", n );

  TQObject::connect( client, TQT_SIGNAL( applicationRegistered( const TQCString&)),
                    obj1, TQT_SLOT( registered( const TQCString& )));

  TQObject::connect( client, TQT_SIGNAL( applicationRemoved( const TQCString&)),
                    obj1, TQT_SLOT( unregistered( const TQCString& )));

  // Enable the above signals
  client->setNotifications( true );

  TQCString foundApp;
  TQCString foundObj;

  // Find a object called "object1" in any application that
  // meets the criteria "canLaunchRockets()"
//  bool boolResult = client->tqfindObject( "", "object1", "canLaunchRockets()", data, foundApp, foundObj);
//  qDebug("tqfindObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
//	foundApp.data(), foundObj.data());

  // Find an application that matches with "konqueror*"
  bool boolResult = client->tqfindObject( "konqueror*", "", "", data, foundApp, foundObj);
  qDebug("tqfindObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
	foundApp.data(), foundObj.data());

  // Find an object called "object1" in any application.
  boolResult = client->tqfindObject( "", "ksycoca", "", data, foundApp, foundObj);
  qDebug("tqfindObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
	foundApp.data(), foundObj.data());

  // Find ourselves in any application.
  boolResult = client->tqfindObject( "testdcop", "ksycoca", "", data, foundApp, foundObj);
  qDebug("tqfindObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
	foundApp.data(), foundObj.data());

  DCOPClient *client2 = new DCOPClient();
  client2->registerAs(app.name(), false);
  qDebug("I2 registered as '%s'", client2->appId().data() );

qDebug("Sending to object1");
  client2->send(app.name(), "object1", "aFunction(TQString,int)", data );

qDebug("Calling object1");
  if (!client2->call(app.name(), "object1", "aFunction(TQString,int)", data, replyType, reply))
    qDebug("I couldn't call myself");
  else
      qDebug("return type was '%s'", replyType.data() ); 

qDebug("Calling countDown() in object1");
  if (!client2->call(app.name(), "object1", "countDown()", data, replyType, reply))
    qDebug("I couldn't call myself");
  else
      qDebug("return type was '%s'", replyType.data() ); 

  // Find ourselves in any application.
  boolResult = client2->tqfindObject( "testdcop", "object1", "", data, foundApp, foundObj);
  qDebug("tqfindObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
	foundApp.data(), foundObj.data());

  client->detach();
  return 0;
}

#include "testdcop.moc"
