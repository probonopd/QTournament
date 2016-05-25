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

#include <QKeyEvent>

#include "DlgSeedingEditor.h"
#include "ui_DlgSeedingEditor.h"

DlgSeedingEditor::DlgSeedingEditor(TournamentDB* _db, QWidget *parent) :
  QDialog(parent), db(_db), positionInput(0),
  ui(new Ui::DlgSeedingEditor)
{
  ui->setupUi(this);
  ui->lwSeeding->setDatabase(db);

  // set the window title
  setWindowTitle(tr("Define seeding"));

  updateButtons();

  ui->sbRangeMin->setValue(2);
  ui->sbRangeMin->setMinimum(2);

  // intercept keypress-events for some widgets for implementing
  // an "player warp" by pressing a number key
  ui->lwSeeding->installEventFilter(this);
  ui->btnUp->installEventFilter(this);
  ui->btnDown->installEventFilter(this);

  // combine subsequent keypresses into one number; for this we
  // need a timer
  keypressTimer = new QTimer();
  keypressTimer->setInterval(SUBSEQUENT_KEYPRESS_TIMEOUT__MS);
  keypressTimer->setSingleShot(true);
  connect(keypressTimer, SIGNAL(timeout()), this, SLOT(onKeypressTimerElapsed()));
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

PlayerPairList DlgSeedingEditor::getSeeding()
{
  return ui->lwSeeding->getSeedList();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onKeypressTimerElapsed()
{
  ui->lwSeeding->warpSelectedPlayerTo(positionInput-1);
  positionInput = 0;
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

//----------------------------------------------------------------------------

bool DlgSeedingEditor::eventFilter(QObject* target, QEvent* event)
{
  // we only want to catch key presses here
  if (event->type() != QEvent::KeyPress)
  {
    return QDialog::eventFilter(target, event);
  }

  // we only want to catch keys 0...9
  QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
  int key = keyEvent->key();
  if ((key != Qt::Key_0) && (key != Qt::Key_1) && (key != Qt::Key_2) && (key != Qt::Key_3) &&
      (key != Qt::Key_4) && (key != Qt::Key_5) && (key != Qt::Key_6) && (key != Qt::Key_7) &&
      (key != Qt::Key_8) && (key != Qt::Key_9))
  {
    return QDialog::eventFilter(target, event);
  }

  // we only want to catch key events sent to the list widget or the up/down buttons
  // (read: keys that are pressed while these widgets own the focus)
  if ((target != ui->lwSeeding) && (target != ui->btnUp) && (target != ui->btnDown))
  {
    return QDialog::eventFilter(target, event);
  }

  // processing keys here makes no sense if no row is selected in
  // the list widget
  if (ui->lwSeeding->currentRow() < 0)
  {
    return QDialog::eventFilter(target, event);
  }

  // convert the event ID into a real number
  int keyValue = 0;
  switch (key)
  {
  case Qt::Key_1:
    keyValue = 1;
    break;
  case Qt::Key_2:
    keyValue = 2;
    break;
  case Qt::Key_3:
    keyValue = 3;
    break;
  case Qt::Key_4:
    keyValue = 4;
    break;
  case Qt::Key_5:
    keyValue = 5;
    break;
  case Qt::Key_6:
    keyValue = 6;
    break;
  case Qt::Key_7:
    keyValue = 7;
    break;
  case Qt::Key_8:
    keyValue = 8;
    break;
  case Qt::Key_9:
    keyValue = 9;
    break;
  }

  // if this is the first keypress after a pause of
  // SUBSEQUENT_KEYPRESS_TIMEOUT__MS we start gathering
  // a new number. Otherwise, we combine the previously
  // pressed digits with the new one
  if (keypressTimer->isActive())
  {
    positionInput = positionInput * 10 + keyValue;
  } else {
    positionInput = keyValue;
    keypressTimer->start();
  }

  return true;
}
