#include <QMessageBox>

#include "DlgConnectionSettings.h"
#include "ui_DlgConnectionSettings.h"

DlgConnectionSettings::DlgConnectionSettings(QWidget *parent, QString customUrl, QString customPubKey, int customTimeout_ms) :
  QDialog(parent),
  ui(new Ui::DlgConnectionSettings)
{
  ui->setupUi(this);

  // initialize the custom URL
  ui->cbUrl->setChecked(!(customUrl.isEmpty()));
  if (!(customUrl.isEmpty()))
  {
    ui->leUrl->setText(customUrl);
  } else {
    ui->leUrl->setEnabled(false);
  }

  // initialize the custom key
  ui->cbPubKey->setChecked(!(customPubKey.isEmpty()));
  if (!(customPubKey.isEmpty()))
  {
    ui->lePubKey->setText(customPubKey);
  } else {
    ui->lePubKey->setEnabled(false);
  }

  // initialize the timeout
  ui->cbTimeout->setChecked(customTimeout_ms > 0);
  if (customTimeout_ms > 0)
  {
    ui->sbTimeout->setValue(customTimeout_ms / 1000);
  } else {
    ui->sbTimeout->setEnabled(false);
  }
}

//----------------------------------------------------------------------------

DlgConnectionSettings::~DlgConnectionSettings()
{
  delete ui;
}

//----------------------------------------------------------------------------

QString DlgConnectionSettings::getCustomUrl() const
{
  return (ui->cbUrl->isChecked()) ? ui->leUrl->text() : "";
}

//----------------------------------------------------------------------------

QString DlgConnectionSettings::getCustomPubKey() const
{
  return (ui->cbPubKey->isChecked()) ? ui->lePubKey->text() : "";
}

//----------------------------------------------------------------------------

int DlgConnectionSettings::getCustomTimeout_ms() const
{
  return (ui->cbTimeout->isChecked()) ? (ui->sbTimeout->value() * 1000) : -1;
}

//----------------------------------------------------------------------------

bool DlgConnectionSettings::validateInputs()
{
  if ((ui->cbUrl->isChecked()) && (ui->leUrl->text().isEmpty()))
  {
    QString msg = tr("Please provide a custom URL or switch back to defaults!");
    QMessageBox::warning(this, tr("Connection Settings"), msg);
    return false;
  }

  if ((ui->cbPubKey->isChecked()) && (ui->lePubKey->text().isEmpty()))
  {
    QString msg = tr("Please provide a custom public key or switch back to defaults!");
    QMessageBox::warning(this, tr("Connection Settings"), msg);
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------

void DlgConnectionSettings::onCheckboxToggled()
{
  ui->leUrl->setEnabled(ui->cbUrl->isChecked());
  ui->lePubKey->setEnabled(ui->cbPubKey->isChecked());
  ui->sbTimeout->setEnabled(ui->cbTimeout->isChecked());
}

//----------------------------------------------------------------------------

void DlgConnectionSettings::onBtnOkayClicked()
{
  if (!(validateInputs())) return;

  accept();
}
