/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#include <SqliteOverlay/KeyValueTab.h>

#include "DlgSeedingEditor.h"
#include "ui_DlgSeedingEditor.h"
#include "SvgBracket.h"
#include "HelperFunc.h"

using namespace QTournament;

DlgSeedingEditor::DlgSeedingEditor(const TournamentDB* _db, const QString& catName, std::optional<QTournament::SvgBracketMatchSys> brackSys, QWidget *parent) :
  QDialog(parent), ui(new Ui::DlgSeedingEditor), db{_db}, msys{brackSys}
{
  ui->setupUi(this);

  // set the window title
  setWindowTitle(tr("Define seeding"));

  // hide the report view, if necessary and resize
  // the widget to a proper size
  if (brackSys)
  {
    resize(WidthWithBracket, height());

    // initialize the meta tags
    auto cfg = SqliteOverlay::KeyValueTab{*db, TabCfg};
    cbt.tnmtName = cfg[CfgKey_TnmtName];
    cbt.club = cfg[CfgKey_TnmtOrga];
    cbt.catName = QString2StdString(catName);
    cbt.subtitle = QString2StdString(tr("Preview for initial seeding"));

  } else {
    resize(WidthWitouthBracket, height());
    ui->repView->hide();
  }

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
  keypressTimer->setInterval(SubsequentKeypressTimeout_ms);
  keypressTimer->setSingleShot(true);
  connect(keypressTimer, SIGNAL(timeout()), this, SLOT(onKeypressTimerElapsed()));
}

//----------------------------------------------------------------------------

DlgSeedingEditor::~DlgSeedingEditor()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::initSeedingList(const std::vector<SeedingListWidget::AnnotatedSeedEntry>& _seed)
{
  ui->lwSeeding->clearListAndFillFromSeed(_seed);

  if (_seed.size() > 2)
  {
    ui->sbRangeMin->setMaximum(_seed.size() - 1);
  }

  updateButtons();
  updateBracket();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onBtnUpClicked()
{
  ui->lwSeeding->moveSelectedPlayerUp();
  updateBracket();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onBtnDownClicked()
{
  ui->lwSeeding->moveSelectedPlayerDown();
  updateBracket();
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
  updateBracket();
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

std::vector<int> DlgSeedingEditor::getSeeding()
{
  return ui->lwSeeding->getSeedList();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::onKeypressTimerElapsed()
{
  ui->lwSeeding->warpSelectedPlayerTo(positionInput-1);
  positionInput = 0;
  updateBracket();
}

//----------------------------------------------------------------------------

void DlgSeedingEditor::updateBracket()
{
  if (!msys) return;

  // convert the current seeding list to player pairs
  //
  // FIX ME: the svgBracket should be redesigned to get only
  // pre-filled structs instead of database objects; the bracket and the
  // SVG representation should not need to deal with the database
  PlayerPairList sortedSeed;
  for (const auto& pairId : ui->lwSeeding->getSeedList())
  {
    sortedSeed.push_back(PlayerPair{*db, pairId});
  }

  // prepare a bracket
  try
  {
    auto pages = SvgBracket::substSvgBracketTags(*db, *msys, sortedSeed, {}, cbt);

    if (pages.empty())
    {
      ui->repView->setReport(nullptr);
      bracketRep.reset();
      ui->repView->setEnabled(false);
      return;
    }

    // append all pages to a new report
    auto newBracketReport = std::make_unique<SimpleReportLib::SimpleReportGenerator>(pages[0].width_mm, pages[0].height_mm, 10);
    for (const auto& pg : pages)
    {
      newBracketReport->startNextPage();
      newBracketReport->addSVG_byData_setW(QPointF{0,0}, SimpleReportLib::RECT_CORNER::TOP_LEFT, pg.content, pg.width_mm);
    }

    // replace the current report with the new one
    ui->repView->setReport(newBracketReport.get());
    ui->repView->showPage(0);

    // store the new report and properly delete the old one
    bracketRep = std::move(newBracketReport);
  }
  catch (std::runtime_error&)
  {
    ui->repView->setReport(nullptr);
    bracketRep.reset();
    ui->repView->setEnabled(false);
  }

}

//----------------------------------------------------------------------------

void DlgSeedingEditor::updateButtons()
{
  bool hasItems = (ui->lwSeeding->rowCount() > 0);

  // okay is only possible if there are items
  ui->btnOkay->setEnabled(hasItems);

  // shuffling is only possible with at least two items
  ui->gbShuffle->setEnabled(ui->lwSeeding->rowCount() > 1);

  // shuffling in a certain range only possible with at least three items
  ui->btnShuffle->setEnabled(ui->lwSeeding->rowCount() > 2);

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
