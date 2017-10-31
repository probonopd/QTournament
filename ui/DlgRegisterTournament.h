#ifndef DLGREGISTERTOURNAMENT_H
#define DLGREGISTERTOURNAMENT_H

#include <QDialog>

namespace Ui {
  class DlgRegisterTournament;
}

// forward
namespace QTournament
{
  struct OnlineRegistrationData;
}

class DlgRegisterTournament : public QDialog
{
  Q_OBJECT

public:
  explicit DlgRegisterTournament(QWidget *parent, const QString& tnmtName, const QString club);
  ~DlgRegisterTournament();

  QTournament::OnlineRegistrationData getValidatedRegistrationData() const;

protected slots:
  void onBtnOkayClicked();
  void onFirstDayChanged();
  void onLastDayChanged();

private:
  Ui::DlgRegisterTournament *ui;
};

#endif // DLGREGISTERTOURNAMENT_H
