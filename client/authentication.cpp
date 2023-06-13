#include "authentication.h"
#include "ui_authentication.h"
#include "client.h"
#include <QMessageBox>
#include <QDebug>
#include <QFontDatabase>

Authentication::Authentication(QWebSocket* socket, QString sessionId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Authentication)
    , m_socket(socket)
    , m_sessionId(sessionId)
    ,m_stackedWidget(dynamic_cast<Client*>(parent)->getStackedWidget())
{
    ui->setupUi(this);
    QPixmap pix(":/img/img/enter.png");
    ui->enterLabel->setPixmap(pix);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/Jura.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);

    // Создание экземпляра шрифта
    QFont Jura(fontFamily, 12);
    QFont Jura_8(fontFamily, 8);
    QFont Jura_16(fontFamily, 16);

    // Установка шрифта для виджета
    ui->label->setFont(Jura_8);
    ui->passwordEdit->setFont(Jura_16);
    ui->entranceButton->setFont(Jura);

    ui->entranceButton->setStyleSheet("background-color: #5CCCCC; border-radius: 14px;");
    ui->passwordEdit->setStyleSheet("border-radius: 14px;");

    // Подключаем обработчик сообщений
    connect(m_socket, &QWebSocket::textMessageReceived, this, &Authentication::onTextMessageReceived); 
}

Authentication::~Authentication()
{
    delete ui;
}

void Authentication::on_entranceButton_clicked()
{
    QJsonObject messagePassword;

    // Формирование полей JSON объекта
    messagePassword["method"] = "inputPassword";
    messagePassword["password"] = ui->passwordEdit->text();
    messagePassword["sessionId"] = m_sessionId;

    QJsonDocument doc(messagePassword);
    qDebug() << doc.toJson();

    // Отправляем JSON-объект на сервер
    m_socket->sendTextMessage(doc.toJson());
}

void Authentication::onTextMessageReceived(QString message)
{
    // преобразуем полученную строку в объект JSON
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());

    // получаем значение поля "method" из JSON объекта
    QString method = jsonResponse.object().value("method").toString();

    // получаем значение поля "clientId" из JSON объекта
    QString serverSessionId = jsonResponse.object().value("sessionId").toString();

    //в этом объекте еще фото профиля оно передается ниже

    // проверяем значение поля "method"
    if (method == "successfulEntry") {
        // Переключение на форму чатов пользователя
        m_stackedWidget->setCurrentIndex(4);

        // вызываем сигнал, передавая объект jsonResponse.object()
        emit jsonReceived(jsonResponse.object());

    }else if (method == "loginFailed") {
        // Вывод ошибки на label
        QString errorMessage = "Неверный пароль";
        ui->errorLabel->setText(QString("<font color=\"red\">%1</font>").arg(errorMessage));
    }else if (serverSessionId == m_sessionId) {
        // Сессионные идентификаторы соответствуют
        // Продолжаем обработку сообщения
    } else {
        // Сессионные идентификаторы не совпадают
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Неверный сессионный идентификатор");
    }
}

