
#include <QMessageBox>

#include "DlgImportPlayer.h"
#include "ui_DlgImportPlayer.h"

DlgImportPlayer::DlgImportPlayer(QWidget *parent, ExternalPlayerDB* _extDb) :
  QDialog(parent),
  ui(new Ui::DlgImportPlayer),
  extDb(_extDb)
{
  ui->setupUi(this);

  ui->lwNames->clear();
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

void DlgImportPlayer::onNameListSelectionChanged()
{
  auto selItems = ui->lwNames->selectedItems();

  ui->btnImport->setEnabled(!(selItems.isEmpty()));
}

//----------------------------------------------------------------------------

void DlgImportPlayer::onSearchStringChanged()
{
  // clear the old search results
  ui->lwNames->clear();

  // get and trim the search string
  QString searchString = ui->leSearchString->text();
  searchString = searchString.trimmed();

  // we need a minimum length of three characters
  // before we start any search
  if (searchString.length() < 3)
  {
    ui->btnImport->setEnabled(false);

    return;
  }

  // query the database for possible matches
  auto matchingEntries = extDb->searchForMatchingPlayers(searchString);

  // update the list widget with the search results
  for (const ExternalPlayerDatabaseEntry& entry : matchingEntries)
  {
    auto newItem = new QListWidgetItem(entry.getDisplayName(), ui->lwNames);
    newItem->setData(Qt::UserRole, entry.getId());
  }
}

//----------------------------------------------------------------------------

