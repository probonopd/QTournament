/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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
#include "CatMngr.h"
#include "PlayerMngr.h"

DlgSelectPlayer::DlgSelectPlayer(TournamentDB* _db, QWidget *parent, DLG_CONTEXT _ctxt, Category* _cat) :
  QDialog(parent), db(_db),
  ui(new Ui::DlgSelectPlayer), ctxt(_ctxt), cat(_cat)
{
  ui->setupUi(this);

  // we always need a valid cat ref, unless
  // our context is NONE
  if ((ctxt != DLG_CONTEXT::NONE) && (cat == nullptr))
  {
    throw std::invalid_argument("Received empty category reference for player selection dialog!");
  }

  PlayerMngr pm{db};

  // define the set of players that should be available for selection
  PlayerList applicablePlayers;
  if (ctxt == DLG_CONTEXT::NONE)
  {
    applicablePlayers = pm.getAllPlayers();
  }
  else if (ctxt == DLG_CONTEXT::ADD_TO_CATEGORY)
  {
    for (const Player& pl : pm.getAllPlayers())
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

  // react to item double clicks
  //connect(ui->lwPlayers, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));
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

  PlayerMngr pm{db};
  for (auto& item : ui->lwPlayers->selectedItems())
  {
    int playerId = item->data(Qt::UserRole).toInt();
    result.push_back(pm.getPlayer(playerId));
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

void DlgSelectPlayer::onItemDoubleClicked(QListWidgetItem* item)
{
  if (item == nullptr) return;

  accept();
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

