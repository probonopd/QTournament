#include <exception>

#include <QListWidgetItem>

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

  auto cm = Tournament::getCatMngr();
  auto pm = Tournament::getPlayerMngr();

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
        applicablePlayers.append(pl);
      }
    }
  }
  else if (ctxt == DLG_CONTEXT::REMOVE_FROM_CATEGORY)
  {
    for (const Player& pl : cat->getAllPlayersInCategory())
    {
      if (cat->canRemovePlayer(pl))
      {
        applicablePlayers.append(pl);
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

  PlayerMngr* pm = Tournament::getPlayerMngr();
  for (auto& item : ui->lwPlayers->selectedItems())
  {
    int playerId = item->data(Qt::UserRole).toInt();
    result.append(pm->getPlayer(playerId));
  }

  return result;
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

