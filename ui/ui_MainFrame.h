/********************************************************************************
** Form generated from reading UI file 'MainFrame.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
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
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "CatTabWidget.h"
#include "PlayerTabWidget.h"
#include "TeamTabWidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainFrame
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSettings;
    QAction *action_Quit;
    QAction *actionEmpty;
    QAction *actionScenario01;
    QAction *actionScenario02;
    QAction *actionScenario03;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QTabWidget *mainTab;
    TeamTabWidget *tabTeams;
    QVBoxLayout *verticalLayout_5;
    PlayerTabWidget *tabPlayers;
    QVBoxLayout *verticalLayout_6;
    CatTabWidget *tabCategories;
    QMenuBar *menubar;
    QMenu *menuTournament;
    QMenu *menuTesting;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainFrame)
    {
        if (MainFrame->objectName().isEmpty())
            MainFrame->setObjectName(QStringLiteral("MainFrame"));
        MainFrame->setWindowModality(Qt::ApplicationModal);
        MainFrame->resize(1017, 760);
        actionNew = new QAction(MainFrame);
        actionNew->setObjectName(QStringLiteral("actionNew"));
        actionOpen = new QAction(MainFrame);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionSettings = new QAction(MainFrame);
        actionSettings->setObjectName(QStringLiteral("actionSettings"));
        action_Quit = new QAction(MainFrame);
        action_Quit->setObjectName(QStringLiteral("action_Quit"));
        actionEmpty = new QAction(MainFrame);
        actionEmpty->setObjectName(QStringLiteral("actionEmpty"));
        actionScenario01 = new QAction(MainFrame);
        actionScenario01->setObjectName(QStringLiteral("actionScenario01"));
        actionScenario02 = new QAction(MainFrame);
        actionScenario02->setObjectName(QStringLiteral("actionScenario02"));
        actionScenario03 = new QAction(MainFrame);
        actionScenario03->setObjectName(QStringLiteral("actionScenario03"));
        centralwidget = new QWidget(MainFrame);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        centralwidget->setEnabled(true);
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        mainTab = new QTabWidget(centralwidget);
        mainTab->setObjectName(QStringLiteral("mainTab"));
        mainTab->setEnabled(true);
        tabTeams = new TeamTabWidget();
        tabTeams->setObjectName(QStringLiteral("tabTeams"));
        tabTeams->setEnabled(true);
        verticalLayout_5 = new QVBoxLayout(tabTeams);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        mainTab->addTab(tabTeams, QString());
        tabPlayers = new PlayerTabWidget();
        tabPlayers->setObjectName(QStringLiteral("tabPlayers"));
        verticalLayout_6 = new QVBoxLayout(tabPlayers);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        mainTab->addTab(tabPlayers, QString());
        tabCategories = new CatTabWidget();
        tabCategories->setObjectName(QStringLiteral("tabCategories"));
        mainTab->addTab(tabCategories, QString());

        verticalLayout->addWidget(mainTab);


        verticalLayout_2->addLayout(verticalLayout);

        MainFrame->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainFrame);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 1017, 25));
        menuTournament = new QMenu(menubar);
        menuTournament->setObjectName(QStringLiteral("menuTournament"));
        menuTesting = new QMenu(menubar);
        menuTesting->setObjectName(QStringLiteral("menuTesting"));
        MainFrame->setMenuBar(menubar);
        statusbar = new QStatusBar(MainFrame);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainFrame->setStatusBar(statusbar);

        menubar->addAction(menuTournament->menuAction());
        menubar->addAction(menuTesting->menuAction());
        menuTournament->addAction(actionNew);
        menuTournament->addAction(actionOpen);
        menuTournament->addSeparator();
        menuTournament->addAction(actionSettings);
        menuTournament->addSeparator();
        menuTournament->addAction(action_Quit);
        menuTesting->addAction(actionEmpty);
        menuTesting->addAction(actionScenario01);
        menuTesting->addAction(actionScenario02);
        menuTesting->addAction(actionScenario03);

        retranslateUi(MainFrame);
        QObject::connect(action_Quit, SIGNAL(triggered()), MainFrame, SLOT(close()));
        QObject::connect(actionNew, SIGNAL(triggered()), MainFrame, SLOT(newTournament()));
        QObject::connect(actionOpen, SIGNAL(triggered()), MainFrame, SLOT(openTournament()));
        QObject::connect(actionEmpty, SIGNAL(triggered()), MainFrame, SLOT(setupEmptyScenario()));
        QObject::connect(actionScenario01, SIGNAL(triggered()), MainFrame, SLOT(setupScenario01()));
        QObject::connect(actionScenario02, SIGNAL(triggered()), MainFrame, SLOT(setupScenario02()));
        QObject::connect(actionScenario03, SIGNAL(triggered()), MainFrame, SLOT(setupScenario03()));

        mainTab->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainFrame);
    } // setupUi

    void retranslateUi(QMainWindow *MainFrame)
    {
        MainFrame->setWindowTitle(QApplication::translate("MainFrame", "QTournament", 0));
        actionNew->setText(QApplication::translate("MainFrame", "&New", 0));
        actionOpen->setText(QApplication::translate("MainFrame", "&Open", 0));
        actionSettings->setText(QApplication::translate("MainFrame", "&Settings...", 0));
        action_Quit->setText(QApplication::translate("MainFrame", "&Quit", 0));
        actionEmpty->setText(QApplication::translate("MainFrame", "Empty", 0));
        actionScenario01->setText(QApplication::translate("MainFrame", "Scenario01", 0));
        actionScenario02->setText(QApplication::translate("MainFrame", "Scenario02", 0));
        actionScenario03->setText(QApplication::translate("MainFrame", "Scenario03", 0));
        mainTab->setTabText(mainTab->indexOf(tabTeams), QApplication::translate("MainFrame", "Teams", 0));
        mainTab->setTabText(mainTab->indexOf(tabPlayers), QApplication::translate("MainFrame", "Players", 0));
        mainTab->setTabText(mainTab->indexOf(tabCategories), QApplication::translate("MainFrame", "Page", 0));
        menuTournament->setTitle(QApplication::translate("MainFrame", "Tournament", 0));
        menuTesting->setTitle(QApplication::translate("MainFrame", "Testing", 0));
    } // retranslateUi

};

namespace Ui {
    class MainFrame: public Ui_MainFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINFRAME_H
