/********************************************************************************
** Form generated from reading UI file 'MainFrame.ui'
**
** Created by: Qt User Interface Compiler version 5.2.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINFRAME_H
#define UI_MAINFRAME_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainFrame
{
public:
    QWidget *centralwidget;
    QPushButton *pushButton;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainFrame)
    {
        if (MainFrame->objectName().isEmpty())
            MainFrame->setObjectName(QStringLiteral("MainFrame"));
        MainFrame->resize(800, 600);
        centralwidget = new QWidget(MainFrame);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(250, 180, 92, 27));
        MainFrame->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainFrame);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        MainFrame->setMenuBar(menubar);
        statusbar = new QStatusBar(MainFrame);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainFrame->setStatusBar(statusbar);

        retranslateUi(MainFrame);

        QMetaObject::connectSlotsByName(MainFrame);
    } // setupUi

    void retranslateUi(QMainWindow *MainFrame)
    {
        MainFrame->setWindowTitle(QApplication::translate("MainFrame", "MainFrame", 0));
        pushButton->setText(QApplication::translate("MainFrame", "PushButton", 0));
    } // retranslateUi

};

namespace Ui {
    class MainFrame: public Ui_MainFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINFRAME_H
