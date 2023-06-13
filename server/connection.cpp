#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

//Создание соединения с базой данных
bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPort(5432); // порт по умолчанию для PostgreSQL
    db.setDatabaseName("messenger");
    db.setUserName("postgres"); //postgres messenger
    db.setPassword("2468027");

    if (db.open())
    {
        qDebug() << "Соединение с базой данных установлено!";
    }

    else
    {
        qDebug() << "Ошибка подключения к базе данных: " << db.lastError().text();
    }

    return true;
}

