/********************************************************************************
** Form generated from reading UI file 'GroupConfigWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GROUPCONFIGWIDGET_H
#define UI_GROUPCONFIGWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GroupConfigWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_3;
    QLabel *label;
    QComboBox *comboBox;
    QCheckBox *checkBox;
    QFrame *line;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_2;
    QLabel *label_10;
    QHBoxLayout *horizontalLayout_2;
    QSpinBox *spinBox;
    QSpinBox *spinBox_2;
    QHBoxLayout *horizontalLayout_4;
    QSpinBox *spinBox_4;
    QSpinBox *spinBox_3;
    QHBoxLayout *horizontalLayout_5;
    QSpinBox *spinBox_6;
    QSpinBox *spinBox_5;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QLabel *label_7;
    QLabel *label_4;
    QLabel *label_3;
    QLabel *label_6;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_11;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *pushButton;
    QLabel *label_12;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *GroupConfigWidget)
    {
        if (GroupConfigWidget->objectName().isEmpty())
            GroupConfigWidget->setObjectName(QStringLiteral("GroupConfigWidget"));
        GroupConfigWidget->resize(255, 431);
        verticalLayout_2 = new QVBoxLayout(GroupConfigWidget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        label = new QLabel(GroupConfigWidget);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout_3->addWidget(label);

        comboBox = new QComboBox(GroupConfigWidget);
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setEditable(false);
        comboBox->setFrame(false);

        verticalLayout_3->addWidget(comboBox);

        checkBox = new QCheckBox(GroupConfigWidget);
        checkBox->setObjectName(QStringLiteral("checkBox"));

        verticalLayout_3->addWidget(checkBox, 0, Qt::AlignVCenter);


        horizontalLayout->addLayout(verticalLayout_3);


        verticalLayout->addLayout(horizontalLayout);

        line = new QFrame(GroupConfigWidget);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        label_2 = new QLabel(GroupConfigWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(label_2);

        label_10 = new QLabel(GroupConfigWidget);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(label_10);


        verticalLayout->addLayout(horizontalLayout_7);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        spinBox = new QSpinBox(GroupConfigWidget);
        spinBox->setObjectName(QStringLiteral("spinBox"));

        horizontalLayout_2->addWidget(spinBox);

        spinBox_2 = new QSpinBox(GroupConfigWidget);
        spinBox_2->setObjectName(QStringLiteral("spinBox_2"));

        horizontalLayout_2->addWidget(spinBox_2);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        spinBox_4 = new QSpinBox(GroupConfigWidget);
        spinBox_4->setObjectName(QStringLiteral("spinBox_4"));

        horizontalLayout_4->addWidget(spinBox_4);

        spinBox_3 = new QSpinBox(GroupConfigWidget);
        spinBox_3->setObjectName(QStringLiteral("spinBox_3"));

        horizontalLayout_4->addWidget(spinBox_3);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        spinBox_6 = new QSpinBox(GroupConfigWidget);
        spinBox_6->setObjectName(QStringLiteral("spinBox_6"));

        horizontalLayout_5->addWidget(spinBox_6);

        spinBox_5 = new QSpinBox(GroupConfigWidget);
        spinBox_5->setObjectName(QStringLiteral("spinBox_5"));

        horizontalLayout_5->addWidget(spinBox_5);


        verticalLayout->addLayout(horizontalLayout_5);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_5 = new QLabel(GroupConfigWidget);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 0, 1, 1, 1);

        label_7 = new QLabel(GroupConfigWidget);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 0, 2, 1, 1);

        label_4 = new QLabel(GroupConfigWidget);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 1, 1, 1);

        label_3 = new QLabel(GroupConfigWidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        label_6 = new QLabel(GroupConfigWidget);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 0, 0, 1, 1);

        label_8 = new QLabel(GroupConfigWidget);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 0, 3, 1, 1);

        label_9 = new QLabel(GroupConfigWidget);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout->addWidget(label_9, 3, 2, 1, 1);

        label_11 = new QLabel(GroupConfigWidget);
        label_11->setObjectName(QStringLiteral("label_11"));

        gridLayout->addWidget(label_11, 3, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        pushButton = new QPushButton(GroupConfigWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout_6->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout_6);

        label_12 = new QLabel(GroupConfigWidget);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_12);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(GroupConfigWidget);

        comboBox->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(GroupConfigWidget);
    } // setupUi

    void retranslateUi(QWidget *GroupConfigWidget)
    {
        GroupConfigWidget->setWindowTitle(QApplication::translate("GroupConfigWidget", "GroupConfigWidget", 0));
        label->setText(QApplication::translate("GroupConfigWidget", "KO rounds start with", 0));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("GroupConfigWidget", "Semi Finals", 0)
         << QApplication::translate("GroupConfigWidget", "Quarter Finals", 0)
         << QApplication::translate("GroupConfigWidget", "Round of 16", 0)
        );
        checkBox->setText(QApplication::translate("GroupConfigWidget", "Second in group qualifies", 0));
        label_2->setText(QApplication::translate("GroupConfigWidget", "Groups", 0));
        label_10->setText(QApplication::translate("GroupConfigWidget", "Group size", 0));
        label_5->setText(QApplication::translate("GroupConfigWidget", "0", 0));
        label_7->setText(QApplication::translate("GroupConfigWidget", "Sum:", 0));
        label_4->setText(QApplication::translate("GroupConfigWidget", "0", 0));
        label_3->setText(QApplication::translate("GroupConfigWidget", "Required:", 0));
        label_6->setText(QApplication::translate("GroupConfigWidget", "Sum:", 0));
        label_8->setText(QApplication::translate("GroupConfigWidget", "0", 0));
        label_9->setText(QApplication::translate("GroupConfigWidget", "Required:", 0));
        label_11->setText(QApplication::translate("GroupConfigWidget", "0", 0));
        pushButton->setText(QApplication::translate("GroupConfigWidget", "Auto Group Config", 0));
        label_12->setText(QApplication::translate("GroupConfigWidget", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class GroupConfigWidget: public Ui_GroupConfigWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GROUPCONFIGWIDGET_H
