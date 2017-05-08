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
  void setData(QTournament::TournamentDB* _db, const vector<QTournament::CSVImportRecord>& initialData);
  QString getErrMsg(int row, int col);
  vector<QTournament::CSVImportRecord> getRecords() const {return records; }
  size_t getNumRecords() const { return records.size(); }

signals:
  void warnCountChanged(int errCount, int warnCount, int totalRowCount) const;

protected slots:
  void onCellDoubleClicked(int row, int col);
  void onBtnDelRowClicked();
  void onBtnAddRowClicked();

protected:
  void rebuildContents();
  void updateWarnings();
  void createOrUpdateCellItem(int row, int col, const QString& txt);
  void createOrUpdateCellItem(int row, int col);
  void createOrUpdateCellItem(int row);

private:
  QTournament::TournamentDB* db;
  vector<QTournament::CSVImportRecord> records;
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
  explicit DlgImportCSV_Step2(QWidget *parent, QTournament::TournamentDB* _db, const vector<QTournament::CSVImportRecord>& initialData);
  ~DlgImportCSV_Step2();
  vector<QTournament::CSVImportRecord> getRecords() const;

public slots:
  void onWarnCountChanged(int errCount, int warnCount, int totalRowCount);

protected slots:
  void onCellSelectionChanged(int row, int col);

private:
  Ui::DlgImportCSV_Step2 *ui;
  QTournament::TournamentDB* db;
};

#endif // DLGIMPORTCSV_STEP2_H
