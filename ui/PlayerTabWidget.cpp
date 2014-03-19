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
  ERR e = Tournament::getPlayerTableModel()->createNewPlayer(
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

