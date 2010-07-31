#ifndef _ITEMCONTAINERTEST_H
#define _ITEMCONTAINERTEST_H

#include <tqwidget.h>

class KIconView;
class KListView;
class KListBox;
class QButtonGroup;
class QLabel;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( TQWidget *parent=0, const char *name=0 );

    enum ViewID { IconView, ListView, ListBox };
    enum ModeID { NoSelection, Single, Multi, Extended };

public slots:
    //void slotSwitchView( int id );
    void slotSwitchMode( int id ); 

    void slotIconViewExec( TQIconViewItem* item );
    void slotListViewExec( TQListViewItem* item ); 
    void slotListBoxExec( TQListBoxItem* item );
    void slotToggleSingleColumn( bool b );

    void slotClicked( TQIconViewItem* ) { qDebug("CLICK");}
    void slotDoubleClicked( TQIconViewItem* ) { qDebug("DOUBLE CLICK");}
protected:
    KIconView* m_pIconView;
    KListView* m_pListView;
    KListBox* m_pListBox;

    TQButtonGroup* m_pbgView;
    TQButtonGroup* m_pbgMode;
    TQLabel* m_plblWidget;
    TQLabel* m_plblSignal;
    TQLabel* m_plblItem;
};

#endif
