#ifndef TDEMAINWINDOWTEST_H
#define TDEMAINWINDOWTEST_H

#include <tdemainwindow.h>

class MainWindow : public TDEMainWindow
{
    Q_OBJECT
public:
    MainWindow();

private slots:
    void showMessage();
};

#endif // TDEMAINWINDOWTEST_H
/* vim: et sw=4 ts=4
 */
