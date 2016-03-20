/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <exception>

#include <QListWidgetItem>
#include <QMessageBox>

#include "DlgSelectPlayer.h"
#include "ui_DlgSelectPlayer.h"

DlgSelectPlayer::DlgSelectPlayer(QWidget *parent, DLG_CONTEXT _ctxt, Category* _cat) :
  QDialog(parent),
  ui(new Ui::DlgSelectPlayer), ctxt(_ctxt), cat(_cat)
{
  ui->setupUi(this);

  // we always need a valid cat ref, unless
  // our context is NONE
  if ((ctxt != DLG_CONTEXT::NONE) && (cat == nullptr))
  {
    throw std::invalid_argument("Receied empty category reference for player selection dialog!");
  }

  auto tnmt = Tournament::getActiveTournament();
  auto cm = tnmt->getCatMngr();
  auto pm = tnmt->getPlayerMngr();

  // define the set of players that should be available for selection
  PlayerList applicablePlayers;
  if (ctxt == DLG_CONTEXT::NONE)
  {
    applicablePlayers = pm->getAllPlayers();
  }
  else if (ctxt == DLG_CONTEXT::ADD_TO_CATEGORY)
  {
    for (const Player& pl : pm->getAllPlayers())
    {
      if (cat->hasPlayer(pl)) continue;

      if (cat->getAddState(pl.getSex()) == CAN_JOIN)
      {
        applicablePlayers.push_back(pl);
      }
    }
  }
  else if (ctxt == DLG_CONTEXT::REMOVE_FROM_CATEGORY)
  {
    for (const Player& pl : cat->getAllPlayersInCategory())
    {
      if (cat->canRemovePlayer(pl))
      {
        applicablePlayers.push_back(pl);
      }
    }
  }

  // sort them by name
  std::sort(applicablePlayers.begin(), applicablePlayers.end(), PlayerMngr::getPlayerSortFunction_byName());

  // create list widget items
  for (const Player& pl : applicablePlayers)
  {
    auto newItem = new QListWidgetItem(pl.getDisplayName());
    newItem->setData(Qt::UserRole, pl.getId());
    ui->lwPlayers->addItem(newItem);
  }

  // default status:
  // nothing selected, user can only select "Cancel"
  ui->lwPlayers->clearSelection();
  ui->btnCancel->setEnabled(true);
  ui->btnOkay->setEnabled(false);
}

//----------------------------------------------------------------------------

DlgSelectPlayer::~DlgSelectPlayer()
{
  delete ui;
}

//----------------------------------------------------------------------------

PlayerList DlgSelectPlayer::getSelectedPlayers() const
{
  PlayerList result;

  auto tnmt = Tournament::getActiveTournament();
  PlayerMngr* pm = tnmt->getPlayerMngr();
  for (auto& item : ui->lwPlayers->selectedItems())
  {
    int playerId = item->data(Qt::UserRole).toInt();
    result.push_back(pm->getPlayer(playerId));
  }

  return result;
}

//----------------------------------------------------------------------------

int DlgSelectPlayer::exec()
{
  // throw an error if we have an empty list
  if (ui->lwPlayers->count() == 0)
  {
    QString msg = tr("There no players available for this operation.");
    QMessageBox::warning(parentWidget(), tr("Select player(s)"), msg);
    return QDialog::Rejected;
  }

  // proceed with normal execution
  return QDialog::exec();
}

//----------------------------------------------------------------------------

void DlgSelectPlayer::onItemSelectionChanged()
{
  auto selItems = ui->lwPlayers->selectedItems();
  bool hasItemsSelected = (selItems.length() > 0);

  ui->btnOkay->setEnabled(hasItemsSelected);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

