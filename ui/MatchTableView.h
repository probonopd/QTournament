/* 
 * File:   PlayerTableView.h
 * Author: volker
 *
 * Created on March 17, 2014, 8:19 PM
 */

#ifndef MATCHTABLEVIEW_H
#define	MATCHTABLEVIEW_H

#include <memory>

#include "Tournament.h"
//#include "delegates/PlayerItemDelegate.h"

#include <QTableView>
#include <QSortFilterProxyModel>

using namespace QTournament;

class MatchTableView : public QTableView
{
  Q_OBJECT
  
public:
  //enum class FilterType : std::int8_t { IDLE = 1, STAGED = 2, NONE = 0 };

  MatchTableView (QWidget* parent);
  virtual ~MatchTableView ();
  //void setFilter(FilterType ft);
  //void clearFilter();
  unique_ptr<Match> getSelectedMatch();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);
  //void onFilterUpdateTriggered();
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  //PlayerItemDelegate* itemDelegate;
  //FilterType currentFilter;

};

#endif	/* MATCHTABLEVIEW_H */

