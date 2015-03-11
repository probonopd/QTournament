/* 
 * File:   PlayerTableView.h
 * Author: volker
 *
 * Created on March 17, 2014, 8:19 PM
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
  unique_ptr<Match> getSelectedMatch();
  void updateSelectionAfterDataChange();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);

private slots:
  void onSelectionChanged(const QItemSelection&selectedItem, const QItemSelection&deselectedItem);
  void onContextMenuRequested(const QPoint& pos);
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  MatchItemDelegate* itemDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actPostponeMatch;
  QAction* actWalkover;
  QAction* actCourtSubmenu;
  QMenu* courtSelectionMenu;

  void prepContextMenu();
  void updateCourtSelectionMenu();
};

#endif	/* MATCHTABLEVIEW_H */

