#-------------------------------------------------
#
# Project created by QtCreator 2022-01-17T15:38:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LevelEditor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    ItemModels/treeitemmodel.cpp \
    ItemModels/nodeinfo.cpp \
    Dialogs/dlgchoseetype.cpp \
    Dialogs/dlgconditiontype.cpp \
    ItemModels/eventtype.cpp \
    Dialogs/dlgeditvalue.cpp \
    ItemModels/functioninfo.cpp \
    Values/valuemanager.cpp \
    Dialogs/dlgvariablemanager.cpp \
    Dialogs/dlgsetvariable.cpp \
    Dialogs/dlgchoseactiontype.cpp \
    Values/enuminfo.cpp \
    Dialogs/dlgwaiting.cpp \
    ItemModels/treeitemmodel_custom.cpp \
    ItemModels/treeitemmodel_event.cpp \
    nodesclipboard.cpp \
    Dialogs/multiselectcombobox.cpp \
    Values/structinfo.cpp \
    Dialogs/dlgeditstructvalue.cpp \
    Values/structvalueclass.cpp \
    Values/valueclass.cpp \
    Dialogs/multilevelcombobox.cpp \
    Utils/pinyin.cpp

HEADERS += \
        mainwindow.h \
    ItemModels/treeitemmodel.h \
    ItemModels/nodeinfo.h \
    Dialogs/dlgchoseetype.h \
    Dialogs/dlgconditiontype.h \
    ItemModels/enumdefine.h \
    ItemModels/eventtype.h \
    Dialogs/dlgeditvalue.h \
    ItemModels/functioninfo.h \
    Values/valuemanager.h \
    Values/valueclass.h \
    Dialogs/dlgvariablemanager.h \
    Dialogs/dlgsetvariable.h \
    Dialogs/dlgchoseactiontype.h \
    Values/enuminfo.h \
    Dialogs/dlgwaiting.h \
    ItemModels/treeitemmodel_custom.h \
    ItemModels/treeitemmodel_event.h \
    nodesclipboard.h \
    Dialogs/multiselectcombobox.h \
    Values/structinfo.h \
    Dialogs/dlgeditstructvalue.h \
    Dialogs/multilevelcombobox.h \
    Utils/pinyin.h

FORMS += \
        mainwindow.ui \
    Dialogs/dlgchoseetype.ui \
    Dialogs/dlgconditiontype.ui \
    Dialogs/dlgeditvalue.ui \
    Dialogs/dlgvariablemanager.ui \
    Dialogs/dlgsetvariable.ui \
    Dialogs/dlgchoseactiontype.ui \
    Dialogs/dlgwaiting.ui \
    Dialogs/dlgeditstructvalue.ui
