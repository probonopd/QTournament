#include "DlgPickPlayerSex.h"
#include "ui_DlgPickPlayerSex.h"

DlgPickPlayerSex::DlgPickPlayerSex(QWidget *parent, const QString& playerName) :
  QDialog(parent),
  ui(new Ui::DlgPickPlayerSex)
{
  ui->setupUi(this);

  if (playerName.isEmpty())
  {
    ui->lblHeadline->setVisible(false);
    ui->lblName->setVisible(false);
  } else {
    ui->lblName->setText(playerName);
  }
}

DlgPickPlayerSex::~DlgPickPlayerSex()
{
  delete ui;
}

SEX DlgPickPlayerSex::getSelectedSex()
{
  return selectedSex;
}

void DlgPickPlayerSex::onBtnMaleClicked()
{
  selectedSex = M;
  accept();
}

void DlgPickPlayerSex::onBtnFemaleClicked()
{
  selectedSex = F;
  accept();
}

