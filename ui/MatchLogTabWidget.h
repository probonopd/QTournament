#ifndef MATCHLOGTABWIDGET_H
#define MATCHLOGTABWIDGET_H

#include <QWidget>

#include "TournamentDB.h"

namespace Ui {
  class MatchLogTabWidget;
}


class MatchLogTabWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MatchLogTabWidget(QWidget *parent = nullptr);
  ~MatchLogTabWidget();

  void setDatabase(QTournament::TournamentDB* _db);

private:
  QTournament::TournamentDB* db;
  Ui::MatchLogTabWidget *ui;
};

#endif // MATCHLOGWIDGET_H
