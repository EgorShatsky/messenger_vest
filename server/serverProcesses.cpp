#include <QSqlQuery>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>
#include <QtSql>
#include <QtNetwork>
#include <QPixmap>
#include "server.h"
#include <QtWebSockets/QWebSocket>

void Server::inputPhoneNumber(const QString& phoneNumber)
{
    // Обработка номера телефона
    qDebug() << "Номер телефона:" << phoneNumber;

    int code = QRandomGenerator::global()->bounded(1000, 10000); // генерация 4-х значного кода
    qDebug() << "Код входа для пользователя: " << code;

    // Отправляем SMS
    // Бесплатно 5 штук в день
    //sendSms(phoneNumber, QString::number(code));

    //проверяем существует ли запись о введенном номере телефона в БД
    QSqlQuery query;
    query.prepare("SELECT user_id, verification_code_id FROM users WHERE telephone_number = :phone");
    query.bindValue(":phone", phoneNumber);

    if (query.exec() && query.first()) {
        // Номер телефона уже существует в базе данных
        int userId = query.value("user_id").toInt();
        int verificationCodeId = query.value("verification_code_id").toInt();

        // Удаление записи из таблицы предыдщуий verification_code
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM verification_code WHERE id = :verification_code_id");
        deleteQuery.bindValue(":verification_code_id", verificationCodeId);

        if (deleteQuery.exec()) {
            qDebug() << "Запись успешно удалена из таблицы verification_code";
        } else {
            qWarning() << "Ошибка при удалении записи из таблицы verification_code: " << deleteQuery.lastError().text();
        }
        // Обновление записи в таблице users
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE users SET verification_code_id = NULL WHERE user_id = :user_id");
        updateQuery.bindValue(":user_id", userId);

        if (updateQuery.exec()) {
            qDebug() << "Запись успешно обновлена в таблице users";
        } else {
            qWarning() << "Ошибка при обновлении записи в таблице users: " << updateQuery.lastError().text();
        }
    } else {
        // Номер телефона не существует в базе данных
        query.prepare("INSERT INTO users (telephone_number) VALUES (:phone) RETURNING user_id");
        query.bindValue(":phone", phoneNumber);

        if (query.exec() && query.first()) {
            int userId = query.value("user_id").toInt();
            qDebug() << "Номер телефона сохранен в базе данных, ID пользователя: " << userId;
        } else {
            qWarning() << "Ошибка при сохранении номера телефона в базу данных: " << query.lastError().text();
        }
    }

    // Генерация даты создания кода
    QDateTime dateTime = QDateTime::currentDateTime(); // Получаем текущую дату и время
    QString dateTimeStr = dateTime.toString(Qt::ISODate); // Преобразуем дату и время в строку в формате ISO

    // Получение ID последней вставленной записи в таблицу users
    int userId = query.value("user_id").toInt();
    qDebug() << "Последний вставленный айди: " << userId;

    // Сохранение кода подтверждения в базу данных
    QSqlQuery query2;
    query2.prepare("INSERT INTO verification_code (code, created_at, user_id, used) VALUES (:code, :created_at, :user_id, :used)");
    query2.bindValue(":code", QString::number(code));
    query2.bindValue(":created_at", dateTimeStr);
    query2.bindValue(":user_id", userId);
    query2.bindValue(":used", false);

    if (query2.exec()) {
        qDebug() << "Код подтверждения сохранен в базу данных";
        int verificationCodeId = query2.lastInsertId().toInt();
        qDebug() << "ID кода подтверждения: " << verificationCodeId;

        // Обновление записи в таблице users
        QSqlQuery query3;
        query3.prepare("UPDATE users SET verification_code_id = :verification_code_id WHERE user_id = :user_id");
        query3.bindValue(":verification_code_id", verificationCodeId);
        query3.bindValue(":user_id", userId);

        if (query3.exec()) {
            qDebug() << "ID кода подтверждения сохранен в таблице users";
        } else {
            qWarning() << "Ошибка при сохранении ID кода подтверждения в таблице users: " << query3.lastError().text();
        }

    } else {
        qWarning() << "Ошибка при сохранении кода подтверждения в базу данных: " << query2.lastError().text();
    }
}

void Server::inputCode(const QString &code, const QString &sessionId)
{
    QString phoneNumber = m_phoneNumbers.value(sessionId);
    QSqlQuery query;
    query.prepare("SELECT verification_code.code, verification_code.created_at FROM users JOIN verification_code ON users.verification_code_id = verification_code.id WHERE users.telephone_number = :phone AND verification_code.used = 'false'");
    query.bindValue(":phone", phoneNumber);
    if (query.exec() && query.next()) {
        QString codeFromDataBase = query.value("code").toString();

        QDateTime createdAt = query.value("created_at").toDateTime();
        QDateTime now = QDateTime::currentDateTime();
        // Сравниваем code из аргументов метода и из БД
        if (codeFromDataBase.compare(code) == 0) {
            // Коды совпадают
            // Проверяем, прошло ли 2 минуты с момента создания кода
            if (createdAt.secsTo(now) <= 120) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE verification_code SET used = 'true' WHERE code = :code");
                updateQuery.bindValue(":code",codeFromDataBase);
                // Изменяем значение в базе данных
                updateQuery.exec();

                QSqlQuery queryIsEmpty;
                queryIsEmpty.prepare("SELECT password, name, surname FROM users WHERE telephone_number = :phone");
                queryIsEmpty.bindValue(":phone", phoneNumber);
                if (queryIsEmpty.exec() && queryIsEmpty.next()) {
                    QString password = queryIsEmpty.value("password").toString();
                    QString name = queryIsEmpty.value("name").toString();
                    QString surname = queryIsEmpty.value("surname").toString();
                    if (password.isEmpty() || name.isEmpty() || surname.isEmpty()) {
                        sendJsonResponse("registrationPage", sessionId);
                    } else {
                        sendJsonResponse("authenticationPage", sessionId);
                    }
                } else {
                    // Обработка ошибки
                    QMessageBox::warning(this, "Ошибка!", "Произошла ошибка при выполнении операции. Пожалуйста, попробуйте еще раз");
                }
            } else {
                // Коды совпадают, но прошло более 2 минут
                sendJsonResponse("repeatCode", sessionId);
            }

        } else {
            // Коды не совпадают
            // Отправляем ошибку
            sendJsonResponse("errorCode", sessionId);
        }

    } else {
        // Обработка ошибки
        QMessageBox::warning(this, "Ошибка!", "Произошла ошибка при выполнении операции. Пожалуйста, попробуйте еще раз");
    }
}

void Server::inputPassword(const QString &password, const QString &sessionId) {
    // Необходимо проверить пароль
    QString phoneNumber = m_phoneNumbers.value(sessionId); // Получаем номер телефона по идентификатору сессии

    int userId = getUserIdByPhoneNumber(phoneNumber);
    qDebug() << "Id пользователя, который отправил запрос:" << userId;

    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE user_id = :userId");
    query.bindValue(":userId", userId);
    if (query.exec() && query.first()) {
        QString dbPassword = query.value("password").toString();

        // Проверка пароля
        if (password.compare(dbPassword) == 0) {
            // Пароли совпадают, формируем объект JSON со списком чатов
            QMap<QString, QVariantList> chatList = getChatList(userId);
            QJsonObject json;
            QJsonArray chatListJson;
            for (auto chatId : chatList.keys()) {
                QVariantList chatData = chatList[chatId];
                QJsonArray membersJson;
                for (auto memberData : chatData) {
                    QVariantMap memberMap = memberData.toMap();
                    QJsonObject memberJson;
                    memberJson["name"] = memberMap["name"].toString();
                    memberJson["surname"] = memberMap["surname"].toString();
                    memberJson["status"] = memberMap["status"].toString();
                    memberJson["telephone_number"] = memberMap["telephone_number"].toString();

                    int profilePictureId = memberMap["profile_picture_id"].toInt();
                    if (profilePictureId != 0) {
                        QSqlQuery queryProfilePicture;
                        queryProfilePicture.prepare("SELECT file_path FROM file_storage WHERE file_id = :file_id");
                        queryProfilePicture.bindValue(":file_id", profilePictureId);
                        if (queryProfilePicture.exec() && queryProfilePicture.next()) {
                            QString filePath = queryProfilePicture.value("file_path").toString();

                            // Загружаем фото профиля и добавляем его в JSON объект
                            QFile profilePictureFile(filePath);
                            if (profilePictureFile.open(QIODevice::ReadOnly)) {
                                QByteArray imageData = profilePictureFile.readAll();
                                QString base64ImageData = QString::fromUtf8(imageData.toBase64());
                                memberJson["profile_image"] = base64ImageData;
                                profilePictureFile.close();
                            }
                        }
                    } else {
                        memberJson["profile_image"] = "zaglushka";
                    }

                    membersJson.append(memberJson);
                }
                QJsonObject chatJson;
                chatJson["chat_id"] = chatId;
                chatJson["name"] = chatData[0].toString();
                chatJson["members"] = membersJson;
                chatListJson.append(chatJson);
            }
            json["chat_list"] = chatListJson;

            // Отправляем JSON объект на клиент
            sendJsonObject("successfulEntry", sessionId, json);
        } else {
            // Пароль неверный, выводим сообщение об ошибке
            sendJsonResponse("loginFailed", sessionId);
        }
    }
}


void Server::inputUserData(const QString& name, const QString& surname, const QString& password,const QString& sessionId)
{
    QString phoneNumber = m_phoneNumbers.value(sessionId); // получаем номер телефона по идентификатору сессии

    QSqlQuery query;
    query.prepare("UPDATE users SET name = :name, surname = :surname, password = :password WHERE telephone_number = :phone");
    query.bindValue(":phone", phoneNumber);
    query.bindValue(":name", name);
    query.bindValue(":surname", surname);
    query.bindValue(":password", password);

    if (query.exec()) {
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE users SET status = 'true' WHERE telephone_number = :phone");
        updateQuery.bindValue(":phone", phoneNumber);
        if (updateQuery.exec()) {
            //Вы зарегистрированы!
            sendJsonResponse("loginSuccessful", sessionId);
        } else {
            //Не удалось обновить статус пользователя
            sendJsonResponse("statusError", sessionId);
        }
    } else {
        //Пользователь не зарегистрирован(
        sendJsonResponse("registrationFailed", sessionId);
    }
}


void Server::findPhone(const QString& searchPhone, const QString& sessionId)
{
    // получаем номер телефона по идентификатору сессии
    QString phoneNumber = m_phoneNumbers.value(sessionId);
    qDebug() << "Номер телефона пользователя, который отправил запрос:" <<phoneNumber;

    int userId = getUserIdByPhoneNumber(phoneNumber);
    qDebug() << "Id пользователя, который отправил запрос:" <<userId;

    int searchUserId = getUserIdByPhoneNumber(searchPhone);
    qDebug() << "Id пользователя с которым создается чат:" <<searchUserId;

    // Проверяем существование номера телефона в таблице users
    QSqlQuery checkPhoneQuery;
    checkPhoneQuery.prepare("SELECT user_id FROM users WHERE telephone_number = :telephone_number");
    checkPhoneQuery.bindValue(":telephone_number", searchPhone);
    if (!checkPhoneQuery.exec()) {
        //sendJsonResponse("doesNotExists", sessionId);
    }

    if (checkPhoneQuery.next()) {
        //int foundUserId = checkPhoneQuery.value("user_id").toInt();
        if (searchPhone == phoneNumber || searchUserId == userId) {
            // Проверка на создание чата с самим собой
            // Если номера совпадают, то сообщаем на клиент
            sendJsonResponse("errorPhoneNumber", sessionId);
        } else{
            // Проверяем существование чата с данным пользователем
            QSqlQuery checkChatQuery;
            checkChatQuery.prepare("SELECT chat_id FROM chat_members WHERE member_id = :member_id AND chat_id IN (SELECT chat_id FROM chat_members WHERE member_id = :search_user_id)");
            checkChatQuery.bindValue(":member_id", userId);
            checkChatQuery.bindValue(":search_user_id", searchUserId);
            if (!checkChatQuery.exec()) {
                qDebug() << "Ошибка при выполнении запроса:" << checkChatQuery.lastError().text();
                return;
            }

            if (checkChatQuery.next()) {
                // Если чат уже существует, то сообщаем на клиент
                sendJsonResponse("alreadyExists", sessionId);
            } else {
                // Генерация даты создания чата
                QDateTime dateTime = QDateTime::currentDateTime(); // Получаем текущую дату и время
                QString dateTimeStr = dateTime.toString(Qt::ISODate); // Преобразуем дату и время в строку в формате ISO

                QSqlQuery insertChatQuery;
                insertChatQuery.prepare("INSERT INTO chats (chat_type, created_at, creator_id) VALUES ('single', :created_at, :creator_id)");
                insertChatQuery.bindValue(":created_at", dateTimeStr);
                insertChatQuery.bindValue(":creator_id", userId);

                if (!insertChatQuery.exec()) {
                    qDebug() << "Ошибка при создании записи в таблице chats:" << insertChatQuery.lastError().text();
                    return;
                }

                // id созданного чата
                int chatId = insertChatQuery.lastInsertId().toInt();

                // Добавление участников в чат
                QSqlQuery insertChatMembersQuery;
                insertChatMembersQuery.prepare("INSERT INTO chat_members (chat_id, member_id) VALUES (:chat_id, :member_id), (:chat_id, :search_member_id)");
                insertChatMembersQuery.bindValue(":chat_id", chatId);
                insertChatMembersQuery.bindValue(":member_id", userId);
                insertChatMembersQuery.bindValue(":search_member_id", searchUserId);
                if (!insertChatMembersQuery.exec()) {
                    qDebug() << "Ошибка при добавлении участников чата:" << insertChatMembersQuery.lastError().text();
                    return;
                }

                // Отправка информации о новом чате на клиент
                QSqlQuery queryPhone;
                queryPhone.prepare("SELECT name, surname, status FROM users WHERE telephone_number = :searchPhone");
                queryPhone.bindValue(":searchPhone", searchPhone);
                QJsonObject userMessage;
                if (queryPhone.exec() && queryPhone.next()) {
                    QString name = queryPhone.value("name").toString();
                    QString surname = queryPhone.value("surname").toString();
                    QString status = queryPhone.value("status").toString();
                    userMessage["name"] = name;
                    userMessage["surname"] = surname;
                    userMessage["status"] = status;
                    userMessage["chat_id"] = chatId;
                    //отправка на клиент
                    sendJsonObject("createChat", sessionId, userMessage);
                }

            }
        }
    }else {
        // Если пользователь с данным номером телефона не найден, то сообщаем на клиент
        sendJsonResponse("doesNotExists", sessionId);
    }
}

void Server::messageOutput(const QString&chatId,const QString&sessionId)
{
    // получаем номер телефона по идентификатору сессии
    QString phoneNumber = m_phoneNumbers.value(sessionId);

    // Получаем user_id отправителя
    QString clientId = "";
    QSqlQuery querySender;
    querySender.prepare("SELECT user_id FROM users WHERE telephone_number = :telephone_number");
    querySender.bindValue(":telephone_number", phoneNumber);
    if (querySender.exec() && querySender.next()) {
        clientId = querySender.value("user_id").toString();
    } else {
        qDebug() << "Ошибка при получении user_id отправителя из базы данных";
    }


    // Формируем запрос к базе данных
    QSqlQuery query;
    query.prepare("SELECT * FROM messages WHERE chat_id = :chatId");
    query.bindValue(":chatId", chatId);

    // Выполняем запрос
    if (!query.exec()) {
        qDebug() << "Ошибка извлечения сообщений из базы данных:" << query.lastError().text();
        return;
    }

    // Создаем json объект для хранения информации о сообщениях
    QJsonObject messagesObject;

    // Создаем массив для хранения информации о каждом сообщении
    QJsonArray messagesArray;

    // Перебираем результаты запроса
    while (query.next()) {
        // Создаем объект для хранения информации о текущем сообщении
        QJsonObject messageObject;

        // Добавляем информацию о текущем сообщении в объект
        messageObject.insert("client_id", clientId);
        messageObject.insert("sender_id", query.value("sender_id").toString());
        messageObject.insert("recipient_id", query.value("recipient_id").toString());
        messageObject.insert("text", query.value("message_text").toString());
        messageObject.insert("sent_at", query.value("sent_at").toDateTime().toString());

        // Добавляем объект с информацией о текущем сообщении в массив
        messagesArray.append(messageObject);
    }

    // Если массив сообщений не пустой, то добавляем его в json объект
    if (!messagesArray.isEmpty()) {
        messagesObject.insert("messages", messagesArray);
        // Создаем json документ на основе json объект
        sendJsonObject("messageFromDb",sessionId,messagesObject);
    } else {
        // Если массив сообщений пустой, то отправляем noMessages
        sendJsonResponse("noMessages", sessionId);
    }
}

void Server::sendMessage(const QString& chatId, const QString& telephone, const QString& text, const QString& sessionId)
{
    // получаем номер телефона по идентификатору сессии
    QString phoneNumber = m_phoneNumbers.value(sessionId);

    // Получаем user_id отправителя
    int senderId = -1;
    QSqlQuery querySender;
    querySender.prepare("SELECT user_id FROM users WHERE telephone_number = :telephone_number");
    querySender.bindValue(":telephone_number", phoneNumber);
    if (querySender.exec() && querySender.next()) {
        senderId = querySender.value("user_id").toInt();
    } else {
        qDebug() << "Ошибка при получении user_id отправителя из базы данных";
    }

    // Получаем user_id получателя
    int recipientId = -1;
    QSqlQuery queryRecipient;
    queryRecipient.prepare("SELECT user_id FROM users WHERE telephone_number = :telephone_number");
    queryRecipient.bindValue(":telephone_number", telephone);
    if (queryRecipient.exec() && queryRecipient.next()) {
        recipientId = queryRecipient.value("user_id").toInt();
    } else {
        qDebug() << "Ошибка при получении user_id получателя из базы данных";
    }

    // Получаем sessionId получателя
    QString receiverSessionId;
    for (auto iter = m_phoneNumbers.constBegin(); iter != m_phoneNumbers.constEnd(); ++iter) {
        if (iter.value() == telephone) {
            receiverSessionId = iter.key();
            break;
        }
    }

    // Записываем сообщение в базу данных
    QSqlQuery query;
    query.prepare("INSERT INTO messages (chat_id, sender_id, recipient_id, message_text, message_type, sent_at, updated_at, is_read) "
                  "VALUES (:chat_id, :sender_id, :recipient_id, :message_text, :message_type, :sent_at, :updated_at, :is_read)");
    query.bindValue(":chat_id", chatId);
    query.bindValue(":sender_id", senderId);
    query.bindValue(":recipient_id", recipientId);
    query.bindValue(":message_text", text);
    query.bindValue(":message_type", "text");
    query.bindValue(":sent_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":updated_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":is_read", false);

    if (!query.exec()) {
        qDebug() << "Ошибка записи сообщения в базу данных:" << query.lastError().text();
    }

    // Проверяем, находится ли пользователь с этим sessionId в онлайн-списке m_connections
    if (m_connections.values().contains(receiverSessionId)) {
        // Получаем указатель на сокет получателя
        QWebSocket* receiverSocket = nullptr;
        for (auto iter = m_connections.begin(); iter != m_connections.end(); ++iter) {
            if (iter.value() == receiverSessionId) {
                receiverSocket = iter.key();
                break;
            }
        }
        if (receiverSocket) {
            // Отправляем сообщение получателю
            QJsonObject messageObject;

            // Формирование полей JSON объекта
            messageObject["method"] = "receiveMessage";
            messageObject["text"] = text;
            messageObject["telephone_number"] = telephone;
            messageObject["chat_id"] = chatId.toInt();
            messageObject["sessionId"] = receiverSessionId;
            QJsonDocument messageDoc(messageObject);
            receiverSocket->sendTextMessage(messageDoc.toJson(QJsonDocument::Compact));
        } else {
            // Если получатель не найден, выводим сообщение об ошибке
            qDebug() << "Ошибка: сокет получателя не найден";
        }
    } else {
        // Если получатель оффлайн, выводим сообщение об этом
        qDebug() << "Пользователь с sessionId: " << receiverSessionId << " не в сети";
    }
}


// Метод getChatList(int userId) возвращает список чатов, в которых участвует пользователь с идентификатором userId.
// Метод возвращает данные в формате QMap<QString, QVariantList>
QMap<QString, QVariantList> Server::getChatList(int userId) {
    QMap<QString, QVariantList> chatList;

    // Получаем список чатов, в которых участвует пользователь
    QSqlQuery query;
    query.prepare("SELECT DISTINCT chat_id FROM chat_members WHERE member_id = :userId");
    query.bindValue(":userId", userId);
    if (query.exec()) {
        while (query.next()) {
            QString chatId = query.value("chat_id").toString();
            QVariantList chatData;

            // Получаем информацию об участниках чата, кроме текущего пользователя
            QSqlQuery memberQuery;
            memberQuery.prepare("SELECT u.name, u.surname, u.status, u.telephone_number, u.profile_picture_id FROM users u INNER JOIN chat_members cm ON u.user_id = cm.member_id WHERE cm.chat_id = :chatId AND cm.member_id != :userId");
            memberQuery.bindValue(":chatId", chatId);
            memberQuery.bindValue(":userId", userId);
            if (memberQuery.exec()) {
                while (memberQuery.next()) {
                    QVariantMap memberData;
                    memberData.insert("name", memberQuery.value("name"));
                    memberData.insert("surname", memberQuery.value("surname"));
                    memberData.insert("status", memberQuery.value("status"));
                    memberData.insert("telephone_number", memberQuery.value("telephone_number"));
                    memberData.insert("profile_picture_id", memberQuery.value("profile_picture_id"));
                    chatData.append(QVariant::fromValue(memberData));
                }
            }
            chatList.insert(chatId, chatData);
        }
    }
    return chatList;
}
//Chat ID: 2
//Members:
//    Name: John
//    Surname: Doe
//    Status: true

void Server::inputUserData(const QString&name, const QString&surname, const QString&password, const QString&phone, const QString&sessionId)
{

    // получаем номер телефона по идентификатору сессии
    QString phoneNumber = m_phoneNumbers.value(sessionId);

    // Получаем user_id отправителя
    int senderId = -1;
    QSqlQuery querySender;
    querySender.prepare("SELECT user_id FROM users WHERE telephone_number = :telephone_number");
    querySender.bindValue(":telephone_number", phoneNumber);
    if (querySender.exec() && querySender.next()) {
        senderId = querySender.value("user_id").toInt();
    } else {
        qDebug() << "Ошибка при получении user_id отправителя из базы данных";
        sendJsonResponse("errorDataChange", sessionId);
    }

    // Формируем и выполняем запросы на обновление данных
    if (!name.isEmpty()) {
        QSqlQuery queryName;
        queryName.prepare("UPDATE users SET name = :name WHERE user_id = :user_id");
        queryName.bindValue(":name", name);
        queryName.bindValue(":user_id", senderId);
        if (!queryName.exec()) {
            qDebug() << "Ошибка при обновлении имени в базе данных";
            sendJsonResponse("errorDataChange", sessionId);
            return;
        }
    }

    if (!surname.isEmpty()) {
        QSqlQuery querySurname;
        querySurname.prepare("UPDATE users SET surname = :surname WHERE user_id = :user_id");
        querySurname.bindValue(":surname", surname);
        querySurname.bindValue(":user_id", senderId);
        if (!querySurname.exec()) {
            qDebug() << "Ошибка при обновлении фамилии в базе данных";
            sendJsonResponse("errorDataChange", sessionId);
            return;
        }
    }

    if (!password.isEmpty()) {
        QSqlQuery queryPassword;
        queryPassword.prepare("UPDATE users SET password = :password WHERE user_id = :user_id");
        queryPassword.bindValue(":password", password);
        queryPassword.bindValue(":user_id", senderId);
        if (!queryPassword.exec()) {
            qDebug() << "Ошибка при обновлении пароля в базе данных";
            sendJsonResponse("errorDataChange", sessionId);
            return;
        }
    }

    if (!phone.isEmpty()) {
        QSqlQuery queryTelephone;
        queryTelephone.prepare("UPDATE users SET telephone_number = :telephone_number WHERE user_id = :user_id");
        queryTelephone.bindValue(":telephone_number", phone);
        queryTelephone.bindValue(":user_id", senderId);
        if (!queryTelephone.exec()) {
        qDebug() << "Ошибка при обновлении номера телефона в базе данных";
        sendJsonResponse("errorDataChange", sessionId);
        return;
        }
    }

    sendJsonResponse("successfulDataChange", sessionId);
}


void Server::saveProfilePhoto(const QString& sessionId, const QString& base64ImageData, const QString& imageName, const int& imageSize, const QString& extension)
{
    // получаем номер телефона по идентификатору сессии
    QString phoneNumber = m_phoneNumbers.value(sessionId);

    // Получаем user_id отправителя
    int senderId = -1;
    QSqlQuery querySender;
    querySender.prepare("SELECT user_id FROM users WHERE telephone_number = :telephone_number");
    querySender.bindValue(":telephone_number", phoneNumber);
    if (querySender.exec() && querySender.next()) {
        senderId = querySender.value("user_id").toInt();
    } else {
        qDebug() << "Ошибка при получении user_id отправителя из базы данных";
        sendJsonResponse("errorDataChange", sessionId);
    }

    // Создаем путь к файлу на сервере (используем id пользователя)
    QString dirPath = QString("C:/Qt/server/file_storage/profile_picture/%1").arg(senderId);
    QString filePath = QString("%1/%2").arg(dirPath).arg(imageName);

    // Создаем директорию, если ее нет
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Декодируем base64 строку в QByteArray
    QByteArray imageData = QByteArray::fromBase64(base64ImageData.toUtf8());

    // Создаем файл и записываем в него данные
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(imageData);
        file.close();
        qDebug() << "Файл успешно сохранен";

        // Вставка записи в таблицу file_storage
        QSqlQuery queryFileStorage;
        queryFileStorage.prepare("INSERT INTO file_storage (user_id, file_type, file_name, file_size, file_path, created_at, updated_at) "
                                 "VALUES (:user_id, :file_type, :file_name, :file_size, :file_path, :created_at, :updated_at)");
        queryFileStorage.bindValue(":user_id", senderId);
        queryFileStorage.bindValue(":file_type", "image");
        queryFileStorage.bindValue(":file_name", imageName);
        queryFileStorage.bindValue(":file_size", imageSize);
        queryFileStorage.bindValue(":file_path", filePath);
        queryFileStorage.bindValue(":created_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        queryFileStorage.bindValue(":updated_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        if (!queryFileStorage.exec()) {
            qDebug() << "Не удалось вставить запись в таблицу file_storage";
            sendJsonResponse("errorDataChange", sessionId);
            return;
        }

        // Получение идентификатора последней вставленной записи
        int fileId = queryFileStorage.lastInsertId().toInt();

        // Обновляем profile_picture_id в таблице users
        QSqlQuery queryProfilePictureId;
        queryProfilePictureId.prepare("UPDATE users SET profile_picture_id = :profile_picture_id WHERE user_id = :user_id");
        queryProfilePictureId.bindValue(":profile_picture_id", fileId);
        queryProfilePictureId.bindValue(":user_id", senderId);
        if (!queryProfilePictureId.exec()) {
            qDebug() << "Не удалось обновить profile_picture_id в таблице users";
            sendJsonResponse("errorDataChange", sessionId);
            return;
        }

    } else {
        qDebug() << "Ошибка в сохранении файла";
    }
}

void Server::pushSettingsButton(const QString& sessionId)
{
    // Получаем номер телефона по идентификатору сессии
    QString phoneNumber = m_phoneNumbers.value(sessionId);

    // Получаем user_id отправителя
    int senderId = -1;
    QSqlQuery querySender;
    querySender.prepare("SELECT user_id FROM users WHERE telephone_number = :telephone_number");
    querySender.bindValue(":telephone_number", phoneNumber);
    if (querySender.exec() && querySender.next()) {
        senderId = querySender.value("user_id").toInt();
    } else {
        qDebug() << "Ошибка при получении user_id отправителя из базы данных";
    }

    // Создаем JSON объект
    QJsonObject jsonObject;

    // Извлекаем name и surname из таблицы users
    QSqlQuery queryUserInfo;
    queryUserInfo.prepare("SELECT name, surname, profile_picture_id FROM users WHERE user_id = :user_id");
    queryUserInfo.bindValue(":user_id", senderId);
    if (queryUserInfo.exec() && queryUserInfo.next()) {
        QString name = queryUserInfo.value("name").toString();
        QString surname = queryUserInfo.value("surname").toString();
        jsonObject["name"] = name;
        jsonObject["surname"] = surname;

        // Извлекаем путь к фото профиля из таблицы file_storage
        int profilePictureId = queryUserInfo.value("profile_picture_id").toInt();
        if (profilePictureId != 0) {
            QSqlQuery queryProfilePicture;
            queryProfilePicture.prepare("SELECT file_path FROM file_storage WHERE file_id = :file_id");
            queryProfilePicture.bindValue(":file_id", profilePictureId);
            if (queryProfilePicture.exec() && queryProfilePicture.next()) {
                QString filePath = queryProfilePicture.value("file_path").toString();

                // Загружаем фото профиля и добавляем его в JSON объект
                QFile profilePictureFile(filePath);
                if (profilePictureFile.open(QIODevice::ReadOnly)) {
                    QByteArray imageData = profilePictureFile.readAll();
                    QString base64ImageData = QString::fromUtf8(imageData.toBase64());
                    jsonObject["profile_picture"] = base64ImageData;
                    profilePictureFile.close();
                }
            }
        } else {
            jsonObject["profile_picture"] = "zaglushka";
        }
    }

    // Отправляем JSON строку на клиент
    sendJsonObject("updateNameSurnameProfile",sessionId, jsonObject);
}


void Server::changePhoneNumber(const QString &sessionId)
{
    // Допиши метод потом
}

void Server::sendJsonResponse(QString method,const QString &sessionId) {
    // создаем JSON объект и заполняем его данными
    QJsonObject message;
    message["method"] = method;
    message["sessionId"] = sessionId;

    // ищем сокет по sessionId
    QMap<QWebSocket*, QString>::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        if (it.value() == sessionId) {
            // создаем JSON документ и отправляем его на клиент
            QJsonDocument doc(message);
            qDebug() << doc.toJson();
            it.key()->sendTextMessage(doc.toJson());
            return;
        }
    }
    qDebug() << "Сокет не найден для sessionId: " << sessionId;
}

void Server::sendJsonObject(const QString& method, const QString& sessionId, const QJsonObject& userObject)
{
    QJsonObject message;
    message["method"] = method;
    message["sessionId"] = sessionId;
    message["data"] = userObject;
    // ищем сокет по sessionId
    QMap<QWebSocket*, QString>::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        if (it.value() == sessionId) {
            // создаем JSON документ и отправляем его на клиент
            QJsonDocument doc(message);
            qDebug() << doc.toJson();
            it.key()->sendTextMessage(doc.toJson());
            return;
        }
    }
    qDebug() << "Сокет не найден для sessionId: " << sessionId;
}


int Server::getUserIdByPhoneNumber(const QString& phoneNumber) {
    QSqlQuery query;
    query.prepare("SELECT user_id FROM users WHERE telephone_number = :phone_number");
    query.bindValue(":phone_number", phoneNumber);
    if (query.exec() && query.next()) {
        return query.value("user_id").toInt();
    } else {
        qDebug() << "Ошибка при получении id пользователя:" << query.lastError().text();
        return -1;
    }
}


void Server::sendSms(QString phoneNumber, QString message)
{
    QString apiUrl = "https://sms.ru/sms/send";
    QString apiKey = "2E86A30C-4DA6-050C-F25F-E38864647179";

    QString url = apiUrl + "?api_id=" + apiKey + "&to=" + phoneNumber + "&msg=" + message + "&json=1";
    QNetworkRequest request(url);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &Server::onSmsSent);
    manager->get(request);
}


void Server::onSmsSent(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Ответ сервера SMS.RU:" << data;
        // здесь происходит обработка ответа
    } else {
        qDebug() << "Ошибка при отправке SMS на сервер SMS.RU:" << reply->errorString();
        // здесь происходит обработка ошибки
    }

    reply->deleteLater();
}

