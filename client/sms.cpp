#include "sms.h"
#include "ui_sms.h"
#include "client.h"
#include <QMessageBox>
#include <QDebug>
#include <QFontDatabase>

Sms::Sms(QWebSocket* socket, QString sessionId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Sms)
    , m_socket(socket)
    , m_sessionId(sessionId)
    ,m_stackedWidget(dynamic_cast<Client*>(parent)->getStackedWidget())
{
    ui->setupUi(this);
    ui->smsEdit->setInputMask("0000");
    ui->smsEdit->setPlaceholderText("0000");
    ui->smsEdit->setCursorPosition(0);
    ui->smsEdit->setFocus();

    QPixmap pix(":/img/img/podver.png");
    ui->numberLabel->setPixmap(pix);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/Jura.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);

    // Создание экземпляра шрифта
    QFont Jura(fontFamily, 12);
    QFont Jura_8(fontFamily, 8);

    // Установка шрифта для виджета
    ui->numberLabel->setFont(Jura_8);
    ui->errorLabel->setFont(Jura_8);
    ui->continueButton->setFont(Jura);
    ui->acceptButton->setFont(Jura);
    ui->smsEdit->setFont(Jura);

    ui->continueButton->setStyleSheet("background-color: #5CCCCC; border-radius: 14px;");
    ui->acceptButton->setStyleSheet("background-color: #5CCCCC; border-radius: 14px;");
    ui->smsEdit->setStyleSheet("border-radius: 14px;");

    // Подключаем обработчик сообщений
    connect(m_socket, &QWebSocket::textMessageReceived, this, &Sms::onTextMessageReceived);

}

Sms::~Sms()
{
    delete ui;
}

void Sms::on_acceptButton_clicked()
{
    if (ui->smsEdit->hasAcceptableInput()){
        QJsonObject messageSms;
        messageSms["method"] = "inputCode";
        messageSms["code"] = ui->smsEdit->text();
        messageSms["sessionId"] = m_sessionId;

        QJsonDocument doc(messageSms);
        qDebug() << doc.toJson();

        // Отправляем JSON-объект на сервер
        m_socket->sendTextMessage(doc.toJson());
    }else {
        // В поле ввода есть ошибки
        QMessageBox::critical(this, "Ошибка", "Пожалуйста, введите код подтверждения полностью");
    }

}

void Sms::onTextMessageReceived(QString message)
{
    // преобразуем полученную строку в объект JSON
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());

    // получаем значение поля "method" из JSON объекта
    QString method = jsonResponse.object().value("method").toString();

    // получаем значение поля "clientId" из JSON объекта
    QString serverSessionId = jsonResponse.object().value("sessionId").toString();

    // проверяем, что значение поля "method" равно "inputCode"
    if (method == "registrationPage") {
        // Переключение на форму регистрации
        m_stackedWidget->setCurrentIndex(2);
        //on_acceptButton_clicked();
    }else if (method == "authenticationPage") {
        // Переключение на форму авторизации
        m_stackedWidget->setCurrentIndex(1);
    }else if (method == "errorCode") {
        // Вывод ошибки
        QString errorMessage = "Неверный код подтверждения";
        ui->errorLabel->setText(QString("<font color=\"red\">%1</font>").arg(errorMessage));
    }else if (method == "repeatCode") {
        // Перегенерируем код и отправляем еще раз
    }else if (serverSessionId == m_sessionId) {
        // Сессионные идентификаторы соответствуют
        // Продолжаем обработку сообщения
    } else {
        // Сессионные идентификаторы не совпадают
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Неверный сессионный идентификатор");
    }
}

void Sms::on_continueButton_clicked()
{
    QJsonObject messagechangePhoneNumber;
    messagechangePhoneNumber["method"] = "changePhoneNumber";
    messagechangePhoneNumber["sessionId"] = m_sessionId;

    QJsonDocument doc(messagechangePhoneNumber);
    qDebug() << doc.toJson();

    // Отправляем JSON-объект на сервер
    m_socket->sendTextMessage(doc.toJson());
}
