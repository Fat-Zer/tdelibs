#include <kapplication.h>
#include <tqwidget.h>
#include <tqtimer.h>
#include <stdlib.h>
#include "kled.h"
#include "kledtest.h"



KLedTest::KLedTest(TQWidget* parent)
  : TQWidget(parent, 0),
    LedWidth(16),
    LedHeight(10),
    Grid(3),
    ledcolor(0),
    red(TQColor("red")),
    blue(TQColor("blue")),
    green(TQColor("green")),
    yellow(TQColor("yellow")),
    kled_round(true) // Switch HERE between rectangle and circular leds
{
  if (kled_round) {
    //KLed l(KLed::red, &qw);				// create lamp
    //KLed l(KLed::blue, &qw);				// create lamp
    l = new KLed(Qt::green, this);				// create lamp
    //KLed l(KLed::yellow, &qw);				// create lamp
    //KLed l(KLed::orange, &qw);				// create lamp


    l->resize(16,30);
    //l.setLook(KLed::flat);
    l->setShape(KLed::Circular);
    //l->setShape(KLed::Rectangular);

    //l->setLook(KLed::Flat);
    //l->setLook(KLed::Flat);
    //l->setLook(KLed::Flat);

    l->move(5,5);
    //    ktmp tmpobj(l);
    
    t_toggle.start(1000, false);
    t_color.start(3500, false);
    t_look.start(3500, false);
    TQObject::connect(&t_toggle, TQT_SIGNAL(timeout()), l, TQT_SLOT(toggle()));
    TQObject::connect(&t_color, TQT_SIGNAL(timeout()), this, TQT_SLOT(nextColor()));
    TQObject::connect(&t_look, TQT_SIGNAL(timeout()), this, TQT_SLOT(nextLook()));
    l->show();
    resize(240,140);
  }
  else {
    y=Grid; index=0;
    for( int tqshape=0; (int)tqshape<2; tqshape=(KLed::Shape)(tqshape+1)) {
      x=Grid;
      for( int look=0; (int)look<3; look=(KLed::Look)(look+1)) {
	for(state=KLed::Off; (int)state<2; state=(KLed::State)(state+1))
	    {
	      leds[index]=new KLed(Qt::yellow, state,
				   (KLed::Look)(look+1),
				   (KLed::Shape)(tqshape+1), this);
	      leds[index]->setGeometry(x, y, LedWidth, LedHeight);
	      ++index;
	      x+=Grid+LedWidth;
	    }
	}
      y+=Grid+LedHeight;
      }
    setFixedSize(x+Grid, y+Grid);
    connect(&timer, TQT_SIGNAL(timeout()), TQT_SLOT(timeout()));
    timer.start(500);
  }
}


KLedTest::~KLedTest()
{
  if (kled_round) {
    delete l;
  }
}


void 
KLedTest::nextColor() {

  ledcolor++;
  ledcolor%=4;

  switch(ledcolor) {
  default:
  case 0: l->setColor(green); break;
  case 1: l->setColor(blue); break;
  case 2: l->setColor(red); break;
  case 3: l->setColor(yellow); break;
  }
}


void 
KLedTest::nextLook() { 
  register int tmp;
  if (kled_round) {
    tmp = (static_cast<int>(ledlook) +1 ) % 3 ;
  }
  else {  
    tmp = (static_cast<int>(ledlook) + 1) % 3;
  }
  ledlook = static_cast<KLed::Look>(tmp);
  l->setLook(ledlook);
  //qDebug("painting look %i", ledlook);
  //l->tqrepaint(); 
}


void
KLedTest::timeout() 
{
  const int NoOfLeds=sizeof(leds)/sizeof(leds[0]);
  int count;
  // -----
  for(count=0; count<NoOfLeds; ++count)
    {
      if(leds[count]->state()==KLed::Off)
	{
	  leds[count]->setState(KLed::On);
	} else {
	  leds[count]->setState(KLed::Off);
	}
    }
}


/*#include <stdio.h>*/

int main( int argc, char **argv )
{
    KApplication a( argc, argv, "KLedTest" );
    KLedTest widget;
    // -----
    /*
    if (argc>1) { // look out for round or circular led command
      if (strncmp(argv[1],"-c",2)) {	
	// paint circular
	printf("painting circular led\n");
	widget.kled_round = true;
      }
      else if (strncmp(argv[1],"-r",2)) {
	// paint rectangle
	printf("painting rectangular led\n");
	widget.kled_round = false;
      }
    }
    */
    a.setMainWidget(&widget);
    widget.show();
    return a.exec();				// go
}

#include "kledtest.moc"

