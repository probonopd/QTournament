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
#include "delegates/MatchItemDelegate.h"

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
  unique_ptr<Match> getSelectedMatch();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  MatchItemDelegate* itemDelegate;

};

#endif	/* MATCHTABLEVIEW_H */

