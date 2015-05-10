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
#include "Match.h"

using namespace QTournament;

class CourtTableView : public QTableView
{
  Q_OBJECT
  
public:
  //enum class FilterType : std::int8_t { IDLE = 1, STAGED = 2, NONE = 0 };

  CourtTableView (QWidget* parent);
  virtual ~CourtTableView ();
  unique_ptr<Court> getSelectedCourt() const;
  unique_ptr<Match> getSelectedMatch() const;

public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);

private slots:
  void onSelectionChanged(const QItemSelection&selectedItem, const QItemSelection&deselectedItem);
  void onContextMenuRequested(const QPoint& pos);
  void onActionAddCourtTriggered();
  void onWalkoverP1Triggered();
  void onWalkoverP2Triggered();

private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  CourtItemDelegate* itemDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actAddCourt;
  QAction* actUndoCall;
  QAction* actFinishMatch;
  QMenu* walkoverSelectionMenu;
  QAction* actWalkoverP1;
  QAction* actWalkoverP2;

  void initContextMenu();
  void updateContextMenu(bool isRowClicked);
  void execWalkover(int playerNum) const;
};

#endif	/* COURTTABLEVIEW_H */

