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

#include "DlgTournamentSettings.h"
#include "ui_DlgTournamentSettings.h"

DlgTournamentSettings::DlgTournamentSettings(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgTournamentSettings)
{
  ui->setupUi(this);

  // fill the combo box for the umpire mode
  ui->cbUmpire->clear();
  ui->cbUmpire->addItem(tr("<Please select>"), -1);
  ui->cbUmpire->addItem(tr("No umpires"), static_cast<int>(QTournament::REFEREE_MODE::NONE));
  ui->cbUmpire->addItem(tr("Handwritten assignment"), static_cast<int>(QTournament::REFEREE_MODE::HANDWRITTEN));
  ui->cbUmpire->addItem(tr("Pick from all players"), static_cast<int>(QTournament::REFEREE_MODE::ALL_PLAYERS));
  ui->cbUmpire->addItem(tr("Pick from recent losers"), static_cast<int>(QTournament::REFEREE_MODE::RECENT_LOSERS));
  ui->cbUmpire->addItem(tr("Pick from special team"), static_cast<int>(QTournament::REFEREE_MODE::SPECIAL_TEAM));

  updateButtons();
}

//----------------------------------------------------------------------------

DlgTournamentSettings::~DlgTournamentSettings()
{
  delete ui;
}

std::unique_ptr<QTournament::TournamentSettings> DlgTournamentSettings::getTournamentSettings() const
{
  QString tName = ui->leTournamentName->text().trimmed();
  if (tName.isEmpty()) return nullptr;

  QString orgName = ui->leOrgaClub->text().trimmed();
  if (orgName.isEmpty()) return nullptr;

  int refereeModeId = ui->cbUmpire->currentData().toInt();
  if (refereeModeId < 0) return nullptr;

  // the next attribute is always "1" for the time being;
  // maybe I'll add the possibility of having a "team-free"
  // tournament sometime later
  bool useTeams = ui->cbUseTeams->isChecked();

  QTournament::TournamentSettings* result = new QTournament::TournamentSettings();
  result->organizingClub = orgName;
  result->tournamentName = tName;
  result->useTeams = useTeams;
  result->refereeMode = static_cast<QTournament::REFEREE_MODE>(refereeModeId);

  return std::unique_ptr<QTournament::TournamentSettings>(result);
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

