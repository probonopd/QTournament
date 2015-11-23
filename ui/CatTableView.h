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
  void onAddCategory();
  void onRemoveCategory();
  void onRunCategory();
  void onCloneCategory();
  void onAddPlayers();
  void onRemovePlayers();
  void onCreatePlayer();
  void onImportPlayer();
  
private slots:
  void onContextMenuRequested(const QPoint& pos);

private:
  Tournament* tnmt;
  QStringListModel* emptyModel;

  unique_ptr<QMenu> contextMenu;
  QAction* actAddCategory;
  QAction* actCloneCategory;
  QAction* actRunCategory;
  QAction* actRemoveCategory;
  QAction* actAddPlayer;
  QAction* actRemovePlayer;
  QAction* actCreateNewPlayerInCat;
  QAction* actImportPlayerToCat;

  void initContextMenu();

  void handleIntermediateSeedingForSelectedCat();
  bool unfreezeAndCleanup(unique_ptr<Category> selectedCat);

signals:
  void catModelChanged();

};

#endif	/* PLAYERTABLEVIEW_H */

