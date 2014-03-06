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
#include <QtWidgets/QColumnView>
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

QT_BEGIN_NAMESPACE

class Ui_MainFrame
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSettings;
    QAction *action_Quit;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QTabWidget *mainTab;
    QWidget *tabTeams;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QColumnView *columnView;
    QWidget *tabPlayers;
    QMenuBar *menubar;
    QMenu *menuTournament;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainFrame)
    {
        if (MainFrame->objectName().isEmpty())
            MainFrame->setObjectName(QStringLiteral("MainFrame"));
        MainFrame->resize(938, 760);
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
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        mainTab = new QTabWidget(centralwidget);
        mainTab->setObjectName(QStringLiteral("mainTab"));
        tabTeams = new QWidget();
        tabTeams->setObjectName(QStringLiteral("tabTeams"));
        verticalLayout_5 = new QVBoxLayout(tabTeams);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        pushButton = new QPushButton(tabTeams);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/document-new-5.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton->setIcon(icon);

        horizontalLayout->addWidget(pushButton);

        pushButton_2 = new QPushButton(tabTeams);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/edit-rename.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_2->setIcon(icon1);

        horizontalLayout->addWidget(pushButton_2);

        pushButton_3 = new QPushButton(tabTeams);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/edit-delete-9.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_3->setIcon(icon2);

        horizontalLayout->addWidget(pushButton_3);


        verticalLayout_4->addLayout(horizontalLayout);

        columnView = new QColumnView(tabTeams);
        columnView->setObjectName(QStringLiteral("columnView"));

        verticalLayout_4->addWidget(columnView);


        verticalLayout_5->addLayout(verticalLayout_4);

        mainTab->addTab(tabTeams, QString());
        tabPlayers = new QWidget();
        tabPlayers->setObjectName(QStringLiteral("tabPlayers"));
        mainTab->addTab(tabPlayers, QString());

        verticalLayout->addWidget(mainTab);


        verticalLayout_2->addLayout(verticalLayout);

        MainFrame->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainFrame);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 938, 25));
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
        pushButton->setText(QApplication::translate("MainFrame", "New...", 0));
        pushButton_2->setText(QApplication::translate("MainFrame", "Rename...", 0));
        pushButton_3->setText(QApplication::translate("MainFrame", "Delete...", 0));
        mainTab->setTabText(mainTab->indexOf(tabTeams), QApplication::translate("MainFrame", "Teams", 0));
        mainTab->setTabText(mainTab->indexOf(tabPlayers), QApplication::translate("MainFrame", "Players", 0));
        menuTournament->setTitle(QApplication::translate("MainFrame", "Tournament", 0));
    } // retranslateUi

};

namespace Ui {
    class MainFrame: public Ui_MainFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINFRAME_H
