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

#include <memory>

#include <QTextDocument>

#include "DlgBulkImportToExtDb.h"
#include "ui_DlgBulkImportToExtDb.h"
#include "TeamMngr.h"
#include "CatMngr.h"

using namespace QTournament;

DlgBulkImportToExtDb::DlgBulkImportToExtDb(TournamentDB* _db, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgBulkImportToExtDb), db(_db)
{
  ui->setupUi(this);

  initDropBoxes();
}

//----------------------------------------------------------------------------

DlgBulkImportToExtDb::~DlgBulkImportToExtDb()
{
  delete ui;
}

//----------------------------------------------------------------------------

QString DlgBulkImportToExtDb::getText() const
{
  return ui->textEdit->document()->toPlainText();
}

//----------------------------------------------------------------------------

int DlgBulkImportToExtDb::getTargetTeamId() const
{
  if (ui->cbAddToTournament->isChecked())
  {
    // this value is guaranteed to be valid, otherwise
    // the user couldn't have clicked the "import" button
    return ui->cbTeam->currentData().toInt();
  }

  return -1;
}

//----------------------------------------------------------------------------

int DlgBulkImportToExtDb::getTargetCatId() const
{
  if (ui->cbAddToCat->isChecked())
  {
    // this value is guaranteed to be valid, otherwise
    // the user couldn't have clicked the "import" button
    return ui->cbCat->currentData().toInt();
  }

  return -1;
}

//----------------------------------------------------------------------------

void DlgBulkImportToExtDb::initDropBoxes()
{
  // prepare a list of all teams
  ui->cbTeam->clear();
  TeamMngr tm{db};
  auto teamList = tm.getAllTeams();
  std::sort(teamList.begin(), teamList.end(), [](Team& t1, Team& t2) {
    return t1.getName() < t2.getName();
  });
  ui->cbTeam->addItem(tr("<Select team>"), -1);
  for (const Team& team : teamList)
  {
    ui->cbTeam->addItem(team.getName(), team.getId());
  }

  // prepare a list of all categories
  // that can potentially joined by new players
  ui->cbCat->clear();
  CatMngr cm{db};
  auto catList = cm.getAllCategories();
  std::sort(catList.begin(), catList.end(), [](Category& c1, Category& c2) {
    return c1.getName() < c2.getName();
  });
  ui->cbCat->addItem(tr("<Select category>"), -1);
  for (const Category& cat : catList)
  {
    if (!(cat.canAddPlayers())) continue;
    ui->cbCat->addItem(cat.getName(), cat.getId());
  }
}

//----------------------------------------------------------------------------

void DlgBulkImportToExtDb::onTournamentAddStateChanged()
{
  bool isOn = ui->cbAddToTournament->isChecked();

  ui->cbTeam->setEnabled(isOn);
  ui->cbAddToCat->setEnabled(isOn);

  updateImportButton();
}

//----------------------------------------------------------------------------

void DlgBulkImportToExtDb::onCategoryAddStateChanged()
{
  bool isOn = ui->cbAddToCat->isChecked();
  ui->cbCat->setEnabled(isOn);

  updateImportButton();
}

//----------------------------------------------------------------------------

void DlgBulkImportToExtDb::onTeamSelectionChanged()
{
  updateImportButton();
}

//----------------------------------------------------------------------------

void DlgBulkImportToExtDb::onCatSelectionChanged()
{
  updateImportButton();
}

void DlgBulkImportToExtDb::updateImportButton()
{
  bool doTeamImport = ui->cbAddToTournament->isChecked();
  bool hasValidTeam = (ui->cbTeam->currentData().toInt() > 0);
  bool doCatImport = ui->cbAddToCat->isChecked();
  bool hasValidCat = (ui->cbCat->currentData().toInt() > 0);

  bool permitImport = false;

  if (!doTeamImport && !doCatImport)
  {
    permitImport = true;
  }
  else if (doTeamImport && !doCatImport && hasValidTeam)
  {
    permitImport = true;
  }
  else if (doTeamImport && hasValidTeam && doCatImport && hasValidCat)
  {
    permitImport = true;
  }

  ui->btnImport->setEnabled(permitImport);
}
