#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QtCore>
#include <QWebSocketServer>
#include <QtNetwork>
#include <QMap>
#include <QString>
#include <QVariantList>

QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QMainWindow
{
    Q_OBJECT

public:
    Server(quint16 port, QObject *parent = nullptr);
    void start();
    int getUserIdByPhoneNumber(const QString& phoneNumber);
    QMap<QString, QVariantList> getChatList(int userId);
    ~Server();

signals:
    void messageReceived(const QString& message, QWebSocket* socket);

private slots:
    void onNewConnection();
    void processTextMessage(const QString& message);
    void socketDisconnected();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void inputPhoneNumber(const QString& phoneNumber);
    void inputCode(const QString &code, const QString &sessionId);
    void changePhoneNumber(const QString &sessionId);
    void inputPassword(const QString& phoneNumber, const QString &sessionId);
    void inputUserData(const QString& name, const QString& surname, const QString& password,const QString& sessionId);
    void sendJsonResponse(QString method, const QString &sessionId);
    void sendJsonObject(const QString& method, const QString& sessionId, const QJsonObject& userObject);
    void findPhone(const QString&phone, const QString&sessionId);
    void messageOutput(const QString&chatId,const QString&sessionId);
    void inputUserData(const QString&name, const QString&surname, const QString&password, const QString&telephone, const QString&sessionId);
    void sendMessage(const QString&chatId, const QString&telephone, const QString&text, const QString&sessionId);
    void pushSettingsButton(const QString& sessionId);
    void saveProfilePhoto(const QString& sessionId, const QString& base64ImageData, const QString& imageName, const int& imageSize, const QString& extension);
    void sendSms(QString phoneNumber, QString message);
    void onSmsSent(QNetworkReply *reply);
    void updateTextEdit(const QString& message);

    void on_logsButton_clicked();

private:
    Ui::Server *ui;
    QString phoneNumber;
    quint16 m_port;
    QWebSocketServer* m_server; // указатель на объект QWebSocketServer
    QMap<QWebSocket*, QString> m_connections; // QMap для хранения соединений с клиентами
    QMap<QString, QString> m_phoneNumbers; // QMap для хранения пары "sessionId - phoneNumber"
};

#endif // SERVER_H
