/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include <SqliteOverlay/KeyValueTab.h>

#include "DlgTournamentSettings.h"
#include "ui_DlgTournamentSettings.h"

#include "HelperFunc.h"

using namespace QTournament;

DlgTournamentSettings::DlgTournamentSettings(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgTournamentSettings), db(nullptr)
{
  ui->setupUi(this);

  fillRefereeComboBox(true);

  updateButtons();
}

//----------------------------------------------------------------------------

DlgTournamentSettings::DlgTournamentSettings(const QTournament::TournamentDB& _db, QWidget* parent) :
  QDialog(parent),
  ui(new Ui::DlgTournamentSettings), db(&_db)
{
  ui->setupUi(this);

  fillRefereeComboBox(false);

  // initialize the controls with the existing values
  SqliteOverlay::KeyValueTab cfg{*db, TabCfg};
  std::string tmp = cfg[CfgKey_TnmtOrga];
  ui->leOrgaClub->setText(stdString2QString(tmp));
  tmp = cfg[CfgKey_TnmtName];
  ui->leTournamentName->setText(stdString2QString(tmp));
  int tnmtDefaultRefereeModeId = cfg.getInt(CfgKey_DefaultRefereemode);
  int idx = ui->cbUmpire->findData(tnmtDefaultRefereeModeId);
  ui->cbUmpire->setCurrentIndex(idx);

  // hide the "number of courts" spin box because we only want
  // to show it when creating the tournament
  ui->laCourtCount->hide();
  ui->sbCourtCount->hide();
}

//----------------------------------------------------------------------------

DlgTournamentSettings::~DlgTournamentSettings()
{
  delete ui;
}

//----------------------------------------------------------------------------

std::optional<QTournament::TournamentSettings> DlgTournamentSettings::getTournamentSettings() const
{
  QString tName = ui->leTournamentName->text().trimmed();
  if (tName.isEmpty()) return {};

  QString orgName = ui->leOrgaClub->text().trimmed();
  if (orgName.isEmpty()) return {};

  int refereeModeId = ui->cbUmpire->currentData().toInt();
  if (refereeModeId < 0) return {};

  // the next attribute is always "1" for the time being;
  // maybe I'll add the possibility of having a "team-free"
  // tournament sometime later
  bool useTeams = true;

  QTournament::TournamentSettings result;
  result.organizingClub = orgName;
  result.tournamentName = tName;
  result.useTeams = useTeams;
  result.refereeMode = static_cast<QTournament::RefereeMode>(refereeModeId);

  return result;
}

//----------------------------------------------------------------------------

int DlgTournamentSettings::getCourtCount() const
{
  return ui->sbCourtCount->value();
}

//----------------------------------------------------------------------------

void DlgTournamentSettings::onTournamentNameChanged()
{
  updateButtons();
}

void DlgTournamentSettings::onOrgaNameChanged()
{
  updateButtons();
}

//----------------------------------------------------------------------------

void DlgTournamentSettings::onUmpireSelectionChanged()
{
  updateButtons();
}

//----------------------------------------------------------------------------

void DlgTournamentSettings::updateButtons()
{
  bool okayButtonEnabled = true;
  if (ui->leOrgaClub->text().trimmed().isEmpty())
  {
    okayButtonEnabled = false;
  }
  if (ui->leTournamentName->text().trimmed().isEmpty())
  {
    okayButtonEnabled = false;
  }
  if (ui->cbUmpire->currentData().toInt() < 0)
  {
    okayButtonEnabled = false;
  }

  ui->btnOkay->setEnabled(okayButtonEnabled);
}

//----------------------------------------------------------------------------

void DlgTournamentSettings::fillRefereeComboBox(bool includeSelectHint)
{
  ui->cbUmpire->clear();
  if (includeSelectHint) ui->cbUmpire->addItem(tr("<Please select>"), -1);
  ui->cbUmpire->addItem(tr("No umpires"), static_cast<int>(QTournament::RefereeMode::None));
  ui->cbUmpire->addItem(tr("Handwritten assignment"), static_cast<int>(QTournament::RefereeMode::HandWritten));
  ui->cbUmpire->addItem(tr("Pick from all players"), static_cast<int>(QTournament::RefereeMode::AllPlayers));
  ui->cbUmpire->addItem(tr("Pick from recent finishers"), static_cast<int>(QTournament::RefereeMode::RecentFinishers));
  ui->cbUmpire->addItem(tr("Pick from special team"), static_cast<int>(QTournament::RefereeMode::SpecialTeam));
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

