#ifndef MATCHLOGTABWIDGET_H
#define MATCHLOGTABWIDGET_H

#include <QWidget>

#include "TournamentDB.h"

namespace Ui {
  class MatchLogTabWidget;
}

using namespace QTournament;

class MatchLogTabWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MatchLogTabWidget(QWidget *parent = 0);
  ~MatchLogTabWidget();

  void setDatabase(TournamentDB* _db);

private:
  TournamentDB* db;
  Ui::MatchLogTabWidget *ui;
};

#endif // MATCHLOGWIDGET_H
