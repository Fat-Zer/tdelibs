#ifndef testwindow_h
#define testwindow_h

#include <tqpopupmenu.h>
#include <tqtimer.h>
#include <tqprogressbar.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kmainwindow.h>

class QMultiLineEdit;
class KToolBarRadioGroup;
class KHelpMenu;

class testWindow  : public KMainWindow
{
    Q_OBJECT

public:
    testWindow (TQWidget *parent=0, const char *name=0);
    ~testWindow ();
    
public slots:
    void beFixed();
    void beYFixed();

    void slotNew();
    void slotPrint();
    void slotReturn();
    void slotSave();
    void slotList(const TQString &str);
    void slotOpen();
    void slotCompletion();
    void slotCompletionsMenu(int id);
    void slotHide2 ();
    void slotInsertClock();
    void slotHide1 ();
    void slotLined ();
    void slotImportant ();
    void slotExit();
    void slotFrame();
    void slotListCompletion();
    void slotMessage(int, bool);
    void slotToggle(bool);
    void slotClearCombo();
    void slotGoGoGoo();
    void slotInsertListInCombo ();
    void slotMakeItem3Current ();
    void slotToggled(int);
protected:
    KMenuBar *menuBar;
    TQPopupMenu *fileMenu;
    TQPopupMenu *itemsMenu;
    TQPopupMenu *completions;
    TQPopupMenu *toolBarMenu;
    KStatusBar *statusBar;
    KHelpMenu *helpMenu;
    KToolBar *tb;
    KToolBar *tb1;
    bool lineL;
    bool exitB;
    bool greenF;
    bool ena;
    TQMultiLineEdit *widget;
    TQTimer *timer;
    TQProgressBar *pr;
    KToolBarRadioGroup *rg;
};
#endif

