
#ifndef __parts_h__
#define __parts_h__

#include <kparts/part.h>

class TQMultiLineEdit;
namespace KParts {
class GUIActivateEvent;
};

class Part1 : public KParts::ReadOnlyPart
{
  Q_OBJECT
public:
  Part1( TQObject *parent, TQWidget * tqparentWidget );
  virtual ~Part1();

protected:
  virtual bool openFile();

protected:
  TQMultiLineEdit * m_edit;
  KInstance *m_instance;
};

class Part2 : public KParts::Part
{
  Q_OBJECT
public:
  Part2( TQObject *parent, TQWidget * tqparentWidget );
  virtual ~Part2();

protected:
  // This is not mandatory - only if you care about setting the
  // part caption when the part is used in a multi-part environment
  // (i.e. in a part manager)
  // There is a default impl for ReadOnlyPart...
  virtual void guiActivateEvent( KParts::GUIActivateEvent * );
  KInstance *m_instance;
};

#endif
