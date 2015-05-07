#ifndef DLGMATCHRESULT_H
#define DLGMATCHRESULT_H

#include <QDialog>

#include "Match.h"

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

private slots:
  void onGameScoreSelectionChanged();

private:
  Ui::DlgMatchResult *ui;
  Match ma;
  void updateControls();
};

#endif // DLGMATCHRESULT_H
