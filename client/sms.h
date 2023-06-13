#ifndef SMS_H
#define SMS_H

#include <QWidget>
#include <QtWebSockets>
#include <QStackedWidget>

namespace Ui {
class Sms;
}

class Sms : public QWidget
{
    Q_OBJECT

public:
    explicit Sms(QWebSocket* socket, QString sessionId, QWidget *parent);
    ~Sms();

private slots:
    void on_acceptButton_clicked();
    void on_continueButton_clicked();
    void onTextMessageReceived(QString message);

private:
    Ui::Sms *ui;
    QWebSocket *m_socket; // приватное поле для хранения объекта сокета
    QString m_sessionId;
    QStackedWidget *m_stackedWidget; // новый член класса
};

#endif // SMS_H
