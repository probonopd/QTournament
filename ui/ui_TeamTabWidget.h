/********************************************************************************
** Form generated from reading UI file 'TeamTabWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEAMTABWIDGET_H
#define UI_TEAMTABWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QColumnView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "TeamListView.h"

QT_BEGIN_NAMESPACE

class Ui_TeamTabWidget
{
public:
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnNewTeam;
    QColumnView *columnView;
    TeamListView *teamList;

    void setupUi(QWidget *TeamTabWidget)
    {
        if (TeamTabWidget->objectName().isEmpty())
            TeamTabWidget->setObjectName(QStringLiteral("TeamTabWidget"));
        TeamTabWidget->resize(1000, 636);
        verticalLayout_3 = new QVBoxLayout(TeamTabWidget);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        btnNewTeam = new QPushButton(TeamTabWidget);
        btnNewTeam->setObjectName(QStringLiteral("btnNewTeam"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/document-new-5.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnNewTeam->setIcon(icon);

        horizontalLayout->addWidget(btnNewTeam);


        verticalLayout_4->addLayout(horizontalLayout);

        columnView = new QColumnView(TeamTabWidget);
        columnView->setObjectName(QStringLiteral("columnView"));

        verticalLayout_4->addWidget(columnView);

        teamList = new TeamListView(TeamTabWidget);
        teamList->setObjectName(QStringLiteral("teamList"));

        verticalLayout_4->addWidget(teamList);


        verticalLayout_3->addLayout(verticalLayout_4);


        retranslateUi(TeamTabWidget);
        QObject::connect(btnNewTeam, SIGNAL(clicked()), TeamTabWidget, SLOT(onCreateTeamClicked()));

        QMetaObject::connectSlotsByName(TeamTabWidget);
    } // setupUi

    void retranslateUi(QWidget *TeamTabWidget)
    {
        TeamTabWidget->setWindowTitle(QString());
        btnNewTeam->setText(QApplication::translate("TeamTabWidget", "Create new team", 0));
    } // retranslateUi

};

namespace Ui {
    class TeamTabWidget: public Ui_TeamTabWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEAMTABWIDGET_H
