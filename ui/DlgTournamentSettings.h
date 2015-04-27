#ifndef DLGTOURNAMENTSETTINGS_H
#define DLGTOURNAMENTSETTINGS_H

#include <memory>

#include <QDialog>

#include "TournamentDataDefs.h"

namespace Ui {
  class DlgTournamentSettings;
}

class DlgTournamentSettings : public QDialog
{
  Q_OBJECT

public:
  explicit DlgTournamentSettings(QWidget *parent = 0);
  ~DlgTournamentSettings();
  std::unique_ptr<QTournament::TournamentSettings> getTournamentSettings() const;

public slots:
  void onTournamentNameChanged();
  void onOrgaNameChanged();

private:
  Ui::DlgTournamentSettings *ui;
  void updateButtons();
};

#endif // DLGTOURNAMENTSETTINGS_H
