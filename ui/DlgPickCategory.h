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
  DlgPickCategory(QWidget *parent, const QTournament::TournamentDB& _db, QTournament::Sex sex = QTournament::Sex::DontCare);
  void applyPreselection(const std::vector<QTournament::Category>& preSelection);
  void applyPreselection(const QString& commaSepCatNames);

  QString getSelection_CommaSep() const;
  std::vector<int> getSelection_Id() const;
  std::vector<QString> getSelection_strVec() const;
  std::vector<QTournament::Category> getSelection() const;

  ~DlgPickCategory();

private:
  Ui::DlgPickCategory *ui;
  std::reference_wrapper<const QTournament::TournamentDB> db;
};

#endif // DLGPICKCATEGORY_H
