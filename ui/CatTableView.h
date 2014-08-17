/* 
 * File:   PlayerTableView.h
 * Author: volker
 *
 * Created on March 17, 2014, 8:19 PM
 */

#ifndef CATTABLEVIEW_H
#define	CATTABLEVIEW_H

#include "Tournament.h"

#include <QTableView>

using namespace QTournament;

class CategoryTableView : public QTableView
{
  Q_OBJECT
  
public:
  CategoryTableView (QWidget* parent);
  virtual ~CategoryTableView ();
  bool isEmptyModel();
  Category getSelectedCategory();
  bool hasCategorySelected();
  
public slots:
  void onTournamentClosed();
  void onTournamentOpened(Tournament* tnmt);
  void onCategoryDoubleClicked(const QModelIndex& index);
  
private:
  Tournament* tnmt;
  QStringListModel* emptyModel;

signals:
  void catModelChanged();

};

#endif	/* PLAYERTABLEVIEW_H */

