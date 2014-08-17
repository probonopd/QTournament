/********************************************************************************
** Form generated from reading UI file 'PlayerTabWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLAYERTABWIDGET_H
#define UI_PLAYERTABWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "PlayerTableView.h"

QT_BEGIN_NAMESPACE

class Ui_PlayerTabWidget
{
public:
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *btnAddPlayer;
    QPushButton *pushButton;
    PlayerTableView *playerView;

    void setupUi(QWidget *PlayerTabWidget)
    {
        if (PlayerTabWidget->objectName().isEmpty())
            PlayerTabWidget->setObjectName(QStringLiteral("PlayerTabWidget"));
        PlayerTabWidget->resize(703, 495);
        verticalLayout = new QVBoxLayout(PlayerTabWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        btnAddPlayer = new QPushButton(PlayerTabWidget);
        btnAddPlayer->setObjectName(QStringLiteral("btnAddPlayer"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/document-new-5.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnAddPlayer->setIcon(icon);

        horizontalLayout_2->addWidget(btnAddPlayer);

        pushButton = new QPushButton(PlayerTabWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout_2->addWidget(pushButton);


        verticalLayout_3->addLayout(horizontalLayout_2);

        playerView = new PlayerTableView(PlayerTabWidget);
        playerView->setObjectName(QStringLiteral("playerView"));

        verticalLayout_3->addWidget(playerView);


        verticalLayout->addLayout(verticalLayout_3);


        retranslateUi(PlayerTabWidget);
        QObject::connect(btnAddPlayer, SIGNAL(clicked()), PlayerTabWidget, SLOT(onCreatePlayerClicked()));
        QObject::connect(playerView, SIGNAL(doubleClicked(QModelIndex)), PlayerTabWidget, SLOT(onPlayerDoubleClicked(QModelIndex)));

        QMetaObject::connectSlotsByName(PlayerTabWidget);
    } // setupUi

    void retranslateUi(QWidget *PlayerTabWidget)
    {
        PlayerTabWidget->setWindowTitle(QApplication::translate("PlayerTabWidget", "PlayerTabWidget", 0));
        btnAddPlayer->setText(QApplication::translate("PlayerTabWidget", "Add Player", 0));
        pushButton->setText(QApplication::translate("PlayerTabWidget", "PushButton", 0));
    } // retranslateUi

};

namespace Ui {
    class PlayerTabWidget: public Ui_PlayerTabWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLAYERTABWIDGET_H
