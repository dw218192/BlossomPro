#include <QApplication>
#include "WidgetTestWindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    WidgetTestWindow w;
    w.show();
    return a.exec();
}