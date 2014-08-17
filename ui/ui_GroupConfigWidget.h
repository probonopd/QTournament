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
    QComboBox *cbKOStart;
    QCheckBox *cbSecondSurvives;
    QFrame *line;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_2;
    QLabel *label_10;
    QHBoxLayout *horizontalLayout_2;
    QSpinBox *spGroupCount1;
    QSpinBox *spGroupSize1;
    QHBoxLayout *horizontalLayout_4;
    QSpinBox *spGroupCount2;
    QSpinBox *spGroupSize2;
    QHBoxLayout *horizontalLayout_5;
    QSpinBox *spGroupCount3;
    QSpinBox *spGroupSize3;
    QGridLayout *gridLayout;
    QLabel *laTotalGroups;
    QLabel *label_7;
    QLabel *laReqGroupCount;
    QLabel *label_3;
    QLabel *label_6;
    QLabel *laTotalPlayers;
    QLabel *label_9;
    QLabel *laReqPlayerCount;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *btnAutoConf;
    QLabel *laValidity;
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

        cbKOStart = new QComboBox(GroupConfigWidget);
        cbKOStart->setObjectName(QStringLiteral("cbKOStart"));
        cbKOStart->setEditable(false);
        cbKOStart->setFrame(false);

        verticalLayout_3->addWidget(cbKOStart);

        cbSecondSurvives = new QCheckBox(GroupConfigWidget);
        cbSecondSurvives->setObjectName(QStringLiteral("cbSecondSurvives"));

        verticalLayout_3->addWidget(cbSecondSurvives, 0, Qt::AlignVCenter);


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
        spGroupCount1 = new QSpinBox(GroupConfigWidget);
        spGroupCount1->setObjectName(QStringLiteral("spGroupCount1"));

        horizontalLayout_2->addWidget(spGroupCount1);

        spGroupSize1 = new QSpinBox(GroupConfigWidget);
        spGroupSize1->setObjectName(QStringLiteral("spGroupSize1"));

        horizontalLayout_2->addWidget(spGroupSize1);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        spGroupCount2 = new QSpinBox(GroupConfigWidget);
        spGroupCount2->setObjectName(QStringLiteral("spGroupCount2"));

        horizontalLayout_4->addWidget(spGroupCount2);

        spGroupSize2 = new QSpinBox(GroupConfigWidget);
        spGroupSize2->setObjectName(QStringLiteral("spGroupSize2"));

        horizontalLayout_4->addWidget(spGroupSize2);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        spGroupCount3 = new QSpinBox(GroupConfigWidget);
        spGroupCount3->setObjectName(QStringLiteral("spGroupCount3"));

        horizontalLayout_5->addWidget(spGroupCount3);

        spGroupSize3 = new QSpinBox(GroupConfigWidget);
        spGroupSize3->setObjectName(QStringLiteral("spGroupSize3"));

        horizontalLayout_5->addWidget(spGroupSize3);


        verticalLayout->addLayout(horizontalLayout_5);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        laTotalGroups = new QLabel(GroupConfigWidget);
        laTotalGroups->setObjectName(QStringLiteral("laTotalGroups"));

        gridLayout->addWidget(laTotalGroups, 0, 1, 1, 1);

        label_7 = new QLabel(GroupConfigWidget);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 0, 2, 1, 1);

        laReqGroupCount = new QLabel(GroupConfigWidget);
        laReqGroupCount->setObjectName(QStringLiteral("laReqGroupCount"));

        gridLayout->addWidget(laReqGroupCount, 3, 1, 1, 1);

        label_3 = new QLabel(GroupConfigWidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        label_6 = new QLabel(GroupConfigWidget);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 0, 0, 1, 1);

        laTotalPlayers = new QLabel(GroupConfigWidget);
        laTotalPlayers->setObjectName(QStringLiteral("laTotalPlayers"));

        gridLayout->addWidget(laTotalPlayers, 0, 3, 1, 1);

        label_9 = new QLabel(GroupConfigWidget);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout->addWidget(label_9, 3, 2, 1, 1);

        laReqPlayerCount = new QLabel(GroupConfigWidget);
        laReqPlayerCount->setObjectName(QStringLiteral("laReqPlayerCount"));

        gridLayout->addWidget(laReqPlayerCount, 3, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        btnAutoConf = new QPushButton(GroupConfigWidget);
        btnAutoConf->setObjectName(QStringLiteral("btnAutoConf"));

        horizontalLayout_6->addWidget(btnAutoConf);


        verticalLayout->addLayout(horizontalLayout_6);

        laValidity = new QLabel(GroupConfigWidget);
        laValidity->setObjectName(QStringLiteral("laValidity"));
        laValidity->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(laValidity);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(GroupConfigWidget);
        QObject::connect(cbKOStart, SIGNAL(currentIndexChanged(int)), GroupConfigWidget, SLOT(onStartLevelChanged(int)));
        QObject::connect(cbSecondSurvives, SIGNAL(clicked()), GroupConfigWidget, SLOT(onSecondSurvivesChanged()));
        QObject::connect(spGroupCount1, SIGNAL(valueChanged(int)), GroupConfigWidget, SLOT(onSpinBoxGroupCount1Changed(int)));
        QObject::connect(spGroupCount2, SIGNAL(valueChanged(int)), GroupConfigWidget, SLOT(onSpinBoxGroupCount2Changed(int)));
        QObject::connect(spGroupCount3, SIGNAL(valueChanged(int)), GroupConfigWidget, SLOT(onSpinBoxGroupCount3Changed(int)));
        QObject::connect(spGroupSize1, SIGNAL(valueChanged(int)), GroupConfigWidget, SLOT(onSpinBoxGroupSize1Changed(int)));
        QObject::connect(spGroupSize3, SIGNAL(valueChanged(int)), GroupConfigWidget, SLOT(onSpinBoxGroupSize3Changed(int)));
        QObject::connect(spGroupSize2, SIGNAL(valueChanged(int)), GroupConfigWidget, SLOT(onSpinBoxGroupSize2Changed(int)));

        cbKOStart->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(GroupConfigWidget);
    } // setupUi

    void retranslateUi(QWidget *GroupConfigWidget)
    {
        GroupConfigWidget->setWindowTitle(QApplication::translate("GroupConfigWidget", "GroupConfigWidget", 0));
        label->setText(QApplication::translate("GroupConfigWidget", "KO rounds start with", 0));
        cbKOStart->clear();
        cbKOStart->insertItems(0, QStringList()
         << QApplication::translate("GroupConfigWidget", "Semi Finals", 0)
         << QApplication::translate("GroupConfigWidget", "Quarter Finals", 0)
         << QApplication::translate("GroupConfigWidget", "Round of 16", 0)
        );
        cbSecondSurvives->setText(QApplication::translate("GroupConfigWidget", "Second in group qualifies", 0));
        label_2->setText(QApplication::translate("GroupConfigWidget", "Groups", 0));
        label_10->setText(QApplication::translate("GroupConfigWidget", "Group size", 0));
        laTotalGroups->setText(QApplication::translate("GroupConfigWidget", "0", 0));
        label_7->setText(QApplication::translate("GroupConfigWidget", "Sum:", 0));
        laReqGroupCount->setText(QApplication::translate("GroupConfigWidget", "0", 0));
        label_3->setText(QApplication::translate("GroupConfigWidget", "Required:", 0));
        label_6->setText(QApplication::translate("GroupConfigWidget", "Sum:", 0));
        laTotalPlayers->setText(QApplication::translate("GroupConfigWidget", "0", 0));
        label_9->setText(QApplication::translate("GroupConfigWidget", "Required:", 0));
        laReqPlayerCount->setText(QApplication::translate("GroupConfigWidget", "0", 0));
        btnAutoConf->setText(QApplication::translate("GroupConfigWidget", "Auto Group Config", 0));
        laValidity->setText(QApplication::translate("GroupConfigWidget", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class GroupConfigWidget: public Ui_GroupConfigWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GROUPCONFIGWIDGET_H
