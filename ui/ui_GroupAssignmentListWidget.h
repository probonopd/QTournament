/********************************************************************************
** Form generated from reading UI file 'GroupAssignmentListWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GROUPASSIGNMENTLISTWIDGET_H
#define UI_GROUPASSIGNMENTLISTWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GroupAssignmentListWidget
{
public:
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QWidget *gridLayoutWidget;
    QGridLayout *grid;

    void setupUi(QWidget *GroupAssignmentListWidget)
    {
        if (GroupAssignmentListWidget->objectName().isEmpty())
            GroupAssignmentListWidget->setObjectName(QStringLiteral("GroupAssignmentListWidget"));
        GroupAssignmentListWidget->resize(603, 416);
        gridLayout = new QGridLayout(GroupAssignmentListWidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        scrollArea = new QScrollArea(GroupAssignmentListWidget);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 583, 396));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(scrollAreaWidgetContents->sizePolicy().hasHeightForWidth());
        scrollAreaWidgetContents->setSizePolicy(sizePolicy);
        gridLayoutWidget = new QWidget(scrollAreaWidgetContents);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 10, 160, 80));
        grid = new QGridLayout(gridLayoutWidget);
        grid->setObjectName(QStringLiteral("grid"));
        grid->setSizeConstraint(QLayout::SetMinAndMaxSize);
        grid->setContentsMargins(0, 0, 0, 0);
        scrollArea->setWidget(scrollAreaWidgetContents);

        gridLayout->addWidget(scrollArea, 0, 0, 1, 1);


        retranslateUi(GroupAssignmentListWidget);

        QMetaObject::connectSlotsByName(GroupAssignmentListWidget);
    } // setupUi

    void retranslateUi(QWidget *GroupAssignmentListWidget)
    {
        GroupAssignmentListWidget->setWindowTitle(QApplication::translate("GroupAssignmentListWidget", "GroupAssignmentListWidget", 0));
    } // retranslateUi

};

namespace Ui {
    class GroupAssignmentListWidget: public Ui_GroupAssignmentListWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GROUPASSIGNMENTLISTWIDGET_H
