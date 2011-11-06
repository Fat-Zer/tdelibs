// $Id$

#ifndef KLOCALETEST_H
#define KLOCALETEST_H

#include <tqwidget.h>

class TQLabel;

/** test: a small test program for KLocale
    */
class Test : public TQWidget
{
  Q_OBJECT
  
public:
  /**@name methods */
  //@{
  /** Constructor  
	*/
  Test( TQWidget *parent=0, const char *name=0 );
  /** Destructor
	*/
  ~Test();

private:
  TQString showLocale(TQString cat);
  void createFields();

  TQLabel *label;
};
#endif // TEST_H
