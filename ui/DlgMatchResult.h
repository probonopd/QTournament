#ifndef DLGMATCHRESULT_H
#define DLGMATCHRESULT_H

#include <memory>

#include <QDialog>

#include "Match.h"
#include "Score.h"

namespace Ui {
  class DlgMatchResult;
}

using namespace QTournament;

class DlgMatchResult : public QDialog
{
  Q_OBJECT

public:
  explicit DlgMatchResult(QWidget *parent, const Match& _ma);
  ~DlgMatchResult();
  unique_ptr<MatchScore> getMatchScore() const;


private slots:
  void onGameScoreSelectionChanged();

private:
  Ui::DlgMatchResult *ui;
  Match ma;
  void updateControls();
  bool isGame3Necessary() const;
  bool hasValidResult() const;
};

#endif // DLGMATCHRESULT_H
