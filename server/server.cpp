#include "server.h"
#include "ui_server.h"
#include <QtWebSockets>
#include <QtCore>
#include <QDebug>
#include <QFileDialog>
#include <QTextDocument>

// Конструктор класса Server, принимает в качестве аргументов порт и родительский объект
Server::Server(quint16 port, QObject *parent)
    : QMainWindow(nullptr)
    , ui(new Ui::Server)
    , m_port(port)
{
    ui->setupUi(this);
    ui->lcdNumber->setStyleSheet("color: red;");
    m_server = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);
    connect(m_server, &QWebSocketServer::newConnection, this, &Server::onNewConnection);
}

// Деструктор класса Server, удаляет объект ui и закрывает сервер.
Server::~Server()
{
    delete ui;
    m_server->close();
}

// Запуск сервера
void Server::start()
{
    if (m_server->listen(QHostAddress::Any, m_port)) {
        qDebug() << "Сервер запущен на порту:" << m_port;
        QString message = "Сервер успешно запущен! Порт: " + QString::number(m_port);
        updateTextEdit(message);

    } else {
        qDebug() << "Failed to start WebSocket server on port" << m_port;
        qDebug() << "Error:" << m_server->errorString();
    }
}

// Метод класса Server, который вызывается при подключении нового клиента к серверу.
void Server::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QWebSocket::textMessageReceived, this, &Server::processTextMessage);
    connect(socket, &QWebSocket::disconnected, this, &Server::socketDisconnected);

    // Сохраняем соединение в списке
    m_connections.insert(socket, "");

    // Выводим информацию о подключении клиента в QTextEdit
    QString message = "Новый клиент подключен: " + socket->peerAddress().toString();
    updateTextEdit(message);

    // Обновляем количество клиентов в QLabel
    int clientCount = m_connections.size();
    ui->lcdNumber->display(clientCount);
}

void Server::updateTextEdit(const QString& message)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeString = currentTime.toString("hh:mm:ss");  // Форматирование времени

    QString messageWithTime = "[" + timeString + "] " + message;
    ui->textEdit->append(messageWithTime);
}

// Вызывается при получении текстового сообщения от клиента.
void Server::processTextMessage(const QString& message)
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
        if (socket) {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
            QJsonObject jsonObj = jsonDoc.object();
            QString method = jsonObj.value("method").toString();
            if (method == "inputPhoneNumber") {
                QString phoneNumber = jsonObj.value("phoneNumber").toString();
                QString sessionId = jsonObj.value("sessionId").toString();
                // Сохраняем пару "sessionId - phoneNumber" в QMap
                m_phoneNumbers.insert(sessionId, phoneNumber);
                // Обновляем значение соответствующей записи в списке m_connections
                m_connections[socket] = sessionId;
                inputPhoneNumber(phoneNumber);
            }else if (method == "inputCode") {
                QString code = jsonObj.value("code").toString();
                QString sessionId = jsonObj.value("sessionId").toString();
                inputCode(code,sessionId);
            }else if (method == "changePhoneNumber") {
                // Допиши метод потом
            }else if (method == "inputPassword"){
                QString password = jsonObj.value("password").toString();
                QString sessionId = jsonObj.value("sessionId").toString();
                inputPassword(password,sessionId);
                QString messageText = "Авторизация пользователя: sessionId=" + sessionId;
                updateTextEdit(messageText);
            }else if (method == "inputUserData") {
                QString name = jsonObj.value("name").toString();
                QString surname = jsonObj.value("surname").toString();
                QString password = jsonObj.value("password").toString();
                QString sessionId = jsonObj.value("sessionId").toString();
                inputUserData(name, surname, password, sessionId);
                QString messageText = "Регистрация пользователя: sessionId=" + sessionId;
                updateTextEdit(messageText);
            }else if (method == "findPhone") {
                QString searchPhone = jsonObj.value("phone").toString();
                QString sessionId = jsonObj.value("sessionId").toString();
                findPhone(searchPhone, sessionId);
            }else if (method == "messageOutput") {
                QString chatId = jsonObj.value("chat_id").toString();
                QString sessionId = jsonObj.value("sessionId").toString();
                messageOutput(chatId,sessionId);
            }else if (method == "sendMessage") {
                QString sessionId = jsonObj.value("sessionId").toString();
                QString text = jsonObj.value("text").toString();
                QString telephone = jsonObj.value("telephone_number").toString();
                QString chatId = jsonObj.value("chat_id").toString();
                sendMessage(chatId, telephone, text, sessionId);
                QString messageText = "Отправка сообщения: sessionId=" + sessionId + ", от=" + sessionId + ", to=" + telephone;
                updateTextEdit(messageText);
            }else if(method == "inputUserData"){
                QString sessionId = jsonObj.value("sessionId").toString();
                QString name = jsonObj.value("name").toString();
                QString surname = jsonObj.value("surname").toString();
                QString password = jsonObj.value("password").toString();
                QString phone = jsonObj.value("telephone").toString();
                inputUserData(name, surname, password, phone, sessionId);
            }else if (method == "profilePhotoUpload") {
                QString sessionId = jsonObj.value("sessionId").toString();
                QString base64ImageData = jsonObj.value("image").toString();
                QString imageName = jsonObj.value("name").toString();
                qint64 imageSize = jsonObj.value("size").toDouble(); // Используется toDouble() для преобразования в qint64
                QString extension = jsonObj.value("extension").toString();
                saveProfilePhoto(sessionId,base64ImageData,imageName, imageSize, extension);
            }else if (method == "pushSettingsButton"){
                QString sessionId = jsonObj.value("sessionId").toString();
                pushSettingsButton(sessionId);
            }
        }
}

// Вызывается при отключении клиента от сервера
void Server::socketDisconnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    if (socket) {
        // Удаление клиента из списка подключений
        m_connections.remove(socket);

        // Выводим информацию об отключении клиента в QTextEdit
        QString message = "Клиент отключился: " + socket->peerAddress().toString();
        updateTextEdit(message);

        // Обновляем количество клиентов в QLabel
        int clientCount = m_connections.size();
        ui->lcdNumber->display(clientCount);

        socket->deleteLater();
    }
}

void Server::on_startButton_clicked()
{
    start(); // вызываем метод start(), чтобы запустить сервер
        if (m_server->isListening()) {
            qDebug() << "Сервер успешно запущен на порту" << m_port;
        } else {
            qDebug() << "Не удалось запустить сервер на порту" << m_port;
            qDebug() << "Ошибка:" << m_server->errorString();
        }
}

void Server::on_stopButton_clicked()
{
    m_server->close(); // закрываем сервер
    QThread::msleep(100); // ждем 100 миллисекунд
        if (m_server->isListening()) {
            qDebug() << "Не удалось остановить сервер";
        } else {
            qDebug() << "Сервер остановлен";
        }

    QString message = "Сервер остановлен";
    updateTextEdit(message);

}

void Server::on_logsButton_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить логи", QString(), "Текстовые файлы (*.txt)");

        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                stream << ui->textEdit->toPlainText();
                file.close();
                qDebug() << "Логи успешно сохранены в файл:" << filePath;
            } else {
                qDebug() << "Не удалось открыть файл для сохранения:" << file.errorString();
            }
        } else {
            qDebug() << "Выбор файла для сохранения отменен";
        }
}

