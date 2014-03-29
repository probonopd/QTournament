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
}

//----------------------------------------------------------------------------

void CatTabWidget::onTabSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
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
  if ((mt == MIXED) || (sex == DONT_CARE))
  {
    ui.rbMen->hide();
    ui.rbLadies->hide();
  } else {
    ui.rbMen->show();
    ui.rbLadies->show();
  }
  
  // update the applicable sex
  ui.rbMen->setChecked((sex == M) && (mt != MIXED));
  ui.rbLadies->setChecked((sex == F) && (mt != MIXED));
  ui.rbDontCare->setChecked(sex == DONT_CARE);
  
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
    
