#ifndef DLGIMPORTCSV_STEP1_H
#define DLGIMPORTCSV_STEP1_H

#include <vector>
#include <string>

#include <QDialog>

using namespace std;

namespace Ui {
  class DlgImportCSV_Step1;
}

namespace QTournament
{
  class TournamentDB;
}

class DlgImportCSV_Step1 : public QDialog
{
  Q_OBJECT

public:
  explicit DlgImportCSV_Step1(QWidget *parent, QTournament::TournamentDB* _db);
  ~DlgImportCSV_Step1();
  vector<vector<string>> getSplitData() const;


protected slots:
  void onContentChanged();

private:
  Ui::DlgImportCSV_Step1 *ui;
  QTournament::TournamentDB* db;
};

#endif // DLGIMPORTCSV_STEP1_H
