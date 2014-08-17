/********************************************************************************
** Form generated from reading UI file 'CatTabWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CATTABWIDGET_H
#define UI_CATTABWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include "CatTableView.h"
#include "GroupConfigWidget.h"

QT_BEGIN_NAMESPACE

class Ui_CatTabWidget
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_6;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *btnAddCategory;
    CategoryTableView *catTableView;
    QVBoxLayout *verticalLayout_7;
    QGroupBox *gbGeneric;
    QVBoxLayout *verticalLayout_2;
    QComboBox *cbMatchSystem;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_2;
    QRadioButton *rbSingles;
    QRadioButton *rbDoubles;
    QRadioButton *rbMixed;
    QVBoxLayout *verticalLayout_3;
    QLabel *label;
    QRadioButton *rbMen;
    QRadioButton *rbLadies;
    QCheckBox *cbDontCare;
    QCheckBox *cbDraw;
    QSpinBox *sbWinScore;
    QSpinBox *sbDrawScore;
    QGroupBox *gbGroups;
    QHBoxLayout *horizontalLayout_3;
    GroupConfigWidget *grpCfgWidget;
    QGroupBox *gbSwiss;
    QVBoxLayout *verticalLayout_5;
    QLabel *label_4;
    QGroupBox *gbRandom;
    QVBoxLayout *verticalLayout_9;
    QCheckBox *cbRandPairs;
    QGroupBox *gbPairs;
    QHBoxLayout *horizontalLayout_5;
    QListWidget *lwUnpaired;
    QGroupBox *verticalGroupBox_2;
    QVBoxLayout *verticalLayout_8;
    QPushButton *btnPair;
    QPushButton *btnSplit;
    QSpacerItem *verticalSpacer;
    QFrame *line;
    QSpacerItem *verticalSpacer_2;
    QPushButton *btnRandomizeRemaining;
    QPushButton *btnRandomizeAll;
    QCheckBox *cbAvoidSameTeam;
    QListWidget *lwPaired;
    QButtonGroup *rbgSex;
    QButtonGroup *rbgMatchType;

    void setupUi(QDialog *CatTabWidget)
    {
        if (CatTabWidget->objectName().isEmpty())
            CatTabWidget->setObjectName(QStringLiteral("CatTabWidget"));
        CatTabWidget->resize(1009, 858);
        verticalLayout = new QVBoxLayout(CatTabWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(CatTabWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        btnAddCategory = new QPushButton(groupBox);
        btnAddCategory->setObjectName(QStringLiteral("btnAddCategory"));

        horizontalLayout_4->addWidget(btnAddCategory);


        verticalLayout_6->addLayout(horizontalLayout_4);

        catTableView = new CategoryTableView(groupBox);
        catTableView->setObjectName(QStringLiteral("catTableView"));

        verticalLayout_6->addWidget(catTableView);


        horizontalLayout->addLayout(verticalLayout_6);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        gbGeneric = new QGroupBox(groupBox);
        gbGeneric->setObjectName(QStringLiteral("gbGeneric"));
        verticalLayout_2 = new QVBoxLayout(gbGeneric);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        cbMatchSystem = new QComboBox(gbGeneric);
        cbMatchSystem->setObjectName(QStringLiteral("cbMatchSystem"));

        verticalLayout_2->addWidget(cbMatchSystem);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        label_2 = new QLabel(gbGeneric);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy);

        verticalLayout_4->addWidget(label_2);

        rbSingles = new QRadioButton(gbGeneric);
        rbgMatchType = new QButtonGroup(CatTabWidget);
        rbgMatchType->setObjectName(QStringLiteral("rbgMatchType"));
        rbgMatchType->addButton(rbSingles);
        rbSingles->setObjectName(QStringLiteral("rbSingles"));

        verticalLayout_4->addWidget(rbSingles);

        rbDoubles = new QRadioButton(gbGeneric);
        rbgMatchType->addButton(rbDoubles);
        rbDoubles->setObjectName(QStringLiteral("rbDoubles"));

        verticalLayout_4->addWidget(rbDoubles);

        rbMixed = new QRadioButton(gbGeneric);
        rbgMatchType->addButton(rbMixed);
        rbMixed->setObjectName(QStringLiteral("rbMixed"));

        verticalLayout_4->addWidget(rbMixed);


        horizontalLayout_2->addLayout(verticalLayout_4);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        label = new QLabel(gbGeneric);
        label->setObjectName(QStringLiteral("label"));
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);

        verticalLayout_3->addWidget(label);

        rbMen = new QRadioButton(gbGeneric);
        rbgSex = new QButtonGroup(CatTabWidget);
        rbgSex->setObjectName(QStringLiteral("rbgSex"));
        rbgSex->addButton(rbMen);
        rbMen->setObjectName(QStringLiteral("rbMen"));

        verticalLayout_3->addWidget(rbMen);

        rbLadies = new QRadioButton(gbGeneric);
        rbgSex->addButton(rbLadies);
        rbLadies->setObjectName(QStringLiteral("rbLadies"));

        verticalLayout_3->addWidget(rbLadies);

        cbDontCare = new QCheckBox(gbGeneric);
        cbDontCare->setObjectName(QStringLiteral("cbDontCare"));

        verticalLayout_3->addWidget(cbDontCare);


        horizontalLayout_2->addLayout(verticalLayout_3);


        verticalLayout_2->addLayout(horizontalLayout_2);

        cbDraw = new QCheckBox(gbGeneric);
        cbDraw->setObjectName(QStringLiteral("cbDraw"));

        verticalLayout_2->addWidget(cbDraw);

        sbWinScore = new QSpinBox(gbGeneric);
        sbWinScore->setObjectName(QStringLiteral("sbWinScore"));
        sbWinScore->setValue(2);

        verticalLayout_2->addWidget(sbWinScore);

        sbDrawScore = new QSpinBox(gbGeneric);
        sbDrawScore->setObjectName(QStringLiteral("sbDrawScore"));
        sbDrawScore->setValue(1);

        verticalLayout_2->addWidget(sbDrawScore);


        verticalLayout_7->addWidget(gbGeneric);


        horizontalLayout->addLayout(verticalLayout_7);

        gbGroups = new QGroupBox(groupBox);
        gbGroups->setObjectName(QStringLiteral("gbGroups"));
        horizontalLayout_3 = new QHBoxLayout(gbGroups);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        grpCfgWidget = new GroupConfigWidget(gbGroups);
        grpCfgWidget->setObjectName(QStringLiteral("grpCfgWidget"));

        horizontalLayout_3->addWidget(grpCfgWidget);


        horizontalLayout->addWidget(gbGroups);

        gbSwiss = new QGroupBox(groupBox);
        gbSwiss->setObjectName(QStringLiteral("gbSwiss"));
        verticalLayout_5 = new QVBoxLayout(gbSwiss);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        label_4 = new QLabel(gbSwiss);
        label_4->setObjectName(QStringLiteral("label_4"));

        verticalLayout_5->addWidget(label_4);


        horizontalLayout->addWidget(gbSwiss);

        gbRandom = new QGroupBox(groupBox);
        gbRandom->setObjectName(QStringLiteral("gbRandom"));
        verticalLayout_9 = new QVBoxLayout(gbRandom);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        cbRandPairs = new QCheckBox(gbRandom);
        cbRandPairs->setObjectName(QStringLiteral("cbRandPairs"));

        verticalLayout_9->addWidget(cbRandPairs);


        horizontalLayout->addWidget(gbRandom);


        verticalLayout->addWidget(groupBox);

        gbPairs = new QGroupBox(CatTabWidget);
        gbPairs->setObjectName(QStringLiteral("gbPairs"));
        horizontalLayout_5 = new QHBoxLayout(gbPairs);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(9, 9, 9, 9);
        lwUnpaired = new QListWidget(gbPairs);
        lwUnpaired->setObjectName(QStringLiteral("lwUnpaired"));
        lwUnpaired->setSelectionMode(QAbstractItemView::MultiSelection);

        horizontalLayout_5->addWidget(lwUnpaired);

        verticalGroupBox_2 = new QGroupBox(gbPairs);
        verticalGroupBox_2->setObjectName(QStringLiteral("verticalGroupBox_2"));
        verticalLayout_8 = new QVBoxLayout(verticalGroupBox_2);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        btnPair = new QPushButton(verticalGroupBox_2);
        btnPair->setObjectName(QStringLiteral("btnPair"));

        verticalLayout_8->addWidget(btnPair);

        btnSplit = new QPushButton(verticalGroupBox_2);
        btnSplit->setObjectName(QStringLiteral("btnSplit"));

        verticalLayout_8->addWidget(btnSplit);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_8->addItem(verticalSpacer);

        line = new QFrame(verticalGroupBox_2);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_8->addWidget(line);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_8->addItem(verticalSpacer_2);

        btnRandomizeRemaining = new QPushButton(verticalGroupBox_2);
        btnRandomizeRemaining->setObjectName(QStringLiteral("btnRandomizeRemaining"));

        verticalLayout_8->addWidget(btnRandomizeRemaining);

        btnRandomizeAll = new QPushButton(verticalGroupBox_2);
        btnRandomizeAll->setObjectName(QStringLiteral("btnRandomizeAll"));

        verticalLayout_8->addWidget(btnRandomizeAll);

        cbAvoidSameTeam = new QCheckBox(verticalGroupBox_2);
        cbAvoidSameTeam->setObjectName(QStringLiteral("cbAvoidSameTeam"));

        verticalLayout_8->addWidget(cbAvoidSameTeam);


        horizontalLayout_5->addWidget(verticalGroupBox_2);

        lwPaired = new QListWidget(gbPairs);
        lwPaired->setObjectName(QStringLiteral("lwPaired"));
        lwPaired->setSelectionMode(QAbstractItemView::MultiSelection);

        horizontalLayout_5->addWidget(lwPaired);


        verticalLayout->addWidget(gbPairs);


        retranslateUi(CatTabWidget);
        QObject::connect(cbDraw, SIGNAL(toggled(bool)), CatTabWidget, SLOT(onCbDrawChanged(bool)));
        QObject::connect(sbDrawScore, SIGNAL(valueChanged(int)), CatTabWidget, SLOT(onDrawScoreChanged(int)));
        QObject::connect(sbWinScore, SIGNAL(valueChanged(int)), CatTabWidget, SLOT(onWinScoreChanged(int)));
        QObject::connect(btnPair, SIGNAL(clicked()), CatTabWidget, SLOT(onBtnPairClicked()));
        QObject::connect(btnSplit, SIGNAL(clicked()), CatTabWidget, SLOT(onBtnSplitClicked()));
        QObject::connect(rbgMatchType, SIGNAL(buttonClicked(int)), CatTabWidget, SLOT(onMatchTypeButtonClicked(int)));
        QObject::connect(rbgSex, SIGNAL(buttonClicked(int)), CatTabWidget, SLOT(onSexClicked(int)));
        QObject::connect(cbDontCare, SIGNAL(clicked()), CatTabWidget, SLOT(onDontCareClicked()));
        QObject::connect(btnAddCategory, SIGNAL(clicked()), CatTabWidget, SLOT(onBtnAddCatClicked()));
        QObject::connect(catTableView, SIGNAL(doubleClicked(QModelIndex)), catTableView, SLOT(onCategoryDoubleClicked(QModelIndex)));
        QObject::connect(cbMatchSystem, SIGNAL(currentIndexChanged(int)), CatTabWidget, SLOT(onMatchSystemChanged(int)));

        QMetaObject::connectSlotsByName(CatTabWidget);
    } // setupUi

    void retranslateUi(QDialog *CatTabWidget)
    {
        CatTabWidget->setWindowTitle(QApplication::translate("CatTabWidget", "CatTabWidget", 0));
        groupBox->setTitle(QApplication::translate("CatTabWidget", "Category Selection", 0));
        btnAddCategory->setText(QApplication::translate("CatTabWidget", "Add Category", 0));
        gbGeneric->setTitle(QApplication::translate("CatTabWidget", "Generic Settings", 0));
        label_2->setText(QApplication::translate("CatTabWidget", "Type:", 0));
        rbSingles->setText(QApplication::translate("CatTabWidget", "Singles", 0));
        rbDoubles->setText(QApplication::translate("CatTabWidget", "Doubles", 0));
        rbMixed->setText(QApplication::translate("CatTabWidget", "Mixed", 0));
        label->setText(QApplication::translate("CatTabWidget", "Sex:", 0));
        rbMen->setText(QApplication::translate("CatTabWidget", "Men", 0));
        rbLadies->setText(QApplication::translate("CatTabWidget", "Ladies", 0));
        cbDontCare->setText(QApplication::translate("CatTabWidget", "Don't Care", 0));
        cbDraw->setText(QApplication::translate("CatTabWidget", "Allow Draw", 0));
        sbWinScore->setSuffix(QString());
        sbWinScore->setPrefix(QApplication::translate("CatTabWidget", "Winner Score: ", 0));
        sbDrawScore->setSuffix(QString());
        sbDrawScore->setPrefix(QApplication::translate("CatTabWidget", "Draw Score: ", 0));
        gbGroups->setTitle(QApplication::translate("CatTabWidget", "Settings for Group Matches", 0));
        gbSwiss->setTitle(QApplication::translate("CatTabWidget", "Settings for Swiss Ladder", 0));
        label_4->setText(QApplication::translate("CatTabWidget", "TextLabel", 0));
        gbRandom->setTitle(QApplication::translate("CatTabWidget", "Settings for Random Matches", 0));
        cbRandPairs->setText(QApplication::translate("CatTabWidget", "New, random partners\n"
"for each round", 0));
        gbPairs->setTitle(QApplication::translate("CatTabWidget", "Player Pairs", 0));
        btnPair->setText(QApplication::translate("CatTabWidget", "Pair --->", 0));
        btnSplit->setText(QApplication::translate("CatTabWidget", "<--- Split", 0));
        btnRandomizeRemaining->setText(QApplication::translate("CatTabWidget", "Random --->", 0));
        btnRandomizeAll->setText(QApplication::translate("CatTabWidget", "<--- Randomize All --->", 0));
        cbAvoidSameTeam->setText(QApplication::translate("CatTabWidget", "Avoid same team pairs", 0));
    } // retranslateUi

};

namespace Ui {
    class CatTabWidget: public Ui_CatTabWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CATTABWIDGET_H
