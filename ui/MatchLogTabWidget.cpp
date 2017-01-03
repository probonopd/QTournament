#include "MatchLogTabWidget.h"
#include "ui_MatchLogTabWidget.h"

MatchLogTabWidget::MatchLogTabWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MatchLogTabWidget)
{
  ui->setupUi(this);

  setDatabase(nullptr);
}

//----------------------------------------------------------------------------

MatchLogTabWidget::~MatchLogTabWidget()
{
  delete ui;
}

//----------------------------------------------------------------------------

void MatchLogTabWidget::setDatabase(TournamentDB* _db)
{
  db = _db;

  ui->tableWidget->setDatabase(db);

  if (db != nullptr)
  {
    // things to do when we open a new tournament
  } else {
    // things to do when we close a tournament
  }

  setEnabled(db != nullptr);
}
