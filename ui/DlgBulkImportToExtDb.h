/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DLGBULKIMPORTTOEXTDB_H
#define DLGBULKIMPORTTOEXTDB_H

#include <memory>

#include <QDialog>
#include <QTextDocument>

#include "TournamentDB.h"

namespace Ui {
  class DlgBulkImportToExtDb;
}

using namespace std;
using namespace QTournament;

class DlgBulkImportToExtDb : public QDialog
{
  Q_OBJECT

public:
  explicit DlgBulkImportToExtDb(TournamentDB* _db, QWidget *parent = 0);
  ~DlgBulkImportToExtDb();
  QString getText() const;
  int getTargetTeamId() const;
  int getTargetCatId() const;

private:
  Ui::DlgBulkImportToExtDb *ui;
  TournamentDB* db;
  void initDropBoxes();

private slots:
  void onTournamentAddStateChanged();
  void onCategoryAddStateChanged();
  void onTeamSelectionChanged();
  void onCatSelectionChanged();
  void updateImportButton();
};

#endif // DLGBULKIMPORTTOEXTDB_H
