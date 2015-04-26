#include "DlgSeedingEditor.h"
#include "ui_DlgSeedingEditor.h"

DlgSeedingEditor::DlgSeedingEditor(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgSeedingEditor)
{
  ui->setupUi(this);

  updateButtons();

  ui->sbRangeMin->setValue(2);
  ui->sbRangeMin->setMinimum(2);
}

//----------------------------------------------------------------------------

DlgSeedingEditor::~DlgSeedingEditor()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::initSeedingList(const PlayerPairList& _seed)
{
  ui->lwSeeding->clearListAndFillFromSeed(_seed);

  if (_seed.size() > 2)
  {
    ui->sbRangeMin->setMaximum(_seed.size() - 1);
  }

  updateButtons();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onBtnUpClicked()
{
  ui->lwSeeding->moveSelectedPlayerUp();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onBtnDownClicked()
{
  ui->lwSeeding->moveSelectedPlayerDown();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onBtnShuffleClicked()
{
  int fromIndex = 0;
  if (ui->rbShuffleRange->isChecked())
  {
    fromIndex = ui->sbRangeMin->value() - 1;
  }
  ui->lwSeeding->shufflePlayers(fromIndex);
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onShuffleModeChange()
{
  bool useRange = ui->rbShuffleRange->isChecked();
  ui->sbRangeMin->setEnabled(useRange);
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onSelectionChanged()
{
  updateButtons();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::updateButtons()
{
  bool hasItems = (ui->lwSeeding->count() > 0);

  // okay is only possible if there are items
  ui->btnOkay->setEnabled(hasItems);

  // shuffling is only possible with at least two items
  ui->gbShuffle->setEnabled(ui->lwSeeding->count() > 1);

  // shuffling in a certain range only possible with at least three items
  ui->btnShuffle->setEnabled(ui->lwSeeding->count() > 2);

  // up / down availability depends on the selected item
  ui->btnUp->setEnabled(ui->lwSeeding->canMoveSelectedPlayerUp());
  ui->btnDown->setEnabled(ui->lwSeeding->canMoveSelectedPlayerDown());
}
