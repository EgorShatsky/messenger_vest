#include "client.h"
#include "ui_client.h"
#include "authentication.h"
#include "registration.h"
#include "sms.h"
#include "userchat.h"
#include "userprofile.h"
#include "chat.h"
#include "createchat.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStackedWidget>
#include <QUuid>
#include <QFontDatabase>


Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    ui->phoneNumberEdit->setInputMask("+7(999) 999-99-99");
    QPixmap pix(":/img/img/Group 13.png");
    ui->logoLabel->setPixmap(pix);
    m_socket = new QWebSocket();
    connect(m_socket, &QWebSocket::connected, this, &Client::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &Client::onDisconnected);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/Jura.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);

    // Создание экземпляра шрифта
    QFont Jura(fontFamily, 12);
    QFont Jura_8(fontFamily, 8);

    // Установка шрифта для виджета
    ui->label->setFont(Jura_8);
    ui->continueButton->setFont(Jura);
    ui->phoneNumberEdit->setFont(Jura);

    // Создание уникального идентификатора
    QUuid uuid = QUuid::createUuid();
    //QString m_sessionId = uuid.toString();
    m_sessionId = uuid.toString();

    // Инициализация QStackedWidget и добавление на него форм
    m_stackedWidget = new QStackedWidget(this);

    m_stackedWidget->addWidget(centralWidget());  // добавление главного виджета в QStackedWidget
    m_authForm = new Authentication(m_socket, m_sessionId, this);
    m_regForm = new Registration(m_socket, m_sessionId, this);
    m_smsForm = new Sms(m_socket, m_sessionId, this);
    m_chatForm = new userChat(m_socket, m_sessionId,m_authForm, this);
    m_userProfileForm = new userProfile(m_socket, m_sessionId, m_chatForm, this);
    m_Chat = new Chat(m_socket, m_sessionId, m_chatForm, this);
    m_createChatForm = new createChat(m_socket, m_sessionId, this);

    m_stackedWidget->addWidget(m_authForm); //(1)
    m_stackedWidget->addWidget(m_regForm); //(2)
    m_stackedWidget->addWidget(m_smsForm); //(3)
    m_stackedWidget->addWidget(m_chatForm); //(4)
    m_stackedWidget->addWidget(m_userProfileForm); //(5)
    m_stackedWidget->addWidget(m_Chat); //(6)
    m_stackedWidget->addWidget(m_createChatForm); //(7)

    m_stackedWidget->setCurrentIndex(0); // установка текущего окна (Authentication)
    setCentralWidget(m_stackedWidget); // установка QStackedWidget в качестве центрального виджета

    ui->continueButton->setStyleSheet("background-color: #5CCCCC; border-radius: 14px;");
    ui->phoneNumberEdit->setStyleSheet("border-radius: 14px;");
}

Client::~Client()
{
    delete ui;
    delete m_stackedWidget;

}

QStackedWidget* Client::getStackedWidget() {
    return m_stackedWidget;
}

// Метод onConnected() вызывается, когда установлено соединение с сервером
void Client::onConnected()
{
    if (ui->phoneNumberEdit->hasAcceptableInput()) {
        qDebug() << "Соединение с сервером установлено";

        QJsonObject message;
        message["method"] = "inputPhoneNumber";
        message["phoneNumber"] = ui->phoneNumberEdit->text();
        message["sessionId"] = m_sessionId; // Добавляем уникальный идентификатор сессии в сообщение

        QJsonDocument doc(message);
        qDebug() << doc.toJson();
        m_socket->sendTextMessage(doc.toJson());

        // Переход на форму sms после корректного ввода номера телефона
        m_stackedWidget->setCurrentIndex(3);
    }
    else{
        // В поле ввода есть ошибки
        QMessageBox::critical(this, "Ошибка", "Пожалуйста, введите номер телефона полностью");
    }

}

void Client::onDisconnected()
{
    qDebug() << "Отключен от сервера";
}

void Client::on_continueButton_clicked()
{
    m_socket->open(QUrl("ws://localhost:8080")); // замените на адрес и порт вашего сервера
}

void Client::on_continueButton_pressed()
{

}

