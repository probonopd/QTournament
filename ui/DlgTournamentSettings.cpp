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

  // the next attribute is always "1" for the time being;
  // maybe I'll add the possibility of having a "team-free"
  // tournament sometime later
  bool useTeams = ui->cbUseTeams->isChecked();

  QTournament::TournamentSettings* result = new QTournament::TournamentSettings();
  result->organizingClub = orgName;
  result->tournamentName = tName;
  result->useTeams = useTeams;

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

