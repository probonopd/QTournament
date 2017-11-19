#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QRegularExpression>

#include "DlgPassword.h"
#include "ui_DlgPassword.h"

DlgPassword::DlgPassword(QWidget *parent, int dlgMode) :
  QDialog(parent),
  ui(new Ui::DlgPassword), mode{dlgMode}
{
  ui->setupUi(this);

  // activate / deactivate widgets, based on the
  // requested dialog mode
  switch (dlgMode)
  {
  case DlgMode_SetNewPassword:
    ui->laCurPw->hide();
    ui->leCurPw->hide();
    break;

  case DlgMode_ChangePassword:
    ui->laInfo->setText(tr("Please enter your current and your new password:"));
    break;

  case DlgMode_EnterPassword:
    ui->laNewPw->hide();
    ui->laReapeatPw->hide();
    ui->leNewPw->hide();
    ui->leRepeatPw->hide();
    ui->laRules->hide();
    ui->laInfo->setText(tr("Please enter your current password:"));
    break;

  default:
    throw std::invalid_argument("DlgPassword: ctor called with invalid mode!");
  }

  // initialize the red hint label
  onLineEditContentChange();
}

//----------------------------------------------------------------------------

DlgPassword::~DlgPassword()
{
  delete ui;
}

//----------------------------------------------------------------------------

QString DlgPassword::getOldPassword() const
{
  if (mode != DlgMode_ChangePassword) return "";

  return ui->leCurPw->text();
}

//----------------------------------------------------------------------------

QString DlgPassword::getNewPassword() const
{
  if (mode == DlgMode_EnterPassword) return "";

  return ui->leNewPw->text();
}

//----------------------------------------------------------------------------

QString DlgPassword::getCurrentPassword() const
{
  if (mode == DlgMode_SetNewPassword) return "";

  return ui->leCurPw->text();
}

//----------------------------------------------------------------------------

void DlgPassword::onBtnOkayClicked()
{
  accept();
}

//----------------------------------------------------------------------------

void DlgPassword::onLineEditContentChange()
{
  bool isValid = false;

  switch (mode)
  {
  case DlgMode_ChangePassword:
    isValid = validateInput_PasswordChangeMode();
    break;

  case DlgMode_EnterPassword:
    isValid = validateInput_PasswordEnterMode();
    break;

  default:
    isValid = validateInput_NewPasswordMode();
  }

  ui->btnOkay->setEnabled(isValid);
  ui->laHint->setVisible(!isValid);
}

//----------------------------------------------------------------------------

bool DlgPassword::validateInput_NewPasswordMode()
{
  if (ui->leNewPw->text().isEmpty())
  {
    ui->laHint->setText(tr("Hint: please enter a new password"));
    return false;
  }

  if (ui->leNewPw->text() != ui->leRepeatPw->text())
  {
    ui->laHint->setText(tr("Hint: the two passwords do not match"));
    return false;
  }

  return checkPasswordRules(ui->leNewPw->text());
}

//----------------------------------------------------------------------------

bool DlgPassword::validateInput_PasswordChangeMode()
{
  if (ui->leCurPw->text().isEmpty())
  {
    ui->laHint->setText(tr("Hint: please enter the current password"));
    return false;
  }

  return validateInput_NewPasswordMode();
}

//----------------------------------------------------------------------------

bool DlgPassword::validateInput_PasswordEnterMode()
{
  if (ui->leCurPw->text().isEmpty())
  {
    ui->laHint->setText(tr("Hint: please enter the current password"));
    return false;
  }

  return validateInput_NewPasswordMode();
}

//----------------------------------------------------------------------------

bool DlgPassword::checkPasswordRules(const QString& pw)
{
  if (pw.length() < MinPasswordLen)
  {
    QString msg = tr("Hint: the password should have at least %1 characters");
    msg = msg.arg(MinPasswordLen);
    ui->laHint->setText(msg);
    return false;
  }

  QRegularExpression reDigit{"\\d"};
  QRegularExpression reChar{"[a-z]", QRegularExpression::CaseInsensitiveOption};

  if (!(pw.contains(reDigit)))
  {
    QString msg = tr("Hint: the password must contain at least one digit");
    ui->laHint->setText(msg);
    return false;
  }

  if (!(pw.contains(reChar)))
  {
    QString msg = tr("Hint: the password must contain at least one letter");
    ui->laHint->setText(msg);
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------

