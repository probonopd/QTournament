#ifndef DLGPICKTEAM_H
#define DLGPICKTEAM_H

#include <QDialog>
#include <QLineEdit>

namespace QTournament
{
  class TournamentDB;
}

namespace Ui {
  class DlgPickTeam;
}

//----------------------------------------------------------------------------

class LineEditWithHelpText : public QLineEdit
{
  Q_OBJECT

public:
  LineEditWithHelpText(QWidget* parent);
  void setHelpText(const QString& txt);
  bool hasValidText() const { return !showsHelpText; }

protected:
  void focusInEvent(QFocusEvent* ev) override;

private:
  bool showsHelpText;
};

//----------------------------------------------------------------------------

class DlgPickTeam : public QDialog
{
  Q_OBJECT

public:
  explicit DlgPickTeam(QWidget *parent, QTournament::TournamentDB* _db);
  ~DlgPickTeam();
  int getSelectedTeamId() const { return selectedTeamId; }

protected slots:
  void updateControls();
  void onFinishedRequested();
  bool canFinish() const;

private:
  Ui::DlgPickTeam *ui;
  QTournament::TournamentDB* db;
  int selectedTeamId;
  bool leShowsHelpText;
};

#endif // DLGPICKTEAM_H
