/********************************************************************************
** Form generated from reading UI file 'MainFrame.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
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
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainFrame
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSettings;
    QAction *action_Quit;
    QWidget *centralwidget;
    QPushButton *pushButton;
    QMenuBar *menubar;
    QMenu *menuTournament;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainFrame)
    {
        if (MainFrame->objectName().isEmpty())
            MainFrame->setObjectName(QStringLiteral("MainFrame"));
        MainFrame->resize(800, 600);
        actionNew = new QAction(MainFrame);
        actionNew->setObjectName(QStringLiteral("actionNew"));
        actionOpen = new QAction(MainFrame);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionSettings = new QAction(MainFrame);
        actionSettings->setObjectName(QStringLiteral("actionSettings"));
        action_Quit = new QAction(MainFrame);
        action_Quit->setObjectName(QStringLiteral("action_Quit"));
        centralwidget = new QWidget(MainFrame);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(250, 180, 92, 27));
        MainFrame->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainFrame);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        menuTournament = new QMenu(menubar);
        menuTournament->setObjectName(QStringLiteral("menuTournament"));
        MainFrame->setMenuBar(menubar);
        statusbar = new QStatusBar(MainFrame);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainFrame->setStatusBar(statusbar);

        menubar->addAction(menuTournament->menuAction());
        menuTournament->addAction(actionNew);
        menuTournament->addAction(actionOpen);
        menuTournament->addSeparator();
        menuTournament->addAction(actionSettings);
        menuTournament->addSeparator();
        menuTournament->addAction(action_Quit);

        retranslateUi(MainFrame);

        QMetaObject::connectSlotsByName(MainFrame);
    } // setupUi

    void retranslateUi(QMainWindow *MainFrame)
    {
        MainFrame->setWindowTitle(QApplication::translate("MainFrame", "QTournament", 0));
        actionNew->setText(QApplication::translate("MainFrame", "&New", 0));
        actionOpen->setText(QApplication::translate("MainFrame", "&Open", 0));
        actionSettings->setText(QApplication::translate("MainFrame", "&Settings...", 0));
        action_Quit->setText(QApplication::translate("MainFrame", "&Quit", 0));
        pushButton->setText(QApplication::translate("MainFrame", "PushButton", 0));
        menuTournament->setTitle(QApplication::translate("MainFrame", "Tournament", 0));
    } // retranslateUi

};

namespace Ui {
    class MainFrame: public Ui_MainFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINFRAME_H
