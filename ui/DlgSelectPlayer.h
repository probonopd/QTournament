/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#ifndef DLGSELECTPLAYER_H
#define DLGSELECTPLAYER_H

#include <QDialog>
#include <QListWidgetItem>

#include "TournamentDB.h"
#include "Player.h"
#include "Category.h"

namespace Ui {
  class DlgSelectPlayer;
}

using namespace QTournament;

class DlgSelectPlayer : public QDialog
{
  Q_OBJECT

public:
  enum class DLG_CONTEXT
  {
    ADD_TO_CATEGORY,
    REMOVE_FROM_CATEGORY,
    NONE
  };

  explicit DlgSelectPlayer(TournamentDB* _db, QWidget *parent = 0, DLG_CONTEXT _ctxt = DLG_CONTEXT::NONE, Category* _cat = nullptr);
  ~DlgSelectPlayer();
  PlayerList getSelectedPlayers() const;

public slots:
  virtual int exec() override;

private:
  Ui::DlgSelectPlayer *ui;
  TournamentDB* db;
  DLG_CONTEXT ctxt;
  Category* cat;

private slots:
  void onItemSelectionChanged();
  void onItemDoubleClicked(QListWidgetItem* item);
};

#endif // DLGSELECTPLAYER_H
