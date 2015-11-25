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

#ifndef MATCHTABLEVIEW_H
#define	MATCHTABLEVIEW_H

#include <memory>

#include <QTableView>
#include <QSortFilterProxyModel>
#include <QListWidget>
#include <QMenu>
#include <QAction>

#include "Tournament.h"
#include "delegates/MatchItemDelegate.h"

using namespace QTournament;

class MatchTableView : public QTableView
{
  Q_OBJECT
  
public:
  //enum class FilterType : std::int8_t { IDLE = 1, STAGED = 2, NONE = 0 };

  MatchTableView (QWidget* parent);
  virtual ~MatchTableView ();
  unique_ptr<Match> getSelectedMatch() const;
  void updateSelectionAfterDataChange();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);

private slots:
  void onSelectionChanged(const QItemSelection&selectedItem, const QItemSelection&deselectedItem);
  void onContextMenuRequested(const QPoint& pos);
  void onWalkoverP1Triggered();
  void onWalkoverP2Triggered();
  void onMatchDoubleClicked(const QModelIndex& index);

signals:
  void matchSelectionChanged(int newlySelectedMatchId);

private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  MatchItemDelegate* itemDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actPostponeMatch;
  QMenu* walkoverSelectionMenu;
  QMenu* courtSelectionMenu;
  QAction* actWalkoverP1;
  QAction* actWalkoverP2;

  void initContextMenu();
  void updateContextMenu();
  void execWalkover(int playerNum);
  void execCall(const Match& ma, const Court& co);
};

#endif	/* MATCHTABLEVIEW_H */

