#ifndef DLGREGISTERTOURNAMENT_H
#define DLGREGISTERTOURNAMENT_H

#include <QDialog>

namespace Ui {
  class DlgRegisterTournament;
}

class DlgRegisterTournament : public QDialog
{
  Q_OBJECT

public:
  explicit DlgRegisterTournament(QWidget *parent, const QString& tnmtName, const QString club);
  ~DlgRegisterTournament();

protected slots:
  void onBtnOkayClicked();
  void onFirstDayChanged();
  void onLastDayChanged();

private:
  Ui::DlgRegisterTournament *ui;
};

#endif // DLGREGISTERTOURNAMENT_H
