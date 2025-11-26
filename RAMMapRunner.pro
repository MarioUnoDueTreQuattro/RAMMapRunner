QT       += core gui

CONFIG += c++11
#CONFIG += windows
#CONFIG -= console
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += winextras

TEMPLATE = app

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        logger.cpp \
        main.cpp \
        osdwidget.cpp \
        reducememoryusage.cpp \
        widget.cpp

win32:LIBS += -lpsapi

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    logger.h \
    osdwidget.h \
    reducememoryusage.h \
    widget.h

FORMS += \
    widget.ui

# Disabilita completamente i messaggi qDebug() nella build di rilascio (Release)
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}
