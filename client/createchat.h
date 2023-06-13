#ifndef CREATECHAT_H
#define CREATECHAT_H

#include <QWidget>
#include <QtWebSockets>
#include <QStackedWidget>

namespace Ui {
class createChat;
}

class createChat : public QWidget
{
    Q_OBJECT

public:
    explicit createChat(QWebSocket* socket, QString sessionId, QWidget *parent);
    ~createChat();

private slots:
    void on_backButton_clicked();
    void on_findButton_clicked();
    void onTextMessageReceived(QString message);

private:
    Ui::createChat *ui;
    QWebSocket *m_socket; // приватное поле для хранения объекта сокета
    QString m_sessionId;
    QStackedWidget *m_stackedWidget; // новый член класса
};

#endif // CREATECHAT_H
