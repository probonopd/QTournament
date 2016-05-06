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

#ifndef DLGTOURNAMENTSETTINGS_H
#define DLGTOURNAMENTSETTINGS_H

#include <memory>

#include <QDialog>

#include "TournamentDataDefs.h"

namespace Ui {
  class DlgTournamentSettings;
}

class DlgTournamentSettings : public QDialog
{
  Q_OBJECT

public:
  explicit DlgTournamentSettings(QWidget *parent = 0);
  ~DlgTournamentSettings();
  std::unique_ptr<QTournament::TournamentSettings> getTournamentSettings() const;

public slots:
  void onTournamentNameChanged();
  void onOrgaNameChanged();
  void onUmpireSelectionChanged();

private:
  Ui::DlgTournamentSettings *ui;
  void updateButtons();
};

#endif // DLGTOURNAMENTSETTINGS_H
