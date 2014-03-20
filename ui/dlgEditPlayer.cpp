/*
 * File:   dlgEditPlayer.cpp
 * Author: volker
 *
 * Created on March 18, 2014, 8:00 PM
 */

#include <qt/QtWidgets/qmessagebox.h>
#include <qt4/QtGui/qwidget.h>

#include "Tournament.h"

#include "dlgEditPlayer.h"

DlgEditPlayer::DlgEditPlayer(Player* _selectedPlayer)
{
  ui.setupUi(this);
  selectedPlayer = (_selectedPlayer != NULL) ? _selectedPlayer : NULL;
  
  initTeamList();
  
  if (selectedPlayer)
  {
    selectedPlayer = _selectedPlayer;
    initFromPlayerData();
  }
}

//----------------------------------------------------------------------------

DlgEditPlayer::~DlgEditPlayer()
{
}

//----------------------------------------------------------------------------

/**
 * Validates the user input before finally closing the dialog
 */
void DlgEditPlayer::done(int result)
{
  if (result != QDialog::Accepted)
  {
    // the user hit cancel. Do what the original function
    // would have done
    QDialog::done(result);
    return;
  }
  
  //
  // So the user hit the "Okay" button. Let's see if all input
  // data is valid
  //
  QString newFirst = ui.leFirstName->text().trimmed();
  QString newLast = ui.leLastName->text().trimmed();
  bool isMale = ui.rbMale->isChecked();
  bool isFemale = ui.rbFemale->isChecked();
  
  // non-empty first name
  if (newFirst.isEmpty())
  {
    QMessageBox::critical(this, tr("Error in player data"), tr("The first name may not be empty!"));
    return;  // do not call parent done(), do not close the dialog)
  }
  
  // non-empty last name
  if (newLast.isEmpty())
  {
    QMessageBox::critical(this, tr("Error in player data"), tr("The last name may not be empty!"));
    return;  // do not call parent done(), do not close the dialog)
  }
  
  // check a valid sex selection
  if ((isMale && isFemale) || (!isMale && !isFemale))
  {
    QMessageBox::critical(this, tr("Error in player data"), tr("Please select a valid sex for the player!"));
    return;  // do not call parent done(), do not close the dialog)
  }
  
  // make sure the player name is unique
  bool hasNameChange = true;
  if (selectedPlayer)
  {
    if ((selectedPlayer->getFirstName() == newFirst) && (selectedPlayer->getLastName() == newLast))
    {
      hasNameChange = false; // no name change
    }
  }
  if (hasNameChange)   // this "if" triggers only for newly inserted players or for existing players with a name change
  {
    if (Tournament::getPlayerMngr()->hasPlayer(newFirst, newLast))
    {
      QMessageBox::critical(this, tr("Error in player data"), tr("A player of that name already exists!"));
      return;  // do not call parent done(), do not close the dialog)
    }
  }
  
  // valid team selection
  if (ui.cbTeams->currentData().toInt() < 0)
  {
    QMessageBox::critical(this, tr("Error in player data"), tr("Please select a valid team!"));
    return;  // do not call parent done(), do not close the dialog)
  }
  
  // everything is okay
  QDialog::done(result);
}

//----------------------------------------------------------------------------

void DlgEditPlayer::initFromPlayerData()
{
  if (selectedPlayer == NULL)
  {
    return;
  }
  
  // initialize the name fields
  ui.leFirstName->setText(selectedPlayer->getFirstName());
  ui.leLastName->setText(selectedPlayer->getLastName());

  // set the correct sex
  SEX sex = selectedPlayer->getSex();
  ui.rbMale->setChecked(sex == M);
  ui.rbFemale->setChecked(sex == F);
  
  // disable the radio buttons, because we don't
  // support changing a player's sex right now
  ui.rbMale->setEnabled(false);
  ui.rbFemale->setEnabled(false);
  
  // select the correct team
  // Note: we rely on a properly initialized team list here
  Team t = selectedPlayer->getTeam();
  ui.cbTeams->setCurrentText(t.getName());
}

//----------------------------------------------------------------------------

void DlgEditPlayer::initTeamList()
{
  QList<Team> allTeams = Tournament::getTeamMngr()->getAllTeams();
  
  ui.cbTeams->clear();
  
  // add a "please select"-entry if we have no
  // pre-selected player. Read: if we have a new player
  if (selectedPlayer == NULL)
  {
    ui.cbTeams->addItem(tr("<Please select>"), -1);
  }
  
  for (int i=0; i < allTeams.length(); i++)
  {
    ui.cbTeams->addItem(allTeams.at(i).getName(), allTeams.at(i).getId());
  }
}

//----------------------------------------------------------------------------

QString DlgEditPlayer::getFirstName()
{
  return ui.leFirstName->text().trimmed();
}

//----------------------------------------------------------------------------

QString DlgEditPlayer::getLastName()
{
  return ui.leLastName->text().trimmed();
}

//----------------------------------------------------------------------------

bool DlgEditPlayer::hasNameChange()
{
  return _hasNameChange;
}

//----------------------------------------------------------------------------

SEX DlgEditPlayer::getSex()
{
  return (ui.rbMale->isChecked()) ? M : F;
}

//----------------------------------------------------------------------------

Team DlgEditPlayer::getTeam()
{
  int id = ui.cbTeams->currentData().toInt();
  return Tournament::getTeamMngr()->getTeamById(id);
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

