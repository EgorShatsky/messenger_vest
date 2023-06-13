#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QWidget>
#include <QtWebSockets>
#include <QStackedWidget>

namespace Ui {
class Registration;
}

class Registration : public QWidget
{
    Q_OBJECT

public:
    explicit Registration(QWebSocket* socket, QString sessionId, QWidget *parent);
    ~Registration();

private slots:
    void on_createAccountButton_clicked();
    void onTextMessageReceived(QString message);

private:
    Ui::Registration *ui;
    QWebSocket *m_socket; // приватное поле для хранения объекта сокета
    QString m_sessionId;
    QStackedWidget *m_stackedWidget; // новый член класса
};

#endif // REGISTRATION_H
