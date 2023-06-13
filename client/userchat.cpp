#include "userchat.h"
#include "ui_userchat.h"
#include "client.h"
#include <QMessageBox>
#include <QDebug>
#include <QFontDatabase>
#include <QHBoxLayout>

userChat::userChat(QWebSocket* socket, QString sessionId, Authentication* auth, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::userChat)
    , m_socket(socket)
    , m_sessionId(sessionId)
    , m_stackedWidget(dynamic_cast<Client*>(parent)->getStackedWidget())
    , m_authentication(auth)
{
    ui->setupUi(this);

    initializeFonts();

    //Установка фона для кнопки настроек
    QPixmap settingsPixmap(":/img/img/settings.png");
    ui->settingsButton->setIcon(QIcon(settingsPixmap));
    ui->settingsButton->setIconSize(settingsPixmap.size());

    //Установка фона для кнопки создания чата
    QPixmap newchatPixmap(":/img/img/newchat.png");
    ui->newchatButton->setIcon(QIcon(newchatPixmap));
    ui->newchatButton->setIconSize(newchatPixmap.size());

    QPixmap pix(":/img/img/messenger_logo.png");
    ui->logoLabel->setPixmap(pix);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/Jura.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);

    // Создание экземпляра шрифта
    QFont Jura(fontFamily, 12);
    QFont Jura_8(fontFamily, 8);

    // Установка шрифта для виджета
    ui->listWidget->setFont(Jura);

    ui->listWidget->setStyleSheet("QListWidget {background-color: #FFFFFF;}");
    ui->label->setStyleSheet("background-color: #D9D9D9;");
    ui->newchatButton->setStyleSheet("background-color: #D9D9D9; border: none; border-radius: 14px;");
    ui->settingsButton->setStyleSheet("background-color: #D9D9D9; border: none; border-radius: 14px;");

    // Подключаем обработчик сообщений
    connect(m_socket, &QWebSocket::textMessageReceived, this, &userChat::onTextMessageReceived);

    // Подключаем сигнал из Authentication к слоту в UserChat
    connect(m_authentication, &Authentication::jsonReceived, this, &userChat::onJsonReceived);
}

userChat::~userChat()
{
    delete ui;
}

void userChat::on_newchatButton_clicked()
{
    // Переход на форму нового чата
    m_stackedWidget->setCurrentIndex(7);
}

void userChat::on_settingsButton_clicked()
{
    // Создание JSON объекта
    QJsonObject settingsButtonObject;

    // Формирование полей JSON объекта
    settingsButtonObject["method"] = "pushSettingsButton";
    settingsButtonObject["sessionId"] = m_sessionId;

    QJsonDocument doc(settingsButtonObject);

    // Отправляем JSON-объект на сервер
    m_socket->sendTextMessage(doc.toJson());

}

void userChat::onTextMessageReceived(QString message)
{
    // Преобразуем полученную строку в объект JSON
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());

    // Получаем значение поля "method" из JSON объекта
    QString method = jsonResponse.object().value("method").toString();

    // Получаем значение поля "sessionId" из JSON объекта
    QString serverSessionId = jsonResponse.object().value("sessionId").toString();

    // Проверяем значение поля "method"
    if (method == "updateNameSurnameProfile") {
        // Получаем значения полей "name", "surname" и "profile_picture" из JSON объекта
        QString name = QString::fromUtf8(jsonResponse.object().value("data").toObject().value("name").toString().toUtf8());
        QString surname = QString::fromUtf8(jsonResponse.object().value("data").toObject().value("surname").toString().toUtf8());
        QString base64ImageData = jsonResponse.object().value("data").toObject().value("profile_picture").toString();

        // Создаем JSON объект для отправки
        QJsonObject jsonObject;

        // Формируем поля JSON объекта
        jsonObject["name"] = name;
        jsonObject["surname"] = surname;
        jsonObject["base64ImageData"] = base64ImageData;

        // Создаем JSON документ из JSON объекта
        QJsonDocument doc(jsonObject);

        // Отправляем JSON объект сигналом
        emit jsonReceived(jsonObject);

        // Переключаемся на форму пользовательских настроек
        m_stackedWidget->setCurrentIndex(5);
    }
    else if (serverSessionId == m_sessionId) {
        // Сессионные идентификаторы соответствуют
        // Продолжаем обработку сообщения
    }
    else {
        // Сессионные идентификаторы не совпадают
        // Выводим ошибку
        QMessageBox::critical(this, "Ошибка", "Неверный сессионный идентификатор");
    }
}

void userChat::initializeFonts()
{
    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/Jura.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);

    // Создание экземпляров шрифтов
    QFont Jura(fontFamily, 12);
    QFont Jura_10(fontFamily, 10);
    QFont Jura_8(fontFamily, 8);

    // Сохранение шрифтов в члены класса
    m_labelFont = Jura_10;
    m_timeLabelFont = Jura_8;
    m_namesurname = Jura;
}

void userChat::onJsonReceived(QJsonObject json) {
    // Получение массива "chat_list"
    QJsonArray chatList = json.value("data").toObject().value("chat_list").toArray();

    // Очищаем список перед заполнением новыми элементами
    ui->listWidget->clear();

    // Перебор элементов массива "chat_list"
    for (int i = 0; i < chatList.size(); i++) {
        QJsonObject chatObj = chatList.at(i).toObject();
        QString chatId = chatObj.value("chat_id").toString();

        // Получение массива "members"
        QJsonArray membersList = chatObj.value("members").toArray();

        // Перебор элементов массива "members"
        for (int j = 0; j < membersList.size(); j++) {
            QJsonObject memberObj = membersList.at(j).toObject();
            QString name = memberObj.value("name").toString();
            QString surname = memberObj.value("surname").toString();
            QString telephone = memberObj.value("telephone_number").toString();
            QString status = memberObj.value("status").toString();

            // Создание нового элемента списка
            QListWidgetItem* item = new QListWidgetItem();

            // Создание горизонтального контейнера для хранения изображения профиля и имени/фамилии
            QGridLayout* mainLayout = new QGridLayout();

            // Создание горизонтального контейнера для хранения изображения профиля
            QHBoxLayout* profileLayout = new QHBoxLayout();

            // Получение фото профиля, если оно есть
            if (memberObj.contains("profile_image")) {
                QString profileImage = memberObj.value("profile_image").toString();
                if (profileImage == "zaglushka") {
                    // Вывод заглушки
                    QPixmap placeholder(":/img/img/zaglushka.png");
                    QLabel* placeholderLabel = new QLabel();
                    placeholderLabel->setPixmap(placeholder.scaledToHeight(50));
                    profileLayout->addWidget(placeholderLabel);
                } else {
                    // Преобразование строки изображения в объект QPixmap
                    QByteArray imageData = QByteArray::fromBase64(profileImage.toLatin1());
                    QPixmap pixmap;
                    pixmap.loadFromData(imageData);

                    // Создание QLabel для фото профиля
                    QLabel* profileImageLabel = new QLabel();
                    profileImageLabel->setPixmap(pixmap.scaledToHeight(50));
                    profileLayout->addWidget(profileImageLabel);
                }
            }

            // Создание QLabel для имени и фамилии
            QLabel* nameLabel = new QLabel(name + " " + surname);
            nameLabel->setFont(m_namesurname);
            nameLabel->setContentsMargins(0, 0, 0, 0); // Установка отступов внутри QLabel

            // Создание QLabel для номера телефона
            QLabel* telephoneLabel = new QLabel(telephone);
            telephoneLabel->setFont(m_timeLabelFont);
            telephoneLabel->setContentsMargins(0, 0, 0, 0); // Установка отступов внутри QLabel

            // Добавление элементов в QGridLayout
            mainLayout->addLayout(profileLayout, 0, 0, 2, 1, Qt::AlignTop); // Размещение profileLayout в первой колонке первой и второй строки, выравнивание по верхней границе
            mainLayout->addWidget(nameLabel, 0, 1, Qt::AlignLeft | Qt::AlignTop); // Размещение nameLabel во второй колонке первой строки, выравнивание по левой границе и по верхней границе
            mainLayout->addWidget(telephoneLabel, 1, 1, Qt::AlignLeft | Qt::AlignTop); // Размещение telephoneLabel во второй колонке второй строки, выравнивание по левой границе и по верхней границе
            mainLayout->setColumnStretch(1, 1); // Растягивание второй колонки для заполнения доступного пространства
            mainLayout->setHorizontalSpacing(10); // Установка горизонтального расстояния между элементами
            mainLayout->setVerticalSpacing(10); // Установка вертикального расстояния между элементами
            mainLayout->setContentsMargins(10, 10, 10, 10); // Установка отступов внутри контейнера

            // Создание QWidget для установки mainLayout
            QWidget* widget = new QWidget();
            widget->setLayout(mainLayout);
            item->setSizeHint(widget->sizeHint());
            ui->listWidget->addItem(item);
            ui->listWidget->setItemWidget(item, widget);

            // Скрытие chat_id в пользовательских данных элемента списка
            item->setData(Qt::UserRole, chatId);
            item->setData(Qt::UserRole + 1, name);
            item->setData(Qt::UserRole + 2, surname);
            item->setData(Qt::UserRole + 3, telephone);
            item->setData(Qt::UserRole + 4, status);
            item->setData(Qt::UserRole + 5, memberObj.value("profile_image").toString());
        }
    }
}

// Слот для обработки нажатия на элемент списка listWidget
void userChat::on_listWidget_itemClicked(QListWidgetItem *item)
{
    // Получаем chat_id из скрытых пользовательских данных выбранного элемента
    QString chatId = item->data(Qt::UserRole).toString();
    QString name = item->data(Qt::UserRole + 1).toString();
    QString surname = item->data(Qt::UserRole + 2).toString();
    QString telephone = item->data(Qt::UserRole + 3).toString();
    QString status = item->data(Qt::UserRole + 4).toString();
    QString image = item->data(Qt::UserRole + 5).toString();

    // Создание JSON объекта
    QJsonObject jsonObject;

    // Формирование полей JSON объекта
    jsonObject["method"] = "messageOutput";
    jsonObject["sessionId"] = m_sessionId;
    jsonObject["chat_id"] = chatId;
    jsonObject["name"] = name;
    jsonObject["surname"] = surname;
    jsonObject["status"] = status;
    jsonObject["telephone_number"] = telephone;
    jsonObject["profile_image"] = image;

    QJsonDocument doc(jsonObject);

    // Отправляем JSON-объект на сервер
    m_socket->sendTextMessage(doc.toJson());

    // Отправка JSON объекта
    emit jsonReceived(jsonObject);

    // Выполняем переход на форму chat
    m_stackedWidget->setCurrentIndex(6);
}
