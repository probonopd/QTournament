/*
 * File:   CatTabWidget.cpp
 * Author: volker
 *
 * Created on March 24, 2014, 7:13 PM
 */

#include <qt/QtWidgets/qmessagebox.h>
#include <qt4/QtCore/qnamespace.h>

#include "CatTabWidget.h"

CatTabWidget::CatTabWidget()
{
  ui.setupUi(this);
  
  // connect to the view's signal that tells us that the model has changed
  connect(ui.catTableView, &CategoryTableView::catModelChanged, this, &CatTabWidget::onCatModelChanged);
  
  // connect the selection change signal of the two list widgets
  connect(ui.lwUnpaired, &QListWidget::itemSelectionChanged, this, &CatTabWidget::onUnpairedPlayersSelectionChanged);
  connect(ui.lwPaired, &QListWidget::itemSelectionChanged, this, &CatTabWidget::onPairedPlayersSelectionChanged);

  // hide unused settings groups
  ui.gbGroups->hide();
  ui.gbSwiss->hide();
  
  // initialize the entries in the drop box
  ui.cbMatchSystem->addItem(tr("Swiss ladder"), static_cast<int>(SWISS_LADDER));
  ui.cbMatchSystem->addItem(tr("Group matches with KO rounds"), static_cast<int>(GROUPS_WITH_KO));
  ui.cbMatchSystem->addItem(tr("Random matches (for fun tournaments)"), static_cast<int>(RANDOMIZE));
  ui.cbMatchSystem->addItem(tr("Tree-like ranking system"), static_cast<int>(RANKING));
}

//----------------------------------------------------------------------------
    
CatTabWidget::~CatTabWidget()
{
}

//----------------------------------------------------------------------------

void CatTabWidget::onCatModelChanged()
{
  // react on selection changes in the category table
  connect(ui.catTableView->selectionModel(),
	  SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	  SLOT(onTabSelectionChanged(const QItemSelection&, const QItemSelection&)));

  updateControls();
  updatePairs();
}

//----------------------------------------------------------------------------

void CatTabWidget::onTabSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  updatePairs();
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::updateControls()
{
  bool isActive = !(ui.catTableView->isEmptyModel());
  if (isActive)
  {
    QModelIndexList indexes = ui.catTableView->selectionModel()->selection().indexes();
    isActive = (indexes.count() > 0);
  }
  
  if (!isActive)
  {
    ui.gbGeneric->setEnabled(false);
    ui.gbGroups->hide();
    ui.gbSwiss->hide();
    ui.gbRandom->hide();
    return;
  }
  
  //
  // if made it to this point, we can be sure to have a valid category selected
  //
  Category selectedCat = ui.catTableView->getSelectedCategory();
  SEX sex = selectedCat.getSex();
  MATCH_TYPE mt = selectedCat.getMatchType();

  ui.gbGeneric->setEnabled(true);
  
  // update the list box showing the match system
  int matchSysId = static_cast<int>(selectedCat.getMatchSystem());
  ui.cbMatchSystem->setCurrentIndex(ui.cbMatchSystem->findData(matchSysId, Qt::UserRole));
  
  // update the match type
  ui.rbSingles->setChecked(mt == SINGLES);
  ui.rbDoubles->setChecked(mt == DOUBLES);
  ui.rbMixed->setChecked(mt == MIXED);
  
  // disable radio buttons for male / female for mixed categories
  ui.rbMen->setEnabled(mt != MIXED);
  ui.rbLadies->setEnabled(mt != MIXED);
  if (mt == MIXED)
  {
    ui.rbMen->hide();
    ui.rbLadies->hide();
  } else {
    ui.rbMen->show();
    ui.rbLadies->show();
  }
  
  // update the applicable sex; this requires an extra hack if the sex is
  // set to "don't care", because in this case, both radio buttons in a
  // radio button group have to be deactivated
  if (sex == DONT_CARE)
  {
    ui.rbgSex->setExclusive(false);
    ui.rbMen->setChecked(false);
    ui.rbLadies->setChecked(false);
    ui.rbgSex->setExclusive(true);
  } else {
    ui.rbMen->setChecked((sex == M) && (mt != MIXED));
    ui.rbLadies->setChecked((sex == F) && (mt != MIXED));
  }
  ui.cbDontCare->setChecked(sex == DONT_CARE);
  
  // the "accept draw" checkbox
  bool allowDraw = selectedCat.getParameter(ALLOW_DRAW).toBool();
  ui.cbDraw->setChecked(allowDraw);
  
  // the score spinnboxes
  int drawScore = selectedCat.getParameter(DRAW_SCORE).toInt();
  int winScore = selectedCat.getParameter(WIN_SCORE).toInt();
  ui.sbDrawScore->setValue(drawScore);
  ui.sbWinScore->setValue(winScore);
  if (allowDraw)
  {
    ui.sbWinScore->setMinimum(drawScore + 1);
    ui.sbWinScore->setMaximum(99);
    ui.sbDrawScore->setMaximum(winScore - 1);
    ui.sbDrawScore->setMinimum(1);
    ui.sbDrawScore->show();
  } else {
    ui.sbWinScore->setMinimum(1);
    ui.sbWinScore->setMaximum(99);
    ui.sbDrawScore->hide();
  }
  
  // group box for configuring player pairs
  if (mt == SINGLES)
  {
    ui.gbPairs->setEnabled(false);
  } else {
    ui.gbPairs->setEnabled(true);    
  }
}

//----------------------------------------------------------------------------

void CatTabWidget::onCbDrawChanged(bool newState)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(ALLOW_DRAW, newState);
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onWinScoreChanged(int newVal)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(WIN_SCORE, newVal);
  updateControls();
}

//----------------------------------------------------------------------------
    
void CatTabWidget::onDrawScoreChanged(int newVal)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(DRAW_SCORE, newVal);
  updateControls();
}

//----------------------------------------------------------------------------

/**
 * Updates the list widgets with the player pairs. Should only be called
 * when the selected category changes.
 * 
 * This method disables the pairing buttons.
 */
void CatTabWidget::updatePairs()
{
  // remove all entries
  ui.lwUnpaired->clear();
  ui.lwPaired->clear();
  unpairedPlayerId1 = -1;
  unpairedPlayerId2 = -1;
  ui.btnPair->setEnabled(false);
  ui.btnSplit->setEnabled(false);
  
  // if no model is loaded or no category is selected, simply returns
  bool isActive = !(ui.catTableView->isEmptyModel());
  if (isActive)
  {
    QModelIndexList indexes = ui.catTableView->selectionModel()->selection().indexes();
    isActive = (indexes.count() > 0);
  }
  if (!isActive)
  {
    return;
  }
  
  // get the pair data from the selected category
  Category selCat = ui.catTableView->getSelectedCategory();
  QList<PlayerPair> pairList = selCat.getPlayerPairs();
  
  for (int i=0; i < pairList.count(); i++)
  {
    PlayerPair pp = pairList.at(i);
    QListWidgetItem* item = new QListWidgetItem(pp.getDisplayName());
    
    if (pp.hasPlayer2())
    {
      item->setData(Qt::UserRole, pp.getPairId());
      ui.lwPaired->addItem(item);
    } else {
      item->setData(Qt::UserRole, pp.getPlayer1().getId());
      ui.lwUnpaired->addItem(item);
    }
  }
}

//----------------------------------------------------------------------------

void CatTabWidget::onUnpairedPlayersSelectionChanged()
{
  QList<QListWidgetItem *> selPlayers = ui.lwUnpaired->selectedItems();
  
  if (selPlayers.count() == 0)
  {
    unpairedPlayerId1 = -1;
    unpairedPlayerId2 = -1;
  }
  
  if (selPlayers.count() == 1)
  {
    unpairedPlayerId1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
    unpairedPlayerId2 = -1;
  }
  
  if (selPlayers.count() == 2)
  {
    int id1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
    int id2 = selPlayers.at(1)->data(Qt::UserRole).toInt();
    
    unpairedPlayerId2 = (id1 == unpairedPlayerId1) ? id2 : id1;
  }
  
  if (selPlayers.count() == 3)
  {
    int id1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
    int id2 = selPlayers.at(1)->data(Qt::UserRole).toInt();
    int id3 = selPlayers.at(2)->data(Qt::UserRole).toInt();
    
    // remove the oldest selection, which is the entry in in
    // unpairedPlayerId1
    int idToBeDeselected = unpairedPlayerId1;
    
    // shift the stored selection by one
    unpairedPlayerId1 = unpairedPlayerId2;
    
    // find out which selected item is the newest one
    if ((id1 != idToBeDeselected) && (id1 != unpairedPlayerId1))
    {
      unpairedPlayerId2 = id1;
    }
    else if ((id2 != idToBeDeselected) && (id2 != unpairedPlayerId1))
    {
      unpairedPlayerId2 = id2;
    }
    else {
      unpairedPlayerId2 = id3;
    }
    
    // find which item is to be deselected
    if (idToBeDeselected == id1)
    {
      selPlayers.at(0)->setSelected(false);
    }
    else if (idToBeDeselected == id2)
    {
      selPlayers.at(1)->setSelected(false);
    }
    else
    {
      selPlayers.at(2)->setSelected(false);
    }
  }
  
  // fall-back: deselect all
  if (selPlayers.count() > 3)
  {
    for (int i=0; i < selPlayers.count(); i++ )
    {
      selPlayers.at(i)->setSelected(false);
    }
    unpairedPlayerId1 = -1;
    unpairedPlayerId2 = -1;
  }
  
  // update the "pair" button, if necessary
  bool canPair = false;
  if ((unpairedPlayerId1 > 0) && (unpairedPlayerId2 > 0))
  {
    Player p1 = Tournament::getPlayerMngr()->getPlayer(unpairedPlayerId1);
    Player p2 = Tournament::getPlayerMngr()->getPlayer(unpairedPlayerId2);
    
    Category c = ui.catTableView->getSelectedCategory();
    canPair = (c.canPairPlayers(p1, p2) == OK);
  }
  ui.btnPair->setEnabled(canPair);
}

//----------------------------------------------------------------------------

void CatTabWidget::onBtnPairClicked()
{
  QList<QListWidgetItem *> selPlayers = ui.lwUnpaired->selectedItems();
  
  //
  // I'll repeat a few of the checks here, in case the "selection-changed" handler
  // has not been called and the current selection is invalid for pairing
  //
  if (selPlayers.count() != 2)
  {
    QMessageBox::warning(this, tr("Need exactly two selected players for pairing!"), tr("Pairing impossible"));
    updatePairs();
    return;
  }
  
  int id1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
  int id2 = selPlayers.at(1)->data(Qt::UserRole).toInt();
  Category c = ui.catTableView->getSelectedCategory();
  Player p1 = Tournament::getPlayerMngr()->getPlayer(id1);
  Player p2 = Tournament::getPlayerMngr()->getPlayer(id2);
  if (c.canPairPlayers(p1, p2) != OK)
  {
    QMessageBox::warning(this, tr("These two players can't be paired for this category!"), tr("Pairing impossible"));
    updatePairs();
    return;
  }
  
  ERR e = Tournament::getCatMngr()->pairPlayers(c, p1, p2);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("Something went wrong during pairing. This shouldn't happen. For the records: Error code = ") + e, tr("Pairing impossible"));
  }
  
  updatePairs();
}

//----------------------------------------------------------------------------
    
void CatTabWidget::onPairedPlayersSelectionChanged()
{
  QList<QListWidgetItem *> selPairs = ui.lwPaired->selectedItems();
  
  ui.btnSplit->setEnabled(selPairs.count() != 0);
}

//----------------------------------------------------------------------------

void CatTabWidget::onBtnSplitClicked()
{
  QList<QListWidgetItem *> selPairs = ui.lwPaired->selectedItems();
  Category c = ui.catTableView->getSelectedCategory();
  CatMngr* cmngr = Tournament::getCatMngr();
  
  for (int i=0; i < selPairs.count(); i++)
  {
    int pairId = selPairs.at(i)->data(Qt::UserRole).toInt();
    ERR e = cmngr->splitPlayers(c, pairId);
    if (e != OK)
    {
      QMessageBox::warning(this, tr("Something went wrong during splitting. This shouldn't happen. For the records: Error code = ") + e, tr("Splitting impossible"));
    }
  }
  
  updatePairs();
}

//----------------------------------------------------------------------------

void CatTabWidget::onMatchTypeButtonClicked(int btn)
{
  Category selCat = ui.catTableView->getSelectedCategory();
  
  MATCH_TYPE oldType = selCat.getMatchType();
  
  MATCH_TYPE newType = SINGLES;
  if (ui.rbDoubles->isChecked()) newType = DOUBLES;
  if (ui.rbMixed->isChecked()) newType = MIXED;
  
  // do we actually have a change?
  if (oldType == newType)
  {
    return;  // nope, no action required
  }
  
  // change the type
  ERR e = selCat.setMatchType(newType);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("Error"), tr("Could change match type. Error number = ") + e);
  }
  
  // in case the change wasn't successful, restore the old, correct type;
  // furthermore, the pairs could have changed. So basically we need to
  // reset the whole widget
  updatePairs();
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onSexClicked(int btn)
{
  // the new sex must be either M or F
  SEX newSex = M;
  if (ui.rbLadies->isChecked()) newSex = F;
  
  // in any case, we can de-select the "don't care" box
  ui.cbDontCare->setChecked(false);
  
  // actually change the sex
  Category selCat = ui.catTableView->getSelectedCategory();
  ERR e = selCat.setSex(newSex);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("Error"), tr("Could change sex. Error number = ") + e);
  }
  
  // in case the change wasn't successful, restore the old, correct type;
  // furthermore, the pairs could have changed. So basically we need to
  // reset the whole widget
  updatePairs();
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onDontCareClicked()
{
  bool newState = ui.cbDontCare->isChecked();
  Category selCat = ui.catTableView->getSelectedCategory();
  
  // unless we are in a mixed category, "don't care" can only be deactivated
  // by selecting a specific sex
  if (!newState && (selCat.getMatchType() != MIXED))
  {
    QString msg = tr("Please deactivate 'Don't care' by selecting a specific sex!");
    QMessageBox::information(this, tr("Change category sex"), msg);
    
    // re-check the checkbox
    ui.cbDontCare->setChecked(true);
    
    return;
  }
  
  SEX newSex = DONT_CARE;
  
  // if we are in a mixed category, allow simple de-activation of "Don't care"
  if (!newState && (selCat.getMatchType() == MIXED))
  {
    // set a dummy default value that is not "Don't care"
    newSex = M;
  }
  
  // un-check "Men" and "Ladies" if "Don't care" was activated
  if (newState)
  {
    ui.rbgSex->setExclusive(false);
    ui.rbMen->setChecked(false);
    ui.rbLadies->setChecked(false);
    ui.rbgSex->setExclusive(true);
  }
  
  // set the new value
  ERR e = selCat.setSex(newSex);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("Error"), tr("Could change sex. Error number = ") + e);
  }
  
  // in case the change wasn't successful, restore the old, correct type;
  // furthermore, the pairs could have changed. So basically we need to
  // reset the whole widget
  updatePairs();
  updateControls();
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
