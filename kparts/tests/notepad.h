
#ifndef __notepad_h__
#define __notepad_h__

#include <kparts/genericfactory.h>
#include <kparts/part.h>

class KAboutData;
class TQMultiLineEdit;

/**
 * Who said writing a part should be complex ? :-)
 * Here is a very simple kedit-like part
 * @internal
 */
class NotepadPart : public KParts::ReadWritePart
{
  Q_OBJECT
public:
  NotepadPart( TQWidget*, const char* widgetName,
               TQObject* parent, const char* name,
               const TQStringList& args = TQStringList() );
  virtual ~NotepadPart();

  virtual void setReadWrite( bool rw );

  static KAboutData* createAboutData();

protected:
  virtual bool openFile();
  virtual bool saveFile();

protected slots:
  void slotSearchReplace();

protected:
  TQMultiLineEdit * m_edit;
};

typedef KParts::GenericFactory<NotepadPart> NotepadFactory;

#endif
