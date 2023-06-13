#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <QWidget>
#include <QWebSocket>
#include <QStackedWidget>

namespace Ui {
class Authentication;
}

class Authentication : public QWidget
{
    Q_OBJECT

public:
    explicit Authentication(QWebSocket* socket, QString sessionId, QWidget *parent);
    ~Authentication();

signals:
    void jsonReceived(const QJsonObject& obj);

private slots:
    void on_entranceButton_clicked();
    void onTextMessageReceived(QString message);

private:
    Ui::Authentication *ui;
    QWebSocket *m_socket; // приватное поле для хранения объекта сокета
    QString m_sessionId;
    QStackedWidget *m_stackedWidget; // новый член класса
};

#endif // AUTHENTICATION_H
