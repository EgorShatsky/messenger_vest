#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QtWebSockets>
#include <QStackedWidget>
#include "userchat.h"

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(QWebSocket* socket, QString sessionId, userChat*uchat, QWidget *parent);
    ~Chat();

private slots:
    void on_sendButton_clicked();
    void on_addFileButton_clicked();
    void on_findButton_clicked();
    void on_backButton_clicked();
    void onTextMessageReceived(QString message);
    void onJsonReceived(QJsonObject json);

private:
    void initializeFonts();
    Ui::Chat *ui;
    QWebSocket *m_socket; // приватное поле для хранения объекта сокета
    QString m_sessionId;
    QStackedWidget *m_stackedWidget; // новый член класса
    userChat * m_userChat;
    QString chatId;
    QString telephone;

    QFont m_labelFont;
    QFont m_timeLabelFont;
    QFont m_namesurname;
};

#endif // CHAT_H
