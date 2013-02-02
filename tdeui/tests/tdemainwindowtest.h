#ifndef KMAINWINDOWTEST_H
#define KMAINWINDOWTEST_H

#include <tdemainwindow.h>

class MainWindow : public TDEMainWindow
{
    Q_OBJECT
public:
    MainWindow();

private slots:
    void showMessage();
};

#endif // KMAINWINDOWTEST_H
/* vim: et sw=4 ts=4
 */
