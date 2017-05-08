#ifndef DLGPICKCATEGORY_H
#define DLGPICKCATEGORY_H

#include <QDialog>

#include "Category.h"

namespace Ui {
  class DlgPickCategory;
}

namespace QTournament
{
  class TournamentDB;
}

class DlgPickCategory : public QDialog
{
  Q_OBJECT

public:
  DlgPickCategory(QWidget *parent, QTournament::TournamentDB* _db, QTournament::SEX sex = QTournament::DONT_CARE);
  void applyPreselection(const vector<QTournament::Category>& preSelection);
  void applyPreselection(const QString& commaSepCatNames);

  QString getSelection_CommaSep() const;
  vector<int> getSelection_Id() const;
  vector<QString> getSelection_strVec() const;
  vector<QTournament::Category> getSelection() const;

  ~DlgPickCategory();

private:
  Ui::DlgPickCategory *ui;
  QTournament::TournamentDB* db;
};

#endif // DLGPICKCATEGORY_H
