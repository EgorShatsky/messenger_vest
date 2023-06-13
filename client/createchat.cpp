#include "createchat.h"
#include "ui_createchat.h"
#include "client.h"
#include <QMessageBox>
#include <QFontDatabase>

createChat::createChat(QWebSocket* socket, QString sessionId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::createChat)
    , m_socket(socket)
    , m_sessionId(sessionId)
    ,m_stackedWidget(dynamic_cast<Client*>(parent)->getStackedWidget())
{
    ui->setupUi(this);
    ui->phoneNumberEdit->setInputMask("+7(999) 999-99-99");

    ui->headerLabel->setStyleSheet("background-color: #D9D9D9;");

    //Установка фона для кнопки назад
    QPixmap backPixmap(":/img/img/arrowback.png");
    ui->backButton->setIcon(QIcon(backPixmap));
    ui->backButton->setIconSize(backPixmap.size());

    QPixmap pix(":/img/img/createnewchat.png");
    ui->createaLabel->setPixmap(pix);


    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/Jura.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);

    // Создание экземпляра шрифта
    QFont Jura(fontFamily, 12);
    QFont Jura_8(fontFamily, 8);
    QFont Jura_16(fontFamily, 16);

    // Установка шрифта для виджета
    ui->label_2->setFont(Jura_8);
    ui->label_3->setFont(Jura_8);
    ui->phoneNumberEdit->setFont(Jura);
    ui->findButton->setFont(Jura);

    ui->findButton->setStyleSheet("background-color: #5CCCCC; border-radius: 14px;");
    ui->backButton->setStyleSheet("background-color: #D9D9D9; border: none;");
    ui->phoneNumberEdit->setStyleSheet("border-radius: 14px;");


    // Подключаем обработчик сообщений
    connect(m_socket, &QWebSocket::textMessageReceived, this, &createChat::onTextMessageReceived);
}

createChat::~createChat()
{
    delete ui;
}

void createChat::on_backButton_clicked()
{
    // Переход на форму профиля
    m_stackedWidget->setCurrentIndex(4);

    // Очищаем поле ввода
    ui->phoneNumberEdit->clear();
}

void createChat::on_findButton_clicked()
{
    if (ui->phoneNumberEdit->hasAcceptableInput()){
        QJsonObject messageFindPhone;
        messageFindPhone["method"] = "findPhone";
        messageFindPhone["phone"] = ui->phoneNumberEdit->text();
        messageFindPhone["sessionId"] = m_sessionId;

        QJsonDocument doc(messageFindPhone);
        qDebug() << doc.toJson();

        // Отправляем JSON-объект на сервер
        m_socket->sendTextMessage(doc.toJson());

        // Очищаем поле ввода
        ui->phoneNumberEdit->clear();
    }else {
        // В поле ввода есть ошибки
        QMessageBox::critical(this, "Ошибка", "Пожалуйста, введите номер телефона полностью");
    }

}

void createChat::onTextMessageReceived(QString message)
{
    // преобразуем полученную строку в объект JSON
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());

    // получаем значение поля "method" из JSON объекта
    QString method = jsonResponse.object().value("method").toString();

    // получаем значение полей из JSON объекта
    QString serverSessionId = jsonResponse.object().value("sessionId").toString();
    QString name = jsonResponse.object().value("name").toString(); // имя пользователя с которым создается чат
    QString surname = jsonResponse.object().value("surname").toString(); // фамилия пользователя
    QString status = jsonResponse.object().value("status").toString(); // текущий статус (онлайн/ офлайн)
    int chatId = jsonResponse.object().value("chat_id").toInt(); // id созданного чата
    //надо принять фото профиля

    // проверяем, что значение поля "method" равно "inputCode"
    if (method == "createChat") {
        // Переключение на форму регистрации
        m_stackedWidget->setCurrentIndex(6);
        //on_acceptButton_clicked();
    } else if (method == "errorPhoneNumber") {
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Нельзя создать чат со своим номером телефона!");
    } else if (method == "alreadyExists") {
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Чат с данным пользователем уже существует!");
    }else if (method == "doesNotExists") {
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Пользователя с таким номером телефона не существует!");
    }else if (serverSessionId == m_sessionId) {
        // Сессионные идентификаторы соответствуют
        // Продолжаем обработку сообщения
    } else {
        // Сессионные идентификаторы не совпадают
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Неверный сессионный идентификатор");
    }
}

