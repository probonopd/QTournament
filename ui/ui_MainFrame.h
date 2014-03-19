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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "PlayerTableView.h"
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
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QTabWidget *mainTab;
    QWidget *tabTeams;
    QVBoxLayout *verticalLayout_5;
    TeamTabWidget *widget;
    QWidget *tabPlayers;
    QVBoxLayout *verticalLayout_6;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *btnAddPlayer;
    QPushButton *pushButton;
    PlayerTableView *playerView;
    QMenuBar *menubar;
    QMenu *menuTournament;
    QMenu *menuTesting;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainFrame)
    {
        if (MainFrame->objectName().isEmpty())
            MainFrame->setObjectName(QStringLiteral("MainFrame"));
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
        tabTeams = new QWidget();
        tabTeams->setObjectName(QStringLiteral("tabTeams"));
        tabTeams->setEnabled(true);
        verticalLayout_5 = new QVBoxLayout(tabTeams);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        widget = new TeamTabWidget(tabTeams);
        widget->setObjectName(QStringLiteral("widget"));

        verticalLayout_5->addWidget(widget);

        mainTab->addTab(tabTeams, QString());
        tabPlayers = new QWidget();
        tabPlayers->setObjectName(QStringLiteral("tabPlayers"));
        verticalLayout_6 = new QVBoxLayout(tabPlayers);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        btnAddPlayer = new QPushButton(tabPlayers);
        btnAddPlayer->setObjectName(QStringLiteral("btnAddPlayer"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/document-new-5.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnAddPlayer->setIcon(icon);

        horizontalLayout_2->addWidget(btnAddPlayer);

        pushButton = new QPushButton(tabPlayers);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout_2->addWidget(pushButton);


        verticalLayout_3->addLayout(horizontalLayout_2);

        playerView = new PlayerTableView(tabPlayers);
        playerView->setObjectName(QStringLiteral("playerView"));

        verticalLayout_3->addWidget(playerView);


        verticalLayout_6->addLayout(verticalLayout_3);

        mainTab->addTab(tabPlayers, QString());

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

        retranslateUi(MainFrame);
        QObject::connect(action_Quit, SIGNAL(triggered()), MainFrame, SLOT(close()));
        QObject::connect(actionNew, SIGNAL(triggered()), MainFrame, SLOT(newTournament()));
        QObject::connect(actionOpen, SIGNAL(triggered()), MainFrame, SLOT(openTournament()));
        QObject::connect(actionEmpty, SIGNAL(triggered()), MainFrame, SLOT(setupEmptyScenario()));
        QObject::connect(actionScenario01, SIGNAL(triggered()), MainFrame, SLOT(setupScenario01()));
        QObject::connect(btnAddPlayer, SIGNAL(clicked()), MainFrame, SLOT(onCreatePlayerClicked()));
        QObject::connect(playerView, SIGNAL(doubleClicked(QModelIndex)), MainFrame, SLOT(onPlayerDoubleClicked(QModelIndex)));

        mainTab->setCurrentIndex(0);


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
        mainTab->setTabText(mainTab->indexOf(tabTeams), QApplication::translate("MainFrame", "Teams", 0));
        btnAddPlayer->setText(QApplication::translate("MainFrame", "Add Player", 0));
        pushButton->setText(QApplication::translate("MainFrame", "PushButton", 0));
        mainTab->setTabText(mainTab->indexOf(tabPlayers), QApplication::translate("MainFrame", "Players", 0));
        menuTournament->setTitle(QApplication::translate("MainFrame", "Tournament", 0));
        menuTesting->setTitle(QApplication::translate("MainFrame", "Testing", 0));
    } // retranslateUi

};

namespace Ui {
    class MainFrame: public Ui_MainFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINFRAME_H
