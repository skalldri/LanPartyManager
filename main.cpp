#include <QtGui/QApplication>
#include "widget.h"

#if defined(WIN32) && defined(_DEBUG)
  #define new DEBUG_NEW
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Q_INIT_RESOURCE(resource);
    a.addLibraryPath("d:/LanPartyManager/release/plugins");
    a.setQuitOnLastWindowClosed(false);
    Widget * mainApp;
    mainApp = new Widget();
    return a.exec();
}

