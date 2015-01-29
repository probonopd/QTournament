/* 
 * File:   PlayerTableView.h
 * Author: volker
 *
 * Created on March 17, 2014, 8:19 PM
 */

#ifndef MATCHGROUPTABLEVIEW_H
#define	MATCHGROUPTABLEVIEW_H

#include <memory>

#include "Tournament.h"
//#include "delegates/PlayerItemDelegate.h"

#include <QTableView>
#include <QSortFilterProxyModel>

using namespace QTournament;

class MatchGroupTableView : public QTableView
{
  Q_OBJECT
  
public:
  enum class FilterType : std::int8_t { IDLE = 1, STAGED = 2, NONE = 0 };

  MatchGroupTableView (QWidget* parent);
  virtual ~MatchGroupTableView ();
  void setFilter(FilterType ft);
  void clearFilter();
  unique_ptr<MatchGroup> getSelectedMatchGroup();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);
  void onFilterUpdateTriggered();
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  //PlayerItemDelegate* itemDelegate;
  FilterType currentFilter;

};

#endif	/* MATCHGROUPTABLEVIEW_H */

