#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QSG_MAX_SURFACE_COUNT", "2000");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
