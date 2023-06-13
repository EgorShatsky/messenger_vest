#include "registration.h"
#include "ui_registration.h"
#include "client.h"
#include <QMessageBox>
#include <QDebug>
#include <QFontDatabase>

Registration::Registration(QWebSocket* socket, QString sessionId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Registration)
    , m_socket(socket)
    , m_sessionId(sessionId)
    ,m_stackedWidget(dynamic_cast<Client*>(parent)->getStackedWidget())
{
    ui->setupUi(this);

    QPixmap pix(":/img/img/registration.png");
    ui->registrationLabel->setPixmap(pix);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/Jura.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);

    // Создание экземпляра шрифта
    QFont Jura(fontFamily, 12);
    QFont Jura_8(fontFamily, 8);

    // Установка шрифта для виджета
    ui->label_2->setFont(Jura_8);
    ui->label_3->setFont(Jura_8);
    ui->label_4->setFont(Jura_8);
    ui->label_5->setFont(Jura_8);
    ui->createAccountButton->setFont(Jura);
    ui->nameEdit->setFont(Jura);
    ui->passwordConfirmationEdit->setFont(Jura);
    ui->passwordEdit->setFont(Jura);
    ui->surnameEdit->setFont(Jura);

    ui->createAccountButton->setStyleSheet("background-color: #5CCCCC; border-radius: 14px;");
    ui->nameEdit->setStyleSheet("border-radius: 5px;");
    ui->passwordConfirmationEdit->setStyleSheet("border-radius: 5px;");
    ui->passwordEdit->setStyleSheet("border-radius: 5px;");
    ui->surnameEdit->setStyleSheet("border-radius: 5px;");

    // Подключаем обработчик сообщений
    connect(m_socket, &QWebSocket::textMessageReceived, this, &Registration::onTextMessageReceived);
}

Registration::~Registration()
{
    delete ui;
}

void Registration::on_createAccountButton_clicked()
{
    QString name = ui->nameEdit->text(); //записываем имя в переменную
    //qDebug() << "Имя" << name;
    QString surname = ui->surnameEdit->text(); //записываем фамилию в переменную
    //qDebug() << "Фамилия" << surname;
    QString password = ui->passwordEdit->text(); //записываем пароль в переменную
    QString confirmPassword = ui->passwordConfirmationEdit->text(); //записываем пароль в переменную

    QRegExp nameRegex("[а-яА-ЯёЁ]+"); // Регулярное выражение для имени (только русские символы)
    QRegExp surnameRegex("[а-яА-ЯёЁ]+"); // Регулярное выражение для фамилии (только русские символы)

    if (!nameRegex.exactMatch(name)) {
        QMessageBox::warning(this, "Предупреждение!", "Некорректно введено имя");
        return;
    }

    if (!surnameRegex.exactMatch(surname)) {
        QMessageBox::warning(this, "Предупреждение!", "Некорректно введена фамилия");
        return;
    }

    QRegExp passwordRegex("^(?=.*[A-Z])(?=.*[!@#?\\/.,])(?=.*[0-9])(?=.*[a-z]).{8,255}$"); // Регулярное выражение для пароля

    if (!passwordRegex.exactMatch(password)) {
        QMessageBox::warning(this, "Предупреждение!", "Пароль должен содержать минимум 8 символов, одну строчную и одну заглавную латинскую букву, а также специальный символ");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Предупреждение!", "Пароли не совпадают");
        return;
    }

    QJsonObject messageRegistration;

    messageRegistration["method"] = "inputUserData";
    messageRegistration["name"] = name; // отправялем имя пользователя
    messageRegistration["surname"] = surname; // отправялем фамилию пользователя
    messageRegistration["password"] = password; // отправялем пароль пользователя
    messageRegistration["sessionId"] = m_sessionId; // отправялем sessin_id пользователя

    QJsonDocument doc(messageRegistration);
    qDebug() << doc.toJson();

    // Отправляем JSON-объект на сервер
    m_socket->sendTextMessage(doc.toJson());

}

void Registration::onTextMessageReceived(QString message)
{
    // преобразуем полученную строку в объект JSON
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());

    // получаем значение поля "method" из JSON объекта
    QString method = jsonResponse.object().value("method").toString();

    // получаем значение поля "clientId" из JSON объекта
    QString serverSessionId = jsonResponse.object().value("sessionId").toString();

    // проверяем, что значение поля "method" равно "inputCode"
    if (method == "loginSuccessful") {
        // Переключение на форму чатов пользователя
        m_stackedWidget->setCurrentIndex(4);
        //on_acceptButton_clicked();
    }else if (method == "statusError") {
        // Вывод ошибки на label
        QString errorMessage = "не удалось обновить статус пользователя";
        qDebug()<< errorMessage;
    }else if (method == "registrationFailed"){
        QString errorMessage = "Регистрация не успешна";
        qDebug()<< errorMessage;
    }else if (serverSessionId == m_sessionId) {
        // Сессионные идентификаторы соответствуют
        // Продолжаем обработку сообщения
    } else {
        // Сессионные идентификаторы не совпадают
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Неверный сессионный идентификатор");
    }
}

