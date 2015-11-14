/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include "CatTableView.h"
#include "MainFrame.h"
#include "Category.h"
#include <stdexcept>
#include <QInputDialog>
#include <QString>
#include <QtWidgets/qmessagebox.h>
#include "ui/delegates/CatItemDelegate.h"

#include "CatMngr.h"

CategoryTableView::CategoryTableView(QWidget* parent)
:QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  
  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &CategoryTableView::onTournamentOpened);

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // define a delegate for drawing the category items
  auto itemDelegate = new CatItemDelegate(this);
  setItemDelegate(itemDelegate);

  // prep the context menu
  initContextMenu();
}

//----------------------------------------------------------------------------
    
CategoryTableView::~CategoryTableView()
{
  delete emptyModel;
}

//----------------------------------------------------------------------------
    
void CategoryTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  setModel(Tournament::getCategoryTableModel());
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &CategoryTableView::onTournamentClosed);
  
  // tell the rest of the category widget, that the model has changed
  // and that it should update
  emit catModelChanged();
}

//----------------------------------------------------------------------------
    
void CategoryTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &CategoryTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = 0;
  setModel(emptyModel);
  setEnabled(false);
  
  
  // tell the rest of the category widget, that the model has changed
  // and that it should update
  emit catModelChanged();
}

//----------------------------------------------------------------------------

bool CategoryTableView::isEmptyModel()
{
  if (model())
  {
    return (model()->rowCount() == 0);
  }
  return true;
}

//----------------------------------------------------------------------------

bool CategoryTableView::hasCategorySelected()
{
  if (isEmptyModel())
  {
    //throw std::invalid_argument("No model/tournament loaded");
    return false;
  }
  
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    //throw std::invalid_argument("No category selected");
    return false;
  }

  return true;  
}

//----------------------------------------------------------------------------

Category CategoryTableView::getSelectedCategory()
{
  if (!(hasCategorySelected()))
  {
    throw std::invalid_argument("No category selected");
  }
  
  QModelIndexList indexes = selectionModel()->selection().indexes();
  int selectedSeqNum = indexes.at(0).row();
  
  return Tournament::getCatMngr()->getCategoryBySeqNum(selectedSeqNum);
}

//----------------------------------------------------------------------------

void CategoryTableView::onCategoryDoubleClicked(const QModelIndex& index)
{
  if (!(index.isValid()))
  {
    return;
  }
  
  Category selectedCat = Tournament::getCatMngr()->getCategoryBySeqNum(index.row());
  
  QString oldName = selectedCat.getName();
  
  bool isOk = false;
  while (!isOk)
  {
    QString newName = QInputDialog::getText(this, tr("Rename category"), tr("Enter new category name:"),
	    QLineEdit::Normal, oldName, &isOk);

    if (!isOk)
    {
      return;  // the user hit cancel
    }

    if (newName.isEmpty())
    {
      QMessageBox::critical(this, tr("Rename category"), tr("The new name may not be empty!"));
      isOk = false;
      continue;
    }

    if (oldName == newName)
    {
      return;
    }

    // okay, we have a valid name. try to rename the category
    newName = newName.trimmed();
    ERR e = Tournament::getCatMngr()->renameCategory(selectedCat, newName);

    if (e == INVALID_NAME)
    {
      QMessageBox::critical(this, tr("Rename category"), tr("The name you entered is invalid (e.g., too long)"));
      isOk = false;
      continue;
    }

    if (e == NAME_EXISTS)
    {
      QMessageBox::critical(this, tr("Rename category"), tr("A category of this name already exists"));
      isOk = false;
      continue;
    }
  }
}

//----------------------------------------------------------------------------

void CategoryTableView::onAddCategory()
{

}

//----------------------------------------------------------------------------

void CategoryTableView::onRemoveCategory()
{
  if (!(hasCategorySelected())) return;

  Category cat = getSelectedCategory();
  CatMngr* cm = Tournament::getCatMngr();

  // can the category be deleted at all?
  ERR err = cm->canDeleteCategory(cat);

  // category is already beyond config state
  if (err == CATEGORY_NOT_CONFIGURALE_ANYMORE)
  {
    QString msg = tr("The has already been started.\n");
    msg += tr("Running categories cannot be deleted anymore.");
    QMessageBox::critical(this, tr("Delete category"), msg);
    return;
  }

  // not all players removable
  if (err == PLAYER_NOT_REMOVABLE_FROM_CATEGORY)
  {
    QString msg = tr("Cannot remove all players from the category.\n");
    msg += tr("The category cannot be deleted.");
    QMessageBox::critical(this, tr("Delete category"), msg);
    return;
  }

  // okay, the category can be deleted. Get a confirmation
  QString msg = tr("Note: this will remove all player assignments from this category\n");
  msg += tr("and the category from the whole tournament.\n\n");
  msg += tr("This step is irrevocable!\n\n");
  msg += tr("Proceed?");
  int result = QMessageBox::question(this, tr("Delete category"), msg);
  if (result != QMessageBox::Yes) return;

  // we can actually delete the category. Let's go!
  err = cm->deleteCategory(cat);
  if (err != OK) {
    QString msg = tr("Something went wrong when deleting the category. This shouldn't happen.\n\n");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int> (err));
    QMessageBox::warning(this, tr("WTF??"), msg);
  }
}

//----------------------------------------------------------------------------

void CategoryTableView::onRunCategory()
{

}

//----------------------------------------------------------------------------

void CategoryTableView::onContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = viewport()->mapToGlobal(pos);

  // resolve the click coordinates to the table row
  int clickedRow = rowAt(pos.y());
  int clickedCol = columnAt(pos.x());
  bool isCellClicked = ((clickedRow >= 0) && (clickedCol >= 0));

  // check if we have a valid category selection and
  // determine the state of the selected category
  OBJ_STATE catState = STAT_CO_DISABLED;   // an arbitrary, dummy default... not related to categories
  if (hasCategorySelected())
  {
    Category cat = getSelectedCategory();
    catState = cat.getState();
  }

  // enable / disable selection-specific actions
  actAddCategory->setEnabled(true);   // always possible
  actRunCategory->setEnabled(isCellClicked && (catState == STAT_CAT_CONFIG));
  actRemoveCategory->setEnabled(isCellClicked && (catState == STAT_CAT_CONFIG));

  // show the context menu
  QAction* selectedItem = contextMenu->exec(globalPos);
}

//----------------------------------------------------------------------------

void CategoryTableView::initContextMenu()
{
  // prepare all action
  actAddCategory = new QAction(tr("Add category"), this);
  actRunCategory = new QAction(tr("Run category..."), this);
  actRemoveCategory = new QAction(tr("Remove category..."), this);

  // create the context menu and connect it to the actions
  contextMenu = unique_ptr<QMenu>(new QMenu());
  contextMenu->addAction(actAddCategory);
  contextMenu->addSeparator();
  contextMenu->addAction(actRunCategory);
  contextMenu->addSeparator();
  contextMenu->addAction(actRemoveCategory);

  // connect signals and slots
  connect(actAddCategory, SIGNAL(triggered(bool)), this, SLOT(onAddCategory()));
  connect(actRunCategory, SIGNAL(triggered(bool)), this, SLOT(onRunCategory()));
  connect(actRemoveCategory, SIGNAL(triggered(bool)), this, SLOT(onRemoveCategory()));
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
    

