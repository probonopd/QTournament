/* 
 * File:   PlayerTableView.h
 * Author: volker
 *
 * Created on March 17, 2014, 8:19 PM
 */

#ifndef PLAYERTABLEVIEW_H
#define	PLAYERTABLEVIEW_H

#include <memory>

#include <QTableView>
#include <QSortFilterProxyModel>

#include "Tournament.h"
#include "delegates/PlayerItemDelegate.h"

using namespace QTournament;

class PlayerTableView : public QTableView
{
  Q_OBJECT
  
public:
  PlayerTableView (QWidget* parent);
  virtual ~PlayerTableView ();
  unique_ptr<Player> getSelectedPlayer() const;
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);
  QModelIndex mapToSource(const QModelIndex& proxyIndex);
  void onAddPlayerTriggered();
  void onEditPlayerTriggered();
  void onRemovePlayerTriggered();
  void onShowNextMatchesForPlayerTriggered();
  void onRegisterPlayerTriggered();
  void onUnregisterPlayerTriggered();

private slots:
  void onContextMenuRequested(const QPoint& pos);
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  PlayerItemDelegate* itemDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actAddPlayer;
  QAction* actEditPlayer;
  QAction* actRemovePlayer;
  QAction* actShowNextMatchesForPlayer;
  QAction* actRegister;
  QAction* actUnregister;


  void initContextMenu();
};

#endif	/* PLAYERTABLEVIEW_H */

