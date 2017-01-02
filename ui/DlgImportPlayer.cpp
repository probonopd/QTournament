/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include "DlgImportPlayer.h"
#include "ui_DlgImportPlayer.h"

DlgImportPlayer::DlgImportPlayer(QWidget *parent, ExternalPlayerDB* _extDb) :
  QDialog(parent),
  ui(new Ui::DlgImportPlayer),
  extDb(_extDb), isViewFiltered(false)
{
  ui->setupUi(this);

  // Fill the list with all names in the database
  ui->lwNames->clear();
  if (extDb != nullptr)
  {
    allDatabaseEntries = extDb->getAllPlayers();
    addPlayerEntryToListWidget(allDatabaseEntries);
  }

  // initialize all other widgets
  ui->leSearchString->clear();
  ui->leSearchString->setFocus();
  ui->btnImport->setEnabled(false);
}

//----------------------------------------------------------------------------

DlgImportPlayer::~DlgImportPlayer()
{
  delete ui;
}

//----------------------------------------------------------------------------

int DlgImportPlayer::getSelectedExternalPlayerId()
{
  auto selItems = ui->lwNames->selectedItems();
  if (selItems.length() != 1) return -1;

  auto selItem = selItems.at(0);

  return selItem->data(Qt::UserRole).toInt();
}

//----------------------------------------------------------------------------

int DlgImportPlayer::exec()
{
  // throw an error if we have no valid database handle
  if (extDb == nullptr)
  {
    QString msg = tr("No valid database for player import provided.");
    QMessageBox::warning(parentWidget(), tr("Import player"), msg);
    return QDialog::Rejected;
  }

  // proceed with normal execution
  return QDialog::exec();
}

//----------------------------------------------------------------------------

void DlgImportPlayer::addPlayerEntryToListWidget(const ExternalPlayerDatabaseEntryList& entryList)
{
  for (const ExternalPlayerDatabaseEntry& entry : entryList)
  {
    auto newItem = new QListWidgetItem(entry.getDisplayName(), ui->lwNames);
    newItem->setData(Qt::UserRole, entry.getId());
  }
}

//----------------------------------------------------------------------------

void DlgImportPlayer::onNameListSelectionChanged()
{
  auto selItems = ui->lwNames->selectedItems();

  ui->btnImport->setEnabled(!(selItems.isEmpty()));
}

//----------------------------------------------------------------------------

void DlgImportPlayer::onSearchStringChanged()
{
  // get and trim the search string
  QString searchString = ui->leSearchString->text();
  searchString = searchString.trimmed();

  // we need a minimum length of three characters
  // before we start any search
  if (searchString.length() < 3)
  {
    ui->btnImport->setEnabled(false);

    // the the name list contained a previous search
    // result, we invalidate the previous search and
    // show all names
    if (isViewFiltered)
    {
      ui->lwNames->clear();
      addPlayerEntryToListWidget(allDatabaseEntries);
      isViewFiltered = false;
    }

    return;
  }

  // query the database for possible matches
  auto matchingEntries = extDb->searchForMatchingPlayers(searchString);

  // update the list widget with the search results
  ui->lwNames->clear();
  addPlayerEntryToListWidget(matchingEntries);
  isViewFiltered = true;
}

//----------------------------------------------------------------------------

