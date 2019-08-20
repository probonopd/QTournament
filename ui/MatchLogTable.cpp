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

#include <QHeaderView>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTableWidgetItem>
#include <QMessageBox>

#include "MatchLogTable.h"
#include "MatchMngr.h"
#include "CentralSignalEmitter.h"
#include "DlgMatchResult.h"

MatchLogTable::MatchLogTable(QWidget* parent)
  :CommonMatchTableWidget{parent}
{
  connect(CentralSignalEmitter::getInstance(), SIGNAL(matchStatusChanged(int,int,ObjState,ObjState)),
          this, SLOT(onMatchStatusChanged(int,int,ObjState,ObjState)), Qt::DirectConnection);

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // if a (running) category is deleted, we need to rebuild the widget's
  // contents, otherwise the deleted matches cause blank rows
  connect(CentralSignalEmitter::getInstance(), SIGNAL(categoryRemovedFromTournament(int,int)),
          this, SLOT(onCategoryRemoved()));

  // prep the context menu
  initContextMenu();
}

//----------------------------------------------------------------------------

std::optional<Match> MatchLogTable::getSelectedMatch() const
{
  if ((currentRow() < 0) || (currentItem() == nullptr)) return nullptr;

  int maId = currentItem()->data(Qt::UserRole).toInt();

  MatchMngr mm{db};
  return mm.getMatch(maId);
}

//----------------------------------------------------------------------------

void MatchLogTable::onMatchStatusChanged(int maId, int maSeqNum, ObjState oldStat, ObjState newStat)
{
  if (newStat != ObjState::MA_FINISHED) return;
  if (db == nullptr) return;

  MatchMngr mm{db};
  auto ma = mm.getMatch(maId);
  if (ma != nullptr) insertMatch(0, *ma);
  resizeRowToContents(0);
}

//----------------------------------------------------------------------------

void MatchLogTable::onCategoryRemoved()
{
  clearContents();
  setRowCount(0);
  fillFromDatabase();
}

//----------------------------------------------------------------------------

void MatchLogTable::onModMatchResultTriggered()
{
  upMatch ma = getSelectedMatch();
  if (ma == nullptr) return;

  // test if we can modify the match result at all
  Category cat = ma->getCategory();
  auto pCat = cat.convertToSpecializedObject();
  if (pCat == nullptr) return;
  ModMatchResult mmr = pCat->canModifyMatchResult(*ma);

  // display an error message an abort
  QString msg;
  if (mmr == ModMatchResult::NotImplemented)
  {
    msg = tr("Modifying match results has not yet been implemented ");
    msg += tr("for the match system that this match belongs to.\n\n");
    msg += tr("Please wait for a later software release... ;)");
  }
  if (mmr == ModMatchResult::NotPossible)
  {
    msg = tr("The result of this match can't be modified anymore.\n\n");
    msg += tr("The tournament has progressed too much.");
  }

  if (!(msg.isEmpty()))
  {
    QMessageBox::critical(this, "Modify match result", msg);
    return;
  }

  // a warning if only limited changes are permitted
  if (mmr == ModMatchResult::ScoreOnly)
  {
    msg = tr("<b>Please note:</b><br>");
    msg += tr("You may modify the match result only to the extend ");
    msg += tr("that the winner/loser doesn't change!");
    QMessageBox::warning(this, "Modify match result", msg);
  }

  // display the dialog for entering match results
  DlgMatchResult dlg{this, *ma};
  int rc = dlg.exec();
  if (rc != QDialog::Accepted)
  {
    return;
  }
  auto matchResult = dlg.getMatchScore();

  // the following check should never trigger,
  // because if there was no valid result, the user
  // should not be able to hit the "Okay" button
  if (matchResult == nullptr)
  {
    msg = tr("An error has occurred. The match result has not been changed.");
    QMessageBox::critical(this, "Modify match result", msg);
    return;
  }

  // try to apply the new score.
  // if the user violated the "score only" constraint it
  // will be detected by the category directly. thus, we do not need
  // to perform any further checks here
  mmr = pCat->modifyMatchResult(*ma, *matchResult);

  if (mmr == ModMatchResult::ScoreOnly)
  {
    msg = tr("Winner/Loser of the new match result is not ");
    msg += tr("identical with the winner/loser of the existing result.\n\n");
    msg += tr("Can't change the winner/loser anymore for this match.\n\n");
    msg += tr("Match result not updated.");
    QMessageBox::critical(this, "Modify match result", msg);
    return;
  }

  if (mmr != ModMatchResult::ModDone)
  {
    msg = tr("An unexpected error occured.\n\n");
    msg += tr("Match result not updated.");
    QMessageBox::critical(this, "Modify match result", msg);
    return;
  }

}

//----------------------------------------------------------------------------

void MatchLogTable::onContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = viewport()->mapToGlobal(pos);

  // resolve the click coordinates to the table row
  int clickedRow = rowAt(pos.y());

  // exit if no table row is under the cursor
  if (clickedRow < 0) return;

  // find out which match is selected
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;  // shouldn't happen

  // show the context menu
  QAction* selectedItem = contextMenu->exec(globalPos);

  if (selectedItem == nullptr) return; // user canceled

}

//----------------------------------------------------------------------------

void MatchLogTable::hook_onDatabaseOpened()
{
  // call parent
  CommonMatchTableWidget::hook_onDatabaseOpened();

  fillFromDatabase();
}

//----------------------------------------------------------------------------

void MatchLogTable::fillFromDatabase()
{
  if (db == nullptr) return;

  MatchMngr mm{db};
  MatchList ml = mm.getFinishedMatches();

  // sort matches by finish time, earliest finisher first
  std::sort(ml.begin(), ml.end(), [](const Match& ma1, const Match& ma2) {
    return ma1.getFinishTime() < ma2.getFinishTime();
  });

  // add them in reverse order: each match is inserted BEFORE the previous one
  for (const Match& ma : ml)
  {
    insertMatch(0, ma);
  }
}

//----------------------------------------------------------------------------

void MatchLogTable::initContextMenu()
{
  // prepare all actions
  actModMatchResult = new QAction(tr("Modify match result..."), this);

  // create the context menu and connect it to the actions
  contextMenu = make_unique<QMenu>();
  contextMenu->addAction(actModMatchResult);

  // connect actions and slots
  connect(actModMatchResult, SIGNAL(triggered(bool)), this, SLOT(onModMatchResultTriggered()));
}

//----------------------------------------------------------------------------

