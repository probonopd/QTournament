/********************************************************************************
** Form generated from reading UI file 'dlgGroupAssignment.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGGROUPASSIGNMENT_H
#define UI_DLGGROUPASSIGNMENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include "GroupAssignmentListWidget.h"

QT_BEGIN_NAMESPACE

class Ui_dlgGroupAssignment
{
public:
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    GroupAssignmentListWidget *grpWidget;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer_2;
    QPushButton *btnSwap;
    QSpacerItem *verticalSpacer_3;
    QPushButton *btnRandomize;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *dlgGroupAssignment)
    {
        if (dlgGroupAssignment->objectName().isEmpty())
            dlgGroupAssignment->setObjectName(QStringLiteral("dlgGroupAssignment"));
        dlgGroupAssignment->resize(1227, 576);
        horizontalLayout_2 = new QHBoxLayout(dlgGroupAssignment);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        grpWidget = new GroupAssignmentListWidget(dlgGroupAssignment);
        grpWidget->setObjectName(QStringLiteral("grpWidget"));

        horizontalLayout->addWidget(grpWidget);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        btnSwap = new QPushButton(dlgGroupAssignment);
        btnSwap->setObjectName(QStringLiteral("btnSwap"));

        verticalLayout->addWidget(btnSwap);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        btnRandomize = new QPushButton(dlgGroupAssignment);
        btnRandomize->setObjectName(QStringLiteral("btnRandomize"));

        verticalLayout->addWidget(btnRandomize);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(dlgGroupAssignment);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(buttonBox->sizePolicy().hasHeightForWidth());
        buttonBox->setSizePolicy(sizePolicy);
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        horizontalLayout->addLayout(verticalLayout);


        horizontalLayout_2->addLayout(horizontalLayout);


        retranslateUi(dlgGroupAssignment);
        QObject::connect(buttonBox, SIGNAL(accepted()), dlgGroupAssignment, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), dlgGroupAssignment, SLOT(reject()));
        QObject::connect(btnRandomize, SIGNAL(clicked()), dlgGroupAssignment, SLOT(onBtnRandomizeClicked()));
        QObject::connect(btnSwap, SIGNAL(clicked()), dlgGroupAssignment, SLOT(onBtnSwapClicked()));

        QMetaObject::connectSlotsByName(dlgGroupAssignment);
    } // setupUi

    void retranslateUi(QDialog *dlgGroupAssignment)
    {
        dlgGroupAssignment->setWindowTitle(QApplication::translate("dlgGroupAssignment", "dlgGroupAssignment", 0));
        btnSwap->setText(QApplication::translate("dlgGroupAssignment", "Swap Players", 0));
        btnRandomize->setText(QApplication::translate("dlgGroupAssignment", "Randomize Everything", 0));
    } // retranslateUi

};

namespace Ui {
    class dlgGroupAssignment: public Ui_dlgGroupAssignment {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGGROUPASSIGNMENT_H
