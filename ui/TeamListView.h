/* 
 * File:   TeamListView.h
 * Author: volker
 *
 * Created on March 15, 2014, 7:28 PM
 */

#ifndef TEAMLISTVIEW_H
#define	TEAMLISTVIEW_H

#include "Tournament.h"

#include <QListView>

using namespace QTournament;

class TeamListView : public QListView
{
  Q_OBJECT
  
public:
  TeamListView (QWidget* parent);
  virtual ~TeamListView();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;

};

#endif	/* TEAMLISTVIEW_H */

