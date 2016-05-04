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

#ifndef _DLGEDITPLAYER_H
#define	_DLGEDITPLAYER_H

#include "ui_dlgEditPlayer.h"

#include "Player.h"
#include "ExternalPlayerDB.h"
#include "TournamentDB.h"

using namespace QTournament;

class DlgEditPlayer : public QDialog
{
  Q_OBJECT
public:
  DlgEditPlayer (TournamentDB* _db, QWidget *parent, Player* _selectedPlayer = nullptr);
  DlgEditPlayer (TournamentDB* _db, QWidget *parent, SEX _sexPreset, const Category& _catPreset);
  DlgEditPlayer (TournamentDB* _db, QWidget *parent, const ExternalPlayerDatabaseEntry& nameAndSexPreset, int _presetCatId=-1);
  virtual ~DlgEditPlayer ();
  QString getFirstName();
  QString getLastName();
  bool hasNameChange();
  SEX getSex();
  Team getTeam();
  QHash<Category, bool> getCategoryCheckState();
  
private:
  Ui::dlgEditPlayer ui;
  TournamentDB* db;
  Player* selectedPlayer;
  void initFromPlayerData();
  void initTeamList();
  bool _hasNameChange;
  void updateCatList(QHash<Category, CAT_ADD_STATE> catStatus, int preselectCatId = -1);

  SEX sexPreset;
  int presetCatId = -1;

public slots:
  virtual void done (int result);
  
private slots:
  void onSexSelectionChanged(int preselectCatId = -1);
};

#endif	/* _DLGEDITPLAYER_H */
