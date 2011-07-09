#ifndef _EDITOR_CHOOSER_H_
#define  _EDITOR_CHOOSER_H_

#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>

#include <tqwidget.h>

class KConfig;
class TQString;

namespace KTextEditor
{

class KTEXTEDITOR_EXPORT EditorChooser: public TQWidget
{                    
  friend class PrivateEditorChooser;

  Q_OBJECT

  public:
    EditorChooser(TQWidget *parent=0,const char *name=0);
    virtual ~EditorChooser();
    
   /* void writeSysDefault();*/

    void readAppSetting(const TQString& postfix=TQString::null);
    void writeAppSetting(const TQString& postfix=TQString::null);

    static KTextEditor::Document *createDocument(TQObject* parent=0,const char *name=0,const TQString& postfix=TQString::null, bool fallBackToKatePart=true);
    static KTextEditor::Editor *createEditor(TQWidget *parentWidget,TQObject *parent,const char* widgetName=0,const char* name=0,const TQString& postfix=TQString::null,bool fallBackToKatePart=true);
  private:
    class PrivateEditorChooser *d;
};

/*
class EditorChooserBackEnd: public ComponentChooserPlugin {

Q_OBJECT
public:
	EditorChooserBackEnd(TQObject *parent=0, const char *name=0);
	virtual ~EditorChooserBackEnd();

	virtual TQWidget *widget(TQWidget *);
	virtual const TQStringList &choices();
	virtual void saveSettings();

	void readAppSetting(KConfig *cfg,const TQString& postfix);
	void writeAppSetting(KConfig *cfg,const TQString& postfix);

public slots:
	virtual void madeChoice(int pos,const TQString &choice);

};
*/

}
#endif
