#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QWebSocket>
#include <QStackedWidget>
#include "authentication.h"
#include "sms.h"
#include "registration.h"
#include "userchat.h"
#include "userprofile.h"
#include "chat.h"
#include "createchat.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class QStackedWidget;

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    QStackedWidget* getStackedWidget(); // новый метод
    ~Client();

private slots:
    void onConnected();
    void onDisconnected();
    void on_continueButton_clicked();
    void on_continueButton_pressed();

private:
    Ui::Client *ui;
    QWebSocket *m_socket;
    QStackedWidget *m_stackedWidget; // объявление QStackedWidget
    Authentication *m_authForm;
    Registration *m_regForm;
    Sms *m_smsForm;
    userChat *m_chatForm;
    userProfile *m_userProfileForm;
    Chat *m_Chat;
    createChat *m_createChatForm;
    QString m_sessionId; // Добавляем переменную sessionId как приватный член класса
};

#endif // CLIENT_H
