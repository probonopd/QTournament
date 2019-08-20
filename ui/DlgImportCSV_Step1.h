#ifndef DLGIMPORTCSV_STEP1_H
#define DLGIMPORTCSV_STEP1_H

#include <vector>
#include <string>

#include <Sloppy/String.h>

#include <QDialog>

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
  explicit DlgImportCSV_Step1(QWidget *parent, const QTournament::TournamentDB& _db);
  ~DlgImportCSV_Step1();
  std::vector<std::vector<Sloppy::estring> > getSplitData() const;


protected slots:
  void onContentChanged();
  void onBtnLoadFileClicked();

private:
  Ui::DlgImportCSV_Step1 *ui;
  std::reference_wrapper<const QTournament::TournamentDB> db;
};

#endif // DLGIMPORTCSV_STEP1_H
