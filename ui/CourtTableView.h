/* 
 * File:   PlayerTableView.h
 * Author: volker
 *
 * Created on March 17, 2014, 8:19 PM
 */

#ifndef COURTTABLEVIEW_H
#define	COURTTABLEVIEW_H

#include <memory>

#include <QTableView>
#include <QSortFilterProxyModel>

#include "Tournament.h"
#include "delegates/CourtItemDelegate.h"

using namespace QTournament;

class CourtTableView : public QTableView
{
  Q_OBJECT
  
public:
  //enum class FilterType : std::int8_t { IDLE = 1, STAGED = 2, NONE = 0 };

  CourtTableView (QWidget* parent);
  virtual ~CourtTableView ();
  unique_ptr<Court> getSelectedCourt();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);

private slots:
  void onSelectionChanged(const QItemSelection&selectedItem, const QItemSelection&deselectedItem);
  void onContextMenuRequested(const QPoint& pos);
  void onActionAddCourtTriggered();

private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  CourtItemDelegate* itemDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actAddCourt;
  QAction* actWalkover;
  QAction* actUndoCall;
  QAction* actFinishMatch;
  QMenu* walkoverSelectionMenu;

  void initContextMenu();
  void updateContextMenu();
};

#endif	/* COURTTABLEVIEW_H */

