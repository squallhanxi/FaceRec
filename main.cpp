#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
//high DPI
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
