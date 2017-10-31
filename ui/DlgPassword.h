#ifndef DLGPASSWORD_H
#define DLGPASSWORD_H

#include <QDialog>

namespace Ui {
  class DlgPassword;
}

class DlgPassword : public QDialog
{
  Q_OBJECT

public:
  static constexpr int DlgMode_SetNewPassword = 0;
  static constexpr int DlgMode_ChangePassword = 1;
  static constexpr int DlgMode_EnterPassword = 2;
  static constexpr int MinPasswordLen = 6;

  explicit DlgPassword(QWidget *parent, int dlgMode);
  ~DlgPassword();

  QString getOldPassword() const;
  QString getNewPassword() const;
  QString getCurrentPassword() const;

protected slots:
  void onBtnOkayClicked();
  void onLineEditContentChange();

protected:
  bool validateInput_NewPasswordMode();
  bool validateInput_PasswordChangeMode();
  bool validateInput_PasswordEnterMode();
  bool checkPasswordRules(const QString& pw);

private:
  Ui::DlgPassword *ui;
  int mode;
};

#endif // DLGPASSWORD_H
