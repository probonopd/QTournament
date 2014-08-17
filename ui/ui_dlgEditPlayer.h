/********************************************************************************
** Form generated from reading UI file 'dlgEditPlayer.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGEDITPLAYER_H
#define UI_DLGEDITPLAYER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_dlgEditPlayer
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout_5;
    QLabel *label_2;
    QLineEdit *leFirstName;
    QLabel *label;
    QLineEdit *leLastName;
    QLabel *label_3;
    QComboBox *cbTeams;
    QVBoxLayout *verticalLayout_3;
    QRadioButton *rbMale;
    QRadioButton *rbFemale;
    QGroupBox *grpCategories;
    QHBoxLayout *horizontalLayout_3;
    QListWidget *catList;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *dlgEditPlayer)
    {
        if (dlgEditPlayer->objectName().isEmpty())
            dlgEditPlayer->setObjectName(QStringLiteral("dlgEditPlayer"));
        dlgEditPlayer->resize(612, 520);
        verticalLayout_2 = new QVBoxLayout(dlgEditPlayer);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox_2 = new QGroupBox(dlgEditPlayer);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_2);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(-1, -1, 15, -1);
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy);
        label_2->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        verticalLayout_5->addWidget(label_2);

        leFirstName = new QLineEdit(groupBox_2);
        leFirstName->setObjectName(QStringLiteral("leFirstName"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(2);
        sizePolicy1.setHeightForWidth(leFirstName->sizePolicy().hasHeightForWidth());
        leFirstName->setSizePolicy(sizePolicy1);
        leFirstName->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        verticalLayout_5->addWidget(leFirstName);

        label = new QLabel(groupBox_2);
        label->setObjectName(QStringLiteral("label"));
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        verticalLayout_5->addWidget(label);

        leLastName = new QLineEdit(groupBox_2);
        leLastName->setObjectName(QStringLiteral("leLastName"));
        sizePolicy1.setHeightForWidth(leLastName->sizePolicy().hasHeightForWidth());
        leLastName->setSizePolicy(sizePolicy1);
        leLastName->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        verticalLayout_5->addWidget(leLastName);

        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        verticalLayout_5->addWidget(label_3);

        cbTeams = new QComboBox(groupBox_2);
        cbTeams->setObjectName(QStringLiteral("cbTeams"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(2);
        sizePolicy2.setHeightForWidth(cbTeams->sizePolicy().hasHeightForWidth());
        cbTeams->setSizePolicy(sizePolicy2);

        verticalLayout_5->addWidget(cbTeams);


        verticalLayout_4->addLayout(verticalLayout_5);


        horizontalLayout->addLayout(verticalLayout_4);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(-1, -1, 15, -1);
        rbMale = new QRadioButton(groupBox_2);
        rbMale->setObjectName(QStringLiteral("rbMale"));

        verticalLayout_3->addWidget(rbMale);

        rbFemale = new QRadioButton(groupBox_2);
        rbFemale->setObjectName(QStringLiteral("rbFemale"));

        verticalLayout_3->addWidget(rbFemale);


        horizontalLayout->addLayout(verticalLayout_3);


        horizontalLayout_2->addLayout(horizontalLayout);


        verticalLayout->addWidget(groupBox_2);

        grpCategories = new QGroupBox(dlgEditPlayer);
        grpCategories->setObjectName(QStringLiteral("grpCategories"));
        horizontalLayout_3 = new QHBoxLayout(grpCategories);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        catList = new QListWidget(grpCategories);
        catList->setObjectName(QStringLiteral("catList"));

        horizontalLayout_3->addWidget(catList);


        verticalLayout->addWidget(grpCategories);

        buttonBox = new QDialogButtonBox(dlgEditPlayer);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(dlgEditPlayer);
        QObject::connect(buttonBox, SIGNAL(accepted()), dlgEditPlayer, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), dlgEditPlayer, SLOT(reject()));
        QObject::connect(rbMale, SIGNAL(clicked()), dlgEditPlayer, SLOT(onSexSelectionChanged()));
        QObject::connect(rbFemale, SIGNAL(clicked()), dlgEditPlayer, SLOT(onSexSelectionChanged()));

        QMetaObject::connectSlotsByName(dlgEditPlayer);
    } // setupUi

    void retranslateUi(QDialog *dlgEditPlayer)
    {
        dlgEditPlayer->setWindowTitle(QApplication::translate("dlgEditPlayer", "Edit Player Data", 0));
        groupBox_2->setTitle(QApplication::translate("dlgEditPlayer", "Personal Data", 0));
        label_2->setText(QApplication::translate("dlgEditPlayer", "First name:", 0));
        label->setText(QApplication::translate("dlgEditPlayer", "Last name:", 0));
        label_3->setText(QApplication::translate("dlgEditPlayer", "Team:", 0));
        rbMale->setText(QApplication::translate("dlgEditPlayer", "Male", 0));
        rbFemale->setText(QApplication::translate("dlgEditPlayer", "Female", 0));
        grpCategories->setTitle(QApplication::translate("dlgEditPlayer", "Categories", 0));
    } // retranslateUi

};

namespace Ui {
    class dlgEditPlayer: public Ui_dlgEditPlayer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGEDITPLAYER_H
