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

CategoryTableView::CategoryTableView(QWidget* parent)
:QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  
  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &CategoryTableView::onTournamentOpened);

  // define a delegate for drawing the category items
  auto itemDelegate = new CatItemDelegate(this);
  setItemDelegate(itemDelegate);
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
    

