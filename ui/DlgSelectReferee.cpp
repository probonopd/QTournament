
#include <QMessageBox>

#include "DlgSelectReferee.h"
#include "ui_DlgSelectReferee.h"

#include "KeyValueTab.h"

#include "PlayerPair.h"
#include "Team.h"
#include "TeamMngr.h"
#include "PlayerMngr.h"

DlgSelectReferee::DlgSelectReferee(TournamentDB* _db, const Match& _ma, bool _matchIsBeingCalled, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgSelectReferee), db(_db), ma(_ma), matchIsBeingCalled(_matchIsBeingCalled)
{
  ui->setupUi(this);

  // initialize the match information section
  PlayerPair pp1 = ma.getPlayerPair1();
  PlayerPair pp2 = ma.getPlayerPair2();
  ui->laPair1Player1Name->setText(pp1.getPlayer1().getDisplayName_FirstNameFirst());
  ui->laPair1Player1Team->setText(pp1.getPlayer1().getTeam().getName());
  ui->laPair2Player1Name->setText(pp2.getPlayer1().getDisplayName_FirstNameFirst());
  ui->laPair2Player1Team->setText(pp2.getPlayer1().getTeam().getName());
  if (pp1.hasPlayer2())
  {
    ui->laPair1Player2Name->setText(pp1.getPlayer2().getDisplayName_FirstNameFirst());
    ui->laPair1Player2Team->setText(pp1.getPlayer2().getTeam().getName());
    ui->laPair2Player2Name->setText(pp2.getPlayer2().getDisplayName_FirstNameFirst());
    ui->laPair2Player2Team->setText(pp2.getPlayer2().getTeam().getName());
  } else {
    ui->laPair1Player2Name->setHidden(true);
    ui->laPair1Player2Team->setHidden(true);
    ui->laPair2Player2Name->setHidden(true);
    ui->laPair2Player2Team->setHidden(true);
  }

  // initialize the drop box for the selection mode
  ui->cbFilterMode->addItem(tr("All players"), static_cast<int>(REFEREE_MODE::ALL_PLAYERS));
  ui->cbFilterMode->addItem(tr("Recent losers"), static_cast<int>(REFEREE_MODE::RECENT_LOSERS));
  ui->cbFilterMode->addItem(tr("Special team member"), static_cast<int>(REFEREE_MODE::SPECIAL_TEAM));
  REFEREE_MODE curRefMode = ma.getRefereeMode();
  switch (curRefMode)
  {
  case REFEREE_MODE::ALL_PLAYERS:
    ui->cbFilterMode->setCurrentIndex(0);
    break;

  case REFEREE_MODE::RECENT_LOSERS:
    ui->cbFilterMode->setCurrentIndex(1);
    break;

  case REFEREE_MODE::SPECIAL_TEAM:
    ui->cbFilterMode->setCurrentIndex(2);
    break;

  case REFEREE_MODE::HANDWRITTEN:
    reject();  // nothing to do for us
    break;

  case REFEREE_MODE::NONE:
    reject();  // nothing to do for us
    break;
  }

  // get the tournament-wide default value for the referee-team, if any
  auto cfg = SqliteOverlay::KeyValueTab::getTab(db, TAB_CFG);
  int defaultRefereeTeamId = cfg->getInt(CFG_KEY_REFEREE_TEAM_ID);
  initTeamList(defaultRefereeTeamId);


  // if the dialog is executed when the match is NOT BEING CALLED (--> we have a "pre-assignment)
  // then continuing without an umpire is no option
  if (!matchIsBeingCalled)
  {
    ui->btnNone->setHidden(true);
  }

  // fill the player table
  rebuildPlayerList();

  updateControls();
}

//----------------------------------------------------------------------------

DlgSelectReferee::~DlgSelectReferee()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onFilterModeChanged()
{
  rebuildPlayerList();
  updateControls();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onTeamSelectionChanged()
{
  rebuildPlayerList();
  updateControls();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onPlayerSelectionChanged()
{
  updateControls();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::updateControls()
{
  // disable the team selection drop box if the filter
  // type is not "special team"
  int curFilterModeId = ui->cbFilterMode->currentData().toInt();
  REFEREE_MODE curFilterMode = static_cast<REFEREE_MODE>(curFilterModeId);
  ui->cbTeamSelection->setHidden(curFilterMode != REFEREE_MODE::SPECIAL_TEAM);
  ui->laTeamSelection->setHidden(curFilterMode != REFEREE_MODE::SPECIAL_TEAM);

  // only enable the "select" button if a player is selected
  ui->btnSelect->setEnabled(ui->tabPlayers->hasPlayerSelected());
}

//----------------------------------------------------------------------------

void DlgSelectReferee::initTeamList(int defaultTeamId)
{
  TeamMngr tm{db};
  vector<Team> allTeams = tm.getAllTeams();

  // Sort the list aphabetically
  std::sort(allTeams.begin(), allTeams.end(), [](Team& t1, Team& t2) {
    return (t1.getName() < t2.getName());
  });

  ui->cbTeamSelection->clear();

  // add a "please select"-entry if we have no
  // pre-selected player. Read: if we have a new player
  if (defaultTeamId < 1)
  {
    ui->cbTeamSelection->addItem(tr("<Please select>"), -1);
  }

  for (int i=0; i < allTeams.size(); i++)
  {
    ui->cbTeamSelection->addItem(allTeams.at(i).getName(), allTeams.at(i).getId());
    if (allTeams.at(i).getId() == defaultTeamId) ui->cbTeamSelection->setCurrentIndex(i);
  }
}

//----------------------------------------------------------------------------

void DlgSelectReferee::rebuildPlayerList()
{
  // determine the current filter and team selection
  int curFilterModeId = ui->cbFilterMode->currentData().toInt();
  REFEREE_MODE curFilterMode = static_cast<REFEREE_MODE>(curFilterModeId);
  int curTeamId = ui->cbTeamSelection->currentData().toInt();

  // if the current filter is "team" but there is no team selected,
  // stop here
  if ((curFilterMode == REFEREE_MODE::SPECIAL_TEAM) && (curTeamId < 1))
  {
    ui->tabPlayers->rebuildPlayerList(PlayerList());
    return;
  }

  // determine the list of players for display
  PlayerMngr pm{db};
  TeamMngr tm{db};
  PlayerList pList;
  if (curFilterMode == REFEREE_MODE::ALL_PLAYERS)
  {
    pList = pm.getAllPlayers();
  }
  if (curFilterMode == REFEREE_MODE::SPECIAL_TEAM)
  {
    Team selTeam = tm.getTeamById(curTeamId);
    pList = tm.getPlayersForTeam(selTeam);
  }
  if (curFilterMode == REFEREE_MODE::RECENT_LOSERS)
  {
    pList = getPlayerList_recentLosers();
  }

  // add the players to the table
  ui->tabPlayers->rebuildPlayerList(pList);
}

//----------------------------------------------------------------------------

PlayerList DlgSelectReferee::getPlayerList_recentLosers()
{
  return PlayerList();
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

RefereeTableWidget::RefereeTableWidget(QWidget* parent)
  :QTableWidget(parent), db(nullptr)
{
  // prepare the table layout (columns, headers)
  setColumnCount(NUM_TAB_COLUMNS);
  QStringList horHeaders{tr("Player name"), tr("Team"), tr("Uses")};
  setHorizontalHeaderLabels(horHeaders);
  verticalHeader()->hide();
}

//----------------------------------------------------------------------------

void RefereeTableWidget::rebuildPlayerList(const PlayerList& pList)
{
  // erase everything from the table
  clearContents();
  setRowCount(0);

  // populate the table rows
  setRowCount(pList.size());
  int idxRow = 0;
  for (const Player& p : pList)
  {
    QTableWidgetItem* newItem = new QTableWidgetItem(p.getDisplayName());
    newItem->setData(Qt::UserRole, p.getId());
    setItem(idxRow, NAME_COL_ID, newItem);

    idxRow++;
  }

  // grab the current database handle from the first entry
  // in the player list
  if (pList.empty())
  {
    db = nullptr;
  } else {
    db = pList.at(0).getDatabaseHandle();
  }
}

//----------------------------------------------------------------------------

upPlayer RefereeTableWidget::getSelectedPlayer()
{
  // could we ever store a database handle? If not,
  // we can't determine the player
  if (db == nullptr) return nullptr;

  // is something selected?
  if (!(hasPlayerSelected())) return nullptr;

  // get the player item for the selected row
  QTableWidgetItem* playerItem = item(currentRow(), NAME_COL_ID);
  if (playerItem == nullptr) return nullptr;

  // the user data of the player item contains the player id
  int playerId = playerItem->data(Qt::UserRole).toInt();

  // return the associated player
  PlayerMngr pm{db};
  return pm.getPlayer_up(playerId);
}

bool RefereeTableWidget::hasPlayerSelected()
{
  return ((currentRow() > 0) && (currentItem() != nullptr));
}

//----------------------------------------------------------------------------

void RefereeTableWidget::resizeEvent(QResizeEvent* event)
{
  // autosize all column in a fixed ratio
  int widthIncrement = width() / (REL_WIDTH_NAME + REL_WIDTH_TEAM +  1 * REL_WIDTH_OTHER);
  setColumnWidth(NAME_COL_ID, widthIncrement * REL_WIDTH_NAME);
  setColumnWidth(TEAM_COL_ID, widthIncrement * REL_WIDTH_TEAM);
  setColumnWidth(REFEREE_COUNT_COL_ID, widthIncrement * REL_WIDTH_OTHER);
}
