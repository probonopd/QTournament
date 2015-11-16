#include "DlgPickPlayerSex.h"
#include "ui_DlgPickPlayerSex.h"

DlgPickPlayerSex::DlgPickPlayerSex(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgPickPlayerSex)
{
  ui->setupUi(this);


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

