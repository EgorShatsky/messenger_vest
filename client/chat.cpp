#include "chat.h"
#include "ui_chat.h"
#include "client.h"
#include "createchat.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFontDatabase>

Chat::Chat(QWebSocket* socket, QString sessionId, userChat*uchat, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Chat)
    , m_socket(socket)
    , m_sessionId(sessionId)
    , m_stackedWidget(dynamic_cast<Client*>(parent)->getStackedWidget())
    , m_userChat(uchat)
{
    ui->setupUi(this);

    // Инициализация шрифтов
    initializeFonts();

    //Установка фона для кнопки назад
    QPixmap backPixmap(":/img/img/arrowback.png");
    ui->backButton->setIcon(QIcon(backPixmap));
    ui->backButton->setIconSize(backPixmap.size());

    //Установка фона для кнопки поиска сообщений
    QPixmap findPixmap(":/img/img/lupa.png");
    ui->findButton->setIcon(QIcon(findPixmap));
    ui->findButton->setIconSize(findPixmap.size());

    //Установка фона для кнопки прикрепления файла
    QPixmap addFilePixmap(":/img/img/screpka.png");
    ui->addFileButton->setIcon(QIcon(addFilePixmap));
    ui->addFileButton->setIconSize(addFilePixmap.size());

    //Установка фона для кнопки отрпвки сообщения
    QPixmap sendPixmap(":/img/img/send.png");
    ui->sendButton->setIcon(QIcon(sendPixmap));
    ui->sendButton->setIconSize(sendPixmap.size());

    //Установка фона для фрейма
    ui->frame_2->setStyleSheet("background-color: #D9D9D9;");
    ui->frame->setStyleSheet("background-color: #D9D9D9;");

    // Ставим кнопкам цвет и убираем бордер, чтобы не было тени и сливались с фоном
    ui->backButton->setStyleSheet("background-color: #D9D9D9; border: none;");
    ui->findButton->setStyleSheet("background-color: #D9D9D9; border: none;");
    ui->addFileButton->setStyleSheet("background-color: #D9D9D9; border: none;");
    ui->sendButton->setStyleSheet("background-color: #D9D9D9; border: none;");

    ui->messageLineEdit->setPlaceholderText("Введите текст сообщения...");
    ui->messageLineEdit->setFont(m_labelFont);
    ui->messageLineEdit->setStyleSheet("background-color:#FFFFFF;");

    ui->messageListWidget->setStyleSheet("QListWidget {padding: 10px; background-color: #FFFFFF; border: none;}");

    // Подключаем обработчик сообщений
    connect(m_socket, &QWebSocket::textMessageReceived, this, &Chat::onTextMessageReceived);

    // Подключаем сигнал из Authentication к слоту в UserChat
    connect(m_userChat, &userChat::jsonReceived, this, &Chat::onJsonReceived);
}

Chat::~Chat()
{
    delete ui;
}

void Chat::on_sendButton_clicked()
{
    // Получаем текст из messageLineEdit
    QString text = ui->messageLineEdit->text();
    if(text.isEmpty()) return;

    // Получение текущей даты и времени
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // Преобразование времени
    QString sendAt = currentDateTime.toString("hh:mm");

    // Создаем новый элемент списка и добавляем в него виджет с текстом сообщения
    QListWidgetItem* item = new QListWidgetItem(ui->messageListWidget);
    QLabel* label = new QLabel(text, ui->messageListWidget);
    QLabel* timeLabel = new QLabel(sendAt, ui->messageListWidget); // Создаем виджет QLabel с временем отправки
    label->setAlignment(Qt::AlignRight);
    timeLabel->setAlignment(Qt::AlignRight);
    label->setWordWrap(true); // Разрешаем перенос слов
    label->adjustSize(); // Подстраиваем размер виджета под текст
    timeLabel->adjustSize();
    item->setSizeHint(label->sizeHint()); // Устанавливаем размер элемента списка
    label->setFont(m_labelFont);

    QVBoxLayout* layout = new QVBoxLayout(); // Создаем вертикальный layout для объединения label и timeLabel
    layout->addWidget(label);
    layout->addWidget(timeLabel);
    layout->setSpacing(0);

    QWidget* widget = new QWidget(ui->messageListWidget); // Создаем виджет-контейнер для объединенного layout
    widget->setLayout(layout);
    widget->adjustSize(); // Подстраиваем размер виджета под содержимое

    item->setSizeHint(widget->sizeHint()); // Устанавливаем размер элемента списка

    ui->messageListWidget->addItem(item); // Добавляем элемент в список
    ui->messageListWidget->setItemWidget(item, widget); // Устанавливаем виджет в элемент списка

    ui->messageListWidget->setSpacing(5);

    // Формируем json-объект и отправляем на сервер
    QJsonObject json;
    json["method"] = "sendMessage";
    json["sessionId"] = m_sessionId;
    json["text"] = text;
    json["chat_id"] = chatId;
    json["telephone_number"] = telephone;

    m_socket->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));

    // Очищаем messageLineEdit
    ui->messageLineEdit->clear();
}

void Chat::on_addFileButton_clicked()
{

}

void Chat::on_findButton_clicked()
{
    // Код для поиска сообщений
}

void Chat::on_backButton_clicked()
{
    // Переход на форму чатов
    m_stackedWidget->setCurrentIndex(4);

    // Очистка формы чатов
    ui->messageListWidget->clear();
}

void Chat::initializeFonts()
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

void Chat::onTextMessageReceived(QString message)
{
    // преобразуем полученную строку в объект JSON
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());

    // получаем значение поля "method" из JSON объекта
    QString method = jsonResponse.object().value("method").toString();
    QString text = jsonResponse.object().value("text").toString();
    QString telephone = jsonResponse.object().value("telephone_number").toString();

    // проверяем значение поля "method"
    if (method == "receiveMessage") {
        // Получение текущей даты и времени
        QDateTime currentDateTime = QDateTime::currentDateTime();

        // Преобразование времени
        QString sendAt = currentDateTime.toString("hh:mm");

        // Создаем новый элемент списка и добавляем в него виджет с текстом сообщения
        QListWidgetItem* item = new QListWidgetItem(ui->messageListWidget);
        QLabel* label = new QLabel(text, ui->messageListWidget);
        QLabel* timeLabel = new QLabel(sendAt, ui->messageListWidget); // Создаем виджет QLabel с временем отправки
        label->setAlignment(Qt::AlignLeft);
        timeLabel->setAlignment(Qt::AlignLeft);
        label->setWordWrap(true); // Разрешаем перенос слов
        label->adjustSize(); // Подстраиваем размер виджета под текст
        timeLabel->adjustSize();
        item->setSizeHint(label->sizeHint()); // Устанавливаем размер элемента списка
        label->setFont(m_labelFont);

        QVBoxLayout* layout = new QVBoxLayout(); // Создаем вертикальный layout для объединения label и timeLabel
        layout->addWidget(label);
        layout->addWidget(timeLabel);
        layout->setSpacing(0);

        QWidget* widget = new QWidget(ui->messageListWidget); // Создаем виджет-контейнер для объединенного layout
        widget->setLayout(layout);
        widget->adjustSize(); // Подстраиваем размер виджета под содержимое

        item->setSizeHint(widget->sizeHint()); // Устанавливаем размер элемента списка

        ui->messageListWidget->addItem(item); // Добавляем элемент в список
        ui->messageListWidget->setItemWidget(item, widget); // Устанавливаем виджет в элемент списка

        ui->messageListWidget->setSpacing(5);

    }else if (method == "messageFromDb") {

        // получаем список сообщений из JSON объекта
        QJsonArray messages = jsonResponse.object().value("data").toObject().value("messages").toArray();

        // перебираем все сообщения и отображаем их в QListWidget
        for (int i = 0; i < messages.size(); i++) {
            QJsonObject message = messages.at(i).toObject();
            QString senderId = message.value("sender_id").toString();
            //QString recipientId = message.value("recipient_id").toString();
            QString text = message.value("text").toString();
            QString dateTimeString = message.value("sent_at").toString();

            //преобразование времени
            QDateTime dateTime = QDateTime::fromString(dateTimeString, "ddd MMM d hh:mm:ss yyyy");
            QString sendAt = dateTime.toString("hh:mm");

            QString clientId = message.value("client_id").toString();

            // Создаем новый элемент списка и добавляем в него виджет с текстом сообщения
            QListWidgetItem* item = new QListWidgetItem(ui->messageListWidget);
            QLabel* label = new QLabel(text, ui->messageListWidget);
            QLabel* timeLabel = new QLabel(sendAt, ui->messageListWidget); // Создаем виджет QLabel с временем отправки
            label->setWordWrap(true); // Разрешаем перенос слов
            label->adjustSize(); // Подстраиваем размер виджета под текст
            item->setSizeHint(label->sizeHint()); // Устанавливаем размер элемента списка

            // Устанавливаем сохраненные шрифты
            label->setFont(m_labelFont);
            timeLabel->setFont(m_timeLabelFont);

            // выравниваем текст по левому или правому краю в зависимости от значения senderId
            if (clientId == senderId) {
                label->setAlignment(Qt::AlignRight);
                timeLabel->setAlignment(Qt::AlignRight); // Выравниваем время по левому краю
            } else {
                label->setAlignment(Qt::AlignLeft);
                timeLabel->setAlignment(Qt::AlignLeft); // Выравниваем время по правому краю
            }

            QVBoxLayout* layout = new QVBoxLayout(); // Создаем вертикальный layout для объединения label и timeLabel
            layout->addWidget(label);
            layout->addWidget(timeLabel);
            layout->setSpacing(0);

            QWidget* widget = new QWidget(ui->messageListWidget); // Создаем виджет-контейнер для объединенного layout
            widget->setLayout(layout);
            widget->adjustSize(); // Подстраиваем размер виджета под содержимое

            item->setSizeHint(widget->sizeHint()); // Устанавливаем размер элемента списка

            ui->messageListWidget->addItem(item); // Добавляем элемент в список
            ui->messageListWidget->setItemWidget(item, widget); // Устанавливаем виджет в элемент списка
        }
    }else if (method == "noMessages") {

    }
}

void Chat::onJsonReceived(QJsonObject json)
{
    chatId = json.value("chat_id").toString();
    QString name = json.value("name").toString();
    QString surname = json.value("surname").toString();
    QString status = json.value("status").toString();
    telephone = json.value("telephone_number").toString();
    QString profileImage = json.value("profile_image").toString();

    if (profileImage == "zaglushka") {
        // Вывод заглушки
        QPixmap placeholder(":/img/img/zaglushka.png");
        ui->photoLabel->setPixmap(placeholder.scaledToHeight(50));

    } else {
        // Преобразование строки изображения в объект QPixmap
        QByteArray imageData = QByteArray::fromBase64(profileImage.toLatin1());
        QPixmap pixmap;
        pixmap.loadFromData(imageData);
        ui->photoLabel->setPixmap(pixmap.scaledToHeight(50));
    }

    if (status == "true") {
        ui->statusLabel->setText("online");
        ui->statusLabel->setFont(m_timeLabelFont);
    } else {
        ui->statusLabel->setText("offline");
        ui->statusLabel->setFont(m_timeLabelFont);
    }
    ui->statusLabel->adjustSize(); // подстраиваем размер под текст
    ui->nameLabel->setFont(m_namesurname);

    // Создаем экземпляр QGridLayout
    QGridLayout* layout = new QGridLayout();

    // Объединяем name и surname
    QString fullName = name + " " + surname;

    ui->nameLabel->setText(fullName); // Устанавливаем объединенное имя и фамилию
    ui->nameLabel->adjustSize();

    // Добавляем QLabel в QGridLayout
    layout->addWidget(ui->photoLabel, 0, 0, 2, 1, Qt::AlignTop); // ui->photoLabel в первой колонке первой и второй строки, выравнивание по верхней границе
    layout->addWidget(ui->nameLabel, 0, 1, 1, 2, Qt::AlignLeft ); // ui->nameLabel во второй и третьей колонке первой строки, выравнивание по левой границе и по верхней границе
    layout->addWidget(ui->statusLabel, 1, 1, 1, 2,  Qt::AlignLeft); // ui->statusLabel во второй и третьей колонке второй строки, выравнивание по левой границе и по верхней границе

    layout->setColumnStretch(1, 1); // Растягивание третьей колонки для заполнения доступного пространства

    layout->setHorizontalSpacing(15); // Установка горизонтального расстояния между элементами
    layout->setVerticalSpacing(5); // Установка вертикального расстояния между элементами
    layout->setContentsMargins(0, 0, 0, 0); // Установка отступов внутри контейнера

    // Удаляем существующую компоновку, если она есть
    QLayout* existingLayout = ui->frame_3->layout();
    if (existingLayout) {
        delete existingLayout;
    }

    // Устанавливаем QGridLayout для вашего виджета (например, QWidget или QFrame)
    ui->frame_3->setLayout(layout);
}

