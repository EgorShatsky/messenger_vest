#include "client.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(":/img/img/icon.ico"));
    Client w;
    w.setWindowTitle("Весть");
    w.show();
    return a.exec();
}
