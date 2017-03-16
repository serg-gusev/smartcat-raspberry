#include <QApplication>
#include "application.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Application b;
    b.runConnect();

    return a.exec();
}
