QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    algorithms.cpp \
    edge.cpp \
    graph.cpp \
    graphwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    node.cpp \
    saves.cpp \
    simplex_solver.cpp \
    textbox.cpp

HEADERS += \
    edge.h \
    graph.h \
    graphenums.h \
    graphwidget.h \
    mainwindow.h \
    node.h \
    textbox.h

FORMS += \
    mainwindow.ui

RC_ICONS = Logo.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
