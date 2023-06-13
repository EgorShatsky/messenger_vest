#include "server.h"
#include "connection.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if(!createConnection()) {
           QMessageBox::critical(nullptr, "Ошибка подключения", "Не удалось подключиться к базе данных!");
           return -1;
       }
    Server w(8080); // передаем порт
    w.show();
    return a.exec();
}
