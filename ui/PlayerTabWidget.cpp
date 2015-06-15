/*
 * File:   PlayerTabWidget.cpp
 * Author: volker
 *
 * Created on March 19, 2014, 7:38 PM
 */

#include <QMessageBox>

#include "PlayerTabWidget.h"
#include "MainFrame.h"

PlayerTabWidget::PlayerTabWidget()
:QWidget()
{
  ui.setupUi(this);

  // prepare a label with the total number of players
  onPlayerCountChanged();

  // subscribe to the tournament-opened- and -closed-signal
  auto mf = MainFrame::getMainFramePointer();
  connect(mf, SIGNAL(tournamentOpened(Tournament*)), this, SLOT(onTournamentOpened()), Qt::DirectConnection);
  connect(mf, SIGNAL(tournamentClosed()), this, SLOT(onTournamentClosed()), Qt::DirectConnection);
}

//----------------------------------------------------------------------------

PlayerTabWidget::~PlayerTabWidget()
{
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onCreatePlayerClicked()
{
  ui.playerView->onAddPlayerTriggered();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onPlayerDoubleClicked(const QModelIndex& index)
{
  ui.playerView->onEditPlayerTriggered();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onPlayerCountChanged()
{
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (pm == nullptr)
  {
    ui.laPlayerCount->setText(QString()); // no tournament started / opem
  } else {
    QString txt = QString::number(pm->getTotalPlayerCount());
    txt += tr(" players in tournament");
    ui.laPlayerCount->setText(txt);
  }
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onTournamentOpened()
{
  // connect to all events that modify the number of players
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (pm == nullptr) return;
  connect(pm, SIGNAL(endCreatePlayer(int)), this, SLOT(onPlayerCountChanged()));
  connect(pm, SIGNAL(endDeletePlayer()), this, SLOT(onPlayerCountChanged()));
  onPlayerCountChanged();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onTournamentClosed()
{
  // disconnect all signals
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (pm == nullptr) return;
  disconnect(pm, SIGNAL(endCreatePlayer(int)), this, SLOT(onPlayerCountChanged()));
  disconnect(pm, SIGNAL(endDeletePlayer()), this, SLOT(onPlayerCountChanged()));
  onPlayerCountChanged();
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

