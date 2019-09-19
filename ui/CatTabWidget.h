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

#ifndef _CATTABWIDGET_H
#define	_CATTABWIDGET_H

#include <QMenu>
#include <QAction>

#include "ui_CatTabWidget.h"
#include "TournamentDB.h"
#include "ui/delegates/CatTabPlayerItemDelegate.h"

#include "CustomMetatypes.h"

using namespace QTournament;

class CatTabWidget : public QDialog
{
  Q_OBJECT
public:
  CatTabWidget(QWidget* parent = nullptr);
  virtual ~CatTabWidget();
  void setDatabase(QTournament::TournamentDB* _db);

private:
  QTournament::TournamentDB* db;
  Ui::CatTabWidget ui;
  void updateControls();
  void updatePairs();
  int unpairedPlayerId1;
  int unpairedPlayerId2;

  std::unique_ptr<QMenu> lwUnpairedContextMenu;
  QAction* actRemovePlayer;
  QAction* actBulkRemovePlayers;
  QAction* actAddPlayer;
  QAction* actRegister;
  QAction* actUnregister;
  std::unique_ptr<QMenu> listOfCats_CopyPlayerSubmenu;
  std::unique_ptr<QMenu> listOfCats_MovePlayerSubmenu;
  QAction* actCreateNewPlayerInCat;
  QAction* actImportPlayerToCat;

  std::unique_ptr<QMenu> lwPairsContextMenu;
  QAction* actSplitPair;
  std::unique_ptr<QMenu> listOfCats_CopyPairSubmenu;
  std::unique_ptr<QMenu> listOfCats_MovePairSubmenu;

  void initContextMenu();
  std::optional<QTournament::Player> lwUnpaired_getSelectedPlayer() const;
  std::optional<QTournament::PlayerPair> lwPaired_getSelectedPair() const;

  std::unique_ptr<CatTabPlayerItemDelegate> playerItemDelegate;

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
  void onCatStateChanged(const Category& c, const ObjState fromState, const ObjState toState);
  void onPlayerStateChanged(int playerId, int seqNum, ObjState fromState, ObjState toState);
  void onRemovePlayerFromCat();
  void onBulkRemovePlayersFromCat();
  void onAddPlayerToCat();
  void onUnpairedContextMenuRequested(const QPoint& pos);
  void onPairedContextMenuRequested(const QPoint& pos);
  void onRegisterPlayer();
  void onUnregisterPlayer();
  void onCopyOrMovePlayer(int targetCatId, bool isMove);
  void onCopyOrMovePair(const QTournament::PlayerPair& selPair, int targetCatId, bool isMove);
  void onCreatePlayer();
  void onImportPlayer();
  void onCategoryRemoved();
  void onTwoIterationsChanged();
  void onBracketSysChanged(int newIndex);
  void onFirstRoundChanged(int newVal);
} ;

#endif	/* _CATTABWIDGET_H */
