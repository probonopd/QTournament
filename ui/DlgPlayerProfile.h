#ifndef DLGPLAYERPROFILE_H
#define DLGPLAYERPROFILE_H

#include <QDialog>

#include "PlayerProfile.h"
#include "CommonMatchTableWidget.h"

namespace Ui {
  class DlgPlayerProfile;
}

namespace QTournament {
  class Player;
  class TournamentDB;
}

class DlgPlayerProfile : public QDialog
{
  Q_OBJECT

public:
  explicit DlgPlayerProfile(const QTournament::Player& _p, QWidget *parent = nullptr);
  ~DlgPlayerProfile();

protected:
  QTournament::Player p;
  std::reference_wrapper<const QTournament::TournamentDB> db;
  QTournament::PlayerProfile pp;

  void fillLabels();
  void fillTables();

private:
  Ui::DlgPlayerProfile *ui;
};

#endif // DLGPLAYERPROFILE_H
