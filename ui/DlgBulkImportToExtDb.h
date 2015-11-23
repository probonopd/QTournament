#ifndef DLGBULKIMPORTTOEXTDB_H
#define DLGBULKIMPORTTOEXTDB_H

#include <memory>

#include <QDialog>
#include <QTextDocument>

namespace Ui {
  class DlgBulkImportToExtDb;
}

using namespace std;

class DlgBulkImportToExtDb : public QDialog
{
  Q_OBJECT

public:
  explicit DlgBulkImportToExtDb(QWidget *parent = 0);
  ~DlgBulkImportToExtDb();
  QString getText() const;
  int getTargetTeamId() const;
  int getTargetCatId() const;

private:
  Ui::DlgBulkImportToExtDb *ui;
  void initDropBoxes();

private slots:
  void onTournamentAddStateChanged();
  void onCategoryAddStateChanged();
  void onTeamSelectionChanged();
  void onCatSelectionChanged();
  void updateImportButton();
};

#endif // DLGBULKIMPORTTOEXTDB_H
