#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QWidget>
#include <QtWebSockets>
#include <QStackedWidget>
#include "userchat.h"

namespace Ui {
class userProfile;
}

class userProfile : public QWidget
{
    Q_OBJECT

public:
    explicit userProfile(QWebSocket* socket, QString sessionId, userChat *uchat,  QWidget *parent);
    ~userProfile();

private slots:
    void on_backButton_clicked();
    void on_pushButton_clicked();
    void on_saveButton_clicked();
    void on_cameraButton_clicked();
    void onTextMessageReceived(QString message);
    void onJsonReceived(QJsonObject json);

private:
    Ui::userProfile *ui;
    QWebSocket *m_socket; // приватное поле для хранения объекта сокета
    QString m_sessionId;
    QStackedWidget *m_stackedWidget; // новый член класса
    userChat *m_userchat;
};

#endif // USERPROFILE_H
