#include "DlgPlayerProfile.h"
#include "ui_DlgPlayerProfile.h"

DlgPlayerProfile::DlgPlayerProfile(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgPlayerProfile)
{
  ui->setupUi(this);
}

DlgPlayerProfile::~DlgPlayerProfile()
{
  delete ui;
}
