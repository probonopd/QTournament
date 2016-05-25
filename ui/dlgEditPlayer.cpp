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

#include <QMessageBox>
#include <QRadioButton>

#include "dlgEditPlayer.h"
#include "Category.h"
#include "PlayerMngr.h"
#include "CatMngr.h"
#include "TeamMngr.h"

DlgEditPlayer::DlgEditPlayer(TournamentDB* _db, QWidget* parent, Player* _selectedPlayer)
  :QDialog(parent), db(_db), sexPreset(M), presetCatId(-1)
{
  ui.setupUi(this);
  selectedPlayer = (_selectedPlayer != NULL) ? _selectedPlayer : NULL;
  _hasNameChange = true;
  
  initTeamList();
  
  if (selectedPlayer)
  {
    selectedPlayer = _selectedPlayer;
    initFromPlayerData();
  }
}

//----------------------------------------------------------------------------

DlgEditPlayer::DlgEditPlayer(TournamentDB* _db, QWidget *parent, SEX _sexPreset, const Category &_catPreset)
:QDialog(parent), db(_db), _hasNameChange(true), selectedPlayer(nullptr),
  sexPreset(_sexPreset), presetCatId(_catPreset.getId())
{
  ui.setupUi(this);

  initTeamList();

  // apply the presets
  ui.rbFemale->setChecked(sexPreset == F);
  ui.rbMale->setChecked(sexPreset == M);
  onSexSelectionChanged(presetCatId);
}

//----------------------------------------------------------------------------

DlgEditPlayer::DlgEditPlayer(TournamentDB* _db, QWidget* parent, const ExternalPlayerDatabaseEntry& nameAndSexPreset, int _presetCatId)
  :QDialog(parent), db(_db), _hasNameChange(true), selectedPlayer(nullptr), sexPreset(M), presetCatId(_presetCatId)
{
  ui.setupUi(this);

  initTeamList();

  // apply the preset from the external database
  ui.leFirstName->setText(nameAndSexPreset.getFirstname());
  ui.leLastName->setText(nameAndSexPreset.getLastname());
  if (nameAndSexPreset.getSex() != DONT_CARE)
  {
    ui.rbFemale->setChecked(nameAndSexPreset.getSex() == F);
    ui.rbMale->setChecked(nameAndSexPreset.getSex() == M);
    onSexSelectionChanged(presetCatId);
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
  _hasNameChange = true;
  if (selectedPlayer)
  {
    if ((selectedPlayer->getFirstName() == newFirst) && (selectedPlayer->getLastName() == newLast))
    {
      _hasNameChange = false; // no name change
    }
  }
  if (_hasNameChange)   // this "if" triggers only for newly inserted players or for existing players with a name change
  {
    PlayerMngr pm{db};
    if (pm.hasPlayer(newFirst, newLast))
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
  
  // initialize the list of applicable categories
  CatMngr cm{db};
  QHash<Category,CAT_ADD_STATE> catStatus = cm.getAllCategoryAddStates(*selectedPlayer);
  updateCatList(catStatus);
}

//----------------------------------------------------------------------------

void DlgEditPlayer::initTeamList()
{
  TeamMngr tm{db};
  vector<Team> allTeams = tm.getAllTeams();

  // Sort the list aphabetically
  std::sort(allTeams.begin(), allTeams.end(), [](Team& t1, Team& t2) {
    return (t1.getName() < t2.getName());
  });
  
  ui.cbTeams->clear();
  
  // add a "please select"-entry if we have no
  // pre-selected player. Read: if we have a new player
  if (selectedPlayer == NULL)
  {
    ui.cbTeams->addItem(tr("<Please select>"), -1);
  }
  
  for (int i=0; i < allTeams.size(); i++)
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
  TeamMngr tm{db};
  return tm.getTeamById(id);
}

//----------------------------------------------------------------------------

void DlgEditPlayer::onSexSelectionChanged(int preselectCatId)
{
  // we assume that the change of a player
  // can't be changed after creation.
  //
  // Read: we'll only reach this slot if we're dealing
  // with a new player. So it's the sex, not the player
  // itself that determines the can-add-state
  //
  CatMngr cm{db};
  QHash<Category,CAT_ADD_STATE> catStatus = cm.getAllCategoryAddStates(getSex());
  
  updateCatList(catStatus, preselectCatId);
}

//----------------------------------------------------------------------------

void DlgEditPlayer::updateCatList(QHash<Category,CAT_ADD_STATE> catStatus, int preselectCatId)
{
  ui.catList->clear();
  QHash<Category,CAT_ADD_STATE>::const_iterator it;
  
  for (it = catStatus.begin(); it != catStatus.end(); ++it)
  {
    CAT_ADD_STATE stat = (*it);
    QListWidgetItem* item = new QListWidgetItem(it.key().getName());
    int catId = it.key().getId();
    item->setData(Qt::UserRole, catId);
    
    if (stat == CAN_JOIN)
    {
      item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      //item->setCheckState(Qt::Unchecked);
      item->setCheckState((catId == preselectCatId) ? Qt::Checked : Qt::Unchecked);
    }
    if (stat == WRONG_SEX)
    {
      delete item;
      continue;
    }
    if (stat == ALREADY_MEMBER)
    {
      item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      item->setCheckState(Qt::Checked);
      
      // do we edit an existing player and could this player possibly be
      // removed from the category?
      // If not, deactivate the item
      if ((selectedPlayer != NULL) && (!(it.key().canRemovePlayer(*selectedPlayer)))) {
	item->setFlags(item->flags() & !Qt::ItemIsEditable & !Qt::ItemIsSelectable);
	item->setForeground(Qt::gray);
      }
    }
    if (stat == CAT_CLOSED)
    {
      item->setFlags(item->flags() & !Qt::ItemIsEditable & !Qt::ItemIsSelectable);
      item->setForeground(Qt::gray);
    }
    
    ui.catList->addItem(item);
  }
}

//----------------------------------------------------------------------------

QHash<Category,bool> DlgEditPlayer::getCategoryCheckState()
{
  QHash<Category,bool> result;
  CatMngr cmngr{db};
  
  for (int i=0; i < ui.catList->count(); i++)
  {
    QListWidgetItem* item = ui.catList->item(i);
    int catId = item->data(Qt::UserRole).toInt();
    Category cat = cmngr.getCategoryById(catId);
    result[cat] = (item->checkState() == Qt::Checked);
  }
  
  return result;
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

