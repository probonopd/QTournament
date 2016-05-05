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

#ifndef _CATTABWIDGET_H
#define	_CATTABWIDGET_H

#include <QMenu>
#include <QAction>

#include "ui_CatTabWidget.h"
#include "TournamentDB.h"

class CatTabWidget : public QDialog
{
  Q_OBJECT
public:
  CatTabWidget();
  virtual ~CatTabWidget();
  void setDatabase(TournamentDB* _db);

private:
  TournamentDB* db;
  Ui::CatTabWidget ui;
  void updateControls();
  void updatePairs();
  int unpairedPlayerId1;
  int unpairedPlayerId2;

  unique_ptr<QMenu> lwUnpairedContextMenu;
  QAction* actRemovePlayer;
  QAction* actBulkRemovePlayers;
  QAction* actAddPlayer;
  QAction* actRegister;
  QAction* actUnregister;
  unique_ptr<QMenu> listOfCats_CopyPlayerSubmenu;
  unique_ptr<QMenu> listOfCats_MovePlayerSubmenu;
  QAction* actCreateNewPlayerInCat;
  QAction* actImportPlayerToCat;

  unique_ptr<QMenu> lwPairsContextMenu;
  QAction* actSplitPair;
  unique_ptr<QMenu> listOfCats_CopyPairSubmenu;
  unique_ptr<QMenu> listOfCats_MovePairSubmenu;

  void initContextMenu();
  upPlayer lwUnpaired_getSelectedPlayer() const;
  unique_ptr<PlayerPair> lwPaired_getSelectedPair() const;

public slots:
  void onCatModelChanged();
  void onCatSelectionChanged(const QItemSelection &, const QItemSelection &);
  void onCbDrawChanged(bool newState);
  void onDrawScoreChanged(int newVal);
  void onWinScoreChanged(int newVal);
  void onUnpairedPlayersSelectionChanged();
  void onBtnPairClicked();
  void onPairedPlayersSelectionChanged();
  void onBtnSplitClicked();
  void onMatchTypeButtonClicked(int btn);
  void onSexClicked(int btn);
  void onDontCareClicked();
  void onBtnAddCatClicked();
  void onBtnRunCatClicked();
  void onMatchSystemChanged(int newId);
  void onGroupConfigChanged(const KO_Config& newCfg);
  void onPlayerAddedToCategory(const Player& p, const Category& c);
  void onPlayerRemovedFromCategory(const Player& p, const Category& c);
  void onPlayerRenamed(const Player& p);
  void onCatStateChanged(const Category& c, const OBJ_STATE fromState, const OBJ_STATE toState);
  void onPlayerStateChanged(int playerId, int seqNum, const OBJ_STATE fromState, const OBJ_STATE toState);
  void onRemovePlayerFromCat();
  void onBulkRemovePlayersFromCat();
  void onAddPlayerToCat();
  void onUnpairedContextMenuRequested(const QPoint& pos);
  void onPairedContextMenuRequested(const QPoint& pos);
  void onRegisterPlayer();
  void onUnregisterPlayer();
  void onCopyOrMovePlayer(int targetCatId, bool isMove);
  void onCopyOrMovePair(const PlayerPair& selPair, int targetCatId, bool isMove);
  void onCreatePlayer();
  void onImportPlayer();
  void onCategoryRemoved();
} ;

#endif	/* _CATTABWIDGET_H */
