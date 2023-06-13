#ifndef USERCHAT_H
#define USERCHAT_H

#include <QWidget>
#include <QtWebSockets>
#include <QStackedWidget>
#include <QListWidget>
#include "authentication.h"


namespace Ui {
class userChat;
}

class userChat : public QWidget
{
    Q_OBJECT

public:
    explicit userChat(QWebSocket* socket, QString sessionId, Authentication* auth, QWidget *parent);
    ~userChat();

signals:
    void jsonReceived(const QJsonObject& obj);

private slots:
    void on_newchatButton_clicked();
    void on_settingsButton_clicked();
    void onTextMessageReceived(QString message);
    void onJsonReceived(QJsonObject json);
    void on_listWidget_itemClicked(QListWidgetItem *item);


private:
    void initializeFonts();
    Ui::userChat *ui;
    QWebSocket *m_socket; // приватное поле для хранения объекта сокета
    QString m_sessionId;
    QStackedWidget *m_stackedWidget; // новый член класса
    Authentication *m_authentication; // объявляем переменную authentication как член класса
    userChat *m_userchat;

    QFont m_labelFont;
    QFont m_timeLabelFont;
    QFont m_namesurname;
};

#endif // USERCHAT_H
