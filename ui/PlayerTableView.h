/* 
 * File:   PlayerTableView.h
 * Author: volker
 *
 * Created on March 17, 2014, 8:19 PM
 */

#ifndef PLAYERTABLEVIEW_H
#define	PLAYERTABLEVIEW_H

#include "Tournament.h"
#include "delegates/PlayerItemDelegate.h"

#include <QTableView>
#include <QSortFilterProxyModel>

using namespace QTournament;

class PlayerTableView : public QTableView
{
  Q_OBJECT
  
public:
  PlayerTableView (QWidget* parent);
  virtual ~PlayerTableView ();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);
  QModelIndex mapToSource(const QModelIndex& proxyIndex);
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  PlayerItemDelegate* itemDelegate;

};

#endif	/* PLAYERTABLEVIEW_H */

