/* 
 * File:   PlayerTableView.h
 * Author: volker
 *
 * Created on March 17, 2014, 8:19 PM
 */

#ifndef MATCHGROUPTABLEVIEW_H
#define	MATCHGROUPTABLEVIEW_H

#include "Tournament.h"
//#include "delegates/PlayerItemDelegate.h"

#include <QTableView>

using namespace QTournament;

class MatchGroupTableView : public QTableView
{
  Q_OBJECT
  
public:
  MatchGroupTableView (QWidget* parent);
  virtual ~MatchGroupTableView ();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;
  //PlayerItemDelegate* itemDelegate;

};

#endif	/* MATCHGROUPTABLEVIEW_H */

