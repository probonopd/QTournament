#ifndef DLGPICKPLAYERSEX_H
#define DLGPICKPLAYERSEX_H

#include <QDialog>

#include "TournamentDataDefs.h"

namespace Ui {
  class DlgPickPlayerSex;
}

using namespace QTournament;

class DlgPickPlayerSex : public QDialog
{
  Q_OBJECT

public:
  explicit DlgPickPlayerSex(QWidget *parent, const QString& playerName);
  ~DlgPickPlayerSex();
  SEX getSelectedSex();

private slots:
  void onBtnMaleClicked();
  void onBtnFemaleClicked();

private:
  Ui::DlgPickPlayerSex *ui;
  SEX selectedSex = M;

};

#endif // DLGPICKPLAYERSEX_H
