#include "userprofile.h"
#include "ui_userprofile.h"
#include "client.h"
#include "userchat.h"
#include <QFontDatabase>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>
#include <QPainterPathStroker>

userProfile::userProfile(QWebSocket* socket, QString sessionId, userChat *uchat, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::userProfile)
    , m_socket(socket)
    , m_sessionId(sessionId)
    , m_stackedWidget(dynamic_cast<Client*>(parent)->getStackedWidget())
    , m_userchat(uchat)
{
    ui->setupUi(this);
    ui->phoneEdit->setInputMask("+7(999) 999-99-99");

    ui->label_5->setStyleSheet("background-color: #D9D9D9;");
    ui->label_6->setStyleSheet("background-color: #FFFFFF;");
    ui->backButton->setStyleSheet("background-color: #D9D9D9; border: none; border-radius: 14px;");
    ui->cameraButton->setStyleSheet("background-color: #FFFFFF; border: none; border-radius: 14px;");

    QPixmap zaglushka(":/img/img/zaglushka.png");
    ui->zaglushkaLabel->setPixmap(zaglushka);

    //Установка фона для кнопки назад
    QPixmap backPixmap(":/img/img/arrowback.png");
    ui->backButton->setIcon(QIcon(backPixmap));
    ui->backButton->setIconSize(backPixmap.size());

    //Установка фона для кнопки камеры
    QPixmap cameraPixmap(":/img/img/camera.png");
    ui->cameraButton->setIcon(QIcon(cameraPixmap));
    ui->cameraButton->setIconSize(cameraPixmap.size());

    QPixmap pix(":/img/img/profile.png");
    ui->label->setPixmap(pix);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/Jura.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);

    // Создание экземпляра шрифта
    QFont Jura(fontFamily, 12);
    QFont Jura_8(fontFamily, 8);
    QFont Jura_10(fontFamily, 10);

    // Установка шрифта для виджета
    ui->label_2->setFont(Jura_8);
    ui->label_3->setFont(Jura_8);
    ui->label_4->setFont(Jura_8);
    ui->label_7->setFont(Jura_8);
    ui->label_8->setFont(Jura_8);
    ui->nameLabel->setFont(Jura_10);
    ui->surnameLabel->setFont(Jura_10);
    ui->nameEdit->setFont(Jura);
    ui->phoneEdit->setFont(Jura);
    ui->surnameEdit->setFont(Jura);
    ui->passwordEdit->setFont(Jura);
    ui->passwordEdit_2->setFont(Jura);

    ui->saveButton->setStyleSheet("background-color: #5CCCCC; border-radius: 14px;");
    ui->nameEdit->setStyleSheet("border-radius: 5px; border: 1px solid black;");
    ui->phoneEdit->setStyleSheet("border-radius: 5px; border: 1px solid black;");
    ui->surnameEdit->setStyleSheet("border-radius: 5px; border: 1px solid black;");
    ui->passwordEdit->setStyleSheet("border-radius: 5px; border: 1px solid black;");
    ui->passwordEdit_2->setStyleSheet("border-radius: 5px; border: 1px solid black;");

    connect(m_userchat, &userChat::jsonReceived, this, &userProfile::onJsonReceived);
}

userProfile::~userProfile()
{
    delete ui;
}

void userProfile::on_backButton_clicked()
{
    // Переход на форму чатов
    m_stackedWidget->setCurrentIndex(4);
}

void userProfile::on_saveButton_clicked()
{
    // Проверяем, было ли введено значение в поле номера телефона
    QString phone = ui->phoneEdit->text();
    QString name = ui->nameEdit->text(); // записываем имя в переменную
    QString surname = ui->surnameEdit->text(); // записываем фамилию в переменную
    QString password = ui->passwordEdit->text(); // записываем пароль в переменную
    QString confirmPassword = ui->passwordEdit_2->text(); // записываем подтверждение пароля в переменную

    QRegExp nameRegex("[а-яА-ЯёЁ]+"); // Регулярное выражение для имени (только русские символы)
    QRegExp surnameRegex("[а-яА-ЯёЁ]+"); // Регулярное выражение для фамилии (только русские символы)

    // Проверяем, были ли введены какие-либо данные
    if (phone == "+7() --" && name.isEmpty() && surname.isEmpty() && password.isEmpty() && confirmPassword.isEmpty()) {
        QMessageBox::critical(this, "Информация", "Изменений не обнаружено");
        return;
    }

    // Проверяем, было ли изменено имя и соответствует ли оно требованиям
    if (!name.isEmpty() && !nameRegex.exactMatch(name)) {
        QMessageBox::warning(this, "Предупреждение!", "Некорректно введено имя");
        return;
    }

    // Проверяем, была ли изменена фамилия и соответствует ли она требованиям
    if (!surname.isEmpty() && !surnameRegex.exactMatch(surname)) {
        QMessageBox::warning(this, "Предупреждение!", "Некорректно введена фамилия");
        return;
    }

    // Проверяем, был ли изменен номер телефона
    if (!phone.isEmpty() && phone != "+7() --" && !ui->phoneEdit->hasAcceptableInput()) {
        QMessageBox::critical(this, "Ошибка", "Пожалуйста, введите номер телефона полностью");
        return;
    }

    // Проверяем, был ли изменен пароль и соответствует ли он требованиям
    QRegExp passwordRegex("^(?=.*[A-Z])(?=.*[!@#?\\/.,])(?=.*[0-9])(?=.*[a-z]).{8,255}$"); // Регулярное выражение для пароля
    if (!password.isEmpty() && !passwordRegex.exactMatch(password)) {
        QMessageBox::warning(this, "Предупреждение!", "Пароль должен содержать минимум 8 символов, одну строчную и одну заглавную латинскую букву, а также специальный символ");
        return;
    }

    // Проверяем, совпадают ли пароль и подтверждение пароля
    if (!password.isEmpty() && password != confirmPassword) {
        QMessageBox::warning(this, "Предупреждение!", "Пароли не совпадают");
        return;
    }

    // Создаем JSON-объект с измененными данными
    QJsonObject messageUserProfile;
    messageUserProfile["method"] = "inputUserData";
    messageUserProfile["sessionId"] = m_sessionId; // отправляем session_id пользователя

    if (!name.isEmpty())
        messageUserProfile["name"] = name; // отправляем имя пользователя

    if (!surname.isEmpty())
        messageUserProfile["surname"] = surname;

    if (ui->phoneEdit->hasAcceptableInput())
        messageUserProfile["telephone"] = phone;

    if (!password.isEmpty())
        messageUserProfile["password"] = password;


    // Отправляем JSON-объект на сервер
    QJsonDocument doc(messageUserProfile);

    // Отправляем JSON-объект на сервер
    m_socket->sendTextMessage(doc.toJson());

    // Очищаем все LineEdits
    ui->phoneEdit->clear();
    ui->nameEdit->clear();
    ui->surnameEdit->clear();
    ui->passwordEdit->clear();
    ui->passwordEdit_2->clear();
}

void userProfile::on_pushButton_clicked()
{

}

void userProfile::on_cameraButton_clicked()
{
    // Выбор изображениея
    QString imagePath = QFileDialog::getOpenFileName(this, "Выберите фото профиля", "", "(*.jpg *.png)");

    // Если выборан файл
    if (!imagePath.isEmpty()) {
        // Загрузка изображения с QImage
        QImage image(imagePath);

        // Изменить размер на 70x70 пикселей
        QImage resizedImage = image.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Создание QPixmap из изображения с измененным размером
        QPixmap pixmap = QPixmap::fromImage(resizedImage);

        // Создание круглой маски для растрового изображения
        QPixmap circularPixmap(70, 70);
        circularPixmap.fill(Qt::transparent);

        QPainter painter(&circularPixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, 70, 70);
        painter.setClipRegion(QRegion(0, 0, 70, 70, QRegion::Ellipse));
        painter.drawPixmap(0, 0, 70, 70, pixmap);

        // Отображение круглого изображения в виджете zaglushkaLabel
        ui->zaglushkaLabel->setPixmap(circularPixmap);
        ui->zaglushkaLabel->setScaledContents(true);

        // Преобразование круглого QPixmap в QByteArray
        QByteArray imageData;
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        circularPixmap.save(&buffer, "PNG"); // Сохранение QPixmap в формате PNG в буфер

        // Подготовка информации об изображении
        QFileInfo fileInfo(imagePath);
        QString fileName = fileInfo.fileName();
        qint64 fileSize = fileInfo.size();
        QString fileExtension = fileInfo.suffix();

        // Создание JSON-объекта для хранения информации об изображении
        QJsonObject imageInfo;
        imageInfo["sessionId"] = m_sessionId;
        imageInfo["method"] = "profilePhotoUpload";
        imageInfo["name"] = fileName;
        imageInfo["size"] = fileSize;
        imageInfo["extension"] = fileExtension;
        imageInfo["image"] = QString::fromLatin1(imageData.toBase64());
        imageInfo["info"] = imageInfo;

        // Преобразование JSON-объекта в QJsonDocument
        QJsonDocument jsonDocument(imageInfo);

        // Отправка данных и информации об изображении на сервер через web socket
        m_socket->sendTextMessage(jsonDocument.toJson());
    }
}

void userProfile::onTextMessageReceived(QString message)
{
    // преобразуем полученную строку в объект JSON
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());

    // получаем значение поля "method" из JSON объекта
    QString method = jsonResponse.object().value("method").toString();

    // получаем значение поля "clientId" из JSON объекта
    QString serverSessionId = jsonResponse.object().value("sessionId").toString();

    // проверяем, что значение поля "method" равно "inputCode"
    if (method == "successfulDataChange") {
        // Переключение на форму чатов пользователя
        m_stackedWidget->setCurrentIndex(4);
        //on_acceptButton_clicked();
    }else if (method == "errorDataChange") {
        // Вывод ошибки
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить информацию о пользователе");
    }else if (serverSessionId == m_sessionId) {
        // Сессионные идентификаторы соответствуют
        // Продолжаем обработку сообщения
    } else {
        // Сессионные идентификаторы не совпадают
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Неверный сессионный идентификатор");
    }
}

void userProfile::onJsonReceived(QJsonObject json)
{
    // Получение переданных данных
    QString name = json.value("name").toString();
    QString surname = json.value("surname").toString();
    QString base64ImageData = json.value("base64ImageData").toString();

    // Пример обновления интерфейса
    ui->nameLabel->setText(name);
    ui->surnameLabel->setText(surname);

    // Пример сохранения изображения
    QByteArray imageData = QByteArray::fromBase64(base64ImageData.toUtf8());

    // Пример сохранения изображения
    if (base64ImageData != "zaglushka") {
        QByteArray imageData = QByteArray::fromBase64(base64ImageData.toUtf8());
        ui->zaglushkaLabel->setPixmap(QPixmap::fromImage(QImage::fromData(imageData)));
    }
}
