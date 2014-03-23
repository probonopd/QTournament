/*
 * File:   PlayerTabWidget.cpp
 * Author: volker
 *
 * Created on March 19, 2014, 7:38 PM
 */

#include <QMessageBox>

#include "PlayerTabWidget.h"
#include "dlgEditPlayer.h"

PlayerTabWidget::PlayerTabWidget(QWidget* parent)
:QWidget(parent)
{
  ui.setupUi(this);
}

//----------------------------------------------------------------------------

PlayerTabWidget::~PlayerTabWidget()
{
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onCreatePlayerClicked()
{
  DlgEditPlayer dlg;
  
  dlg.setModal(true);
  int result = dlg.exec();
  
  if (result != QDialog::Accepted)
  {
    return;
  }
  
  // we can be sure that all selected data in the dialog
  // is valid. That has been checked before the dialog
  // returns with "Accept". So we can directly step
  // into the creation of the new player
  ERR e = Tournament::getPlayerMngr()->createNewPlayer(
                                                       dlg.getFirstName(),
                                                       dlg.getLastName(),
                                                       dlg.getSex(),
                                                       dlg.getTeam().getName()
                                                       );
  
  if (e != OK)
  {
    QString msg = tr("Something went wrong when inserting the player. This shouldn't happen.");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int>(e));
    QMessageBox::warning(this, tr("WTF??"), msg);
    return;
  }
  Player p = Tournament::getPlayerMngr()->getPlayer(dlg.getFirstName(), dlg.getLastName());
  
  // assign the player to the selected categories
  //
  // we can be sure that all selected categories in the dialog
  // are valid. That has been checked upon creation of the "selectable"
  // category entries. So we can directly step
  // into the assignment of the categories
  CatMngr* cmngr = Tournament::getCatMngr();
  
  QHash<Category, bool> catSelection = dlg.getCategoryCheckState();
  QHash<Category, bool>::const_iterator it = catSelection.constBegin();
  
  while (it != catSelection.constEnd()) {
    if (it.value()) {
      Category cat = it.key();
      ERR e = cmngr->addPlayerToCategory(p, cat);
      
      if (e != OK) {
	QString msg = tr("Something went wrong when adding the player to a category. This shouldn't happen.");
	msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
	QMessageBox::warning(this, tr("WTF??"), msg);
      }
    }
    ++it;
  }
  
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onPlayerDoubleClicked(const QModelIndex& index)
{
  if (!(index.isValid()))
  {
    return;
  }
  
  Player selectedPlayer = Tournament::getPlayerMngr()->getPlayerBySeqNum(index.row());
  
  DlgEditPlayer dlg(&selectedPlayer);
  
  dlg.setModal(true);
  int result = dlg.exec();
  
  if (result != QDialog::Accepted)
  {
    return;
  }
  
  //
  // apply the changes, if any
  //
  
  // name changes
  if (dlg.hasNameChange())
  {
    ERR e = selectedPlayer.rename(dlg.getFirstName(), dlg.getLastName());

    if (e != OK)
    {
      QString msg = tr("Something went wrong when renaming the player. This shouldn't happen.");
      msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
      QMessageBox::warning(this, tr("WTF??"), msg);
    }
  }
  
  // category changes
  CatMngr* cmngr = Tournament::getCatMngr();
  
  QHash<Category, bool> catSelection = dlg.getCategoryCheckState();
  QHash<Category, bool>::const_iterator it = catSelection.constBegin();
  while (it != catSelection.constEnd()) {
    Category cat = it.key();
    bool isAlreadyInCat = cat.hasPlayer(selectedPlayer);
    bool isCatSelected = it.value();
    
    if (isAlreadyInCat && !isCatSelected) {    // remove player from category
      ERR e = cmngr->removePlayerFromCategory(selectedPlayer, cat);
      
      if (e != OK) {
	QString msg = tr("Something went wrong when removing the player from a category. This shouldn't happen.");
	msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
	QMessageBox::warning(this, tr("WTF??"), msg);
      }
    }
    
    if (!isAlreadyInCat && isCatSelected) {    // add player to category
      ERR e = cmngr->addPlayerToCategory(selectedPlayer, cat);
      
      if (e != OK) {
	QString msg = tr("Something went wrong when adding the player to a category. This shouldn't happen.");
	msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
	QMessageBox::warning(this, tr("WTF??"), msg);
      }
    }
    ++it;
  }
  
  // Team changes
  TeamMngr* tmngr = Tournament::getTeamMngr();
  Team newTeam = dlg.getTeam();
  if (newTeam != selectedPlayer.getTeam())
  {
    ERR e = tmngr->changeTeamAssigment(selectedPlayer, newTeam);

    if (e != OK) {
      QString msg = tr("Something went wrong when changing the player's team assignment. This shouldn't happen.");
      msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
      QMessageBox::warning(this, tr("WTF??"), msg);
    }
  }
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

