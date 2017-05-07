#ifndef DLGIMPORTCSV_STEP2_H
#define DLGIMPORTCSV_STEP2_H

#include <vector>
#include <string>

#include <QDialog>
#include <QString>

#include "AutoSizingTable.h"
#include "CSVImporter.h"
#include "Category.h"

using namespace std;

namespace QTournament {
  class TournamentDB;
}

//----------------------------------------------------------------------------

class CSVDataTableWidget : public GuiHelpers::AutoSizingTableWidget
{
  Q_OBJECT

public:
  CSVDataTableWidget(QWidget* parent = nullptr);
  void setData(QTournament::TournamentDB* _db, const vector<vector<string>>& _splitData);
  QString getErrMsg(int row, int col);

signals:
  void warnCountChanged(int errCount, int warnCount, int totalRowCount) const;

protected slots:
  void onCellDoubleClicked(int row, int col);

protected:
  void rebuildContents();
  void updateWarnings();
  void syncedCellUpdate(int row, int col, const string& txt);
  void syncedCellUpdate(int row, int col, const QString& txt);
  void insertDataOfExistingPlayers();
  void enforceConsistentSex(int specificRow = -1);


private:
  QTournament::TournamentDB* db;
  vector<vector<string>> splitData;
  vector<QTournament::CSVError> errList;
  vector<QTournament::Category> availCategories;
  vector<string> availCatNames;
};

//----------------------------------------------------------------------------

namespace Ui {
  class DlgImportCSV_Step2;
}

class DlgImportCSV_Step2 : public QDialog
{
  Q_OBJECT

public:
  explicit DlgImportCSV_Step2(QWidget *parent, QTournament::TournamentDB* _db, const vector<vector<string>>& initialData);
  ~DlgImportCSV_Step2();

public slots:
  void onWarnCountChanged(int errCount, int warnCount, int totalRowCount);

protected slots:
  void onCellSelectionChanged(int row, int col);

private:
  Ui::DlgImportCSV_Step2 *ui;
  QTournament::TournamentDB* db;
};

#endif // DLGIMPORTCSV_STEP2_H
