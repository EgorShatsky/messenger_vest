QT       += core gui network sql websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

android {
    LIBS += -llog
}

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    authentication.cpp \
    chat.cpp \
    createchat.cpp \
    main.cpp \
    client.cpp \
    registration.cpp \
    sms.cpp \
    userchat.cpp \
    userprofile.cpp

HEADERS += \
    authentication.h \
    chat.h \
    client.h \
    createchat.h \
    registration.h \
    sms.h \
    userchat.h \
    userprofile.h

FORMS += \
    authentication.ui \
    chat.ui \
    client.ui \
    createchat.ui \
    registration.ui \
    sms.ui \
    userchat.ui \
    userprofile.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
